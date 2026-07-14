#pragma once
#include "ShapedBlurWindow.h"
#include <QTimer>
#include <QSvgRenderer>
#include <QPixmap>
#include <QHash>
#include <QString>
#include <QPointF>
#include <QRectF>
#include <memory>

class ClockWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit ClockWidget(QWidget *parent = nullptr);
protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    struct HandAsset {
        QPixmap pixmap;
        QPointF pivot;
        bool valid = false;
    };

    void scheduleNextTick();
    void onTick();
    void rebuildAssets();

    // Duration of the wild per-second "burst" animation window, shared by
    // the timer scheduler (so it knows how long to tick fast) and the
    // paint code (so it knows how to phase the animation).
    static constexpr double kSnapDurationMs = 420.0;

    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    QTimer *m_timer;
    bool m_showSecondsHand;

    // Cached-rendering state (see rebuildAssets/paintEvent in the .cpp)
    bool m_assetsDirty = true;
    QRectF m_clockRect;
    double m_svgScale = 1.0;
    QPointF m_shadowOffset;
    QPixmap m_facePixmap;
    QPixmap m_overlayPixmap;
    QHash<QString, HandAsset> m_handAssets;
};
