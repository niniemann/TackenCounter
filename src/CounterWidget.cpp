#include "CounterWidget.hpp"
#include "../ui/ui_counterwidget.h"

#include "PlayerStateDelegate.hpp"

#include <fstream>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

CounterWidget::CounterWidget(QWidget* parent)
    : QWidget(parent), form_(new Ui::CounterWidget), model_(nullptr)
{
    form_->setupUi(this);

    //form_->treeView->setModel(model_);

    PlayerStateDelegate* delegate = new PlayerStateDelegate(form_->treeView);
    form_->treeView->setItemDelegateForColumn(LogModel::Player1, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player2, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player3, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player4, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player5, delegate);

    // disable most tabs at startup
    form_->tabWidget->removeTab(form_->tabWidget->indexOf(form_->tabLog));
    form_->tabWidget->removeTab(form_->tabWidget->indexOf(form_->tabStats));

    connect(form_->inputName1, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName2, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName3, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName4, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName5, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->boxFivePlayers, &QCheckBox::stateChanged, this, &CounterWidget::playerCountChanged);
    connect(form_->radioCumulative, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);
    connect(form_->radioSingle, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);

    connect(form_->btnNewGame, &QPushButton::clicked, this, &CounterWidget::newGame);
    connect(form_->btnLoadGame, &QPushButton::clicked, this, &CounterWidget::loadGame);

    saveDelayTimer_.setSingleShot(true);
    connect(&saveDelayTimer_, &QTimer::timeout, this, &CounterWidget::save);


    // setup charts
    for (int p = 0; p < 5; p++)
    {
        auto series = statsModel_.playerValueSeries(p);
        playerValueChart_.addSeries(series);
    }

    form_->widget->setChart(&playerValueChart_);

    // recalculate the statistics whenever the current tab changed.
    connect(form_->tabWidget, &QTabWidget::currentChanged, this, &CounterWidget::updateStatistics);

    form_->treeWidget->QTreeView::setModel(&statsModel_);
}

CounterWidget::~CounterWidget()
{
    save(); // save on exit

    delete form_;
    if (model_) delete model_;
}


void CounterWidget::updateStatistics()
{
    if (!model_) return;
    model_->recalcCumSum();
    statsModel_.recalculate();

    for (int i = 0; i < 5; i++)
    {
        auto series = statsModel_.playerValueSeries(i);
        series->setName(model_->playerName(i));

        // charts are dumb. need to remove+add to update it...
        playerValueChart_.removeSeries(series);
        playerValueChart_.addSeries(series);
    }

    playerValueChart_.createDefaultAxes();
}


void CounterWidget::setModel(LogModel* model, const QString& filename)
{
    form_->treeView->setModel(model);

    if (model_)
    {
        disconnect(model_, &LogModel::dataChanged, this, &CounterWidget::delayedSave);
        disconnect(model_, &LogModel::headerDataChanged, this, &CounterWidget::delayedSave);
        save();
        delete model_;
    }

    filename_ = filename;
    setWindowTitle("TackenCounter - " + filename_);
    model_ = model;
    connect(model_, &LogModel::dataChanged, this, &CounterWidget::delayedSave);
    connect(model_, &LogModel::headerDataChanged, this, &CounterWidget::delayedSave);

    statsModel_.setLogModel(model);
    form_->tabWidget->insertTab(2, form_->tabStats, "Statistics");
}


void CounterWidget::delayedSave()
{
    // we could do with just a small 2 second delay to accumulate most bursts
    // of editing. But still, there are many things that trigger this, even when
    // the real data changed... So what the hell, just go with a 30 second
    // delay. Doesn't matter, the game is saved in the dtor of this widget,
    // -> automatically on closing the app. This is just in case of crashes.
    saveDelayTimer_.start(std::chrono::seconds(30));
}

void CounterWidget::newGame()
{
    QString filename = QFileDialog::getSaveFileName(
        this, "Game will be automatically saved to...",
        QDir::home().filePath(
            QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm")
                                        .append(".doko")
        ),
        "DoKo save file (*.doko)"
    );

    if (!filename.isEmpty())
    {
        form_->tabWidget->insertTab(1, form_->tabLog, "Log");

        auto newModel = new LogModel();
        setModel(newModel, filename);

        // set focus to the log tab
        form_->tabWidget->setCurrentWidget(form_->tabLog);
    }
}


void CounterWidget::loadGame()
{

    QString filename = QFileDialog::getOpenFileName(
            this, "Load game...",
            QDir::homePath(),
            "DoKo save file (*.doko)");

    if (!filename.isEmpty())
    {
        // save currently open game
        save();

        // load the game
        auto newModel = new LogModel();

        try {
            std::ifstream savefile(filename.toStdString());
            cereal::JSONInputArchive ar(savefile);
            ar(*newModel);

            // remember the names!
            // somethings keeps changing them on the model while I'm trying to
            // set them in the gui...
            QString names[5] =
            {
                newModel->playerName(0),
                newModel->playerName(1),
                newModel->playerName(2),
                newModel->playerName(3),
                newModel->playerName(4)
            };

            setModel(newModel, filename);
            form_->tabWidget->insertTab(1, form_->tabLog, "Log");
            form_->tabWidget->setCurrentWidget(form_->tabLog);

            form_->boxFivePlayers->setChecked(newModel->fivePlayers());

            form_->inputName1->setText(names[0]);
            form_->inputName2->setText(names[1]);
            form_->inputName3->setText(names[2]);
            form_->inputName4->setText(names[3]);
            form_->inputName5->setText(names[4]);
            playerNameChanged();

        } catch (std::exception& e) {
            QMessageBox::critical(this, "Error", "Error loading file: " + QString(e.what()));
            delete newModel;
        }
    }
}


void CounterWidget::save()
{
    if (!model_) return;

    std::ofstream savefile(filename_.toStdString());
    cereal::JSONOutputArchive ar(savefile);
    ar(*model_);
}


void CounterWidget::displayCumSumChanged()
{
    if (!model_) return;

    model_->showCumSum(form_->radioCumulative->isChecked());
}

void CounterWidget::playerNameChanged()
{
    if (!model_) return;

    model_->setPlayerName(0, form_->inputName1->text());
    model_->setPlayerName(1, form_->inputName2->text());
    model_->setPlayerName(2, form_->inputName3->text());
    model_->setPlayerName(3, form_->inputName4->text());
    model_->setPlayerName(4, form_->inputName5->text());
}


void CounterWidget::playerCountChanged()
{
    if (!model_) return;

    bool five = form_->boxFivePlayers->isChecked();
    form_->inputName5->setEnabled(five);
    if (five)
    {
        model_->setPlayerName(4, form_->inputName5->text());
    }
    else
    {
        model_->setPlayerName(4, "-");
    }

    model_->setFivePlayers(five);
}
