#pragma once

#include "ShapedBlurWindow.h"
#include <QImage>
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
    void renderStarfield();
    void renderSignature();

    QTimer *m_timer;
    QImage m_starfield;
    QImage m_signature;
    qreal m_phase = 0.0;
};
