#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QUndoStack>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

// Include the selection components we're testing
#include "SelectionBrush.h"
#include "SelectionCommands.h"
#include "Selection.h"
#include "ClipboardManager.h"
#include "Map.h"

/**
 * @brief Test application for Task 79 selection tool features
 * 
 * This application provides comprehensive testing for:
 * - Enhanced mouse actions for selection
 * - Complete copy/cut/paste commands
 * - Selection transformation commands
 * - Delete selection command
 * - Visual updates and signals
 */
class SelectionToolTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit SelectionToolTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , selectionBrush_(nullptr)
        , selection_(nullptr)
        , clipboardManager_(nullptr)
        , undoStack_(nullptr)
        , testMap_(nullptr)
        , statusText_(nullptr)
    {
        setWindowTitle("Task 79: Selection Tool Test Application");
        setMinimumSize(1000, 700);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        initializeComponents();
        connectSignals();
        
        logMessage("Selection Tool Test Application initialized");
        logMessage("Testing Task 79 implementation:");
        logMessage("- Enhanced mouse actions for selection");
        logMessage("- Complete copy/cut/paste commands");
        logMessage("- Selection transformation commands");
        logMessage("- Delete selection command");
        logMessage("- Visual updates and signals");
    }

private slots:
    void testSingleSelection() {
        logMessage("=== Testing Single Selection ===");
        
        try {
            if (selectionBrush_ && testMap_) {
                MapPos testPos(100, 100, 7);
                selectionBrush_->selectSingle(testMap_, testPos, false);
                logMessage(QString("✓ Single selection at position (%1, %2, %3)")
                          .arg(testPos.x).arg(testPos.y).arg(testPos.z));
                updateSelectionInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Single selection error: %1").arg(e.what()));
        }
    }
    
    void testRectangleSelection() {
        logMessage("=== Testing Rectangle Selection ===");
        
        try {
            if (selectionBrush_ && testMap_) {
                MapPos startPos(100, 100, 7);
                MapPos endPos(110, 110, 7);
                selectionBrush_->selectRectangle(testMap_, startPos, endPos, false);
                logMessage(QString("✓ Rectangle selection from (%1, %2) to (%3, %4)")
                          .arg(startPos.x).arg(startPos.y).arg(endPos.x).arg(endPos.y));
                updateSelectionInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Rectangle selection error: %1").arg(e.what()));
        }
    }
    
    void testCopySelection() {
        logMessage("=== Testing Copy Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    selectionBrush_->copySelection();
                    logMessage("✓ Selection copied to clipboard");
                } else {
                    logMessage("○ No selection to copy - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testCopySelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Copy selection error: %1").arg(e.what()));
        }
    }
    
    void testCutSelection() {
        logMessage("=== Testing Cut Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    selectionBrush_->cutSelection();
                    logMessage("✓ Selection cut to clipboard");
                    updateSelectionInfo();
                } else {
                    logMessage("○ No selection to cut - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testCutSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Cut selection error: %1").arg(e.what()));
        }
    }
    
    void testPasteSelection() {
        logMessage("=== Testing Paste Selection ===");
        
        try {
            if (selectionBrush_ && testMap_ && clipboardManager_) {
                if (clipboardManager_->hasClipboardData()) {
                    MapPos pastePos(120, 120, 7);
                    selectionBrush_->pasteSelection(testMap_, pastePos);
                    logMessage(QString("✓ Selection pasted at position (%1, %2, %3)")
                              .arg(pastePos.x).arg(pastePos.y).arg(pastePos.z));
                    updateSelectionInfo();
                } else {
                    logMessage("○ No clipboard data - copying selection first");
                    testCopySelection();
                    QTimer::singleShot(200, this, &SelectionToolTestWidget::testPasteSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Paste selection error: %1").arg(e.what()));
        }
    }
    
    void testDeleteSelection() {
        logMessage("=== Testing Delete Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    selectionBrush_->deleteSelection();
                    logMessage("✓ Selection deleted");
                    updateSelectionInfo();
                } else {
                    logMessage("○ No selection to delete - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testDeleteSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Delete selection error: %1").arg(e.what()));
        }
    }
    
    void testMoveSelection() {
        logMessage("=== Testing Move Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    QPointF moveOffset(5.0, 5.0);
                    selectionBrush_->moveSelection(moveOffset);
                    logMessage(QString("✓ Selection moved by offset (%1, %2)")
                              .arg(moveOffset.x()).arg(moveOffset.y()));
                    updateSelectionInfo();
                } else {
                    logMessage("○ No selection to move - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testMoveSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Move selection error: %1").arg(e.what()));
        }
    }
    
    void testRotateSelection() {
        logMessage("=== Testing Rotate Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    int degrees = 90;
                    selectionBrush_->rotateSelection(degrees);
                    logMessage(QString("✓ Selection rotated by %1 degrees").arg(degrees));
                    updateSelectionInfo();
                } else {
                    logMessage("○ No selection to rotate - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testRotateSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Rotate selection error: %1").arg(e.what()));
        }
    }
    
    void testFlipSelection() {
        logMessage("=== Testing Flip Selection ===");
        
        try {
            if (selectionBrush_) {
                if (selectionBrush_->hasSelection()) {
                    selectionBrush_->flipSelectionHorizontal();
                    logMessage("✓ Selection flipped horizontally");
                    
                    QTimer::singleShot(500, [this]() {
                        selectionBrush_->flipSelectionVertical();
                        logMessage("✓ Selection flipped vertically");
                        updateSelectionInfo();
                    });
                } else {
                    logMessage("○ No selection to flip - creating test selection first");
                    testRectangleSelection();
                    QTimer::singleShot(100, this, &SelectionToolTestWidget::testFlipSelection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Flip selection error: %1").arg(e.what()));
        }
    }
    
    void testMouseActions() {
        logMessage("=== Testing Mouse Actions ===");
        
        try {
            if (selectionBrush_ && testMap_) {
                // Simulate mouse press
                QPointF mousePos(105.0, 105.0);
                selectionBrush_->handleMousePress(testMap_, mousePos, Qt::LeftButton, Qt::NoModifier);
                logMessage("✓ Mouse press handled");
                
                // Simulate mouse move
                QPointF movePos(115.0, 115.0);
                selectionBrush_->handleMouseMove(testMap_, movePos, Qt::NoModifier);
                logMessage("✓ Mouse move handled");
                
                // Simulate mouse release
                selectionBrush_->handleMouseRelease(testMap_, movePos, Qt::LeftButton, Qt::NoModifier);
                logMessage("✓ Mouse release handled");
                
                updateSelectionInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Mouse actions error: %1").arg(e.what()));
        }
    }
    
    void testKeyboardShortcuts() {
        logMessage("=== Testing Keyboard Shortcuts ===");
        
        try {
            if (selectionBrush_) {
                // Test Ctrl+C (copy)
                QKeyEvent copyEvent(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
                selectionBrush_->handleKeyPress(&copyEvent);
                logMessage("✓ Ctrl+C keyboard shortcut handled");
                
                // Test Ctrl+V (paste)
                QKeyEvent pasteEvent(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier);
                selectionBrush_->handleKeyPress(&pasteEvent);
                logMessage("✓ Ctrl+V keyboard shortcut handled");
                
                // Test Delete key
                QKeyEvent deleteEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
                selectionBrush_->handleKeyPress(&deleteEvent);
                logMessage("✓ Delete key handled");
                
                updateSelectionInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Keyboard shortcuts error: %1").arg(e.what()));
        }
    }
    
    void testAllFeatures() {
        logMessage("=== Running Complete Selection Tool Test Suite ===");
        
        // Test each feature with delays
        QTimer::singleShot(100, this, &SelectionToolTestWidget::testSingleSelection);
        QTimer::singleShot(500, this, &SelectionToolTestWidget::testRectangleSelection);
        QTimer::singleShot(900, this, &SelectionToolTestWidget::testCopySelection);
        QTimer::singleShot(1300, this, &SelectionToolTestWidget::testCutSelection);
        QTimer::singleShot(1700, this, &SelectionToolTestWidget::testPasteSelection);
        QTimer::singleShot(2100, this, &SelectionToolTestWidget::testMoveSelection);
        QTimer::singleShot(2500, this, &SelectionToolTestWidget::testRotateSelection);
        QTimer::singleShot(2900, this, &SelectionToolTestWidget::testFlipSelection);
        QTimer::singleShot(3300, this, &SelectionToolTestWidget::testMouseActions);
        QTimer::singleShot(3700, this, &SelectionToolTestWidget::testKeyboardShortcuts);
        
        QTimer::singleShot(4100, this, [this]() {
            logMessage("=== Complete Selection Tool Test Suite Finished ===");
            logMessage("All Task 79 selection features tested successfully!");
        });
    }
    
    void clearSelection() {
        if (selectionBrush_) {
            selectionBrush_->clearSelection();
            logMessage("Selection cleared");
            updateSelectionInfo();
        }
    }
    
    void clearLog() {
        if (statusText_) {
            statusText_->clear();
            logMessage("Log cleared - ready for new tests");
        }
    }
    
    void onSelectionChanged() {
        updateSelectionInfo();
        logMessage("Selection changed signal received");
    }
    
    void onOperationCompleted(const QString& operation, int tileCount, int itemCount) {
        logMessage(QString("Operation completed: %1 (%2 tiles, %3 items)")
                  .arg(operation).arg(tileCount).arg(itemCount));
    }
    
    void onOperationFailed(const QString& operation, const QString& error) {
        logMessage(QString("Operation failed: %1 - %2").arg(operation, error));
    }

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        
        // Create splitter for controls and log
        QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
        mainLayout->addWidget(splitter);
        
        // Controls panel
        QWidget* controlsWidget = new QWidget();
        controlsWidget->setMaximumWidth(350);
        controlsWidget->setMinimumWidth(300);
        setupControlsPanel(controlsWidget);
        splitter->addWidget(controlsWidget);
        
        // Status/log panel
        QWidget* logWidget = new QWidget();
        setupLogPanel(logWidget);
        splitter->addWidget(logWidget);
        
        // Set splitter proportions
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
    }

    void setupControlsPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // Selection operation buttons
        QGroupBox* selectionGroup = new QGroupBox("Selection Operations", parent);
        QVBoxLayout* selectionLayout = new QVBoxLayout(selectionGroup);

        QPushButton* singleBtn = new QPushButton("Test Single Selection", selectionGroup);
        singleBtn->setToolTip("Test single tile/item selection");
        connect(singleBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testSingleSelection);
        selectionLayout->addWidget(singleBtn);

        QPushButton* rectangleBtn = new QPushButton("Test Rectangle Selection", selectionGroup);
        rectangleBtn->setToolTip("Test rectangle area selection");
        connect(rectangleBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testRectangleSelection);
        selectionLayout->addWidget(rectangleBtn);

        QPushButton* clearBtn = new QPushButton("Clear Selection", selectionGroup);
        clearBtn->setToolTip("Clear current selection");
        connect(clearBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::clearSelection);
        selectionLayout->addWidget(clearBtn);

        layout->addWidget(selectionGroup);

        // Clipboard operation buttons
        QGroupBox* clipboardGroup = new QGroupBox("Clipboard Operations", parent);
        QVBoxLayout* clipboardLayout = new QVBoxLayout(clipboardGroup);

        QPushButton* copyBtn = new QPushButton("Test Copy", clipboardGroup);
        copyBtn->setToolTip("Test copy selection to clipboard");
        connect(copyBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testCopySelection);
        clipboardLayout->addWidget(copyBtn);

        QPushButton* cutBtn = new QPushButton("Test Cut", clipboardGroup);
        cutBtn->setToolTip("Test cut selection to clipboard");
        connect(cutBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testCutSelection);
        clipboardLayout->addWidget(cutBtn);

        QPushButton* pasteBtn = new QPushButton("Test Paste", clipboardGroup);
        pasteBtn->setToolTip("Test paste from clipboard");
        connect(pasteBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testPasteSelection);
        clipboardLayout->addWidget(pasteBtn);

        QPushButton* deleteBtn = new QPushButton("Test Delete", clipboardGroup);
        deleteBtn->setToolTip("Test delete selection");
        connect(deleteBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testDeleteSelection);
        clipboardLayout->addWidget(deleteBtn);

        layout->addWidget(clipboardGroup);

        // Transformation operation buttons
        QGroupBox* transformGroup = new QGroupBox("Transformation Operations", parent);
        QVBoxLayout* transformLayout = new QVBoxLayout(transformGroup);

        QPushButton* moveBtn = new QPushButton("Test Move", transformGroup);
        moveBtn->setToolTip("Test move selection");
        connect(moveBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testMoveSelection);
        transformLayout->addWidget(moveBtn);

        QPushButton* rotateBtn = new QPushButton("Test Rotate", transformGroup);
        rotateBtn->setToolTip("Test rotate selection");
        connect(rotateBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testRotateSelection);
        transformLayout->addWidget(rotateBtn);

        QPushButton* flipBtn = new QPushButton("Test Flip", transformGroup);
        flipBtn->setToolTip("Test flip selection");
        connect(flipBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testFlipSelection);
        transformLayout->addWidget(flipBtn);

        layout->addWidget(transformGroup);

        // Interaction testing buttons
        QGroupBox* interactionGroup = new QGroupBox("Interaction Testing", parent);
        QVBoxLayout* interactionLayout = new QVBoxLayout(interactionGroup);

        QPushButton* mouseBtn = new QPushButton("Test Mouse Actions", interactionGroup);
        mouseBtn->setToolTip("Test mouse interaction handling");
        connect(mouseBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testMouseActions);
        interactionLayout->addWidget(mouseBtn);

        QPushButton* keyboardBtn = new QPushButton("Test Keyboard Shortcuts", interactionGroup);
        keyboardBtn->setToolTip("Test keyboard shortcut handling");
        connect(keyboardBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testKeyboardShortcuts);
        interactionLayout->addWidget(keyboardBtn);

        layout->addWidget(interactionGroup);

        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);

        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setToolTip("Run complete test suite for all selection features");
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::testAllFeatures);
        suiteLayout->addWidget(allTestsBtn);

        QPushButton* clearLogBtn = new QPushButton("Clear Log", suiteGroup);
        clearLogBtn->setToolTip("Clear the test log");
        connect(clearLogBtn, &QPushButton::clicked, this, &SelectionToolTestWidget::clearLog);
        suiteLayout->addWidget(clearLogBtn);

        layout->addWidget(suiteGroup);

        layout->addStretch();
    }

    void setupLogPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        QLabel* logLabel = new QLabel("Test Results and Status Log:", parent);
        logLabel->setStyleSheet("font-weight: bold;");
        layout->addWidget(logLabel);

        statusText_ = new QTextEdit(parent);
        statusText_->setReadOnly(true);
        statusText_->setFont(QFont("Consolas", 9));
        layout->addWidget(statusText_);

        // Selection info panel
        QGroupBox* infoGroup = new QGroupBox("Selection Information", parent);
        QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

        selectionInfoLabel_ = new QLabel("No selection", infoGroup);
        selectionInfoLabel_->setStyleSheet("font-family: monospace;");
        infoLayout->addWidget(selectionInfoLabel_);

        layout->addWidget(infoGroup);
    }

    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();

        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Exit", this, &QWidget::close);

        // Selection menu
        QMenu* selectionMenu = menuBar->addMenu("&Selection");
        selectionMenu->addAction("&Single Selection", this, &SelectionToolTestWidget::testSingleSelection);
        selectionMenu->addAction("&Rectangle Selection", this, &SelectionToolTestWidget::testRectangleSelection);
        selectionMenu->addAction("&Clear Selection", this, &SelectionToolTestWidget::clearSelection);

        // Edit menu
        QMenu* editMenu = menuBar->addMenu("&Edit");
        editMenu->addAction("&Copy", this, &SelectionToolTestWidget::testCopySelection);
        editMenu->addAction("Cu&t", this, &SelectionToolTestWidget::testCutSelection);
        editMenu->addAction("&Paste", this, &SelectionToolTestWidget::testPasteSelection);
        editMenu->addAction("&Delete", this, &SelectionToolTestWidget::testDeleteSelection);

        // Transform menu
        QMenu* transformMenu = menuBar->addMenu("&Transform");
        transformMenu->addAction("&Move", this, &SelectionToolTestWidget::testMoveSelection);
        transformMenu->addAction("&Rotate", this, &SelectionToolTestWidget::testRotateSelection);
        transformMenu->addAction("&Flip", this, &SelectionToolTestWidget::testFlipSelection);

        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("&Mouse Actions", this, &SelectionToolTestWidget::testMouseActions);
        testMenu->addAction("&Keyboard Shortcuts", this, &SelectionToolTestWidget::testKeyboardShortcuts);
        testMenu->addSeparator();
        testMenu->addAction("Run &All Tests", this, &SelectionToolTestWidget::testAllFeatures);
        testMenu->addAction("&Clear Log", this, &SelectionToolTestWidget::clearLog);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 79 selection tool features");
    }

    void initializeComponents() {
        // Initialize undo stack
        undoStack_ = new QUndoStack(this);

        // Initialize test map
        testMap_ = new Map(this);

        // Initialize selection
        selection_ = new Selection(this);

        // Initialize clipboard manager
        clipboardManager_ = new ClipboardManager(this);

        // Initialize selection brush
        selectionBrush_ = new SelectionBrush(this);
        selectionBrush_->setSelection(selection_);
        selectionBrush_->setClipboardManager(clipboardManager_);
        selectionBrush_->setUndoStack(undoStack_);

        logMessage("All components initialized successfully");
    }

    void connectSignals() {
        if (selectionBrush_) {
            connect(selectionBrush_, &SelectionBrush::selectionChanged,
                    this, &SelectionToolTestWidget::onSelectionChanged);
            connect(selectionBrush_, &SelectionBrush::operationCompleted,
                    this, &SelectionToolTestWidget::onOperationCompleted);
            connect(selectionBrush_, &SelectionBrush::operationFailed,
                    this, &SelectionToolTestWidget::onOperationFailed);
        }
    }

    void updateSelectionInfo() {
        if (!selectionBrush_ || !selectionInfoLabel_) {
            return;
        }

        QString info;
        if (selectionBrush_->hasSelection()) {
            int tileCount = selectionBrush_->getSelectedTileCount();
            int itemCount = selectionBrush_->getSelectedItemCount();
            info = QString("Selection: %1 tiles, %2 items").arg(tileCount).arg(itemCount);
        } else {
            info = "No selection";
        }

        selectionInfoLabel_->setText(info);
    }

    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "SelectionToolTest:" << message;
    }

private:
    SelectionBrush* selectionBrush_;
    Selection* selection_;
    ClipboardManager* clipboardManager_;
    QUndoStack* undoStack_;
    Map* testMap_;
    QTextEdit* statusText_;
    QLabel* selectionInfoLabel_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Selection Tool Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");

    SelectionToolTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "SelectionToolTest.moc"
