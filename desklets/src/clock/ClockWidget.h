#pragma once
#include "ShapedBlurWindow.h"
#include <QTimer>
#include <QSvgRenderer>
#include <memory>

class ClockWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit ClockWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    QTimer *m_timer;
    bool m_showSecondsHand;
};
