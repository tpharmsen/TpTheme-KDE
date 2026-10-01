#include <QApplication>
#include "SystemWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    SystemWidget w;
    w.show();
    return app.exec();
}
