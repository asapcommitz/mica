#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/statvfs.h>

#define COLOR_RESET  "\x1b[0m"
#define COLOR_BOLD   "\x1b[1m"
#define COLOR_BLUE   "\x1b[34m"

#define NUM_LINES 14

static const char *logo[] = {
    "             %s.',;::::;,'.         %s",
    "         %s.';:cccccccccccc:;,.     %s",
    "      %s.;cccccccccccccccccccccc;.  %s",
    "    %s.:cccccccccccccccccccccccccc:.%s",
    "  %s.;ccccccccccccc;.:dddl:.;ccccccc;.%s",
    " %s.:ccccccccccccc;OWMKOOXMWd;ccccccc:.%s",
    "%s.:ccccccccccccc;KMMc;cc;xMMc;ccccccc:.%s",
    "%s,cccccccccccccc;MMM.;cc;;WW:;cccccccc,%s",
    "%s:cccccccccccccc;MMM.;cccccccccccccccc:%s",
    "%s:ccccccc;oxOOOo;MMM000k.;cccccccccccc:%s",
    " %scccccc;0MMKxdd:;MMMkddc.;cccccccccccc;%s",
    " %sccccc;XMO';cccc;MMM.;cccccccccccccccc'%s",
    " %sccccc;MMo;ccccc;MMW.;ccccccccccccccc;%s",
    " %sccccc;0MNc.ccc.xMMd;ccccccccccccccc;%s",
    "  %scccccc;dNMWXXXWM0:;cccccccccccccc:,%s",
    "   %scccccccc;.:odl:.;cccccccccccccc:,.%s",
    "    %sccccccccccccccccccccccccccccc:'.%s",
    "     %s:ccccccccccccccccccccccc:;,..%s",
    "      %s':cccccccccccccccc::;,.%s"
};

static void read_file(const char *path, char *buf, size_t len) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { strncpy(buf, "Unknown", len); return; }
    ssize_t n = read(fd, buf, len - 1);
    buf[n > 0 ? n : 0] = '\0';
    if (n > 0) { char *p = strchr(buf, '\n'); if (p) *p = '\0'; }
    close(fd);
}

void get_gpu(char *buf, size_t len) {
    char v[16] = "0", d[16] = "0";
    // Try card1 then card0
    int fd = open("/sys/class/drm/card1/device/vendor", O_RDONLY);
    if (fd < 0) fd = open("/sys/class/drm/card0/device/vendor", O_RDONLY);
    if (fd >= 0) { read(fd, v, 15); close(fd); }
    
    fd = open("/sys/class/drm/card1/device/device", O_RDONLY);
    if (fd < 0) fd = open("/sys/class/drm/card0/device/device", O_RDONLY);
    if (fd >= 0) { read(fd, d, 15); close(fd); }

    if (strstr(v, "0x8086")) {
        if (strstr(d, "0x9bc8")) strncpy(buf, "Intel UHD Graphics 630", len);
        else strncpy(buf, "Intel Graphics", len);
    } else if (strstr(v, "0x10de")) strncpy(buf, "NVIDIA GPU", len);
    else if (strstr(v, "0x1002")) strncpy(buf, "AMD GPU", len);
    else strncpy(buf, "Unknown GPU", len);
}

