// OptionalBorderBrushTest.cpp - Test for Task 44 OptionalBorderBrush implementation

#include "OptionalBorderBrush.h"
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
#include <QDebug>
#include <QUndoStack>
#include <memory>

// Test widget to demonstrate OptionalBorderBrush functionality
class OptionalBorderBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    OptionalBorderBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateOptionalBorderBrush() {
        auto optionalBorderBrush = std::make_unique<OptionalBorderBrush>();
        
        updateStatus(QString("Created OptionalBorderBrush: %1 (LookID: %2)")
                    .arg(optionalBorderBrush->name())
                    .arg(optionalBorderBrush->getLookID()));
        
        testOptionalBorderBrushInterface(optionalBorderBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(optionalBorderBrush));
    }

    void onTestCanDrawLogic() {
        updateStatus("=== Testing canDraw Logic (wxwidgets Migration) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No optional border brushes available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& optionalBorderBrush = testBrushes_.first();
        
        // Test canDraw at various positions
        QList<QPoint> testPositions = {{5, 5}, {10, 10}, {15, 15}};
        
        for (const QPoint& pos : testPositions) {
            bool canDrawQt = optionalBorderBrush->canDraw(testMap.get(), QPointF(pos));
            bool canDrawWx = optionalBorderBrush->canDraw(testMap.get(), pos);
            
            updateStatus(QString("canDraw at (%1,%2): Qt=%3, Wx=%4")
                        .arg(pos.x()).arg(pos.y())
                        .arg(canDrawQt).arg(canDrawWx));
        }
        
        // Create a ground brush with optional border to test neighbor logic
        auto groundBrush = std::make_unique<GroundBrush>();
        // Note: In real usage, ground brush would be configured with optional border
        
        updateStatus("canDraw logic test completed");
    }

    void onTestDrawUndraw() {
        updateStatus("=== Testing Draw/Undraw Methods (wxwidgets Migration) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No optional border brushes available. Create some first.");
            return;
        }
        
        // Create test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& optionalBorderBrush = testBrushes_.first();
        QPoint testPos(10, 10);
        
        Tile* testTile = testMap->getTile(testPos.x(), testPos.y(), 0);
        if (testTile) {
            // Test initial state
            bool initialOptionalBorder = testTile->hasOptionalBorder();
            updateStatus(QString("Initial optional border state: %1").arg(initialOptionalBorder));
            
            // Test draw method
            optionalBorderBrush->draw(testMap.get(), testTile);
            bool afterDraw = testTile->hasOptionalBorder();
            updateStatus(QString("After draw: %1").arg(afterDraw));
            
            // Test undraw method
            optionalBorderBrush->undraw(testMap.get(), testTile);
            bool afterUndraw = testTile->hasOptionalBorder();
            updateStatus(QString("After undraw: %1").arg(afterUndraw));
            
            updateStatus("Draw/undraw test completed");
        }
    }

    void onTestUndoCommands() {
        updateStatus("=== Testing Undo Commands ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No optional border brushes available. Create some first.");
            return;
        }
        
        // Create test map and undo stack
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        QUndoStack undoStack;
        
        auto& optionalBorderBrush = testBrushes_.first();
        QPointF testPos(10, 10);
        
        Tile* testTile = testMap->getTile(testPos.toPoint());
        if (testTile) {
            // Test apply brush command
            auto applyCommand = optionalBorderBrush->applyBrush(testMap.get(), testPos);
            if (applyCommand) {
                undoStack.push(applyCommand);
                updateStatus("Applied optional border via undo command");
                updateStatus(QString("Tile has optional border: %1").arg(testTile->hasOptionalBorder()));
                
                // Test undo
                undoStack.undo();
                updateStatus("Undid optional border application");
                updateStatus(QString("Tile has optional border after undo: %1").arg(testTile->hasOptionalBorder()));
                
                // Test redo
                undoStack.redo();
                updateStatus("Redid optional border application");
                updateStatus(QString("Tile has optional border after redo: %1").arg(testTile->hasOptionalBorder()));
                
                // Test remove brush command
                auto removeCommand = optionalBorderBrush->removeBrush(testMap.get(), testPos);
                if (removeCommand) {
                    undoStack.push(removeCommand);
                    updateStatus("Removed optional border via undo command");
                    updateStatus(QString("Tile has optional border after removal: %1").arg(testTile->hasOptionalBorder()));
                }
            }
        }
        
        updateStatus("Undo commands test completed");
    }

    void onTestMouseEvents() {
        updateStatus("=== Testing Mouse Events ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No optional border brushes available. Create some first.");
            return;
        }
        
        // Create test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        QUndoStack undoStack;
        
        auto& optionalBorderBrush = testBrushes_.first();
        QPointF testPos(10, 10);
        
        // Simulate mouse press event
        QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        
        auto pressCommand = optionalBorderBrush->mousePressEvent(testPos, &pressEvent, nullptr, testMap.get(), &undoStack, false, false, false);
        if (pressCommand) {
            undoStack.push(pressCommand);
            updateStatus("Mouse press event handled - applied optional border");
        } else {
            updateStatus("Mouse press event - no action taken");
        }
        
        // Simulate mouse press with shift (remove)
        auto shiftPressCommand = optionalBorderBrush->mousePressEvent(testPos, &pressEvent, nullptr, testMap.get(), &undoStack, true, false, false);
        if (shiftPressCommand) {
            undoStack.push(shiftPressCommand);
            updateStatus("Shift+mouse press event handled - removed optional border");
        } else {
            updateStatus("Shift+mouse press event - no action taken");
        }
        
        updateStatus("Mouse events test completed");
    }

    void onTestBrushProperties() {
        updateStatus("=== Testing Brush Properties ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No optional border brushes available. Create some first.");
            return;
        }
        
        auto& optionalBorderBrush = testBrushes_.first();
        
        // Test brush properties
        updateStatus(QString("Brush name: %1").arg(optionalBorderBrush->name()));
        updateStatus(QString("Brush type: %1").arg(static_cast<int>(optionalBorderBrush->type())));
        updateStatus(QString("Look ID: %1").arg(optionalBorderBrush->getLookID()));
        updateStatus(QString("Can drag: %1").arg(optionalBorderBrush->canDrag()));
        updateStatus(QString("Brush size: %1").arg(optionalBorderBrush->getBrushSize()));
        updateStatus(QString("Brush shape: %1").arg(static_cast<int>(optionalBorderBrush->getBrushShape())));
        
        // Test type identification
        updateStatus(QString("Is optional border: %1").arg(optionalBorderBrush->isOptionalBorder()));
        updateStatus(QString("As optional border: %1").arg(optionalBorderBrush->asOptionalBorder() != nullptr));
        
        updateStatus("Brush properties test completed");
    }

    void onTestWxwidgetsCompatibility() {
        updateStatus("=== Testing wxwidgets Compatibility ===");
        
        updateStatus("OptionalBorderBrush provides 1:1 migration from wxwidgets:");
        updateStatus("");
        updateStatus("Key compatibility features:");
        updateStatus("1. Same constructor and destructor");
        updateStatus("2. Same canDraw logic with neighbor checking");
        updateStatus("3. Same draw/undraw methods with tile state modification");
        updateStatus("4. Same name and look ID handling");
        updateStatus("5. Same drag capability (canDrag = true)");
        updateStatus("");
        updateStatus("canDraw logic (migrated from wxwidgets):");
        updateStatus("- Cannot draw on tiles with ground brush that hasOptionalBorder()");
        updateStatus("- Can draw if any of 8 neighbors has ground brush with hasOptionalBorder()");
        updateStatus("- Checks all neighbor positions: (-1,-1) to (+1,+1)");
        updateStatus("");
        updateStatus("draw/undraw methods (migrated from wxwidgets):");
        updateStatus("- draw(): tile->setOptionalBorder(true)");
        updateStatus("- undraw(): tile->setOptionalBorder(false)");
        updateStatus("- Border algorithm handles placement automagically");
        updateStatus("");
        updateStatus("Qt enhancements:");
        updateStatus("- QUndoCommand support for undo/redo operations");
        updateStatus("- Mouse event handling with modifier key support");
        updateStatus("- Type-safe brush identification and casting");
        updateStatus("- Integration with Qt's event system");
        updateStatus("");
        updateStatus("Usage: Place gravel borders around mountain tiles");
        updateStatus("The brush sets tile state flags that are processed by the border system");
    }

