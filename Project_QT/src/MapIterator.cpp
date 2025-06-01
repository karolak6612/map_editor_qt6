#include "MapIterator.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>

// --- MapIterator Implementation ---

MapIterator::MapIterator(Map* map, bool atEnd) 
    : map_(map), currentIndex_(atEnd ? -1 : 0), maxIndex_(0), 
      cachedX_(0), cachedY_(0), cachedZ_(0), positionCached_(false) {
    if (map_) {
        maxIndex_ = map_->width() * map_->height() * map_->floors();
        if (!atEnd && maxIndex_ > 0) {
            findNextValidTile(); // Find first valid tile
        } else {
            currentIndex_ = maxIndex_; // Set to end
        }
    }
}

MapIterator::MapIterator(const MapIterator& other) 
    : map_(other.map_), currentIndex_(other.currentIndex_), maxIndex_(other.maxIndex_),
      cachedX_(other.cachedX_), cachedY_(other.cachedY_), cachedZ_(other.cachedZ_),
      positionCached_(other.positionCached_) {
}

MapIterator& MapIterator::operator=(const MapIterator& other) {
    if (this != &other) {
        map_ = other.map_;
        currentIndex_ = other.currentIndex_;
        maxIndex_ = other.maxIndex_;
        cachedX_ = other.cachedX_;
        cachedY_ = other.cachedY_;
        cachedZ_ = other.cachedZ_;
        positionCached_ = other.positionCached_;
    }
    return *this;
}

MapIterator::~MapIterator() {
    // Nothing to clean up
}

MapIterator& MapIterator::operator++() {
    if (currentIndex_ < maxIndex_) {
        ++currentIndex_;
        positionCached_ = false; // Invalidate position cache
        findNextValidTile();
    }
    return *this;
}

MapIterator MapIterator::operator++(int) {
    MapIterator temp(*this);
    ++(*this);
    return temp;
}

bool MapIterator::operator==(const MapIterator& other) const {
    return map_ == other.map_ && currentIndex_ == other.currentIndex_;
}

bool MapIterator::operator!=(const MapIterator& other) const {
    return !(*this == other);
}

Tile* MapIterator::operator*() const {
    return get();
}

Tile* MapIterator::operator->() const {
    return get();
}

Tile* MapIterator::get() const {
    if (!isValid()) {
        return nullptr;
    }
    
    updatePosition(); // Ensure position is calculated
    return map_->getTile(cachedX_, cachedY_, cachedZ_);
}

int MapIterator::x() const {
    updatePosition();
    return cachedX_;
}

int MapIterator::y() const {
    updatePosition();
    return cachedY_;
}

int MapIterator::z() const {
    updatePosition();
    return cachedZ_;
}

int MapIterator::getIndex() const {
    return currentIndex_;
}

bool MapIterator::isValid() const {
    return map_ && currentIndex_ >= 0 && currentIndex_ < maxIndex_;
}

bool MapIterator::atEnd() const {
    return !isValid() || currentIndex_ >= maxIndex_;
}

void MapIterator::reset() {
    currentIndex_ = 0;
    positionCached_ = false;
    if (maxIndex_ > 0) {
        findNextValidTile();
    }
}

void MapIterator::findNextValidTile() {
    if (!map_) return;
    
    while (currentIndex_ < maxIndex_) {
        updatePosition(); // Calculate position for current index
        Tile* tile = map_->getTile(cachedX_, cachedY_, cachedZ_);
        if (tile) {
            return; // Found valid tile
        }
        ++currentIndex_;
        positionCached_ = false;
    }
    // Reached end without finding valid tile
}

void MapIterator::updatePosition() {
    if (positionCached_ || !map_) return;
    
    int width = map_->width();
    int height = map_->height();
    
    if (width > 0 && height > 0) {
        cachedZ_ = currentIndex_ / (width * height);
        int remainder = currentIndex_ % (width * height);
        cachedY_ = remainder / width;
        cachedX_ = remainder % width;
        positionCached_ = true;
    }
}

// --- ConstMapIterator Implementation ---

