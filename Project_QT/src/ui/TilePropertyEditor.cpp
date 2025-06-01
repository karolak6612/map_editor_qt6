// TilePropertyEditor.cpp - Enhanced Tile Property Editor for Task 49

#include "TilePropertyEditor.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QListWidget>
#include <QTabWidget>
#include <QDebug>
#include <QSplitter>

TilePropertyEditor::TilePropertyEditor(QWidget *parent)
    : QWidget(parent), currentTile_(nullptr) {
    setObjectName("TilePropertyEditor");
    qDebug() << "TilePropertyEditor constructor started (Task 49 Enhanced).";

    setupUI();
}

TilePropertyEditor::~TilePropertyEditor() {
    qDebug() << "TilePropertyEditor destructor called.";
}

void TilePropertyEditor::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Create tab widget for organized property display
    m_tabWidget = new QTabWidget(this);

    // Setup individual tabs
    setupBasicPropertiesGroup();
    setupFlagsGroup();
    setupItemsGroup();
    setupAdvancedGroup();

    // Add tabs to widget
    m_tabWidget->addTab(m_basicTab, tr("Basic"));
    m_tabWidget->addTab(m_flagsTab, tr("Flags"));
    m_tabWidget->addTab(m_itemsTab, tr("Items"));
    m_tabWidget->addTab(m_advancedTab, tr("Advanced"));

    mainLayout->addWidget(m_tabWidget);

    // Connect signals
    connect(m_refreshButton, &QPushButton::clicked, this, &TilePropertyEditor::onRefreshProperties);

    qDebug() << "TilePropertyEditor UI setup completed.";
}

void TilePropertyEditor::setupBasicPropertiesGroup() {
    m_basicTab = new QWidget();
    QFormLayout* formLayout = new QFormLayout(m_basicTab);
    formLayout->setContentsMargins(10, 10, 10, 10);
    formLayout->setSpacing(8);

    // Position
    m_positionLineEdit = new QLineEdit();
    m_positionLineEdit->setReadOnly(true);
    formLayout->addRow(tr("Position:"), m_positionLineEdit);

    // House ID
    m_houseIdLineEdit = new QLineEdit();
    m_houseIdLineEdit->setReadOnly(true);
    formLayout->addRow(tr("House ID:"), m_houseIdLineEdit);

    // Zone IDs
    m_zoneIdsLineEdit = new QLineEdit();
    m_zoneIdsLineEdit->setReadOnly(true);
    formLayout->addRow(tr("Zone IDs:"), m_zoneIdsLineEdit);

    // Counts
    m_itemCountLineEdit = new QLineEdit();
    m_itemCountLineEdit->setReadOnly(true);
    formLayout->addRow(tr("Item Count:"), m_itemCountLineEdit);

    m_creatureCountLineEdit = new QLineEdit();
    m_creatureCountLineEdit->setReadOnly(true);
    formLayout->addRow(tr("Creature Count:"), m_creatureCountLineEdit);

    // Memory usage
    m_memoryUsageLineEdit = new QLineEdit();
    m_memoryUsageLineEdit->setReadOnly(true);
    formLayout->addRow(tr("Memory Usage:"), m_memoryUsageLineEdit);

    // State checkboxes
    m_isEmptyCheckBox = new QCheckBox();
    m_isEmptyCheckBox->setEnabled(false);
    formLayout->addRow(tr("Is Empty:"), m_isEmptyCheckBox);

    m_isModifiedCheckBox = new QCheckBox();
    m_isModifiedCheckBox->setEnabled(false);
    formLayout->addRow(tr("Is Modified:"), m_isModifiedCheckBox);

    m_isSelectedCheckBox = new QCheckBox();
    m_isSelectedCheckBox->setEnabled(false);
    formLayout->addRow(tr("Is Selected:"), m_isSelectedCheckBox);

    m_isBlockingCheckBox = new QCheckBox();
    m_isBlockingCheckBox->setEnabled(false);
    formLayout->addRow(tr("Is Blocking:"), m_isBlockingCheckBox);
}

