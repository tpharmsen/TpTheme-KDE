#include "QuoteWidget.h"

#include <QPainter>

QuoteWidget::QuoteWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(520, 86);
    m_scrollX = width();
    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_scrollX -= 1.5;
        update();
    });
    m_timer->start();
}

void QuoteWidget::paintEvent(QPaintEvent *event) {
    ShapedBlurWindow::paintEvent(event);
    static const QStringList quotes{
        QStringLiteral("STAY CURIOUS."),
        QStringLiteral("MAKE IT WORK, THEN MAKE IT BEAUTIFUL."),
        QStringLiteral("SMALL STEPS STILL MOVE YOU FORWARD."),
        QStringLiteral("SIMPLICITY IS THE SOUL OF EFFICIENCY."),
        QStringLiteral("CREATE SOMETHING WORTH REMEMBERING.")
    };

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QFont font(QStringLiteral("Monospace"), 13, QFont::DemiBold);
    const QFontMetrics metrics(font);
    const QString text = QStringLiteral("   //   %1   //   ").arg(quotes.at(m_quoteIndex));
    const int textWidth = metrics.horizontalAdvance(text);

    if (m_scrollX < -textWidth) {
        m_quoteIndex = (m_quoteIndex + 1) % quotes.size();
        m_scrollX = width();
    }

    p.save();
    p.setClipRect(QRect(18, 18, width() - 36, height() - 36));
    p.setFont(font);
    p.setPen(QColor(200, 113, 55));
    p.drawText(QPointF(m_scrollX, height() / 2.0 + metrics.ascent() / 2.0 - 2), text);

    p.restore();
}

QPainterPath QuoteWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
