#ifndef MINIMAPWINDOW_H
#define MINIMAPWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QFrame>
#include <QGroupBox>
#include <QSplitter>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QStringList>

#include "MapPos.h"

// Forward declarations
class Map;
class MapView;
class Tile;
class MinimapRenderer;
class MinimapWaypoint;

/**
 * @brief Task 90: Complete MinimapWindow implementation
 * 
 * Full minimap functionality including:
 * - Tile rendering with minimap colors from wxwidgets compatibility
 * - Click navigation to center MapView on clicked position
 * - Viewport synchronization with MapView panning and zooming
 * - Map content synchronization when tiles are edited
 * - Optimized block-based rendering with caching
 * - Waypoint system for navigation
 * - Floor selection and multi-floor view
 * - Zoom controls and view options
 * - Selection box visualization
 * - Hover tooltips with tile information
 * - 1:1 compatibility with wxwidgets minimap functionality
 */

/**
 * @brief Minimap Color Palette (from wxwidgets)
 */
struct MinimapColor {
    quint8 red;
    quint8 green;
    quint8 blue;
    
    MinimapColor(quint8 r = 0, quint8 g = 0, quint8 b = 0) : red(r), green(g), blue(b) {}
    
    QColor toQColor() const { return QColor(red, green, blue); }
    quint32 toRGB() const { return (red << 16) | (green << 8) | blue; }
};

/**
 * @brief Minimap Waypoint for navigation
 */
struct MinimapWaypoint {
    QString name;
    MapPos position;
    QColor color;
    bool visible;
    
    MinimapWaypoint(const QString& n = QString(), const MapPos& pos = MapPos(), 
                   const QColor& c = Qt::red, bool v = true)
        : name(n), position(pos), color(c), visible(v) {}
};

/**
 * @brief Minimap Block for optimized rendering
 */
struct MinimapBlock {
    static const int BLOCK_SIZE = 256; // Optimal block size from wxwidgets
    
    QPixmap bitmap;
    bool needsUpdate;
    bool wasSeen;
    int floor;
    QElapsedTimer lastUpdate;
    
    MinimapBlock() : needsUpdate(true), wasSeen(false), floor(-1) {}
};

/**
 * @brief Main Minimap Window Widget
 */
class MinimapWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWindow(QWidget* parent = nullptr);
    ~MinimapWindow() override;

    // Configuration
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    Map* getMap() const { return map_; }
    MapView* getMapView() const { return mapView_; }

    // Floor management
    void setCurrentFloor(int floor);
    int getCurrentFloor() const { return currentFloor_; }
    void setShowAllFloors(bool show);
    bool isShowAllFloors() const { return showAllFloors_; }

    // View control
    void centerOnPosition(const MapPos& position);
    void setZoomLevel(double zoom);
    double getZoomLevel() const { return zoomLevel_; }
    void setViewportRect(const QRect& rect);
    QRect getViewportRect() const { return viewportRect_; }

    // Rendering options
    void setShowGrid(bool show);
    bool isShowGrid() const { return showGrid_; }
    void setShowWaypoints(bool show);
    bool isShowWaypoints() const { return showWaypoints_; }
    void setShowSelection(bool show);
    bool isShowSelection() const { return showSelection_; }
    void setShowTooltips(bool show);
    bool isShowTooltips() const { return showTooltips_; }

    // Cache management
    void clearCache();
    void updateCache();
    void preloadArea(const QRect& area);

    // Waypoint management
    void addWaypoint(const MinimapWaypoint& waypoint);
    void removeWaypoint(int index);
    void clearWaypoints();
    QList<MinimapWaypoint> getWaypoints() const { return waypoints_; }
    void setWaypoints(const QList<MinimapWaypoint>& waypoints);

    // Static color palette access
    static const MinimapColor* getColorPalette();
    static QColor getMinimapColor(quint8 colorIndex);
    static quint8 findClosestColorIndex(const QColor& color);

public slots:
    void updateMinimap();
    void updateViewport();
    void onMapChanged();
    void onMapViewChanged();
    void onTileChanged(int x, int y, int z);
    void onSelectionChanged();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void goToWaypoint(int index);

signals:
    void positionClicked(const MapPos& position);
    void viewportChanged(const QRect& viewport);
    void zoomChanged(double zoom);
    void waypointClicked(int index);
    void minimapUpdated();

protected:
    // Event handlers
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void leaveEvent(QEvent* event) override;

    // Tooltip support
    bool event(QEvent* event) override;
    void showTooltip(const QPoint& position);
    void hideTooltip();

private slots:
    void onUpdateTimer();
    void onResizeTimer();