void TilePropertyEditor::setupFlagsGroup() {
    m_flagsTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_flagsTab);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Map Flags Group
    m_mapFlagsGroup = new QGroupBox(tr("Map Flags"));
    QVBoxLayout* mapFlagsLayout = new QVBoxLayout(m_mapFlagsGroup);

    m_pzCheckBox = new QCheckBox(tr("Protection Zone"));
    m_pzCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_pzCheckBox);

    m_noPvpCheckBox = new QCheckBox(tr("No PVP"));
    m_noPvpCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_noPvpCheckBox);

    m_noLogoutCheckBox = new QCheckBox(tr("No Logout"));
    m_noLogoutCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_noLogoutCheckBox);

    m_pvpZoneCheckBox = new QCheckBox(tr("PVP Zone"));
    m_pvpZoneCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_pvpZoneCheckBox);

    m_refreshCheckBox = new QCheckBox(tr("Refresh"));
    m_refreshCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_refreshCheckBox);

    m_zoneBrushCheckBox = new QCheckBox(tr("Zone Brush"));
    m_zoneBrushCheckBox->setEnabled(false);
    mapFlagsLayout->addWidget(m_zoneBrushCheckBox);

    mainLayout->addWidget(m_mapFlagsGroup);

    // State Flags Group
    m_stateFlagsGroup = new QGroupBox(tr("State Flags"));
    QVBoxLayout* stateFlagsLayout = new QVBoxLayout(m_stateFlagsGroup);

    m_uniqueCheckBox = new QCheckBox(tr("Has Unique Item"));
    m_uniqueCheckBox->setEnabled(false);
    stateFlagsLayout->addWidget(m_uniqueCheckBox);

    m_optionalBorderCheckBox = new QCheckBox(tr("Optional Border"));
    m_optionalBorderCheckBox->setEnabled(false);
    stateFlagsLayout->addWidget(m_optionalBorderCheckBox);

    m_hasTableCheckBox = new QCheckBox(tr("Has Table"));
    m_hasTableCheckBox->setEnabled(false);
    stateFlagsLayout->addWidget(m_hasTableCheckBox);

    m_hasCarpetCheckBox = new QCheckBox(tr("Has Carpet"));
    m_hasCarpetCheckBox->setEnabled(false);
    stateFlagsLayout->addWidget(m_hasCarpetCheckBox);

    mainLayout->addWidget(m_stateFlagsGroup);

    // Raw flag values
    QFormLayout* rawFlagsLayout = new QFormLayout();

    m_mapFlagsLineEdit = new QLineEdit();
    m_mapFlagsLineEdit->setReadOnly(true);
    rawFlagsLayout->addRow(tr("Map Flags (Hex):"), m_mapFlagsLineEdit);

    m_stateFlagsLineEdit = new QLineEdit();
    m_stateFlagsLineEdit->setReadOnly(true);
    rawFlagsLayout->addRow(tr("State Flags (Hex):"), m_stateFlagsLineEdit);

    mainLayout->addLayout(rawFlagsLayout);
    mainLayout->addStretch();
}

void TilePropertyEditor::setupItemsGroup() {
    m_itemsTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_itemsTab);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Items list
    QLabel* itemsLabel = new QLabel(tr("Items on Tile:"));
    mainLayout->addWidget(itemsLabel);

    m_itemsList = new QListWidget();
    m_itemsList->setMaximumHeight(150);
    mainLayout->addWidget(m_itemsList);

    // Special item accessors
    QFormLayout* itemsLayout = new QFormLayout();

    m_groundItemLineEdit = new QLineEdit();
    m_groundItemLineEdit->setReadOnly(true);
    itemsLayout->addRow(tr("Ground Item:"), m_groundItemLineEdit);

    m_topItemLineEdit = new QLineEdit();
    m_topItemLineEdit->setReadOnly(true);
    itemsLayout->addRow(tr("Top Item:"), m_topItemLineEdit);

    m_topSelectableItemLineEdit = new QLineEdit();
    m_topSelectableItemLineEdit->setReadOnly(true);
    itemsLayout->addRow(tr("Top Selectable:"), m_topSelectableItemLineEdit);

    m_creatureLineEdit = new QLineEdit();
    m_creatureLineEdit->setReadOnly(true);
    itemsLayout->addRow(tr("Creature:"), m_creatureLineEdit);

    m_spawnLineEdit = new QLineEdit();
    m_spawnLineEdit->setReadOnly(true);
    itemsLayout->addRow(tr("Spawn:"), m_spawnLineEdit);

    mainLayout->addLayout(itemsLayout);
    mainLayout->addStretch();
}

