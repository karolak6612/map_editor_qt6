// GroundBrushTest.cpp - Comprehensive test for Task 38 GroundBrush implementation

#include "GroundBrush.h"
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
#include <QCheckBox>
#include <QLineEdit>
#include <QDebug>
#include <QTimer>
#include <QDomDocument>
#include <memory>

// Test widget to demonstrate GroundBrush functionality
class GroundBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    GroundBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateGroundBrush() {
        quint16 groundId = groundIdSpin_->value();
        
        auto groundBrush = std::make_unique<GroundBrush>(groundId, this);
        
        // Set optional properties
        groundBrush->hasOptionalBorder_ = hasOptionalBorderCheck_->isChecked();
        groundBrush->useSoloOptionalBorder_ = useSoloOptionalCheck_->isChecked();
        groundBrush->isReRandomizable_ = isReRandomizableCheck_->isChecked();
        
        // Set ground equivalent group if specified
        QString equivalentGroup = equivalentGroupEdit_->text();
        if (!equivalentGroup.isEmpty()) {
            QStringList idStrings = equivalentGroup.split(",", Qt::SkipEmptyParts);
            for (const QString& idStr : idStrings) {
                bool ok;
                quint16 id = idStr.trimmed().toUShort(&ok);
                if (ok && id > 0) {
                    groundBrush->groundEquivalentGroup_.append(id);
                }
            }
        }
        
        updateStatus(QString("Created GroundBrush: %1 (ID: %2, LookID: %3)")
                    .arg(groundBrush->name())
                    .arg(groundBrush->getID())
                    .arg(groundBrush->getLookID()));
        
        updateStatus(QString("  Properties - HasOptionalBorder: %1, UseSoloOptional: %2, IsReRandomizable: %3")
                    .arg(groundBrush->hasOptionalBorder())
                    .arg(groundBrush->useSoloOptionalBorder())
                    .arg(groundBrush->isReRandomizable()));
        
