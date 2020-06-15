#include "LogModel.hpp"
#include <QDebug>
#include <QColor>

LogModel::LogModel()
    : fivePlayers_(false), showCumSum_(false), bockLimit_(2)
{
    /*
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
    */

    recalcBockTriggers();
    recalcCumSum();

    // always recalculate the cum sum when something changes.
    // this is not efficient, but we are talking about adding some numbers...
    // nothing really heavy...
    connect(this, &QAbstractItemModel::dataChanged, this, &LogModel::recalcCumSum);
}


void LogModel::showCumSum(bool on)
{
    if (showCumSum_ != on)
    {
        showCumSum_ = on;
        auto tl = this->index(0, 0);
        auto br = this->index(log_.size()-1, Columns::ColumnCount-1);
        emit dataChanged(tl, br);
    }
}

int LogModel::cumSum(int index, int player) const
{
    return log_[index].cumSum[player];
}

int LogModel::columnCount(const QModelIndex& /*parent*/) const
{
    return Columns::ColumnCount;
}

int LogModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return log_.size()+1; // +1: one dummy entry to add more rows!
}

QModelIndex LogModel::index(int row, int col, const QModelIndex& /*parent*/) const
{
    return createIndex(row, col);
}

QModelIndex LogModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}


void LogModel::recalcCumSum()
{
    int sum[5] = {0, 0, 0, 0, 0};

    size_t index = 0;
    for (auto& entry : log_)
    {
        for (int p = 0; p < 5; p++)
        {
            sum[p] += wonValue(index, p);
            entry.cumSum[p] = sum[p];
        }
        index++;
    }

    // f*ck it...
    this->beginResetModel();
    this->endResetModel();
}


bool LogModel::fivePlayers() const
{
    return fivePlayers_;
}

void LogModel::setFivePlayers(bool on)
{
    if (on != fivePlayers_)
    {
        // use resetModel as this basically affects all bocks, everything...
        this->beginResetModel();
        fivePlayers_ = on;
        this->endResetModel();
    }

    recalcBockTriggers();
    recalcCumSum();
}

