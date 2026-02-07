#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <errno.h>

#define COLOR_RESET  "\x1b[0m"
#define COLOR_BOLD   "\x1b[1m"
#define COLOR_BLUE   "\x1b[34m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_RED    "\x1b[31m"

// --- Utilities ---

void print_header(const char *title) {
    printf("%s%s[ %s ]%s\n", COLOR_BOLD, COLOR_BLUE, title, COLOR_RESET);
}

// --- Local IP ---
void get_local_ips() {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char addr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, addr, INET_ADDRSTRLEN);
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                printf("  %s%-8s%s %s\n", COLOR_BLUE, ifa->ifa_name, COLOR_RESET, addr);
            }
        }
    }
    freeifaddrs(ifaddr);
}

// --- Gateway ---
void get_gateway() {
    FILE *f = fopen("/proc/net/route", "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char iface[16];
        unsigned int dest, gw;
        if (sscanf(line, "%15s %x %x", iface, &dest, &gw) == 3) {
            if (dest == 0 && gw != 0) {
                struct in_addr addr;
                addr.s_addr = gw;
                printf("  %sGateway:%s  %s (on %s)\n", COLOR_BLUE, COLOR_RESET, inet_ntoa(addr), iface);
                break;
            }
        }
    }
    fclose(f);
}

// --- Interface Stats ---
void get_if_stats() {
    FILE *f = fopen("/proc/net/dev", "r");
    if (!f) return;

    char line[256];
    // Skip headers
    fgets(line, sizeof(line), f);
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        char name[16];
        unsigned long long rx, tx;
        if (sscanf(line, " %15[^:]: %llu %*u %*u %*u %*u %*u %*u %*u %llu", name, &rx, &tx) == 3) {
            if (strcmp(name, "lo") == 0) continue;
            printf("  %s%-8s%s RX: %llu MiB  TX: %llu MiB\n", 
                   COLOR_BLUE, name, COLOR_RESET, rx / (1024*1024), tx / (1024*1024));
        }
    }
    fclose(f);
}

// --- Public IP (The Socket Boss) ---
void get_public_ip() {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("icanhazip.com", "80", &hints, &res) != 0) {
        printf("  %sPublic IP:%s  %sOffline/DNS Error%s\n", COLOR_BLUE, COLOR_RESET, COLOR_RED, COLOR_RESET);
        return;
    }

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0) {
        printf("  %sPublic IP:%s  %sSocket Error%s\n", COLOR_BLUE, COLOR_RESET, COLOR_RED, COLOR_RESET);
        freeaddrinfo(res);
        return;
    }

    struct timeval tv;
    tv.tv_sec = 1; tv.tv_usec = 0; // 1s timeout
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        printf("  %sPublic IP:%s  %sTimeout%s\n", COLOR_BLUE, COLOR_RESET, COLOR_RED, COLOR_RESET);
        close(s);
        freeaddrinfo(res);
        return;
    }

    char *req = "GET / HTTP/1.0\r\nHost: icanhazip.com\r\n\r\n";
    send(s, req, strlen(req), 0);

    char buf[1024];
    int n = recv(s, buf, sizeof(buf)-1, 0);
    if (n > 0) {
        buf[n] = '\0';
        char *body = strstr(buf, "\r\n\r\n");
        if (body) {
            body += 4;
            while (*body == ' ' || *body == '\r' || *body == '\n') body++;
            char *p = body;
            while (*p && *p != '\r' && *p != '\n') p++;
            *p = '\0';
            printf("  %sPublic IP:%s  %s\n", COLOR_BLUE, COLOR_RESET, body);
        } else {
            printf("  %sPublic IP:%s  %sParse Error%s\n", COLOR_BLUE, COLOR_RESET, COLOR_RED, COLOR_RESET);
        }
    } else {
        printf("  %sPublic IP:%s  %sNo Response%s\n", COLOR_BLUE, COLOR_RESET, COLOR_RED, COLOR_RESET);
    }
    close(s);
    freeaddrinfo(res);
}

// --- Active Ping (The "Cheat" - TCP Ping) ---
void do_ping() {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (getaddrinfo("8.8.8.8", "53", &hints, &res) != 0) {
        printf("  %sPing %s8.8.8.8:53%s: %sDown%s\n", COLOR_BLUE, COLOR_BOLD, COLOR_RESET, COLOR_RED, COLOR_RESET);
        return;
    }

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0) { freeaddrinfo(res); return; }

    struct timeval tv;
    tv.tv_sec = 0; tv.tv_usec = 500000; // 500ms timeout
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        printf("  %sPing %s8.8.8.8:53%s: %sDown/Timeout%s\n", COLOR_BLUE, COLOR_BOLD, COLOR_RESET, COLOR_RED, COLOR_RESET);
    } else {
        gettimeofday(&end, NULL);
        double ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
        printf("  %sPing %s8.8.8.8:53%s: %s%.2f ms%s\n", COLOR_BLUE, COLOR_BOLD, COLOR_RESET, COLOR_GREEN, ms, COLOR_RESET);
    }

    close(s);
    freeaddrinfo(res);
}

int main() {
    printf("\n");
    print_header("Mica Network Tool (mnet)");
    
    printf("\n%s  Addresses%s\n", COLOR_BOLD, COLOR_RESET);
    get_local_ips();
    get_public_ip();
    get_gateway();

    printf("\n%s  Traffic%s\n", COLOR_BOLD, COLOR_RESET);
    get_if_stats();

    printf("\n%s  Connectivity%s\n", COLOR_BOLD, COLOR_RESET);
    do_ping();

    printf("\n");
    return 0;
}
