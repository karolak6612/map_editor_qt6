#include "Selection.h"
#include "Map.h" // Included for MapPos if not fully visible via Selection.h, and for Map context
#include <QDebug> // For potential debug messages

Selection::Selection(Map* mapParent)
    : mapParent_(mapParent), currentMode_(SelectionMode::Tiles) {
    // Initialization as per requirements
}

Selection::~Selection() {
    // No explicit cleanup needed for selectedTiles_ (QSet of MapPos)
    // or mapParent_ (raw pointer, not owned)
}

void Selection::addTile(const MapPos& tilePos) {
    selectedTiles_.insert(tilePos);
}

void Selection::removeTile(const MapPos& tilePos) {
    selectedTiles_.remove(tilePos);
}

void Selection::clear() {
    selectedTiles_.clear();
}

bool Selection::isSelected(const MapPos& tilePos) const {
    return selectedTiles_.contains(tilePos);
}

bool Selection::isEmpty() const {
    return selectedTiles_.isEmpty();
}

QSet<MapPos> Selection::getSelectedTiles() const {
    return selectedTiles_;
}

void Selection::setMode(SelectionMode mode) {
    currentMode_ = mode;
}

SelectionMode Selection::getMode() const {
    return currentMode_;
}

int Selection::count() const {
    return selectedTiles_.count();
}
