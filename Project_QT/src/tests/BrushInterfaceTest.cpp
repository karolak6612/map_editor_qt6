// BrushInterfaceTest.cpp - Comprehensive test for Task 36 Brush interface and smart pointers

#include "Brush.h"
#include "BrushManager.h"
#include "DoorBrush.h"
#include "PixelBrush.h"
#include "GroundBrush.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDebug>
#include <QTimer>
#include <memory>

// Test widget to demonstrate Brush interface and smart pointer usage
class BrushInterfaceTestWidget : public QWidget {
    Q_OBJECT

public:
    BrushInterfaceTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateRawPointer() {
        Brush::Type type = static_cast<Brush::Type>(brushTypeCombo_->currentData().toInt());
        
        QVariantMap parameters;
        if (type == Brush::Type::Ground) {
            parameters["groundId"] = groundIdSpin_->value();
        } else if (type == Brush::Type::Door) {
            parameters["doorType"] = static_cast<int>(DoorType::Normal);
        } else if (type == Brush::Type::Pixel) {
            parameters["color"] = QColor(Qt::red);
        }
        
        Brush* brush = brushManager_.createBrush(type, parameters);
        if (brush) {
            updateStatus(QString("Created raw pointer brush: %1 (ID: %2)")
                        .arg(brush->name()).arg(brush->getID()));
            testBrushInterface(brush);
        } else {
            updateStatus("Failed to create raw pointer brush");
        }
    }

    void onCreateSharedPointer() {
        Brush::Type type = static_cast<Brush::Type>(brushTypeCombo_->currentData().toInt());
        
        QVariantMap parameters;
        parameters["size"] = brushSizeSpin_->value();
        
        auto sharedBrush = brushManager_.createBrushShared(type, parameters);
        if (sharedBrush) {
            updateStatus(QString("Created shared_ptr brush: %1 (ID: %2, use_count: %3)")
                        .arg(sharedBrush->name()).arg(sharedBrush->getID()).arg(sharedBrush.use_count()));
            testBrushInterface(sharedBrush.get());
            
            // Test shared ownership
            auto sharedCopy = sharedBrush;
            updateStatus(QString("Shared ownership test - use_count: %1").arg(sharedBrush.use_count()));
        } else {
            updateStatus("Failed to create shared_ptr brush");
        }
    }

    void onCreateUniquePointer() {
        Brush::Type type = static_cast<Brush::Type>(brushTypeCombo_->currentData().toInt());
        
        QVariantMap parameters;
        parameters["size"] = brushSizeSpin_->value();
        
        auto uniqueBrush = brushManager_.createBrushUnique(type, parameters);
        if (uniqueBrush) {
            updateStatus(QString("Created unique_ptr brush: %1 (ID: %2)")
                        .arg(uniqueBrush->name()).arg(uniqueBrush->getID()));
            testBrushInterface(uniqueBrush.get());
            
            // Test unique ownership transfer
            auto movedBrush = std::move(uniqueBrush);
            updateStatus(QString("Unique ownership transfer test - original is null: %1, moved is valid: %2")
                        .arg(uniqueBrush == nullptr).arg(movedBrush != nullptr));
        } else {
            updateStatus("Failed to create unique_ptr brush");
        }
    }

    void onTestPolymorphism() {
        updateStatus("=== Testing Polymorphic Interface ===");
        
        // Create different brush types
        auto groundBrush = brushManager_.createBrushShared(Brush::Type::Ground, {{"groundId", 100}});
        auto doorBrush = brushManager_.createBrushShared(Brush::Type::Door, {{"doorType", static_cast<int>(DoorType::Magic)}});
        auto pixelBrush = brushManager_.createBrushShared(Brush::Type::Pixel, {{"color", QColor(Qt::blue)}});
        
        QList<std::shared_ptr<Brush>> brushes = {groundBrush, doorBrush, pixelBrush};
        
        for (auto brush : brushes) {
            if (brush) {
                updateStatus(QString("Brush: %1, Type: %2, LookID: %3, CanDrag: %4, NeedBorders: %5")
                            .arg(brush->name())
                            .arg(static_cast<int>(brush->type()))
                            .arg(brush->getLookID())
                            .arg(brush->canDrag())
                            .arg(brush->needBorders()));
                
                // Test type casting
                testTypeCasting(brush.get());
            }
        }
    }

