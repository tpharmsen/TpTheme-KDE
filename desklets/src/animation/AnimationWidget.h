#pragma once

#include "ShapedBlurWindow.h"
#include <QTimer>

class AnimationWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit AnimationWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QTimer *m_timer;
    qreal m_phase = 0.0;
};
