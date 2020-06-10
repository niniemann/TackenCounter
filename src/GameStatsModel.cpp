#include "GameStatsModel.hpp"
#include <QColor>
#include <cmath>

GameStatsModel::GameStatsModel()
    : model_(nullptr)
{
    statName_[RoundsPlayed] = "Rounds played";
    statName_[SoliPlayed] = "Soli played";
    statName_[SoliWon] = "Soli won";
    statName_[BockTrigger] = "Bock trigger";
    statName_[BockRounds] = "Bock rounds";
    statName_[DoubleBockRounds] = "Double-Bock rounds";
}


int GameStatsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    if (!model_) return 0;
    return NumStats;
}

int GameStatsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    if (!model_) return 0;
    return 2; // name of statistic + value
}

QVariant GameStatsModel::headerData(int /*section*/, Qt::Orientation, int /*role*/) const
{
    return QVariant();
}


QVariant GameStatsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole) return Qt::AlignCenter;


    if (index.column() == 0)
    {
        if (role == Qt::DisplayRole)
        {
            return statName_[static_cast<Statistic>(index.row())];
        }
    }
    else
    {
        if (role == Qt::DisplayRole)
        {
            // special case for bock-trigger to add show only the non-suppressed
            if (index.row() == BockTrigger)
            {
                return QString("%1 (%2)")
                    .arg(statValue_[BockTrigger] - statValue_[BockTriggerSuppressed])
                    .arg(statValue_[BockTrigger]);
            }
            return statValue_[static_cast<Statistic>(index.row())];
        }
    }

    return QVariant();
}


void GameStatsModel::setLogModel(LogModel* model)
{
    model_ = model;
    recalculate();
}

void GameStatsModel::recalculate()
{
    beginResetModel();

    if (!model_)
    {
        endResetModel();
        return;
    }

    for (int stat = 0; stat < NumStats; stat++)
    {
        statValue_[static_cast<Statistic>(stat)] = 0;
    }
    statValue_[BockTriggerSuppressed] = 0;


    // iterate the LogModel and calc stats
    for (int i = 0; i < model_->rowCount()-1; i++)
    {
        if (!model_->isGameValid(i)) continue;

        //
        statValue_[RoundsPlayed] += 1;
        //
        int soloPlayer;
        if (model_->isGameSolo(i, &soloPlayer))
        {
            statValue_[SoliPlayed] += 1;
            auto result = model_->index(i, LogModel::Player1+soloPlayer).data(Qt::EditRole);
            if (result == "WON")
            {
                statValue_[SoliWon] += 1;
            }
        }
        //
        if (model_->hasBockTrigger(i))
        {
            statValue_[BockTrigger] += 1;
            if (!model_->canTriggerBock(i)) statValue_[BockTriggerSuppressed] += 1;
        }
        //
        int active = model_->activeBockCount(i);
        if (active >= 1) statValue_[BockRounds] += 1;
        if (active >= 2) statValue_[DoubleBockRounds] += 1;
    }

    endResetModel();
}

