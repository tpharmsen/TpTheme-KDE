# KDE Plasma 6 — Shaped, Live-Blurred Desktop Widgets

Three standalone apps — a circular clock, a hexagonal terminal, and a
rounded `journalctl -f` viewer — each frameless, non-rectangular, and
live-blurred against your desktop wallpaper via KWin's real compositor
blur (`KWindowEffects::enableBlurBehind`).

The core logic (`ShapedBlurWindow`) was compiled and linked successfully
against Qt5/KF5WindowSystem during development as a sanity check, since
this exact API is unchanged between KF5 and KF6 — `clock-widget` and
`journal-widget` built and linked cleanly with no errors. This
`CMakeLists.txt` targets **Qt6 + KF6WindowSystem**, which is what your
Plasma 6 desktop already has installed.

## 1. Install dependencies

**Arch:**
```bash
sudo pacman -S cmake qt6-base extra-cmake-modules kwindowsystem
yay -S qtermwidget                 # for the terminal widget
```

**Fedora:**
```bash
sudo dnf install cmake qt6-qtbase-devel extra-cmake-modules kf6-kwindowsystem-devel
sudo dnf install qtermwidget5-devel   # or the qt6 variant if your repo has split it
```

**Debian/Ubuntu (with KDE Neon/Kubuntu or a recent Plasma 6 install):**
```bash
sudo apt install cmake qt6-base-dev libkf6windowsystem-dev
sudo apt install libqtermwidget5-1-dev   # or qtermwidget6-dev if available
```

If `qtermwidget` isn't available, `cmake` will still configure
successfully and just skip the `terminal-widget` target — you'll get the
clock and journal widgets, and can add the terminal target later.

## 2. Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Binaries land in `build/`: `clock-widget`, `journal-widget`, and (if
qtermwidget was found) `terminal-widget`.

Optional install:
```bash
sudo make install     # puts them in /usr/local/bin
```

## 3. Set up the video wallpaper

Right-click desktop → **Configure Desktop and Wallpaper** → Wallpaper
Type → **Video Wallpaper**. If it's not listed:
```bash
# Arch
sudo pacman -S plasma6-wallpapers-video
# Fedora
sudo dnf install plasma-wallpapers-video
# Debian/Ubuntu
sudo apt install plasma-wallpaper-video
```
Pick a video encoded at 30fps, looping enabled.

## 4. Enable KWin Blur

**System Settings → Apps & Windows → Desktop Effects → Blur** → toggle on.

## 5. Run the widgets

```bash
./build/clock-widget &
./build/journal-widget &
./build/terminal-widget &
```

Each opens as a frameless, shaped, blurred window near the top-left of
your screen (drag isn't wired up — reposition however you like, e.g. by
editing the `resize()`/adding a `move()` call in each widget's
constructor, or by adding your own drag handling in `mousePressEvent` /
`mouseMoveEvent` on `ShapedBlurWindow`).

## 6. Pin them as permanent desktop widgets

By default these are just floating windows. To make them behave like
real desktop widgets — always below other windows, no taskbar entry,
visible on every virtual desktop — set a KWin Window Rule per app:

1. Run the widget.
2. Right-click its titlebar area (or use **Alt+F3** while it's focused) →
   **More Actions → Configure Special Window Settings**.
3. In the dialog, set and **force**:
   - **Keep below other windows** → Force → Yes
   - **Skip taskbar** → Force → Yes
   - **Skip switcher** → Force → Yes
   - **Skip pager** → Force → Yes
   - **On all desktops** → Force → Yes
4. Apply. Repeat for each of the three widgets.

You can review/edit these later in **System Settings → Window Management
→ Window Rules**.

## Project layout

```
CMakeLists.txt
src/
  common/
    ShapedBlurWindow.h / .cpp   — shared base: masking + blur-behind
  clock/
    ClockWidget.h / .cpp        — circular clock
    main.cpp
  journal/
    JournalWidget.h / .cpp      — rounded-rect journalctl -f tail viewer
    main.cpp
  terminal/
    TerminalWidget.h / .cpp     — hexagonal, QTermWidget-backed terminal
    main.cpp
```

## Customizing shapes

Every widget shape is defined by overriding `shapePath()`, which returns
a `QPainterPath` in the widget's local coordinates. That same path is
used both as the click-mask and the KWin blur region, so anything you
draw there — a blob made of Bézier curves, an imported SVG path, a
star — works automatically for both. See `TerminalWidget::shapePath()`
for an example that clips the corners into a hexagon, versus
`ClockWidget::shapePath()`'s plain ellipse.

## Notes

- `journal-widget` runs `journalctl -f --no-pager -n 50 -o short`. Add
  `-u <unit>` to follow a specific service instead of the whole journal.
- For `systemctl status` instead of the log stream, either poll it on a
  `QTimer` with `QProcess`, or (more efficient) subscribe to
  `PropertiesChanged` signals on `org.freedesktop.systemd1` over D-Bus.
- `TerminalWidget` reads `$SHELL` and falls back to `/bin/bash`.
