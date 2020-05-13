#include "LogModel.hpp"
#include <QDebug>
#include <QColor>

LogModel::LogModel()
{
    LogEntry e;
    e.baseValue = 1;
    e.startsBock = false;
    e.results[0] = PlayerState::LOST;
    e.results[1] = PlayerState::LOST;
    e.results[2] = PlayerState::WON;
    e.results[3] = PlayerState::WON;
    e.results[4] = PlayerState::SKIP;

    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
    log_.push_back(e);
}


int LogModel::columnCount(const QModelIndex& /*parent*/) const
{
    return Columns::ColumnCount;
}

int LogModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return log_.size();
}

QModelIndex LogModel::index(int row, int col, const QModelIndex& /*parent*/) const
{
    return createIndex(row, col);
}

QModelIndex LogModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

QString LogModel::playerName(int num) const
{
    return QString::fromStdString(playerNames_[num]);
}

QVariant LogModel::headerData(int column, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (column)
        {
            case GameNumber: return "No.";
            case GameValue: return "Value";
            case GameStartsBock: return "B";
            case Player1: return playerName(0);
            case Player2: return playerName(1);
            case Player3: return playerName(2);
            case Player4: return playerName(3);
            case Player5: return playerName(4);
            case ColumnCount: return ""; // dummy. never reached
        }
    }
    else if (role == Qt::EditRole)
    {
        switch (column)
        {
            case Player1: return playerName(0);
            case Player2: return playerName(1);
            case Player3: return playerName(2);
            case Player4: return playerName(3);
            case Player5: return playerName(4);
            default:
                          return QVariant();
        }
    }

    return QVariant();
}


bool LogModel::setHeaderData(int column, Qt::Orientation orientation,
                             const QVariant& value, int role)
{
    if (role == Qt::EditRole)
    {
        if (column >= Player1 && column <= Player5)
        {
            playerNames_[column - Player1] = value.toString().toStdString();
            emit headerDataChanged(orientation, column, column);
            return true;
        }
    }
    return false;
}




Qt::ItemFlags LogModel::flags(const QModelIndex& index) const
{
    switch (index.column())
    {
        case ColumnCount: return Qt::NoItemFlags;
        case GameNumber: return Qt::ItemIsEnabled;
        case GameValue: return Qt::ItemIsEnabled | Qt::ItemIsEditable;
        case GameStartsBock: return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        case Player1:
        case Player2:
        case Player3:
        case Player4:
        case Player5:
             return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return QAbstractItemModel::flags(index);
}

QVariant LogModel::data(const QModelIndex& index, int role) const
{
    // just display the number of the round
    if (index.column() == GameNumber)
    {
        bool valid = isGameValid(index.row());
        if (valid)
        {
            if (role == Qt::DisplayRole) return index.row() + 1;
        }
        else
        {
            if (role == Qt::DisplayRole) return "INVALID";
            if (role == Qt::BackgroundRole) return QColor(220, 50, 47);
        }
    }
    // the value (display including modifiers, edit only base)
    else if (index.column() == GameValue)
    {
        if (role == Qt::DisplayRole) return totalValue(index.row());
        if (role == Qt::EditRole) return baseValue(index.row());
    }
    // checkbox whether the game starts a bock or not,
    // plus if we are in a bock or double-bock.
    else if (index.column() == GameStartsBock)
    {
        if (role == Qt::CheckStateRole)
            return (log_[index.row()].startsBock ? Qt::Checked : Qt::Unchecked);
        else if (role == Qt::DisplayRole)
        {
            return QString::fromStdString(bockState(index.row()));
        }
    }
    // display/edit won/lost/skipped rounds
    else if (index.column() >= Player1 && index.column() <= Player5)
    {
        PlayerState result = log_[index.row()].results[index.column() - Player1];
        qDebug() << "state: " << static_cast<int>(result);
        if (role == Qt::BackgroundRole)
        {
            switch (result)
            {
                case PlayerState::WON:
                    return QColor(133, 153, 0); // green
                case PlayerState::LOST:
                    return QColor(220, 50, 47); // red
                case PlayerState::SKIP:
                    return QColor::fromHsl(0, 0, 160); // grey
            }
        }
        else if (role == Qt::EditRole)
        {
            switch (result)
            {
                case PlayerState::WON: return "WON";
                case PlayerState::LOST: return "LOST";
                case PlayerState::SKIP: return "SKIPPED";
            }
        }
    }

    return QVariant();
}


bool LogModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // base game value
    if (index.column() == GameValue && role == Qt::EditRole)
    {
        log_[index.row()].baseValue = value.toInt();

        // TODO: this affects a lot more! Total/cumulative sums
        emit dataChanged(index, index);
        return true;
    }
    // check state for triggers-bock
    else if (index.column() == GameStartsBock && role == Qt::CheckStateRole)
    {
        qDebug() << value;
        log_[index.row()].startsBock = (value == Qt::Checked);

        // this changes the value of this,
        // the bock state of the following and the displayed value of the
        // following entries, as well as the cumulative sums etc...
        // Would a global "recalculate" be easier?
        auto tl = this->index(index.row(), GameValue);
        auto br = this->index(rowCount()-1, GameStartsBock);

        qDebug() << tl << br;
        emit dataChanged(tl, br);

        return true;
    }
    // player won/lost/skipped
    else if (index.column() >= Player1 && index.column() <= Player5 && role == Qt::EditRole)
    {
        QString stateStr = value.toString();
        PlayerState newState = (value == "WON"? PlayerState::WON :
                                (value == "LOST"? PlayerState::LOST :
                                                    PlayerState::SKIP));

        log_[index.row()].results[index.column() - Player1] = newState;

        // this changes everything after this round!
        auto tl = index.sibling(index.row(), GameValue);
        auto br = index.sibling(rowCount()-1, Player5);
        emit dataChanged(tl, br);

        return true;
    }

    return false;
}