int main() {
    char os[64]="Unknown", host[64], kern[64], uptime[32], cpu[128]="Unknown", mem[64], disk[64], gpu[64];
    
    // OS
    FILE *f = fopen("/etc/os-release", "r");
    if (f) {
        char line[256];
        while (fgets(line, 256, f))
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *s = line + 13; char *e = strrchr(s, '"'); if (e) *e = '\0';
                strncpy(os, s, 64); break;
            }
        fclose(f);
    }

    read_file("/sys/class/dmi/id/product_name", host, 64);
    read_file("/proc/sys/kernel/osrelease", kern, 64);
    
    // Uptime
    f = fopen("/proc/uptime", "r");
    if (f) {
        double s; fscanf(f, "%lf", &s);
        int h = (int)s/3600, m = ((int)s%3600)/60;
        if (h>0) snprintf(uptime, 32, "%dh %dm", h, m);
        else snprintf(uptime, 32, "%dm", m);
        fclose(f);
    }

    // CPU
    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, 256, f))
            if (strncmp(line, "model name", 10) == 0) {
                char *s = strchr(line, ':');
                if (s) { s += 2; char *n = strchr(s, '\n'); if (n) *n = '\0'; strncpy(cpu, s, 128); break; }
            }
        fclose(f);
    }

    // Mem
    f = fopen("/proc/meminfo", "r");
    if (f) {
        long t=0, fr=0, b=0, c=0, s=0;
        char line[256];
        while (fgets(line, 256, f)) {
            if (sscanf(line, "MemTotal: %ld kB", &t)){}
            else if (sscanf(line, "MemFree: %ld kB", &fr)){}
            else if (sscanf(line, "Buffers: %ld kB", &b)){}
            else if (sscanf(line, "Cached: %ld kB", &c)){}
            else if (sscanf(line, "Slab: %ld kB", &s)){}
        }
        snprintf(mem, 64, "%ldMiB / %ldMiB", (t-fr-b-c-s)/1024, t/1024);
        fclose(f);
    }

    // Disk
    struct statvfs v;
    if (statvfs("/", &v) == 0) {
        unsigned long long t = (unsigned long long)v.f_blocks * v.f_frsize;
        unsigned long long fr = (unsigned long long)v.f_bfree * v.f_frsize;
        snprintf(disk, 64, "%lluGiB / %lluGiB", (t-fr)/(1024*1024*1024), t/(1024*1024*1024));
    }

    get_gpu(gpu, 64);
    char *user = getenv("USER"), hname[64]; gethostname(hname, 64);
    char *sh = getenv("SHELL"); if (sh) { char *l = strrchr(sh, '/'); if (l) sh = l+1; }
    char *de = getenv("XDG_CURRENT_DESKTOP");

    char info[NUM_LINES][256];
    snprintf(info[0], 256, "%s%s%s@%s%s%s", COLOR_BLUE, user?user:"user", COLOR_RESET, COLOR_BLUE, hname, COLOR_RESET);
    snprintf(info[1], 256, "------------");
    snprintf(info[2], 256, "%sOS:%s      %s", COLOR_BLUE, COLOR_RESET, os);
    snprintf(info[3], 256, "%sHost:%s    %s", COLOR_BLUE, COLOR_RESET, host);
    snprintf(info[4], 256, "%sKernel:%s  %s", COLOR_BLUE, COLOR_RESET, kern);
    snprintf(info[5], 256, "%sUptime:%s  %s", COLOR_BLUE, COLOR_RESET, uptime);
    snprintf(info[6], 256, "%sShell:%s   %s", COLOR_BLUE, COLOR_RESET, sh ? sh : "unknown");
    snprintf(info[7], 256, "%sDE:%s      %s", COLOR_BLUE, COLOR_RESET, de ? de : "n/a");
    snprintf(info[8], 256, "%sGPU:%s     %s", COLOR_BLUE, COLOR_RESET, gpu);
    snprintf(info[9], 256, "%sCPU:%s     %s", COLOR_BLUE, COLOR_RESET, cpu);
    snprintf(info[10], 256, "%sMemory:%s  %s", COLOR_BLUE, COLOR_RESET, mem);
    snprintf(info[11], 256, "%sDisk:%s    %s", COLOR_BLUE, COLOR_RESET, disk);
    for (int i=12; i<NUM_LINES; i++) info[i][0] = '\0';

    for (int i = 0; i < 19; i++) {
        printf(logo[i], COLOR_BLUE, (i < NUM_LINES) ? info[i] : "");
        printf("%s\n", COLOR_RESET);
    }
    return 0;
}
