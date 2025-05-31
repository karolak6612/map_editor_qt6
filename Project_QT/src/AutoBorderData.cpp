#include "AutoBorderData.h"
// #include <QDebug> // Uncomment if needed for future debugging

AutoBorderData::AutoBorderData(quint32 definitionId)
    : definitionId_(definitionId),
      borderGroupId_(0),       // Explicitly initialize, matches default member init in .h
      definesGroundEquivalent_(false) { // Explicitly initialize, matches default member init in .h
    // edgeItemIds_ is default-constructed (empty QMap)
}

AutoBorderData::~AutoBorderData() {
    // No dynamic memory managed directly by this class's simple members that requires manual cleanup.
    // QMap will handle its own nodes.
}

quint32 AutoBorderData::definitionId() const {
    return definitionId_;
}

quint16 AutoBorderData::borderGroupId() const {
    return borderGroupId_;
}

bool AutoBorderData::definesGroundEquivalent() const {
    return definesGroundEquivalent_;
}

quint16 AutoBorderData::getEdgeItemId(BorderEdgeType edge) const {
    // QMap::value(key, defaultValue) returns defaultValue if key is not found.
    // Using 0 as the default to signify "no item" for that edge.
    return edgeItemIds_.value(edge, 0);
}

void AutoBorderData::setDefinitionId(quint32 id) {
    // This is typically set once at creation from a definition source (e.g., XML).
    // Making it settable allows modification if needed, but consider if it should be const after construction.
    definitionId_ = id;
}

void AutoBorderData::setBorderGroupId(quint16 groupId) {
    borderGroupId_ = groupId;
}

void AutoBorderData::setDefinesGroundEquivalent(bool defines) {
    definesGroundEquivalent_ = defines;
}

void AutoBorderData::setEdgeItemId(BorderEdgeType edge, quint16 itemId) {
    if (itemId == 0) {
        // If itemId is 0, it signifies that no specific item should be used for this edge,
        // or to clear a previously set item for this edge.
        // Removing the key from the map is an efficient way to represent this.
        edgeItemIds_.remove(edge);
    } else {
        edgeItemIds_.insert(edge, itemId);
    }
}

void AutoBorderData::clearEdgeItemIds() {
    edgeItemIds_.clear();
}