private:
    void setupUI();
    void initializeColorPalette();
    void connectSignals();
    
    // Rendering methods
    void renderMinimap(QPainter* painter);
    void renderFloor(QPainter* painter, int floor);
    void renderBlock(QPainter* painter, int blockX, int blockY, int floor);
    void renderTile(QPainter* painter, int x, int y, int floor);
    void renderViewport(QPainter* painter);
    void renderWaypoints(QPainter* painter);
    void renderSelection(QPainter* painter);
    void renderGrid(QPainter* painter);
    
    // Block management
    MinimapBlock* getBlock(int blockX, int blockY, int floor);
    void updateBlock(int blockX, int blockY, int floor);
    void invalidateBlock(int blockX, int blockY, int floor);
    void clearBlockCache();
    
    // Coordinate conversion
    QPoint mapToWidget(const MapPos& mapPos) const;
    MapPos widgetToMap(const QPoint& widgetPos) const;
    QRect mapRectToWidget(const QRect& mapRect) const;
    QRect widgetRectToMap(const QRect& widgetRect) const;
    
    // Utility methods
    quint8 getTileColor(int x, int y, int floor) const;
    QString getTileTooltip(int x, int y, int floor) const;
    void updateVisibleArea();
    void scheduleUpdate();
    
    // Cache optimization
    bool isBlockVisible(int blockX, int blockY) const;
    void cleanupCache();

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    
    // Rendering state
    int currentFloor_;
    bool showAllFloors_;
    double zoomLevel_;
    QRect viewportRect_;
    QPoint centerPosition_;
    
    // Display options
    bool showGrid_;
    bool showWaypoints_;
    bool showSelection_;
    bool showTooltips_;
    
    // Cache system
    QMap<QString, MinimapBlock*> blockCache_; // Key: "x_y_floor"
    QMutex cacheMutex_;
    QTimer* updateTimer_;
    QTimer* resizeTimer_;
    bool needsUpdate_;
    bool isResizing_;
    
    // Waypoints
    QList<MinimapWaypoint> waypoints_;
    int selectedWaypoint_;
    
    // Mouse interaction
    bool dragging_;
    QPoint lastMousePos_;
    QPoint tooltipPos_;
    
    // Performance tracking
    QElapsedTimer renderTimer_;
    int frameCount_;
    double averageFrameTime_;
    
    // Static color palette
    static MinimapColor colorPalette_[256];
    static bool paletteInitialized_;
    
    // Constants
    static const int UPDATE_INTERVAL = 100; // milliseconds
    static const int RESIZE_DELAY = 200; // milliseconds
    static const double MIN_ZOOM = 0.1;
    static const double MAX_ZOOM = 5.0;
    static const double ZOOM_STEP = 0.2;
};

/**
 * @brief Minimap Control Panel for additional controls
 */
class MinimapControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapControlPanel(MinimapWindow* minimap, QWidget* parent = nullptr);
    ~MinimapControlPanel() override = default;

    void setMinimapWindow(MinimapWindow* minimap);

signals:
    void floorChanged(int floor);
    void zoomChanged(double zoom);
    void optionChanged(const QString& option, bool enabled);

private slots:
    void onFloorChanged();
    void onZoomInClicked();
    void onZoomOutClicked();
    void onResetZoomClicked();
    void onShowGridToggled(bool show);
    void onShowWaypointsToggled(bool show);
    void onShowSelectionToggled(bool show);
    void onShowTooltipsToggled(bool show);
    void onShowAllFloorsToggled(bool show);
    void onClearCacheClicked();

private:
    void setupUI();
    void connectSignals();
    void updateControls();

private:
    MinimapWindow* minimap_;
    
    // Floor controls
    QGroupBox* floorGroup_;
    QSpinBox* floorSpinBox_;
    QCheckBox* showAllFloorsCheckBox_;
    
    // Zoom controls
    QGroupBox* zoomGroup_;
    QToolButton* zoomInButton_;
    QToolButton* zoomOutButton_;
    QToolButton* resetZoomButton_;
    QLabel* zoomLabel_;
    
    // Display options
    QGroupBox* optionsGroup_;
    QCheckBox* showGridCheckBox_;
    QCheckBox* showWaypointsCheckBox_;
    QCheckBox* showSelectionCheckBox_;
    QCheckBox* showTooltipsCheckBox_;
    
    // Cache controls
    QGroupBox* cacheGroup_;
    QPushButton* clearCacheButton_;
    QLabel* cacheStatusLabel_;
};

/**
 * @brief Minimap Waypoint Manager
 */
class MinimapWaypointManager : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWaypointManager(MinimapWindow* minimap, QWidget* parent = nullptr);
    ~MinimapWaypointManager() override = default;

    void setMinimapWindow(MinimapWindow* minimap);
    void refreshWaypoints();

signals:
    void waypointAdded(const MinimapWaypoint& waypoint);
    void waypointRemoved(int index);
    void waypointSelected(int index);

private slots:
    void onAddWaypoint();
    void onRemoveWaypoint();
    void onGoToWaypoint();
    void onWaypointSelectionChanged();
    void onSaveWaypoints();
    void onLoadWaypoints();

private:
    void setupUI();
    void connectSignals();
    void updateWaypointList();

private:
    MinimapWindow* minimap_;
    
    QComboBox* waypointComboBox_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* goToButton_;
    QPushButton* saveButton_;
    QPushButton* loadButton_;
};

#endif // MINIMAPWINDOW_H
