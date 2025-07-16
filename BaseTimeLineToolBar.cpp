#include "BaseTimeLineToolBar.h"
#include "TimeLineStyle.h"
#include <QStyle>
#include <QApplication>

BaseTimelineToolbar::BaseTimelineToolbar(QWidget* parent)
    : QToolBar(parent),
    _OscMapping(std::make_shared<std::unordered_map<QString, QAction*>>())
{
    createActions();
    setupUI();
}

BaseTimelineToolbar::~BaseTimelineToolbar()
{
    delete m_playAction;
    delete m_stopAction;
    delete m_loopAction;
    delete m_nextFrameAction;
}

void BaseTimelineToolbar::createActions()
{
    // 创建播放动作
    m_playAction = new QAction(this);
    m_playAction->setIcon(QIcon(":/icons/icons/play.png"));
    m_playAction->setToolTip(tr("Play"));
    m_playAction->setShortcut(QKeySequence(Qt::Key_Space));
    connect(m_playAction, &QAction::triggered, [this]() {
        m_isPlaying = !m_isPlaying;
        m_playAction->setIcon(QIcon(m_isPlaying ? ":/icons/icons/pause.png" : ":/icons/icons/play.png"));
        m_playAction->setToolTip(m_isPlaying ? tr("Pause") : tr("Play"));
        if (m_isPlaying) {
            emit playClicked();
        } else {
            emit pauseClicked();
        }
    });

    // 创建停止动作
    m_stopAction = new QAction(this);
    m_stopAction->setIcon(QIcon(":/icons/icons/stop.png"));
    m_stopAction->setToolTip(tr("Stop"));
    connect(m_stopAction, &QAction::triggered, [this]() {
        if (m_isPlaying) {
            m_isPlaying = false;
            m_playAction->setIcon(QIcon(":/icons/icons/play.png"));
            m_playAction->setToolTip(tr("Play"));
        }
        emit stopClicked();
    });

    // 创建循环动作
    m_loopAction = new QAction(this);
    m_loopAction->setIcon(QIcon(":/icons/icons/repeat.png"));
    m_loopAction->setToolTip(tr("Loop"));
    m_loopAction->setCheckable(true);
    connect(m_loopAction, &QAction::toggled, this, &BaseTimelineToolbar::loopToggled);

    m_nextFrameAction = new QAction(this);
    m_nextFrameAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));
    m_nextFrameAction->setIcon(QIcon(":/icons/icons/rewind-forward.png"));
    m_nextFrameAction->setToolTip(tr("Next Frame"));
    connect(m_nextFrameAction, &QAction::triggered, this, &BaseTimelineToolbar::nextFrameClicked);

    m_previousFrameAction = new QAction(this);
    m_previousFrameAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
    m_previousFrameAction->setIcon(QIcon(":/icons/icons/rewind-back.png"));
    m_previousFrameAction->setToolTip(tr("Previous Frame"));
    connect(m_previousFrameAction, &QAction::triggered, this, &BaseTimelineToolbar::prevFrameClicked);

    m_nextMediaAction = new QAction(this);
    m_nextMediaAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right));
    m_nextMediaAction->setIcon(QIcon(":/icons/icons/play-next.png"));
    m_nextMediaAction->setToolTip(tr("Next Media"));
    // connect(m_nextMediaAction, &QAction::triggered, this, &BaseTimelineToolbar::nextMediaClicked);

    m_previousMediaAction = new QAction(this);
    m_previousMediaAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left));
    m_previousMediaAction->setIcon(QIcon(":/icons/icons/play-previous.png"));
    m_previousMediaAction->setToolTip(tr("Previous Media"));
    // connect(m_previousMediaAction, &QAction::triggered, this, &BaseTimelineToolbar::previousMediaClicked);

    m_fullscreenAction = new QAction(this);
    m_fullscreenAction->setIcon(QIcon(":/icons/icons/fullscreen-enter.png"));
    m_fullscreenAction->setToolTip(tr("Fullscreen"));
    // connect(m_fullscreenAction, &QAction::triggered, this, &BaseTimelineToolbar::fullscreenClicked);
    
    m_settingsAction = new QAction(this);
    m_settingsAction->setIcon(QIcon(":/icons/icons/settings.png"));
    m_settingsAction->setToolTip(tr("Settings"));
    connect(m_settingsAction, &QAction::triggered, this, [this]() {
        emit settingsClicked();
    });
    
    m_outputAction = new QAction(this);
    m_outputAction->setIcon(QIcon(":/icons/icons/views.png"));
    m_outputAction->setToolTip(tr("Output Window"));
    m_outputAction->setCheckable(true);
    connect(m_outputAction, &QAction::toggled, this, &BaseTimelineToolbar::outputWindowToggled);
    
    m_moveClipLeftAction = new QAction(this);
