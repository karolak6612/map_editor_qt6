#ifndef MAPDRAWINGPRIMITIVES_H
#define MAPDRAWINGPRIMITIVES_H

#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QVector>

// Forward declarations
class Brush;
class Map;
class MapPos;

// Task 72: Map tooltip structure for enhanced rendering
struct MapTooltip {
    QPointF position;
    QString text;
    QColor color;
    bool ellipsis;
    int maxCharsPerLine;
    int maxChars;

    MapTooltip(const QPointF& pos, const QString& txt, const QColor& col = QColor(255, 255, 255))
        : position(pos), text(txt), color(col), ellipsis(false), maxCharsPerLine(40), maxChars(200) {}

    MapTooltip(qreal x, qreal y, const QString& txt, const QColor& col = QColor(255, 255, 255))
        : position(x, y), text(txt), color(col), ellipsis(false), maxCharsPerLine(40), maxChars(200) {}

    void checkLineEnding() {
        if (text.length() > maxChars) {
            ellipsis = true;
        }
    }
};

/**
 * @brief Drawing options for map primitives
 * 
 * This structure contains all drawing configuration options
 * that affect how primitives are rendered on the map.
 */
struct DrawingOptions {
    // Grid options
    bool showGrid = false;
    QColor gridColor = QColor(128, 128, 128, 128);
    int gridLineWidth = 1;
    
    // Selection options
    QColor selectionColor = QColor(255, 255, 0, 180);  // Yellow with transparency
    QColor selectionBorderColor = QColor(255, 255, 0, 255);
    int selectionBorderWidth = 2;
    Qt::PenStyle selectionBorderStyle = Qt::DashLine;
    
    // Brush preview options
    bool showBrushPreview = true;
    QColor validBrushColor = QColor(0, 255, 0, 128);    // Green for valid
    QColor invalidBrushColor = QColor(255, 0, 0, 128);  // Red for invalid
    QColor neutralBrushColor = QColor(255, 255, 255, 128); // White for neutral
    int brushPreviewBorderWidth = 2;
    
    // Special brush colors (from wxwidgets)
    QColor houseBrushColor = QColor(255, 165, 0, 128);   // Orange
    QColor flagBrushColor = QColor(255, 0, 255, 128);    // Magenta
    QColor spawnBrushColor = QColor(0, 255, 255, 128);   // Cyan
    QColor eraserBrushColor = QColor(128, 128, 128, 128); // Gray
    
    // Ingame box options
    bool showIngameBox = false;
    QColor ingameBoxColor = QColor(255, 255, 255, 64);
    
    // Transparency and effects
    int globalAlpha = 255;
    bool useAntialiasing = true;
    
    // Tile size for calculations
    int tileSize = 32;
    
    // Floor adjustment for 3D effect
    bool show3DEffect = true;
    int floorOffset = 8; // Pixels per floor level
};

/**
 * @brief Brush shape enumeration
 */
enum class BrushShape {
    Square,
    Circle
};

/**
 * @brief Brush validity state
 */
enum class BrushValidity {
    Valid,      // Can draw here
    Invalid,    // Cannot draw here
    Neutral     // No specific validity (default state)
};

/**
 * @brief MapDrawingPrimitives - Core drawing functions for map visualization
 * 
 * This class provides QPainter-based drawing primitives for map elements,
 * replacing the OpenGL-based drawing from the wxwidgets version.
 */
class MapDrawingPrimitives {
public:
    explicit MapDrawingPrimitives(const DrawingOptions& options = DrawingOptions());
    ~MapDrawingPrimitives() = default;
    
    // Configuration
    void setDrawingOptions(const DrawingOptions& options);
    const DrawingOptions& getDrawingOptions() const { return options_; }
    
    // Basic shape drawing
    void drawRect(QPainter* painter, const QRectF& rect, const QColor& color, 
                  int borderWidth = 1, Qt::PenStyle borderStyle = Qt::SolidLine);
    void drawFilledRect(QPainter* painter, const QRectF& rect, const QColor& fillColor, 
                       const QColor& borderColor = QColor(), int borderWidth = 1);
    void drawLine(QPainter* painter, const QPointF& start, const QPointF& end, 
                  const QColor& color, int width = 1, Qt::PenStyle style = Qt::SolidLine);
    void drawEllipse(QPainter* painter, const QRectF& rect, const QColor& color, 
                     int borderWidth = 1, Qt::PenStyle borderStyle = Qt::SolidLine);
    void drawFilledEllipse(QPainter* painter, const QRectF& rect, const QColor& fillColor, 
                          const QColor& borderColor = QColor(), int borderWidth = 1);
    
