#ifndef PLUGINLOADER_CPP
#define PLUGINLOADER_CPP


#include "BasePluginLoader.h"

    BasePluginLoader::BasePluginLoader(QObject *parent) : QObject(parent) {}


    void BasePluginLoader::loadPlugins() {
        QDir pluginsDir(QApplication::applicationDirPath() + "/plugins/TimeLine");
        
        if (!pluginsDir.exists()) {
            qDebug() << "Plugins directory does not exist:" << pluginsDir.path();
            emit pluginsLoaded();
            return;
        }

        foreach (QString fileName, pluginsDir.entryList(QStringList() << "*.plugin", QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            
            if (plugin) {
                ClipPlugInterface* clipPlugin = qobject_cast<ClipPlugInterface*>(plugin);
                if (clipPlugin) {
                    QString type = clipPlugin->clipType();
                    m_pluginsByType[type] = clipPlugin;
                    m_loadedPlugins.append(plugin);
                    qDebug() << "Successfully loaded plugin:" << fileName << "Type:" << type;
                    emit pluginLoaded(plugin);
                }
            }
        }
        qDebug() << "Plugins loaded";
        emit pluginsLoaded();
    }

    QList<QObject*> BasePluginLoader::getLoadedPlugins() const {
        return m_loadedPlugins;
    }

    QStringList BasePluginLoader::getAvailableTypes() const {
        return m_pluginsByType.keys();
    }

    ClipPlugInterface* BasePluginLoader::getPluginByType(const QString& type) const {
        return m_pluginsByType.value(type, nullptr);
    }

    AbstractClipModel* BasePluginLoader::createModelForType(const QString& type, int start) const {
        ClipPlugInterface* plugin = getPluginByType(type);
        if (plugin) {
            return plugin->createModel(start);
        }
        return nullptr;
    }

    // AbstractClipDelegate* BasePluginLoader::createDelegateForType(const QString& type) const {
    //     ClipPlugInterface* plugin = getPluginByType(type);
    //     if (plugin) {
    //         return plugin->createDelegate();
    //     }
    //     return nullptr;
    // }


#endif // PLUGINLOADER_HPP