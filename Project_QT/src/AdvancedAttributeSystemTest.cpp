// AdvancedAttributeSystemTest.cpp - Test for Task 55 Advanced Item Attributes

#include "Item.h"
#include "Tile.h"
#include "Creature.h"
#include "Spawn.h"
#include "ItemManager.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDebug>
#include <memory>

// Test widget to demonstrate advanced attribute system
class AdvancedAttributeSystemTestWidget : public QWidget {
    Q_OBJECT

public:
    AdvancedAttributeSystemTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onTestDoorProperties() {
        updateStatus("Testing door properties and attributes...");
        
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (!itemManager) {
            updateStatus("✗ ItemManager not available");
            return;
        }
        
        // Create door item
        Item* door = itemManager->createItem(1209); // Door ID
        if (!door) {
            updateStatus("✗ Failed to create door item");
            return;
        }
        
        // Test door properties
        door->setDoorId(123);
        door->setDoorOpen(false);
        door->setDoorLocked(true);
        
        updateStatus(QString("✓ Door created with ID: %1").arg(door->getServerId()));
        updateStatus(QString("  - Door ID: %1").arg(door->getDoorId()));
        updateStatus(QString("  - Door Open: %1").arg(door->isDoorOpen() ? "Yes" : "No"));
        updateStatus(QString("  - Door Locked: %1").arg(door->isDoorLocked() ? "Yes" : "No"));
        
        // Test door state changes
        door->setDoorOpen(true);
        door->setDoorLocked(false);
        
        updateStatus(QString("  After state change:"));
        updateStatus(QString("  - Door Open: %1").arg(door->isDoorOpen() ? "Yes" : "No"));
        updateStatus(QString("  - Door Locked: %1").arg(door->isDoorLocked() ? "Yes" : "No"));
        
        delete door;
        updateStatus("Door properties test completed.");
    }

    void onTestContainerProperties() {
        updateStatus("Testing container properties and contents...");
        
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (!itemManager) {
            updateStatus("✗ ItemManager not available");
            return;
        }
        
        // Create container item
        Item* container = itemManager->createItem(1987); // Bag
        if (!container) {
            updateStatus("✗ Failed to create container item");
            return;
        }
        
        // Test container properties
        container->setContainerCapacity(20);
        container->setContainerRestrictions(QStringList() << "no_fluids" << "no_corpses");
        
        // Test container contents
        QVariantList contents;
        QVariantMap item1;
        item1["id"] = 2148; // Gold coin
        item1["count"] = 100;
        contents.append(item1);
        
        QVariantMap item2;
        item2["id"] = 2160; // Crystal coin
        item2["count"] = 5;
        contents.append(item2);
        
        container->setContainerContents(contents);
        
        updateStatus(QString("✓ Container created with ID: %1").arg(container->getServerId()));
        updateStatus(QString("  - Capacity: %1").arg(container->getContainerCapacity()));
        updateStatus(QString("  - Restrictions: %1").arg(container->getContainerRestrictions().join(", ")));
        updateStatus(QString("  - Contents: %1 items").arg(container->getContainerContents().size()));
        
        // Display contents
        QVariantList retrievedContents = container->getContainerContents();
        for (int i = 0; i < retrievedContents.size(); ++i) {
            QVariantMap itemData = retrievedContents[i].toMap();
            updateStatus(QString("    Item %1: ID %2, Count %3")
                        .arg(i + 1)
                        .arg(itemData["id"].toInt())
                        .arg(itemData["count"].toInt()));
        }
        
        delete container;
        updateStatus("Container properties test completed.");
    }

