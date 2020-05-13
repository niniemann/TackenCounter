#include "PlayerStateDelegate.hpp"
#include <QComboBox>

QWidget* PlayerStateDelegate::createEditor(
        QWidget* parent, const QStyleOptionViewItem& /*option*/,
        const QModelIndex& /*index*/) const
{
    QComboBox* cb = new QComboBox(parent);
    cb->addItem("WON");
    cb->addItem("LOST");
    cb->addItem("SKIPPED");
    return cb;
}


void PlayerStateDelegate::setEditorData(
        QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(cb);

    QString currentState = index.data(Qt::EditRole).toString();
    int cbIndex = cb->findText(currentState);
    if (cbIndex >= 0) cb->setCurrentIndex(cbIndex);
}


void PlayerStateDelegate::setModelData(
        QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(cb);

    model->setData(index, cb->currentText(), Qt::EditRole);
}
