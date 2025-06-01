#ifndef MAPUTILITIES_H
#define MAPUTILITIES_H

#include "Map.h"
#include "MapIterator.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include <QDebug>

/**
 * Template utility functions for high-performance map operations.
 * These functions mirror the original wxWidgets foreach_ItemOnMap functionality
 * but are optimized for Qt data structures and provide better type safety.
 */

/**
 * Iterate over all items on the map, calling the provided function object for each item.
 * This is a high-performance template function that skips empty tiles automatically.
 * 
 * @param map The map to iterate over
 * @param foreachFunc Function object that will be called for each item
 * @param selectedTilesOnly If true, only process items on selected tiles
 * 
 * Function signature for foreachFunc should be:
 * void operator()(Map& map, Tile* tile, Item* item, quint64 itemCount)
 */
template <typename ForeachType>
inline void foreach_ItemOnMap(Map& map, ForeachType& foreachFunc, bool selectedTilesOnly = false) {
    MapIterator tileIter = map.begin();
    MapIterator end = map.end();
    quint64 itemCount = 0;

    while (tileIter != end) {
        Tile* tile = tileIter.get();
        if (!tile) {
            ++tileIter;
            continue;
        }

        // Skip non-selected tiles if requested
        if (selectedTilesOnly && !tile->isSelected()) {
            ++tileIter;
            continue;
        }

        // Process ground item
        Item* ground = tile->getGround();
        if (ground) {
            ++itemCount;
            foreachFunc(map, tile, ground, itemCount);
        }

        // Process all other items on the tile
        const QVector<Item*>& items = tile->items();
        for (Item* item : items) {
            if (item && item != ground) { // Skip ground item (already processed)
                ++itemCount;
                foreachFunc(map, tile, item, itemCount);
            }
        }

        ++tileIter;
    }
}

/**
 * Const version of foreach_ItemOnMap for read-only operations
 */
template <typename ForeachType>
inline void foreach_ItemOnMap_const(const Map& map, ForeachType& foreachFunc, bool selectedTilesOnly = false) {
    ConstMapIterator tileIter = map.begin();
    ConstMapIterator end = map.end();
    quint64 itemCount = 0;

    while (tileIter != end) {
        const Tile* tile = tileIter.get();
        if (!tile) {
            ++tileIter;
            continue;
        }

        // Skip non-selected tiles if requested
        if (selectedTilesOnly && !tile->isSelected()) {
            ++tileIter;
            continue;
        }

        // Process ground item
        Item* ground = tile->getGround();
        if (ground) {
            ++itemCount;
            foreachFunc(map, tile, ground, itemCount);
        }

        // Process all other items on the tile
        const QVector<Item*>& items = tile->items();
        for (Item* item : items) {
            if (item && item != ground) { // Skip ground item (already processed)
                ++itemCount;
                foreachFunc(map, tile, item, itemCount);
            }
        }

        ++tileIter;
    }
}

/**
 * Iterate over all tiles on the map, calling the provided function object for each tile.
 * This skips empty tiles automatically for performance.
 * 
 * @param map The map to iterate over
 * @param foreachFunc Function object that will be called for each tile
 * @param selectedTilesOnly If true, only process selected tiles
 * 
 * Function signature for foreachFunc should be:
 * void operator()(Map& map, Tile* tile, quint64 tileCount)
 */
template <typename ForeachType>
inline void foreach_TileOnMap(Map& map, ForeachType& foreachFunc, bool selectedTilesOnly = false) {
    MapIterator tileIter = map.begin();
    MapIterator end = map.end();
    quint64 tileCount = 0;

    while (tileIter != end) {
        Tile* tile = tileIter.get();
        if (!tile) {
            ++tileIter;
            continue;
        }

        // Skip non-selected tiles if requested
        if (selectedTilesOnly && !tile->isSelected()) {
            ++tileIter;
            continue;
        }

        ++tileCount;
        foreachFunc(map, tile, tileCount);
        ++tileIter;
    }
}

/**
 * Iterate over all creatures on the map, calling the provided function object for each creature.
 * 
 * @param map The map to iterate over
 * @param foreachFunc Function object that will be called for each creature
 * @param selectedTilesOnly If true, only process creatures on selected tiles
 * 
 * Function signature for foreachFunc should be:
 * void operator()(Map& map, Tile* tile, Creature* creature, quint64 creatureCount)
 */
template <typename ForeachType>
inline void foreach_CreatureOnMap(Map& map, ForeachType& foreachFunc, bool selectedTilesOnly = false) {
    MapIterator tileIter = map.begin();
    MapIterator end = map.end();
    quint64 creatureCount = 0;

    while (tileIter != end) {
        Tile* tile = tileIter.get();
        if (!tile) {
            ++tileIter;
            continue;
        }

        // Skip non-selected tiles if requested
        if (selectedTilesOnly && !tile->isSelected()) {
            ++tileIter;
            continue;
        }

        // Process creature on the tile
        Creature* creature = tile->creature();
        if (creature) {
            ++creatureCount;
            foreachFunc(map, tile, creature, creatureCount);
        }

        ++tileIter;
    }
}

/**
 * Utility function to count items of specific types on the map
 * 
 * @param map The map to analyze
 * @param itemIds Set of item IDs to count
 * @param selectedTilesOnly If true, only count items on selected tiles
 * @return Total count of matching items
 */
inline quint64 countItemsOnMap(const Map& map, const QSet<quint16>& itemIds, bool selectedTilesOnly = false) {
    quint64 count = 0;
    
    auto countFunc = [&](const Map&, const Tile*, Item* item, quint64) {
        if (itemIds.contains(item->getID())) {
            ++count;
        }
    };
    
    foreach_ItemOnMap_const(map, countFunc, selectedTilesOnly);
    return count;
}

/**
 * Utility function to find all tiles containing specific item types
 * 
 * @param map The map to search
 * @param itemIds Set of item IDs to find
 * @param selectedTilesOnly If true, only search selected tiles
 * @return Vector of tiles containing the specified items
 */
inline QVector<Tile*> findTilesWithItems(Map& map, const QSet<quint16>& itemIds, bool selectedTilesOnly = false) {
    QVector<Tile*> result;
    QSet<Tile*> foundTiles; // Avoid duplicates
    
    auto findFunc = [&](Map&, Tile* tile, Item* item, quint64) {
        if (itemIds.contains(item->getID()) && !foundTiles.contains(tile)) {
            result.append(tile);
            foundTiles.insert(tile);
        }
    };
    
    foreach_ItemOnMap(map, findFunc, selectedTilesOnly);
    return result;
}

#endif // MAPUTILITIES_H
