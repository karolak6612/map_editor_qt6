#ifndef MAPITERATOR_H
#define MAPITERATOR_H

#include <QVector>

// Forward declarations
class Map;
class Tile;

/**
 * High-performance iterator for traversing tiles in a Map.
 * Provides efficient iteration over non-null tiles, similar to the original wxWidgets MapIterator.
 * This iterator skips null tiles automatically for performance.
 */
class MapIterator {
public:
    explicit MapIterator(Map* map, bool atEnd = false);
    MapIterator(const MapIterator& other);
    MapIterator& operator=(const MapIterator& other);
    ~MapIterator();

    // Iterator operations
    MapIterator& operator++(); // Pre-increment
    MapIterator operator++(int); // Post-increment
    bool operator==(const MapIterator& other) const;
    bool operator!=(const MapIterator& other) const;
    
    // Dereference operations
    Tile* operator*() const; // Get current tile
    Tile* operator->() const; // Access current tile members
    Tile* get() const; // Alternative to operator*
    
    // Position information
    int x() const;
    int y() const;
    int z() const;
    int getIndex() const; // Get current flat index in tiles vector
    
    // Validity check
    bool isValid() const;
    bool atEnd() const;
    
    // Reset to beginning
    void reset();

private:
    void findNextValidTile(); // Skip to next non-null tile
    void updatePosition(); // Update x,y,z from current index
    
    Map* map_;
    int currentIndex_;
    int maxIndex_;
    mutable int cachedX_, cachedY_, cachedZ_; // Cached position values
    mutable bool positionCached_; // Whether position cache is valid
};

/**
 * Const version of MapIterator for read-only iteration
 */
class ConstMapIterator {
public:
    explicit ConstMapIterator(const Map* map, bool atEnd = false);
    ConstMapIterator(const ConstMapIterator& other);
    ConstMapIterator& operator=(const ConstMapIterator& other);
    ~ConstMapIterator();

    // Iterator operations
    ConstMapIterator& operator++(); // Pre-increment
    ConstMapIterator operator++(int); // Post-increment
    bool operator==(const ConstMapIterator& other) const;
    bool operator!=(const ConstMapIterator& other) const;
    
    // Dereference operations (const)
    const Tile* operator*() const; // Get current tile
    const Tile* operator->() const; // Access current tile members
    const Tile* get() const; // Alternative to operator*
    
    // Position information
    int x() const;
    int y() const;
    int z() const;
    int getIndex() const; // Get current flat index in tiles vector
    
    // Validity check
    bool isValid() const;
    bool atEnd() const;
    
    // Reset to beginning
    void reset();

private:
    void findNextValidTile(); // Skip to next non-null tile
    void updatePosition(); // Update x,y,z from current index
    
    const Map* map_;
    int currentIndex_;
    int maxIndex_;
    mutable int cachedX_, cachedY_, cachedZ_; // Cached position values
    mutable bool positionCached_; // Whether position cache is valid
};

#endif // MAPITERATOR_H
