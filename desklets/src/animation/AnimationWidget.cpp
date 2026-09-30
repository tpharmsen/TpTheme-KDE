#include "AnimationWidget.h"

#include <iterator>
#include <QPainter>
#include <QtMath>

AnimationWidget::AnimationWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(540, 330);

    m_timer = new QTimer(this);
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_phase += 0.018;
        if (m_phase > 2.0 * M_PI)
            m_phase -= 2.0 * M_PI;
        update();
    });
    m_timer->start();
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.translate(rect().center());

    const qreal scale = qMin(width(), height()) / 420.0;
    painter.scale(scale, scale);

    const QColor orange(200, 113, 55);
    const QColor paleOrange(255, 183, 106);
    const QColor softOrange(200, 113, 55, 32);

    // A sparse, deterministic star field keeps the motion atmospheric without
    // allocating or randomizing anything during paint events.
    const QPointF stars[] = {
        {-150, -105}, {-92, -128}, {-26, -142}, {54, -122}, {142, -96},
        {-176, -34}, {-118, -60}, {-42, -76}, {42, -54}, {126, -42},
        {178, 18}, {104, 60}, {24, 78}, {-64, 66}, {-156, 48},
        {-132, 120}, {-18, 132}, {86, 116}, {164, 98}
    };
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < int(std::size(stars)); ++i) {
        const qreal twinkle = 1.0 + (qSin(m_phase * 2.0 + i * 1.7) + 1.0) * 0.8;
        painter.setBrush(QColor(255, 183, 106, 35 + (i % 3) * 12));
        painter.drawEllipse(stars[i], twinkle, twinkle);
    }

    painter.setPen(QPen(softOrange, 1.4));
    for (int ring = 0; ring < 5; ++ring) {
        const qreal radius = 48.0 + ring * 31.0;
        const qreal wobble = qSin(m_phase * 1.4 + ring * 0.8) * 7.0;
        painter.save();
        painter.rotate(qRadiansToDegrees(m_phase * 0.08 * (ring % 2 ? -1 : 1)));
        painter.drawEllipse(QPointF(0, 0), radius + wobble, radius * 0.56 - wobble * 0.35);
        painter.restore();
    }

    for (int orbit = 0; orbit < 4; ++orbit) {
        const qreal radius = 72.0 + orbit * 35.0;
        const qreal angle = m_phase * (0.75 + orbit * 0.16) + orbit * 1.7;
        const QPointF point(qCos(angle) * radius,
                            qSin(angle) * (radius * 0.56));

        painter.save();
        for (int trail = 5; trail >= 1; --trail) {
            const qreal trailAngle = angle - trail * 0.075;
            const QPointF trailPoint(qCos(trailAngle) * radius,
                                     qSin(trailAngle) * (radius * 0.56));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(200, 113, 55, 10 + (5 - trail) * 7));
            painter.drawEllipse(trailPoint, 3.0 + orbit * 0.45, 3.0 + orbit * 0.45);
        }
        painter.translate(point);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(200, 113, 55, 28));
        painter.drawEllipse(QPointF(0, 0), 16.0 + orbit * 2.0, 16.0 + orbit * 2.0);
        painter.setBrush(QColor(255, 183, 106, 100));
        painter.drawEllipse(QPointF(0, 0), 7.0 + orbit * 0.7, 7.0 + orbit * 0.7);
        painter.setBrush(orange);
        painter.drawEllipse(QPointF(0, 0), 3.0 + orbit * 0.8, 3.0 + orbit * 0.8);
        painter.restore();
    }

    const qreal pulse = 15.0 + (qSin(m_phase * 2.0) + 1.0) * 5.0;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 113, 55, 28));
    painter.drawEllipse(QPointF(0, 0), pulse + 18.0, pulse + 18.0);
    painter.setBrush(QColor(255, 183, 106, 55));
    painter.drawEllipse(QPointF(0, 0), pulse + 8.0, pulse + 8.0);
    painter.setBrush(paleOrange);
    painter.drawEllipse(QPointF(0, 0), pulse, pulse);
}

QPainterPath AnimationWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
