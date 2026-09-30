#pragma once

#include "ShapedBlurWindow.h"
#include <QTimer>

class QuoteWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit QuoteWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *m_timer;
    int m_quoteIndex = 0;
    qreal m_scrollX = 0;
};
