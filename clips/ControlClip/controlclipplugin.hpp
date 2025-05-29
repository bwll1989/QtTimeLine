#ifndef CONTROLCLIPPLUGIN_HPP
#define CONTROLCLIPPLUGIN_HPP

#include <QObject>
#include "../../AbstractClipInterface.h"
#include "../../AbstractClipModel.h"
#include "controlclipmodel.hpp"

class ControlClipPlugin : public QObject, public ClipPlugInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.timeline.ClipPlugInterface")
    Q_INTERFACES(ClipPlugInterface)

public:
    QString clipType() const override {
        return "Control";
    }

    AbstractClipModel* createModel(int start) override {
        return new ControlClipModel(start);
    }
};

#endif // ControlCLIPPLUGIN_HPP