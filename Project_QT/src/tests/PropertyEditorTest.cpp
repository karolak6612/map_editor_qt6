// PropertyEditorTest.cpp - Test for Task 45 Property Editor implementation

#include "ui/properties/ItemPropertyEditorBase.h"
#include "ui/properties/ItemPropertyEditor.h"
#include "ui/properties/OldPropertiesWindow.h"
#include "ui/TilePropertyEditor.h"
#include "Item.h"
#include "Tile.h"
#include "Map.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QDebug>
#include <memory>

// Test widget to demonstrate Property Editor functionality
class PropertyEditorTestWidget : public QWidget {
    Q_OBJECT

public:
    PropertyEditorTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateTestItem() {
        quint16 itemId = itemIdSpin_->value();
        
        // Create a test item
        auto testItem = std::make_unique<Item>(itemId);
        testItem->setName(QString("Test Item %1").arg(itemId));
        
        updateStatus(QString("Created test item: ID=%1, Name=%2")
                    .arg(testItem->getServerId())
                    .arg(testItem->name()));
        
        // Store for later use
        testItems_.append(std::move(testItem));
        
        // Update the item property editor
        if (!testItems_.isEmpty()) {
            itemPropertyEditor_->setEditingObject(testItems_.last().get());
        }
    }

    void onCreateTestTile() {
        quint32 x = xPosSpin_->value();
        quint32 y = yPosSpin_->value();
        quint32 z = zPosSpin_->value();
        
        // Create a test tile
        auto testTile = std::make_unique<Tile>(x, y, z);
        testTile->setHouseId(houseIdSpin_->value());
        testTile->setPZ(pzCheck_->isChecked());
        testTile->setNoPVP(noPvpCheck_->isChecked());
        
        updateStatus(QString("Created test tile: Pos=(%1,%2,%3), HouseID=%4")
                    .arg(x).arg(y).arg(z)
                    .arg(testTile->getHouseId()));
        
        // Store for later use
        testTiles_.append(std::move(testTile));
        
        // Update the tile property editor
        if (!testTiles_.isEmpty()) {
            tilePropertyEditor_->displayTileProperties(testTiles_.last().get());
        }
    }

    void onTestItemPropertyEditor() {
        updateStatus("=== Testing ItemPropertyEditor ===");
        
        if (testItems_.isEmpty()) {
            updateStatus("No test items available. Create some first.");
            return;
        }
        
        auto& testItem = testItems_.first();
        
        // Test setEditingObject
        itemPropertyEditor_->setEditingObject(testItem.get());
        updateStatus("Set editing object on ItemPropertyEditor");
        
        // Test property manipulation
        itemPropertyEditor_->setProperty("name", "Modified Test Item");
        itemPropertyEditor_->setProperty("id", 9999);
        
        QVariant nameProperty = itemPropertyEditor_->getProperty("name");
        updateStatus(QString("Retrieved property 'name': %1").arg(nameProperty.toString()));
        
        // Test change tracking
        bool hasPendingChanges = itemPropertyEditor_->hasPendingChanges();
        updateStatus(QString("Has pending changes: %1").arg(hasPendingChanges));
        
        // Test validation
        bool isValid = itemPropertyEditor_->validateInput();
        updateStatus(QString("Input validation: %1").arg(isValid));
        
        updateStatus("ItemPropertyEditor test completed");
    }

    void onTestOldPropertiesWindow() {
        updateStatus("=== Testing OldPropertiesWindow ===");
        
        if (testItems_.isEmpty()) {
            updateStatus("No test items available. Create some first.");
            return;
        }
        
        auto& testItem = testItems_.first();
        
        // Test setEditingObject
        oldPropertiesWindow_->setEditingObject(testItem.get());
        updateStatus("Set editing object on OldPropertiesWindow");
        
        // Test property manipulation
        oldPropertiesWindow_->setProperty("description", "Old Properties Test");
        oldPropertiesWindow_->setProperty("category", "Test Category");
        
        QVariant descProperty = oldPropertiesWindow_->getProperty("description");
        updateStatus(QString("Retrieved property 'description': %1").arg(descProperty.toString()));
        
        // Test change tracking
        bool hasPendingChanges = oldPropertiesWindow_->hasPendingChanges();
        updateStatus(QString("Has pending changes: %1").arg(hasPendingChanges));
        
        updateStatus("OldPropertiesWindow test completed");
    }