void LogModel::setPlayerName(int num, const QString& name)
{
    playerNames_[num] = name.toStdString();

    emit headerDataChanged(Qt::Vertical, 0, Columns::ColumnCount);
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
            case GameNumber: return "Round #";
            case GameValue: return "Tacken";
            case GameStartsBock: return "Bock?";
            case Player1:
            case Player2:
            case Player3:
            case Player4:
            case Player5:
            {
                int p = column - Player1;
                int sum = 0;
                if (log_.size() > 0)
                {
                    sum = log_.rbegin()->cumSum[p];
                }

                QString txt = playerName(p) + "\n[" + QString::number(sum) + "] ";
                if (playedASolo(column - Player1)) txt += QChar(0x2713); // checkmark
                else                               txt += QChar(0x2715); // x
                return txt;
            }
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
    else if (role == Qt::TextAlignmentRole)
    {
//        if (column >= Player1 && column <= Player5)
        {
            return Qt::AlignCenter;
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

bool LogModel::playedASolo(int player) const
{
    for (size_t i = 0; i < log_.size(); i++)
    {
        int p;
        if (isGameSolo(i, &p) && p == player) return true;
    }
    return false;
}


Qt::ItemFlags LogModel::flags(const QModelIndex& index) const
{
    switch (index.column())
    {
        case ColumnCount: return Qt::NoItemFlags;
        case GameNumber: return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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

bool LogModel::isGameSolo(int index, int* player) const
{
    auto& entry = log_[index];
    if (!isGameValid(index)) return false;

    int numWon = 0;
    int playerThatWon = -1;
    int playerThatLost = -1;

    for (int i = 0; i < 5; i++)
    {
        if (entry.results[i] == PlayerState::WON)
        {
            numWon++;
            playerThatWon = i;
        }
        else if (entry.results[i] == PlayerState::LOST)
        {
            playerThatLost = i;
        }
    }

    if (numWon == 1)
    {
        if (player) *player = playerThatWon;
        return true;
    }
    else if (numWon == 3)
    {
        if (player) *player = playerThatLost;
        return true;
    }

    return false;
}


QVariant LogModel::dummyData(int column, int role) const
{
    if (column == GameNumber)
    {
        if (role == Qt::DisplayRole) return QString::number(log_.size()+1);
        if (role == Qt::TextAlignmentRole) return Qt::AlignRight;
    }
    else if (column >= Player1 && column <= Player5)
    {
        int player = column - Player1;
        bool skipsNext;
        if (log_.size() > 0)
        {
            // when player with five players, the one who skips changes each
            // round. If this player must skip the next round is determined by
            // checking if the previous player skipped the last round.
            if (fivePlayers_)
            {
                int previousPlayer = (player + 4) % 5;
                if (log_.back().results[previousPlayer] == PlayerState::SKIP)
                {
                    skipsNext = true;
                }
                else
                {
                    skipsNext = false;
                }
            }
            else
            {
                // with 4 players its easy: the 5th player always skips.
                skipsNext = (column == Player5);
            }
        }
        else
        {
            // if this is the first round, the 5th player skips.
            skipsNext = (column == Player5);
        }

        if (role == Qt::BackgroundRole)
        {
            if (skipsNext) return QColor::fromHsl(0, 0, 160);
        }
    }
    else if (column == GameValue)
    {
        if (role == Qt::EditRole)
        {
            return 0;
        }
    }

    return QVariant();
}


bool LogModel::setDummyData(int column, const QVariant& value, int role)
{
    LogEntry entry;
    entry.baseValue = 0;
    entry.startsBock = false;
    for (int p = Player1; p <= Player5; p++)
    {
        // hack: only the to-skip-player gets a background role
        auto state = dummyData(p, Qt::BackgroundRole);
        if (state.isNull()) entry.results[p - Player1] = PlayerState::LOST;
        else                 entry.results[p - Player1] = PlayerState::SKIP;
    }

    beginInsertRows(QModelIndex(), log_.size(), log_.size());
    log_.push_back(entry);
    endInsertRows();

    return setData(this->index(log_.size()-1, column), value, role);
}

QVariant LogModel::data(const QModelIndex& index, int role) const
{
    // handle the dummy entry:
    if (static_cast<size_t>(index.row()) == log_.size()) return dummyData(index.column(), role);

    // just display the number of the round
    if (index.column() == GameNumber)
    {
        bool valid = isGameValid(index.row());
        if (valid)
        {
            if (role == Qt::DisplayRole)
            {
                QString txt("%1 %2");
                txt = txt.arg( isGameSolo(index.row()) ? "SOLO" : "", 4, ' ');
                txt = txt.arg(QString::number(index.row()+1), 3, ' ');
                return txt;
            }
            else if (role == Qt::TextAlignmentRole)
            {
                return Qt::AlignCenter;
            }
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
        if (role == Qt::TextAlignmentRole) return Qt::AlignCenter;
    }
    // checkbox whether the game starts a bock or not,
    // plus if we are in a bock or double-bock.
    else if (index.column() == GameStartsBock)
    {
        if (role == Qt::CheckStateRole)
        {
            if (log_[index.row()].startsBock)
            {
                if (log_[index.row()].bockTriggerSuppressed)
                    return Qt::PartiallyChecked;
                return Qt::Checked;
            }
            return Qt::Unchecked;
        }
        else if (role == Qt::DisplayRole)
        {
            return QString::fromStdString(bockState(index.row()));
        }
    }
    // display/edit won/lost/skipped rounds
    else if (index.column() >= Player1 && index.column() <= Player5)
    {
        PlayerState result = log_[index.row()].results[index.column() - Player1];
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
        else if (role == Qt::DisplayRole)
        {
            if (result == PlayerState::SKIP) return QVariant();
            if (showCumSum_)
            {
                return log_[index.row()].cumSum[index.column() - Player1];
            }
            else
            {
                int value = wonValue(index.row(), index.column() - Player1);
                QString r("%1%2");
                r = r.arg(value > 0 ? "+" : "")
                     .arg(value);
                return r;
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

void LogModel::setBockLimit(int num)
{
    bockLimit_ = num;
    recalcBockTriggers();
    recalcCumSum();
}

int LogModel::bockLimit() const
{
    return bockLimit_;
}

bool LogModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // handle dummy data first
    if (static_cast<size_t>(index.row()) == log_.size())
        return setDummyData(index.column(), value, role);

    // base game value
    if (index.column() == GameValue && role == Qt::EditRole)
    {
        int val = value.toInt();
        if (val < 0) val = 0; // don't accept values less than 0!

        log_[index.row()].baseValue = val;

        // TODO: this affects a lot more! Total/cumulative sums
        emit dataChanged(index, index);
        return true;
    }
    // check state for triggers-bock
    else if (index.column() == GameStartsBock && role == Qt::CheckStateRole)
    {
        // a bit of a special edge case here:
        // if startsBock is already set, but the trigger is suppressed,
        // we display that as a "partial check" state. But when clicking a
        // partitally checked item, the check-box-delegate wants to set it to
        // Qt::Checked, but, Qt::Unchecked would be the logical new value.
        // So, when Checked is requested, but it is already checked, we toggle.
        // Sounds like a baaaaaad idea, but if it works?
        if (value == Qt::Checked && log_[index.row()].startsBock)
            log_[index.row()].startsBock = false;
        else
            log_[index.row()].startsBock = (value == Qt::Checked);

        // this changes the value of this,
        // the bock state of the following and the displayed value of the
        // following entries, as well as the cumulative sums etc...
        // Would a global "recalculate" be easier?
        //auto tl = this->index(index.row(), GameValue);
        //auto br = this->index(rowCount()-1, GameStartsBock);

        // actually, need to recalculate the suppressed bock triggers... :/
        // this emits a model reset signal:
        recalcBockTriggers();
        // oh wait, this also affects the cumsum...
        recalcCumSum();

        //emit dataChanged(tl, br);

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

void LogModel::recalcBockTriggers()
{
    // incremental: a sliding window. activeBock always hold the number of
    // not-suppressed bock-triggers in the last 4 or 5 rounds
    int activeBock = 0;
    int bockSpan = (fivePlayers_ ? 5 : 4);

    for (size_t i = 0; i < log_.size(); i++)
    {
        // decrement for expiring bock
        int expiredIndex = i - bockSpan;
        if (expiredIndex >= 0 &&
            log_[expiredIndex].startsBock &&
            !log_[expiredIndex].bockTriggerSuppressed)
        {
            activeBock--;
        }

        // increment for the round before, which was just decided for if it is
        // suppressed or not in the last iteration
        int indexLastDecided = i - 1;
        if (indexLastDecided >= 0 &&
            log_[indexLastDecided].startsBock &&
            !log_[indexLastDecided].bockTriggerSuppressed)
        {
            activeBock++;
        }

        // decide for this entry
        log_[i].bockTriggerSuppressed = (activeBock >= bockLimit_);
    }


    // ... update everything ...
    beginResetModel();
    endResetModel();
}


bool LogModel::addsBock(int above, int index) const
{
    if (above < 0) return false;
    if (above >= index) return false;

    if (fivePlayers_ && (index-above) <= 5)
        return log_[above].startsBock && !log_[above].bockTriggerSuppressed;
    if (!fivePlayers_ && (index-above) <= 4)
        return log_[above].startsBock && !log_[above].bockTriggerSuppressed;

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


int LogModel::bockLevelStartedBy(int index) const
{
    if (index < 0) return -1;

    // which is the bock-level of the *latest* bock that affects index+1?
    int latest = -1;
    for (int above = index-1; above > index-6; above--)
    {
        if (addsBock(above, index+1))
        {
            latest = bockLevelStartedBy(above);
            break;
        }
    }

    // well, we need to just add 1.
    return latest + 1;
}


bool LogModel::hasBockTrigger(int index) const
{
    return log_[index].startsBock;
}

bool LogModel::canTriggerBock(int index) const
{
    return !log_[index].bockTriggerSuppressed;
}

int LogModel::activeBockCount(int index) const
{
    int num = 0;
    for (int above = index-5; above < index; above++)
    {
        if (addsBock(above, index)) num++;
    }
    return num;
}


std::string LogModel::bockState(int index) const
{
    if (index <= 0) return "";

    bool firstBockFound = false;
    QString state;
    for (int above = index-5; above < index; above++)
    {
        auto bockStateOfAbove = bockState(above, index);
        if (!bockStateOfAbove.empty() && !firstBockFound)
        {
            // the indentation to be used here is determined by how deep
            // the earliest bock-trigger started it. The earliest bock-trigger
            // was at "above", but it started it at "above+1".
            firstBockFound = true;

            int indent = bockLevelStartedBy(above);
            state += QString(" ").repeated(indent);
        }
        state += QString::fromStdString(bockStateOfAbove);
    }

    return state.toStdString();
}


int LogModel::wonValue(int index, int player) const
{
    if (!isGameValid(index)) return 0;

    int soloPlayer = -1;
    bool solo = isGameSolo(index, &soloPlayer);

    int value = totalValue(index);
    // in a normal game, everyone wins or loses the value
    // but in a solo, the solo-player wins/loses 3x the value.
    if (solo && player == soloPlayer)
    {
        value *= 3;
    }

    if (log_[index].results[player] == PlayerState::WON) return value;
    if (log_[index].results[player] == PlayerState::LOST) return -value;
    return 0;
}

