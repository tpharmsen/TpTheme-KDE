#pragma once

#include "ShapedBlurWindow.h"
#include <QString>
#include <QTimer>

class QuoteWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit QuoteWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    void refreshSpecs();

    QTimer *m_timer;
    QString m_specs;
    qreal m_scrollX = 0;
};
