#include "Brush.h"
#include "MapView.h"
#include "Map.h"
#include "Tile.h"
#include <QMouseEvent>
#include <QDebug>
#include <QDomElement>

// Static ID counter initialization (from wxwidgets)
quint32 Brush::id_counter_ = 0;

Brush::Brush(QObject *parent)
    : QObject(parent),
      id_(++id_counter_),
      isVisibleInPalette_(true),
      usesCollection_(false) {
    qDebug() << "Brush: Created brush with ID" << id_;
}

Brush::~Brush() {
    qDebug() << "Brush: Destroyed brush with ID" << id_;
}

// Core brush interface implementations (Task 36 requirement)

void Brush::setName(const QString& newName) {
    Q_UNUSED(newName);
    qWarning() << "Brush::setName: setName attempted on nameless brush!";
    Q_ASSERT_X(false, "Brush::setName", "setName attempted on nameless brush!");
}

bool Brush::load(const QDomElement& element, QStringList& warnings) {
    Q_UNUSED(element);
    Q_UNUSED(warnings);
    return true; // Default implementation does nothing
}

// Default implementations for virtual methods from wxBrush

bool Brush::needBorders() const {
    return false;
}

bool Brush::canDrag() const {
    return false;
}

bool Brush::canSmear() const {
    // Original wxBrush defaulted to true, but for Qt, let's default to false
    // as smearing is a specific behavior. Subclasses can override.
    return false;
}

bool Brush::oneSizeFitsAll() const {
    return false;
}

int Brush::getMaxVariation() const {
    return 0;
}

// Default implementations for is[Type]() methods
bool Brush::isRaw() const { return false; }
bool Brush::isDoodad() const { return false; }
bool Brush::isTerrain() const { return false; }
bool Brush::isGround() const { return false; }
bool Brush::isWall() const { return false; }
bool Brush::isWallDecoration() const { return false; }
bool Brush::isTable() const { return false; }
bool Brush::isCarpet() const { return false; }
bool Brush::isOptionalBorder() const { return false; }
bool Brush::isCreature() const { return false; }
bool Brush::isSpawn() const { return false; }
bool Brush::isHouse() const { return false; }
bool Brush::isHouseExit() const { return false; }
bool Brush::isWaypoint() const { return false; }
bool Brush::isFlag() const { return false; }
bool Brush::isDoor() const { return false; }
bool Brush::isEraser() const { return false; }


// Default implementations for as[Type]() methods
class RAWBrush;
class DoodadBrush;
class TerrainBrush;
class GroundBrush;
class WallBrush;
class WallDecorationBrush;
class CreatureBrush;
class SpawnBrush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class FlagBrush;
class DoorBrush;
class EraserBrush;

RAWBrush* Brush::asRaw() { return nullptr; }
const RAWBrush* Brush::asRaw() const { return nullptr; }
DoodadBrush* Brush::asDoodad() { return nullptr; }
const DoodadBrush* Brush::asDoodad() const { return nullptr; }
TerrainBrush* Brush::asTerrain() { return nullptr; }
const TerrainBrush* Brush::asTerrain() const { return nullptr; }
GroundBrush* Brush::asGround() { return nullptr; }
const GroundBrush* Brush::asGround() const { return nullptr; }
WallBrush* Brush::asWall() { return nullptr; }
const WallBrush* Brush::asWall() const { return nullptr; }
WallDecorationBrush* Brush::asWallDecoration() { return nullptr; }
const WallDecorationBrush* Brush::asWallDecoration() const { return nullptr; }
TableBrush* Brush::asTable() { return nullptr; }
const TableBrush* Brush::asTable() const { return nullptr; }

CarpetBrush* Brush::asCarpet() { return nullptr; }
const CarpetBrush* Brush::asCarpet() const { return nullptr; }

OptionalBorderBrush* Brush::asOptionalBorder() { return nullptr; }
const OptionalBorderBrush* Brush::asOptionalBorder() const { return nullptr; }
CreatureBrush* Brush::asCreature() { return nullptr; }
const CreatureBrush* Brush::asCreature() const { return nullptr; }
SpawnBrush* Brush::asSpawn() { return nullptr; }
const SpawnBrush* Brush::asSpawn() const { return nullptr; }
HouseBrush* Brush::asHouse() { return nullptr; }
const HouseBrush* Brush::asHouse() const { return nullptr; }
HouseExitBrush* Brush::asHouseExit() { return nullptr; }
const HouseExitBrush* Brush::asHouseExit() const { return nullptr; }
WaypointBrush* Brush::asWaypoint() { return nullptr; }
const WaypointBrush* Brush::asWaypoint() const { return nullptr; }
FlagBrush* Brush::asFlag() { return nullptr; }
const FlagBrush* Brush::asFlag() const { return nullptr; }
DoorBrush* Brush::asDoor() { return nullptr; }
const DoorBrush* Brush::asDoor() const { return nullptr; }
EraserBrush* Brush::asEraser() { return nullptr; }
const EraserBrush* Brush::asEraser() const { return nullptr; }

// Alternative interface for direct tile manipulation (from wxwidgets)
void Brush::draw(Map* map, Tile* tile, void* parameter) {
    Q_UNUSED(map);
    Q_UNUSED(tile);
    Q_UNUSED(parameter);
    qWarning() << "Brush::draw: Default implementation called - should be overridden by subclass";
}

void Brush::undraw(Map* map, Tile* tile) {
    Q_UNUSED(map);
    Q_UNUSED(tile);
    qWarning() << "Brush::undraw: Default implementation called - should be overridden by subclass";
}

bool Brush::canDraw(Map* map, const QPoint& position) const {
    Q_UNUSED(map);
    Q_UNUSED(position);
    return false; // Default: cannot draw
}

// Mouse event handling interface (Task 36 requirement)
void Brush::mousePressEvent(QMouseEvent* event, MapView* mapView) {
    Q_UNUSED(event);
    Q_UNUSED(mapView);
    // Default implementation does nothing - subclasses can override for custom behavior
}

void Brush::mouseMoveEvent(QMouseEvent* event, MapView* mapView) {
    Q_UNUSED(event);
    Q_UNUSED(mapView);
    // Default implementation does nothing - subclasses can override for custom behavior
}

void Brush::mouseReleaseEvent(QMouseEvent* event, MapView* mapView) {
    Q_UNUSED(event);
    Q_UNUSED(mapView);
    // Default implementation does nothing - subclasses can override for custom behavior
}

// Brush state management for complex interactions
bool Brush::startPaint(Map* map, const QPoint& position) {
    Q_UNUSED(map);
    Q_UNUSED(position);
    return true; // Default: always ready to start painting
}

bool Brush::endPaint(Map* map, const QPoint& position) {
    Q_UNUSED(map);
    Q_UNUSED(position);
    return true; // Default: always successful
}

void Brush::resetBrushState() {
    // Default implementation does nothing - subclasses can override for state cleanup
}


// Implement other common methods or provide default behaviors here if any

// Note: The pure virtual functions (mouse events, name, type, applyBrush, etc.)
// do not have implementations in the base class cpp file.
