#include <QApplication>
//#include <LayerShellQt/Shell>
#include "ClockWidget.h"

int main(int argc, char **argv) {
    // Must be called before QApplication is constructed.
    //LayerShellQt::Shell::useLayerShell();

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("clock-widget"));

    ClockWidget w;
    w.show();

    return app.exec();
}
