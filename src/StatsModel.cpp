#include "StatsModel.hpp"
#include <QColor>
#include <cmath>

StatsModel::StatsModel()
    : model_(nullptr)
{
    statName_[RoundsPlayed] = "Rounds played";
    statName_[RoundsWon] = "Rounds won #";
    statName_[RoundsWonPercentage] = "Rounds won %";
    statName_[RoundsLost] = "Rounds lost";
    statName_[SoliPlayed] = "Soli played";
    statName_[SoliWon] = "Soli won";
    statName_[SoliBeaten] = "Soli beaten";
}


int StatsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    if (!model_) return 0;
    return NumStats;
}

int StatsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    if (!model_) return 0;
    return (model_->fivePlayers() ? 5 : 4) + 1; // +1: Name of statistic
}

QVariant StatsModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (section > 0)
        {
            if (model_)
                return model_->playerName(section-1);
        }
    }

    return QVariant();
}


QVariant StatsModel::data(const QModelIndex& index, int role) const
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
            auto& stats = playerStats_[index.column()-1];

            if (index.row() == RoundsWonPercentage)
            {
                return QString::number(stats.stats[RoundsWonPercentage]) + "%";
            }

            return stats.stats[static_cast<Statistic>(index.row())];
        }
        else if (role == Qt::BackgroundRole)
        {
            // mark best and worst values with green/red
            QColor colBest(133, 153, 0);
            QColor colWorst(220, 50, 47);
            switch (index.row())
            {
                case RoundsWon:
                case RoundsWonPercentage:
                case SoliPlayed:
                case SoliWon:
                case SoliBeaten:
                    // all positive so far, the more the better
                {
                    if (playerHasMax(index.column()-1, static_cast<Statistic>(index.row())))
                        return colBest;
                    if (playerHasMin(index.column()-1, static_cast<Statistic>(index.row())))
                        return colWorst;
                    break;
                }
            }
        }
    }

    return QVariant();
}

bool StatsModel::playerHasMax(int player, Statistic stat) const
{
    if (player >= 4 && !model_->fivePlayers()) return false;

    int maxVal = std::numeric_limits<int>::min();

    for (int p = 0; p < (model_->fivePlayers() ? 5 : 4); p++)
    {
        if (maxVal < playerStats_[p].stats[stat])
        {
            maxVal = playerStats_[p].stats[stat];
        }
    }

    int playerVal = playerStats_[player].stats[stat];
    return playerVal == maxVal;
}


bool StatsModel::playerHasMin(int player, Statistic stat) const
{
    if (player >= 4 && !model_->fivePlayers()) return false;

    int minVal = std::numeric_limits<int>::max();

    for (int p = 0; p < (model_->fivePlayers() ? 5 : 4); p++)
    {
        if (minVal > playerStats_[p].stats[stat])
        {
            minVal = playerStats_[p].stats[stat];
        }
    }

    int playerVal = playerStats_[player].stats[stat];
    return playerVal == minVal;
}



void StatsModel::setLogModel(LogModel* model)
{
    model_ = model;
    recalculate();
}

void StatsModel::recalculate()
{
    beginResetModel();

    for (int p = 0; p < 5; p++)
    {
        auto& stats = playerStats_[p];
        stats.stats.clear();

        stats.stats[RoundsPlayed] = 0;
        stats.stats[RoundsWon] = 0;
        stats.stats[RoundsLost] = 0;
        stats.stats[SoliPlayed] = 0;
        stats.stats[SoliWon] = 0;
        stats.stats[SoliBeaten] = 0;

        stats.valueSeries.clear();
    }

    if (!model_)
    {
        endResetModel();
        return;
    }

    // iterate the LogModel and calc stats
    for (int i = 0; i < model_->rowCount()-1; i++)
    {
        if (!model_->isGameValid(i)) continue;

        // sum per player
        for (int p = 0; p < 5; p++)
        {
            auto& stats = playerStats_[p].stats;

            auto playerColumn = LogModel::Player1 + p;
            auto index = model_->index(i, playerColumn);

            auto result = index.data(Qt::EditRole);

            // count won and lost
            if (result == "WON")       stats[RoundsWon]++;
            else if (result == "LOST") stats[RoundsLost]++;

            // count played games
            if (result != "SKIPPED") stats[RoundsPlayed]++;

            // recalc percentage won
            stats[RoundsWonPercentage] = round((double) stats[RoundsWon] * 100. / (double) stats[RoundsPlayed]);

            // add data point (cumsum over time)
            auto& series = playerStats_[p].valueSeries;
            auto cumsum = model_->cumSum(i, p);
            series.append(i, cumsum);
        }

        // count soli
        int soloPlayer = -1;
        bool isSolo = model_->isGameSolo(i, &soloPlayer);
        if (isSolo)
        {
            // number soli started by the player
            playerStats_[soloPlayer].stats[SoliPlayed]++;
            // number soli won
            auto result = model_->index(i, LogModel::Player1+soloPlayer).data(Qt::EditRole);

            if (result == "WON")
            {
                // yay, a solo won!
                playerStats_[soloPlayer].stats[SoliWon]++;
            }
            else
            {
                // ouh, the other active players have beaten the solo
                for (int p = 0; p < 5; p++)
                {
                    if (p == soloPlayer) continue;

                    auto result = model_->index(i, LogModel::Player1+p).data(Qt::EditRole);
                    if (result != "SKIPPED")
                    {
                        playerStats_[p].stats[SoliBeaten]++;
                    }
                }
            }
        }
    }

    endResetModel();
}


QLineSeries* StatsModel::playerValueSeries(int player)
{
    return &(playerStats_[player].valueSeries);
}
