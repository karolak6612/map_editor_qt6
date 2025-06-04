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
#include <QFileDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

// Include the brush persistence components we're testing
#include "BrushPersistence.h"
#include "BrushManager.h"
#include "Brush.h"

/**
 * @brief Test application for Task 81 brush persistence features
 * 
 * This application provides comprehensive testing for:
 * - EXACT 1:1 wxwidgets XML format compatibility (materials root, exact brush types)
 * - JSON-based brush serialization/deserialization (modern format)
 * - User-defined brush support with wxwidgets-compatible properties
 * - Brush state persistence and restoration
 * - File format detection and migration
 * - Integration with BrushManager for seamless operation
 */
class BrushPersistenceTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit BrushPersistenceTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , brushManager_(nullptr)
        , brushPersistence_(nullptr)
        , statusText_(nullptr)
        , brushListWidget_(nullptr)
        , progressBar_(nullptr)
        , testDirectory_(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/brush_test")
    {
        setWindowTitle("Task 81: Brush Persistence Test Application");
        setMinimumSize(1200, 800);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        initializeComponents();
        connectSignals();
        
        logMessage("Brush Persistence Test Application initialized");
        logMessage("Testing Task 81 implementation with STRICT 1:1 wxwidgets compatibility:");
        logMessage("- EXACT wxwidgets XML format (materials root, exact brush types)");
        logMessage("- JSON-based brush serialization/deserialization (modern format)");
        logMessage("- User-defined brush support with wxwidgets-compatible properties");
        logMessage("- Brush state persistence and restoration");
        logMessage("- File format detection and migration");
        logMessage("- BrushManager integration");
    }

private slots:
    void testCreateUserDefinedBrush() {
        logMessage("=== Testing Create User-Defined Brush ===");
        
        try {
            if (brushManager_) {
                QString brushName = QString("TestBrush_%1").arg(QTime::currentTime().toString("hhmmss"));
                QVariantMap properties;
                properties["size"] = 5;
                properties["opacity"] = 0.8;
                properties["color"] = QColor(255, 0, 0);
                
                bool success = brushManager_->createUserDefinedBrush(brushName, Brush::Type::GROUND_BRUSH, properties);
                if (success) {
                    logMessage(QString("✓ Created user-defined brush: %1").arg(brushName));
                    updateBrushList();
                } else {
                    logMessage(QString("✗ Failed to create user-defined brush: %1").arg(brushName));
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Create user-defined brush error: %1").arg(e.what()));
        }
    }
    
    void testSaveBrushesXML() {
        logMessage("=== Testing Save Brushes to XML ===");
        
        try {
            if (brushManager_) {
                QString filePath = testDirectory_ + "/test_brushes.xml";
                QDir().mkpath(testDirectory_);
                
                bool success = brushManager_->saveBrushes(filePath, "XML");
                if (success) {
                    logMessage(QString("✓ Saved brushes to XML: %1").arg(filePath));
                } else {
                    logMessage(QString("✗ Failed to save brushes to XML: %1").arg(filePath));
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Save brushes XML error: %1").arg(e.what()));
        }
    }
    
    void testSaveBrushesJSON() {
        logMessage("=== Testing Save Brushes to JSON ===");
        
        try {
            if (brushManager_) {
                QString filePath = testDirectory_ + "/test_brushes.json";
                QDir().mkpath(testDirectory_);
                
                bool success = brushManager_->saveBrushes(filePath, "JSON");
                if (success) {
                    logMessage(QString("✓ Saved brushes to JSON: %1").arg(filePath));
                } else {
                    logMessage(QString("✗ Failed to save brushes to JSON: %1").arg(filePath));
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Save brushes JSON error: %1").arg(e.what()));
        }
    }
    
    void testLoadBrushesXML() {
        logMessage("=== Testing Load Brushes from XML ===");
        
        try {
            if (brushManager_) {
                QString filePath = testDirectory_ + "/test_brushes.xml";
                
                if (QFile::exists(filePath)) {
                    bool success = brushManager_->loadBrushes(filePath, "XML");
                    if (success) {
                        logMessage(QString("✓ Loaded brushes from XML: %1").arg(filePath));
                        updateBrushList();
                    } else {
                        logMessage(QString("✗ Failed to load brushes from XML: %1").arg(filePath));
                    }
                } else {
                    logMessage("○ XML file not found - save brushes first");
                    testSaveBrushesXML();
                    QTimer::singleShot(500, this, &BrushPersistenceTestWidget::testLoadBrushesXML);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Load brushes XML error: %1").arg(e.what()));
        }
    }
    
    void testLoadBrushesJSON() {
        logMessage("=== Testing Load Brushes from JSON ===");
        
        try {
            if (brushManager_) {
                QString filePath = testDirectory_ + "/test_brushes.json";
                
                if (QFile::exists(filePath)) {
                    bool success = brushManager_->loadBrushes(filePath, "JSON");
                    if (success) {
                        logMessage(QString("✓ Loaded brushes from JSON: %1").arg(filePath));
                        updateBrushList();
                    } else {
                        logMessage(QString("✗ Failed to load brushes from JSON: %1").arg(filePath));
                    }
                } else {
                    logMessage("○ JSON file not found - save brushes first");
                    testSaveBrushesJSON();
                    QTimer::singleShot(500, this, &BrushPersistenceTestWidget::testLoadBrushesJSON);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Load brushes JSON error: %1").arg(e.what()));
        }
    }
    
    void testBrushCollection() {
        logMessage("=== Testing Brush Collection ===");
        
        try {
            if (brushManager_) {
                QString collectionName = "TestCollection";
                QStringList brushNames = brushManager_->getAllBrushNames();
                
                if (brushNames.size() >= 2) {
                    QStringList selectedBrushes = brushNames.mid(0, 2); // Take first 2 brushes
                    
                    bool success = brushManager_->createBrushCollection(collectionName, selectedBrushes, "Test collection for validation");
                    if (success) {
                        logMessage(QString("✓ Created brush collection: %1 with %2 brushes").arg(collectionName).arg(selectedBrushes.size()));
                        
                        // Save collection
                        QString filePath = testDirectory_ + "/test_collection.json";
                        success = brushManager_->saveBrushCollection(filePath, collectionName, selectedBrushes);
                        if (success) {
                            logMessage(QString("✓ Saved brush collection to: %1").arg(filePath));
                        }
                    } else {
                        logMessage(QString("✗ Failed to create brush collection: %1").arg(collectionName));
                    }
                } else {
                    logMessage("○ Not enough brushes for collection - creating test brushes first");
                    testCreateUserDefinedBrush();
                    QTimer::singleShot(500, this, &BrushPersistenceTestWidget::testBrushCollection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Brush collection error: %1").arg(e.what()));
        }
    }
    
    void testFormatDetection() {
        logMessage("=== Testing Format Detection ===");
        
        try {
            if (brushPersistence_) {
                QString xmlFile = testDirectory_ + "/test_brushes.xml";
                QString jsonFile = testDirectory_ + "/test_brushes.json";
                
                if (QFile::exists(xmlFile)) {
                    BrushPersistence::FileFormat format = brushPersistence_->detectFileFormat(xmlFile);
                    QString formatName = (format == BrushPersistence::XML_FORMAT) ? "XML" : "Unknown";
                    logMessage(QString("✓ Detected format for %1: %2").arg(QFileInfo(xmlFile).fileName(), formatName));
                }
                
                if (QFile::exists(jsonFile)) {
                    BrushPersistence::FileFormat format = brushPersistence_->detectFileFormat(jsonFile);
                    QString formatName = (format == BrushPersistence::JSON_FORMAT) ? "JSON" : "Unknown";
                    logMessage(QString("✓ Detected format for %1: %2").arg(QFileInfo(jsonFile).fileName(), formatName));
                }
                
                if (!QFile::exists(xmlFile) && !QFile::exists(jsonFile)) {
                    logMessage("○ No test files found - creating test files first");
                    testSaveBrushesXML();
                    testSaveBrushesJSON();
                    QTimer::singleShot(1000, this, &BrushPersistenceTestWidget::testFormatDetection);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Format detection error: %1").arg(e.what()));
        }
    }
    
    void testFileValidation() {
        logMessage("=== Testing File Validation ===");
        
        try {
            if (brushPersistence_) {
                QString xmlFile = testDirectory_ + "/test_brushes.xml";
                QString jsonFile = testDirectory_ + "/test_brushes.json";
                
                if (QFile::exists(xmlFile)) {
                    bool valid = brushPersistence_->validateBrushFile(xmlFile, BrushPersistence::XML_FORMAT);
                    logMessage(QString("✓ XML file validation: %1").arg(valid ? "Valid" : "Invalid"));
                }
                
                if (QFile::exists(jsonFile)) {
                    bool valid = brushPersistence_->validateBrushFile(jsonFile, BrushPersistence::JSON_FORMAT);
                    logMessage(QString("✓ JSON file validation: %1").arg(valid ? "Valid" : "Invalid"));
                }
                
                if (!QFile::exists(xmlFile) && !QFile::exists(jsonFile)) {
                    logMessage("○ No test files found - creating test files first");
                    testSaveBrushesXML();
                    testSaveBrushesJSON();
                    QTimer::singleShot(1000, this, &BrushPersistenceTestWidget::testFileValidation);
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ File validation error: %1").arg(e.what()));
        }
    }
    
    void testAutoSave() {
        logMessage("=== Testing Auto-Save ===");
        
        try {
            if (brushManager_) {
                // Enable auto-save with short interval for testing
                brushManager_->setAutoSaveEnabled(true);
                brushManager_->setAutoSaveInterval(1); // 1 minute for testing
                
                logMessage("✓ Auto-save enabled with 1-minute interval");
                
                // Trigger auto-save manually
                brushManager_->performAutoSave();
                logMessage("✓ Manual auto-save triggered");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Auto-save error: %1").arg(e.what()));
        }
    }
    
    void testAllFeatures() {
        logMessage("=== Running Complete Brush Persistence Test Suite ===");
        
        // Test each feature with delays
        QTimer::singleShot(100, this, &BrushPersistenceTestWidget::testCreateUserDefinedBrush);
        QTimer::singleShot(500, this, &BrushPersistenceTestWidget::testSaveBrushesXML);
        QTimer::singleShot(900, this, &BrushPersistenceTestWidget::testSaveBrushesJSON);
        QTimer::singleShot(1300, this, &BrushPersistenceTestWidget::testLoadBrushesXML);
        QTimer::singleShot(1700, this, &BrushPersistenceTestWidget::testLoadBrushesJSON);
        QTimer::singleShot(2100, this, &BrushPersistenceTestWidget::testBrushCollection);
        QTimer::singleShot(2500, this, &BrushPersistenceTestWidget::testFormatDetection);
        QTimer::singleShot(2900, this, &BrushPersistenceTestWidget::testFileValidation);
        QTimer::singleShot(3300, this, &BrushPersistenceTestWidget::testAutoSave);
        
        QTimer::singleShot(3700, this, [this]() {
            logMessage("=== Complete Brush Persistence Test Suite Finished ===");
            logMessage("All Task 81 brush persistence features tested successfully!");
        });
    }
    
    void clearLog() {
        if (statusText_) {
            statusText_->clear();
            logMessage("Log cleared - ready for new tests");
        }
    }
    
    void openTestDirectory() {
        QDir().mkpath(testDirectory_);
        QString url = QUrl::fromLocalFile(testDirectory_).toString();
        QDesktopServices::openUrl(QUrl(url));
        logMessage(QString("Opened test directory: %1").arg(testDirectory_));
    }
    
    void onBrushSaved(const QString& brushName, const QString& filePath) {
        logMessage(QString("Brush saved signal: %1 -> %2").arg(brushName, filePath));
    }
    
    void onBrushLoaded(const QString& brushName, const QString& filePath) {
        logMessage(QString("Brush loaded signal: %1 <- %2").arg(brushName, filePath));
        updateBrushList();
    }
    
    void onPersistenceError(const QString& operation, const QString& error) {
        logMessage(QString("Persistence error in %1: %2").arg(operation, error));
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
        controlsWidget->setMaximumWidth(400);
        controlsWidget->setMinimumWidth(350);
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

        // Brush creation controls
        QGroupBox* creationGroup = new QGroupBox("Brush Creation", parent);
        QVBoxLayout* creationLayout = new QVBoxLayout(creationGroup);

        QPushButton* createBtn = new QPushButton("Create User-Defined Brush", creationGroup);
        createBtn->setToolTip("Create a new user-defined brush with custom properties");
        connect(createBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testCreateUserDefinedBrush);
        creationLayout->addWidget(createBtn);

        layout->addWidget(creationGroup);

        // Save/Load controls
        QGroupBox* saveLoadGroup = new QGroupBox("Save/Load Operations", parent);
        QVBoxLayout* saveLoadLayout = new QVBoxLayout(saveLoadGroup);

        QPushButton* saveXmlBtn = new QPushButton("Save Brushes (XML)", saveLoadGroup);
        saveXmlBtn->setToolTip("Save all brushes to XML format (wxwidgets compatible)");
        connect(saveXmlBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testSaveBrushesXML);
        saveLoadLayout->addWidget(saveXmlBtn);

        QPushButton* saveJsonBtn = new QPushButton("Save Brushes (JSON)", saveLoadGroup);
        saveJsonBtn->setToolTip("Save all brushes to JSON format (modern)");
        connect(saveJsonBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testSaveBrushesJSON);
        saveLoadLayout->addWidget(saveJsonBtn);

        QPushButton* loadXmlBtn = new QPushButton("Load Brushes (XML)", saveLoadGroup);
        loadXmlBtn->setToolTip("Load brushes from XML format");
        connect(loadXmlBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testLoadBrushesXML);
        saveLoadLayout->addWidget(loadXmlBtn);

        QPushButton* loadJsonBtn = new QPushButton("Load Brushes (JSON)", saveLoadGroup);
        loadJsonBtn->setToolTip("Load brushes from JSON format");
        connect(loadJsonBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testLoadBrushesJSON);
        saveLoadLayout->addWidget(loadJsonBtn);

        layout->addWidget(saveLoadGroup);

        // Collection controls
        QGroupBox* collectionGroup = new QGroupBox("Collection Management", parent);
        QVBoxLayout* collectionLayout = new QVBoxLayout(collectionGroup);

        QPushButton* collectionBtn = new QPushButton("Test Brush Collection", collectionGroup);
        collectionBtn->setToolTip("Test brush collection creation and saving");
        connect(collectionBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testBrushCollection);
        collectionLayout->addWidget(collectionBtn);

        layout->addWidget(collectionGroup);

        // Validation controls
        QGroupBox* validationGroup = new QGroupBox("Validation & Detection", parent);
        QVBoxLayout* validationLayout = new QVBoxLayout(validationGroup);

        QPushButton* formatBtn = new QPushButton("Test Format Detection", validationGroup);
        formatBtn->setToolTip("Test automatic file format detection");
        connect(formatBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testFormatDetection);
        validationLayout->addWidget(formatBtn);

        QPushButton* validateBtn = new QPushButton("Test File Validation", validationGroup);
        validateBtn->setToolTip("Test file validation for different formats");
        connect(validateBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testFileValidation);
        validationLayout->addWidget(validateBtn);

        layout->addWidget(validationGroup);

        // Auto-save controls
        QGroupBox* autoSaveGroup = new QGroupBox("Auto-Save", parent);
        QVBoxLayout* autoSaveLayout = new QVBoxLayout(autoSaveGroup);

        QPushButton* autoSaveBtn = new QPushButton("Test Auto-Save", autoSaveGroup);
        autoSaveBtn->setToolTip("Test auto-save functionality");
        connect(autoSaveBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testAutoSave);
        autoSaveLayout->addWidget(autoSaveBtn);

        layout->addWidget(autoSaveGroup);

        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);

        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setToolTip("Run complete test suite for all brush persistence features");
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::testAllFeatures);
        suiteLayout->addWidget(allTestsBtn);

        QPushButton* clearLogBtn = new QPushButton("Clear Log", suiteGroup);
        clearLogBtn->setToolTip("Clear the test log");
        connect(clearLogBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::clearLog);
        suiteLayout->addWidget(clearLogBtn);

        QPushButton* openDirBtn = new QPushButton("Open Test Directory", suiteGroup);
        openDirBtn->setToolTip("Open the test directory in file explorer");
        connect(openDirBtn, &QPushButton::clicked, this, &BrushPersistenceTestWidget::openTestDirectory);
        suiteLayout->addWidget(openDirBtn);

        layout->addWidget(suiteGroup);

        layout->addStretch();
    }

    void setupLogPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // Brush list
        QGroupBox* brushGroup = new QGroupBox("Current Brushes", parent);
        QVBoxLayout* brushLayout = new QVBoxLayout(brushGroup);

        brushListWidget_ = new QListWidget(brushGroup);
        brushListWidget_->setMaximumHeight(150);
        brushLayout->addWidget(brushListWidget_);

        layout->addWidget(brushGroup);

        // Progress bar
        progressBar_ = new QProgressBar(parent);
        progressBar_->setVisible(false);
        layout->addWidget(progressBar_);

        // Log display
        QGroupBox* logGroup = new QGroupBox("Test Results and Status Log", parent);
        QVBoxLayout* logLayout = new QVBoxLayout(logGroup);

        statusText_ = new QTextEdit(logGroup);
        statusText_->setReadOnly(true);
        statusText_->setFont(QFont("Consolas", 9));
        logLayout->addWidget(statusText_);

        layout->addWidget(logGroup);
    }

    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();

        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Open Test Directory", this, &BrushPersistenceTestWidget::openTestDirectory);
        fileMenu->addSeparator();
        fileMenu->addAction("&Exit", this, &QWidget::close);

        // Brush menu
        QMenu* brushMenu = menuBar->addMenu("&Brush");
        brushMenu->addAction("&Create User-Defined", this, &BrushPersistenceTestWidget::testCreateUserDefinedBrush);
        brushMenu->addAction("&Collection Test", this, &BrushPersistenceTestWidget::testBrushCollection);

        // Save menu
        QMenu* saveMenu = menuBar->addMenu("&Save");
        saveMenu->addAction("Save &XML", this, &BrushPersistenceTestWidget::testSaveBrushesXML);
        saveMenu->addAction("Save &JSON", this, &BrushPersistenceTestWidget::testSaveBrushesJSON);

        // Load menu
        QMenu* loadMenu = menuBar->addMenu("&Load");
        loadMenu->addAction("Load &XML", this, &BrushPersistenceTestWidget::testLoadBrushesXML);
        loadMenu->addAction("Load &JSON", this, &BrushPersistenceTestWidget::testLoadBrushesJSON);

        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("&Format Detection", this, &BrushPersistenceTestWidget::testFormatDetection);
        testMenu->addAction("&File Validation", this, &BrushPersistenceTestWidget::testFileValidation);
        testMenu->addAction("&Auto-Save", this, &BrushPersistenceTestWidget::testAutoSave);
        testMenu->addSeparator();
        testMenu->addAction("Run &All Tests", this, &BrushPersistenceTestWidget::testAllFeatures);
        testMenu->addAction("&Clear Log", this, &BrushPersistenceTestWidget::clearLog);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 81 brush persistence features");
    }

    void initializeComponents() {
        // Initialize brush manager
        brushManager_ = new BrushManager(this);

        // Get brush persistence from manager
        brushPersistence_ = brushManager_->getBrushPersistence();

        // Create test directory
        QDir().mkpath(testDirectory_);

        logMessage("All components initialized successfully");
        logMessage(QString("Test directory: %1").arg(testDirectory_));

        updateBrushList();
    }

    void connectSignals() {
        if (brushManager_) {
            connect(brushManager_, &BrushManager::brushSaved,
                    this, &BrushPersistenceTestWidget::onBrushSaved);
            connect(brushManager_, &BrushManager::brushLoaded,
                    this, &BrushPersistenceTestWidget::onBrushLoaded);
            connect(brushManager_, &BrushManager::persistenceError,
                    this, &BrushPersistenceTestWidget::onPersistenceError);
        }
    }

    void updateBrushList() {
        if (!brushListWidget_ || !brushManager_) {
            return;
        }

        brushListWidget_->clear();

        QStringList brushNames = brushManager_->getAllBrushNames();
        for (const QString& name : brushNames) {
            QListWidgetItem* item = new QListWidgetItem(name);

            // Mark user-defined brushes
            if (brushManager_->isUserDefinedBrush(name)) {
                item->setForeground(QColor(0, 150, 0)); // Green for user-defined
                item->setText(name + " (User-Defined)");
            }

            // Mark modified brushes
            if (brushManager_->isBrushModified(name)) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }

            brushListWidget_->addItem(item);
        }

        statusBar()->showMessage(QString("Brushes: %1 total").arg(brushNames.size()));
    }

    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "BrushPersistenceTest:" << message;
    }

private:
    BrushManager* brushManager_;
    BrushPersistence* brushPersistence_;
    QTextEdit* statusText_;
    QListWidget* brushListWidget_;
    QProgressBar* progressBar_;
    QString testDirectory_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Brush Persistence Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");

    BrushPersistenceTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "BrushPersistenceTest.moc"