        testGroundBrushInterface(groundBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(groundBrush));
    }

    void onTestGroundPlacement() {
        updateStatus("=== Testing Ground Placement ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No ground brushes available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(100, 100, 7); // 100x100 tiles, 7 floors
        
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
                updateStatus("Ground placement command executed");
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
            updateStatus("Ground removal command executed");
            delete removeCommand;
        }
    }

    void onTestGroundEquivalent() {
        updateStatus("=== Testing Ground Equivalent System ===");
        
        // Create ground brushes with different equivalent groups
        auto grassBrush = std::make_unique<GroundBrush>(100, this);
        grassBrush->setSpecificName("Grass");
        grassBrush->groundEquivalentGroup_ = {100, 101, 102}; // Grass group
        
        auto stoneBrush = std::make_unique<GroundBrush>(200, this);
        stoneBrush->setSpecificName("Stone");
        stoneBrush->groundEquivalentGroup_ = {200, 201, 202}; // Stone group
        
        auto sandBrush = std::make_unique<GroundBrush>(300, this);
        sandBrush->setSpecificName("Sand");
        // No equivalent group - can place anywhere
        
        // Create test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(50, 50, 7);
        
        QPointF testPos(5.0, 5.0);
        
        // Test ground equivalent checking
        updateStatus(QString("Grass brush can place grass (100): %1")
                    .arg(grassBrush->checkGroundEquivalent(testMap.get(), testPos, 100)));
        updateStatus(QString("Grass brush can place grass variant (101): %1")
                    .arg(grassBrush->checkGroundEquivalent(testMap.get(), testPos, 101)));
        updateStatus(QString("Grass brush can place stone (200): %1")
                    .arg(grassBrush->checkGroundEquivalent(testMap.get(), testPos, 200)));
        
        updateStatus(QString("Stone brush can place stone (200): %1")
                    .arg(stoneBrush->checkGroundEquivalent(testMap.get(), testPos, 200)));
        updateStatus(QString("Stone brush can place grass (100): %1")
                    .arg(stoneBrush->checkGroundEquivalent(testMap.get(), testPos, 100)));
        
        updateStatus(QString("Sand brush (no restrictions) can place anything: %1")
                    .arg(sandBrush->checkGroundEquivalent(testMap.get(), testPos, 999)));
        
        // Test placement validation
        updateStatus(QString("Grass brush can place at position: %1")
                    .arg(grassBrush->canPlaceGroundAt(testMap.get(), testPos, 101)));
        updateStatus(QString("Stone brush can place at position: %1")
                    .arg(stoneBrush->canPlaceGroundAt(testMap.get(), testPos, 201)));
        
        // Store brushes
        testBrushes_.append(std::move(grassBrush));
        testBrushes_.append(std::move(stoneBrush));
        testBrushes_.append(std::move(sandBrush));
    }

    void onTestBorderSystem() {
        updateStatus("=== Testing Border System Hooks ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No ground brushes available. Create some first.");
            return;
        }
        
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& brush = testBrushes_.first();
        QPointF testPos(8.0, 8.0);
        
        // Test border update request
        updateStatus(QString("Testing border update request for %1 at (8,8)").arg(brush->specificName()));
        brush->requestBorderUpdate(testMap.get(), testPos);
        updateStatus("Border update request completed (check debug output)");
        
        // Test with multiple positions
        QList<QPointF> positions = {{5.0, 5.0}, {10.0, 10.0}, {15.0, 15.0}};
        for (const QPointF& pos : positions) {
            brush->requestBorderUpdate(testMap.get(), pos);
        }
        updateStatus("Multiple border update requests completed");
    }

    void onTestXMLLoading() {
        updateStatus("=== Testing XML Loading Support ===");
        
        // Create XML document for testing
        QDomDocument doc;
        QDomElement root = doc.createElement("groundBrush");
        root.setAttribute("groundId", "150");
        root.setAttribute("hasOptionalBorder", "true");
        root.setAttribute("useSoloOptionalBorder", "false");
        root.setAttribute("isReRandomizable", "true");
        root.setAttribute("groundEquivalentGroup", "150,151,152,153");
        doc.appendChild(root);
        
        // Create ground brush and load from XML
        auto groundBrush = std::make_unique<GroundBrush>(this);
        QStringList warnings;
        
        bool loadSuccess = groundBrush->load(root, warnings);
        
        updateStatus(QString("XML loading successful: %1").arg(loadSuccess));
        if (!warnings.isEmpty()) {
            updateStatus(QString("Warnings: %1").arg(warnings.join("; ")));
        }
        
        updateStatus(QString("Loaded brush: %1 (Ground ID: %2)")
                    .arg(groundBrush->name())
                    .arg(groundBrush->getCurrentGroundItemId()));
        
        updateStatus(QString("Properties - HasOptionalBorder: %1, UseSoloOptional: %2, IsReRandomizable: %3")
                    .arg(groundBrush->hasOptionalBorder())
                    .arg(groundBrush->useSoloOptionalBorder())
                    .arg(groundBrush->isReRandomizable()));
        
        updateStatus(QString("Equivalent group size: %1").arg(groundBrush->groundEquivalentGroup_.size()));
        
        testBrushes_.append(std::move(groundBrush));
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        BrushManager manager;
        
        // Test ground brush creation through manager
        auto groundBrush = manager.createBrushShared(Brush::Type::Ground, {{"groundId", 250}});
        
        if (groundBrush && groundBrush->isGround()) {
            GroundBrush* ground = groundBrush->asGround();
            if (ground) {
                ground->setCurrentGroundItemId(250);
                ground->setSpecificName("Manager Created Ground");
                
                updateStatus(QString("BrushManager created ground brush: %1 (Ground ID: %2)")
                            .arg(ground->name()).arg(ground->getCurrentGroundItemId()));
                
                // Test terrain brush properties
                updateStatus(QString("Terrain properties - NeedBorders: %1, CanDrag: %2, Z-order: %3")
                            .arg(ground->needBorders())
                            .arg(ground->canDrag())
                            .arg(ground->getZ()));
                
                // Test type casting
                TerrainBrush* terrain = ground->asTerrain();
                updateStatus(QString("TerrainBrush cast successful: %1").arg(terrain != nullptr));
                
                if (terrain) {
                    updateStatus(QString("Terrain specific name: %1").arg(terrain->specificName()));
                }
            }
        } else {
            updateStatus("Failed to create ground brush through BrushManager");
        }
    }

