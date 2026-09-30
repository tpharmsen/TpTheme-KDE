#include "SystemWidget.h"
#include <QFontDatabase>
#include <QPainter>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QTimer>
#include <QCoreApplication>
#include <QFile>

SystemWidget::SystemWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(370, 245);

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

        QFile uptimeFile(QStringLiteral("/proc/uptime"));
        if (uptimeFile.open(QIODevice::ReadOnly)) {
            const double seconds = uptimeFile.readLine().split(' ').first().toDouble();
            const int hours = int(seconds) / 3600;
            const int minutes = (int(seconds) % 3600) / 60;
            m_uptime = QStringLiteral("%1h %2m").arg(hours).arg(minutes);
        }
    };

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, refreshStatus);
    timer->start(500);

    refreshStatus();
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
    const QColor colors[] = {
        QColor(112, 190, 255),
        QColor(205, 137, 255),
        QColor(255, 174, 103),
        QColor(112, 224, 173),
        QColor(112, 190, 255)
    };

    const QString labels[] = {
        QStringLiteral("CPU"), QStringLiteral("RAM"), QStringLiteral("TEMPERATURE"),
        QStringLiteral("GPU"), QStringLiteral("VRAM"), QStringLiteral("UPTIME")
    };
    const int values[] = {
        m_usage.cpu, m_usage.ram, m_usage.temperature, m_usage.gpu, m_usage.vram
    };
    const QRectF cards[] = {
        QRectF(20, 20, 103, 82), QRectF(133, 20, 103, 82), QRectF(246, 20, 104, 82),
        QRectF(20, 116, 103, 82), QRectF(133, 116, 103, 82), QRectF(246, 116, 104, 82)
    };

    const QFont labelFont(QStringLiteral("Sans"), 8, QFont::DemiBold);
    const QFont valueFont(QStringLiteral("Sans"), 14, QFont::DemiBold);

    for (int i = 0; i < 6; ++i) {
        const QRectF card = cards[i];
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 12));
        painter.drawRoundedRect(card, 10, 10);

        painter.setFont(labelFont);
        painter.setPen(secondary);
        painter.drawText(card.adjusted(9, 10, -9, -52), labels[i]);

        painter.setFont(valueFont);
        painter.setPen(primary);
        const QString value = i == 2
            ? QStringLiteral("%1°C").arg(values[i])
            : (i == 5 ? m_uptime : QStringLiteral("%1%").arg(values[i]));
        painter.drawText(card.adjusted(9, 24, -9, -24), value,
                         Qt::AlignRight | Qt::AlignVCenter);

        if (i < 5) {
            const QRectF bar(card.left() + 9, card.bottom() - 16, card.width() - 18, 4);
            const int barValue = i == 2 ? qBound(0, values[i], 100) : values[i];
            painter.setPen(Qt::NoPen);
            painter.setBrush(track);
            painter.drawRoundedRect(bar, 2, 2);
            painter.setBrush(colors[i]);
            painter.drawRoundedRect(
                QRectF(bar.left(), bar.top(), bar.width() * barValue / 100.0, bar.height()),
                2, 2);
        }
    }
}

QPainterPath SystemWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
