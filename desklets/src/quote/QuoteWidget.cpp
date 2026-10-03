#include "QuoteWidget.h"

#include <QFile>
#include <QProcess>
#include <QSysInfo>
#include <QPainter>

QuoteWidget::QuoteWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(560, 50);
    m_scrollX = width();
    refreshSpecs();

    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_scrollX -= 1.5;
        update();
    });
    m_timer->start();
}

void QuoteWidget::refreshSpecs() {
    auto readFirstLine = [](const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();
        return QString::fromUtf8(file.readLine()).trimmed();
    };

    QString os = QStringLiteral("Linux");
    QFile osRelease(QStringLiteral("/etc/os-release"));
    if (osRelease.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!osRelease.atEnd()) {
            const QString line = QString::fromUtf8(osRelease.readLine()).trimmed();
            if (line.startsWith(QStringLiteral("PRETTY_NAME="))) {
                os = line.mid(12).trimmed();
                if (os.startsWith('"') && os.endsWith('"'))
                    os = os.mid(1, os.size() - 2);
                break;
            }
        }
    }

    auto runCommand = [](const QString &program, const QStringList &arguments) {
        QProcess process;
        process.start(program, arguments);
        if (!process.waitForFinished(1000)) {
            process.kill();
            process.waitForFinished(100);
            return QString();
        }
        return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    };

    QString cpu = QStringLiteral("Unknown");
    QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
    if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!cpuInfo.atEnd()) {
            const QString line = QString::fromUtf8(cpuInfo.readLine()).trimmed();
            if (line.startsWith(QStringLiteral("model name"))
                || line.startsWith(QStringLiteral("Hardware"))
                || line.startsWith(QStringLiteral("Processor"))) {
                cpu = line.section(':', 1).trimmed();
                if (!cpu.isEmpty())
                    break;
            }
        }
    }
    if (cpu == QStringLiteral("Unknown")) {
        const QString lscpu = runCommand(QStringLiteral("lscpu"), {});
        for (const QString &line : lscpu.split('\n')) {
            if (line.startsWith(QStringLiteral("Model name:"))) {
                cpu = line.section(':', 1).trimmed();
                break;
            }
        }
    }
    if (cpu.isEmpty())
        cpu = QStringLiteral("Unknown");

    QString memory = QStringLiteral("Unknown");
    const QString memInfo = readFirstLine(QStringLiteral("/proc/meminfo"));
    if (!memInfo.isEmpty()) {
        bool ok = false;
        const qlonglong kib = memInfo.section(' ', -2, -2).toLongLong(&ok);
        if (ok)
            memory = QStringLiteral("%1 GiB").arg(kib / (1024.0 * 1024.0), 0, 'f', 1);
    }

    QString gpu = QStringLiteral("Unknown");
    QProcess gpuProcess;
    gpuProcess.start(QStringLiteral("sh"), {
        QStringLiteral("-c"),
        QStringLiteral("nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1")
    });
    if (gpuProcess.waitForFinished(1000)) {
        const QString detected = QString::fromUtf8(gpuProcess.readAllStandardOutput()).trimmed();
        if (!detected.isEmpty())
            gpu = detected;
    }

    const QString host = QSysInfo::machineHostName();
    const QString windowManager = qEnvironmentVariable("XDG_SESSION_TYPE")
        == QStringLiteral("wayland")
        ? QStringLiteral("KWin (Wayland)")
        : QStringLiteral("KWin (X11)");

    QString desktop = QStringLiteral("KDE Plasma");
    const QString plasmaVersion = runCommand(
        QStringLiteral("plasmashell"), {QStringLiteral("--version")});
    if (!plasmaVersion.isEmpty()) {
        const QString version = plasmaVersion.section(' ', -1).trimmed();
        if (!version.isEmpty())
            desktop += QStringLiteral(" %1").arg(version);
    } else {
        const QString sessionVersion = qEnvironmentVariable("KDE_SESSION_VERSION");
        if (!sessionVersion.isEmpty())
            desktop += QStringLiteral(" %1").arg(sessionVersion);
    }

    const QString detectedLocalIp = runCommand(
        QStringLiteral("sh"),
        {QStringLiteral("-c"),
         QStringLiteral("hostname -I 2>/dev/null | awk '{print $1}'")});
    const QString localIp = detectedLocalIp.isEmpty()
        ? QStringLiteral("Unknown")
        : detectedLocalIp;

    const QStringList fields{
        QStringLiteral("OS: %1").arg(os),
        QStringLiteral("HOST: %1").arg(host),
        QStringLiteral("KERNEL: %1").arg(QSysInfo::kernelVersion()),
        QStringLiteral("DE: %1").arg(desktop),
        QStringLiteral("WM: %1").arg(windowManager),
        QStringLiteral("THEME: TpTheme"),
        QStringLiteral("CPU: %1").arg(cpu),
        QStringLiteral("GPU: %1").arg(gpu),
        QStringLiteral("MEMORY: %1").arg(memory),
        QStringLiteral("LOCAL IP: %1").arg(localIp)
    };
    const QString nextSpecs = fields.join(QStringLiteral("     "));
    if (nextSpecs != m_specs) {
        m_specs = nextSpecs;
        m_scrollX = width();
    }
}

void QuoteWidget::paintEvent(QPaintEvent *event) {
    ShapedBlurWindow::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QFont font(QStringLiteral("Monospace"), 13, QFont::DemiBold);
    const QFontMetrics metrics(font);
    const QString text = QStringLiteral("   %1   ").arg(m_specs);
    const int textWidth = metrics.horizontalAdvance(text);

    if (textWidth > 0 && m_scrollX <= -textWidth)
        m_scrollX += textWidth;

    p.save();
    p.setClipRect(QRect(18, 18, width() - 36, height() - 36));
    p.setFont(font);
    p.setPen(QColor(QStringLiteral("#c87137")));
    const qreal baseline = height() / 2.0 + metrics.ascent() / 2.0 - 2;
    for (qreal x = m_scrollX; x < width(); x += textWidth)
        p.drawText(QPointF(x, baseline), text);

    p.restore();
}

QPainterPath QuoteWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