//    m_moveClipLeftAction->setShortcut(QKeySequence(Qt::Key_Left));
    m_moveClipLeftAction->setIcon(QIcon(":/icons/icons/move-left.png"));
    m_moveClipLeftAction->setToolTip(tr("Move Clip -1"));
    connect(m_moveClipLeftAction, &QAction::triggered, this,[this]() {
       emit moveClipClicked(-1);
    });    

    m_moveClipRightAction = new QAction(this);
//    m_moveClipRightAction->setShortcut(QKeySequence(Qt::Key_Right));
    m_moveClipRightAction->setIcon(QIcon(":/icons/icons/move-right.png"));
    m_moveClipRightAction->setToolTip(tr("Move Clip +1"));
    connect(m_moveClipRightAction, &QAction::triggered, this,[this]() {
       emit moveClipClicked(1);
    });
    // 删除
    m_deleteClipAction = new QAction(this);
//    m_deleteClipAction->setShortcut(QKeySequence(Qt::Key_Delete));
    m_deleteClipAction->setIcon(QIcon(":/icons/icons/delete-clip.png"));
    m_deleteClipAction->setToolTip(tr("Delete Clip"));
    connect(m_deleteClipAction, &QAction::triggered, this, &BaseTimelineToolbar::deleteClipClicked);
    // 创建缩放动作
    m_zoomInAction = new QAction(this);
    m_zoomInAction->setIcon(QIcon(":/icons/icons/zoomin.png"));
    m_zoomInAction->setToolTip(tr("Zoom In"));
    connect(m_zoomInAction, &QAction::triggered, this, &BaseTimelineToolbar::zoomInClicked);

    m_zoomOutAction = new QAction(this);
    m_zoomOutAction->setIcon(QIcon(":/icons/icons/zoomout.png"));
    m_zoomOutAction->setToolTip(tr("Zoom Out"));
    connect(m_zoomOutAction, &QAction::triggered, this, &BaseTimelineToolbar::zoomOutClicked);
}

void BaseTimelineToolbar::setupUI()
{
    // 添加动作到工具栏
    addAction(m_previousMediaAction);

    registerOSCControl("/previousMedia",m_previousMediaAction);
    addAction(m_previousFrameAction);
    registerOSCControl("/previousFrame",m_previousFrameAction);
    addAction(m_playAction);
    registerOSCControl("/play",m_playAction);
    addAction(m_stopAction);
    registerOSCControl("/stop",m_stopAction);
    addAction(m_nextFrameAction);
    registerOSCControl("/nextFrame",m_nextFrameAction);
    addAction(m_nextMediaAction);
    registerOSCControl("/nextMedia",m_nextMediaAction);
    addAction(m_fullscreenAction);
    registerOSCControl("/fullscreen",m_fullscreenAction);
    addAction(m_settingsAction);
    registerOSCControl("/settings",m_settingsAction);
    addAction(m_loopAction);
    registerOSCControl("/loop",m_loopAction);
    addAction(m_outputAction);
    registerOSCControl("/output",m_outputAction);
    addSeparator();
    addAction(m_moveClipLeftAction);
    registerOSCControl("/moveClipLeft",m_moveClipLeftAction);
    addAction(m_moveClipRightAction);
    registerOSCControl("/moveClipRight",m_moveClipRightAction);
    addAction(m_deleteClipAction);
    registerOSCControl("/deleteClip",m_deleteClipAction);
    addAction(m_zoomInAction);
    addAction(m_zoomOutAction);
    // 设置工具栏样式
    setMovable(false);
    setIconSize(QSize(toolbarButtonWidth, toolbarButtonWidth));
} 

void BaseTimelineToolbar::setPlaybackState(bool isPlaying)
{
    m_isPlaying = isPlaying;
    m_playAction->setIcon(QIcon(m_isPlaying ? ":/icons/icons/pause.png" : ":/icons/icons/play.png"));
    m_playAction->setToolTip(m_isPlaying ? tr("Pause") : tr("Play"));
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
    message.address = "/timeline/toolbar" + oscAddress;
    message.host = "127.0.0.1";
    message.port = 8991;

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