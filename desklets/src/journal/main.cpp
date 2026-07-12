#include <QApplication>
#include "JournalWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    JournalWidget w;
    w.show();
    return app.exec();
}