void TilePropertyEditor::setupAdvancedGroup() {
    m_advancedTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_advancedTab);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QFormLayout* advancedLayout = new QFormLayout();

    m_minimapColorLineEdit = new QLineEdit();
    m_minimapColorLineEdit->setReadOnly(true);
    advancedLayout->addRow(tr("Minimap Color:"), m_minimapColorLineEdit);

    mainLayout->addLayout(advancedLayout);

    // Debug info
    QLabel* debugLabel = new QLabel(tr("Debug Information:"));
    mainLayout->addWidget(debugLabel);

    m_debugInfoText = new QTextEdit();
    m_debugInfoText->setReadOnly(true);
    m_debugInfoText->setMaximumHeight(200);
    mainLayout->addWidget(m_debugInfoText);

    // Refresh button
    m_refreshButton = new QPushButton(tr("Refresh Properties"));
    mainLayout->addWidget(m_refreshButton);

    mainLayout->addStretch();
}

void TilePropertyEditor::displayTileProperties(Tile* tile) {
    currentTile_ = tile;

    if (!tile) {
        clearProperties();
        return;
    }

    // If previously disabled due to null tile, re-enable the widget
    // if (!isEnabled()) {
    //     setEnabled(true);
    // }

    qDebug() << "TilePropertyEditor: Displaying properties for tile at" << tile->mapPos();

    // Basic Properties Tab
    m_positionLineEdit->setText(QString("(%1, %2, %3)").arg(tile->x()).arg(tile->y()).arg(tile->z()));
    m_houseIdLineEdit->setText(tile->getHouseId() == 0 ? tr("None") : QString::number(tile->getHouseId()));

    // Zone IDs
    QString zoneIdsStr;
    const QVector<quint16>& zoneIds = tile->getZoneIds();
    for (int i = 0; i < zoneIds.size(); ++i) {
        zoneIdsStr += QString::number(zoneIds.at(i));
        if (i < zoneIds.size() - 1) {
            zoneIdsStr += ", ";
        }
    }
    if (zoneIdsStr.isEmpty()) {
        zoneIdsStr = tr("None");
    }
    m_zoneIdsLineEdit->setText(zoneIdsStr);

    // Counts and states
    m_itemCountLineEdit->setText(QString::number(tile->itemCount()));
    m_creatureCountLineEdit->setText(QString::number(tile->creatureCount()));
    m_memoryUsageLineEdit->setText(QString("%1 bytes").arg(tile->memsize()));

    m_isEmptyCheckBox->setChecked(tile->isEmpty());
    m_isModifiedCheckBox->setChecked(tile->isModified());
    m_isSelectedCheckBox->setChecked(tile->isSelected());
    m_isBlockingCheckBox->setChecked(tile->isBlocking());

    // Flags Tab - Map Flags
    m_pzCheckBox->setChecked(tile->isPZ());
    m_noPvpCheckBox->setChecked(tile->isNoPVP());
    m_noLogoutCheckBox->setChecked(tile->isNoLogout());
    m_pvpZoneCheckBox->setChecked(tile->isPVPZone());
    m_refreshCheckBox->setChecked(tile->hasMapFlag(Tile::TileMapFlag::Refresh));
    m_zoneBrushCheckBox->setChecked(tile->hasMapFlag(Tile::TileMapFlag::ZoneBrush));

    // Flags Tab - State Flags
    m_uniqueCheckBox->setChecked(tile->hasUniqueItem());
    m_optionalBorderCheckBox->setChecked(tile->hasStateFlag(Tile::TileStateFlag::OptionalBorder));
    m_hasTableCheckBox->setChecked(tile->hasTable());
    m_hasCarpetCheckBox->setChecked(tile->hasCarpet());

    // Raw flag values
    m_mapFlagsLineEdit->setText(QString("0x%1").arg(static_cast<quint16>(tile->getMapFlags()), 4, 16, QChar('0')));
    m_stateFlagsLineEdit->setText(QString("0x%1").arg(static_cast<quint16>(tile->getStateFlags()), 4, 16, QChar('0')));

    // Items Tab
    updateItemsList(tile);
    updateSpecialItems(tile);

    // Advanced Tab
    m_minimapColorLineEdit->setText(QString("0x%1").arg(tile->getMiniMapColor(), 2, 16, QChar('0')));
    updateDebugInfo(tile);
}

