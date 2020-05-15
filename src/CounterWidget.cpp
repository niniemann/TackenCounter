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

    form_->tabWidget->removeTab(form_->tabWidget->indexOf(form_->tabLog));

    connect(form_->inputName1, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName2, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName3, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName4, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName5, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->boxFivePlayers, &QCheckBox::stateChanged, this, &CounterWidget::playerCountChanged);
    connect(form_->radioCumulative, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);
    connect(form_->radioSingle, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);

    connect(form_->btnNewGame, &QPushButton::clicked, this, &CounterWidget::newGame);
}

CounterWidget::~CounterWidget()
{
    delete form_;
    if (model_) delete model_;
}


void CounterWidget::newGame()
{
    QString filename = QFileDialog::getSaveFileName(
                            this, "Game will be automatically saved to...",
                            QDir::home().filePath(
                                QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm")
                                                            .append(".doko")
                            ),
                            "DoKo save file (*.doko)");

    if (!filename.isEmpty())
    {
        qDebug() << "start new game!";
        form_->tabWidget->insertTab(1, form_->tabLog, "Log");

        if (model_)
        {
            qDebug() << "save old game";
            // first save the old one!
            {
                std::ofstream savefile(filename_.toStdString()); // <- previously configured filename!
                cereal::JSONOutputArchive ar(savefile);
                ar(*model_);
            }
        }

        // create a new model, delete the old one
        auto newModel = new LogModel();
        form_->treeView->setModel(newModel);
        delete model_;
        model_ = newModel;
        filename_ = filename;
        // set focus to the log tab
        form_->tabWidget->setCurrentWidget(form_->tabLog);
    }
}


void CounterWidget::loadGame()
{
    // save currently open game
    save();

    // TODO message box etc
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
