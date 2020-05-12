#ifndef COUNTERWIDGET_HPP_
#define COUNTERWIDGET_HPP_

#include <QWidget>
#include "LogModel.hpp"

namespace Ui {
    class CounterWidget;
}


/**
    A widget with a treeview to display and manipulate a LogModel.
*/
class CounterWidget : public QWidget {
    Ui::CounterWidget* form_;

    LogModel model_;
public:
    CounterWidget(QWidget* parent = nullptr);
    ~CounterWidget();
};



#endif /* include guard: COUNTERWIDGET_HPP_ */
