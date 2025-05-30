#ifndef CLIPPLUGINTERFACE_H
#define CLIPPLUGINTERFACE_H

#include <QtPlugin>
#include "AbstractClipModel.hpp"
// #include "Widget/TimeLineWidget/TimelineAbstract/AbstractClipDelegate.hpp"

class ClipPlugInterface
{
public:
    /**
     * 析构函数
     */
    virtual ~ClipPlugInterface() {}
    /**
     * 获取类型
     * @return QString 类型
     */
    virtual QString clipType() const = 0;
    /**
     * 创建模型
     * @param int start 开始
     * @return AbstractClipModel* 模型
     */
    virtual AbstractClipModel* createModel(int start) = 0;
    /**
     * 创建代理
     * @return AbstractClipDelegate* 代理
     */
    // virtual AbstractClipDelegate* createDelegate() = 0;
};

#define ClipPlugInterface_iid "com.timeline.ClipPlugInterface"
Q_DECLARE_INTERFACE(ClipPlugInterface, ClipPlugInterface_iid)

#endif // CLIPPLUGINTERFACE_H