    // Grid drawing
    void drawGrid(QPainter* painter, const QRectF& viewRect, int currentFloor = 0);
    void drawGridLines(QPainter* painter, const QRectF& viewRect, 
                      const QColor& color, int lineWidth, int spacing);
    
    // Selection drawing
    void drawSelectionBox(QPainter* painter, const QRectF& selectionRect, int currentFloor = 0);
    void drawSelectionArea(QPainter* painter, const QVector<QRectF>& tiles, int currentFloor = 0);
    void drawSelectionBorder(QPainter* painter, const QRectF& rect, int currentFloor = 0);
    
    // Brush preview drawing
    void drawBrushPreview(QPainter* painter, const QPointF& centerPos, Brush* brush, 
                         int brushSize, BrushShape shape, BrushValidity validity, 
                         int currentFloor = 0);
    void drawSquareBrushPreview(QPainter* painter, const QPointF& centerPos, int brushSize, 
                               const QColor& color, int currentFloor = 0);
    void drawCircleBrushPreview(QPainter* painter, const QPointF& centerPos, int brushSize, 
                               const QColor& color, int currentFloor = 0);
    void drawBrushTile(QPainter* painter, const QPointF& tilePos, const QColor& color, 
                      BrushValidity validity, int currentFloor = 0);
    
    // Drag drawing preview
    void drawDragPreview(QPainter* painter, const QRectF& dragArea, Brush* brush, 
                        BrushShape shape, int currentFloor = 0);
    void drawRectangleDragPreview(QPainter* painter, const QRectF& dragArea, 
                                 const QColor& color, int currentFloor = 0);
    void drawCircleDragPreview(QPainter* painter, const QRectF& dragArea, 
                              const QColor& color, int currentFloor = 0);
    
    // Special drawing
    void drawIngameBox(QPainter* painter, const QRectF& viewRect, int currentFloor = 0);
    void drawFloorIndicator(QPainter* painter, const QPointF& pos, int floor, int currentFloor = 0);

    // Task 72: Enhanced drawing methods (OpenGL replacement)
    void drawTooltips(QPainter* painter, const QList<MapTooltip>& tooltips, int currentFloor = 0);
    void drawHoverIndicator(QPainter* painter, const QPointF& position, int currentFloor = 0);
    void drawBrushIndicator(QPainter* painter, const QPointF& position, Brush* brush,
                           const QColor& color, int currentFloor = 0);
    void drawHookIndicator(QPainter* painter, const QPointF& position, bool hookSouth,
                          bool hookEast, int currentFloor = 0);
    void drawWaypointIndicator(QPainter* painter, const QPointF& position, int currentFloor = 0);
    void drawSpawnIndicator(QPainter* painter, const QPointF& position, bool selected, int currentFloor = 0);

    // Task 76: Enhanced brush indicator methods
    void drawBrushIndicatorShape(QPainter* painter, const QPointF& center, Brush* brush, const QColor& color);
    void drawBrushSizeIndicator(QPainter* painter, const QPointF& center, Brush* brush, const QColor& color);
    void drawBrushTypeIndicator(QPainter* painter, const QPointF& center, Brush* brush, const QColor& color);
    void drawHouseExitIndicator(QPainter* painter, const QPointF& position, bool current, int currentFloor = 0);
    void drawTownTempleIndicator(QPainter* painter, const QPointF& position, int currentFloor = 0);

    // Task 72: Advanced brush preview drawing
    void drawRawBrushPreview(QPainter* painter, const QPointF& position, quint16 itemId,
                            const QColor& color, int currentFloor = 0);
    void drawWaypointBrushPreview(QPainter* painter, const QPointF& position, Brush* brush,
                                 BrushValidity validity, int currentFloor = 0);
    void drawHouseBrushPreview(QPainter* painter, const QPointF& position, quint32 houseId,
                              BrushValidity validity, int currentFloor = 0);
    void drawSpawnBrushPreview(QPainter* painter, const QPointF& position, const QString& spawnName,
                              BrushValidity validity, int currentFloor = 0);

