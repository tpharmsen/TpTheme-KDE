#pragma once
#include "ShapedBlurWindow.h"
#include <QProcess>
#include <QTextEdit>

class JournalWidget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit JournalWidget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;

private:
    QString ansiToHtml(QString text);
    QTextEdit *m_output;
    QProcess *m_process;
}; // <--- This semicolon and brace are CRITICAL
