#include "AnimationWidget.h"

#include <QCoreApplication>
#include <QPainter>
#include <QRandomGenerator>
#include <QSvgRenderer>
#include <QtMath>
#include <cmath>

AnimationWidget::AnimationWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(380, 380);
    renderStarfield();
    renderSignature();

    m_timer = new QTimer(this);
    m_timer->setInterval(42);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_phase = std::fmod(m_phase + 0.006, 4.0 * M_PI);
        update();
    });
    m_timer->start();
}

void AnimationWidget::renderStarfield() {
    constexpr qreal cacheScale = 4.0;
    const QSize size(1520, 1520);
    m_starfield = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_starfield.fill(Qt::transparent);

    QPainter painter(&m_starfield);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QRandomGenerator random(QRandomGenerator::securelySeeded());
    for (int i = 0; i < 140; ++i) {
        const qreal x = (random.generateDouble() * 370.0 - 185.0) * cacheScale;
        const qreal y = (random.generateDouble() * 370.0 - 185.0) * cacheScale;
        const qreal starSize = (0.65 + (i % 4) * 0.35) * cacheScale;
        painter.setBrush(QColor(255, 183, 106,
                                32 + int(random.generateDouble() * 60.0)));
        painter.drawEllipse(QPointF(x + 760.0, y + 760.0), starSize, starSize);
    }
}

void AnimationWidget::renderSignature() {
    const QSize size(480, 232);
    QImage source(size, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::transparent);

    const QString assetPath = QCoreApplication::applicationDirPath()
        + QStringLiteral("/signatureTP.svg");
    QSvgRenderer renderer(assetPath);
    if (!renderer.isValid())
        return;
    QPainter svgPainter(&source);
    renderer.render(&svgPainter, QRectF(QPointF(0, 0), size));
    svgPainter.end();

    m_signature = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_signature.fill(QColor(200, 113, 55));
    QPainter tintPainter(&m_signature);
    tintPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    tintPainter.drawImage(QPoint(0, 0), source);
}

void AnimationWidget::showEvent(QShowEvent *event) {
    ShapedBlurWindow::showEvent(event);
    m_timer->start();
}

void AnimationWidget::hideEvent(QHideEvent *event) {
    m_timer->stop();
    ShapedBlurWindow::hideEvent(event);
}

void AnimationWidget::paintEvent(QPaintEvent *event) {
    ShapedBlurWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(rect().center());

    const qreal scale = qMin(width(), height()) / 380.0;
    painter.scale(scale, scale);

    const QColor orange(200, 113, 55);
    const QColor paleOrange(255, 183, 106);

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRectF(-190.0, -190.0, 380.0, 380.0), m_starfield);

    struct Orbit {
        qreal radius;
        qreal tilt;
        qreal rotation;
        qreal speed;
        qreal size;
        qreal phase;
    };
    const Orbit orbits[] = {
        {88.0, 0.42, -24.0, 1.0, 7.0, 0.2},
        {124.0, 0.55, 38.0, -1.0, 8.0, 2.0},
        {158.0, 0.34, 67.0, 0.5, 13.0, 4.1},
        {178.0, 0.48, -52.0, -0.5, 8.0, 5.4}
    };

    painter.setBrush(Qt::NoBrush);
    for (const Orbit &orbit : orbits) {
        painter.save();
        painter.rotate(orbit.rotation);
        painter.setPen(QPen(QColor(200, 113, 55, 38), 1.0));
        painter.drawEllipse(QPointF(0, 0), orbit.radius,
                            orbit.radius * orbit.tilt);
        painter.restore();
    }

    for (int i = 0; i < 4; ++i) {
        const Orbit &orbit = orbits[i];
        const qreal angle = m_phase * orbit.speed + orbit.phase;
        const QPointF localPlanet(qCos(angle) * orbit.radius,
                                  qSin(angle) * orbit.radius * orbit.tilt);
        QTransform orbitTransform;
        orbitTransform.rotate(orbit.rotation);
        const QPointF planet = orbitTransform.map(localPlanet);

        painter.save();
        painter.translate(planet);
        painter.rotate(qRadiansToDegrees(angle));

        if (i == 1) {
            painter.setPen(QPen(QColor(255, 183, 106, 125), 2.0));
            painter.drawEllipse(QPointF(0, 0), orbit.size + 15.0, 4.2);
            painter.setPen(QPen(QColor(200, 113, 55, 105), 1.0));
            painter.drawEllipse(QPointF(0, 0), orbit.size + 20.0, 5.8);
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(200, 113, 55, 42));
        painter.drawEllipse(QPointF(0, 0), orbit.size + 5.0, orbit.size + 5.0);
        painter.setBrush(i == 2 ? paleOrange : orange);
        painter.drawEllipse(QPointF(0, 0), orbit.size, orbit.size);

        painter.setBrush(QColor(255, 183, 106, 170));
        painter.save();
        painter.resetTransform();
        painter.translate(rect().center());
        painter.scale(scale, scale);
        const auto drawMoon = [&](qreal angle, qreal radius, qreal size) {
            const QPointF localMoon(qCos(angle) * radius,
                                    qSin(angle) * radius * 0.62);
            painter.drawEllipse(planet + orbitTransform.map(localMoon), size, size);
        };
        const qreal moonAngle = m_phase * (3.0 + i) + i;
        drawMoon(moonAngle, orbit.size + 9.0, 2.0 + i * 0.35);
        if (i == 2)
            drawMoon(-m_phase * 5.0 + 1.7, orbit.size + 15.0, 2.5);
        painter.restore();
        painter.restore();
    }

    const qreal pulse = 1.0 + (qSin(m_phase * 2.0) + 1.0) * 0.012;
    painter.save();
    painter.scale(pulse, pulse);

    painter.setPen(QPen(QColor(255, 183, 106, 220), 2.4));
    painter.setBrush(orange);
    painter.drawEllipse(QPointF(0, 0), 20.0, 20.0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(200, 113, 55, 150), 1.2));
    painter.drawEllipse(QPointF(0, 0), 24.0, 24.0);
    painter.restore();

    painter.setOpacity(0.72);
    painter.drawImage(QRectF(32.0, 104.0, 150.0, 72.0), m_signature);
}

QPainterPath AnimationWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
