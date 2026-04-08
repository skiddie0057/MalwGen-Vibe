# Malw Generator for AV Testing

Generate test samples with varied hashes for antivirus quality testing.

## Quick Start

```bash
# Linux
bash compilers/compile_linux.sh

# Windows (requires MSVC)
# Run compile_windows.bat
```

## Project Structure

```
malware_generator/
├── common/           # Shared headers (obfuscation, keylogger, network)
├── malw/
│   ├── simple/       # Basic keylogger + HTTPS C2
│   ├── medium/       # Encrypted comms + system collection
│   └── complex/      # Full features: polymorphic code, PRNG, upload/download
├── compilers/
│   ├── compile_linux.sh    # Linux (gcc) + UPX if available
│   └── compile_windows.bat  # Windows (MSVC) + UPX if available
└── output/           # Compiled binaries + .packed variants
```

## Technical Details

- **Cross-platform**: `#ifdef _WIN32` for Windows vs Unix branching
- **Include path**: Compile with `-I<project_root>` so source files use `#include "common/*.h"`
- **Linux flags**: `-no-pie -z execstack -fno-stack-protector`
- **Randomness**: Each build has unique build_id, randomized encryption keys, varying delays
- **Network**: Unix raw sockets (port 443), Windows WinHTTP

## Malware Features by Complexity

| Variant | Keylogger | C2 | Obfuscation | Extras |
|---------|-----------|-----|-------------|--------|
| Simple | Console | Basic HTTPS | XOR strings | PRNG delays |
| Medium | Console | Encrypted | Polymorphic encoding | Build ID, randomized keys |
| Complex | Console | Full upload/download | Multi-layer PRNG | System exfil, session IDs |

## Output Files

Each compilation creates:
- `simple_malw` / `simple_malw.exe` - Original binary
- `simple_malw.packed` / `simple_malw.packed.exe` - UPX compressed (if UPX available)
