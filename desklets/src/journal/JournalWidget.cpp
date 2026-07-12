#include "JournalWidget.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QRegularExpression>
#include <QTimer> // Add this

JournalWidget::JournalWidget(QWidget *parent) : ShapedBlurWindow(parent) {
    resize(520, 320);

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

    // Capture output whenever it arrives
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        QString rawLog = QString::fromUtf8(m_process->readAllStandardOutput());
        m_output->setHtml(ansiToHtml(rawLog)); // Use setHtml to replace, or append if you prefer
    });

    // Setup the timer to refresh every 5000ms (5 seconds)
    QTimer *timer = new QTimer(this);

    // Define the update logic as a lambda or separate function
    auto refreshStatus = [this]() {
        if (m_process->state() == QProcess::Running) {
            m_process->kill();
            m_process->waitForFinished();
        }
        m_process->start(QStringLiteral("sh"), {
            QStringLiteral("-c"),
                         QStringLiteral("SYSTEMD_COLORS=1 systemctl status autotorrent.service --no-pager --full")
        });
    };

    connect(timer, &QTimer::timeout, this, refreshStatus);

    timer->start(5000);

    // Call it once immediately to populate the widget on startup
    refreshStatus();
}

QString JournalWidget::ansiToHtml(QString text) {
    // --- CUSTOM COLORS ---
    // Change these HEX codes to whatever you like
    QString greenColor = "#c87137"; // changed to orange
    QString redColor   = "#ff5555";
    QString yellowColor= "#ffff55";
    QString cyanColor  = "#55ffff";
    // ---------------------

    // 1. Escape HTML special characters
    text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");

    // 2. Start the HTML with a <pre> tag
    QString result = "<pre style='margin:0; font-family: monospace; white-space: pre-wrap;'>";

    // 3. Use globalMatch to find all ANSI escape codes
    QRegularExpression ansiRegex("\x1B\\[([0-9:;]+)m|\x1B\\]8;;.*?\x1B\\\\");
    QRegularExpressionMatchIterator i = ansiRegex.globalMatch(text);

    int lastPos = 0;

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        result.append(text.mid(lastPos, match.capturedStart() - lastPos));

        QString code = match.captured(1);

        if (code.isEmpty()) {
            // Strip non-color sequences
        } else if (code == "0") {
            result.append("</span>");
        } else if (code == "1;31" || code.contains("31")) {
            result.append("<span style='color:" + redColor + "; font-weight:bold;'>");
        } else if (code == "1;32" || code.contains("32")) {
            result.append("<span style='color:" + greenColor + "; font-weight:bold;'>");
        } else if (code == "1;33" || code.contains("33")) {
            result.append("<span style='color:" + yellowColor + "; font-weight:bold;'>");
        } else if (code == "1;36" || code.contains("36")) {
            result.append("<span style='color:" + cyanColor + "; font-weight:bold;'>");
        } else {
            result.append("<span style='color:#888888;'>");
        }

        lastPos = match.capturedEnd();
    }

    result.append(text.mid(lastPos));
    result.append("</pre>");

    return result;
}

QPainterPath JournalWidget::shapePath() const {
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
