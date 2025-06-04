// WallBrushTest.cpp - Comprehensive test for Task 39 WallBrush implementation

#include "WallBrush.h"
#include "BrushManager.h"
#include "Map.h"
#include "Tile.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDebug>
#include <QTimer>
#include <QDomDocument>
#include <memory>

// Test widget to demonstrate WallBrush functionality
class WallBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    WallBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateWallBrush() {
        quint16 wallId = wallIdSpin_->value();
        
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setCurrentWallItemId(wallId);
        wallBrush->setSpecificName(QString("Wall %1").arg(wallId));
        
        // Set door type
        WallBrush::DoorType doorType = static_cast<WallBrush::DoorType>(doorTypeCombo_->currentData().toInt());
        wallBrush->setDoorType(doorType);
        
        updateStatus(QString("Created WallBrush: %1 (ID: %2, WallID: %3, DoorType: %4)")
                    .arg(wallBrush->name())
                    .arg(wallBrush->getID())
                    .arg(wallBrush->getCurrentWallItemId())
                    .arg(static_cast<int>(wallBrush->getDoorType())));
        
        testWallBrushInterface(wallBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(wallBrush));
    }

    void onTestWallPlacement() {
        updateStatus("=== Testing Wall Placement ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No wall brushes available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(50, 50, 7);
        
        auto& brush = testBrushes_.first();
        QPointF testPos(10.0, 10.0);
        
        // Test canDraw
        bool canDraw = brush->canDraw(testMap.get(), testPos);
        updateStatus(QString("Can draw %1 at (10,10): %2").arg(brush->specificName()).arg(canDraw));
        
        // Test applyBrush
        if (canDraw) {
            auto command = brush->applyBrush(testMap.get(), testPos);
            if (command) {
                updateStatus(QString("Created apply command: %1").arg(command->text()));
                command->redo(); // Execute the command
                updateStatus("Wall placement command executed");
                delete command;
            } else {
                updateStatus("Failed to create apply command");
            }
        }
        
        // Test removeBrush
        auto removeCommand = brush->removeBrush(testMap.get(), testPos);
        if (removeCommand) {
            updateStatus(QString("Created remove command: %1").arg(removeCommand->text()));
            removeCommand->redo(); // Execute the command
            updateStatus("Wall removal command executed");
            delete removeCommand;
        }
    }

    void onTestWallAlignment() {
        updateStatus("=== Testing Wall Alignment System ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No wall brushes available. Create some first.");
            return;
        }
        
        // Create test map with some walls
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& brush = testBrushes_.first();
        
        // Test different alignment scenarios
        QList<QPointF> testPositions = {
            {5.0, 5.0},   // Isolated wall
            {6.0, 5.0},   // Horizontal connection
            {5.0, 6.0},   // Vertical connection
            {7.0, 5.0},   // T-junction
            {6.0, 6.0}    // Cross
        };
        
        for (const QPointF& pos : testPositions) {
            WallBrush::WallAlignment alignment = brush->calculateWallAlignment(testMap.get(), pos);
            quint16 wallItemId = brush->getWallItemForAlignment(alignment);
            
            updateStatus(QString("Position (%.1f,%.1f): Alignment=%1, WallItemID=%2")
                        .arg(pos.x()).arg(pos.y())
                        .arg(static_cast<int>(alignment))
                        .arg(wallItemId));
        }
        
        // Test wall update requests
        brush->requestWallUpdate(testMap.get(), QPointF(10.0, 10.0));
        updateStatus("Wall update request completed (check debug output)");
    }

    void onTestDoorLogic() {
        updateStatus("=== Testing Door Logic ===");
        
        // Create wall brush with door functionality
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setCurrentWallItemId(2000);
        wallBrush->setSpecificName("Door Wall");
        
        // Test different door types
        QList<WallBrush::DoorType> doorTypes = {
            WallBrush::DoorType::Normal,
            WallBrush::DoorType::Locked,
            WallBrush::DoorType::Quest,
            WallBrush::DoorType::Magic,
            WallBrush::DoorType::Level,
            WallBrush::DoorType::Key
        };
        
        for (WallBrush::DoorType doorType : doorTypes) {
            wallBrush->setDoorType(doorType);
            updateStatus(QString("Set door type to: %1").arg(static_cast<int>(doorType)));
            updateStatus(QString("Current door type: %1").arg(static_cast<int>(wallBrush->getDoorType())));
        }
        
        // Test door item identification
        QList<quint16> testItemIds = {1050, 1150, 1250, 1350, 1450, 1550, 2500, 3500};
        
        for (quint16 itemId : testItemIds) {
            bool isDoor = wallBrush->isDoorItem(itemId);
            WallBrush::DoorType doorType = wallBrush->getDoorTypeFromID(itemId);
            bool isWall = wallBrush->hasWall(itemId);
            
            updateStatus(QString("Item %1: IsDoor=%2, DoorType=%3, IsWall=%4")
                        .arg(itemId)
                        .arg(isDoor)
                        .arg(static_cast<int>(doorType))
                        .arg(isWall));
        }
        
        testBrushes_.append(std::move(wallBrush));
    }

    void onTestWallSelection() {
        updateStatus("=== Testing Wall Selection ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No wall brushes available. Create some first.");
            return;
        }
        
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(15, 15, 7);
        
        auto& brush = testBrushes_.first();
        
        // Test wall selection/deselection
        QList<QPointF> positions = {{3.0, 3.0}, {5.0, 5.0}, {7.0, 7.0}};
        
        for (const QPointF& pos : positions) {
            brush->selectWall(testMap.get(), pos);
            updateStatus(QString("Selected wall at (%.1f,%.1f)").arg(pos.x()).arg(pos.y()));
        }
        
        for (const QPointF& pos : positions) {
            brush->deselectWall(testMap.get(), pos);
            updateStatus(QString("Deselected wall at (%.1f,%.1f)").arg(pos.x()).arg(pos.y()));
        }
        
        updateStatus("Wall selection/deselection tests completed (check debug output)");
    }

    void onTestXMLLoading() {
        updateStatus("=== Testing XML Loading Support ===");
        
        // Create XML document for testing
        QDomDocument doc;
        QDomElement root = doc.createElement("wallBrush");
        root.setAttribute("wallId", "2500");
        root.setAttribute("doorType", "Quest");
        
        // Add alignment mappings
        QDomElement alignments = doc.createElement("alignments");
        
        QDomElement horizontal = doc.createElement("horizontal");
        horizontal.appendChild(doc.createTextNode("2500"));
        alignments.appendChild(horizontal);
        
        QDomElement vertical = doc.createElement("vertical");
        vertical.appendChild(doc.createTextNode("2501"));
        alignments.appendChild(vertical);
        
        QDomElement corner = doc.createElement("corner");
        corner.appendChild(doc.createTextNode("2502"));
        alignments.appendChild(corner);
        
        root.appendChild(alignments);
        
        // Add door ranges
        QDomElement doorRanges = doc.createElement("doorRanges");
        QDomElement doorRange1 = doc.createElement("range");
        doorRange1.setAttribute("start", "1200");
        doorRange1.setAttribute("end", "1299");
        doorRanges.appendChild(doorRange1);
        root.appendChild(doorRanges);
        
        // Add wall ranges
        QDomElement wallRanges = doc.createElement("wallRanges");
        QDomElement wallRange1 = doc.createElement("range");
        wallRange1.setAttribute("start", "2500");
        wallRange1.setAttribute("end", "2599");
        wallRanges.appendChild(wallRange1);
        root.appendChild(wallRanges);
        
        doc.appendChild(root);
        
        // Create wall brush and load from XML
        auto wallBrush = std::make_unique<WallBrush>(this);
        QStringList warnings;
        
        bool loadSuccess = wallBrush->load(root, warnings);
        
        updateStatus(QString("XML loading successful: %1").arg(loadSuccess));
        if (!warnings.isEmpty()) {
            updateStatus(QString("Warnings: %1").arg(warnings.join("; ")));
        }
        
        updateStatus(QString("Loaded brush: %1 (Wall ID: %2, Door Type: %3)")
                    .arg(wallBrush->name())
                    .arg(wallBrush->getCurrentWallItemId())
                    .arg(static_cast<int>(wallBrush->getDoorType())));
        
        // Test loaded alignment mappings
        updateStatus(QString("Horizontal wall item: %1").arg(wallBrush->getWallItemForAlignment(WallBrush::WallAlignment::Horizontal)));
        updateStatus(QString("Vertical wall item: %1").arg(wallBrush->getWallItemForAlignment(WallBrush::WallAlignment::Vertical)));
        updateStatus(QString("Corner wall item: %1").arg(wallBrush->getWallItemForAlignment(WallBrush::WallAlignment::Corner)));
        
        testBrushes_.append(std::move(wallBrush));
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        BrushManager manager;
        
        // Test wall brush creation through manager
        auto wallBrush = manager.createBrushShared(Brush::Type::Wall, {{"wallId", 3000}});
        
        if (wallBrush && wallBrush->isWall()) {
            WallBrush* wall = wallBrush->asWall();
            if (wall) {
                wall->setCurrentWallItemId(3000);
                wall->setSpecificName("Manager Created Wall");
                wall->setDoorType(WallBrush::DoorType::Magic);
                
                updateStatus(QString("BrushManager created wall brush: %1 (Wall ID: %2, Door Type: %3)")
                            .arg(wall->name())
                            .arg(wall->getCurrentWallItemId())
                            .arg(static_cast<int>(wall->getDoorType())));
                
                // Test terrain brush properties
                updateStatus(QString("Terrain properties - NeedBorders: %1, CanDrag: %2, Z-order: %3")
                            .arg(wall->needBorders())
                            .arg(wall->canDrag())
                            .arg(wall->getZ()));
                
                // Test type casting
                TerrainBrush* terrain = wall->asTerrain();
                updateStatus(QString("TerrainBrush cast successful: %1").arg(terrain != nullptr));
                
                if (terrain) {
                    updateStatus(QString("Terrain specific name: %1").arg(terrain->specificName()));
                }
            }
        } else {
            updateStatus("Failed to create wall brush through BrushManager");
        }
    }

