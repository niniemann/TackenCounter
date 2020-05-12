#include "CounterWidget.hpp"
#include "../ui/ui_counterwidget.h"

CounterWidget::CounterWidget(QWidget* parent)
    : QWidget(parent), form_(new Ui::CounterWidget)
{
    form_->setupUi(this);

    form_->treeView->setModel(&model_);
}

CounterWidget::~CounterWidget()
{
    delete form_;
}
