# mica
mica is my collection of absolutely TINY tools for small PCs like IBM's PowerPC, Raspberry Pi ≤3, Linux devices which don't have much power and other low-end or resource-constrained machine.

## mfetch
mfetch is a fast, lightweight fetch tool written in C, made to show essential system information with minimal overhead on low-end machines. Information is read directly from the system, not from external commands.

### Features
- Fast and lightweight
- Simple and clean interface
- Written in C and the musl library
- Static binary
- Small file size (77KB)

### Usage
```bash
./mfetch
```

or copy the binary to `/usr/local/bin/mfetch` and run `mfetch`.

### Screenshot
<details>
  <summary>Click to expand</summary>
  
  ![mfetch on Fedora Workstation 43](screenshots/mfetch.png)
</details>

---

## mnet
mnet is a fast, lightweight network tool written in C, made to show essential network information with minimal overhead on low-end machines. Information is read directly from the system, not from external commands.

### Features
- Fast and lightweight
- Simple and clean interface
- Written in C and the musl library
- Static binary
- Small file size (111KB)
- Again, no external commands

### Usage
```bash
./mnet
```

or copy the binary to `/usr/local/bin/mnet` and run `mnet`.

### Screenshot
<details>
  <summary>Click to expand</summary>
  
  ![mnet on Fedora Workstation 43](screenshots/mnet.png)
</details>

---

### License
mica is licensed under the [MIT License](LICENSE).
