#include "BaseTimeLineToolBar.h"
#include "TimeLineStyle.h"
#include <QStyle>
#include <QApplication>

BaseTimelineToolbar::BaseTimelineToolbar(QWidget* parent)
    : QToolBar(parent)
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
    connect(m_settingsAction, &QAction::triggered, this, &BaseTimelineToolbar::settingsClicked);
    
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
    addAction(m_previousFrameAction);
    addAction(m_playAction);
    addAction(m_stopAction);
    addAction(m_nextFrameAction);
    addAction(m_nextMediaAction);
    addAction(m_fullscreenAction);
    addAction(m_settingsAction);
    addAction(m_loopAction);
    addAction(m_outputAction);
    addSeparator();
    addAction(m_moveClipLeftAction);
    addAction(m_moveClipRightAction);
    addAction(m_deleteClipAction);
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
