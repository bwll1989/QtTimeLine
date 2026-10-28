#pragma once

#include "TimelineExports.hpp"
#include <QGroupBox>
#include <QColor>
#include <QList>

class AbstractClipModel;
class QButtonGroup;
class QLineEdit;
class QLabel;

inline QList<QColor> titleColorPresets()
{
    return {
        QColor(255, 140, 0),   // orange
        QColor(0, 170, 255),   // bright blue
        QColor(255, 105, 180), // pink
        QColor(160, 120, 220), // purple
        QColor(0, 180, 170),   // teal
        QColor(220, 50, 60),   // red
        QColor(240, 190, 40),  // yellow
        QColor(100, 180, 70),  // green
    };
}

class NODE_TIMELINE_PUBLIC ClipAppearancePanel : public QGroupBox {
public:
    explicit ClipAppearancePanel(AbstractClipModel* owner, QWidget* parent = nullptr);

private:
    void syncFromOwner();

    AbstractClipModel* m_owner {nullptr};
    QButtonGroup* m_colors {nullptr};
    QLineEdit* m_noteEdit {nullptr};
    QLabel* m_currentColor {nullptr};
    bool m_syncing {false};
};
