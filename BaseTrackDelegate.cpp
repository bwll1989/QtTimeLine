#include "BaseTrackDelegate.h"

BaseTrackDelegate::BaseTrackDelegate(QObject *parent): QAbstractItemDelegate{parent}{};

void BaseTrackDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    painter->setPen(fillColour);
    painter->setBrush(fillColour.lighter(150));

    if(option.state & QStyle::State_MouseOver)
        painter->setBrush(fillColour);
    painter->drawRect(option.rect);
    painter->save();
    if(option.state & QStyle::State_MouseOver && option.state & QStyle::State_Raised)
        painter->setPen(QPen(fillColour.darker(200),4));
    painter->drawLine(0, option.rect.top(), option.rect.width(), option.rect.top());
    painter->restore();
};

QSize BaseTrackDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return option.rect.size();
};


QWidget *BaseTrackDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    
    QWidget* editor = new QWidget(parent);
    
    QHBoxLayout* layout = new QHBoxLayout(editor);
    layout->setContentsMargins(0,0,0,0);
    // QLabel* title=new QLabel();
    // title->setText("Untitled");
    // title->setStyleSheet("QLabel { background: rgba(255, 255, 255, 0); color: white; border: none; }");
    // title->setAlignment(Qt::AlignCenter);
    // layout->setContentsMargins(5, 5, 5, 5);
    // layout->addWidget(title);

    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setText(index.data(TimelineRoles::TrackNameRole).toString());
    editor->setToolTip(index.data(TimelineRoles::TrackTypeRole).toString());
    lineEdit->setAlignment(Qt::AlignCenter);
    lineEdit->setStyleSheet("QLineEdit { background: rgba(255, 255, 255, 0); color: white; border: none; }");
    connect(lineEdit, &QLineEdit::textChanged, this, [this, index](const QString& text){
        auto* track = static_cast<BaseTrackModel*>(index.internalPointer());
        track->setName(text);
    });
    layout->addWidget(lineEdit);
    // 移动轨道按钮
    QLabel* moveLabel = new QLabel();
    moveLabel->setFixedSize(20,20);
    moveLabel->setPixmap(QPixmap(":/icons/icons/move.png").scaled(moveLabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    moveLabel->setAlignment(Qt::AlignCenter);
    moveLabel->setStyleSheet("QLabel { background: rgba(255, 255, 255, 0); color: white; border: none; }");
    layout->addWidget(moveLabel);
    editor->show();
    editor->setMouseTracking(true);
    return editor;
};


void BaseTrackDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
    QRegion mask = QRegion(0,rulerHeight,option.rect.right(),option.rect.bottom()-rulerHeight-toolbarHeight);
    editor->clearMask();
};


void BaseTrackDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    // QLabel *lineEdit = editor->findChild<QLabel*>();
    // if (lineEdit) {
    //     QString trackType = index.data(TimelineRoles::TrackTypeRole).toString();
    //     lineEdit->setText(trackType);
    // }
    // // Remove or replace the following line if not applicable
    // QAbstractItemDelegate::setEditorData(editor, index); // Call the correct base class method if needed
};