ConstMapIterator::ConstMapIterator(const Map* map, bool atEnd) 
    : map_(map), currentIndex_(atEnd ? -1 : 0), maxIndex_(0), 
      cachedX_(0), cachedY_(0), cachedZ_(0), positionCached_(false) {
    if (map_) {
        maxIndex_ = map_->width() * map_->height() * map_->floors();
        if (!atEnd && maxIndex_ > 0) {
            findNextValidTile(); // Find first valid tile
        } else {
            currentIndex_ = maxIndex_; // Set to end
        }
    }
}

ConstMapIterator::ConstMapIterator(const ConstMapIterator& other) 
    : map_(other.map_), currentIndex_(other.currentIndex_), maxIndex_(other.maxIndex_),
      cachedX_(other.cachedX_), cachedY_(other.cachedY_), cachedZ_(other.cachedZ_),
      positionCached_(other.positionCached_) {
}

ConstMapIterator& ConstMapIterator::operator=(const ConstMapIterator& other) {
    if (this != &other) {
        map_ = other.map_;
        currentIndex_ = other.currentIndex_;
        maxIndex_ = other.maxIndex_;
        cachedX_ = other.cachedX_;
        cachedY_ = other.cachedY_;
        cachedZ_ = other.cachedZ_;
        positionCached_ = other.positionCached_;
    }
    return *this;
}

ConstMapIterator::~ConstMapIterator() {
    // Nothing to clean up
}

ConstMapIterator& ConstMapIterator::operator++() {
    if (currentIndex_ < maxIndex_) {
        ++currentIndex_;
        positionCached_ = false; // Invalidate position cache
        findNextValidTile();
    }
    return *this;
}

ConstMapIterator ConstMapIterator::operator++(int) {
    ConstMapIterator temp(*this);
    ++(*this);
    return temp;
}

bool ConstMapIterator::operator==(const ConstMapIterator& other) const {
    return map_ == other.map_ && currentIndex_ == other.currentIndex_;
}

bool ConstMapIterator::operator!=(const ConstMapIterator& other) const {
    return !(*this == other);
}

const Tile* ConstMapIterator::operator*() const {
    return get();
}

const Tile* ConstMapIterator::operator->() const {
    return get();
}

const Tile* ConstMapIterator::get() const {
    if (!isValid()) {
        return nullptr;
    }
    
    updatePosition(); // Ensure position is calculated
    return map_->getTile(cachedX_, cachedY_, cachedZ_);
}

int ConstMapIterator::x() const {
    updatePosition();
    return cachedX_;
}

int ConstMapIterator::y() const {
    updatePosition();
    return cachedY_;
}

int ConstMapIterator::z() const {
    updatePosition();
    return cachedZ_;
}

int ConstMapIterator::getIndex() const {
    return currentIndex_;
}

bool ConstMapIterator::isValid() const {
    return map_ && currentIndex_ >= 0 && currentIndex_ < maxIndex_;
}

bool ConstMapIterator::atEnd() const {
    return !isValid() || currentIndex_ >= maxIndex_;
}

void ConstMapIterator::reset() {
    currentIndex_ = 0;
    positionCached_ = false;
    if (maxIndex_ > 0) {
        findNextValidTile();
    }
}

void ConstMapIterator::findNextValidTile() {
    if (!map_) return;
    
    while (currentIndex_ < maxIndex_) {
        updatePosition(); // Calculate position for current index
        const Tile* tile = map_->getTile(cachedX_, cachedY_, cachedZ_);
        if (tile) {
            return; // Found valid tile
        }
        ++currentIndex_;
        positionCached_ = false;
    }
    // Reached end without finding valid tile
}

void ConstMapIterator::updatePosition() {
    if (positionCached_ || !map_) return;
    
    int width = map_->width();
    int height = map_->height();
    
    if (width > 0 && height > 0) {
        cachedZ_ = currentIndex_ / (width * height);
        int remainder = currentIndex_ % (width * height);
        cachedY_ = remainder / width;
        cachedX_ = remainder % width;
        positionCached_ = true;
    }
}
