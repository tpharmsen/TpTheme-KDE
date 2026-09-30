#include <QApplication>
#include "QuoteWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QuoteWidget widget;
    widget.show();
    return app.exec();
}
