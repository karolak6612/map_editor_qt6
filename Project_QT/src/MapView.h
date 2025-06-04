#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPointF>
#include <QRectF>   // Added for drawBackground
#include <QPainter> // Added for drawBackground
#include <QEnterEvent>
#include <QFocusEvent> // Added for focusOutEvent
#include <QDebug>
#include <QTimer>   // Added for double-click detection
#include <QToolTip> // For QToolTip* waypointTooltip_

// Forward declarations
class MapViewInputHandler;
class Brush; // Added forward declaration
class BrushManager;
class Map;
class QUndoStack;

// Include Map.h for MapPos definition
#include "Map.h"
class MapDrawingPrimitives; // Task 65: Drawing primitives
class MapOverlayRenderer;   // Task 65: Overlay rendering
class WaypointBrush;        // Task 74: Waypoint brush
class Waypoint;             // Task 74: Waypoint
class WaypointItem;         // Task 74: Waypoint item
#include "DrawingOptions.h" // Task 75: Drawing options
class MapViewZoomSystem;    // Task 80: Enhanced zoom system
class MapViewGridSystem;    // Task 80: Enhanced grid system
class MapViewMouseTracker;  // Task 80: Enhanced mouse tracking
class MapViewDrawingFeedback; // Task 80: Enhanced drawing feedback
class MapViewZoomHandler;     // Task 011: Extracted zoom handling

// Constants
const int GROUND_LAYER = 7;
const int TILE_SIZE = 32;
const int MAP_MAX_LAYERS = 16;
const double MIN_ZOOM = 0.125;
const double MAX_ZOOM = 25.0;

class MapView : public QGraphicsView {
    Q_OBJECT

signals:
    // Enhanced status update signal for MainWindow integration
    void statusUpdateRequested(const QString& type, const QVariantMap& data);

public:
    explicit MapView(BrushManager* brushManager, Map* map, QUndoStack* undoStack, QWidget *parent = nullptr);
    ~MapView() override;

    enum class EditorMode {
        Selection,
        Drawing // Renamed from DrawingMode for consistency if it was different
    };
    
    // Editor Mode and Active Brush
    EditorMode getCurrentEditorMode() const; // Getter for currentEditorMode_
    void setCurrentEditorMode(EditorMode mode); // Setter if needed, or manage internally
    
    void setActiveBrush(Brush* brush);
    Brush* getActiveBrush() const;

    // Configuration
    void setSwitchMouseButtons(bool switched);
    bool getSwitchMouseButtons() const { return switchMouseButtons_; }

    // Public getters
    double getZoomLevel() const { return zoomLevel_; }
    int getCurrentFloor() const { return currentFloor_; }

    // Mouse position tracking (wxWidgets compatibility)
    QPoint getCursorPosition() const { return lastMousePos_; }
    QPointF getLastMapPosition() const { return lastMapPos_; }

    // State getters for wxWidgets compatibility
    bool isDragging() const { return isDragging_; }
    bool isScreenDragging() const { return isScreenDragging_; }
    bool isDrawing() const { return isDrawing_; }

    QPointF screenToMap(const QPoint& screenPos) const;
    QPointF mapToScreen(const QPointF& mapTilePos) const;
    MapPos mapToTilePos(const QPointF& mapPos) const;

    // Public interface for MapViewInputHandler
    void pan(int dx, int dy);
    void zoom(qreal factor, const QPointF& centerScreenPos); // Changed center to screen pos for consistency with wheelEvent
    void setSelectionArea(const QRectF& rect);

    // Enhanced zoom convenience methods
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void centerOnMap();

    // Task 80: Enhanced MapView features
    MapViewZoomSystem* getZoomSystem() const { return zoomSystem_; }
    MapViewGridSystem* getGridSystem() const { return gridSystem_; }
    MapViewMouseTracker* getMouseTracker() const { return mouseTracker_; }
    MapViewDrawingFeedback* getDrawingFeedback() const { return drawingFeedback_; }

    // Enhanced zoom operations
    void zoomToLevel(double level, const QPointF& centerPoint = QPointF());
    void zoomToFit(const QRectF& rect);
    void handleWheelZoom(QWheelEvent* event);

    // Enhanced grid operations
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setGridType(int type);
    void setGridColor(const QColor& color);

    // Enhanced mouse tracking
    void setMouseTrackingEnabled(bool enabled);
    void setHoverEffectsEnabled(bool enabled);
    QPointF getCurrentMousePosition() const;
    QPointF getCurrentMapPosition() const;

    // Enhanced drawing feedback
    void setBrushPreview(const QPointF& position, int size, const QColor& color);
    void clearBrushPreview();
    void setDrawingIndicator(const QPointF& position, const QString& text);
    void clearDrawingIndicator();

