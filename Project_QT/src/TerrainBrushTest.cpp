// TerrainBrushTest.cpp - Comprehensive test for Task 37 TerrainBrush implementation

#include "TerrainBrush.h"
#include "GroundBrush.h"
#include "WallBrush.h"
#include "BrushManager.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QDebug>
#include <QTimer>
#include <memory>

// Test widget to demonstrate TerrainBrush functionality
class TerrainBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    TerrainBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateTerrainBrush() {
        QString brushName = brushNameEdit_->text();
        if (brushName.isEmpty()) {
            brushName = "Test Terrain";
        }
        
        // Create a concrete terrain brush (using GroundBrush as example)
        auto groundBrush = std::make_unique<GroundBrush>(100, this);
        groundBrush->setSpecificName(brushName);
        groundBrush->setLookID(lookIdSpin_->value());
        groundBrush->setZ(zOrderSpin_->value());
        
        updateStatus(QString("Created terrain brush: %1 (ID: %2, LookID: %3, Z: %4)")
                    .arg(groundBrush->name())
                    .arg(groundBrush->getID())
                    .arg(groundBrush->getLookID())
                    .arg(groundBrush->getZ()));
        
        testTerrainBrushInterface(groundBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(groundBrush));
    }

    void onTestFriendSystem() {
        updateStatus("=== Testing Friend/Enemy System ===");
        
        // Create different terrain brushes
        auto grassBrush = std::make_unique<GroundBrush>(100, this);
        grassBrush->setSpecificName("Grass");
        
        auto stoneBrush = std::make_unique<GroundBrush>(101, this);
        stoneBrush->setSpecificName("Stone");
        
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setSpecificName("Stone Wall");
        
        // Set up friend relationships
        grassBrush->addFriendBrushType(Brush::Type::Ground);
        grassBrush->addFriendBrushType(Brush::Type::Wall);
        
        stoneBrush->addFriendBrushType(Brush::Type::Ground);
        stoneBrush->setHatesFriends(hatesFriendsCheck_->isChecked());
        
        wallBrush->addFriendBrushType(Brush::Type::Ground);
        
        // Test friendships
        updateStatus(QString("Grass is friend with Stone: %1")
                    .arg(grassBrush->isFriendWith(stoneBrush->type())));
        updateStatus(QString("Grass is friend with Wall: %1")
                    .arg(grassBrush->isFriendWith(wallBrush->type())));
        updateStatus(QString("Stone hates friends: %1, is friend with Grass: %2")
                    .arg(stoneBrush->hatesFriends())
                    .arg(stoneBrush->isFriendWith(grassBrush->type())));
        
        // Test direct brush friendship
        updateStatus(QString("Grass and Stone are mutual friends: %1")
                    .arg(grassBrush->friendOf(stoneBrush.get())));
        updateStatus(QString("Grass and Wall are mutual friends: %1")
                    .arg(grassBrush->friendOf(wallBrush.get())));
        
        // Store brushes
        testBrushes_.append(std::move(grassBrush));
        testBrushes_.append(std::move(stoneBrush));
        testBrushes_.append(std::move(wallBrush));
    }

    void onTestDrawingStubs() {
        updateStatus("=== Testing Drawing Stubs ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No terrain brushes available. Create some first.");
            return;
        }
        
        // Test drawing stubs with the first available brush
        auto& brush = testBrushes_.first();
        
        updateStatus(QString("Testing drawing stubs for: %1").arg(brush->name()));
        
        // Test canDraw
        bool canDraw = brush->canDraw(nullptr, QPoint(10, 10));
        updateStatus(QString("Can draw at (10,10): %1").arg(canDraw));
        
        // Test draw stub (will output debug message)
        brush->draw(nullptr, nullptr, nullptr);
        
        // Test undraw stub (will output debug message)
        brush->undraw(nullptr, nullptr);
        
        updateStatus("Drawing stub tests completed (check debug output)");
    }

    void onTestPolymorphism() {
        updateStatus("=== Testing Polymorphic Behavior ===");
        
        // Create different types of terrain brushes
        auto groundBrush = std::make_unique<GroundBrush>(200, this);
        groundBrush->setSpecificName("Polymorphic Ground");
        
        auto wallBrush = std::make_unique<WallBrush>(this);
        wallBrush->setSpecificName("Polymorphic Wall");
        
        // Test polymorphic behavior through base class pointers
        QList<TerrainBrush*> terrainBrushes = {groundBrush.get(), wallBrush.get()};
        
        for (TerrainBrush* terrain : terrainBrushes) {
            updateStatus(QString("Terrain Brush: %1, Type: %2, NeedBorders: %3, CanDrag: %4")
                        .arg(terrain->name())
                        .arg(static_cast<int>(terrain->type()))
                        .arg(terrain->needBorders())
                        .arg(terrain->canDrag()));
            
            // Test type casting
            if (terrain->isGround()) {
                GroundBrush* ground = terrain->asGround();
                updateStatus(QString("  Successfully cast to GroundBrush: %1").arg(ground != nullptr));
            }
            
            if (terrain->isWall()) {
                WallBrush* wall = terrain->asWall();
                updateStatus(QString("  Successfully cast to WallBrush: %1").arg(wall != nullptr));
            }
            
            // Test terrain-specific casting
            TerrainBrush* terrainCast = terrain->asTerrain();
            updateStatus(QString("  TerrainBrush cast successful: %1").arg(terrainCast != nullptr));
        }
        
        // Store brushes
        testBrushes_.append(std::move(groundBrush));
        testBrushes_.append(std::move(wallBrush));
    }

    void onTestZOrder() {
        updateStatus("=== Testing Z-Order Management ===");
        
        auto brush = std::make_unique<GroundBrush>(300, this);
        brush->setSpecificName("Z-Order Test");
        
        // Test Z-order
        qint32 initialZ = brush->getZ();
        updateStatus(QString("Initial Z-order: %1").arg(initialZ));
        
        brush->setZ(zOrderSpin_->value());
        qint32 newZ = brush->getZ();
        updateStatus(QString("New Z-order: %1").arg(newZ));
        
        // Test with negative Z-order
        brush->setZ(-5);
        updateStatus(QString("Negative Z-order: %1").arg(brush->getZ()));
        
        testBrushes_.append(std::move(brush));
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        BrushManager manager;
        
        // Test terrain brush creation through manager
        auto terrainBrush = manager.createBrushShared(Brush::Type::Ground, {{"groundId", 400}});
        
        if (terrainBrush && terrainBrush->isTerrain()) {
            TerrainBrush* terrain = terrainBrush->asTerrain();
            if (terrain) {
                terrain->setSpecificName("Manager Created Terrain");
                terrain->setZ(10);
                
                updateStatus(QString("BrushManager created terrain brush: %1 (Z: %2)")
                            .arg(terrain->name()).arg(terrain->getZ()));
                
                // Test friend system through manager
                terrain->addFriendBrushType(Brush::Type::Wall);
                terrain->addFriendBrushType(Brush::Type::Ground);
                
                updateStatus(QString("Added friends - Ground: %1, Wall: %2")
                            .arg(terrain->isFriendWith(Brush::Type::Ground))
                            .arg(terrain->isFriendWith(Brush::Type::Wall)));
            }
        } else {
            updateStatus("Failed to create terrain brush through BrushManager");
        }
    }

