#include "TerminalWidget.h"

#include <qtermwidget.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStandardPaths>

TerminalWidget::TerminalWidget(QWidget *parent)
: ShapedBlurWindow(parent)
{
    resize(620, 360);
    m_wantsKeyboardInput = true;

    // 1. Ensure the parent container is translucent to let the blur shader render
    setAttribute(Qt::WA_TranslucentBackground);

    m_term = new QTermWidget(0, this);

    // Match the user's Konsole Breeze profile: neutral gray background,
    // 50% terminal opacity, and no colorized desktop tint.
    m_term->setTerminalOpacity(0.5);
    const QStringList schemeCandidates{
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            + QStringLiteral("/konsole/Breeze.colorscheme"),
        QStringLiteral("/usr/share/konsole/Breeze.colorscheme"),
        QStringLiteral("/usr/share/qtermwidget6/color-schemes/Breeze.colorscheme"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/Orange.colorscheme")
    };
    QString colorSchemePath;
    for (const QString &candidate : schemeCandidates) {
        if (QFile::exists(candidate)) {
            colorSchemePath = candidate;
            break;
        }
    }
    if (!colorSchemePath.isEmpty()) {
        QTermWidget::addCustomColorSchemeDir(QFileInfo(colorSchemePath).path());
        m_term->setColorScheme(colorSchemePath);
    }
    QString nerdFontFamily;
    const QStringList fontFamilies = QFontDatabase::families();
    const QStringList preferredFonts{
        QStringLiteral("JetBrainsMono Nerd Font"),
        QStringLiteral("Hack Nerd Font"),
        QStringLiteral("FiraCode Nerd Font"),
        QStringLiteral("MesloLGS NF"),
        QStringLiteral("Iosevka Nerd Font")
    };
    for (const QString &preferred : preferredFonts) {
        if (fontFamilies.contains(preferred)) {
            nerdFontFamily = preferred;
            break;
        }
    }
    if (nerdFontFamily.isEmpty()) {
        for (const QString &family : fontFamilies) {
            if (family.contains(QStringLiteral("Nerd Font"), Qt::CaseInsensitive)
                && QFontDatabase::isFixedPitch(family)) {
                nerdFontFamily = family;
                break;
            }
        }
    }

    QFont terminalFont(nerdFontFamily.isEmpty()
        ? QStringLiteral("Monospace")
        : nerdFontFamily, 10);
    terminalFont.setStyleHint(QFont::Monospace);
    m_term->setTerminalFont(terminalFont);
    m_term->setMargin(12);
    m_term->setHistorySize(2000);
    m_term->setScrollBarPosition(QTermWidgetInterface::NoScrollBar);
    m_term->setBlinkingCursor(true);

    // 3. Remove default padding/borders that often appear as "ghost" boxes
    m_term->setTerminalSizeHint(false);

    // 4. Force transparent background via Stylesheet
    // This is more robust than palette manipulation for QTermWidget
    m_term->setStyleSheet(QStringLiteral(
        "QTermWidget { background: transparent; border: none; color: #dcb088; border-radius: 24px; }"
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

void TerminalWidget::resizeEvent(QResizeEvent *event)
{
    ShapedBlurWindow::resizeEvent(event);

    if (!m_term)
        return;

    QPainterPath path;
    path.addRoundedRect(m_term->rect(), 24, 24);
    m_term->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

QPainterPath TerminalWidget::shapePath() const
{
    QPainterPath path;
    // Keep your rounded corners
    path.addRoundedRect(rect(), 24, 24);
    return path;
}
