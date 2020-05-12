#ifndef LOGMODEL_HPP_
#define LOGMODEL_HPP_

#include <QAbstractItemModel>
#include <vector>
#include "LogEntry.hpp"

class LogModel : public QAbstractItemModel {
    std::vector<LogEntry> log_;

    // Whether we are playing with five or four players.
    // Relevant e.g. for BOCK vs BOCKY
    bool fivePlayers_;

    std::string playerNames_[5];
public:
    enum Columns {
        GameNumber = 0,
        GameValue,
        GameStartsBock,
        Player1, Player2, Player3, Player4, Player5,
        ColumnCount // just a marker, not used
    };

    LogModel();

    QString playerName(int num) const;


    // get the base value of a game
    int baseValue(int index) const;

    // get the total value of a game, including modifiers
    int totalValue(int index) const;

    // check if the entry at above influences the entry at index through a bock(y)
    bool addsBock(int above, int index) const;

    // count the number of active bocks at a given index
    int activeBockCount(int index) const;

    // return the char that represents the bock state added by above to index
    std::string bockState(int above, int index) const;

    // returns the current bock state of the given round.
    // BOCKY
    //   BOCKY
    //   ^- "CB"
    std::string bockState(int index) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant headerData(int column, Qt::Orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
};


#endif /* include guard: LOGMODEL_HPP_ */
