#pragma once
#include "ShapedBlurWindow.h"
#include <QProcess>

class SystemWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit SystemWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    struct ResourceUsage {
        int cpu = 0;
        int gpu = 0;
        int vram = 0;
        int ram = 0;
        int temperature = 0;
    };

    void updateUsage(const QString &output);
    void updateBattery();

    QProcess *m_process;
    QString m_outputBuffer;
    ResourceUsage m_usage;
    int m_battery = 0;
    bool m_batteryAvailable = false;
    QString m_uptime;
};
