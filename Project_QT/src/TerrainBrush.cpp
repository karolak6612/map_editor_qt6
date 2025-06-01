#include "TerrainBrush.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>
#include <QUndoCommand>

TerrainBrush::TerrainBrush(QObject* parent)
    : Brush(parent),
      specificBrushName_("Unnamed Terrain"),
      lookId_(0),
      zOrder_(0),
      hatesFriends_(false) {
    qDebug() << "TerrainBrush: Created terrain brush with ID" << getID();
}

TerrainBrush::~TerrainBrush() {
}

// --- Overrides from Brush ---
QString TerrainBrush::name() const {
    // If specificBrushName_ is empty, it means it wasn't set by a subclass,
    // so provide a more generic name than just "Unnamed Terrain".
    return specificBrushName_.isEmpty() || specificBrushName_ == "Unnamed Terrain" ?
           "Generic Terrain Brush" : specificBrushName_;
}

Brush::Type TerrainBrush::type() const {
    return Brush::Type::Terrain;
}

bool TerrainBrush::isTerrain() const {
    return true;
}

// Type casting methods (Task 37 requirement)
TerrainBrush* TerrainBrush::asTerrain() {
    return this;
}

const TerrainBrush* TerrainBrush::asTerrain() const {
    return this;
}

// Name management (Task 37 requirement)
void TerrainBrush::setName(const QString& newName) {
    setSpecificName(newName);
}

bool TerrainBrush::needBorders() const {
    // Default behavior for most terrain brushes is that they require border calculations.
    return true;
}

bool TerrainBrush::canDrag() const {
    // Terrain brushes are typically draggable to draw areas.
    return true;
}

int TerrainBrush::getLookID() const {
    return lookId_;
}

// --- TerrainBrush specific methods ---
QString TerrainBrush::specificName() const {
    return specificBrushName_;
}

void TerrainBrush::setSpecificName(const QString& name) {
    specificBrushName_ = name;
}

void TerrainBrush::setLookID(quint16 id) {
    lookId_ = id;
}

// Z-order management (from wxwidgets)
qint32 TerrainBrush::getZ() const {
    return zOrder_;
}

void TerrainBrush::setZ(qint32 zOrder) {
    zOrder_ = zOrder;
}

void TerrainBrush::addFriendBrushType(Brush::Type friendType) {
    if (!friendBrushTypes_.contains(friendType)) {
        friendBrushTypes_.append(friendType);
    }
}

void TerrainBrush::removeFriendBrushType(Brush::Type friendType) {
    friendBrushTypes_.removeAll(friendType);
}

void TerrainBrush::setHatesFriends(bool hatesFriends) {
    hatesFriends_ = hatesFriends;
}

bool TerrainBrush::hatesFriends() const {
    return hatesFriends_;
}

bool TerrainBrush::isFriendWith(Brush::Type otherType) const {
    // If the list is empty and we are not in "hates friends" mode,
    // it could mean it's friendly with all other terrains by default.
    // Or, it could mean it's friendly with none unless explicitly added.
    // The RME wxwidgets version had complex logic involving checking against
    // generic terrain types if the specific type wasn't in a friend/enemy list.
    // For now, a simple list check:
    bool foundInList = friendBrushTypes_.contains(otherType);

    // If hatesFriends_ is true, the list contains enemies. So, if found, it's NOT a friend.
    // If hatesFriends_ is false, the list contains friends. So, if found, it IS a friend.
    return hatesFriends_ ? !foundInList : foundInList;
}

// Direct brush comparison (from wxwidgets)
bool TerrainBrush::friendOf(TerrainBrush* other) const {
    if (!other) return false;

    // Check if this brush is friendly with the other brush's type
    bool thisToOther = isFriendWith(other->type());

    // Check if the other brush is friendly with this brush's type
    bool otherToThis = other->isFriendWith(this->type());

    // Both must be friendly for a true friendship
    return thisToOther && otherToThis;
}

// Drawing stubs for direct tile manipulation (Task 37 requirement)
void TerrainBrush::draw(Map* map, Tile* tile, void* parameter) {
    Q_UNUSED(map);
    Q_UNUSED(tile);
    Q_UNUSED(parameter);

    qDebug() << "TerrainBrush::draw: Stub implementation called for" << specificName()
             << "at tile position. Concrete terrain brushes should override this method.";

    // This is a stub implementation. Concrete terrain brushes (GroundBrush, WallBrush, etc.)
    // should override this method to provide actual terrain modification logic.
}

void TerrainBrush::undraw(Map* map, Tile* tile) {
    Q_UNUSED(map);
    Q_UNUSED(tile);

    qDebug() << "TerrainBrush::undraw: Stub implementation called for" << specificName()
             << "at tile position. Concrete terrain brushes should override this method.";

    // This is a stub implementation. Concrete terrain brushes should override this method
    // to provide actual terrain removal logic.
}

bool TerrainBrush::canDraw(Map* map, const QPoint& position) const {
    Q_UNUSED(map);
    Q_UNUSED(position);

    // Default implementation: terrain brushes can generally draw anywhere
    // Concrete terrain brushes can override this for specific restrictions
    return true;
}

// Note: The pure virtual methods inherited from Brush.h, such as:
// - getBrushSize()
// - getBrushShape()
// - canDraw()
// - applyBrush()
// - removeBrush()
// - mousePressEvent() (if made pure in Brush and not implemented here)
// - mouseMoveEvent()  (if made pure in Brush and not implemented here)
// - mouseReleaseEvent() (if made pure in Brush and not implemented here)
// - cancel() (if made pure in Brush and not implemented here)
// ...are NOT implemented in TerrainBrush.cpp because TerrainBrush is also an abstract class.
// Concrete subclasses (e.g., GroundBrush, WallBrush) must implement them.
// The current Brush.h makes mouse events and cancel() non-pure with default implementations,
// so TerrainBrush inherits those defaults unless overridden.
// The `applyBrush`, `removeBrush`, `canDraw` were made pure virtual in TerrainBrush.h in the previous step.
// `getBrushSize`, `getBrushShape` are pure virtual inherited from Brush.h.
