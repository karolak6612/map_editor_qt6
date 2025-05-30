#include "AutomagicSettingsDialog.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton> // Included via AutomagicSettingsDialog.h forward declarations if needed
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDebug>

AutomagicSettingsDialog::AutomagicSettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Automagic Settings"));

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Description Label
    descriptionLabel_ = new QLabel(
        tr("Automagic border placement tries to intelligently add borders around tiles based on "
           "their neighbors. It can also attempt to 'layer' carpets by placing full carpets "
           "under items and then bordering the items. These settings control its behavior."),
        this);
    descriptionLabel_->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel_);

    // Automagic Enabled CheckBox
    automagicEnabledCheckBox_ = new QCheckBox(tr("Enable Automagic Bordering"), this);
    mainLayout->addWidget(automagicEnabledCheckBox_);

    // Border Settings GroupBox
    QGroupBox* borderSettingsGroup = new QGroupBox(tr("Border Settings"), this);
    QVBoxLayout* groupBoxLayout = new QVBoxLayout(borderSettingsGroup);

    sameGroundTypeBorderCheckBox_ = new QCheckBox(tr("Border between same ground types (e.g. grass to grass)"), borderSettingsGroup);
    groupBoxLayout->addWidget(sameGroundTypeBorderCheckBox_);

    wallsRepelBordersCheckBox_ = new QCheckBox(tr("Walls 'repel' borders (don't border next to walls)"), borderSettingsGroup);
    groupBoxLayout->addWidget(wallsRepelBordersCheckBox_);

    layerCarpetsCheckBox_ = new QCheckBox(tr("Layer carpets (place full carpet under items, then border items)"), borderSettingsGroup);
    groupBoxLayout->addWidget(layerCarpetsCheckBox_);
    
    borderizeDeleteCheckBox_ = new QCheckBox(tr("Borderize on delete (apply borders when underlying tile is revealed)"), borderSettingsGroup);
    groupBoxLayout->addWidget(borderizeDeleteCheckBox_);


    // Custom Border Sub-section
    QHBoxLayout* customBorderLayout = new QHBoxLayout(); // No parent widget needed for layout itself
    customBorderCheckBox_ = new QCheckBox(tr("Use Custom Border ID:"), borderSettingsGroup);
    customBorderLayout->addWidget(customBorderCheckBox_);

    customBorderIdLabel_ = new QLabel(tr("Border ID:"), borderSettingsGroup); // Label for spinbox
    customBorderLayout->addWidget(customBorderIdLabel_);

    customBorderIdSpinBox_ = new QSpinBox(borderSettingsGroup);
    customBorderIdSpinBox_->setMinimum(0); // Or 1, depending on valid item IDs
    customBorderIdSpinBox_->setMaximum(99999); // Adjust as needed
    customBorderLayout->addWidget(customBorderIdSpinBox_);
    customBorderLayout->addStretch(); // Pushes spinbox and label to the left

    groupBoxLayout->addLayout(customBorderLayout);
    mainLayout->addWidget(borderSettingsGroup);

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    // Set initial default values (will be overwritten by external call to setSettings)
    setSettings(false, true, true, true, false, false, 1);

    // Connections
    connect(automagicEnabledCheckBox_, &QCheckBox::stateChanged, this, &AutomagicSettingsDialog::onAutomagicEnabledChanged);
    connect(customBorderCheckBox_, &QCheckBox::stateChanged, this, &AutomagicSettingsDialog::onCustomBorderEnabledChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateDependentControlStates(); // Set initial enabled states
    
    // Adjust initial size
    // resize(400, 300); // Optional: set a default size
}

AutomagicSettingsDialog::~AutomagicSettingsDialog() {
    // UI elements are children of the dialog, Qt handles their deletion.
}

bool AutomagicSettingsDialog::isAutomagicEnabled() const {
    return automagicEnabledCheckBox_->isChecked();
}

bool AutomagicSettingsDialog::isSameGroundTypeBorderEnabled() const {
    return sameGroundTypeBorderCheckBox_->isChecked();
}

bool AutomagicSettingsDialog::isWallsRepelBordersEnabled() const {
    return wallsRepelBordersCheckBox_->isChecked();
}

bool AutomagicSettingsDialog::isLayerCarpetsEnabled() const {
    return layerCarpetsCheckBox_->isChecked();
}

bool AutomagicSettingsDialog::isBorderizeDeleteEnabled() const {
    return borderizeDeleteCheckBox_->isChecked();
}

bool AutomagicSettingsDialog::isCustomBorderEnabled() const {
    return customBorderCheckBox_->isChecked();
}

int AutomagicSettingsDialog::getCustomBorderId() const {
    return customBorderIdSpinBox_->value();
}

void AutomagicSettingsDialog::setSettings(bool automagicEnabled, bool sameGround, bool wallsRepel,
                                        bool layerCarpets, bool borderizeDelete, bool customBorder,
                                        int customBorderId) {
    automagicEnabledCheckBox_->setChecked(automagicEnabled);
    sameGroundTypeBorderCheckBox_->setChecked(sameGround);
    wallsRepelBordersCheckBox_->setChecked(wallsRepel);
    layerCarpetsCheckBox_->setChecked(layerCarpets);
    borderizeDeleteCheckBox_->setChecked(borderizeDelete);
    customBorderCheckBox_->setChecked(customBorder);
    customBorderIdSpinBox_->setValue(customBorderId);

    updateDependentControlStates();
}

void AutomagicSettingsDialog::onAutomagicEnabledChanged(int /*state*/) {
    updateDependentControlStates();
}

void AutomagicSettingsDialog::onCustomBorderEnabledChanged(int /*state*/) {
    updateDependentControlStates();
}

void AutomagicSettingsDialog::updateDependentControlStates() {
    bool automagicOn = automagicEnabledCheckBox_->isChecked();

    sameGroundTypeBorderCheckBox_->setEnabled(automagicOn);
    wallsRepelBordersCheckBox_->setEnabled(automagicOn);
    layerCarpetsCheckBox_->setEnabled(automagicOn);
    borderizeDeleteCheckBox_->setEnabled(automagicOn);
    customBorderCheckBox_->setEnabled(automagicOn);

    bool customBorderSettingsOn = automagicOn && customBorderCheckBox_->isChecked();
    customBorderIdLabel_->setEnabled(customBorderSettingsOn);
    customBorderIdSpinBox_->setEnabled(customBorderSettingsOn);
}