private:
    void setupUI() {
        setWindowTitle("TerrainBrush Test Application - Task 37");
        setFixedSize(900, 700);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("TerrainBrush Implementation Test (Task 37)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Terrain Brush Configuration");
        QHBoxLayout* configLayout = new QHBoxLayout(configGroup);
        
        configLayout->addWidget(new QLabel("Name:"));
        brushNameEdit_ = new QLineEdit("Test Terrain");
        configLayout->addWidget(brushNameEdit_);
        
        configLayout->addWidget(new QLabel("Look ID:"));
        lookIdSpin_ = new QSpinBox();
        lookIdSpin_->setRange(0, 9999);
        lookIdSpin_->setValue(100);
        configLayout->addWidget(lookIdSpin_);
        
        configLayout->addWidget(new QLabel("Z-Order:"));
        zOrderSpin_ = new QSpinBox();
        zOrderSpin_->setRange(-100, 100);
        zOrderSpin_->setValue(0);
        configLayout->addWidget(zOrderSpin_);
        
        hatesFriendsCheck_ = new QCheckBox("Hates Friends");
        configLayout->addWidget(hatesFriendsCheck_);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Terrain Brush");
        QPushButton* friendBtn = new QPushButton("Test Friend System");
        QPushButton* drawBtn = new QPushButton("Test Drawing Stubs");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(friendBtn);
        buttonLayout1->addWidget(drawBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* polyBtn = new QPushButton("Test Polymorphism");
        QPushButton* zOrderBtn = new QPushButton("Test Z-Order");
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        
        buttonLayout2->addWidget(polyBtn);
        buttonLayout2->addWidget(zOrderBtn);
        buttonLayout2->addWidget(managerBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(350);
        statusText_->setReadOnly(true);
        mainLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(createBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onCreateTerrainBrush);
        connect(friendBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onTestFriendSystem);
        connect(drawBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onTestDrawingStubs);
        connect(polyBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onTestPolymorphism);
        connect(zOrderBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onTestZOrder);
        connect(managerBtn, &QPushButton::clicked, this, &TerrainBrushTestWidget::onTestBrushManager);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("TerrainBrush Test Application Started");
        updateStatus("This application tests the complete TerrainBrush base class implementation");
        updateStatus("as required by Task 37, including drawing stubs and core properties.");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "TerrainBrushTest:" << message;
    }
    
    void testTerrainBrushInterface(TerrainBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - Name: %1, SpecificName: %2, Type: %3")
                    .arg(brush->name())
                    .arg(brush->specificName())
                    .arg(static_cast<int>(brush->type())));
        
        updateStatus(QString("  Properties - NeedBorders: %1, CanDrag: %2, LookID: %3, Z: %4")
                    .arg(brush->needBorders())
                    .arg(brush->canDrag())
                    .arg(brush->getLookID())
                    .arg(brush->getZ()));
        
        updateStatus(QString("  Type checks - IsTerrain: %1, AsTerrainValid: %2")
                    .arg(brush->isTerrain())
                    .arg(brush->asTerrain() != nullptr));
    }
    
    QLineEdit* brushNameEdit_;
    QSpinBox* lookIdSpin_;
    QSpinBox* zOrderSpin_;
    QCheckBox* hatesFriendsCheck_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<TerrainBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    TerrainBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "TerrainBrushTest.moc"
