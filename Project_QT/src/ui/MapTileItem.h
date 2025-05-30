#ifndef MAPTILEITEM_H
#define MAPTILEITEM_H

#include <QGraphicsObject>
#include <QRectF>

// Adjust path assuming MapTileItem.h is in ui/ and Tile.h/DrawingOptions.h are in src/
#include "../Tile.h" 
#include "../DrawingOptions.h"

// Forward declarations
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class Map; // For Map context

class MapTileItem : public QGraphicsObject {
    Q_OBJECT

public:
    static const int TILE_PIXEL_SIZE = 32;

    MapTileItem(Tile* tile, Map* mapContext, QGraphicsItem* parent = nullptr);
    ~MapTileItem() override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override; // For collision/mouse events
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    Tile* getTile() const { return tile_; }
    Map* getMapContext() const { return mapContext_; }

    // Example collision/walkability properties
    bool isSolid() const;
    bool isWalkable() const;

    // Method to update drawing options if needed (e.g., from a view)
    void setDrawingOptions(const DrawingOptions& options);


protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    Tile* tile_ = nullptr;          // Not owned by MapTileItem
    Map* mapContext_ = nullptr;     // Not owned, used for context
    DrawingOptions drawingOptions_; // Member to hold/cache drawing options
};

#endif // MAPTILEITEM_H
