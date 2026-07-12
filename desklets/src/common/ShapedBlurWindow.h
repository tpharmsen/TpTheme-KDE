#pragma once
#include <QWidget>
#include <QPainterPath>
#include <QPaintEvent>

// Base class for a frameless, translucent, non-rectangular desktop widget
// whose shape is also its KWin blur-behind region.
//
// Also pins itself below normal windows and outside the "Show Desktop"
// (Meta+D) window set, using LayerShellQt on Wayland and NET::Desktop
// via KX11Extras on X11.
class ShapedBlurWindow : public QWidget {
    Q_OBJECT
public:
    explicit ShapedBlurWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    // Override in subclasses to define the widget's shape in local coordinates.
    virtual QPainterPath shapePath() const;

    void applyShapeAndBlur();

    // Set to true in a subclass constructor (e.g. TerminalWidget) if the
    // widget needs to receive keyboard input. Defaults to false, since the
    // clock/journal widgets are display-only.
    bool m_wantsKeyboardInput = false;

private:
    void pinToDesktop();
    bool m_pinned = false;
};
