#include "CounterWidget.hpp"
#include "../ui/ui_counterwidget.h"

#include "PlayerStateDelegate.hpp"

CounterWidget::CounterWidget(QWidget* parent)
    : QWidget(parent), form_(new Ui::CounterWidget)
{
    form_->setupUi(this);

    form_->treeView->setModel(&model_);

    PlayerStateDelegate* delegate = new PlayerStateDelegate(form_->treeView);
    form_->treeView->setItemDelegateForColumn(LogModel::Player1, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player2, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player3, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player4, delegate);
    form_->treeView->setItemDelegateForColumn(LogModel::Player5, delegate);

    connect(form_->inputName1, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName2, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName3, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName4, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->inputName5, &QLineEdit::textEdited, this, &CounterWidget::playerNameChanged);
    connect(form_->boxFivePlayers, &QCheckBox::stateChanged, this, &CounterWidget::playerCountChanged);
    connect(form_->radioCumulative, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);
    connect(form_->radioSingle, &QRadioButton::clicked, this, &CounterWidget::displayCumSumChanged);
}

CounterWidget::~CounterWidget()
{
    delete form_;
}

void CounterWidget::displayCumSumChanged()
{
    model_.showCumSum(form_->radioCumulative->isChecked());
}

void CounterWidget::playerNameChanged()
{
    model_.setPlayerName(0, form_->inputName1->text());
    model_.setPlayerName(1, form_->inputName2->text());
    model_.setPlayerName(2, form_->inputName3->text());
    model_.setPlayerName(3, form_->inputName4->text());
    model_.setPlayerName(4, form_->inputName5->text());
}


void CounterWidget::playerCountChanged()
{
    bool five = form_->boxFivePlayers->isChecked();
    form_->inputName5->setEnabled(five);
    if (five)
    {
        model_.setPlayerName(4, form_->inputName5->text());
    }
    else
    {
        model_.setPlayerName(4, "-");
    }

    model_.setFivePlayers(five);
}