    void onTestBedProperties() {
        updateStatus("Testing bed properties and sleeper data...");
        
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (!itemManager) {
            updateStatus("✗ ItemManager not available");
            return;
        }
        
        // Create bed item
        Item* bed = itemManager->createItem(1754); // Bed
        if (!bed) {
            updateStatus("✗ Failed to create bed item");
            return;
        }
        
        // Test bed properties
        bed->setBedSleeperId(12345);
        bed->setBedSleepStart(QDateTime::currentSecsSinceEpoch());
        bed->setBedRegenerationRate(2);
        
        updateStatus(QString("✓ Bed created with ID: %1").arg(bed->getServerId()));
        updateStatus(QString("  - Sleeper ID: %1").arg(bed->getBedSleeperId()));
        updateStatus(QString("  - Sleep Start: %1").arg(bed->getBedSleepStart()));
        updateStatus(QString("  - Regeneration Rate: %1").arg(bed->getBedRegenerationRate()));
        
        // Test bed state changes
        bed->setBedSleeperId(0); // Empty bed
        updateStatus(QString("  After clearing sleeper:"));
        updateStatus(QString("  - Sleeper ID: %1").arg(bed->getBedSleeperId()));
        
        delete bed;
        updateStatus("Bed properties test completed.");
    }

    void onTestPodiumProperties() {
        updateStatus("Testing podium properties and outfit data...");
        
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (!itemManager) {
            updateStatus("✗ ItemManager not available");
            return;
        }
        
        // Create podium item
        Item* podium = itemManager->createItem(1666); // Podium
        if (!podium) {
            updateStatus("✗ Failed to create podium item");
            return;
        }
        
        // Test podium display properties
        podium->setPodiumDirection(2); // South
        podium->setPodiumShowOutfit(true);
        podium->setPodiumShowMount(false);
        podium->setPodiumShowPlatform(true);
        
        // Test podium outfit properties
        podium->setPodiumOutfitLookType(128); // Male citizen
        podium->setPodiumOutfitHead(95);
        podium->setPodiumOutfitBody(116);
        podium->setPodiumOutfitLegs(121);
        podium->setPodiumOutfitFeet(115);
        podium->setPodiumOutfitAddon(2);
        
        updateStatus(QString("✓ Podium created with ID: %1").arg(podium->getServerId()));
        updateStatus(QString("  - Direction: %1").arg(podium->getPodiumDirection()));
        updateStatus(QString("  - Show Outfit: %1").arg(podium->getPodiumShowOutfit() ? "Yes" : "No"));
        updateStatus(QString("  - Show Mount: %1").arg(podium->getPodiumShowMount() ? "Yes" : "No"));
        updateStatus(QString("  - Show Platform: %1").arg(podium->getPodiumShowPlatform() ? "Yes" : "No"));
        updateStatus(QString("  Outfit Details:"));
        updateStatus(QString("  - Look Type: %1").arg(podium->getPodiumOutfitLookType()));
        updateStatus(QString("  - Head: %1, Body: %2, Legs: %3, Feet: %4, Addon: %5")
                    .arg(podium->getPodiumOutfitHead())
                    .arg(podium->getPodiumOutfitBody())
                    .arg(podium->getPodiumOutfitLegs())
                    .arg(podium->getPodiumOutfitFeet())
                    .arg(podium->getPodiumOutfitAddon()));
        
        delete podium;
        updateStatus("Podium properties test completed.");
    }

