# mnet

`mnet` is a tiny, static network diagnostic tool. It provides a quick overview of your network status without the bloat of standard networking suites.

## Features

- **Local IP Detection:** Lists all active IPv4 addresses (excluding loopback).
- **Public IP:** Fetches your external IP via `icanhazip.com` using a raw socket.
- **Gateway & Route:** Identifies the default gateway and the outgoing interface via `/proc/net/route`.
- **Interface Stats:** Shows real-time MiB sent/received since boot from `/proc/net/dev`.
- **TCP Ping (Gatekeeper):** Checks connectivity to `8.8.8.8:53`.

## Why TCP Ping?

Standard ICMP pings require `CAP_NET_RAW` or root privileges on most Linux systems. To keep `mnet` lightweight and accessible for normal users, it uses a **TCP connection handshake** on port 53 (DNS). This provides an accurate latency measurement without needing `sudo`.

## Technical Specs

- **Binary Size:** ~111KB (Statically linked)
- **Socket Logic:** Pure C sockets with custom timeouts (500ms - 1s).
- **Security:** No root required for any standard operation.

## Installation

See the main [Building Guide](building.md) for compilation instructions.

```bash
./mnet
```
