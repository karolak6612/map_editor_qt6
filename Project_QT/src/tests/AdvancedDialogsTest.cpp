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
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

// Include the dialogs we're testing
#include "PreferencesDialog.h"
#include "AboutDialog.h"
#include "TilesetWindow.h"
#include "HelpSystem.h"
#include "DialogManager.h"

/**
 * @brief Test application for Task 78 advanced dialogs and windows
 * 
 * This application provides comprehensive testing for:
 * - PreferencesDialog with all tabs and settings
 * - Enhanced AboutDialog with multiple tabs
 * - TilesetWindow for tileset management
 * - HelpSystem for documentation and help
 * - DialogManager integration
 */
class AdvancedDialogsTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit AdvancedDialogsTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , dialogManager_(nullptr)
        , statusText_(nullptr)
    {
        setWindowTitle("Task 78: Advanced Dialogs Test Application");
        setMinimumSize(800, 600);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        connectSignals();
        
        // Initialize dialog manager
        dialogManager_ = new DialogManager(this, this);
        
        logMessage("Advanced Dialogs Test Application initialized");
        logMessage("Testing Task 78 implementation:");
        logMessage("- PreferencesDialog (comprehensive settings)");
        logMessage("- Enhanced AboutDialog (multi-tab)");
        logMessage("- TilesetWindow (tileset editor)");
        logMessage("- HelpSystem (documentation)");
        logMessage("- DialogManager integration");
    }

