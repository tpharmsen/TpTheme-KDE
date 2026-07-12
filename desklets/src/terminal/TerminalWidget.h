#pragma once
#include "ShapedBlurWindow.h"

class QTermWidget;

class TerminalWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit TerminalWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    QTermWidget *m_term;
};
