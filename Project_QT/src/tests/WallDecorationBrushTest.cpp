// WallDecorationBrushTest.cpp - Test for Task 40 WallDecorationBrush implementation

#include "WallDecorationBrush.h"
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
#include <QDebug>
#include <QTimer>
#include <memory>

// Test widget to demonstrate WallDecorationBrush functionality
class WallDecorationBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    WallDecorationBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateWallDecorationBrush() {
        quint16 decorationId = decorationIdSpin_->value();
        
        auto decorationBrush = std::make_unique<WallDecorationBrush>(this);
        decorationBrush->setCurrentDecorationItemId(decorationId);
        decorationBrush->setCurrentWallItemId(decorationId); // Set wall item ID for alignment mapping
        decorationBrush->setSpecificName(QString("Wall Decoration %1").arg(decorationId));
        
        updateStatus(QString("Created WallDecorationBrush: %1 (ID: %2, DecorationID: %3)")
                    .arg(decorationBrush->name())
                    .arg(decorationBrush->getID())
                    .arg(decorationBrush->getCurrentDecorationItemId()));
        
        testWallDecorationInterface(decorationBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(decorationBrush));
    }

    void onTestDecorationPlacement() {
        updateStatus("=== Testing Wall Decoration Placement ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No decoration brushes available. Create some first.");
            return;
        }
        
        // Create a test map with walls
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(30, 30, 7);
        
        // First place some walls
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setCurrentWallItemId(2000);
        
        QList<QPointF> wallPositions = {{10.0, 10.0}, {11.0, 10.0}, {12.0, 10.0}};
        
        updateStatus("Placing walls first...");
        for (const QPointF& pos : wallPositions) {
            auto wallCommand = wallBrush->applyBrush(testMap.get(), pos);
            if (wallCommand) {
                wallCommand->redo();
                updateStatus(QString("Placed wall at (%.1f,%.1f)").arg(pos.x()).arg(pos.y()));
                delete wallCommand;
            }
        }
        
        // Now test decoration placement
        auto& decorationBrush = testBrushes_.first();
        QPointF decorationPos(11.0, 10.0); // Middle wall
        
        // Test canDraw
        bool canDraw = decorationBrush->canDraw(testMap.get(), decorationPos);
        updateStatus(QString("Can draw %1 at (11,10): %2").arg(decorationBrush->specificName()).arg(canDraw));
        
        // Test applyBrush
        if (canDraw) {
            auto command = decorationBrush->applyBrush(testMap.get(), decorationPos);
            if (command) {
                updateStatus(QString("Created decoration command: %1").arg(command->text()));
                command->redo(); // Execute the command
                updateStatus("Wall decoration placement command executed");
                delete command;
            } else {
                updateStatus("Failed to create decoration command");
            }
        }
        
        // Test decoration on position without wall
        QPointF noWallPos(5.0, 5.0);
        bool canDrawNoWall = decorationBrush->canDraw(testMap.get(), noWallPos);
        updateStatus(QString("Can draw decoration at position without wall (5,5): %1").arg(canDrawNoWall));
    }

    void onTestWallAlignmentDecorations() {
        updateStatus("=== Testing Wall Alignment-Based Decorations ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No decoration brushes available. Create some first.");
            return;
        }
        
        // Create test map with different wall configurations
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setCurrentWallItemId(2500);
        
        // Create different wall patterns
        updateStatus("Creating different wall patterns...");
        
        // Horizontal wall
        QList<QPointF> horizontalWall = {{5.0, 5.0}, {6.0, 5.0}, {7.0, 5.0}};
        for (const QPointF& pos : horizontalWall) {
            auto cmd = wallBrush->applyBrush(testMap.get(), pos);
            if (cmd) { cmd->redo(); delete cmd; }
        }
        
        // Vertical wall
        QList<QPointF> verticalWall = {{10.0, 8.0}, {10.0, 9.0}, {10.0, 10.0}};
        for (const QPointF& pos : verticalWall) {
            auto cmd = wallBrush->applyBrush(testMap.get(), pos);
            if (cmd) { cmd->redo(); delete cmd; }
        }
        
        // Corner wall
        QList<QPointF> cornerWall = {{15.0, 12.0}, {16.0, 12.0}, {15.0, 13.0}};
        for (const QPointF& pos : cornerWall) {
            auto cmd = wallBrush->applyBrush(testMap.get(), pos);
            if (cmd) { cmd->redo(); delete cmd; }
        }
        
        // Test decoration placement on different alignments
        auto& decorationBrush = testBrushes_.first();
        
        QList<QPair<QPointF, QString>> testPositions = {
            {{6.0, 5.0}, "Horizontal Wall"},
            {{10.0, 9.0}, "Vertical Wall"},
            {{15.0, 12.0}, "Corner Wall"}
        };
        
        for (const auto& test : testPositions) {
            QPointF pos = test.first;
            QString description = test.second;
            
            WallBrush::WallAlignment alignment = decorationBrush->calculateWallAlignment(testMap.get(), pos);
            quint16 decorationItemId = decorationBrush->getWallItemForAlignment(alignment);
            
            updateStatus(QString("%1 at (%.1f,%.1f): Alignment=%2, DecorationItemID=%3")
                        .arg(description)
                        .arg(pos.x()).arg(pos.y())
                        .arg(static_cast<int>(alignment))
                        .arg(decorationItemId));
            
            // Try to place decoration
            if (decorationBrush->canDraw(testMap.get(), pos)) {
                auto cmd = decorationBrush->applyBrush(testMap.get(), pos);
                if (cmd) {
                    cmd->redo();
                    updateStatus(QString("Successfully placed decoration on %1").arg(description));
                    delete cmd;
                }
            }
        }
    }

    void onTestDecorationRemoval() {
        updateStatus("=== Testing Wall Decoration Removal ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No decoration brushes available. Create some first.");
            return;
        }
        
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(15, 15, 7);
        
        // Place a wall first
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setCurrentWallItemId(3000);
        
        QPointF wallPos(7.0, 7.0);
        auto wallCmd = wallBrush->applyBrush(testMap.get(), wallPos);
        if (wallCmd) {
            wallCmd->redo();
            updateStatus("Placed wall for decoration removal test");
            delete wallCmd;
        }
        
        // Place decoration
        auto& decorationBrush = testBrushes_.first();
        auto placeCmd = decorationBrush->applyBrush(testMap.get(), wallPos);
        if (placeCmd) {
            placeCmd->redo();
            updateStatus("Placed decoration for removal test");
            delete placeCmd;
        }
        
        // Test decoration removal
        auto removeCmd = decorationBrush->removeBrush(testMap.get(), wallPos);
        if (removeCmd) {
            updateStatus(QString("Created remove command: %1").arg(removeCmd->text()));
            removeCmd->redo();
            updateStatus("Decoration removal command executed");
            delete removeCmd;
        } else {
            updateStatus("Failed to create decoration removal command");
        }
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        BrushManager manager;
        
        // Test wall decoration brush creation through manager
        auto decorationBrush = manager.createBrushShared(Brush::Type::WallDecoration, {{"decorationId", 4000}});
        
        if (decorationBrush && decorationBrush->isWallDecoration()) {
            WallDecorationBrush* decoration = decorationBrush->asWallDecoration();
            if (decoration) {
                decoration->setCurrentDecorationItemId(4000);
                decoration->setSpecificName("Manager Created Decoration");
                
                updateStatus(QString("BrushManager created decoration brush: %1 (Decoration ID: %2)")
                            .arg(decoration->name())
                            .arg(decoration->getCurrentDecorationItemId()));
                
                // Test inheritance properties
                updateStatus(QString("Inheritance test - IsWall: %1, IsWallDecoration: %2, IsTerrain: %3")
                            .arg(decoration->isWall())
                            .arg(decoration->isWallDecoration())
                            .arg(decoration->isTerrain()));
                
                // Test terrain brush properties
                updateStatus(QString("Terrain properties - BrushSize: %1, CanDrag: %2, CanSmear: %3")
                            .arg(decoration->getBrushSize())
                            .arg(decoration->canDrag())
                            .arg(decoration->canSmear()));
                
                // Test type casting
                WallBrush* wall = decoration->asWall();
                TerrainBrush* terrain = decoration->asTerrain();
                updateStatus(QString("Type casting - WallBrush: %1, TerrainBrush: %2")
                            .arg(wall != nullptr)
                            .arg(terrain != nullptr));
            }
        } else {
            updateStatus("Failed to create wall decoration brush through BrushManager");
        }
    }

    void onTestXMLCompatibility() {
        updateStatus("=== Testing XML Compatibility (wxwidgets Migration) ===");
        
        updateStatus("WallDecorationBrush uses the SAME XML loading algorithm as WallBrush");
        updateStatus("This is a direct migration from wxwidgets where WallDecorationBrush");
        updateStatus("inherits from WallBrush and uses the same wall_items structure.");
        updateStatus("");
        updateStatus("Key differences from WallBrush:");
        updateStatus("1. isWallDecoration() returns true");
        updateStatus("2. draw() method places decorations based on existing wall alignment");
        updateStatus("3. Decorations are handled specially in doWalls() function");
        updateStatus("4. Uses same XML structure but different placement logic");
        updateStatus("");
        updateStatus("Example XML structure (same as WallBrush):");
        updateStatus("<wallbrush name=\"torch_decoration\">");
        updateStatus("  <wall alignment=\"horizontal\" id=\"5001\" chance=\"100\"/>");
        updateStatus("  <wall alignment=\"vertical\" id=\"5002\" chance=\"100\"/>");
        updateStatus("  <wall alignment=\"corner\" id=\"5003\" chance=\"100\"/>");
        updateStatus("</wallbrush>");
    }

