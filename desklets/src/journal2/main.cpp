#include <QApplication>
#include "Journal2Widget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Journal2Widget w;
    w.show();
    return app.exec();
}
