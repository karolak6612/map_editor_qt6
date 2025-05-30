#include "SpawnDialog.h"
#include "Spawn.h" // The data class

#include <QLineEdit>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel> // For labels in form layout
#include <QDebug>

SpawnDialog::SpawnDialog(QWidget* parent)
    : QDialog(parent), currentSpawn_(nullptr) {
    setWindowTitle(tr("Spawn Editor"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    radiusSpinBox_ = new QSpinBox(this);
    radiusSpinBox_->setMinimum(0); // 0 for no radius (single tile) or 1 as minimum practical radius
    radiusSpinBox_->setMaximum(50); // Example maximum
    radiusSpinBox_->setToolTip(tr("Radius of the spawn area in tiles."));
    formLayout->addRow(tr("Radius:"), radiusSpinBox_);

    intervalSpinBox_ = new QSpinBox(this);
    intervalSpinBox_->setMinimum(1000); // Minimum 1 second
    intervalSpinBox_->setMaximum(3600000); // Max 1 hour, for example
    intervalSpinBox_->setSuffix(tr(" ms"));
    intervalSpinBox_->setToolTip(tr("Interval between spawn attempts in milliseconds."));
    formLayout->addRow(tr("Interval (ms):"), intervalSpinBox_);

    maxCreaturesSpinBox_ = new QSpinBox(this);
    maxCreaturesSpinBox_->setMinimum(1);
    maxCreaturesSpinBox_->setMaximum(100); // Example maximum
    maxCreaturesSpinBox_->setToolTip(tr("Maximum number of creatures this spawn can have active at once."));
    formLayout->addRow(tr("Max Creatures:"), maxCreaturesSpinBox_);

    mainLayout->addLayout(formLayout);

    // Creature List Management
    QLabel* creaturesLabel = new QLabel(tr("Creatures to Spawn:"), this);
    mainLayout->addWidget(creaturesLabel);

    creatureListWidget_ = new QListWidget(this);
    creatureListWidget_->setToolTip(tr("List of creature names that can be spawned."));
    mainLayout->addWidget(creatureListWidget_);

    QHBoxLayout* creatureAddRemoveLayout = new QHBoxLayout();
    addCreatureLineEdit_ = new QLineEdit(this);
    addCreatureLineEdit_->setPlaceholderText(tr("Enter creature name"));
    creatureAddRemoveLayout->addWidget(addCreatureLineEdit_);

    addCreatureButton_ = new QPushButton(tr("Add"), this);
    addCreatureButton_->setToolTip(tr("Add the creature name to the list."));
    creatureAddRemoveLayout->addWidget(addCreatureButton_);

    removeCreatureButton_ = new QPushButton(tr("Remove"), this);
    removeCreatureButton_->setToolTip(tr("Remove the selected creature name from the list."));
    creatureAddRemoveLayout->addWidget(removeCreatureButton_);
    mainLayout->addLayout(creatureAddRemoveLayout);

    // Dialog Buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox_);

    // Connections
    connect(addCreatureButton_, &QPushButton::clicked, this, &SpawnDialog::onAddCreatureClicked);
    connect(removeCreatureButton_, &QPushButton::clicked, this, &SpawnDialog::onRemoveCreatureClicked);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Set a reasonable default size
    // resize(400, 350);
}

SpawnDialog::SpawnDialog(Spawn* spawnToEdit, QWidget* parent)
    : SpawnDialog(parent) { // Call default constructor
    setSpawnData(spawnToEdit);
}

SpawnDialog::~SpawnDialog() {
    // Qt handles child widget deletion
}

void SpawnDialog::setSpawnData(Spawn* spawn) {
    currentSpawn_ = spawn;
    if (!currentSpawn_) {
        qWarning() << "SpawnDialog::setSpawnData called with nullptr.";
        // Optionally clear fields or set to defaults
        radiusSpinBox_->setValue(0);
        intervalSpinBox_->setValue(10000);
        maxCreaturesSpinBox_->setValue(1);
        creatureListWidget_->clear();
        return;
    }

    radiusSpinBox_->setValue(currentSpawn_->radius());
    intervalSpinBox_->setValue(currentSpawn_->interval());
    maxCreaturesSpinBox_->setValue(currentSpawn_->maxCreatures());

    creatureListWidget_->clear();
    creatureListWidget_->addItems(currentSpawn_->creatureNames());
}

void SpawnDialog::applyToSpawn(Spawn* spawn) const {
    if (!spawn) {
        qWarning() << "SpawnDialog::applyToSpawn called with nullptr spawn target.";
        return;
    }

    spawn->setRadius(radiusSpinBox_->value());
    spawn->setInterval(intervalSpinBox_->value());
    spawn->setMaxCreatures(maxCreaturesSpinBox_->value());

    QStringList names;
    for (int i = 0; i < creatureListWidget_->count(); ++i) {
        QListWidgetItem* item = creatureListWidget_->item(i);
        if (item) {
            names << item->text();
        }
    }
    spawn->setCreatureNames(names);
    // Note: Spawn position is typically not edited here as it's part of its placement on the map.
}

void SpawnDialog::onAddCreatureClicked() {
    QString creatureName = addCreatureLineEdit_->text().trimmed();
    if (!creatureName.isEmpty()) {
        // Check if item already exists to prevent duplicates
        bool found = false;
        for (int i = 0; i < creatureListWidget_->count(); ++i) {
            if (creatureListWidget_->item(i)->text() == creatureName) {
                found = true;
                break;
            }
        }
        if (!found) {
            creatureListWidget_->addItem(creatureName);
        }
        addCreatureLineEdit_->clear();
    }
    addCreatureLineEdit_->setFocus();
}

void SpawnDialog::onRemoveCreatureClicked() {
    QList<QListWidgetItem*> selectedItems = creatureListWidget_->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        delete creatureListWidget_->takeItem(creatureListWidget_->row(item));
    }
}
