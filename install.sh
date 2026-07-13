#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Destination directories
AURORAE_DIR="$HOME/.local/share/aurorae/themes"
KVANTUM_DIR="$HOME/.local/share/Kvantum"
PLASMA_DIR="$HOME/.local/share/plasma/desktoptheme"
ICONS_DIR="$HOME/.icons"

echo "Installing themes..."

rm -rf "$AURORAE_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-aurorae/" "$AURORAE_DIR/TpTheme/"
rm -rf "$KVANTUM_DIR"
mkdir -p "$KVANTUM_DIR"
cp -r "$SCRIPT_DIR/TpTheme-kvantum/." "$KVANTUM_DIR/"
rm -rf "$PLASMA_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-plasma/" "$PLASMA_DIR/TpTheme/"
rm -rf "$ICONS_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/WhiteSur-Rocket-cursors/" "$ICONS_DIR/TpTheme/"

echo "Created:"
echo "  $AURORAE_DIR/TpTheme/"
echo "  $KVANTUM_DIR/*"
echo "  $PLASMA_DIR/TpTheme/"
echo "  $ICONS_DIR/TpTheme/"

AUTOSTART_DIR="$HOME/.config/autostart"

mkdir -p "$AUTOSTART_DIR"

cat > "$AUTOSTART_DIR/journal-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Journal Widget
Exec=$HOME/Documents/themes/TpTheme-KDE/desklets/build/journal-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/clock-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Clock Widget
Exec=$HOME/Documents/themes/TpTheme-KDE/desklets/build/clock-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/journal2-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Journal2 Widget
Exec=$HOME/Documents/themes/TpTheme-KDE/desklets/build/journal2-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF



echo "Created:"
echo "  $AUTOSTART_DIR/journal-widget.desktop"
echo "  $AUTOSTART_DIR/clock-widget.desktop"
echo "  $AUTOSTART_DIR/journal2-widget.desktop"
echo "Installation complete!"