private:
    void setupUI() {
        setWindowTitle("WallBrush Test Application - Task 39");
        setFixedSize(1100, 900);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("WallBrush Implementation Test (Task 39)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Wall Brush Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Wall ID:"));
        wallIdSpin_ = new QSpinBox();
        wallIdSpin_->setRange(1, 9999);
        wallIdSpin_->setValue(2000);
        idLayout->addWidget(wallIdSpin_);
        configLayout->addLayout(idLayout);
        
        QHBoxLayout* doorLayout = new QHBoxLayout();
        doorLayout->addWidget(new QLabel("Door Type:"));
        doorTypeCombo_ = new QComboBox();
        doorTypeCombo_->addItem("None", static_cast<int>(WallBrush::DoorType::None));
        doorTypeCombo_->addItem("Normal", static_cast<int>(WallBrush::DoorType::Normal));
        doorTypeCombo_->addItem("Locked", static_cast<int>(WallBrush::DoorType::Locked));
        doorTypeCombo_->addItem("Quest", static_cast<int>(WallBrush::DoorType::Quest));
        doorTypeCombo_->addItem("Magic", static_cast<int>(WallBrush::DoorType::Magic));
        doorTypeCombo_->addItem("Level", static_cast<int>(WallBrush::DoorType::Level));
        doorTypeCombo_->addItem("Key", static_cast<int>(WallBrush::DoorType::Key));
        doorLayout->addWidget(doorTypeCombo_);
        configLayout->addLayout(doorLayout);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Wall Brush");
        QPushButton* placementBtn = new QPushButton("Test Wall Placement");
        QPushButton* alignmentBtn = new QPushButton("Test Wall Alignment");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(placementBtn);
        buttonLayout1->addWidget(alignmentBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* doorBtn = new QPushButton("Test Door Logic");
        QPushButton* selectionBtn = new QPushButton("Test Wall Selection");
        QPushButton* xmlBtn = new QPushButton("Test XML Loading");
        
        buttonLayout2->addWidget(doorBtn);
        buttonLayout2->addWidget(selectionBtn);
        buttonLayout2->addWidget(xmlBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
        QHBoxLayout* buttonLayout3 = new QHBoxLayout();
        
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        
        buttonLayout3->addWidget(managerBtn);
        buttonLayout3->addStretch();
        
        mainLayout->addLayout(buttonLayout3);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(450);
        statusText_->setReadOnly(true);
        mainLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(createBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onCreateWallBrush);
        connect(placementBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestWallPlacement);
        connect(alignmentBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestWallAlignment);
        connect(doorBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestDoorLogic);
        connect(selectionBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestWallSelection);
        connect(xmlBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestXMLLoading);
        connect(managerBtn, &QPushButton::clicked, this, &WallBrushTestWidget::onTestBrushManager);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("WallBrush Test Application Started");
        updateStatus("This application tests the complete WallBrush implementation");
        updateStatus("as required by Task 39, including wall placement, connection stubs, and door logic.");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "WallBrushTest:" << message;
    }
    
    void testWallBrushInterface(WallBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - Type: %1, IsWall: %2, IsTerrain: %3")
                    .arg(static_cast<int>(brush->type()))
                    .arg(brush->isWall())
                    .arg(brush->isTerrain()));
        
        updateStatus(QString("  Wall properties - CurrentWallID: %1, DoorType: %2, CanSmear: %3")
                    .arg(brush->getCurrentWallItemId())
                    .arg(static_cast<int>(brush->getDoorType()))
                    .arg(brush->canSmear()));
    }
    
    QSpinBox* wallIdSpin_;
    QComboBox* doorTypeCombo_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<WallBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    WallBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "WallBrushTest.moc"
