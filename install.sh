#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Destination directories
AURORAE_DIR="$HOME/.local/share/aurorae/themes"
KVANTUM_DIR="$HOME/.config/Kvantum"
PLASMA_DIR="$HOME/.local/share/plasma/desktoptheme"
ICONS_DIR="$HOME/.icons"

echo "Installing themes..."

rm -rf "$AURORAE_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-aurorae/" "$AURORAE_DIR/TpTheme/"
rm -rf "$KVANTUM_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-kvantum/" "$KVANTUM_DIR/TpTheme/"
rm -rf "$PLASMA_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/TpTheme-plasma/" "$PLASMA_DIR/TpTheme/"
rm -rf "$ICONS_DIR/TpTheme/"
cp -r "$SCRIPT_DIR/WhiteSur-Rocket-cursors/" "$ICONS_DIR/TpTheme/"

echo "Created:"
echo "  $AURORAE_DIR/TpTheme/"
echo "  $KVANTUM_DIR/TpTheme/"
echo "  $PLASMA_DIR/TpTheme/"
echo "  $ICONS_DIR/TpTheme/"

AUTOSTART_DIR="$HOME/.config/autostart"
DESKLETS_BUILD_DIR="$SCRIPT_DIR/desklets/build"

mkdir -p "$AUTOSTART_DIR"

cat > "$AUTOSTART_DIR/journal-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Journal Widget
Exec=$DESKLETS_BUILD_DIR/journal-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/clock-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Clock Widget
Exec=$DESKLETS_BUILD_DIR/clock-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/system-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=System Widget
Exec=$DESKLETS_BUILD_DIR/system-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/quote-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Quote Widget
Exec=$DESKLETS_BUILD_DIR/quote-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

cat > "$AUTOSTART_DIR/animation-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Animation Widget
Exec=$DESKLETS_BUILD_DIR/animation-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF

if [[ -x "$DESKLETS_BUILD_DIR/terminal-widget" ]]; then
    cat > "$AUTOSTART_DIR/terminal-widget.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Terminal Widget
Exec=$DESKLETS_BUILD_DIR/terminal-widget
Hidden=false
Enabled=true
X-KDE-Autostart-Phase=Desktop
EOF
fi

echo "Created:"
echo "  $AUTOSTART_DIR/journal-widget.desktop"
echo "  $AUTOSTART_DIR/clock-widget.desktop"
echo "  $AUTOSTART_DIR/system-widget.desktop"
echo "  $AUTOSTART_DIR/quote-widget.desktop"
echo "  $AUTOSTART_DIR/animation-widget.desktop"
if [[ -f "$AUTOSTART_DIR/terminal-widget.desktop" ]]; then
    echo "  $AUTOSTART_DIR/terminal-widget.desktop"
fi
echo "Installation complete!"
