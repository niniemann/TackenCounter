#ifndef STATSMODEL_HPP_
#define STATSMODEL_HPP_

#include <QAbstractTableModel>
#include <QtCharts>

#include "LogModel.hpp"


/**
    This model connects to a log model and calculates some statistics on it,
    that are provided here separately.
*/
class StatsModel : public QAbstractTableModel {
    Q_OBJECT

    LogModel* model_;
public:
    StatsModel();

    enum Statistic {
        RoundsPlayed = 0,   // number of rounds played
        RoundsWon,          // number of rounds won
        RoundsWonPercentage, // won rounds percentage
        SoliPlayed,         // number of soli the player started himself
        SoliWon,            // number of soli the player started and won
        SoliBeaten,         // number of enemy soli the player has won against

        NumStats, // dummy -- marks one after the last stat, and thus the number of stats

        // stats after NumStats will not be shown
        RoundsLost         // number of rounds lost
    };

    struct PlayerStats {
        QMap<Statistic, int> stats;
        QLineSeries valueSeries;
    };

private:
    PlayerStats playerStats_[5];
    QMap<Statistic, QString> statName_;

    bool playerHasMax(int player, Statistic stat) const;
    bool playerHasMin(int player, Statistic stat) const;
public:

    QLineSeries* playerValueSeries(int player);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;


    void setLogModel(LogModel* model);

public slots:
    void recalculate();
};


#endif /* include guard: STATSMODEL_HPP_ */
