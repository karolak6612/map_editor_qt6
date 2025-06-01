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
    virtual QString name() const = 0;
    virtual void setName(const QString& newName); // Default implementation asserts
    virtual int getLookID() const = 0; // For UI representation, must be implemented

    // Core brush loading interface (Task 36 requirement)
    virtual bool load(const QDomElement& element, QStringList& warnings);

    // Unique brush ID system (from wxwidgets)
    quint32 getID() const { return id_; }

    // Palette visibility and collection management (from wxwidgets)
    bool isVisibleInPalette() const { return isVisibleInPalette_; }
    void setVisibleInPalette(bool visible) { isVisibleInPalette_ = visible; }
    void flagAsVisible() { isVisibleInPalette_ = true; }

    bool hasCollection() const { return usesCollection_; }
    void setCollection() { usesCollection_ = true; }
    bool usesCollection() const { return usesCollection_; }

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
    virtual bool isOptionalBorder() const;
    virtual bool isCreature() const;
    virtual bool isSpawn() const;
    virtual bool isHouse() const;
    virtual bool isHouseExit() const;
    virtual bool isWaypoint() const;
    virtual bool isFlag() const;
    virtual bool isDoor() const;
    virtual bool isEraser() const;

    // Type casting methods (similar to as[Type] in wxBrush)
    // Default to returning nullptr, overridden by specific brush types
    virtual class RAWBrush* asRaw();
    virtual const class RAWBrush* asRaw() const;
    virtual class DoodadBrush* asDoodad();
    virtual const class DoodadBrush* asDoodad() const;
    virtual class TerrainBrush* asTerrain();
    virtual const class TerrainBrush* asTerrain() const;
    virtual class GroundBrush* asGround();
    virtual const class GroundBrush* asGround() const;
    virtual class WallBrush* asWall();
    virtual const class WallBrush* asWall() const;
    virtual class WallDecorationBrush* asWallDecoration();
    virtual const class WallDecorationBrush* asWallDecoration() const;
    virtual class TableBrush* asTable();
    virtual const class TableBrush* asTable() const;
    virtual class CarpetBrush* asCarpet();
    virtual const class CarpetBrush* asCarpet() const;
    virtual class OptionalBorderBrush* asOptionalBorder();
    virtual const class OptionalBorderBrush* asOptionalBorder() const;
    virtual class CreatureBrush* asCreature();
    virtual const class CreatureBrush* asCreature() const;
    virtual class SpawnBrush* asSpawn();
    virtual const class SpawnBrush* asSpawn() const;
    virtual class HouseBrush* asHouse();
    virtual const class HouseBrush* asHouse() const;
    virtual class HouseExitBrush* asHouseExit();
    virtual const class HouseExitBrush* asHouseExit() const;
    virtual class WaypointBrush* asWaypoint();
    virtual const class WaypointBrush* asWaypoint() const;
    virtual class FlagBrush* asFlag();
    virtual const class FlagBrush* asFlag() const;
    virtual class DoorBrush* asDoor();
    virtual const class DoorBrush* asDoor() const;
    virtual class EraserBrush* asEraser();
    virtual const class EraserBrush* asEraser() const;

    // Core brush action interface (Task 36 requirement)
    virtual bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const = 0;
    virtual QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) = 0;
    virtual QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) = 0;

    // Alternative interface for direct tile manipulation (from wxwidgets)
    virtual void draw(Map* map, class Tile* tile, void* parameter = nullptr);
    virtual void undraw(Map* map, class Tile* tile);
    virtual bool canDraw(Map* map, const QPoint& position) const;

    // Mouse event handling interface (Task 36 requirement)
    virtual void mousePressEvent(QMouseEvent* event, class MapView* mapView);
    virtual void mouseMoveEvent(QMouseEvent* event, class MapView* mapView);
    virtual void mouseReleaseEvent(QMouseEvent* event, class MapView* mapView);

    // Brush state management for complex interactions
    virtual bool startPaint(Map* map, const QPoint& position);
    virtual bool endPaint(Map* map, const QPoint& position);
    virtual void resetBrushState();

    // Shared properties and accessors
    bool isVisibleInPalette() const { return isVisibleInPalette_; }
    void setVisibleInPalette(bool visible) { isVisibleInPalette_ = visible; }

    bool usesCollection() const { return usesCollection_; }
    void setUsesCollection(bool uses) { usesCollection_ = uses; }

    // Add other common properties or methods if identified

protected:
    // Static ID counter for unique brush identification (from wxwidgets)
    static quint32 id_counter_;

    // Instance variables (from wxwidgets)
    quint32 id_;                    // Unique brush ID
    bool isVisibleInPalette_;       // Visible in any palette?
    bool usesCollection_;           // Uses collection system?
};

#endif // BRUSH_H
