#ifndef PLAYERSTATEDELEGATE_HPP_
#define PLAYERSTATEDELEGATE_HPP_

#include <QStyledItemDelegate>

/**
    Provides a combobox as an editor-widget in the treeview
    to select whether a player won/lost/skipped the round.
*/
class PlayerStateDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex &index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};


#endif /* include guard: PLAYERSTATEDELEGATE_HPP_ */