void TilePropertyEditor::clearProperties() {
    currentTile_ = nullptr;

    // Clear basic properties
    m_positionLineEdit->clear();
    m_houseIdLineEdit->clear();
    m_zoneIdsLineEdit->clear();
    m_itemCountLineEdit->clear();
    m_creatureCountLineEdit->clear();
    m_memoryUsageLineEdit->clear();

    // Clear checkboxes
    m_isEmptyCheckBox->setChecked(false);
    m_isModifiedCheckBox->setChecked(false);
    m_isSelectedCheckBox->setChecked(false);
    m_isBlockingCheckBox->setChecked(false);

    // Clear flags
    m_pzCheckBox->setChecked(false);
    m_noPvpCheckBox->setChecked(false);
    m_noLogoutCheckBox->setChecked(false);
    m_pvpZoneCheckBox->setChecked(false);
    m_refreshCheckBox->setChecked(false);
    m_zoneBrushCheckBox->setChecked(false);

    m_uniqueCheckBox->setChecked(false);
    m_optionalBorderCheckBox->setChecked(false);
    m_hasTableCheckBox->setChecked(false);
    m_hasCarpetCheckBox->setChecked(false);

    m_mapFlagsLineEdit->clear();
    m_stateFlagsLineEdit->clear();

    // Clear items
    m_itemsList->clear();
    m_groundItemLineEdit->clear();
    m_topItemLineEdit->clear();
    m_topSelectableItemLineEdit->clear();
    m_creatureLineEdit->clear();
    m_spawnLineEdit->clear();

    // Clear advanced
    m_minimapColorLineEdit->clear();
    m_debugInfoText->clear();
}

void TilePropertyEditor::updateItemsList(Tile* tile) {
    m_itemsList->clear();

    if (!tile) return;

    // Add ground item
    Item* groundItem = tile->getGroundItem();
    if (groundItem) {
        QString itemText = QString("Ground: %1 (ID: %2)")
                          .arg(groundItem->name())
                          .arg(groundItem->getServerId());
        m_itemsList->addItem(itemText);
    }

    // Add other items
    const QList<Item*>& items = tile->getItems();
    for (int i = 0; i < items.size(); ++i) {
        Item* item = items[i];
        if (item && item != groundItem) {
            QString itemText = QString("Item %1: %2 (ID: %3)")
                              .arg(i + 1)
                              .arg(item->name())
                              .arg(item->getServerId());
            if (item->getCount() > 1) {
                itemText += QString(" x%1").arg(item->getCount());
            }
            m_itemsList->addItem(itemText);
        }
    }

    // Add creatures
    const QList<Creature*>& creatures = tile->getCreatures();
    for (int i = 0; i < creatures.size(); ++i) {
        Creature* creature = creatures[i];
        if (creature) {
            QString creatureText = QString("Creature %1: %2 (ID: %3)")
                                  .arg(i + 1)
                                  .arg(creature->getName())
                                  .arg(creature->getId());
            m_itemsList->addItem(creatureText);
        }
    }

    // Add spawns
    const QList<Spawn*>& spawns = tile->getSpawns();
    for (int i = 0; i < spawns.size(); ++i) {
        Spawn* spawn = spawns[i];
        if (spawn) {
            QString spawnText = QString("Spawn %1: %2 creatures")
                               .arg(i + 1)
                               .arg(spawn->getCreatureCount());
            m_itemsList->addItem(spawnText);
        }
    }
}

void TilePropertyEditor::updateSpecialItems(Tile* tile) {
    if (!tile) {
        m_groundItemLineEdit->clear();
        m_topItemLineEdit->clear();
        m_topSelectableItemLineEdit->clear();
        m_creatureLineEdit->clear();
        m_spawnLineEdit->clear();
        return;
    }

    // Ground item
    Item* groundItem = tile->getGroundItem();
    if (groundItem) {
        m_groundItemLineEdit->setText(QString("%1 (ID: %2)")
                                     .arg(groundItem->name())
                                     .arg(groundItem->getServerId()));
    } else {
        m_groundItemLineEdit->setText(tr("None"));
    }

    // Top item
    Item* topItem = tile->getTopItem();
    if (topItem) {
        m_topItemLineEdit->setText(QString("%1 (ID: %2)")
                                  .arg(topItem->name())
                                  .arg(topItem->getServerId()));
    } else {
        m_topItemLineEdit->setText(tr("None"));
    }

    // Top selectable item
    Item* topSelectableItem = tile->getTopSelectableItem();
    if (topSelectableItem) {
        m_topSelectableItemLineEdit->setText(QString("%1 (ID: %2)")
                                            .arg(topSelectableItem->name())
                                            .arg(topSelectableItem->getServerId()));
    } else {
        m_topSelectableItemLineEdit->setText(tr("None"));
    }

    // Creature
    Creature* creature = tile->getTopCreature();
    if (creature) {
        m_creatureLineEdit->setText(QString("%1 (ID: %2)")
                                   .arg(creature->getName())
                                   .arg(creature->getId()));
    } else {
        m_creatureLineEdit->setText(tr("None"));
    }

    // Spawn
    Spawn* spawn = tile->getSpawn();
    if (spawn) {
        m_spawnLineEdit->setText(QString("Spawn with %1 creatures")
                                .arg(spawn->getCreatureCount()));
    } else {
        m_spawnLineEdit->setText(tr("None"));
    }
}

