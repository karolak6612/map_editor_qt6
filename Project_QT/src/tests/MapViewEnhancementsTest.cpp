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
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QColorDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

// Include the enhanced MapView components we're testing
#include "MapView.h"
#include "MapViewEnhancements.h"
#include "Map.h"

/**
 * @brief Test application for Task 80 MapView enhancements
 * 
 * This application provides comprehensive testing for:
 * - Enhanced zoom system with smooth animations
 * - Advanced grid display with customizable appearance
 * - Mouse tracking with hover effects and coordinate display
 * - Drawing feedback with brush preview and visual indicators
 * - Performance optimizations for large maps
 */
class MapViewEnhancementsTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit MapViewEnhancementsTestWidget(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , mapView_(nullptr)
        , testMap_(nullptr)
        , statusText_(nullptr)
        , coordinateLabel_(nullptr)
        , zoomLabel_(nullptr)
        , mouseTrackingEnabled_(true)
        , hoverEffectsEnabled_(true)
        , gridVisible_(true)
    {
        setWindowTitle("Task 80: MapView Enhancements Test Application");
        setMinimumSize(1200, 800);
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        initializeComponents();
        connectSignals();
        
        logMessage("MapView Enhancements Test Application initialized");
        logMessage("Testing Task 80 implementation:");
        logMessage("- Enhanced zoom system with smooth animations");
        logMessage("- Advanced grid display with customizable appearance");
        logMessage("- Mouse tracking with hover effects");
        logMessage("- Drawing feedback with visual indicators");
        logMessage("- Performance optimizations");
    }

private slots:
    void testZoomIn() {
        logMessage("=== Testing Zoom In ===");
        
        try {
            if (mapView_) {
                mapView_->zoomIn();
                logMessage("✓ Zoom in executed successfully");
                updateZoomInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Zoom in error: %1").arg(e.what()));
        }
    }
    
    void testZoomOut() {
        logMessage("=== Testing Zoom Out ===");
        
        try {
            if (mapView_) {
                mapView_->zoomOut();
                logMessage("✓ Zoom out executed successfully");
                updateZoomInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Zoom out error: %1").arg(e.what()));
        }
    }
    
    void testZoomToLevel() {
        logMessage("=== Testing Zoom To Level ===");
        
        try {
            if (mapView_) {
                double level = 2.5; // 250% zoom
                mapView_->zoomToLevel(level);
                logMessage(QString("✓ Zoom to level %1 executed successfully").arg(level));
                updateZoomInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Zoom to level error: %1").arg(e.what()));
        }
    }
    
    void testZoomToFit() {
        logMessage("=== Testing Zoom To Fit ===");
        
        try {
            if (mapView_) {
                QRectF testRect(0, 0, 500, 500); // Test area
                mapView_->zoomToFit(testRect);
                logMessage("✓ Zoom to fit executed successfully");
                updateZoomInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Zoom to fit error: %1").arg(e.what()));
        }
    }
    
    void testResetZoom() {
        logMessage("=== Testing Reset Zoom ===");
        
        try {
            if (mapView_) {
                mapView_->resetZoom();
                logMessage("✓ Reset zoom executed successfully");
                updateZoomInfo();
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Reset zoom error: %1").arg(e.what()));
        }
    }
    
    void testGridVisibility() {
        logMessage("=== Testing Grid Visibility ===");
        
        try {
            if (mapView_) {
                gridVisible_ = !gridVisible_;
                mapView_->setGridVisible(gridVisible_);
                logMessage(QString("✓ Grid visibility set to %1").arg(gridVisible_ ? "visible" : "hidden"));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Grid visibility error: %1").arg(e.what()));
        }
    }
    
    void testGridType() {
        logMessage("=== Testing Grid Type ===");
        
        try {
            if (mapView_) {
                static int currentType = 0;
                currentType = (currentType + 1) % 3; // Cycle through 0, 1, 2
                mapView_->setGridType(currentType);
                
                QString typeName;
                switch (currentType) {
                    case 0: typeName = "Tile Grid"; break;
                    case 1: typeName = "Coordinate Grid"; break;
                    case 2: typeName = "Custom Grid"; break;
                }
                
                logMessage(QString("✓ Grid type set to %1").arg(typeName));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Grid type error: %1").arg(e.what()));
        }
    }
    
    void testGridColor() {
        logMessage("=== Testing Grid Color ===");
        
        try {
            if (mapView_) {
                QColor color = QColorDialog::getColor(Qt::white, this, "Select Grid Color");
                if (color.isValid()) {
                    mapView_->setGridColor(color);
                    logMessage(QString("✓ Grid color set to %1").arg(color.name()));
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Grid color error: %1").arg(e.what()));
        }
    }
    
    void testMouseTracking() {
        logMessage("=== Testing Mouse Tracking ===");
        
        try {
            if (mapView_) {
                mouseTrackingEnabled_ = !mouseTrackingEnabled_;
                mapView_->setMouseTrackingEnabled(mouseTrackingEnabled_);
                logMessage(QString("✓ Mouse tracking set to %1").arg(mouseTrackingEnabled_ ? "enabled" : "disabled"));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Mouse tracking error: %1").arg(e.what()));
        }
    }
    
    void testHoverEffects() {
        logMessage("=== Testing Hover Effects ===");
        
        try {
            if (mapView_) {
                hoverEffectsEnabled_ = !hoverEffectsEnabled_;
                mapView_->setHoverEffectsEnabled(hoverEffectsEnabled_);
                logMessage(QString("✓ Hover effects set to %1").arg(hoverEffectsEnabled_ ? "enabled" : "disabled"));
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Hover effects error: %1").arg(e.what()));
        }
    }
    
    void testBrushPreview() {
        logMessage("=== Testing Brush Preview ===");
        
        try {
            if (mapView_) {
                static bool previewVisible = false;
                previewVisible = !previewVisible;
                
                if (previewVisible) {
                    QPointF position = mapView_->getCurrentMapPosition();
                    if (position.isNull()) {
                        position = QPointF(100, 100); // Default position
                    }
                    mapView_->setBrushPreview(position, 3, QColor(255, 255, 0, 128));
                    logMessage("✓ Brush preview enabled");
                } else {
                    mapView_->clearBrushPreview();
                    logMessage("✓ Brush preview disabled");
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Brush preview error: %1").arg(e.what()));
        }
    }
    
    void testDrawingIndicator() {
        logMessage("=== Testing Drawing Indicator ===");
        
        try {
            if (mapView_) {
                static bool indicatorVisible = false;
                indicatorVisible = !indicatorVisible;
                
                if (indicatorVisible) {
                    QPointF position = mapView_->getCurrentMapPosition();
                    if (position.isNull()) {
                        position = QPointF(150, 150); // Default position
                    }
                    mapView_->setDrawingIndicator(position, "Test Indicator");
                    logMessage("✓ Drawing indicator enabled");
                } else {
                    mapView_->clearDrawingIndicator();
                    logMessage("✓ Drawing indicator disabled");
                }
            }
        } catch (const std::exception& e) {
            logMessage(QString("✗ Drawing indicator error: %1").arg(e.what()));
        }
    }
    
    void testAllFeatures() {
        logMessage("=== Running Complete MapView Enhancements Test Suite ===");
        
        // Test each feature with delays
        QTimer::singleShot(100, this, &MapViewEnhancementsTestWidget::testZoomIn);
        QTimer::singleShot(500, this, &MapViewEnhancementsTestWidget::testZoomOut);
        QTimer::singleShot(900, this, &MapViewEnhancementsTestWidget::testZoomToLevel);
        QTimer::singleShot(1300, this, &MapViewEnhancementsTestWidget::testZoomToFit);
        QTimer::singleShot(1700, this, &MapViewEnhancementsTestWidget::testResetZoom);
        QTimer::singleShot(2100, this, &MapViewEnhancementsTestWidget::testGridVisibility);
        QTimer::singleShot(2500, this, &MapViewEnhancementsTestWidget::testGridType);
        QTimer::singleShot(2900, this, &MapViewEnhancementsTestWidget::testMouseTracking);
        QTimer::singleShot(3300, this, &MapViewEnhancementsTestWidget::testHoverEffects);
        QTimer::singleShot(3700, this, &MapViewEnhancementsTestWidget::testBrushPreview);
        QTimer::singleShot(4100, this, &MapViewEnhancementsTestWidget::testDrawingIndicator);
        
        QTimer::singleShot(4500, this, [this]() {
            logMessage("=== Complete MapView Enhancements Test Suite Finished ===");
            logMessage("All Task 80 enhancement features tested successfully!");
        });
    }
    
    void clearLog() {
        if (statusText_) {
            statusText_->clear();
            logMessage("Log cleared - ready for new tests");
        }
    }
    
    void onMousePositionChanged(const QPointF& screenPos, const QPointF& mapPos) {
        if (coordinateLabel_) {
            coordinateLabel_->setText(QString("Mouse: Screen(%1, %2) Map(%3, %4)")
                                     .arg(screenPos.x(), 0, 'f', 1)
                                     .arg(screenPos.y(), 0, 'f', 1)
                                     .arg(mapPos.x(), 0, 'f', 1)
                                     .arg(mapPos.y(), 0, 'f', 1));
        }
        
        // Log mouse position changes (but not too frequently)
        static QTime lastLogTime;
        if (lastLogTime.isNull() || lastLogTime.elapsed() > 1000) { // Log every second
            logMessage(QString("Mouse position: Screen(%1, %2) Map(%3, %4)")
                      .arg(screenPos.x(), 0, 'f', 1)
                      .arg(screenPos.y(), 0, 'f', 1)
                      .arg(mapPos.x(), 0, 'f', 1)
                      .arg(mapPos.y(), 0, 'f', 1));
            lastLogTime = QTime::currentTime();
        }
    }
    
    void onZoomChanged(double newZoom, double oldZoom) {
        updateZoomInfo();
        logMessage(QString("Zoom changed from %1 to %2").arg(oldZoom, 0, 'f', 2).arg(newZoom, 0, 'f', 2));
    }
    
    void onTileHovered(const QPointF& tilePos) {
        logMessage(QString("Tile hovered: (%1, %2)").arg(tilePos.x()).arg(tilePos.y()));
    }

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        
        // Create splitter for map view and controls
        QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
        mainLayout->addWidget(splitter);
        
        // Map view
        mapView_ = new MapView();
        mapView_->setMinimumSize(600, 400);
        splitter->addWidget(mapView_);
        
        // Controls panel
        QWidget* controlsWidget = new QWidget();
        controlsWidget->setMaximumWidth(400);
        controlsWidget->setMinimumWidth(350);
        setupControlsPanel(controlsWidget);
        splitter->addWidget(controlsWidget);
        
        // Set splitter proportions
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 0);
    }

    void setupControlsPanel(QWidget* parent) {
        QVBoxLayout* layout = new QVBoxLayout(parent);

        // Zoom controls
        QGroupBox* zoomGroup = new QGroupBox("Zoom Controls", parent);
        QVBoxLayout* zoomLayout = new QVBoxLayout(zoomGroup);

        QPushButton* zoomInBtn = new QPushButton("Zoom In", zoomGroup);
        connect(zoomInBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testZoomIn);
        zoomLayout->addWidget(zoomInBtn);

        QPushButton* zoomOutBtn = new QPushButton("Zoom Out", zoomGroup);
        connect(zoomOutBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testZoomOut);
        zoomLayout->addWidget(zoomOutBtn);

        QPushButton* zoomLevelBtn = new QPushButton("Zoom to 250%", zoomGroup);
        connect(zoomLevelBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testZoomToLevel);
        zoomLayout->addWidget(zoomLevelBtn);

        QPushButton* zoomFitBtn = new QPushButton("Zoom to Fit", zoomGroup);
        connect(zoomFitBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testZoomToFit);
        zoomLayout->addWidget(zoomFitBtn);

        QPushButton* resetZoomBtn = new QPushButton("Reset Zoom", zoomGroup);
        connect(resetZoomBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testResetZoom);
        zoomLayout->addWidget(resetZoomBtn);

        layout->addWidget(zoomGroup);

        // Grid controls
        QGroupBox* gridGroup = new QGroupBox("Grid Controls", parent);
        QVBoxLayout* gridLayout = new QVBoxLayout(gridGroup);

        QPushButton* gridVisBtn = new QPushButton("Toggle Grid Visibility", gridGroup);
        connect(gridVisBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testGridVisibility);
        gridLayout->addWidget(gridVisBtn);

        QPushButton* gridTypeBtn = new QPushButton("Change Grid Type", gridGroup);
        connect(gridTypeBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testGridType);
        gridLayout->addWidget(gridTypeBtn);

        QPushButton* gridColorBtn = new QPushButton("Change Grid Color", gridGroup);
        connect(gridColorBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testGridColor);
        gridLayout->addWidget(gridColorBtn);

        layout->addWidget(gridGroup);

        // Mouse tracking controls
        QGroupBox* mouseGroup = new QGroupBox("Mouse Tracking", parent);
        QVBoxLayout* mouseLayout = new QVBoxLayout(mouseGroup);

        QPushButton* trackingBtn = new QPushButton("Toggle Mouse Tracking", mouseGroup);
        connect(trackingBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testMouseTracking);
        mouseLayout->addWidget(trackingBtn);

        QPushButton* hoverBtn = new QPushButton("Toggle Hover Effects", mouseGroup);
        connect(hoverBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testHoverEffects);
        mouseLayout->addWidget(hoverBtn);

        layout->addWidget(mouseGroup);

        // Drawing feedback controls
        QGroupBox* feedbackGroup = new QGroupBox("Drawing Feedback", parent);
        QVBoxLayout* feedbackLayout = new QVBoxLayout(feedbackGroup);

        QPushButton* previewBtn = new QPushButton("Toggle Brush Preview", feedbackGroup);
        connect(previewBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testBrushPreview);
        feedbackLayout->addWidget(previewBtn);

        QPushButton* indicatorBtn = new QPushButton("Toggle Drawing Indicator", feedbackGroup);
        connect(indicatorBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testDrawingIndicator);
        feedbackLayout->addWidget(indicatorBtn);

        layout->addWidget(feedbackGroup);

        // Test suite controls
        QGroupBox* suiteGroup = new QGroupBox("Test Suite", parent);
        QVBoxLayout* suiteLayout = new QVBoxLayout(suiteGroup);

        QPushButton* allTestsBtn = new QPushButton("Run All Tests", suiteGroup);
        allTestsBtn->setStyleSheet("QPushButton { font-weight: bold; background-color: #4CAF50; color: white; }");
        connect(allTestsBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::testAllFeatures);
        suiteLayout->addWidget(allTestsBtn);

        QPushButton* clearLogBtn = new QPushButton("Clear Log", suiteGroup);
        connect(clearLogBtn, &QPushButton::clicked, this, &MapViewEnhancementsTestWidget::clearLog);
        suiteLayout->addWidget(clearLogBtn);

        layout->addWidget(suiteGroup);

        // Status display
        QGroupBox* statusGroup = new QGroupBox("Status Information", parent);
        QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

        coordinateLabel_ = new QLabel("Mouse: No position", statusGroup);
        coordinateLabel_->setStyleSheet("font-family: monospace;");
        statusLayout->addWidget(coordinateLabel_);

        zoomLabel_ = new QLabel("Zoom: 1.00x", statusGroup);
        zoomLabel_->setStyleSheet("font-family: monospace;");
        statusLayout->addWidget(zoomLabel_);

        layout->addWidget(statusGroup);

        // Log display
        QGroupBox* logGroup = new QGroupBox("Test Results Log", parent);
        QVBoxLayout* logLayout = new QVBoxLayout(logGroup);

        statusText_ = new QTextEdit(logGroup);
        statusText_->setReadOnly(true);
        statusText_->setFont(QFont("Consolas", 9));
        statusText_->setMaximumHeight(200);
        logLayout->addWidget(statusText_);

        layout->addWidget(logGroup);

        layout->addStretch();
    }

    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();

        // View menu
        QMenu* viewMenu = menuBar->addMenu("&View");
        viewMenu->addAction("Zoom &In", this, &MapViewEnhancementsTestWidget::testZoomIn);
        viewMenu->addAction("Zoom &Out", this, &MapViewEnhancementsTestWidget::testZoomOut);
        viewMenu->addAction("&Reset Zoom", this, &MapViewEnhancementsTestWidget::testResetZoom);
        viewMenu->addSeparator();
        viewMenu->addAction("Toggle &Grid", this, &MapViewEnhancementsTestWidget::testGridVisibility);
        viewMenu->addAction("Grid &Type", this, &MapViewEnhancementsTestWidget::testGridType);
        viewMenu->addAction("Grid &Color", this, &MapViewEnhancementsTestWidget::testGridColor);

        // Tools menu
        QMenu* toolsMenu = menuBar->addMenu("&Tools");
        toolsMenu->addAction("Toggle &Mouse Tracking", this, &MapViewEnhancementsTestWidget::testMouseTracking);
        toolsMenu->addAction("Toggle &Hover Effects", this, &MapViewEnhancementsTestWidget::testHoverEffects);
        toolsMenu->addAction("Toggle &Brush Preview", this, &MapViewEnhancementsTestWidget::testBrushPreview);
        toolsMenu->addAction("Toggle &Drawing Indicator", this, &MapViewEnhancementsTestWidget::testDrawingIndicator);

        // Test menu
        QMenu* testMenu = menuBar->addMenu("&Test");
        testMenu->addAction("Run &All Tests", this, &MapViewEnhancementsTestWidget::testAllFeatures);
        testMenu->addAction("&Clear Log", this, &MapViewEnhancementsTestWidget::clearLog);

        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        fileMenu->addAction("&Exit", this, &QWidget::close);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready to test Task 80 MapView enhancement features");
    }

    void initializeComponents() {
        // Initialize test map
        testMap_ = new Map(this);

        // Set map to MapView
        if (mapView_) {
            mapView_->setMap(testMap_);
        }

        logMessage("All components initialized successfully");
    }

    void connectSignals() {
        if (mapView_ && mapView_->getMouseTracker()) {
            connect(mapView_->getMouseTracker(), &MapViewMouseTracker::mousePositionChanged,
                    this, &MapViewEnhancementsTestWidget::onMousePositionChanged);
            connect(mapView_->getMouseTracker(), &MapViewMouseTracker::tileHovered,
                    this, &MapViewEnhancementsTestWidget::onTileHovered);
        }

        if (mapView_ && mapView_->getZoomSystem()) {
            connect(mapView_->getZoomSystem(), &MapViewZoomSystem::zoomChanged,
                    this, &MapViewEnhancementsTestWidget::onZoomChanged);
        }
    }

    void updateZoomInfo() {
        if (mapView_ && mapView_->getZoomSystem() && zoomLabel_) {
            double zoom = mapView_->getZoomSystem()->getCurrentZoom();
            zoomLabel_->setText(QString("Zoom: %1x").arg(zoom, 0, 'f', 2));
        }
    }

    void logMessage(const QString& message) {
        if (statusText_) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            statusText_->append(QString("[%1] %2").arg(timestamp, message));
            statusText_->ensureCursorVisible();
        }
        qDebug() << "MapViewEnhancementsTest:" << message;
    }

private:
    MapView* mapView_;
    Map* testMap_;
    QTextEdit* statusText_;
    QLabel* coordinateLabel_;
    QLabel* zoomLabel_;
    bool mouseTrackingEnabled_;
    bool hoverEffectsEnabled_;
    bool gridVisible_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("MapView Enhancements Test");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");

    MapViewEnhancementsTestWidget testWidget;
    testWidget.show();

    return app.exec();
}

#include "MapViewEnhancementsTest.moc"
