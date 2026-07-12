#include <QApplication>
#include "TerminalWidget.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("terminal-widget"));

    TerminalWidget w;
    w.show();

    return app.exec();
}