    // Task 65: Drawing primitives integration
    MapDrawingPrimitives* getDrawingPrimitives() const { return drawingPrimitives_; }
    MapOverlayRenderer* getOverlayRenderer() const { return overlayRenderer_; }
    void setShowBrushPreview(bool show);
    void setBrushPreviewState(const QPointF& position, Brush* brush, int size, bool isValid = true);
    void updateBrushPreview(const QPointF& mousePos);

    // Placeholder methods (many are called by MapViewInputHandler via mapView_ pointer)
    void pasteSelection(const QPointF& mapPos);
    void clearSelection();
    void toggleSelectionAt(const QPointF& mapPos);
    bool isOnSelection(const QPointF& mapPos) const;
    void selectObjectAt(const QPointF& mapPos);
    void updateMoveSelectionFeedback(const QPointF& delta);
    void finalizeMoveSelection(const QPointF& delta);
    void updateSelectionRectFeedback(const QPointF& startMapPos, const QPointF& currentMapPos);
    void finalizeSelectionRect(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers);

    // These drawing related placeholders will be removed from MapView.h/cpp
    // as brushes will handle their own drawing logic via mouse events.
    // void startDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void continueDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void finalizeDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void updateDragDrawFeedback(const QPointF& startMapPos, const QPointF& currentMapPos); // To be removed
    // void finalizeDragDraw(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers); // To be removed

    void updateStatusBarWithMapPos(const QPointF& mapPos);
    void updateZoomStatus();
    void updateFloorStatus();

    void increaseBrushSize_placeholder();
    void decreaseBrushSize_placeholder();
    void updateFloorMenu_placeholder();

    // Enhanced methods for wxWidgets compatibility
    void changeFloor(int newFloor); // Made public for InputHandler access

    void showPropertiesDialogFor(const QPointF& mapPos);
    void switchToSelectionMode();
    void endPasting();
    void showContextMenuAt(const QPoint& screenPos);
    void resetActionQueueTimer_placeholder();

    // Task 74: Enhanced waypoint interaction
    void setWaypointBrush(WaypointBrush* brush);
    WaypointBrush* getWaypointBrush() const { return waypointBrush_; }
    bool isWaypointToolActive() const;

    // Task 74: Waypoint placement and editing
    void placeWaypointAt(const QPointF& mapPos);
    void selectWaypoint(Waypoint* waypoint);
    void editWaypoint(Waypoint* waypoint);
    void deleteWaypoint(Waypoint* waypoint);

    // Task 74: Waypoint visual management
    void updateWaypointVisuals();
    void refreshWaypointItems();
    void setWaypointsVisible(bool visible);
    bool areWaypointsVisible() const { return waypointsVisible_; }

    // Task 74: Waypoint tooltips
    void showWaypointTooltip(Waypoint* waypoint, const QPoint& position);
    void hideWaypointTooltip();
    void updateWaypointTooltip(const QPoint& position);

    // Task 74: Waypoint centering
    void centerOnWaypoint(Waypoint* waypoint);
    void highlightWaypoint(Waypoint* waypoint, bool highlight = true);

    // Task 74: Helper methods for coordinate conversion

    // Task 75: View settings management
    void setDrawingOptions(const DrawingOptions& options);
    DrawingOptions getDrawingOptions() const { return drawingOptions_; }

    // Task 75: Individual view setting setters
    void setShowGrid(bool show);
    void setShowZones(bool show);
    void setShowLights(bool show);
    void setShowWaypoints(bool show);
    void setShowCreatures(bool show);
    void setShowSpawns(bool show);
    void setShowHouses(bool show);
    void setShowShade(bool show);
    void setShowSpecialTiles(bool show);
    void setShowTechnicalItems(bool show);
    void setShowIngameBox(bool show);
    void setShowWallHooks(bool show);
    void setShowTowns(bool show);
    void setShowBlocking(bool show);
    void setHighlightItems(bool highlight);
    void setHighlightLockedDoors(bool highlight);
    void setShowExtra(bool show);
    void setShowAllFloors(bool show);
    void setGhostItems(bool ghost);
    void setGhostHigherFloors(bool ghost);
    void setTransparentFloors(bool transparent);
    void setTransparentItems(bool transparent);
    void setShowPreview(bool show);
    void setShowTooltips(bool show);
    void setShowAsMinimapColors(bool show);
    void setShowOnlyColors(bool show);
    void setShowOnlyModified(bool show);
    void setAlwaysShowZones(bool show);
    void setIngameMode(bool ingame);
    void setExperimentalFog(bool fog);
    void setExtHouseShader(bool shader);