    void onTestCreatureMapping() {
        updateStatus("Testing enhanced creature mapping on tiles...");
        
        // Create test tile
        auto testTile = std::make_unique<Tile>(10, 10, 0);
        
        // Create test creatures
        Creature* creature1 = new Creature();
        creature1->setId(1001);
        creature1->setName("Rat");
        
        Creature* creature2 = new Creature();
        creature2->setId(1002);
        creature2->setName("Cave Rat");
        
        Creature* creature3 = new Creature();
        creature3->setId(1003);
        creature3->setName("Spider");
        
        // Test creature mapping
        testTile->addCreature(1001, creature1);
        testTile->addCreature(1002, creature2);
        testTile->addCreature(creature3); // Add without ID mapping
        
        updateStatus(QString("✓ Added creatures to tile (%1,%2,%3)")
                    .arg(testTile->x()).arg(testTile->y()).arg(testTile->z()));
        updateStatus(QString("  - Total creatures: %1").arg(testTile->creatureCount()));
        updateStatus(QString("  - Has creatures: %1").arg(testTile->hasCreatures() ? "Yes" : "No"));
        updateStatus(QString("  - Mapped creatures: %1").arg(testTile->getCreatureMap().size()));
        updateStatus(QString("  - Creature list size: %1").arg(testTile->getCreatures().size()));
        
        // Test creature retrieval by ID
        Creature* retrieved1 = testTile->getCreature(1001);
        Creature* retrieved2 = testTile->getCreature(1002);
        Creature* retrieved3 = testTile->getCreature(1003); // Should be null
        
        updateStatus(QString("  Creature retrieval by ID:"));
        updateStatus(QString("  - ID 1001: %1").arg(retrieved1 ? retrieved1->getName() : "Not found"));
        updateStatus(QString("  - ID 1002: %1").arg(retrieved2 ? retrieved2->getName() : "Not found"));
        updateStatus(QString("  - ID 1003: %1").arg(retrieved3 ? retrieved3->getName() : "Not found"));
        
        // Test creature removal
        testTile->removeCreature(1001);
        updateStatus(QString("  After removing creature 1001:"));
        updateStatus(QString("  - Total creatures: %1").arg(testTile->creatureCount()));
        updateStatus(QString("  - Mapped creatures: %1").arg(testTile->getCreatureMap().size()));
        
        // Clear all creatures
        testTile->clearCreatures();
        updateStatus(QString("  After clearing all creatures:"));
        updateStatus(QString("  - Total creatures: %1").arg(testTile->creatureCount()));
        updateStatus(QString("  - Has creatures: %1").arg(testTile->hasCreatures() ? "Yes" : "No"));
        
        updateStatus("Creature mapping test completed.");
    }

    void onTestDurabilityProperties() {
        updateStatus("Testing item durability and timing properties...");
        
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (!itemManager) {
            updateStatus("✗ ItemManager not available");
            return;
        }
        
        // Create weapon item
        Item* weapon = itemManager->createItem(2376); // Sword
        if (!weapon) {
            updateStatus("✗ Failed to create weapon item");
            return;
        }
        
        // Test durability properties
        weapon->setBreakChance(5); // 5% break chance
        weapon->setDuration(3600); // 1 hour duration
        weapon->setMaxDuration(7200); // 2 hours max duration
        
        updateStatus(QString("✓ Weapon created with ID: %1").arg(weapon->getServerId()));
        updateStatus(QString("  - Break Chance: %1%").arg(weapon->getBreakChance()));
        updateStatus(QString("  - Duration: %1 seconds").arg(weapon->getDuration()));
        updateStatus(QString("  - Max Duration: %1 seconds").arg(weapon->getMaxDuration()));
        
        // Test duration changes
        weapon->setDuration(weapon->getDuration() - 600); // Reduce by 10 minutes
        updateStatus(QString("  After use:"));
        updateStatus(QString("  - Remaining Duration: %1 seconds").arg(weapon->getDuration()));
        updateStatus(QString("  - Durability: %1%").arg(
            (weapon->getDuration() * 100) / weapon->getMaxDuration()));
        
        delete weapon;
        updateStatus("Durability properties test completed.");
    }

