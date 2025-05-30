#ifndef VIDEOCLIPPLUGIN_HPP
#define VIDEOCLIPPLUGIN_HPP

#include <QObject>
#include "../../AbstractClipInterface.h"
#include "../../AbstractClipModel.hpp"
#include "videoclipmodel.hpp"

class VideoClipPlugin : public QObject, public ClipPlugInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.timeline.ClipPlugInterface")
    Q_INTERFACES(ClipPlugInterface)

public:
    QString clipType() const override {
        return "Video";
    }

    AbstractClipModel* createModel(int start) override {
        return new VideoClipModel(start);
    }
};

#endif // VIDEOCLIPPLUGIN_HPP