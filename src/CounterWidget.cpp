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
}

CounterWidget::~CounterWidget()
{
    delete form_;
}
