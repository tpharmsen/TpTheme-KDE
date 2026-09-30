#include <QApplication>
#include "AnimationWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    AnimationWidget widget;
    widget.show();
    return app.exec();
}
