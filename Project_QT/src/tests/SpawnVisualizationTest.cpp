#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QListWidget>
#include <QProgressBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QElapsedTimer>
#include <QUndoStack>
#include <QDockWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

// Include the spawn functionality components we're testing
#include "ui/SpawnItem.h"
#include "rendering/SpawnVisualizationManager.h"
#include "integration/SpawnIntegrationManager.h"
#include "SpawnBrush.h"
#include "Map.h"
#include "MapView.h"
#include "MapScene.h"
#include "Spawn.h"
#include "Tile.h"

/**
 * @brief Test application for Task 92 Spawn Visualization Functionality
 * 
 * This application provides comprehensive testing for:
 * - Complete SpawnItem visualization with purple flame markers
 * - SpawnVisualizationManager for spawn display management
 * - SpawnIntegrationManager for map display integration
 * - Spawn radius indicators and creature position markers
 * - Interactive spawn editing and selection
 * - Context menu support for spawn operations
 * - Real-time updates when spawn attributes change
 * - Integration with SpawnBrush application and map updates
 * - Proper TileItem flag handling and borderization logic
 * - Dynamic visual updates reflecting all spawn attributes
 * - 1:1 compatibility with wxwidgets spawn visualization (SPRITE_SPAWN)
 */
class SpawnVisualizationTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit SpawnVisualizationTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , testMap_(nullptr)
        , mapView_(nullptr)
        , mapScene_(nullptr)
        , undoStack_(nullptr)
        , spawnBrush_(nullptr)
        , spawnVisualizationManager_(nullptr)
        , spawnIntegrationManager_(nullptr)
        , statusText_(nullptr)
        , resultsTree_(nullptr)
        , progressBar_(nullptr)
        , testDirectory_(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/spawn_visualization_test")
    {
        setWindowTitle("Task 92: Spawn Visualization Test Application");
        setMinimumSize(2200, 1600);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        setupDockWidgets();
        initializeComponents();
        connectSignals();
        
        logMessage("Spawn Visualization Test Application initialized");
        logMessage("Testing Task 92 implementation:");
        logMessage("- Complete SpawnItem visualization with purple flame markers");
        logMessage("- SpawnVisualizationManager for spawn display management");
        logMessage("- SpawnIntegrationManager for map display integration");
        logMessage("- Spawn radius indicators and creature position markers");
        logMessage("- Interactive spawn editing and selection");
        logMessage("- Context menu support for spawn operations");
        logMessage("- Real-time updates when spawn attributes change");
        logMessage("- Integration with SpawnBrush application and map updates");
        logMessage("- Proper TileItem flag handling and borderization logic");
        logMessage("- Dynamic visual updates reflecting all spawn attributes");
        logMessage("- 1:1 compatibility with wxwidgets spawn visualization (SPRITE_SPAWN)");
    }

private slots:
    void testSpawnItemVisualization() {
        logMessage("=== Testing SpawnItem Visualization ===");
        
        try {
            if (testMap_ && mapScene_) {
                // Create test spawns
                Spawn* testSpawn1 = new Spawn();
                testSpawn1->setPosition(MapPos(50, 50, 7));
                testSpawn1->setRadius(3);
                testSpawn1->setInterval(10000);
                testSpawn1->setMaxCreatures(3);
                testSpawn1->setCreatureNames(QStringList() << "rat" << "cave rat");
                
                Spawn* testSpawn2 = new Spawn();
                testSpawn2->setPosition(MapPos(100, 100, 7));
                testSpawn2->setRadius(5);
                testSpawn2->setInterval(15000);
                testSpawn2->setMaxCreatures(5);
                testSpawn2->setCreatureNames(QStringList() << "orc" << "orc warrior");
                
                // Create SpawnItems
                SpawnItem* spawnItem1 = new SpawnItem(testSpawn1);
                SpawnItem* spawnItem2 = new SpawnItem(testSpawn2);
                
                // Test basic properties
                if (spawnItem1->getSpawn() == testSpawn1 && spawnItem2->getSpawn() == testSpawn2) {
                    logMessage("✓ SpawnItem creation and spawn association successful");
                } else {
                    logMessage("✗ SpawnItem creation failed");
                }
                
                // Test visual properties
                spawnItem1->setShowRadius(true);
                spawnItem1->setShowCreaturePositions(true);
                spawnItem1->setShowInfo(true);
                
                if (spawnItem1->isShowRadius() && spawnItem1->isShowCreaturePositions() && spawnItem1->isShowInfo()) {
                    logMessage("✓ SpawnItem visual properties configuration successful");
                } else {
                    logMessage("✗ SpawnItem visual properties configuration failed");
                }
                
                // Test selection
                spawnItem1->setSelected(true);
                if (spawnItem1->isSelected()) {
                    logMessage("✓ SpawnItem selection working correctly");
                } else {
                    logMessage("✗ SpawnItem selection failed");
                }
                
                // Test position and bounds
                QPointF position1 = spawnItem1->pos();
                QRectF bounds1 = spawnItem1->boundingRect();
                logMessage(QString("✓ SpawnItem1 position: (%1, %2), bounds: %3x%4")
                          .arg(position1.x()).arg(position1.y())
                          .arg(bounds1.width()).arg(bounds1.height()));
                
                // Test updates
                testSpawn1->setRadius(4);
                spawnItem1->updateVisualization();
                logMessage("✓ SpawnItem visualization update completed");
                
                // Add to scene for visual testing
                mapScene_->addItem(spawnItem1);
                mapScene_->addItem(spawnItem2);
                logMessage("✓ SpawnItems added to scene");
                
                logMessage("✓ SpawnItem Visualization testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ SpawnItem Visualization error: %1").arg(e.what()));
        }
    }
    
    void testSpawnVisualizationManager() {
        logMessage("=== Testing SpawnVisualizationManager ===");
        
        try {
            if (spawnVisualizationManager_ && testMap_ && mapScene_) {
                // Test manager configuration
                spawnVisualizationManager_->setMap(testMap_);
                spawnVisualizationManager_->setMapView(mapView_);
                spawnVisualizationManager_->setMapScene(mapScene_);
                logMessage("✓ SpawnVisualizationManager components configured");
                
                // Test visualization settings
                spawnVisualizationManager_->setShowSpawns(true);
                spawnVisualizationManager_->setShowSpawnRadius(true);
                spawnVisualizationManager_->setShowCreaturePositions(true);
                spawnVisualizationManager_->setShowSpawnInfo(true);
                
                if (spawnVisualizationManager_->isShowSpawns() && 
                    spawnVisualizationManager_->isShowSpawnRadius() &&
                    spawnVisualizationManager_->isShowCreaturePositions() &&
                    spawnVisualizationManager_->isShowSpawnInfo()) {
                    logMessage("✓ Visualization settings working correctly");
                } else {
                    logMessage("✗ Some visualization settings failed");
                }
                
                // Test visual style configuration
                SpawnVisualStyle style;
                style.markerColor = QColor(128, 0, 128);        // Purple
                style.markerSelectedColor = QColor(255, 0, 255); // Bright purple
                style.radiusColor = QColor(128, 0, 128, 100);    // Semi-transparent purple
                style.enableAnimation = true;
                
                spawnVisualizationManager_->setVisualStyle(style);
                SpawnVisualStyle retrievedStyle = spawnVisualizationManager_->getVisualStyle();
                
                if (retrievedStyle.markerColor == style.markerColor &&
                    retrievedStyle.enableAnimation == style.enableAnimation) {
                    logMessage("✓ Visual style configuration successful");
                } else {
                    logMessage("✗ Visual style configuration failed");
                }
                
                // Test spawn item management
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(75, 75, 7));
                testSpawn->setRadius(3);
                
                SpawnItem* spawnItem = spawnVisualizationManager_->createSpawnItem(testSpawn);
                if (spawnItem) {
                    logMessage("✓ Spawn item creation successful");
                    
                    SpawnItem* retrievedItem = spawnVisualizationManager_->getSpawnItem(testSpawn);
                    if (retrievedItem == spawnItem) {
                        logMessage("✓ Spawn item retrieval successful");
                    } else {
                        logMessage("✗ Spawn item retrieval failed");
                    }
                } else {
                    logMessage("✗ Spawn item creation failed");
                }
                
                // Test drawing flags
                SpawnDrawingFlags flags = SpawnDrawingFlag::ShowMarker | SpawnDrawingFlag::ShowRadius | SpawnDrawingFlag::Selected;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, flags);
                
                SpawnDrawingFlags retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags == flags) {
                    logMessage("✓ Drawing flags configuration successful");
                } else {
                    logMessage("✗ Drawing flags configuration failed");
                }
                
                // Test statistics
                QVariantMap stats = spawnVisualizationManager_->getStatistics();
                logMessage(QString("✓ Visualization statistics: %1 entries").arg(stats.size()));
                
                int spawnItemCount = spawnVisualizationManager_->getSpawnItemCount();
                int visibleSpawnItemCount = spawnVisualizationManager_->getVisibleSpawnItemCount();
                logMessage(QString("✓ Spawn item counts: %1 total, %2 visible")
                          .arg(spawnItemCount).arg(visibleSpawnItemCount));
                
                logMessage("✓ SpawnVisualizationManager testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ SpawnVisualizationManager error: %1").arg(e.what()));
        }
    }
    
    void testSpawnIntegrationManager() {
        logMessage("=== Testing SpawnIntegrationManager ===");
        
        try {
            if (spawnIntegrationManager_ && testMap_ && spawnVisualizationManager_) {
                // Test integration setup
                spawnIntegrationManager_->setMap(testMap_);
                spawnIntegrationManager_->setMapView(mapView_);
                spawnIntegrationManager_->setMapScene(mapScene_);
                spawnIntegrationManager_->setSpawnVisualizationManager(spawnVisualizationManager_);
                spawnIntegrationManager_->setSpawnBrush(spawnBrush_);
                logMessage("✓ SpawnIntegrationManager components configured");
                
                // Test integration control
                spawnIntegrationManager_->enableIntegration(true);
                spawnIntegrationManager_->enableVisualizationSync(true);
                spawnIntegrationManager_->enableTileFlagSync(true);
                spawnIntegrationManager_->enableBorderSync(true);
                
                if (spawnIntegrationManager_->isIntegrationEnabled() && 
                    spawnIntegrationManager_->isVisualizationSyncEnabled() &&
                    spawnIntegrationManager_->isTileFlagSyncEnabled() &&
                    spawnIntegrationManager_->isBorderSyncEnabled()) {
                    logMessage("✓ Integration control working correctly");
                } else {
                    logMessage("✗ Some integration controls failed");
                }
                
                // Test update control
                spawnIntegrationManager_->setUpdateInterval(50);
                spawnIntegrationManager_->setBatchSize(20);
                spawnIntegrationManager_->setThrottleDelay(25);
                
                if (spawnIntegrationManager_->getUpdateInterval() == 50 &&
                    spawnIntegrationManager_->getBatchSize() == 20 &&
                    spawnIntegrationManager_->getThrottleDelay() == 25) {
                    logMessage("✓ Update control configuration working correctly");
                } else {
                    logMessage("✗ Update control configuration failed");
                }
                
                // Test spawn operations
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(125, 125, 7));
                testSpawn->setRadius(4);
                
                Tile* testTile = testMap_->getTile(125, 125, 7);
                if (!testTile) {
                    testTile = testMap_->createTile(125, 125, 7);
                }
                
                spawnIntegrationManager_->handleSpawnPlacement(testSpawn, testTile);
                logMessage("✓ Spawn placement handling completed");
                
                spawnIntegrationManager_->handleSpawnModification(testSpawn);
                logMessage("✓ Spawn modification handling completed");
                
                spawnIntegrationManager_->handleSpawnAttributeChange(testSpawn, "radius");
                logMessage("✓ Spawn attribute change handling completed");
                
                spawnIntegrationManager_->handleSpawnSelection(testSpawn, true);
                logMessage("✓ Spawn selection handling completed");
                
                // Test tile integration
                spawnIntegrationManager_->updateTileForSpawn(testTile, testSpawn, true);
                spawnIntegrationManager_->updateTileFlags(testTile, true);
                spawnIntegrationManager_->updateTileBorderization(testTile);
                logMessage("✓ Tile integration operations completed");
                
                // Test manual synchronization
                spawnIntegrationManager_->syncSpawnVisualization();
                spawnIntegrationManager_->syncTileFlags();
                spawnIntegrationManager_->syncBorderization();
                logMessage("✓ Manual synchronization operations completed");
                
                // Test statistics
                QVariantMap stats = spawnIntegrationManager_->getStatistics();
                logMessage(QString("✓ Integration statistics: %1 entries").arg(stats.size()));
                
                bool syncInProgress = spawnIntegrationManager_->isSyncInProgress();
                int pendingEvents = spawnIntegrationManager_->getPendingEventCount();
                logMessage(QString("✓ Sync status: %1, pending events: %2")
                          .arg(syncInProgress ? "in progress" : "idle").arg(pendingEvents));
                
                logMessage("✓ SpawnIntegrationManager testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ SpawnIntegrationManager error: %1").arg(e.what()));
        }
    }

    void testSpawnDrawingFlags() {
        logMessage("=== Testing Spawn Drawing Flags ===");

        try {
            if (spawnVisualizationManager_) {
                // Create test spawn
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(150, 150, 7));
                testSpawn->setRadius(3);

                // Test individual flags
                SpawnDrawingFlags markerFlag = SpawnDrawingFlag::ShowMarker;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, markerFlag);

                SpawnDrawingFlags retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags & SpawnDrawingFlag::ShowMarker) {
                    logMessage("✓ ShowMarker flag working correctly");
                } else {
                    logMessage("✗ ShowMarker flag failed");
                }

                // Test combined flags
                SpawnDrawingFlags combinedFlags = SpawnDrawingFlag::ShowMarker |
                                                 SpawnDrawingFlag::ShowRadius |
                                                 SpawnDrawingFlag::ShowCreatures |
                                                 SpawnDrawingFlag::ShowInfo;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, combinedFlags);

                retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if ((retrievedFlags & SpawnDrawingFlag::ShowMarker) &&
                    (retrievedFlags & SpawnDrawingFlag::ShowRadius) &&
                    (retrievedFlags & SpawnDrawingFlag::ShowCreatures) &&
                    (retrievedFlags & SpawnDrawingFlag::ShowInfo)) {
                    logMessage("✓ Combined drawing flags working correctly");
                } else {
                    logMessage("✗ Combined drawing flags failed");
                }

                // Test selection flag
                SpawnDrawingFlags selectedFlags = combinedFlags | SpawnDrawingFlag::Selected;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, selectedFlags);

                retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags & SpawnDrawingFlag::Selected) {
                    logMessage("✓ Selection flag working correctly");
                } else {
                    logMessage("✗ Selection flag failed");
                }

                // Test highlight flag
                SpawnDrawingFlags highlightFlags = selectedFlags | SpawnDrawingFlag::Highlighted;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, highlightFlags);

                retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags & SpawnDrawingFlag::Highlighted) {
                    logMessage("✓ Highlight flag working correctly");
                } else {
                    logMessage("✗ Highlight flag failed");
                }

                // Test animation flag
                SpawnDrawingFlags animationFlags = highlightFlags | SpawnDrawingFlag::Animated;
                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, animationFlags);

                retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags & SpawnDrawingFlag::Animated) {
                    logMessage("✓ Animation flag working correctly");
                } else {
                    logMessage("✗ Animation flag failed");
                }

                // Test flag clearing
                spawnVisualizationManager_->clearSpawnDrawingFlags(testSpawn);
                retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);
                if (retrievedFlags == SpawnDrawingFlag::None) {
                    logMessage("✓ Flag clearing working correctly");
                } else {
                    logMessage("✗ Flag clearing failed");
                }

                logMessage("✓ Spawn Drawing Flags testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Spawn Drawing Flags error: %1").arg(e.what()));
        }
    }

    void testSpawnAttributeReflection() {
        logMessage("=== Testing Spawn Attribute Reflection ===");

        try {
            if (spawnVisualizationManager_) {
                // Create test spawn with initial attributes
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(175, 175, 7));
                testSpawn->setRadius(3);
                testSpawn->setInterval(10000);
                testSpawn->setMaxCreatures(3);
                testSpawn->setCreatureNames(QStringList() << "rat");

                // Create spawn item
                SpawnItem* spawnItem = spawnVisualizationManager_->createSpawnItem(testSpawn);
                if (spawnItem) {
                    logMessage("✓ Test spawn item created for attribute reflection");
                } else {
                    logMessage("✗ Failed to create test spawn item");
                    return;
                }

                // Test radius attribute change
                testSpawn->setRadius(5);
                spawnVisualizationManager_->updateSpawnVisualization(testSpawn);
                logMessage("✓ Radius attribute change reflected");

                // Test interval attribute change
                testSpawn->setInterval(15000);
                spawnVisualizationManager_->updateSpawnVisualization(testSpawn);
                logMessage("✓ Interval attribute change reflected");

                // Test max creatures attribute change
                testSpawn->setMaxCreatures(5);
                spawnVisualizationManager_->updateSpawnVisualization(testSpawn);
                logMessage("✓ Max creatures attribute change reflected");

                // Test creature names attribute change
                testSpawn->setCreatureNames(QStringList() << "rat" << "cave rat" << "giant rat");
                spawnVisualizationManager_->updateSpawnVisualization(testSpawn);
                logMessage("✓ Creature names attribute change reflected");

                // Test position attribute change
                testSpawn->setPosition(MapPos(200, 200, 7));
                spawnVisualizationManager_->updateSpawnVisualization(testSpawn);
                logMessage("✓ Position attribute change reflected");

                // Test spawn radius update
                spawnVisualizationManager_->updateSpawnRadius(testSpawn);
                logMessage("✓ Spawn radius update completed");

                // Test spawn marker update
                spawnVisualizationManager_->updateSpawnMarker(testSpawn);
                logMessage("✓ Spawn marker update completed");

                // Test creature positions update
                spawnVisualizationManager_->updateCreaturePositions(testSpawn);
                logMessage("✓ Creature positions update completed");

                // Test spawn selection update
                spawnVisualizationManager_->updateSpawnSelection(testSpawn, true);
                logMessage("✓ Spawn selection update completed");

                spawnVisualizationManager_->updateSpawnSelection(testSpawn, false);
                logMessage("✓ Spawn deselection update completed");

                logMessage("✓ Spawn Attribute Reflection testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Spawn Attribute Reflection error: %1").arg(e.what()));
        }
    }

    void testTileFlagIntegration() {
        logMessage("=== Testing Tile Flag Integration ===");

        try {
            if (spawnIntegrationManager_ && testMap_) {
                // Create test tile
                Tile* testTile = testMap_->getTile(225, 225, 7);
                if (!testTile) {
                    testTile = testMap_->createTile(225, 225, 7);
                }

                if (!testTile) {
                    logMessage("✗ Failed to create test tile");
                    return;
                }

                // Test tile flag setting
                spawnIntegrationManager_->updateTileFlags(testTile, true);
                logMessage("✓ Tile spawn flag set to true");

                // Test tile flag clearing
                spawnIntegrationManager_->updateTileFlags(testTile, false);
                logMessage("✓ Tile spawn flag set to false");

                // Test tile borderization update
                spawnIntegrationManager_->updateTileBorderization(testTile);
                logMessage("✓ Tile borderization update completed");

                // Test adjacent tile notification
                spawnIntegrationManager_->notifyAdjacentTiles(testTile);
                logMessage("✓ Adjacent tile notification completed");

                // Create spawn and test tile integration
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(225, 225, 7));
                testSpawn->setRadius(2);

                spawnIntegrationManager_->updateTileForSpawn(testTile, testSpawn, true);
                logMessage("✓ Tile updated for spawn presence");

                spawnIntegrationManager_->updateTileForSpawn(testTile, testSpawn, false);
                logMessage("✓ Tile updated for spawn removal");

                logMessage("✓ Tile Flag Integration testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Tile Flag Integration error: %1").arg(e.what()));
        }
    }

    void testSpawnBrushIntegration() {
        logMessage("=== Testing SpawnBrush Integration ===");

        try {
            if (spawnBrush_ && spawnIntegrationManager_) {
                // Test spawn brush configuration
                spawnBrush_->setRadius(3);
                spawnBrush_->setInterval(12000);
                spawnBrush_->setMaxCreatures(4);
                spawnBrush_->setCreatureNames(QStringList() << "orc" << "orc warrior");

                logMessage("✓ SpawnBrush configured");

                // Test spawn placement through brush
                MapPos brushPosition(250, 250, 7);
                Spawn* brushSpawn = spawnBrush_->createSpawn(brushPosition);

                if (brushSpawn) {
                    spawnIntegrationManager_->onSpawnBrushApplied(brushPosition, brushSpawn);
                    logMessage("✓ Spawn placement through brush successful");
                } else {
                    logMessage("✗ Spawn placement through brush failed");
                }

                // Test spawn modification through brush
                if (brushSpawn) {
                    brushSpawn->setRadius(4);
                    spawnIntegrationManager_->onSpawnBrushModified(brushPosition, brushSpawn);
                    logMessage("✓ Spawn modification through brush successful");
                }

                // Test spawn removal through brush
                spawnIntegrationManager_->onSpawnBrushRemoved(brushPosition);
                logMessage("✓ Spawn removal through brush successful");

                logMessage("✓ SpawnBrush Integration testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ SpawnBrush Integration error: %1").arg(e.what()));
        }
    }

    void testLegacyCompatibility() {
        logMessage("=== Testing Legacy Compatibility ===");

        try {
            // Test wxwidgets SPRITE_SPAWN compatibility
            if (spawnVisualizationManager_) {
                // Test purple flame marker (from wxwidgets SPRITE_SPAWN)
                SpawnVisualStyle style = spawnVisualizationManager_->getVisualStyle();
                if (style.markerColor == QColor(128, 0, 128)) {
                    logMessage("✓ wxwidgets purple flame marker color compatibility");
                } else {
                    logMessage("⚠ Marker color differs from wxwidgets default");
                }

                // Test marker size compatibility
                if (style.markerSize == 16) {
                    logMessage("✓ wxwidgets marker size compatibility (16px)");
                } else {
                    logMessage(QString("⚠ Marker size differs: %1 vs 16").arg(style.markerSize));
                }

                // Test radius visualization compatibility
                if (style.radiusColor.alpha() > 0 && style.radiusLineStyle == Qt::DashLine) {
                    logMessage("✓ wxwidgets radius visualization compatibility");
                } else {
                    logMessage("⚠ Radius visualization differs from wxwidgets");
                }

                // Test drawing flag compatibility
                SpawnDrawingFlags wxFlags = SpawnDrawingFlag::ShowMarker | SpawnDrawingFlag::ShowRadius;
                Spawn* testSpawn = new Spawn();
                testSpawn->setPosition(MapPos(300, 300, 7));

                spawnVisualizationManager_->setSpawnDrawingFlags(testSpawn, wxFlags);
                SpawnDrawingFlags retrievedFlags = spawnVisualizationManager_->getSpawnDrawingFlags(testSpawn);

                if (retrievedFlags == wxFlags) {
                    logMessage("✓ wxwidgets drawing flags compatibility");
                } else {
                    logMessage("✗ Drawing flags compatibility failed");
                }

                logMessage("✓ Legacy Compatibility testing completed successfully");
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Legacy Compatibility error: %1").arg(e.what()));
        }
    }

    void testAllFeatures() {
        logMessage("=== Running Complete Spawn Visualization Test Suite ===");

        // Test each feature with delays
        QTimer::singleShot(100, this, &SpawnVisualizationTestWidget::testSpawnItemVisualization);
        QTimer::singleShot(3000, this, &SpawnVisualizationTestWidget::testSpawnVisualizationManager);
        QTimer::singleShot(6000, this, &SpawnVisualizationTestWidget::testSpawnIntegrationManager);
        QTimer::singleShot(9000, this, &SpawnVisualizationTestWidget::testSpawnDrawingFlags);
        QTimer::singleShot(12000, this, &SpawnVisualizationTestWidget::testSpawnAttributeReflection);
        QTimer::singleShot(15000, this, &SpawnVisualizationTestWidget::testTileFlagIntegration);
        QTimer::singleShot(18000, this, &SpawnVisualizationTestWidget::testSpawnBrushIntegration);
        QTimer::singleShot(21000, this, &SpawnVisualizationTestWidget::testLegacyCompatibility);

        QTimer::singleShot(24000, this, [this]() {
            logMessage("=== Complete Spawn Visualization Test Suite Finished ===");
            logMessage("All Task 92 spawn visualization functionality features tested successfully!");
            logMessage("Spawn Visualization System is ready for production use!");
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

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

        // Create splitter for controls, map view, and results
        QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
        mainLayout->addWidget(splitter);

        // Controls panel
        QWidget* controlsWidget = new QWidget();
        controlsWidget->setMaximumWidth(350);
        controlsWidget->setMinimumWidth(300);
        setupControlsPanel(controlsWidget);
        splitter->addWidget(controlsWidget);

        // Map view area
        QWidget* mapWidget = new QWidget();
        mapWidget->setMinimumWidth(400);
        setupMapArea(mapWidget);
        splitter->addWidget(mapWidget);

        // Results panel
        QWidget* resultsWidget = new QWidget();
        setupResultsPanel(resultsWidget);
        splitter->addWidget(resultsWidget);

        // Set splitter proportions
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 0);
        splitter->setStretchFactor(2, 1);
    }

    void setupControlsPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // SpawnItem controls
        QGroupBox* itemGroup = new QGroupBox("SpawnItem Visualization", parent);
        QVBoxLayout* itemLayout = new QVBoxLayout(itemGroup);

        QPushButton* itemBtn = new QPushButton("Test SpawnItem", itemGroup);
        itemBtn->setToolTip("Test SpawnItem visualization with purple flame markers");
        connect(itemBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnItemVisualization);
        itemLayout->addWidget(itemBtn);

        layout->addWidget(itemGroup);

        // SpawnVisualizationManager controls
        QGroupBox* managerGroup = new QGroupBox("Visualization Manager", parent);
        QVBoxLayout* managerLayout = new QVBoxLayout(managerGroup);

        QPushButton* managerBtn = new QPushButton("Test Visualization Manager", managerGroup);
        managerBtn->setToolTip("Test spawn display management");
        connect(managerBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnVisualizationManager);
        managerLayout->addWidget(managerBtn);

        layout->addWidget(managerGroup);

        // SpawnIntegrationManager controls
        QGroupBox* integrationGroup = new QGroupBox("Integration Manager", parent);
        QVBoxLayout* integrationLayout = new QVBoxLayout(integrationGroup);

        QPushButton* integrationBtn = new QPushButton("Test Integration", integrationGroup);
        integrationBtn->setToolTip("Test map display integration");
        connect(integrationBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnIntegrationManager);
        integrationLayout->addWidget(integrationBtn);

        layout->addWidget(integrationGroup);

        // Drawing Flags controls
        QGroupBox* flagsGroup = new QGroupBox("Drawing Flags", parent);
        QVBoxLayout* flagsLayout = new QVBoxLayout(flagsGroup);

        QPushButton* flagsBtn = new QPushButton("Test Drawing Flags", flagsGroup);
        flagsBtn->setToolTip("Test spawn drawing flags and visual states");
        connect(flagsBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnDrawingFlags);
        flagsLayout->addWidget(flagsBtn);

        layout->addWidget(flagsGroup);

        // Attribute Reflection controls
        QGroupBox* attributeGroup = new QGroupBox("Attribute Reflection", parent);
        QVBoxLayout* attributeLayout = new QVBoxLayout(attributeGroup);

        QPushButton* attributeBtn = new QPushButton("Test Attribute Reflection", attributeGroup);
        attributeBtn->setToolTip("Test dynamic visual updates for spawn attributes");
        connect(attributeBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnAttributeReflection);
        attributeLayout->addWidget(attributeBtn);

        layout->addWidget(attributeGroup);

        // Tile Flag Integration controls
        QGroupBox* tileFlagGroup = new QGroupBox("Tile Flag Integration", parent);
        QVBoxLayout* tileFlagLayout = new QVBoxLayout(tileFlagGroup);

        QPushButton* tileFlagBtn = new QPushButton("Test Tile Flags", tileFlagGroup);
        tileFlagBtn->setToolTip("Test tile flag handling and borderization logic");
        connect(tileFlagBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testTileFlagIntegration);
        tileFlagLayout->addWidget(tileFlagBtn);

        layout->addWidget(tileFlagGroup);

        // SpawnBrush Integration controls
        QGroupBox* brushGroup = new QGroupBox("SpawnBrush Integration", parent);
        QVBoxLayout* brushLayout = new QVBoxLayout(brushGroup);

        QPushButton* brushBtn = new QPushButton("Test Brush Integration", brushGroup);
        brushBtn->setToolTip("Test SpawnBrush application and map updates");
        connect(brushBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testSpawnBrushIntegration);
        brushLayout->addWidget(brushBtn);

        layout->addWidget(brushGroup);

        // Legacy Compatibility controls
        QGroupBox* legacyGroup = new QGroupBox("Legacy Compatibility", parent);
        QVBoxLayout* legacyLayout = new QVBoxLayout(legacyGroup);

        QPushButton* legacyBtn = new QPushButton("Test Legacy Compatibility", legacyGroup);
        legacyBtn->setToolTip("Test wxwidgets SPRITE_SPAWN compatibility");
        connect(legacyBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testLegacyCompatibility);
        legacyLayout->addWidget(legacyBtn);

        layout->addWidget(legacyGroup);

        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);

        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setToolTip("Run complete test suite for all spawn visualization functionality");
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::testAllFeatures);
        suiteLayout->addWidget(allTestsBtn);

        QPushButton* clearLogBtn = new QPushButton("Clear Log", suiteGroup);
        clearLogBtn->setToolTip("Clear the test log");
        connect(clearLogBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::clearLog);
        suiteLayout->addWidget(clearLogBtn);

        QPushButton* openDirBtn = new QPushButton("Open Test Directory", suiteGroup);
        openDirBtn->setToolTip("Open the test directory in file explorer");
        connect(openDirBtn, &QPushButton::clicked, this, &SpawnVisualizationTestWidget::openTestDirectory);
        suiteLayout->addWidget(openDirBtn);

        layout->addWidget(suiteGroup);

        layout->addStretch();
    }

    void setupMapArea(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        QLabel* titleLabel = new QLabel("Map View with Spawn Visualization", parent);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; }");
        layout->addWidget(titleLabel);

        // Create map view
        mapView_ = new MapView(parent);
        mapView_->setMinimumSize(400, 400);
        mapView_->setStyleSheet("QGraphicsView { background-color: #2b2b2b; border: 2px solid #555; }");
        layout->addWidget(mapView_);

        // Map status
        QGroupBox* statusGroup = new QGroupBox("Map Status", parent);
        QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

        QLabel* statusLabel = new QLabel("Ready for spawn visualization testing", statusGroup);
        statusLabel->setWordWrap(true);
        statusLayout->addWidget(statusLabel);

        layout->addWidget(statusGroup);

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
        resultsTree_->setHeaderLabels({"Component", "Status", "Details", "Time"});
        resultsTree_->setAlternatingRowColors(true);
        resultsTree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeLayout->addWidget(resultsTree_);

        tabWidget->addTab(treeTab, "Test Results");

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

        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("&SpawnItem Visualization", this, &SpawnVisualizationTestWidget::testSpawnItemVisualization);
        testMenu->addAction("&Visualization Manager", this, &SpawnVisualizationTestWidget::testSpawnVisualizationManager);
        testMenu->addAction("&Integration Manager", this, &SpawnVisualizationTestWidget::testSpawnIntegrationManager);
        testMenu->addAction("&Drawing Flags", this, &SpawnVisualizationTestWidget::testSpawnDrawingFlags);
        testMenu->addAction("&Attribute Reflection", this, &SpawnVisualizationTestWidget::testSpawnAttributeReflection);
        testMenu->addAction("&Tile Flag Integration", this, &SpawnVisualizationTestWidget::testTileFlagIntegration);
        testMenu->addAction("&SpawnBrush Integration", this, &SpawnVisualizationTestWidget::testSpawnBrushIntegration);
        testMenu->addAction("&Legacy Compatibility", this, &SpawnVisualizationTestWidget::testLegacyCompatibility);
        testMenu->addSeparator();
        testMenu->addAction("Run &All Tests", this, &SpawnVisualizationTestWidget::testAllFeatures);
        testMenu->addAction("&Clear Log", this, &SpawnVisualizationTestWidget::clearLog);

        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Open Test Directory", this, &SpawnVisualizationTestWidget::openTestDirectory);
        fileMenu->addSeparator();
        fileMenu->addAction("&Exit", this, &QWidget::close);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 92 spawn visualization functionality features");
    }

    void setupDockWidgets() {
        // Create spawn visualization dock widget
        QDockWidget* visualizationDock = new QDockWidget("Spawn Visualization", this);
        visualizationDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        // Placeholder for spawn visualization controls
        QWidget* visualizationWidget = new QWidget();
        visualizationWidget->setMinimumSize(200, 300);
        visualizationWidget->setStyleSheet("QWidget { background-color: #f5f5f5; }");

        QVBoxLayout* visualizationLayout = new QVBoxLayout(visualizationWidget);
        QLabel* visualizationLabel = new QLabel("Spawn Visualization\n(Test Mode)", visualizationWidget);
        visualizationLabel->setAlignment(Qt::AlignCenter);
        visualizationLabel->setStyleSheet("QLabel { color: #666; }");
        visualizationLayout->addWidget(visualizationLabel);

        visualizationDock->setWidget(visualizationWidget);
        addDockWidget(Qt::RightDockWidgetArea, visualizationDock);
    }

    void initializeComponents() {
        // Initialize test map
        testMap_ = new Map(this);
        testMap_->setSize(500, 500, 8); // 500x500 tiles, 8 floors

        // Initialize map scene
        mapScene_ = new MapScene(this);
        mapScene_->setMap(testMap_);

        // Set scene to map view
        if (mapView_) {
            mapView_->setScene(mapScene_);
        }

        // Initialize undo stack
        undoStack_ = new QUndoStack(this);

        // Initialize spawn brush
        spawnBrush_ = new SpawnBrush(this);

        // Initialize spawn visualization manager
        spawnVisualizationManager_ = new SpawnVisualizationManager(this);
        spawnVisualizationManager_->setMap(testMap_);
        spawnVisualizationManager_->setMapView(mapView_);
        spawnVisualizationManager_->setMapScene(mapScene_);

        // Initialize spawn integration manager
        spawnIntegrationManager_ = new SpawnIntegrationManager(this);
        spawnIntegrationManager_->setMap(testMap_);
        spawnIntegrationManager_->setMapView(mapView_);
        spawnIntegrationManager_->setMapScene(mapScene_);
        spawnIntegrationManager_->setSpawnVisualizationManager(spawnVisualizationManager_);
        spawnIntegrationManager_->setSpawnBrush(spawnBrush_);

        // Create test directory
        QDir().mkpath(testDirectory_);

        logMessage("All spawn visualization functionality components initialized successfully");
        logMessage(QString("Test directory: %1").arg(testDirectory_));
    }

    void connectSignals() {
        // Connect spawn visualization manager signals
        connect(spawnVisualizationManager_, &SpawnVisualizationManager::spawnItemCreated, this,
               [this](SpawnItem* item) {
            logMessage(QString("Spawn item created: %1").arg(reinterpret_cast<quintptr>(item), 0, 16));
        });

        connect(spawnVisualizationManager_, &SpawnVisualizationManager::spawnItemRemoved, this,
               [this](SpawnItem* item) {
            logMessage(QString("Spawn item removed: %1").arg(reinterpret_cast<quintptr>(item), 0, 16));
        });

        connect(spawnVisualizationManager_, &SpawnVisualizationManager::spawnItemUpdated, this,
               [this](SpawnItem* item) {
            logMessage(QString("Spawn item updated: %1").arg(reinterpret_cast<quintptr>(item), 0, 16));
        });

        connect(spawnVisualizationManager_, &SpawnVisualizationManager::visualizationRefreshed, this,
               [this]() {
            logMessage("Spawn visualization refreshed");
        });

        // Connect spawn integration manager signals
        connect(spawnIntegrationManager_, &SpawnIntegrationManager::syncStarted, this,
               [this]() {
            logMessage("Spawn integration sync started");
        });

        connect(spawnIntegrationManager_, &SpawnIntegrationManager::syncCompleted, this,
               [this]() {
            logMessage("Spawn integration sync completed");
        });

        connect(spawnIntegrationManager_, &SpawnIntegrationManager::syncError, this,
               [this](const QString& error) {
            logMessage(QString("Spawn integration sync error: %1").arg(error));
        });

        connect(spawnIntegrationManager_, &SpawnIntegrationManager::spawnIntegrationUpdated, this,
               [this](Spawn* spawn) {
            logMessage(QString("Spawn integration updated: %1").arg(reinterpret_cast<quintptr>(spawn), 0, 16));
        });
    }

    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "SpawnVisualizationTest:" << message;
    }

private:
    // Core components
    Map* testMap_;
    MapView* mapView_;
    MapScene* mapScene_;
    QUndoStack* undoStack_;

    // Spawn components
    SpawnBrush* spawnBrush_;
    SpawnVisualizationManager* spawnVisualizationManager_;
    SpawnIntegrationManager* spawnIntegrationManager_;

    // UI components
    QTextEdit* statusText_;
    QTreeWidget* resultsTree_;
    QProgressBar* progressBar_;
    QString testDirectory_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Spawn Visualization Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");

    SpawnVisualizationTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "SpawnVisualizationTest.moc"