private:
    void setupUI() {
        setWindowTitle("GroundBrush Test Application - Task 38");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("GroundBrush Implementation Test (Task 38)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Ground Brush Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Ground ID:"));
        groundIdSpin_ = new QSpinBox();
        groundIdSpin_->setRange(1, 9999);
        groundIdSpin_->setValue(100);
        idLayout->addWidget(groundIdSpin_);
        configLayout->addLayout(idLayout);
        
        QHBoxLayout* propsLayout = new QHBoxLayout();
        hasOptionalBorderCheck_ = new QCheckBox("Has Optional Border");
        useSoloOptionalCheck_ = new QCheckBox("Use Solo Optional Border");
        isReRandomizableCheck_ = new QCheckBox("Is Re-Randomizable");
        propsLayout->addWidget(hasOptionalBorderCheck_);
        propsLayout->addWidget(useSoloOptionalCheck_);
        propsLayout->addWidget(isReRandomizableCheck_);
        configLayout->addLayout(propsLayout);
        
        QHBoxLayout* equivalentLayout = new QHBoxLayout();
        equivalentLayout->addWidget(new QLabel("Equivalent Group (comma-separated):"));
        equivalentGroupEdit_ = new QLineEdit("100,101,102");
        equivalentLayout->addWidget(equivalentGroupEdit_);
        configLayout->addLayout(equivalentLayout);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Ground Brush");
        QPushButton* placementBtn = new QPushButton("Test Ground Placement");
        QPushButton* equivalentBtn = new QPushButton("Test Ground Equivalent");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(placementBtn);
        buttonLayout1->addWidget(equivalentBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* borderBtn = new QPushButton("Test Border System");
        QPushButton* xmlBtn = new QPushButton("Test XML Loading");
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        
        buttonLayout2->addWidget(borderBtn);
        buttonLayout2->addWidget(xmlBtn);
        buttonLayout2->addWidget(managerBtn);
        
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
        connect(createBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onCreateGroundBrush);
        connect(placementBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onTestGroundPlacement);
        connect(equivalentBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onTestGroundEquivalent);
        connect(borderBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onTestBorderSystem);
        connect(xmlBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onTestXMLLoading);
        connect(managerBtn, &QPushButton::clicked, this, &GroundBrushTestWidget::onTestBrushManager);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("GroundBrush Test Application Started");
        updateStatus("This application tests the complete GroundBrush implementation");
        updateStatus("as required by Task 38, including item placement and border system hooks.");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "GroundBrushTest:" << message;
    }
    
    void testGroundBrushInterface(GroundBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - Type: %1, IsGround: %2, IsTerrain: %3")
                    .arg(static_cast<int>(brush->type()))
                    .arg(brush->isGround())
                    .arg(brush->isTerrain()));
        
        updateStatus(QString("  Ground properties - CurrentGroundID: %1, BrushSize: %2, BrushShape: %3")
                    .arg(brush->getCurrentGroundItemId())
                    .arg(brush->getBrushSize())
                    .arg(static_cast<int>(brush->getBrushShape())));
    }
    
    QSpinBox* groundIdSpin_;
    QCheckBox* hasOptionalBorderCheck_;
    QCheckBox* useSoloOptionalCheck_;
    QCheckBox* isReRandomizableCheck_;
    QLineEdit* equivalentGroupEdit_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<GroundBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    GroundBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "GroundBrushTest.moc"
