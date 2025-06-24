#include "BaseTrackDelegate.h"

BaseTrackDelegate::BaseTrackDelegate(QObject *parent): QAbstractItemDelegate{parent}{};

void BaseTrackDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {};

QSize BaseTrackDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return option.rect.size();
};


QWidget *BaseTrackDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    QWidget* editor = new QWidget(parent);
    
    QHBoxLayout* layout = new QHBoxLayout(editor);
    layout->setContentsMargins(0,0,0,0);
    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setText(index.data(TimelineRoles::TrackNameRole).toString());
    editor->setToolTip(index.data(TimelineRoles::TrackTypeRole).toString());
    lineEdit->setAlignment(Qt::AlignCenter);
    // lineEdit->setStyleSheet("QLineEdit { background: rgba(255, 255, 255, 0); color: white; border: none; }");
    connect(lineEdit, &QLineEdit::textChanged, this, [this, index](const QString& text){
        // 使用model()获取模型,然后通过setData更新数据
        const_cast<QAbstractItemModel*>(index.model())->setData(index, text, TimelineRoles::TrackNameRole);
    });
    layout->addWidget(lineEdit);
    // 移动轨道按钮
    QLabel* moveLabel = new QLabel();
    moveLabel->setFixedSize(20,20);
    moveLabel->setPixmap(QPixmap(":/icons/icons/move.png").scaled(moveLabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    moveLabel->setAlignment(Qt::AlignCenter);
    // moveLabel->setStyleSheet("QLabel { background: rgba(255, 255, 255, 0); color: white; border: none; }");
    layout->addWidget(moveLabel);
    editor->show();
    editor->setMouseTracking(true);
    return editor;
    // return nullptr;
};


void BaseTrackDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    // if (option.state & QStyle::State_MouseOver) {
    //     qDebug()<<"updateEditorGeometry";
    //     // 鼠标悬停时，编辑器略大一些，边距更小
    //     editor->setGeometry(option.rect.adjusted(1,1,0,0));
    // } else {
    //     qDebug()<<"updateEditorGeometr12";
        // 正常状态，保持原有边距
        editor->setGeometry(option.rect.adjusted(5,1,0,-1));
    // }
    editor->clearMask();
};


void BaseTrackDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    // QLabel *lineEdit = editor->findChild<QLabel*>();
    // if (lineEdit) {
    //     QString trackType = index.data(TimelineRoles::TrackTypeRole).toString();
    //     lineEdit->setText(trackType);
    // }
    // // Remove or replace the following lie if not applicable
    // QAbstractItemDelegate::setEditorData(editor, index); // Call the correct base class method if needed
};