    void onTestTilePropertyEditor() {
        updateStatus("=== Testing TilePropertyEditor ===");
        
        if (testTiles_.isEmpty()) {
            updateStatus("No test tiles available. Create some first.");
            return;
        }
        
        auto& testTile = testTiles_.first();
        
        // Test displayTileProperties
        tilePropertyEditor_->displayTileProperties(testTile.get());
        updateStatus("Displayed tile properties on TilePropertyEditor");
        
        // Test with null tile
        tilePropertyEditor_->displayTileProperties(nullptr);
        updateStatus("Tested TilePropertyEditor with null tile");
        
        // Restore tile display
        tilePropertyEditor_->displayTileProperties(testTile.get());
        updateStatus("Restored tile display");
        
        updateStatus("TilePropertyEditor test completed");
    }

    void onTestPropertyEditorSignals() {
        updateStatus("=== Testing Property Editor Signals ===");
        
        if (testItems_.isEmpty()) {
            updateStatus("No test items available. Create some first.");
            return;
        }
        
        auto& testItem = testItems_.first();
        
        // Connect to signals for testing
        connect(itemPropertyEditor_, &ItemPropertyEditorBase::propertyChanged,
                this, [this](const QString& name, const QVariant& value) {
                    updateStatus(QString("Signal: Property changed - %1 = %2").arg(name).arg(value.toString()));
                });
        
        connect(itemPropertyEditor_, &ItemPropertyEditorBase::editingObjectChanged,
                this, [this](QObject* object) {
                    updateStatus(QString("Signal: Editing object changed - %1").arg(object ? "Valid object" : "nullptr"));
                });
        
        connect(itemPropertyEditor_, &ItemPropertyEditorBase::pendingChangesChanged,
                this, [this](bool hasPendingChanges) {
                    updateStatus(QString("Signal: Pending changes changed - %1").arg(hasPendingChanges));
                });
        
        // Trigger signals
        itemPropertyEditor_->setEditingObject(testItem.get());
        itemPropertyEditor_->setProperty("test_signal", "Signal Test Value");
        itemPropertyEditor_->markAsModified(true);
        
        updateStatus("Property editor signals test completed");
    }

    void onTestPropertyEditorSlots() {
        updateStatus("=== Testing Property Editor Slots ===");
        
        if (testItems_.isEmpty()) {
            updateStatus("No test items available. Create some first.");
            return;
        }
        
        auto& testItem = testItems_.first();
        itemPropertyEditor_->setEditingObject(testItem.get());
        
        // Test applyChanges slot
        itemPropertyEditor_->setProperty("slot_test", "Apply Test");
        itemPropertyEditor_->applyChanges();
        updateStatus("Called applyChanges slot");
        
        // Test discardChanges slot
        itemPropertyEditor_->setProperty("slot_test", "Discard Test");
        itemPropertyEditor_->discardChanges();
        updateStatus("Called discardChanges slot");
        
        // Test refreshFromObject slot
        itemPropertyEditor_->refreshFromObject();
        updateStatus("Called refreshFromObject slot");
        
        updateStatus("Property editor slots test completed");
    }