    // Task 72: Enhanced selection drawing
    void drawSelectionHighlight(QPainter* painter, const QRectF& area, int currentFloor = 0);
    void drawSelectionBounds(QPainter* painter, const QRectF& bounds, int currentFloor = 0);
    void drawSelectionHandles(QPainter* painter, const QRectF& bounds, int currentFloor = 0);
    void drawDragPreviewAdvanced(QPainter* painter, const QRectF& area, Brush* brush,
                                BrushShape shape, qreal opacity, int currentFloor = 0);
    
    // Utility methods
    QColor getBrushColor(Brush* brush, BrushValidity validity = BrushValidity::Neutral) const;
    QRectF getTileRect(const QPointF& tilePos, int currentFloor = 0) const;
    QPointF getFloorOffset(int floor, int currentFloor = 0) const;
    QVector<QPointF> getCircleTiles(const QPointF& center, int radius) const;
    QVector<QPointF> getSquareTiles(const QPointF& center, int size) const;
    
    // Coordinate conversion helpers
    QRectF mapToScene(const QRectF& mapRect, int currentFloor = 0) const;
    QPointF mapToScene(const QPointF& mapPos, int currentFloor = 0) const;
    QRectF sceneToMap(const QRectF& sceneRect, int currentFloor = 0) const;
    QPointF sceneToMap(const QPointF& scenePos, int currentFloor = 0) const;

private:
    // Helper methods
    void setupPainter(QPainter* painter, bool useAntialiasing = true) const;
    void restorePainter(QPainter* painter) const;
    QPen createPen(const QColor& color, int width, Qt::PenStyle style) const;
    QBrush createBrush(const QColor& color, Qt::BrushStyle style = Qt::SolidPattern) const;

    // Task 72: Private helper methods for enhanced drawing
    void drawSingleTooltip(QPainter* painter, const MapTooltip& tooltip, int currentFloor) const;
    
    // Drawing configuration
    DrawingOptions options_;
};

/**
 * @brief MapOverlayRenderer - Handles overlay rendering for MapView
 * 
 * This class manages the rendering of overlays like brush previews,
 * selection boxes, and other visual feedback elements.
 */
class MapOverlayRenderer {
public:
    explicit MapOverlayRenderer(MapDrawingPrimitives* primitives);
    ~MapOverlayRenderer() = default;
    
    // Overlay management
    void setShowBrushPreview(bool show) { showBrushPreview_ = show; }
    void setShowSelectionBox(bool show) { showSelectionBox_ = show; }
    void setShowGrid(bool show) { showGrid_ = show; }
    
    // Brush preview state
    void setBrushPreviewState(const QPointF& position, Brush* brush, int size, 
                             BrushShape shape, BrushValidity validity);
    void clearBrushPreview();
    
    // Selection state
    void setSelectionArea(const QRectF& area);
    void setSelectionTiles(const QVector<QRectF>& tiles);
    void clearSelection();
    
    // Drag state
    void setDragArea(const QRectF& area, Brush* brush, BrushShape shape);
    void clearDragArea();
    
    // Main rendering method
    void renderOverlays(QPainter* painter, const QRectF& viewRect, int currentFloor);
    
private:
    MapDrawingPrimitives* primitives_;
    
    // Overlay state
    bool showBrushPreview_ = true;
    bool showSelectionBox_ = true;
    bool showGrid_ = false;
    
    // Brush preview state
    QPointF brushPreviewPos_;
    Brush* brushPreviewBrush_ = nullptr;
    int brushPreviewSize_ = 1;
    BrushShape brushPreviewShape_ = BrushShape::Square;
    BrushValidity brushPreviewValidity_ = BrushValidity::Neutral;
    bool hasBrushPreview_ = false;
    
    // Selection state
    QRectF selectionArea_;
    QVector<QRectF> selectionTiles_;
    bool hasSelection_ = false;
    
    // Drag state
    QRectF dragArea_;
    Brush* dragBrush_ = nullptr;
    BrushShape dragShape_ = BrushShape::Square;
    bool hasDragArea_ = false;
};

#endif // MAPDRAWINGPRIMITIVES_H
