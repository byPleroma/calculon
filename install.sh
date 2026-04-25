#!/bin/bash

# Calculon Calculator Installation Script
# Display name: Calculadora
# Icon: calculon.png

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/usr/local/bin"
DESKTOP_DIR="/usr/share/applications"
ICON_DIR="/usr/share/icons/hicolor/256x256/apps"
ICON_NAME="calculon.png"

# Check for root privileges
if [ "$EUID" -ne 0 ]; then 
    echo "Este script requer privilégios de root. Execute com sudo."
    exit 1
fi

echo "=== Instalando Calculadora ==="

# Compile the application
echo "Compilando..."
cd "$SCRIPT_DIR"
bash compile-calc.sh

# Check if compilation was successful
if [ ! -f "$SCRIPT_DIR/calculon" ]; then
    echo "Erro: Falha na compilação."
    exit 1
fi

# Install binary
echo "Instalando binário em $INSTALL_DIR..."
install -m 755 "$SCRIPT_DIR/calculon" "$INSTALL_DIR/calculon"

# Create icon directory if it doesn't exist
echo "Criando diretório de ícones..."
mkdir -p "$ICON_DIR"

# Check if icon exists, if not create a simple placeholder
if [ -f "$SCRIPT_DIR/$ICON_NAME" ]; then
    echo "Instalando ícone..."
    install -m 644 "$SCRIPT_DIR/$ICON_NAME" "$ICON_DIR/$ICON_NAME"
else
    echo "Aviso: $ICON_NAME não encontrado. Criando ícone placeholder..."
    # Create a simple SVG icon as placeholder
    cat > "$ICON_DIR/$ICON_NAME" << 'EOF'
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256" viewBox="0 0 256 256">
  <rect width="256" height="256" fill="#0c6aa7" rx="32"/>
  <text x="128" y="160" font-family="Arial, sans-serif" font-size="120" font-weight="bold" fill="white" text-anchor="middle">C</text>
  <rect x="40" y="180" width="40" height="40" rx="8" fill="white" opacity="0.9"/>
  <rect x="88" y="180" width="40" height="40" rx="8" fill="white" opacity="0.9"/>
  <rect x="136" y="180" width="40" height="40" rx="8" fill="white" opacity="0.9"/>
  <rect x="184" y="180" width="32" height="40" rx="8" fill="#ffffff" opacity="0.7"/>
</svg>
EOF
    # Convert SVG to PNG if ImageMagick is available
    if command -v convert &> /dev/null; then
        convert "$ICON_DIR/$ICON_NAME" -resize 256x256 "$ICON_DIR/calculon.png"
        rm "$ICON_DIR/$ICON_NAME"
    else
        mv "$ICON_DIR/$ICON_NAME" "$ICON_DIR/calculon.svg"
        ICON_NAME="calculon.svg"
    fi
fi

# Create desktop entry
echo "Criando entrada de aplicativo..."
cat > "$DESKTOP_DIR/calculon.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Calculadora
Name[pt_BR]=Calculadora
Comment=Calculadora científica moderna
Comment[pt_BR]=Calculadora científica moderna
Exec=calculon
Icon=$ICON_NAME
Terminal=false
Categories=Utility;Calculator;GTK;
StartupNotify=true
EOF

chmod 644 "$DESKTOP_DIR/calculon.desktop"

# Update desktop database
echo "Atualizando banco de dados de aplicativos..."
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$DESKTOP_DIR"
fi

# Update icon cache
echo "Atualizando cache de ícones..."
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$ICON_DIR/../.." 2>/dev/null || true
fi

echo ""
echo "=== Instalação concluída com sucesso! ==="
echo "Binário instalado em: $INSTALL_DIR/calculon"
echo "Entrada desktop: $DESKTOP_DIR/calculon.desktop"
echo "Ícone: $ICON_DIR/$ICON_NAME"
echo ""
echo "Você pode iniciar a Calculadora pelo menu de aplicativos ou executando 'calculon' no terminal."
