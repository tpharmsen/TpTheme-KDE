#include "ShapedBlurWindow.h"
#include <KWindowEffects>
#include <QShowEvent>
#include <QResizeEvent>
#include <QGuiApplication>
#include <QWindow>
#include <QPainter>

//#include <LayerShellQt/Window>

#if HAVE_X11
#include <KX11Extras>
#include <netwm.h>
#endif

ShapedBlurWindow::ShapedBlurWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
}


void ShapedBlurWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(
        QPainter::Antialiasing,
        true
    );

    QColor tint(
        36,
        36,
        36,
        128
    );
    painter.setBrush(tint);
    painter.setPen(Qt::NoPen);
    painter.drawPath(shapePath());
}

QPainterPath ShapedBlurWindow::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}

void ShapedBlurWindow::applyShapeAndBlur() {
    if (size().isEmpty())
        return;

    const QPainterPath path = shapePath();
    const QRegion region(path.toFillPolygon().toPolygon());

    setMask(region);

    if (windowHandle())
        KWindowEffects::enableBlurBehind(windowHandle(), true, region);
}
/*
void ShapedBlurWindow::pinToDesktop() {
    if (m_pinned || !windowHandle())
        return;

    qDebug() << "pinToDesktop: platform =" << QGuiApplication::platformName();

    if (QGuiApplication::platformName() == QStringLiteral("wayland")) {
        // Wayland: layer-shell surfaces sit outside KWin's normal toplevel
        // stack entirely, so Show Desktop (Meta+D) has no window to
        // minimize in the first place.

        auto *layerWindow = LayerShellQt::Window::get(windowHandle());
        qDebug() << "pinToDesktop: LayerShellQt::Window::get() =" << static_cast<void*>(layerWindow);
        if (layerWindow) {
            layerWindow->setLayer(LayerShellQt::Window::LayerBackground);
            layerWindow->setAnchors({}); // no anchors: compositor default placement
            layerWindow->setExclusiveZone(-1); // don't reserve screen space
            layerWindow->setKeyboardInteractivity(
                m_wantsKeyboardInput
                ? LayerShellQt::Window::KeyboardInteractivityOnDemand
                : LayerShellQt::Window::KeyboardInteractivityNone);
            m_pinned = true;
            qDebug() << "pinToDesktop: layer-shell pin applied";
        } else {
            qWarning() << "pinToDesktop: LayerShellQt::Window::get() returned null -"
            " useLayerShell() was likely not called before QApplication"
            " was constructed in this executable's main().";
        }

    }
    #if HAVE_X11
    else if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        // X11: KWin's Show Desktop effect skips both NET::Desktop and
        // NET::Dock windows. Desktop-type windows generally can't take
        // keyboard focus, so widgets that need input (the terminal) use
        // Dock instead; display-only widgets (clock, journal) use Desktop.
        KX11Extras::setType(winId(), m_wantsKeyboardInput ? NET::Dock : NET::Desktop);
        KX11Extras::setOnAllDesktops(winId(), true);
        m_pinned = true;
        qDebug() << "pinToDesktop: X11 NET window-type pin applied";
    }
    #endif
    else {
        qWarning() << "pinToDesktop: unrecognized platform, no pin applied";
    }

}
*/

void ShapedBlurWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // windowHandle()/winId() only exist once the native window is created,
    // which QWidget guarantees by the time showEvent fires.
    //pinToDesktop();

    applyShapeAndBlur();
}

void ShapedBlurWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    applyShapeAndBlur();
}
