#include <iostream>

#include "CounterWidget.hpp"

#include <QtCore>
#include <QApplication>

#include <QFont>
#include <QFontDatabase>
#include <QSettings>
#include <QDebug>

#include <iostream>

int main(int argc, char** args)
{
    QApplication app(argc, args);

    // determine a good default font for the current system
    QFontDatabase fontdb;
    QFont defaultFont;
#ifdef _WIN32
    // select the system-default monospace font
    defaultFont = fontdb.systemFont(QFontDatabase::FixedFont);
    defaultFont.setPointSize(12);
#else
    defaultFont = fontdb.font("Ubuntu Mono", "Normal", 12);
#endif

    // try to read a previously stored font, using the default font as fallback
    QSettings settings("limth", "TackenCounter");
    QFont font = settings.value("font", defaultFont).value<QFont>();

    app.setFont(font);

    CounterWidget widget;
    widget.show();

    return app.exec();
}
