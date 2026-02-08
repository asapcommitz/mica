# mfetch

`mfetch` is a fast, lightweight system information fetcher written in C using the `musl` library. It focuses on providing essential system details with near-zero overhead.

## Information Displayed

| Module | Source (Linux Pure) | Description |
| :--- | :--- | :--- |
| **OS** | `/etc/os-release` | The pretty name of your Linux distribution. |
| **Host** | `/sys/class/dmi/id/product_name` | The hardware model/product name. |
| **Kernel** | `/proc/sys/kernel/osrelease` | Current running kernel version. |
| **Uptime** | `/proc/uptime` | Time since last system boot. |
| **Shell** | `getenv("SHELL")` | The default user shell. |
| **DE** | `getenv("XDG_CURRENT_DESKTOP")` | Current Desktop Environment. |
| **GPU** | `/sys/class/drm/card*/device/` | Graphics processor detection (Intel/NVIDIA/AMD). |
| **CPU** | `/proc/cpuinfo` | Processor model name. |
| **Memory** | `/proc/meminfo` | Used and Total system RAM. |
| **Disk** | `statvfs()` | Used and Total disk space on `/`. |

## Technical Specs

- **Binary Size:** ~77KB (Statically linked)
- **Library:** `musl`
- **Linkage:** Static (no external dependencies)
- **Data Source:** Direct `/proc` and `/sys` interaction; no external command calls.

## Layout

`mfetch` uses a split layout with a customizable ASCII logo on the left and colored system info on the right.

## Installation

See the main [Building Guide](building.md) for compilation instructions. Once built, you can move the binary to your path:

```bash
sudo cp mfetch /usr/local/bin/
```
