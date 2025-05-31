#include "TerrainBrush.h"
#include <QDebug> // For potential future debugging, not strictly needed for this part

TerrainBrush::TerrainBrush(QObject* parent)
    : Brush(parent),
      specificBrushName_("Unnamed Terrain"),
      lookId_(0),
      hatesFriends_(false) {
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