bool LogModel::isGameValid(int index) const
{
    auto& entry = log_[index];

    // number of players that lost/won/skipped this round
    int numLost = 0, numWon = 0, numSkipped = 0;
    for (int p = 0; p < 5; p++)
    {
        if (entry.results[p] == PlayerState::WON) numWon++;
        if (entry.results[p] == PlayerState::LOST) numLost++;
        if (entry.results[p] == PlayerState::SKIP) numSkipped++;
    }

    return numLost > 0 && numWon > 0 && numSkipped == 1;
}



int LogModel::activeBockCount(int index) const
{
    int c = 0;
    const int bockLength = (fivePlayers_ ? 5 : 4);
    for (int above = index-bockLength; above < index; above++)
    {
        if (above >= 0 && log_[above].startsBock) c++;
    }
    return c;
}


int LogModel::baseValue(int index) const
{
    return log_[index].baseValue;
}


int LogModel::totalValue(int index) const
{
    int multiplier = 1;
    for (int above = index-5; above < index; above++)
    {
        if (addsBock(above, index))
        {
            multiplier *= 2;
        }
    }

    return baseValue(index) * multiplier;
}


bool LogModel::addsBock(int above, int index) const
{
    if (above < 0) return false;
    if (above >= index) return false;

    if (fivePlayers_ && (index-above) <= 5) return log_[above].startsBock;
    if (!fivePlayers_ && (index-above) <= 4) return log_[above].startsBock;
    return false;
}


std::string LogModel::bockState(int above, int index) const
{
    std::string states[] = { "B", "O", "C", "K", "Y" };
    if (addsBock(above, index))
    {
        return states[index-above-1];
    }
    return "";
}


std::string LogModel::bockState(int index) const
{
    if (index <= 0) return "";

    QString state;
    for (int above = index-5; above < index; above++)
    {
        state += QString::fromStdString(bockState(above, index));
    }

    // add indentation if necessary
    // NOTE: This does not work perfectly yet... but is good enough for
    // the usual max of 2 stacking BOCKs.
    QString preState = QString::fromStdString(bockState(index-1));
    int expectedSize = preState.size();
    QString cpy = state;
    if (cpy.replace(" ", "").startsWith("Y") && cpy.endsWith("B")) expectedSize++;

    if (!state.isEmpty() && !preState.endsWith("Y"))
        state = QString(" ").repeated(expectedSize - state.length()) + state;

    return state.toStdString();
}
