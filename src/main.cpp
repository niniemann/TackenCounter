#include <iostream>

#include "CounterWidget.hpp"

#include <QtCore>
#include <QApplication>

int main(int argc, char** args)
{
    QApplication app(argc, args);

    CounterWidget widget;
    widget.show();

    return app.exec();
}
