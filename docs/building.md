# Building Mica Tools

The Mica collection is designed to be built with `musl-gcc` to produce small, statically-linked binaries that work on any Linux system without dependencies.

## Prerequisites

You need the `musl` C library and its development headers installed.

**Fedora:**
```bash
sudo dnf install musl-gcc
```

**Ubuntu/Debian:**
```bash
sudo apt install musl-tools
```

## Compilation

Each tool has its own directory and Makefile.

### mfetch
```bash
cd mfetch
make
```

### mnet
```bash
cd mnet
make
```

## Optimization Flags Used

The Makefiles use the following flags to ensure the smallest possible binary:

- `-Os`: Optimize for size.
- `-static`: Link all libraries statically.
- `-s`: Strip all symbols from the final binary (further reducing size).
- `-std=c99`: Ensure modern C standard compliance.

## Portability

Because Mica tools are statically linked against `musl`, you can copy the resulting binaries to any other Linux machine (of the same architecture) and they will run perfectly, even if that machine doesn't have `musl` installed.
