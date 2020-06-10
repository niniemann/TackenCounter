#ifndef GAMESTATSMODEL_HPP_
#define GAMESTATSMODEL_HPP_

#include <QAbstractTableModel>
#include <QtCharts>

#include "LogModel.hpp"


/**
    This model connects to a log model and calculates some statistics on it,
    that are provided here separately.
*/
class GameStatsModel : public QAbstractTableModel {
    Q_OBJECT

    LogModel* model_;
public:
    GameStatsModel();

    enum Statistic {
        RoundsPlayed = 0,   // number of rounds played
        SoliPlayed,         // soli that were played in total
        SoliWon,            // soli that were won
        BockTrigger,        // total number of bock triggers (0-rounds / heart went through)
        BockRounds,         // number of rounds where at least one bock was active
        DoubleBockRounds,   // number of rounds where at least two bocks were active
        NumStats, // dummy -- marks one after the last stat, and thus the number of stats

        // will not be shown, only as in parenthesis after the bock trigger count
        BockTriggerSuppressed, // number of bock triggers that were discarded due to the bock limit
    };

private:
    QMap<Statistic, QString> statName_;
    QMap<Statistic, int> statValue_;

public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;

    void setLogModel(LogModel* model);

public slots:
    void recalculate();
};


#endif /* include guard: GAMESTATSMODEL_HPP_ */