    // Task 75: Individual view setting getters
    bool getShowGrid() const { return drawingOptions_.showGrid; }
    bool getShowZones() const { return drawingOptions_.showZones; }
    bool getShowLights() const { return drawingOptions_.showLights; }
    bool getShowWaypoints() const { return drawingOptions_.showWaypoints; }
    bool getShowCreatures() const { return drawingOptions_.showCreatures; }
    bool getShowSpawns() const { return drawingOptions_.showSpawns; }
    bool getShowHouses() const { return drawingOptions_.showHouses; }
    bool getShowShade() const { return drawingOptions_.showShade; }
    bool getShowSpecialTiles() const { return drawingOptions_.showSpecialTiles; }
    bool getShowTechnicalItems() const { return drawingOptions_.showTechnicalItems; }
    bool getShowIngameBox() const { return drawingOptions_.showIngameBox; }
    bool getShowWallHooks() const { return drawingOptions_.showWallHooks; }
    bool getShowTowns() const { return drawingOptions_.showTowns; }
    bool getShowBlocking() const { return drawingOptions_.showBlocking; }
    bool getHighlightItems() const { return drawingOptions_.highlightItems; }
    bool getHighlightLockedDoors() const { return drawingOptions_.highlightLockedDoors; }
    bool getShowExtra() const { return drawingOptions_.showExtra; }
    bool getShowAllFloors() const { return drawingOptions_.showAllFloors; }
    bool getGhostItems() const { return drawingOptions_.ghostItems; }
    bool getGhostHigherFloors() const { return drawingOptions_.ghostHigherFloors; }
    bool getTransparentFloors() const { return drawingOptions_.transparentFloors; }
    bool getTransparentItems() const { return drawingOptions_.transparentItems; }
    bool getShowPreview() const { return drawingOptions_.showPreview; }
    bool getShowTooltips() const { return drawingOptions_.showTooltips; }
    bool getShowAsMinimapColors() const { return drawingOptions_.showAsMinimapColors; }
    bool getShowOnlyColors() const { return drawingOptions_.showOnlyColors; }
    bool getShowOnlyModified() const { return drawingOptions_.showOnlyModified; }
    bool getAlwaysShowZones() const { return drawingOptions_.alwaysShowZones; }
    bool getIngameMode() const { return drawingOptions_.ingame; }
    bool getExperimentalFog() const { return drawingOptions_.experimentalFog; }
    bool getExtHouseShader() const { return drawingOptions_.extHouseShader; }

    // Task 75: Convenience methods
    void setDefaultViewSettings();
    void setIngameViewSettings();
    void updateAllTileItems();

    // Task 85: Tile locking visual indication
    void setShowLockedTiles(bool show);
    bool isShowLockedTiles() const;
    void setLockedTileOverlayColor(const QColor& color);
    QColor getLockedTileOverlayColor() const;
    void updateLockedTileVisuals();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override; // Added

    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override; // Added
private:
    void updateAndRefreshMapCoordinates(const QPoint& screenPos);

    EditorMode currentEditorMode_ = EditorMode::Selection; // Keep private, use getter/setter
    Brush* currentBrush_ = nullptr;       // Active brush

    double zoomLevel_;
    int currentFloor_;
    bool isPanning_;
    QPoint lastMousePos_;
    QPointF lastMapPos_;
    QPoint dragStartScreenPos_;

    // wxWidgets compatibility state variables
    bool isDragging_ = false;
    bool isScreenDragging_ = false;
    bool isDrawing_ = false;
    bool boundboxSelection_ = false;
    bool draggingDraw_ = false;
    bool replaceDragging_ = false;

    // Last click tracking (wxWidgets compatibility)
    QPoint lastClickScreenPos_;
    QPointF lastClickMapPos_;
    int lastClickMapZ_ = GROUND_LAYER;
    QPoint lastClickAbsPos_;

    // Middle mouse button tracking
    QPoint lastMmbClickPos_;

    bool switchMouseButtons_ = false;
    bool doubleClickProperties_ = true;

    MapViewInputHandler* inputHandler_;

    // Task 65: Drawing primitives and overlay rendering
    MapDrawingPrimitives* drawingPrimitives_;
    MapOverlayRenderer* overlayRenderer_;

    // Selection state variables (Task 61)
    QRectF currentSelectionArea_; // Added for drawing selection
    QPointF selectionMoveOffset_; // Offset for visual feedback during selection moves

    // Task 74: Waypoint system integration
    WaypointBrush* waypointBrush_;
    bool waypointsVisible_;
    Waypoint* selectedWaypoint_;
    QToolTip* waypointTooltip_;
    QTimer* tooltipTimer_;
    QPoint lastTooltipPosition_;
    QHash<Waypoint*, WaypointItem*> waypointItems_;

    // Task 75: Drawing options for view settings
    DrawingOptions drawingOptions_;

    // Task 80: Enhanced MapView systems
    MapViewZoomSystem* zoomSystem_;
    MapViewGridSystem* gridSystem_;
    MapViewMouseTracker* mouseTracker_;
    MapViewDrawingFeedback* drawingFeedback_;

    // Task 011: Extracted zoom handling for mandate M6 compliance
    MapViewZoomHandler* zoomHandler_;

    // Task 85: Tile locking visual system
    bool showLockedTiles_;
    QColor lockedTileOverlayColor_;

    // Helper methods for selection (Task 61)
    void updateSelectionVisuals();
};

#endif // MAPVIEW_H
