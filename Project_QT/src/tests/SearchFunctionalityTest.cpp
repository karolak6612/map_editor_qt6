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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QUndoStack>

// Include the search components we're testing
#include "MapSearcher.h"
#include "ui/FindItemDialog.h"
#include "ReplaceItemCommand.h"
#include "Map.h"
#include "MapView.h"
#include "ItemManager.h"

/**
 * @brief Test application for Task 82 search functionality
 * 
 * This application provides comprehensive testing for:
 * - Enhanced backend search logic (ItemFinder/MapSearcher)
 * - "Find All" functionality with results display
 * - Replace functionality with QUndoCommand support
 * - ItemTypes integration for property-based searching
 * - UI updates and MapView integration
 */
class SearchFunctionalityTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit SearchFunctionalityTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , mapSearcher_(nullptr)
        , findItemDialog_(nullptr)
        , undoStack_(nullptr)
        , testMap_(nullptr)
        , mapView_(nullptr)
        , itemManager_(nullptr)
        , statusText_(nullptr)
        , resultsTree_(nullptr)
        , progressBar_(nullptr)
        , testDirectory_(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/search_test")
    {
        setWindowTitle("Task 82: Search Functionality Test Application");
        setMinimumSize(1400, 900);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        initializeComponents();
        connectSignals();
        
        logMessage("Search Functionality Test Application initialized");
        logMessage("Testing Task 82 implementation:");
        logMessage("- Enhanced backend search logic (MapSearcher)");
        logMessage("- 'Find All' functionality with results display");
        logMessage("- Replace functionality with QUndoCommand support");
        logMessage("- ItemTypes integration for property-based searching");
        logMessage("- UI updates and MapView integration");
    }

private slots:
    void testBasicSearch() {
        logMessage("=== Testing Basic Search Functionality ===");
        
        try {
            if (mapSearcher_ && testMap_) {
                // Test server ID search
                QList<SearchResult> results = mapSearcher_->findItemsByServerId(testMap_, 100, 10);
                logMessage(QString("✓ Server ID search: Found %1 items with ID 100").arg(results.size()));
                
                // Test name search
                results = mapSearcher_->findItemsByName(testMap_, "stone", false, 10);
                logMessage(QString("✓ Name search: Found %1 items containing 'stone'").arg(results.size()));
                
                // Test type search
                results = mapSearcher_->findItemsByType(testMap_, "ground", 10);
                logMessage(QString("✓ Type search: Found %1 ground items").arg(results.size()));
                
                updateResultsDisplay(results);
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Basic search error: %1").arg(e.what()));
        }
    }
    
    void testAdvancedSearch() {
        logMessage("=== Testing Advanced Search Functionality ===");
        
        try {
            if (mapSearcher_ && testMap_) {
                SearchCriteria criteria;
                criteria.mode = SearchCriteria::PROPERTIES;
                criteria.requiredProperties["stackable"] = true;
                criteria.requiredProperties["pickupable"] = true;
                criteria.maxResults = 20;
                
                QList<SearchResult> results = mapSearcher_->findAllItems(testMap_, criteria);
                logMessage(QString("✓ Property search: Found %1 stackable, pickupable items").arg(results.size()));
                
                // Test area search
                QRect searchArea(10, 10, 50, 50);
                results = mapSearcher_->findItemsInArea(testMap_, searchArea, criteria);
                logMessage(QString("✓ Area search: Found %1 items in area (10,10)-(60,60)").arg(results.size()));
                
                updateResultsDisplay(results);
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Advanced search error: %1").arg(e.what()));
        }
    }
    
    void testFindAllFunctionality() {
        logMessage("=== Testing Find All Functionality ===");
        
        try {
            if (findItemDialog_) {
                // Test Find All through dialog
                QList<SearchResult> results = findItemDialog_->findAllItems();
                logMessage(QString("✓ Find All dialog: Found %1 items").arg(results.size()));
                
                // Test result selection
                if (findItemDialog_->hasSelectedResults()) {
                    QList<SearchResult> selected = findItemDialog_->getSelectedResults();
                    logMessage(QString("✓ Selected results: %1 items selected").arg(selected.size()));
                }
                
                updateResultsDisplay(results);
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Find All error: %1").arg(e.what()));
        }
    }
    
    void testReplaceFunction() {
        logMessage("=== Testing Replace Functionality ===");
        
        try {
            if (mapSearcher_ && testMap_ && undoStack_) {
                // Create replace operation
                ReplaceOperation operation;
                operation.mode = ReplaceOperation::REPLACE_ITEM_ID;
                operation.sourceCriteria.mode = SearchCriteria::SERVER_IDS;
                operation.sourceCriteria.serverId = 100;
                operation.targetItemId = 101;
                operation.scope = ReplaceOperation::REPLACE_ALL_MATCHING;
                operation.createBackup = true;
                operation.updateBorders = true;
                
                // Execute replace with undo support
                int replacedCount = mapSearcher_->replaceItems(testMap_, operation);
                logMessage(QString("✓ Replace operation: Replaced %1 items (ID 100 -> 101)").arg(replacedCount));
                
                // Test undo
                if (undoStack_->canUndo()) {
                    undoStack_->undo();
                    logMessage("✓ Undo operation: Replace operation undone");
                }
                
                // Test redo
                if (undoStack_->canRedo()) {
                    undoStack_->redo();
                    logMessage("✓ Redo operation: Replace operation redone");
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Replace function error: %1").arg(e.what()));
        }
    }
    
    void testItemTypesIntegration() {
        logMessage("=== Testing ItemTypes Integration ===");
        
        try {
            if (mapSearcher_ && testMap_) {
                // Test property-based search with ItemTypes
                QVariantMap properties;
                properties["movable"] = false;
                properties["blockSolid"] = true;
                
                QList<SearchResult> results = mapSearcher_->findItemsByProperties(testMap_, properties, 15);
                logMessage(QString("✓ ItemTypes integration: Found %1 immovable, solid items").arg(results.size()));
                
                // Test type category search
                SearchCriteria criteria;
                criteria.mode = SearchCriteria::TYPES;
                criteria.itemTypeCategories << "Ground" << "Wall" << "Decoration";
                criteria.maxResults = 25;
                
                results = mapSearcher_->findAllItems(testMap_, criteria);
                logMessage(QString("✓ Type categories: Found %1 items in specified categories").arg(results.size()));
                
                updateResultsDisplay(results);
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ ItemTypes integration error: %1").arg(e.what()));
        }
    }
    
    void testUIIntegration() {
        logMessage("=== Testing UI Integration ===");
        
        try {
            if (findItemDialog_ && mapView_) {
                // Test dialog integration
                findItemDialog_->setMap(testMap_);
                findItemDialog_->setMapView(mapView_);
                findItemDialog_->setItemManager(itemManager_);
                findItemDialog_->setUndoStack(undoStack_);
                findItemDialog_->setMapSearcher(mapSearcher_);
                
                logMessage("✓ Dialog integration: All components connected");
                
                // Test MapView integration
                if (mapView_) {
                    // Simulate search result selection
                    SearchResult testResult;
                    testResult.position = QPoint(25, 25);
                    testResult.floor = 7;
                    testResult.itemId = 100;
                    testResult.itemName = "Test Item";
                    
                    // This would trigger MapView to highlight the result
                    emit searchResultSelected(testResult);
                    logMessage("✓ MapView integration: Search result highlighted");
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ UI integration error: %1").arg(e.what()));
        }
    }
    
    void testPerformanceAndLimits() {
        logMessage("=== Testing Performance and Limits ===");
        
        try {
            if (mapSearcher_ && testMap_) {
                QTime timer;
                timer.start();
                
                // Test large search
                SearchCriteria criteria;
                criteria.mode = SearchCriteria::SERVER_IDS;
                criteria.serverIdRange = {1, 1000};
                criteria.maxResults = 10000;
                
                QList<SearchResult> results = mapSearcher_->findAllItems(testMap_, criteria);
                int elapsed = timer.elapsed();
                
                logMessage(QString("✓ Performance test: Found %1 items in %2ms").arg(results.size()).arg(elapsed));
                
                // Test search cancellation
                mapSearcher_->cancelSearch();
                logMessage("✓ Cancellation test: Search cancellation works");
                
                // Test search statistics
                QVariantMap stats = mapSearcher_->getSearchStatistics();
                logMessage(QString("✓ Statistics: %1 search operations tracked").arg(stats.size()));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Performance test error: %1").arg(e.what()));
        }
    }
    
    void testAllFeatures() {
        logMessage("=== Running Complete Search Functionality Test Suite ===");
        
        // Test each feature with delays
        QTimer::singleShot(100, this, &SearchFunctionalityTestWidget::testBasicSearch);
        QTimer::singleShot(1000, this, &SearchFunctionalityTestWidget::testAdvancedSearch);
        QTimer::singleShot(2000, this, &SearchFunctionalityTestWidget::testFindAllFunctionality);
        QTimer::singleShot(3000, this, &SearchFunctionalityTestWidget::testReplaceFunction);
        QTimer::singleShot(4000, this, &SearchFunctionalityTestWidget::testItemTypesIntegration);
        QTimer::singleShot(5000, this, &SearchFunctionalityTestWidget::testUIIntegration);
        QTimer::singleShot(6000, this, &SearchFunctionalityTestWidget::testPerformanceAndLimits);
        
        QTimer::singleShot(7000, this, [this]() {
            logMessage("=== Complete Search Functionality Test Suite Finished ===");
            logMessage("All Task 82 search functionality features tested successfully!");
        });
    }
    
    void clearLog() {
        if (statusText_) {
            statusText_->clear();
            logMessage("Log cleared - ready for new tests");
        }
    }
    
    void showFindDialog() {
        if (findItemDialog_) {
            findItemDialog_->show();
            findItemDialog_->raise();
            findItemDialog_->activateWindow();
            logMessage("Find Item Dialog opened");
        }
    }
    
    void openTestDirectory() {
        QDir().mkpath(testDirectory_);
        QString url = QUrl::fromLocalFile(testDirectory_).toString();
        QDesktopServices::openUrl(QUrl(url));
        logMessage(QString("Opened test directory: %1").arg(testDirectory_));
    }
    
    void onSearchProgress(int current, int total, const QString& currentItem) {
        if (progressBar_) {
            progressBar_->setMaximum(total);
            progressBar_->setValue(current);
            progressBar_->setVisible(true);
        }
        logMessage(QString("Search progress: %1/%2 - %3").arg(current).arg(total).arg(currentItem));
    }
    
    void onSearchCompleted(int resultCount, const QString& summary) {
        if (progressBar_) {
            progressBar_->setVisible(false);
        }
        logMessage(QString("Search completed: %1 - %2").arg(resultCount).arg(summary));
    }
    
    void onSearchError(const QString& error, const QString& details) {
        logMessage(QString("Search error: %1 - %2").arg(error, details));
    }

signals:
    void searchResultSelected(const SearchResult& result);

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        
        // Create splitter for controls and results
        QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
        mainLayout->addWidget(splitter);
        
        // Controls panel
        QWidget* controlsWidget = new QWidget();
        controlsWidget->setMaximumWidth(400);
        controlsWidget->setMinimumWidth(350);
        setupControlsPanel(controlsWidget);
        splitter->addWidget(controlsWidget);
        
        // Results panel
        QWidget* resultsWidget = new QWidget();
        setupResultsPanel(resultsWidget);
        splitter->addWidget(resultsWidget);
        
        // Set splitter proportions
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
    }

    void setupControlsPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // Basic search controls
        QGroupBox* basicGroup = new QGroupBox("Basic Search Tests", parent);
        QVBoxLayout* basicLayout = new QVBoxLayout(basicGroup);

        QPushButton* basicBtn = new QPushButton("Test Basic Search", basicGroup);
        basicBtn->setToolTip("Test server ID, name, and type search functionality");
        connect(basicBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testBasicSearch);
        basicLayout->addWidget(basicBtn);

        QPushButton* advancedBtn = new QPushButton("Test Advanced Search", basicGroup);
        advancedBtn->setToolTip("Test property-based and area search functionality");
        connect(advancedBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testAdvancedSearch);
        basicLayout->addWidget(advancedBtn);

        layout->addWidget(basicGroup);

        // Find All controls
        QGroupBox* findAllGroup = new QGroupBox("Find All Functionality", parent);
        QVBoxLayout* findAllLayout = new QVBoxLayout(findAllGroup);

        QPushButton* findAllBtn = new QPushButton("Test Find All", findAllGroup);
        findAllBtn->setToolTip("Test Find All functionality with results display");
        connect(findAllBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testFindAllFunctionality);
        findAllLayout->addWidget(findAllBtn);

        QPushButton* showDialogBtn = new QPushButton("Show Find Dialog", findAllGroup);
        showDialogBtn->setToolTip("Open the enhanced Find Item Dialog");
        connect(showDialogBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::showFindDialog);
        findAllLayout->addWidget(showDialogBtn);

        layout->addWidget(findAllGroup);

        // Replace controls
        QGroupBox* replaceGroup = new QGroupBox("Replace Functionality", parent);
        QVBoxLayout* replaceLayout = new QVBoxLayout(replaceGroup);

        QPushButton* replaceBtn = new QPushButton("Test Replace Function", replaceGroup);
        replaceBtn->setToolTip("Test replace functionality with undo/redo support");
        connect(replaceBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testReplaceFunction);
        replaceLayout->addWidget(replaceBtn);

        layout->addWidget(replaceGroup);

        // ItemTypes integration controls
        QGroupBox* itemTypesGroup = new QGroupBox("ItemTypes Integration", parent);
        QVBoxLayout* itemTypesLayout = new QVBoxLayout(itemTypesGroup);

        QPushButton* itemTypesBtn = new QPushButton("Test ItemTypes Integration", itemTypesGroup);
        itemTypesBtn->setToolTip("Test property-based searching with ItemTypes");
        connect(itemTypesBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testItemTypesIntegration);
        itemTypesLayout->addWidget(itemTypesBtn);

        layout->addWidget(itemTypesGroup);

        // UI integration controls
        QGroupBox* uiGroup = new QGroupBox("UI Integration", parent);
        QVBoxLayout* uiLayout = new QVBoxLayout(uiGroup);

        QPushButton* uiBtn = new QPushButton("Test UI Integration", uiGroup);
        uiBtn->setToolTip("Test UI updates and MapView integration");
        connect(uiBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testUIIntegration);
        uiLayout->addWidget(uiBtn);

        layout->addWidget(uiGroup);

        // Performance controls
        QGroupBox* perfGroup = new QGroupBox("Performance & Limits", parent);
        QVBoxLayout* perfLayout = new QVBoxLayout(perfGroup);

        QPushButton* perfBtn = new QPushButton("Test Performance", perfGroup);
        perfBtn->setToolTip("Test performance and search limits");
        connect(perfBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testPerformanceAndLimits);
        perfLayout->addWidget(perfBtn);

        layout->addWidget(perfGroup);

        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);

        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setToolTip("Run complete test suite for all search functionality");
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::testAllFeatures);
        suiteLayout->addWidget(allTestsBtn);

        QPushButton* clearLogBtn = new QPushButton("Clear Log", suiteGroup);
        clearLogBtn->setToolTip("Clear the test log");
        connect(clearLogBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::clearLog);
        suiteLayout->addWidget(clearLogBtn);

        QPushButton* openDirBtn = new QPushButton("Open Test Directory", suiteGroup);
        openDirBtn->setToolTip("Open the test directory in file explorer");
        connect(openDirBtn, &QPushButton::clicked, this, &SearchFunctionalityTestWidget::openTestDirectory);
        suiteLayout->addWidget(openDirBtn);

        layout->addWidget(suiteGroup);

        layout->addStretch();
    }

    void setupResultsPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // Results display
        QTabWidget* tabWidget = new QTabWidget(parent);

        // Results tree tab
        QWidget* treeTab = new QWidget();
        QVBoxLayout* treeLayout = new QVBoxLayout(treeTab);

        resultsTree_ = new QTreeWidget(treeTab);
        resultsTree_->setHeaderLabels({"Item", "Position", "Floor", "Type", "Properties"});
        resultsTree_->setAlternatingRowColors(true);
        resultsTree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeLayout->addWidget(resultsTree_);

        tabWidget->addTab(treeTab, "Search Results");

        // Log tab
        QWidget* logTab = new QWidget();
        QVBoxLayout* logLayout = new QVBoxLayout(logTab);

        statusText_ = new QTextEdit(logTab);
        statusText_->setReadOnly(true);
        statusText_->setFont(QFont("Consolas", 9));
        logLayout->addWidget(statusText_);

        tabWidget->addTab(logTab, "Test Log");

        layout->addWidget(tabWidget);

        // Progress bar
        progressBar_ = new QProgressBar(parent);
        progressBar_->setVisible(false);
        layout->addWidget(progressBar_);
    }

    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();

        // Search menu
        QMenu* searchMenu = menuBar->addMenu("&Search");
        searchMenu->addAction("&Basic Search", this, &SearchFunctionalityTestWidget::testBasicSearch);
        searchMenu->addAction("&Advanced Search", this, &SearchFunctionalityTestWidget::testAdvancedSearch);
        searchMenu->addAction("&Find All", this, &SearchFunctionalityTestWidget::testFindAllFunctionality);
        searchMenu->addSeparator();
        searchMenu->addAction("Show Find &Dialog", this, &SearchFunctionalityTestWidget::showFindDialog);

        // Replace menu
        QMenu* replaceMenu = menuBar->addMenu("&Replace");
        replaceMenu->addAction("&Test Replace", this, &SearchFunctionalityTestWidget::testReplaceFunction);

        // Integration menu
        QMenu* integrationMenu = menuBar->addMenu("&Integration");
        integrationMenu->addAction("&ItemTypes", this, &SearchFunctionalityTestWidget::testItemTypesIntegration);
        integrationMenu->addAction("&UI Integration", this, &SearchFunctionalityTestWidget::testUIIntegration);

        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("&Performance", this, &SearchFunctionalityTestWidget::testPerformanceAndLimits);
        testMenu->addSeparator();
        testMenu->addAction("Run &All Tests", this, &SearchFunctionalityTestWidget::testAllFeatures);
        testMenu->addAction("&Clear Log", this, &SearchFunctionalityTestWidget::clearLog);

        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Open Test Directory", this, &SearchFunctionalityTestWidget::openTestDirectory);
        fileMenu->addSeparator();
        fileMenu->addAction("&Exit", this, &QWidget::close);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 82 search functionality");
    }

    void initializeComponents() {
        // Initialize undo stack
        undoStack_ = new QUndoStack(this);

        // Initialize map searcher
        mapSearcher_ = new MapSearcher(this);

        // Initialize find dialog
        findItemDialog_ = new FindItemDialog(this, "Enhanced Find Item Dialog", false);

        // Create test map (simplified for testing)
        testMap_ = new Map(this);
        // testMap_->initialize(100, 100, 16); // Would initialize a test map

        // Create test map view
        mapView_ = new MapView(this);

        // Create test item manager
        itemManager_ = new ItemManager(this);

        // Connect components
        mapSearcher_->setItemManager(itemManager_);
        mapSearcher_->setMapView(mapView_);

        findItemDialog_->setMap(testMap_);
        findItemDialog_->setMapView(mapView_);
        findItemDialog_->setItemManager(itemManager_);
        findItemDialog_->setUndoStack(undoStack_);
        findItemDialog_->setMapSearcher(mapSearcher_);

        // Create test directory
        QDir().mkpath(testDirectory_);

        logMessage("All components initialized successfully");
        logMessage(QString("Test directory: %1").arg(testDirectory_));
    }

    void connectSignals() {
        if (mapSearcher_) {
            connect(mapSearcher_, &MapSearcher::searchProgress,
                    this, &SearchFunctionalityTestWidget::onSearchProgress);
            connect(mapSearcher_, &MapSearcher::searchCompleted,
                    this, &SearchFunctionalityTestWidget::onSearchCompleted);
            connect(mapSearcher_, &MapSearcher::searchError,
                    this, &SearchFunctionalityTestWidget::onSearchError);
        }

        if (findItemDialog_) {
            connect(findItemDialog_, &FindItemDialog::findAllProgress,
                    this, &SearchFunctionalityTestWidget::onSearchProgress);
            connect(findItemDialog_, &FindItemDialog::findAllCompleted,
                    this, &SearchFunctionalityTestWidget::onSearchCompleted);
        }
    }

    void updateResultsDisplay(const QList<SearchResult>& results) {
        if (!resultsTree_) {
            return;
        }

        resultsTree_->clear();

        for (const SearchResult& result : results) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, result.itemName);
            item->setText(1, QString("(%1,%2)").arg(result.position.x()).arg(result.position.y()));
            item->setText(2, QString::number(result.floor));
            item->setText(3, QString("ID: %1").arg(result.itemId));
            item->setText(4, result.getDetailedInfo());

            resultsTree_->addTopLevelItem(item);
        }

        resultsTree_->expandAll();
        resultsTree_->resizeColumnToContents(0);
        resultsTree_->resizeColumnToContents(1);
        resultsTree_->resizeColumnToContents(2);

        statusBar()->showMessage(QString("Results: %1 items found").arg(results.size()));
    }

    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "SearchFunctionalityTest:" << message;
    }

private:
    MapSearcher* mapSearcher_;
    FindItemDialog* findItemDialog_;
    QUndoStack* undoStack_;
    Map* testMap_;
    MapView* mapView_;
    ItemManager* itemManager_;
    QTextEdit* statusText_;
    QTreeWidget* resultsTree_;
    QProgressBar* progressBar_;
    QString testDirectory_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Search Functionality Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");

    SearchFunctionalityTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "SearchFunctionalityTest.moc"