void TilePropertyEditor::updateDebugInfo(Tile* tile) {
    if (!tile) {
        m_debugInfoText->clear();
        return;
    }

    QString debugInfo;
    debugInfo += QString("=== Tile Debug Information ===\n");
    debugInfo += QString("Position: (%1, %2, %3)\n").arg(tile->x()).arg(tile->y()).arg(tile->z());
    debugInfo += QString("Memory Size: %1 bytes\n").arg(tile->memsize());
    debugInfo += QString("Hash Code: 0x%1\n").arg(reinterpret_cast<quintptr>(tile), 0, 16);
    debugInfo += QString("\n");

    debugInfo += QString("=== Flags Detail ===\n");
    debugInfo += QString("Map Flags: 0x%1\n").arg(static_cast<quint16>(tile->getMapFlags()), 4, 16, QChar('0'));
    debugInfo += QString("  - Protection Zone: %1\n").arg(tile->isPZ() ? "Yes" : "No");
    debugInfo += QString("  - No PVP: %1\n").arg(tile->isNoPVP() ? "Yes" : "No");
    debugInfo += QString("  - No Logout: %1\n").arg(tile->isNoLogout() ? "Yes" : "No");
    debugInfo += QString("  - PVP Zone: %1\n").arg(tile->isPVPZone() ? "Yes" : "No");
    debugInfo += QString("\n");

    debugInfo += QString("State Flags: 0x%1\n").arg(static_cast<quint16>(tile->getStateFlags()), 4, 16, QChar('0'));
    debugInfo += QString("  - Has Unique Item: %1\n").arg(tile->hasUniqueItem() ? "Yes" : "No");
    debugInfo += QString("  - Has Table: %1\n").arg(tile->hasTable() ? "Yes" : "No");
    debugInfo += QString("  - Has Carpet: %1\n").arg(tile->hasCarpet() ? "Yes" : "No");
    debugInfo += QString("  - Is Blocking: %1\n").arg(tile->isBlocking() ? "Yes" : "No");
    debugInfo += QString("\n");

    debugInfo += QString("=== Content Summary ===\n");
    debugInfo += QString("Total Items: %1\n").arg(tile->itemCount());
    debugInfo += QString("Total Creatures: %1\n").arg(tile->creatureCount());
    debugInfo += QString("Is Empty: %1\n").arg(tile->isEmpty() ? "Yes" : "No");
    debugInfo += QString("Is Modified: %1\n").arg(tile->isModified() ? "Yes" : "No");
    debugInfo += QString("Is Selected: %1\n").arg(tile->isSelected() ? "Yes" : "No");
    debugInfo += QString("\n");

    debugInfo += QString("=== Zone Information ===\n");
    debugInfo += QString("House ID: %1\n").arg(tile->getHouseId() == 0 ? "None" : QString::number(tile->getHouseId()));
    const QVector<quint16>& zoneIds = tile->getZoneIds();
    if (zoneIds.isEmpty()) {
        debugInfo += QString("Zone IDs: None\n");
    } else {
        debugInfo += QString("Zone IDs: ");
        for (int i = 0; i < zoneIds.size(); ++i) {
            if (i > 0) debugInfo += ", ";
            debugInfo += QString::number(zoneIds[i]);
        }
        debugInfo += "\n";
    }

    m_debugInfoText->setPlainText(debugInfo);
}

// Task 49: Placeholder slots for future editing functionality
void TilePropertyEditor::onRefreshProperties() {
    qDebug() << "TilePropertyEditor::onRefreshProperties called";

    if (currentTile_) {
        displayTileProperties(currentTile_);
        emit refreshRequested();
    }
}

void TilePropertyEditor::onTileSelectionChanged() {
    qDebug() << "TilePropertyEditor::onTileSelectionChanged called";

    // This will be connected to the map editor's tile selection signals
    // For now, just refresh the current tile if any
    if (currentTile_) {
        displayTileProperties(currentTile_);
    }
}

#include "TilePropertyEditor.moc"
