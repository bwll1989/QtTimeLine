#include "ClipAppearancePanel.hpp"
#include "AbstractClipModel.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
bool sameRgb(const QColor& a, const QColor& b)
{
    return a.isValid() && b.isValid() && (a.rgb() & 0x00ffffff) == (b.rgb() & 0x00ffffff);
}
}

ClipAppearancePanel::ClipAppearancePanel(AbstractClipModel* owner, QWidget* parent)
    : QGroupBox(parent)
    , m_owner(owner)
{
    setTitle(tr("片段属性"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto* colorRow = new QHBoxLayout();
    colorRow->setSpacing(4);
    colorRow->addWidget(new QLabel(tr("颜色:"), this));

    m_currentColor = new QLabel(this);
    m_currentColor->setFixedSize(22, 22);
    colorRow->addWidget(m_currentColor);

    m_colors = new QButtonGroup(this);
    m_colors->setExclusive(true);
    const QList<QColor> presets = titleColorPresets();
    for (int i = 0; i < presets.size(); ++i) {
        const QColor preset = presets.at(i);
        auto* button = new QToolButton(this);
        button->setCheckable(true);
        button->setFixedSize(22, 22);
        button->setToolTip(preset.name(QColor::HexRgb));
        button->setStyleSheet(QStringLiteral(
            "QToolButton { background-color: %1; border: 1px solid #666666; border-radius: 2px; }"
            "QToolButton:checked { border: 2px solid #ffffff; }")
            .arg(preset.name(QColor::HexRgb)));
        m_colors->addButton(button, i);
        colorRow->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, preset]() {
            if (m_syncing || !m_owner)
                return;
            m_owner->setColor(preset);
        });
    }
    colorRow->addStretch(1);
    layout->addLayout(colorRow);

    auto* noteRow = new QHBoxLayout();
    noteRow->setSpacing(4);
    noteRow->addWidget(new QLabel(tr("备注名:"), this));
    m_noteEdit = new QLineEdit(this);
    m_noteEdit->setPlaceholderText(tr("留空则显示类型名"));
    noteRow->addWidget(m_noteEdit, 1);
    layout->addLayout(noteRow);

    connect(m_noteEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_syncing || !m_owner)
            return;
        m_owner->setNoteName(text);
    });
    if (m_owner) {
        connect(m_owner, &AbstractClipModel::appearanceChanged, this, [this]() {
            syncFromOwner();
        });
    }
    syncFromOwner();
}

void ClipAppearancePanel::syncFromOwner()
{
    if (!m_owner)
        return;
    m_syncing = true;

    const QColor current = m_owner->color();
    m_currentColor->setStyleSheet(QStringLiteral(
        "background-color: %1; border: 1px solid #888888; border-radius: 2px;")
        .arg(current.name(QColor::HexRgb)));

    const QList<QColor> presets = titleColorPresets();
    int matched = -1;
    if (m_owner->hasCustomColor()) {
        for (int i = 0; i < presets.size(); ++i) {
            if (sameRgb(presets.at(i), current)) {
                matched = i;
                break;
            }
        }
    }
    m_colors->setExclusive(false);
    const QList<QAbstractButton*> buttons = m_colors->buttons();
    for (QAbstractButton* button : buttons)
        button->setChecked(false);
    if (matched >= 0) {
        if (QAbstractButton* button = m_colors->button(matched))
            button->setChecked(true);
    }
    m_colors->setExclusive(true);

    {
        QSignalBlocker blocker(m_noteEdit);
        if (m_noteEdit->text() != m_owner->noteName())
            m_noteEdit->setText(m_owner->noteName());
    }
    m_syncing = false;
}
