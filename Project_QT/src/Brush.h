#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QMouseEvent> // For event details if needed by brushes

// Forward declarations
class MapView; // Assuming MapView is the class interacting with brushes
class Tile;    // Placeholder for Tile interactions
class BaseMap; // Placeholder for Map interactions (consider if Map or Editor is better)

class Brush : public QObject {
    Q_OBJECT

public:
    explicit Brush(QObject *parent = nullptr);
    virtual ~Brush();

    // Pure virtual methods for mouse interaction
    // These will be called by MapViewInputHandler, passing necessary context.
    virtual void mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) = 0;
    virtual void mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) = 0;
    virtual void mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) = 0;

    // Pure virtual methods for core brush properties/actions (from wxBrush)
    // virtual void draw(BaseMap* map, Tile* tile, void* parameter = nullptr) = 0; // Actual drawing logic deferred
    // virtual void undraw(BaseMap* map, Tile* tile) = 0; // Actual undrawing logic deferred
    // virtual bool canDraw(BaseMap* map, const QPointF& mapPos) const = 0; // Placeholder for now

    virtual QString name() const = 0;
    virtual int getLookID() const; // For UI representation, default 0

    // Virtual methods with default implementations (from wxBrush)
    virtual bool needBorders() const;
    virtual bool canDrag() const;    // For drag-drawing areas
    virtual bool canSmear() const;   // For continuous drawing on mouse move
    virtual bool oneSizeFitsAll() const; // If brush size parameter is ignored
    virtual int getMaxVariation() const;

    // Type identification methods (from wxBrush)
    virtual bool isRaw() const;
    virtual bool isDoodad() const;
    virtual bool isTerrain() const;
    virtual bool isGround() const;
    virtual bool isWall() const;
    // ... (add other is[Type] from wxBrush as needed, defaulting to false) ...
    virtual bool isEraser() const;


    // Add other common properties or methods if identified

protected:
    // Common properties like brush ID can be added if necessary
    // static uint32_t id_counter; // If unique IDs are needed for Qt brushes
    // uint32_t id_;
};

#endif // BRUSH_H