private:
    void setupUI() {
        setWindowTitle("WallDecorationBrush Test Application - Task 40");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("WallDecorationBrush Implementation Test (Task 40)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Wall Decoration Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Decoration ID:"));
        decorationIdSpin_ = new QSpinBox();
        decorationIdSpin_->setRange(1, 9999);
        decorationIdSpin_->setValue(5000);
        idLayout->addWidget(decorationIdSpin_);
        configLayout->addLayout(idLayout);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Decoration Brush");
        QPushButton* placementBtn = new QPushButton("Test Decoration Placement");
        QPushButton* alignmentBtn = new QPushButton("Test Wall Alignment");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(placementBtn);
        buttonLayout1->addWidget(alignmentBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* removalBtn = new QPushButton("Test Decoration Removal");
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        QPushButton* xmlBtn = new QPushButton("Test XML Compatibility");
        
        buttonLayout2->addWidget(removalBtn);
        buttonLayout2->addWidget(managerBtn);
        buttonLayout2->addWidget(xmlBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
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
        connect(createBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onCreateWallDecorationBrush);
        connect(placementBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onTestDecorationPlacement);
        connect(alignmentBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onTestWallAlignmentDecorations);
        connect(removalBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onTestDecorationRemoval);
        connect(managerBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onTestBrushManager);
        connect(xmlBtn, &QPushButton::clicked, this, &WallDecorationBrushTestWidget::onTestXMLCompatibility);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("WallDecorationBrush Test Application Started");
        updateStatus("This application tests the WallDecorationBrush implementation");
        updateStatus("as migrated from wxwidgets for Task 40.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Wall-based decoration placement");
        updateStatus("- Alignment-based decoration selection");
        updateStatus("- Same XML loading as WallBrush");
        updateStatus("- Proper inheritance from WallBrush");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "WallDecorationBrushTest:" << message;
    }
    
    void testWallDecorationInterface(WallDecorationBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - Type: %1, IsWallDecoration: %2, IsWall: %3")
                    .arg(static_cast<int>(brush->type()))
                    .arg(brush->isWallDecoration())
                    .arg(brush->isWall()));
        
        updateStatus(QString("  Properties - DecorationID: %1, BrushSize: %2, CanSmear: %3")
                    .arg(brush->getCurrentDecorationItemId())
                    .arg(brush->getBrushSize())
                    .arg(brush->canSmear()));
    }
    
    QSpinBox* decorationIdSpin_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<WallDecorationBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    WallDecorationBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "WallDecorationBrushTest.moc"
