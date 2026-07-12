#include "TerminalWidget.h"

#include <qtermwidget.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QProcessEnvironment>
#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>

TerminalWidget::TerminalWidget(QWidget *parent)
: ShapedBlurWindow(parent)
{
    resize(560, 340);

    // 1. Ensure the parent container is translucent to let the blur shader render
    setAttribute(Qt::WA_TranslucentBackground);

    m_term = new QTermWidget(0, this);

    // 2. Configure QTermWidget for complete transparency
    // Setting opacity to 0 ensures the widget background is invisible
    m_term->setTerminalOpacity(0.0);
    m_term->setColorScheme(QStringLiteral("Linux"));

    // 3. Remove default padding/borders that often appear as "ghost" boxes
    m_term->setTerminalSizeHint(false);

    // 4. Force transparent background via Stylesheet
    // This is more robust than palette manipulation for QTermWidget
    m_term->setStyleSheet(QStringLiteral(
        "QTermWidget { background: transparent; border: none; }"
        "QScrollBar { background: transparent; width: 0px; }" // Hide scrollbar if you want a cleaner look
    ));

    // 5. Shell configuration
    const QString shell = QProcessEnvironment::systemEnvironment().value(
        QStringLiteral("SHELL"),
                                                                         QStringLiteral("/bin/bash")
    );
    m_term->setShellProgram(shell);
    m_term->startShellProgram();

    connect(m_term, &QTermWidget::finished, qApp, &QApplication::quit);

    // 6. Layout adjustments
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Flush to edges
    layout->setSpacing(0);
    layout->addWidget(m_term);
}

void TerminalWidget::paintEvent(QPaintEvent *event)
{
    // Let the base class handle the background/blur
    ShapedBlurWindow::paintEvent(event);

    // Apply the tint
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the tint layer
    painter.setBrush(QColor(36, 36, 36, 128));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 24, 24);
}

QPainterPath TerminalWidget::shapePath() const
{
    QPainterPath path;
    // Keep your rounded corners
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
