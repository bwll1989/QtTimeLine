#include "BaseTimeLineToolBar.h"
#include "TimeLineStyle.h"
#include <QStyle>
#include <QApplication>
#include <QClipboard>

BaseTimelineToolbar::BaseTimelineToolbar(QWidget* parent)
    : QToolBar(parent),
    _OscMapping(std::make_shared<std::unordered_map<QString, QAction*>>())
{

}

BaseTimelineToolbar::~BaseTimelineToolbar()
{

}


bool BaseTimelineToolbar::eventFilter(QObject *watched, QEvent *event)

{
    QToolButton* btn = qobject_cast<QToolButton*>(watched);
    if (!btn)
        return false;

    QAction* action = btn->defaultAction();
    if (!action)
        return false;

    // 判断 action 是否在 _OscMapping 里
    auto it = std::find_if(_OscMapping->begin(), _OscMapping->end(),
        [action](const auto& pair) { return pair.second == action; });
    if (it == _OscMapping->end())
        return false;

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && (mouseEvent->modifiers() & Qt::ControlModifier)) {
                dragStartPosition = mouseEvent->pos();
                isDragging = true;
            }
            break;
    }
    case QEvent::MouseMove: {
            if (!isDragging) break;
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if ((mouseEvent->pos() - dragStartPosition).manhattanLength() >= QApplication::startDragDistance()) {
                startDrag(action);
                isDragging = false;
                return true;
            }
            break;
    }
    case QEvent::MouseButtonRelease: {
            isDragging = false;
            break;
    }
    default:
        break;
    }
    return false;
}

void BaseTimelineToolbar::startDrag(QAction* widget)
{
    // 找到对应的OSC地址
    QString oscAddress;
    for (const auto& pair : *_OscMapping) {
        if (pair.second == widget) {
            oscAddress = pair.first;
            break;
        }
    }

    if (oscAddress.isEmpty()) return;

    OSCMessage message;
    message.address = "/timeline/default/toolbar" + oscAddress;
    message.host = "127.0.0.1";
    message.port = 8991;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(message.address);
    // 获取控件的值

    message.value = widget->isChecked();
    message.type = "Int";


    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << message.host << message.port << message.address << message.value<<message.type;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-osc-address", itemData);

    QDrag* drag = new QDrag(widget);
    drag->setMimeData(mimeData);
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    QColor bgColor(40, 40, 40, 200);  // 半透明深灰色
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(pixmap.rect(), 5, 5);  // 圆角矩形
    // 绘制文本
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    QRect textRect = pixmap.rect().adjusted(30, 0, -8, 0);  // 图标右侧的文本区域
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, message.address);

    // 设置拖拽预览
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));  // 热点在中心

    drag->exec(Qt::CopyAction);
}