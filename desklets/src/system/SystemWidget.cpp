#include "SystemWidget.h"
#include <QFontDatabase>
#include <QPainter>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QTimer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

SystemWidget::SystemWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(370, 290);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // Accumulate silently — no widget updates yet.
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        m_outputBuffer += QString::fromUtf8(m_process->readAllStandardOutput());
    });

    // Render exactly once, when the full output is in.
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int, QProcess::ExitStatus) {
                updateUsage(m_outputBuffer);
            });

    const QString scriptPath = QCoreApplication::applicationDirPath() + QStringLiteral("/sysbar.sh");

    auto refreshStatus = [this, scriptPath]() {
        if (m_process->state() != QProcess::NotRunning) {
            return;
        }
        m_outputBuffer.clear();
        m_process->start(QStringLiteral("sh"), { QStringLiteral("-c"), scriptPath });

        updateBattery();

        QFile uptimeFile(QStringLiteral("/proc/uptime"));
        if (uptimeFile.open(QIODevice::ReadOnly)) {
            const int seconds = uptimeFile.readLine().split(' ').first().toInt();
            m_uptime = QStringLiteral("%1h %2m")
                .arg(seconds / 3600)
                .arg((seconds % 3600) / 60);
        }
    };

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, refreshStatus);
    timer->start(500);

    refreshStatus();
}

void SystemWidget::updateBattery() {
    m_batteryAvailable = false;
    const QStringList batteries = QDir(QStringLiteral("/sys/class/power_supply"))
        .entryList({QStringLiteral("BAT*")}, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &battery : batteries) {
        QFile capacityFile(QStringLiteral("/sys/class/power_supply/%1/capacity").arg(battery));
        if (!capacityFile.open(QIODevice::ReadOnly))
            continue;

        bool ok = false;
        const int capacity = capacityFile.readLine().trimmed().toInt(&ok);
        if (ok) {
            m_battery = qBound(0, capacity, 100);
            m_batteryAvailable = true;
            return;
        }
    }
}

void SystemWidget::updateUsage(const QString &output) {
    ResourceUsage usage;
    const QRegularExpression labelPattern(QStringLiteral("^(CPU|GPU|VRAM|RAM)\\s*$"));
    const QRegularExpression inlineValuePattern(QStringLiteral("^(CPU|GPU|VRAM|RAM)\\b.*?(\\d+)%"));
    const QRegularExpression valuePattern(QStringLiteral("(\\d+)%"));
    const QRegularExpression temperaturePattern(QStringLiteral("^TEMP\\s+(\\d+)"));
    QString pendingMetric;

    for (const QString &line : output.split(QChar('\n'), Qt::SkipEmptyParts)) {
        const QString trimmedLine = line.trimmed();
        const auto inlineValueMatch = inlineValuePattern.match(trimmedLine);
        if (inlineValueMatch.hasMatch()) {
            const int value = qBound(0, inlineValueMatch.captured(2).toInt(), 100);
            const QString name = inlineValueMatch.captured(1);
            if (name == QStringLiteral("CPU")) usage.cpu = value;
            else if (name == QStringLiteral("GPU")) usage.gpu = value;
            else if (name == QStringLiteral("VRAM")) usage.vram = value;
            else if (name == QStringLiteral("RAM")) usage.ram = value;
            pendingMetric.clear();
            continue;
        }

        const auto labelMatch = labelPattern.match(trimmedLine);
        if (labelMatch.hasMatch()) {
            pendingMetric = labelMatch.captured(1);
            continue;
        }

        const auto valueMatch = valuePattern.match(trimmedLine);
        if (valueMatch.hasMatch() && !pendingMetric.isEmpty()) {
            const int value = qBound(0, valueMatch.captured(1).toInt(), 100);
            const QString name = pendingMetric;
            if (name == QStringLiteral("CPU")) usage.cpu = value;
            else if (name == QStringLiteral("GPU")) usage.gpu = value;
            else if (name == QStringLiteral("VRAM")) usage.vram = value;
            else if (name == QStringLiteral("RAM")) usage.ram = value;
            pendingMetric.clear();
            continue;
        }

        const auto temperatureMatch = temperaturePattern.match(trimmedLine);
        if (temperatureMatch.hasMatch())
            usage.temperature = temperatureMatch.captured(1).toInt();
    }

    m_usage = usage;
    update();
}

void SystemWidget::paintEvent(QPaintEvent *event) {
    ShapedBlurWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor primary(238, 239, 244);
    const QColor secondary(157, 162, 177);
    const QColor track(255, 255, 255, 24);
    const QColor accent(200, 113, 55);
    const QString labels[] = {
        QStringLiteral("CPU"), QStringLiteral("RAM"),
        QStringLiteral("GPU"), QStringLiteral("VRAM")
    };
    const int values[] = {
        m_usage.cpu, m_usage.ram, m_usage.gpu, m_usage.vram
    };
    const QRectF cards[] = {
        QRectF(20, 20, 160, 82), QRectF(190, 20, 160, 82),
        QRectF(20, 116, 160, 82), QRectF(190, 116, 160, 82)
    };

    const QFont labelFont(QStringLiteral("Sans"), 8, QFont::DemiBold);
    const QFont valueFont(QStringLiteral("Sans"), 14, QFont::DemiBold);

    for (int i = 0; i < 4; ++i) {
        const QRectF card = cards[i];
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 12));
        painter.drawRoundedRect(card, 10, 10);

        painter.setFont(labelFont);
        painter.setPen(secondary);
        painter.drawText(card.adjusted(9, 10, -9, -52), labels[i]);

        painter.setFont(valueFont);
        painter.setPen(primary);
        const QString value = QStringLiteral("%1%").arg(values[i]);
        painter.drawText(card.adjusted(9, 24, -9, -24), value,
                         Qt::AlignRight | Qt::AlignVCenter);

        const QRectF bar(card.left() + 9, card.bottom() - 16, card.width() - 18, 4);
        const int barValue = qBound(0, values[i], 100);
        painter.setPen(Qt::NoPen);
        painter.setBrush(track);
        painter.drawRoundedRect(bar, 2, 2);
        painter.setBrush(accent);
        painter.drawRoundedRect(
            QRectF(bar.left(), bar.top(), bar.width() * barValue / 100.0, bar.height()),
            2, 2);
    }

    const QFont smallFont(QStringLiteral("Sans"), 8, QFont::DemiBold);
    painter.setFont(smallFont);
    painter.setPen(secondary);
    painter.drawText(QRectF(20, 220, 95, 16), Qt::AlignCenter, QStringLiteral("TEMP"));
    painter.drawText(QRectF(133, 220, 95, 16), Qt::AlignCenter, QStringLiteral("BATTERY"));
    painter.drawText(QRectF(246, 220, 104, 16), Qt::AlignCenter, QStringLiteral("UPTIME"));

    const QFont statusValueFont(QStringLiteral("Sans"), 14, QFont::DemiBold);
    painter.setFont(statusValueFont);
    painter.setPen(primary);
    painter.drawText(QRectF(20, 239, 95, 35), Qt::AlignCenter,
                     QStringLiteral("%1°C").arg(m_usage.temperature));

    painter.drawText(QRectF(133, 239, 95, 35), Qt::AlignCenter,
                     m_batteryAvailable
                         ? QStringLiteral("%1%").arg(m_battery)
                         : QStringLiteral("N/A"),
                     nullptr);

    painter.drawText(QRectF(246, 239, 104, 35), Qt::AlignCenter, m_uptime);
}

QPainterPath SystemWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
