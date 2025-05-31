#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QMouseEvent> // For event details if needed by brushes

// Forward declarations
class MapView;
class Tile;
class TableBrush;
class CarpetBrush;
class OptionalBorderBrush; // <-- Add this
class BaseMap;
class QUndoCommand; // Added
class Map;          // Added
class QUndoStack;   // Added

// Define BrushShape enum here or in a common types header
enum class BrushShape {
    Square,
    Circle
    // Add other shapes like Line, Custom etc. if needed
};
// Q_ENUM(BrushShape) // Moved inside class if Brush is QObject

class Brush : public QObject {
    Q_OBJECT
public:
    // Make enums part of the class and use Q_ENUM if Brush is QObject
    enum class BrushShape { // Re-declared inside class for Q_ENUM
        Square,
        Circle
    };
    Q_ENUM(BrushShape)

    enum class Type {
        Unknown,
        Raw,
        Doodad,
        Terrain,
        Ground,
        Wall,
        WallDecoration,
        Table,
        Carpet,
        Door,
        OptionalBorder,
        Creature,
        Spawn,
        House,
        HouseExit,
        Waypoint,
        Flag,
        Eraser,
        Pixel
        // TODO: Add any other brush types from wxwidgets/brush_enums.h
    };
    Q_ENUM(Type)

    explicit Brush(QObject *parent = nullptr);
    virtual ~Brush();

    // Method to get brush type
    virtual Type type() const = 0;

    // Pure virtual methods for mouse interaction
    // These will be called by MapViewInputHandler, passing necessary context.
    virtual QUndoCommand* mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) = 0;
    virtual QUndoCommand* mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) = 0;
    virtual QUndoCommand* mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) = 0;

    // Method to cancel ongoing brush operation
    virtual void cancel() = 0; // Added based on MapViewInputHandler usage

    // Brush properties
    virtual int getBrushSize() const = 0;    // Size typically means radius or half-width
    virtual BrushShape getBrushShape() const = 0;

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
    virtual bool isWallDecoration() const;
    virtual bool isTable() const;
    virtual bool isCarpet() const;
    virtual bool isOptionalBorder() const; // <-- Add this
    // ... (add other is[Type] from wxBrush as needed, defaulting to false) ...
    virtual bool isEraser() const;

    // Type casting methods (similar to as[Type] in wxBrush)
    // Default to returning nullptr, overridden by specific brush types
    // Example for WallBrush might be: virtual WallBrush* asWall() { return nullptr; }
    virtual class TableBrush* asTable();
    virtual const class TableBrush* asTable() const;
    virtual class CarpetBrush* asCarpet();
    virtual const class CarpetBrush* asCarpet() const;
    virtual class OptionalBorderBrush* asOptionalBorder(); // <-- Add this
    virtual const class OptionalBorderBrush* asOptionalBorder() const; // <-- Add this

    // Core brush action interface
    virtual bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const = 0;
    virtual QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) = 0;
    virtual QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) = 0;

    // Shared properties and accessors
    bool isVisibleInPalette() const { return isVisibleInPalette_; }
    void setVisibleInPalette(bool visible) { isVisibleInPalette_ = visible; }

    bool usesCollection() const { return usesCollection_; }
    void setUsesCollection(bool uses) { usesCollection_ = uses; }

    // Add other common properties or methods if identified

protected:
    // Common properties like brush ID can be added if necessary
    bool isVisibleInPalette_ = true;
    bool usesCollection_ = false;
    // static uint32_t id_counter; // If unique IDs are needed for Qt brushes
    // uint32_t id_;
};

#endif // BRUSH_H
