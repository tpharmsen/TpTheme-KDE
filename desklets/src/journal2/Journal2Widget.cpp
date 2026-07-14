#include "Journal2Widget.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QRegularExpression>
#include <QTimer>
#include <QCoreApplication>

Journal2Widget::Journal2Widget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(370, 200);
    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setFrameStyle(QFrame::NoFrame);
    m_output->setAttribute(Qt::WA_TranslucentBackground);
    m_output->viewport()->setAutoFillBackground(false);
    m_output->setStyleSheet(QStringLiteral("QTextEdit { background: transparent; color: #d8d8d8; font-family: monospace; font-size: 10pt; }"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->addWidget(m_output);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // Accumulate silently — no widget updates yet.
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        m_outputBuffer += QString::fromUtf8(m_process->readAllStandardOutput());
    });

    // Render exactly once, when the full output is in.
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int, QProcess::ExitStatus) {
                if (m_outputBuffer != m_output->toPlainText()) {
                    m_output->setPlainText(m_outputBuffer);
                    QScrollBar *bar = m_output->verticalScrollBar();
                    bar->setValue(bar->maximum());
                }
            });

    const QString scriptPath = QCoreApplication::applicationDirPath() + QStringLiteral("/sysbar.sh");

    auto refreshStatus = [this, scriptPath]() {
        if (m_process->state() != QProcess::NotRunning) {
            return;
        }
        m_outputBuffer.clear();
        m_process->start(QStringLiteral("sh"), { QStringLiteral("-c"), scriptPath });
    };

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, refreshStatus);
    timer->start(500);

    refreshStatus();
}

QPainterPath Journal2Widget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
