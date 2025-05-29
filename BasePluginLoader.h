#ifndef BASEPLUGINLOADER_H
#define BASEPLUGINLOADER_H

#include <QObject>
#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QApplication>
#include "AbstractClipInterface.h"

class BasePluginLoader : public QObject
{
    Q_OBJECT
public:
    explicit BasePluginLoader(QObject *parent = nullptr) ;

    /**
     * 加载插件
     */
    virtual void loadPlugins() ;
    /**
     * 获取加载的插件列表
     * @return QList<QObject*> 加载的插件列表
     */
    virtual QList<QObject*> getLoadedPlugins() const;
    /**
     * 获取所有插件类型
     * @return QStringList 插件类型列表
     */
    virtual QStringList getAvailableTypes() const;
    /**
     * 获取指定类型的插件
     * @param const QString& type 类型
    */
    virtual ClipPlugInterface* getPluginByType(const QString& type) const;
    /**
     * 创建指定类型的模型
     * @param const QString& type 类型
     * @param int start 开始
    */      
    virtual AbstractClipModel* createModelForType(const QString& type, int start) const ;
    /**
     * 创建指定类型的代理
     * @param const QString& type 类型
    */
    // AbstractClipDelegate* createDelegateForType(const QString& type) const;

signals:
    /**
     * 插件加载完成信号
     * @param QObject* plugin 插件
    */
    void pluginLoaded(QObject* plugin);
    /**
     * 所有插件加载完成信号
     */
    void pluginsLoaded();

private:
    // 加载的插件列表
    QList<QObject*> m_loadedPlugins;
    // 插件类型列表
    QMap<QString, ClipPlugInterface*> m_pluginsByType;
};

#endif // PLUGINLOADER_H