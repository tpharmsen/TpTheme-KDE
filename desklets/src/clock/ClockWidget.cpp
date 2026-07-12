#include "ClockWidget.h"
#include <QPainter>
#include <QTime>
#include <cmath>

ClockWidget::ClockWidget(QWidget *parent)
: ShapedBlurWindow(parent), m_showSecondsHand(true) {
    // Set a square size matching the analog aspect ratio

    move(250, 250);
    resize(480, 480);

    // Load the clock theme from the Qt resource bundle
    m_svgRenderer = std::make_unique<QSvgRenderer>(QStringLiteral(":/clock.svgz"));

    // FIXED: Changed from 1000ms to 16ms (~60 FPS) to allow for fluid animations
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this] { update(); });
    m_timer->start(16);
}

QPainterPath ClockWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}

void ClockWidget::paintEvent(QPaintEvent *event) {

    ShapedBlurWindow::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (!m_svgRenderer || !m_svgRenderer->isValid()) return;

    QTime time = QTime::currentTime();

    // 1. Establish uniform square drawing boundaries inside the widget
    int side = qMin(width(), height());
    QRectF clockRect(0, 0, side, side);
    clockRect.moveCenter(rect().center());

    // 2. Compute the exact scale ratio relative to the native ClockFace element
    QRectF faceNatural = m_svgRenderer->boundsOnElement(QStringLiteral("ClockFace"));
    if (faceNatural.isEmpty()) return;
    double svgScale = clockRect.width() / faceNatural.width();

    // 3. Compute Drop Shadow Offsets (from theme metadata hints)
    double naturalHShadow = 0;
    if (m_svgRenderer->elementExists(QStringLiteral("hint-hands-shadow-offset-to-west"))) {
        naturalHShadow = -m_svgRenderer->boundsOnElement(QStringLiteral("hint-hands-shadow-offset-to-west")).width();
    } else if (m_svgRenderer->elementExists(QStringLiteral("hint-hands-shadows-offset-to-east"))) {
        naturalHShadow = m_svgRenderer->boundsOnElement(QStringLiteral("hint-hands-shadows-offset-to-east")).width();
    }

    double naturalVShadow = 0;
    if (m_svgRenderer->elementExists(QStringLiteral("hint-hands-shadow-offset-to-north"))) {
        naturalVShadow = -m_svgRenderer->boundsOnElement(QStringLiteral("hint-hands-shadow-offset-to-north")).height();
    } else if (m_svgRenderer->elementExists(QStringLiteral("hint-hands-shadow-offset-to-south"))) {
        naturalVShadow = m_svgRenderer->boundsOnElement(QStringLiteral("hint-hands-shadow-offset-to-south")).height();
    }

    double horizontalShadowOffset = std::round(naturalHShadow * svgScale) + (int)std::round(naturalHShadow * svgScale) % 2;
    double verticalShadowOffset = std::round(naturalVShadow * svgScale) + (int)std::round(naturalVShadow * svgScale) % 2;

    // 4. Draw base Clock Face
    m_svgRenderer->render(&p, QStringLiteral("ClockFace"), clockRect);

    // 5. Matrix-backed Universal Hand Renderer with Radial Offset Control
    auto drawHand = [&](const QString &elementId, const QString &rotationCenterHintId, double angle,
                        double dx, double dy, double radialOffset) {
        if (!m_svgRenderer->elementExists(elementId)) return;

        QRectF handRect = m_svgRenderer->boundsOnElement(elementId);

        double hintedX = handRect.width() / 2.0;
        double hintedY = handRect.height() / 2.0;

        if (!rotationCenterHintId.isEmpty() && m_svgRenderer->elementExists(rotationCenterHintId)) {
            QRectF hintedCenterRect = m_svgRenderer->boundsOnElement(rotationCenterHintId);
            hintedX = hintedCenterRect.x() - handRect.x() + hintedCenterRect.width() / 2.0;
            hintedY = hintedCenterRect.y() - handRect.y() + hintedCenterRect.height() / 2.0;
        }

        p.save();
        p.translate(clockRect.center() + QPointF(dx, dy));
        p.rotate(angle);
        p.translate(0, -radialOffset * svgScale);

        QRectF targetBounds(
            -hintedX * svgScale,
            -hintedY * svgScale,
            handRect.width() * svgScale,
                            handRect.height() * svgScale
        );

        m_svgRenderer->render(&p, elementId, targetBounds);
        p.restore();
        };

        // 6. ANIMATED TIME ANGLE CALCULATION
        double baseSecond = time.second();
        double msec = time.msec();
        double animatedSecond = baseSecond;

        // --- STYLE: MECHANICAL SPRING SNAP ---
        // The hand spends the first 250ms of a second executing a sharp snap
        // with a microscopic mechanical spring bounce at the end, then rests.
        double snapDuration = 250.0;
        if (msec < snapDuration) {
            double t = msec / snapDuration;
            double ts = t - 1.0;
            double bounceOvershoot = 1.4; // Higher number = more springiness
            double easeBackOut = ts * ts * ((bounceOvershoot + 1.0) * ts + bounceOvershoot) + 1.0;

            animatedSecond = (baseSecond - 1.0) + easeBackOut;
        }

        // Angles updated to use high-precision sub-second positions
        double hourAngle   = 180.0 + (time.hour() % 12 * 30.0) + (time.minute() * 0.5);
        double minuteAngle = 180.0 + (time.minute() * 6.0) + (time.second() * 0.1);
        double secondAngle = 180.0 + (animatedSecond * 6.0);

        // 7. SPECIFIC HAND RADIAL OFFSETS
        double hourRadialOffset   = -23.0;
        double minuteRadialOffset = -37.0;
        double secondRadialOffset = -36.5;

        // 8. Render Stacking Hierarchy
        // --- HOUR HAND ---
        drawHand(QStringLiteral("HourHandShadow"), QStringLiteral("hint-hourhandshadow-rotation-center-offset"), hourAngle, horizontalShadowOffset, verticalShadowOffset, hourRadialOffset);
        drawHand(QStringLiteral("HourHand"),       QStringLiteral("hint-hourhand-rotation-center-offset"),       hourAngle, 0, 0, hourRadialOffset);

        // --- MINUTE HAND ---
        drawHand(QStringLiteral("MinuteHandShadow"), QStringLiteral("hint-minutehandshadow-rotation-center-offset"), minuteAngle, horizontalShadowOffset, verticalShadowOffset, minuteRadialOffset);
        drawHand(QStringLiteral("MinuteHand"),       QStringLiteral("hint-minutehand-rotation-center-offset"),       minuteAngle, 0, 0, minuteRadialOffset);

        // --- SECOND HAND ---
        if (m_showSecondsHand) {
            drawHand(QStringLiteral("SecondHandShadow"), QStringLiteral("hint-secondhandshadow-rotation-center-offset"), secondAngle, horizontalShadowOffset, verticalShadowOffset, secondRadialOffset);
            drawHand(QStringLiteral("SecondHand"),       QStringLiteral("hint-secondhand-rotation-center-offset"),       secondAngle, 0, 0, secondRadialOffset);
        }

        // Overlays
        m_svgRenderer->render(&p, QStringLiteral("HandCenterScrew"), clockRect);
        m_svgRenderer->render(&p, QStringLiteral("Glass"), clockRect);
}
