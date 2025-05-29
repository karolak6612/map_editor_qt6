#include "ZoneBrushPanel.h"
#include <QPushButton>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QDebug> // For placeholder event handlers

ZoneBrushPanel::ZoneBrushPanel(QWidget *parent) : QWidget(parent) {
    setupUi();

    // Connections (matching event table from wxwidgets analysis)
    connect(m_zoneButton, &QPushButton::toggled, this, &ZoneBrushPanel::onZoneButtonToggled);
    // QSpinBox has valueChanged(int) for programmatic changes and editingFinished() for when user is done.
    // wxEVT_COMMAND_SPINCTRL_UPDATED is when value changes by arrows.
    // wxEVT_COMMAND_TEXT_UPDATED is when text changes (could be intermediate).
    // Using valueChanged(int) for simplicity to catch all value changes.
    connect(m_zoneIdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ZoneBrushPanel::onZoneIdChanged);
}

ZoneBrushPanel::~ZoneBrushPanel() {
    // Qt's parent-child ownership will handle deletion of child widgets and layout.
}

void ZoneBrushPanel::setupUi() {
    m_zoneButton = new QPushButton(tr("Zone Brush")); // Placeholder text
    m_zoneButton->setCheckable(true); // Emulates wxToggleButton
    m_zoneButton->setToolTip(tr("Zone Brush")); // Tooltip from wxwidgets

    m_zoneIdSpin = new QSpinBox();
    m_zoneIdSpin->setRange(0, 65535); // Typical range for IDs
    m_zoneIdSpin->setValue(1); // Default value like in wxwidgets
    m_zoneIdSpin->setToolTip(tr("Zone ID")); // Tooltip

    QHBoxLayout* layout = new QHBoxLayout(this); // 'this' sets the layout on ZoneBrushPanel
    layout->addWidget(m_zoneButton);
    layout->addWidget(m_zoneIdSpin);
    
    // Replicating wxSizerFlags(1).Center() for zoneIdSpin:
    // The stretch factor of 1 makes it take available horizontal space relative to other widgets.
    // Alignment within its allocated space is usually handled by widget's internal properties
    // or can be influenced by QLayout::setAlignment for specific widgets.
    // For a QSpinBox, vertical centering is typical within a QHBoxLayout.
    // Add stretch to zoneIdSpin to make it expand
    layout->setStretchFactor(m_zoneIdSpin, 1); 

    // setLayout(layout); // Already done by QHBoxLayout(this)
}

void ZoneBrushPanel::onZoneButtonToggled(bool checked) {
    // Placeholder for original event handler logic
    qDebug() << "ZoneBrushPanel: zoneButton toggled:" << checked;
    // Original logic (example):
    // g_gui.ActivatePalette(GetParentPalette()); // Need to adapt how parent palette is accessed
    // g_gui.SelectBrush(g_gui.zone_brush); // Need to adapt global brush access
}

void ZoneBrushPanel::onZoneIdChanged(int value) {
    // Placeholder for original event handler logic
    qDebug() << "ZoneBrushPanel: zoneIdSpin changed:" << value;
    // Original logic (example):
    // g_gui.zone_brush->setZoneId(value); // Need to adapt global brush access
}
