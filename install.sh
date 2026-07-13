#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Destination directories
AURORAE_DIR="$HOME/.local/share/aurorae/themes"
KVANTUM_DIR="$HOME/.local/share/Kvantum"
PLASMA_DIR="$HOME/.local/share/plasma/desktoptheme"
ICONS_DIR="$HOME/.icons"

echo "Installing themes..."

cp -r "$SCRIPT_DIR/TpTheme-aurorae/" "$AURORAE_DIR/"
mv "$AURORAE_DIR/TpTheme-aurorae/" "$AURORAE_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-kvantum/" "$KVANTUM_DIR/"
cp -r "$SCRIPT_DIR/TpTheme-plasma/" "$PLASMA_DIR/"
cp -r "$SCRIPT_DIR/WhiteSur-Rocket-cursors/" "$ICONS_DIR/"

echo "Installation complete!"