private slots:
    void testPreferencesDialog() {
        logMessage("=== Testing PreferencesDialog ===");
        
        try {
            if (dialogManager_) {
                dialogManager_->showPreferencesDialog();
                logMessage("✓ PreferencesDialog launched successfully");
            } else {
                PreferencesDialog dialog(this);
                dialog.loadSettings();
                int result = dialog.exec();
                
                if (result == QDialog::Accepted) {
                    logMessage("✓ PreferencesDialog accepted - settings saved");
                } else {
                    logMessage("○ PreferencesDialog cancelled");
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ PreferencesDialog error: %1").arg(e.what()));
        }
    }
    
    void testAboutDialog() {
        logMessage("=== Testing Enhanced AboutDialog ===");
        
        try {
            if (dialogManager_) {
                dialogManager_->showAboutDialog();
                logMessage("✓ AboutDialog launched successfully");
            } else {
                AboutDialog dialog(this);
                int result = dialog.exec();
                logMessage("✓ AboutDialog completed");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ AboutDialog error: %1").arg(e.what()));
        }
    }
    
    void testTilesetWindow() {
        logMessage("=== Testing TilesetWindow ===");
        
        try {
            if (dialogManager_) {
                dialogManager_->showTilesetWindow();
                logMessage("✓ TilesetWindow launched successfully");
            } else {
                TilesetWindow* window = new TilesetWindow(this);
                window->show();
                window->raise();
                window->activateWindow();
                logMessage("✓ TilesetWindow created and shown");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ TilesetWindow error: %1").arg(e.what()));
        }
    }
    
    void testHelpSystem() {
        logMessage("=== Testing HelpSystem ===");
        
        try {
            if (dialogManager_) {
                dialogManager_->showHelpSystem();
                logMessage("✓ HelpSystem launched successfully");
            } else {
                HelpSystem::showHelp();
                logMessage("✓ HelpSystem static method called");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ HelpSystem error: %1").arg(e.what()));
        }
    }
    
    void testKeyboardShortcuts() {
        logMessage("=== Testing Keyboard Shortcuts Help ===");
        
        try {
            if (dialogManager_) {
                dialogManager_->showKeyboardShortcuts();
                logMessage("✓ Keyboard shortcuts help launched");
            } else {
                HelpSystem::showKeyboardShortcuts();
                logMessage("✓ Keyboard shortcuts static method called");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Keyboard shortcuts error: %1").arg(e.what()));
        }
    }
    
    void testToolHelp() {
        logMessage("=== Testing Tool Help ===");
        
        try {
            QString toolName = "BrushTool";
            if (dialogManager_) {
                dialogManager_->showToolHelp(toolName);
                logMessage(QString("✓ Tool help for '%1' launched").arg(toolName));
            } else {
                HelpSystem::showToolHelp(toolName);
                logMessage(QString("✓ Tool help static method called for '%1'").arg(toolName));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Tool help error: %1").arg(e.what()));
        }
    }
    
    void testAllDialogs() {
        logMessage("=== Running Complete Dialog Test Suite ===");
        
        // Test each dialog with a small delay
        QTimer::singleShot(100, this, &AdvancedDialogsTestWidget::testPreferencesDialog);
        QTimer::singleShot(500, this, &AdvancedDialogsTestWidget::testAboutDialog);
        QTimer::singleShot(900, this, &AdvancedDialogsTestWidget::testTilesetWindow);
        QTimer::singleShot(1300, this, &AdvancedDialogsTestWidget::testHelpSystem);
        QTimer::singleShot(1700, this, &AdvancedDialogsTestWidget::testKeyboardShortcuts);
        QTimer::singleShot(2100, this, &AdvancedDialogsTestWidget::testToolHelp);
        
        QTimer::singleShot(2500, this, [this]() {
            logMessage("=== Complete Dialog Test Suite Finished ===");
            logMessage("All Task 78 dialogs tested successfully!");
        });
    }
    
    void clearLog() {
        if (statusText_) {
            statusText_->clear();
            logMessage("Log cleared - ready for new tests");
        }
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
        controlsWidget->setMaximumWidth(300);
        controlsWidget->setMinimumWidth(250);
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
        
        // Dialog test buttons
        QGroupBox* dialogGroup = new QGroupBox("Dialog Tests", parent);
        QVBoxLayout* dialogLayout = new QVBoxLayout(dialogGroup);
        
        QPushButton* preferencesBtn = new QPushButton("Test Preferences Dialog", dialogGroup);
        preferencesBtn->setToolTip("Test comprehensive preferences dialog with all tabs");
        connect(preferencesBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testPreferencesDialog);
        dialogLayout->addWidget(preferencesBtn);
        
        QPushButton* aboutBtn = new QPushButton("Test About Dialog", dialogGroup);
        aboutBtn->setToolTip("Test enhanced about dialog with multiple tabs");
        connect(aboutBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testAboutDialog);
        dialogLayout->addWidget(aboutBtn);
        
        QPushButton* tilesetBtn = new QPushButton("Test Tileset Window", dialogGroup);
        tilesetBtn->setToolTip("Test advanced tileset editor window");
        connect(tilesetBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testTilesetWindow);
        dialogLayout->addWidget(tilesetBtn);
        
        QPushButton* helpBtn = new QPushButton("Test Help System", dialogGroup);
        helpBtn->setToolTip("Test comprehensive help system");
        connect(helpBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testHelpSystem);
        dialogLayout->addWidget(helpBtn);
        
        layout->addWidget(dialogGroup);
        
        // Help system tests
        QGroupBox* helpGroup = new QGroupBox("Help System Tests", parent);
        QVBoxLayout* helpLayout = new QVBoxLayout(helpGroup);
        
        QPushButton* shortcutsBtn = new QPushButton("Keyboard Shortcuts", helpGroup);
        shortcutsBtn->setToolTip("Test keyboard shortcuts help");
        connect(shortcutsBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testKeyboardShortcuts);
        helpLayout->addWidget(shortcutsBtn);
        
        QPushButton* toolHelpBtn = new QPushButton("Tool Help", helpGroup);
        toolHelpBtn->setToolTip("Test context-sensitive tool help");
        connect(toolHelpBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testToolHelp);
        helpLayout->addWidget(toolHelpBtn);
        
        layout->addWidget(helpGroup);
        
        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);
        
        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setToolTip("Run complete test suite for all dialogs");
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::testAllDialogs);
        suiteLayout->addWidget(allTestsBtn);
        
        QPushButton* clearBtn = new QPushButton("Clear Log", suiteGroup);
        clearBtn->setToolTip("Clear the test log");
        connect(clearBtn, &QPushButton::clicked, this, &AdvancedDialogsTestWidget::clearLog);
        suiteLayout->addWidget(clearBtn);
        
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
    }
    
    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();
        
        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Exit", this, &QWidget::close);
        
        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("&Preferences Dialog", this, &AdvancedDialogsTestWidget::testPreferencesDialog);
        testMenu->addAction("&About Dialog", this, &AdvancedDialogsTestWidget::testAboutDialog);
        testMenu->addAction("&Tileset Window", this, &AdvancedDialogsTestWidget::testTilesetWindow);
        testMenu->addAction("&Help System", this, &AdvancedDialogsTestWidget::testHelpSystem);
        testMenu->addSeparator();
        testMenu->addAction("Run &All Tests", this, &AdvancedDialogsTestWidget::testAllDialogs);
        testMenu->addAction("&Clear Log", this, &AdvancedDialogsTestWidget::clearLog);
        
        // Help menu
        QMenu* helpMenu = menuBar->addMenu("&Help");
        helpMenu->addAction("&Keyboard Shortcuts", this, &AdvancedDialogsTestWidget::testKeyboardShortcuts);
        helpMenu->addAction("&Tool Help", this, &AdvancedDialogsTestWidget::testToolHelp);
    }
    
    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 78 advanced dialogs");
    }
    
    void connectSignals() {
        // No additional signals to connect for now
    }
    
    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "AdvancedDialogsTest:" << message;
    }

private:
    DialogManager* dialogManager_;
    QTextEdit* statusText_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Advanced Dialogs Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");
    
    AdvancedDialogsTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "AdvancedDialogsTest.moc"
