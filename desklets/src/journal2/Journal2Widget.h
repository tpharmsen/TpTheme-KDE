#pragma once
#include "ShapedBlurWindow.h"
#include <QProcess>
#include <QTextEdit>

class Journal2Widget : public ShapedBlurWindow {
    Q_OBJECT
public:
    explicit Journal2Widget(QWidget *parent = nullptr);

protected:
    QPainterPath shapePath() const override;

private:
    QTextEdit *m_output;
    QProcess *m_process;
    QString m_outputBuffer;
}; // <--- This semicolon and brace are CRITICAL