    void onTestAbstractInterface() {
        updateStatus("=== Testing Abstract Interface Compliance ===");
        
        updateStatus("ItemPropertyEditorBase provides abstract interface for:");
        updateStatus("1. setEditingObject(QObject* object) - Pure virtual");
        updateStatus("2. loadPropertiesFromObject() - Pure virtual");
        updateStatus("3. savePropertiesToObject() - Pure virtual");
        updateStatus("4. hasPendingChanges() const - Virtual with default");
        updateStatus("5. setProperty(key, value) - Virtual with default");
        updateStatus("6. getProperty(key) - Virtual with default");
        updateStatus("7. validateInput() - Virtual with default");
        updateStatus("8. resetToOriginalValues() - Virtual with default");
        updateStatus("9. markAsModified(bool) - Virtual with default");
        updateStatus("");
        updateStatus("Signals provided:");
        updateStatus("- propertyChanged(QString, QVariant)");
        updateStatus("- editingObjectChanged(QObject*)");
        updateStatus("- pendingChangesChanged(bool)");
        updateStatus("");
        updateStatus("Slots provided:");
        updateStatus("- applyChanges()");
        updateStatus("- discardChanges()");
        updateStatus("- refreshFromObject()");
        updateStatus("");
        updateStatus("Subclasses implemented:");
        updateStatus("- ItemPropertyEditor (for Item objects)");
        updateStatus("- OldPropertiesWindow (generic property editor)");
        updateStatus("- TilePropertyEditor (specialized for Tile objects)");
        updateStatus("");
        updateStatus("Abstract interface compliance test completed");
    }

private:
    void setupUI() {
        setWindowTitle("Property Editor Test Application - Task 45");
        setFixedSize(1200, 900);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        
        // Left side - Controls
        QWidget* controlsWidget = new QWidget();
        controlsWidget->setFixedWidth(400);
        QVBoxLayout* controlsLayout = new QVBoxLayout(controlsWidget);
        
        // Title
        QLabel* titleLabel = new QLabel("Property Editor Implementation Test (Task 45)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        controlsLayout->addWidget(titleLabel);
        
        // Item creation group
        QGroupBox* itemGroup = new QGroupBox("Test Item Creation");
        QVBoxLayout* itemLayout = new QVBoxLayout(itemGroup);
        
        QHBoxLayout* itemIdLayout = new QHBoxLayout();
        itemIdLayout->addWidget(new QLabel("Item ID:"));
        itemIdSpin_ = new QSpinBox();
        itemIdSpin_->setRange(1, 9999);
        itemIdSpin_->setValue(1000);
        itemIdLayout->addWidget(itemIdSpin_);
        itemLayout->addLayout(itemIdLayout);
        
        QPushButton* createItemBtn = new QPushButton("Create Test Item");
        itemLayout->addWidget(createItemBtn);
        
        controlsLayout->addWidget(itemGroup);
        
        // Tile creation group
        QGroupBox* tileGroup = new QGroupBox("Test Tile Creation");
        QVBoxLayout* tileLayout = new QVBoxLayout(tileGroup);
        
        QHBoxLayout* posLayout = new QHBoxLayout();
        posLayout->addWidget(new QLabel("X:"));
        xPosSpin_ = new QSpinBox();
        xPosSpin_->setRange(0, 999);
        xPosSpin_->setValue(10);
        posLayout->addWidget(xPosSpin_);
        
        posLayout->addWidget(new QLabel("Y:"));
        yPosSpin_ = new QSpinBox();
        yPosSpin_->setRange(0, 999);
        yPosSpin_->setValue(10);
        posLayout->addWidget(yPosSpin_);
        
        posLayout->addWidget(new QLabel("Z:"));
        zPosSpin_ = new QSpinBox();
        zPosSpin_->setRange(0, 15);
        zPosSpin_->setValue(7);
        posLayout->addWidget(zPosSpin_);
        tileLayout->addLayout(posLayout);
        
        QHBoxLayout* houseLayout = new QHBoxLayout();
        houseLayout->addWidget(new QLabel("House ID:"));
        houseIdSpin_ = new QSpinBox();
        houseIdSpin_->setRange(0, 9999);
        houseIdSpin_->setValue(0);
        houseLayout->addWidget(houseIdSpin_);
        tileLayout->addLayout(houseLayout);
        
        pzCheck_ = new QCheckBox("Protection Zone");
        noPvpCheck_ = new QCheckBox("No PVP");
        tileLayout->addWidget(pzCheck_);
        tileLayout->addWidget(noPvpCheck_);
        
        QPushButton* createTileBtn = new QPushButton("Create Test Tile");
        tileLayout->addWidget(createTileBtn);
        
        controlsLayout->addWidget(tileGroup);
        
        // Test buttons
        QGroupBox* testGroup = new QGroupBox("Property Editor Tests");
        QVBoxLayout* testLayout = new QVBoxLayout(testGroup);
        
        QPushButton* testItemBtn = new QPushButton("Test ItemPropertyEditor");
        QPushButton* testOldBtn = new QPushButton("Test OldPropertiesWindow");
        QPushButton* testTileBtn = new QPushButton("Test TilePropertyEditor");
        QPushButton* testSignalsBtn = new QPushButton("Test Signals");
        QPushButton* testSlotsBtn = new QPushButton("Test Slots");
        QPushButton* testInterfaceBtn = new QPushButton("Test Abstract Interface");
        
        testLayout->addWidget(testItemBtn);
        testLayout->addWidget(testOldBtn);
        testLayout->addWidget(testTileBtn);
        testLayout->addWidget(testSignalsBtn);
        testLayout->addWidget(testSlotsBtn);
        testLayout->addWidget(testInterfaceBtn);
        
        controlsLayout->addWidget(testGroup);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(200);
        statusText_->setReadOnly(true);
        controlsLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        controlsLayout->addWidget(exitBtn);
        
        mainLayout->addWidget(controlsWidget);
        
        // Right side - Property Editors
        QTabWidget* editorsTab = new QTabWidget();
        
        // Create property editors
        itemPropertyEditor_ = new ItemPropertyEditor();
        oldPropertiesWindow_ = new OldPropertiesWindow();
        tilePropertyEditor_ = new TilePropertyEditor();
        
        editorsTab->addTab(itemPropertyEditor_, "Item Property Editor");
        editorsTab->addTab(oldPropertiesWindow_, "Old Properties Window");
        editorsTab->addTab(tilePropertyEditor_, "Tile Property Editor");
        
        mainLayout->addWidget(editorsTab);
        
        // Connect buttons
        connect(createItemBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onCreateTestItem);
        connect(createTileBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onCreateTestTile);
        connect(testItemBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestItemPropertyEditor);
        connect(testOldBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestOldPropertiesWindow);
        connect(testTileBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestTilePropertyEditor);
        connect(testSignalsBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestPropertyEditorSignals);
        connect(testSlotsBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestPropertyEditorSlots);
        connect(testInterfaceBtn, &QPushButton::clicked, this, &PropertyEditorTestWidget::onTestAbstractInterface);
    }
    
    void connectSignals() {
        // Additional signal connections will be made during testing
    }
    
    void runTests() {
        updateStatus("Property Editor Test Application Started");
        updateStatus("This application tests the Property Editor implementation");
        updateStatus("for Task 45 - Abstract Editor UI Structure.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- ItemPropertyEditorBase abstract interface");
        updateStatus("- ItemPropertyEditor for Item objects");
        updateStatus("- OldPropertiesWindow generic editor");
        updateStatus("- TilePropertyEditor for Tile objects");
        updateStatus("- Property manipulation and change tracking");
        updateStatus("- Signals and slots for editor events");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "PropertyEditorTest:" << message;
    }
    
    // UI Controls
    QSpinBox* itemIdSpin_;
    QSpinBox* xPosSpin_;
    QSpinBox* yPosSpin_;
    QSpinBox* zPosSpin_;
    QSpinBox* houseIdSpin_;
    QCheckBox* pzCheck_;
    QCheckBox* noPvpCheck_;
    QTextEdit* statusText_;
    
    // Property Editors
    ItemPropertyEditor* itemPropertyEditor_;
    OldPropertiesWindow* oldPropertiesWindow_;
    TilePropertyEditor* tilePropertyEditor_;
    
    // Test Objects
    QList<std::unique_ptr<Item>> testItems_;
    QList<std::unique_ptr<Tile>> testTiles_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    PropertyEditorTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "PropertyEditorTest.moc"
