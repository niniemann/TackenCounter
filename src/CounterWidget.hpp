#ifndef COUNTERWIDGET_HPP_
#define COUNTERWIDGET_HPP_

#include <QWidget>
#include <QTimer>
#include <QtCharts>

#include "LogModel.hpp"
#include "StatsModel.hpp"

namespace Ui {
    class CounterWidget;
}


/**
    A widget with a treeview to display and manipulate a LogModel.
*/
class CounterWidget : public QWidget {
    Q_OBJECT

    Ui::CounterWidget* form_;

    LogModel* model_;
    StatsModel statsModel_;

    QString filename_;

    QChart playerValueChart_;

    /**
        order to not spam the disk with saving after every single little change,
        every single little change starts/resets the timer (single-shot), and
        only when the timer event is triggert the game will be saved.
    */
    QTimer saveDelayTimer_;

    void setModel(LogModel* model, const QString& filename);
public:
    CounterWidget(QWidget* parent = nullptr);
    ~CounterWidget();


public slots:
    void playerNameChanged();
    void playerCountChanged();
    void displayCumSumChanged();

    void newGame();
    void loadGame();

    void save();
    void delayedSave();

    void updateStatistics();
    void changeFont();
};



#endif /* include guard: COUNTERWIDGET_HPP_ */
