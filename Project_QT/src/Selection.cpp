#include "Selection.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include <QDebug>
#include <algorithm>

Selection::Selection(Map* mapParent)
    : QObject(nullptr), mapParent_(mapParent), currentMode_(SelectionMode::Tiles),
      selectionType_(SELECT_CURRENT_FLOOR), busy_(false), compensatedSelect_(true),
      currentSessionFlags_(NONE) {
    // Initialization as per requirements
}

Selection::~Selection() {
    clear();
}

void Selection::addTile(const MapPos& tilePos) {
    if (selectedTiles_.insert(tilePos).second) {
        emit selectionChanged();
    }
}

void Selection::removeTile(const MapPos& tilePos) {
    if (selectedTiles_.remove(tilePos)) {
        emit selectionChanged();
    }
}

void Selection::clear() {
    if (!selectedTiles_.isEmpty() || !selectedTileObjects_.isEmpty()) {
        selectedTiles_.clear();
        selectedTileObjects_.clear();
        emit selectionCleared();
        emit selectionChanged();
    }
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

// --- Enhanced Selection Methods (wxWidgets compatibility) ---

void Selection::add(Tile* tile) {
    if (!tile) return;

    if (selectedTileObjects_.insert(tile).second) {
        if (mapParent_) {
            MapPos pos = mapParent_->getTilePosition(tile);
            selectedTiles_.insert(pos);
        }
        emit selectionChanged();
    }
}

void Selection::add(Tile* tile, Item* item) {
    if (!tile || !item) return;

    // In wxWidgets, this would select the specific item on the tile
    // For now, we'll select the tile and track the item selection state
    if (item->isSelected()) return;

    item->setSelected(true);
    add(tile);
}

void Selection::add(Tile* tile, Creature* creature) {
    if (!tile || !creature) return;

    if (creature->isSelected()) return;

    creature->setSelected(true);
    add(tile);
}

void Selection::add(Tile* tile, Spawn* spawn) {
    if (!tile || !spawn) return;

    if (spawn->isSelected()) return;

    spawn->setSelected(true);
    add(tile);
}

void Selection::remove(Tile* tile) {
    if (!tile) return;

    if (selectedTileObjects_.remove(tile)) {
        if (mapParent_) {
            MapPos pos = mapParent_->getTilePosition(tile);
            selectedTiles_.remove(pos);
        }
        emit selectionChanged();
    }
}

void Selection::remove(Tile* tile, Item* item) {
    if (!tile || !item) return;

    item->setSelected(false);
    // Check if tile should still be selected based on other selected items
    // For simplicity, we'll remove the tile from selection
    remove(tile);
}

void Selection::remove(Tile* tile, Creature* creature) {
    if (!tile || !creature) return;

    creature->setSelected(false);
    remove(tile);
}

void Selection::remove(Tile* tile, Spawn* spawn) {
    if (!tile || !spawn) return;

    spawn->setSelected(false);
    remove(tile);
}

// --- Internal Selection Methods ---

void Selection::addInternal(Tile* tile) {
    if (!tile) return;

    selectedTileObjects_.insert(tile);
    if (mapParent_) {
        MapPos pos = mapParent_->getTilePosition(tile);
        selectedTiles_.insert(pos);
    }
    // No signal emission for internal operations
}

void Selection::removeInternal(Tile* tile) {
    if (!tile) return;

    selectedTileObjects_.remove(tile);
    if (mapParent_) {
        MapPos pos = mapParent_->getTilePosition(tile);
        selectedTiles_.remove(pos);
    }
    // No signal emission for internal operations
}

// --- Mode and Type Management ---

void Selection::setSelectionType(SelectionType type) {
    if (selectionType_ != type) {
        selectionType_ = type;
        emit selectionChanged();
    }
}

SelectionType Selection::getSelectionType() const {
    return selectionType_;
}

// --- Collection Methods ---

size_t Selection::size() const {
    return selectedTiles_.size();
}

QList<Tile*> Selection::getTiles() const {
    return selectedTileObjects_.values();
}

Tile* Selection::getSelectedTile() const {
    if (selectedTileObjects_.size() != 1) {
        qWarning() << "Selection::getSelectedTile() called with" << selectedTileObjects_.size() << "tiles selected";
        return nullptr;
    }
    return *selectedTileObjects_.begin();
}

// --- Bounding Box Methods ---

MapPos Selection::minPosition() const {
    if (selectedTiles_.isEmpty()) {
        return MapPos(0, 0, 0);
    }

    auto it = selectedTiles_.begin();
    MapPos minPos = *it;
    ++it;

    for (; it != selectedTiles_.end(); ++it) {
        const MapPos& pos = *it;
        minPos.x = std::min(minPos.x, pos.x);
        minPos.y = std::min(minPos.y, pos.y);
        minPos.z = std::min(minPos.z, pos.z);
    }

    return minPos;
}

MapPos Selection::maxPosition() const {
    if (selectedTiles_.isEmpty()) {
        return MapPos(0, 0, 0);
    }

    auto it = selectedTiles_.begin();
    MapPos maxPos = *it;
    ++it;

    for (; it != selectedTiles_.end(); ++it) {
        const MapPos& pos = *it;
        maxPos.x = std::max(maxPos.x, pos.x);
        maxPos.y = std::max(maxPos.y, pos.y);
        maxPos.z = std::max(maxPos.z, pos.z);
    }

    return maxPos;
}

// --- Session Management ---

void Selection::start(SessionFlags flags) {
    currentSessionFlags_ = flags;
    busy_ = true;
    emit selectionStarted();
}

void Selection::commit() {
    // In wxWidgets, this would commit the current action to the undo system
    // For Qt implementation, we'll emit a signal for now
    if (busy_) {
        emit selectionChanged();
    }
}

void Selection::finish(SessionFlags flags) {
    currentSessionFlags_ = NONE;
    busy_ = false;
    emit selectionFinished();
}

bool Selection::isBusy() const {
    return busy_;
}

// --- Iterator Support ---

QSet<MapPos>::iterator Selection::begin() {
    return selectedTiles_.begin();
}

QSet<MapPos>::iterator Selection::end() {
    return selectedTiles_.end();
}

QSet<MapPos>::const_iterator Selection::begin() const {
    return selectedTiles_.begin();
}

QSet<MapPos>::const_iterator Selection::end() const {
    return selectedTiles_.end();
}

// --- Selection Validation ---

void Selection::updateSelectionCount() {
    // Update any internal counters or validation
    emit selectionChanged();
}

bool Selection::isCompensatedSelect() const {
    return compensatedSelect_;
}

void Selection::setCompensatedSelect(bool enabled) {
    compensatedSelect_ = enabled;
}


