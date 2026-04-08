# AV Testing Malw Generator

Generates controlled test samples with varying hashes for antivirus quality testing.

## Structure

```
malware_generator/
├── common/           # Shared headers (obfuscation, keylogger, network)
├── malw/
│   ├── simple/       # Basic keylogger + HTTPS C2
│   ├── medium/       # Encrypted comms + system collection
│   └── complex/      # Full-featured with polymorphic code
├── compilers/
│   ├── compile_linux.sh
│   └── compile_windows.bat
└── output/           # Compiled binaries + .packed variants
```

## Usage

### Linux
```bash
cd malware_generator
chmod +x compilers/compile_linux.sh
./compilers/compile_linux.sh
# Output in output/linux/
```

### Windows
```cmd
cd malware_generator\compilers
compile_windows.bat
# Output in output\windows\
```

Requires MSVC compiler (cl.exe) on Windows.

## Variants

| Variant | Keylogger | C2 | Obfuscation | Extras |
|---------|-----------|-----|-------------|--------|
| Simple | Console | Basic HTTPS | XOR strings | PRNG delays |
| Medium | Console | Encrypted | Polymorphic | Unique build ID |
| Complex | Console | Full upload/download | Multi-layer PRNG | System exfil |

## Output

Each compilation creates:
- `simple_malw` - Original binary
- `simple_malw.packed` - UPX compressed (if UPX available)

## Randomness

Each build generates:
- Unique 8-char hex build ID
- Randomized encryption keys
- PRNG-seeded delays and file paths

## Warning

FOR TESTING ONLY. Run in isolated sandbox environments.