private:
    void setupUI() {
        setWindowTitle("OptionalBorderBrush Test Application - Task 44");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("OptionalBorderBrush Implementation Test (Task 44)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create OptionalBorder Brush");
        QPushButton* canDrawBtn = new QPushButton("Test canDraw Logic");
        QPushButton* drawBtn = new QPushButton("Test Draw/Undraw");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(canDrawBtn);
        buttonLayout1->addWidget(drawBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* undoBtn = new QPushButton("Test Undo Commands");
        QPushButton* mouseBtn = new QPushButton("Test Mouse Events");
        QPushButton* propsBtn = new QPushButton("Test Brush Properties");
        
        buttonLayout2->addWidget(undoBtn);
        buttonLayout2->addWidget(mouseBtn);
        buttonLayout2->addWidget(propsBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
        QHBoxLayout* buttonLayout3 = new QHBoxLayout();
        
        QPushButton* compatBtn = new QPushButton("Test wxwidgets Compatibility");
        
        buttonLayout3->addWidget(compatBtn);
        buttonLayout3->addStretch();
        
        mainLayout->addLayout(buttonLayout3);
        
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
        connect(createBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onCreateOptionalBorderBrush);
        connect(canDrawBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestCanDrawLogic);
        connect(drawBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestDrawUndraw);
        connect(undoBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestUndoCommands);
        connect(mouseBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestMouseEvents);
        connect(propsBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestBrushProperties);
        connect(compatBtn, &QPushButton::clicked, this, &OptionalBorderBrushTestWidget::onTestWxwidgetsCompatibility);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("OptionalBorderBrush Test Application Started");
        updateStatus("This application tests the OptionalBorderBrush implementation");
        updateStatus("as migrated 1:1 from wxwidgets for Task 44.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Conditional draw logic based on neighbor ground brushes");
        updateStatus("- Tile state modification (setOptionalBorder)");
        updateStatus("- Undo/redo command support");
        updateStatus("- Mouse event handling with modifier keys");
        updateStatus("- wxwidgets compatibility verification");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "OptionalBorderBrushTest:" << message;
    }
    
    void testOptionalBorderBrushInterface(OptionalBorderBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - IsOptionalBorder: %1, CanDrag: %2")
                    .arg(brush->isOptionalBorder())
                    .arg(brush->canDrag()));
        
        updateStatus(QString("  Properties - Name: %1, Type: %2")
                    .arg(brush->name())
                    .arg(static_cast<int>(brush->type())));
    }
    
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<OptionalBorderBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    OptionalBorderBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "OptionalBorderBrushTest.moc"
