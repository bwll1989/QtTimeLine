#pragma once

#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <QSpinBox>
#include <QLabel>
#include "OSCMessage.h"
#include "TimeLineStyle.h"
#include "TimelineExports.hpp"
#include <QMimeData>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
class NODE_TIMELINE_PUBLIC BaseTimelineToolbar : public QToolBar {
    Q_OBJECT
public:
    explicit BaseTimelineToolbar(QWidget* parent = nullptr);
    ~BaseTimelineToolbar() override;

    //控件OSC地址映射
    std::shared_ptr<std::unordered_map<QString, QAction*>> _OscMapping;

    /**
     * 获取OSC地址和控件的映射
     */
     virtual std::shared_ptr<std::unordered_map<QString, QAction*>> getOscMapping() const
    {
        return _OscMapping;
    }

    /**
     * 创建动作
     */
    virtual void createActions()=0;
    /**
     * 设置UI
     */
    virtual void setupUI()=0;
    /**
 * 注册控件OSC地址和Widget指针
 */
    virtual void registerOSCControl(const QString& oscAddress, QAction* control)
    {
        // 如果oscAddress不以"/"开头，则不注册
        if (!oscAddress.startsWith("/")) return;
        // 构建完整的OSC地址，自动给OSC地址添加前缀，包括节点ID
        if (!control) return;
        // 如果已存在相同地址的映射，先移除旧的
        auto it = _OscMapping.get()->find(oscAddress);
        if (it != _OscMapping.get()->end()) {
            _OscMapping.get()->erase(it);
        }

        // 添加新的映射
         QWidget* widget = widgetForAction(control);
         if (widget) {
             widget->installEventFilter(this);
         }
         (*_OscMapping)[oscAddress] = control;
    }
    /**
     * 注销控件OSC地址和Widget指针
     */
    virtual void unregisterOSCControl(const QString& oscAddress)
    {
        if (!oscAddress.startsWith("/")) return;
        auto it = _OscMapping->find(oscAddress);
        if (it != _OscMapping->end()) {
            _OscMapping->erase(it);
        }
    }
    /**
     * 获取控件OSC地址和Widget指针
     */
    virtual QAction* getWidgetFromOSCAddress(const QString& oscAddress) const
    {
        auto it = _OscMapping->find(oscAddress);
        return it !=_OscMapping->end() ? it->second : nullptr;
    }

protected:
    void startDrag(QAction* widget);

    virtual bool eventFilter(QObject* watched, QEvent* event) override;
private:
    // 拖拽起始位置
    QPoint dragStartPosition;
    // 是否正在拖拽
    bool isDragging = false;

};

