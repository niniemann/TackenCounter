#include "LogModel.hpp"
#include <QDebug>

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

    return QVariant();
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
             return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    return QAbstractItemModel::flags(index);
}

QVariant LogModel::data(const QModelIndex& index, int role) const
{
    if (index.column() == GameNumber)
    {
        if (role == Qt::DisplayRole) return index.row() + 1;
    }
    else if (index.column() == GameValue)
    {
        if (role == Qt::DisplayRole) return totalValue(index.row());
        if (role == Qt::EditRole) return baseValue(index.row());
    }
    else if (index.column() == GameStartsBock)
    {
        if (role == Qt::CheckStateRole)
            return (log_[index.row()].startsBock ? Qt::Checked : Qt::Unchecked);
        else if (role == Qt::DisplayRole)
        {
            return QString::fromStdString(bockState(index.row()));
        }
    }

    return QVariant();
}


bool LogModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column() == GameValue && role == Qt::EditRole)
    {
        log_[index.row()].baseValue = value.toInt();

        // TODO: this affects a lot more! Total/cumulative sums
        emit dataChanged(index, index);
        return true;
    }
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

    return false;
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
    QString state;
    for (int above = index-5; above < index; above++)
    {
        state += QString::fromStdString(bockState(above, index));
    }
    return state.toStdString();
}
