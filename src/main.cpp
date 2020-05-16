#include <iostream>

#include "CounterWidget.hpp"

#include <QtCore>
#include <QApplication>

#include <QFont>
#include <QFontDatabase>

#include <QDebug>

#include <iostream>

int main(int argc, char** args)
{
    QApplication app(argc, args);

    QFontDatabase fontdb;

#ifdef _WIN32
    // select the system-default monospace font
    QFont font = fontdb.systemFont(QFontDatabase::FixedFont);
    font.setPointSize(12);

    std::cout << font.family().toStdString() << ", " << font.pointSize() << std::endl;

    // set it as default for the application
    app.setFont(font);
#else
    QFont font = fontdb.font("Ubuntu Mono", "Normal", 12);
    app.setFont(font);
#endif

    CounterWidget widget;
    widget.show();

    return app.exec();
}