    void onTestBrushProperties() {
        updateStatus("=== Testing Brush Properties ===");
        
        auto brush = brushManager_.createBrushShared(Brush::Type::Ground, {{"groundId", 150}});
        if (brush) {
            // Test visibility and collection properties
            updateStatus(QString("Initial visibility: %1, uses collection: %2")
                        .arg(brush->isVisibleInPalette()).arg(brush->usesCollection()));
            
            brush->setVisibleInPalette(false);
            brush->setCollection();
            
            updateStatus(QString("After changes - visibility: %1, uses collection: %2")
                        .arg(brush->isVisibleInPalette()).arg(brush->usesCollection()));
            
            // Test brush state management
            brush->resetBrushState();
            updateStatus("Brush state reset completed");
        }
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        // Test brush creation and management
        auto brush1 = brushManager_.createBrushShared(Brush::Type::Ground, {{"groundId", 200}});
        auto brush2 = brushManager_.createBrushShared(Brush::Type::Door, {{"doorType", static_cast<int>(DoorType::Quest)}});
        
        if (brush1 && brush2) {
            // Test current brush management
            brushManager_.setCurrentBrushShared(brush1);
            updateStatus(QString("Set current brush to: %1").arg(brushManager_.getCurrentBrush()->name()));
            
            brushManager_.setCurrentBrushShared(brush2);
            updateStatus(QString("Changed current brush to: %1").arg(brushManager_.getCurrentBrush()->name()));
            
            // Test brush enumeration
            QStringList availableTypes = brushManager_.getAvailableBrushTypes();
            updateStatus(QString("Available brush types: %1").arg(availableTypes.join(", ")));
            
            QStringList groundBrushes = brushManager_.getBrushesOfType(Brush::Type::Ground);
            updateStatus(QString("Ground brushes: %1").arg(groundBrushes.join(", ")));
        }
    }

private:
    void setupUI() {
        setWindowTitle("Brush Interface Test Application - Task 36");
        setFixedSize(800, 600);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("Brush Interface & Smart Pointer Test (Task 36)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Brush creation controls
        QHBoxLayout* creationLayout = new QHBoxLayout();
        
        creationLayout->addWidget(new QLabel("Type:"));
        brushTypeCombo_ = new QComboBox();
        brushTypeCombo_->addItem("Ground", static_cast<int>(Brush::Type::Ground));
        brushTypeCombo_->addItem("Door", static_cast<int>(Brush::Type::Door));
        brushTypeCombo_->addItem("Pixel", static_cast<int>(Brush::Type::Pixel));
        creationLayout->addWidget(brushTypeCombo_);
        
        creationLayout->addWidget(new QLabel("Ground ID:"));
        groundIdSpin_ = new QSpinBox();
        groundIdSpin_->setRange(100, 999);
        groundIdSpin_->setValue(100);
        creationLayout->addWidget(groundIdSpin_);
        
        creationLayout->addWidget(new QLabel("Size:"));
        brushSizeSpin_ = new QSpinBox();
        brushSizeSpin_->setRange(1, 10);
        brushSizeSpin_->setValue(1);
        creationLayout->addWidget(brushSizeSpin_);
        
        mainLayout->addLayout(creationLayout);
        
        // Smart pointer test buttons
        QHBoxLayout* pointerLayout = new QHBoxLayout();
        
        QPushButton* rawPtrBtn = new QPushButton("Create Raw Pointer");
        QPushButton* sharedPtrBtn = new QPushButton("Create Shared Pointer");
        QPushButton* uniquePtrBtn = new QPushButton("Create Unique Pointer");
        
        pointerLayout->addWidget(rawPtrBtn);
        pointerLayout->addWidget(sharedPtrBtn);
        pointerLayout->addWidget(uniquePtrBtn);
        
        mainLayout->addLayout(pointerLayout);
        
        // Interface test buttons
        QHBoxLayout* testLayout = new QHBoxLayout();
        
        QPushButton* polymorphismBtn = new QPushButton("Test Polymorphism");
        QPushButton* propertiesBtn = new QPushButton("Test Properties");
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        
        testLayout->addWidget(polymorphismBtn);
        testLayout->addWidget(propertiesBtn);
        testLayout->addWidget(managerBtn);
        
        mainLayout->addLayout(testLayout);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(300);
        statusText_->setReadOnly(true);
        mainLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(rawPtrBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onCreateRawPointer);
        connect(sharedPtrBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onCreateSharedPointer);
        connect(uniquePtrBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onCreateUniquePointer);
        connect(polymorphismBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onTestPolymorphism);
        connect(propertiesBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onTestBrushProperties);
        connect(managerBtn, &QPushButton::clicked, this, &BrushInterfaceTestWidget::onTestBrushManager);
    }
    
    void connectSignals() {
        // Connect BrushManager signals
        connect(&brushManager_, &BrushManager::currentBrushChanged, 
                [this](Brush* newBrush, Brush* previousBrush) {
            QString newName = newBrush ? newBrush->name() : "None";
            QString prevName = previousBrush ? previousBrush->name() : "None";
            updateStatus(QString("SIGNAL: Current brush changed: %1 -> %2").arg(prevName).arg(newName));
        });
        
        connect(&brushManager_, &BrushManager::brushCreated,
                [this](Brush* brush) {
            updateStatus(QString("SIGNAL: Brush created: %1").arg(brush ? brush->name() : "Unknown"));
        });
    }
    
    void runTests() {
        updateStatus("Brush Interface Test Application Started");
        updateStatus("This application tests the complete Brush abstract base class interface");
        updateStatus("and smart pointer integration as required by Task 36.");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "BrushInterfaceTest:" << message;
    }
    
    void testBrushInterface(Brush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - Name: %1, Type: %2, LookID: %3")
                    .arg(brush->name())
                    .arg(static_cast<int>(brush->type()))
                    .arg(brush->getLookID()));
        
        updateStatus(QString("  Properties - CanDrag: %1, CanSmear: %2, NeedBorders: %3, OneSizeFitsAll: %4")
                    .arg(brush->canDrag())
                    .arg(brush->canSmear())
                    .arg(brush->needBorders())
                    .arg(brush->oneSizeFitsAll()));
    }
    
    void testTypeCasting(Brush* brush) {
        if (!brush) return;
        
        QString castResults;
        if (brush->isGround() && brush->asGround()) {
            castResults += "Ground ";
        }
        if (brush->isDoor() && brush->asDoor()) {
            castResults += "Door ";
        }
        if (brush->isPixel()) {
            castResults += "Pixel ";
        }
        
        updateStatus(QString("  Type casting successful for: %1").arg(castResults.isEmpty() ? "None" : castResults));
    }
    
    BrushManager brushManager_;
    QComboBox* brushTypeCombo_;
    QSpinBox* groundIdSpin_;
    QSpinBox* brushSizeSpin_;
    QTextEdit* statusText_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    BrushInterfaceTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "BrushInterfaceTest.moc"
