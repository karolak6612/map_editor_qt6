#ifndef TILERENDERER_H
#define TILERENDERER_H

#include <QObject>
#include <QRectF>

// Forward declarations
class Tile;
class QPainter;
struct DrawingOptions;

/**
 * TileRenderer - Handles all rendering operations for Tile objects
 * This class separates rendering logic from data management in the Tile class
 */
class TileRenderer : public QObject {
    Q_OBJECT

public:
    explicit TileRenderer(QObject* parent = nullptr);
    ~TileRenderer();

    // Main drawing method
    static void draw(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

    // Static helper methods for drawing different components
    static void drawGroundOnly(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawSelectionHighlight(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawGround(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawItems(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawCreatures(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawSpawn(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawZoneFlags(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawDebugInfo(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

    // Enhanced placeholder rendering
    static void drawGroundPlaceholder(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

private:
    // Private helper methods
    static bool shouldSkipRendering(const Tile* tile, const DrawingOptions& options);
    static void applyColorModulation(QPainter* painter, const DrawingOptions& options);
    static void sortItemsByZOrder(QList<class Item*>& items);
};

#endif // TILERENDERER_H