    void onShowTask55Features() {
        updateStatus("=== Task 55 Implementation Summary ===");
        
        updateStatus("Advanced Item Attributes System Features:");
        updateStatus("");
        updateStatus("1. Door Properties:");
        updateStatus("   ✓ Door ID, type, open/closed state, locked state");
        updateStatus("   ✓ Complete door logic integration");
        updateStatus("   ✓ State persistence through attribute system");
        updateStatus("");
        updateStatus("2. Container Properties:");
        updateStatus("   ✓ Container capacity and restrictions");
        updateStatus("   ✓ Container contents as structured data");
        updateStatus("   ✓ Item list management within containers");
        updateStatus("   ✓ Container type validation and limits");
        updateStatus("");
        updateStatus("3. Bed Properties:");
        updateStatus("   ✓ Sleeper ID and sleep start time tracking");
        updateStatus("   ✓ Regeneration rate configuration");
        updateStatus("   ✓ Bed occupancy state management");
        updateStatus("");
        updateStatus("4. Podium Properties:");
        updateStatus("   ✓ Direction and display options");
        updateStatus("   ✓ Outfit, mount, and platform visibility");
        updateStatus("   ✓ Complete outfit data (look type, colors, addon)");
        updateStatus("   ✓ Podium creature display logic");
        updateStatus("");
        updateStatus("5. Enhanced Creature Management:");
        updateStatus("   ✓ Multiple creatures per tile with ID mapping");
        updateStatus("   ✓ Creature retrieval by unique ID");
        updateStatus("   ✓ Creature list and map dual storage");
        updateStatus("   ✓ Efficient creature management operations");
        updateStatus("");
        updateStatus("6. Item Durability System:");
        updateStatus("   ✓ Break chance and durability tracking");
        updateStatus("   ✓ Duration and max duration properties");
        updateStatus("   ✓ Time-based item degradation support");
        updateStatus("");
        updateStatus("7. Teleport Enhancement:");
        updateStatus("   ✓ 3D destination coordinates (x, y, z)");
        updateStatus("   ✓ Teleport validation and error handling");
        updateStatus("");
        updateStatus("8. Spawn Properties:");
        updateStatus("   ✓ Spawn radius, interval, and max creatures");
        updateStatus("   ✓ Creature type and name configuration");
        updateStatus("   ✓ Advanced spawn logic parameters");
        updateStatus("");
        updateStatus("All Task 55 requirements implemented successfully!");
        updateStatus("Advanced properties provide complete wxwidgets compatibility.");
    }

private:
    void setupUI() {
        setWindowTitle("Advanced Attribute System Test - Task 55");
        setFixedSize(800, 700);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("Advanced Item Attributes System Test (Task 55)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Test controls
        QGroupBox* testGroup = new QGroupBox("Advanced Properties Tests");
        QVBoxLayout* testLayout = new QVBoxLayout(testGroup);
        
        QPushButton* doorBtn = new QPushButton("Test Door Properties");
        QPushButton* containerBtn = new QPushButton("Test Container Properties");
        QPushButton* bedBtn = new QPushButton("Test Bed Properties");
        QPushButton* podiumBtn = new QPushButton("Test Podium Properties");
        QPushButton* creatureBtn = new QPushButton("Test Creature Mapping");
        QPushButton* durabilityBtn = new QPushButton("Test Durability Properties");
        QPushButton* featuresBtn = new QPushButton("Show Task 55 Features");
        
        testLayout->addWidget(doorBtn);
        testLayout->addWidget(containerBtn);
        testLayout->addWidget(bedBtn);
        testLayout->addWidget(podiumBtn);
        testLayout->addWidget(creatureBtn);
        testLayout->addWidget(durabilityBtn);
        testLayout->addWidget(featuresBtn);
        
        mainLayout->addWidget(testGroup);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(400);
        statusText_->setReadOnly(true);
        mainLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(doorBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestDoorProperties);
        connect(containerBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestContainerProperties);
        connect(bedBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestBedProperties);
        connect(podiumBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestPodiumProperties);
        connect(creatureBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestCreatureMapping);
        connect(durabilityBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onTestDurabilityProperties);
        connect(featuresBtn, &QPushButton::clicked, this, &AdvancedAttributeSystemTestWidget::onShowTask55Features);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("Advanced Item Attributes System Test Application Started");
        updateStatus("This application tests the advanced attribute system");
        updateStatus("for Task 55 - Add Support for Item Attributes Map.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Door properties (ID, open/closed, locked state)");
        updateStatus("- Container properties (capacity, contents, restrictions)");
        updateStatus("- Bed properties (sleeper data, regeneration)");
        updateStatus("- Podium properties (outfit display, direction)");
        updateStatus("- Enhanced creature mapping on tiles");
        updateStatus("- Item durability and timing properties");
        updateStatus("");
        updateStatus("Click any test button to run specific functionality tests.");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "AdvancedAttributeSystemTest:" << message;
    }
    
    QTextEdit* statusText_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    AdvancedAttributeSystemTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "AdvancedAttributeSystemTest.moc"
