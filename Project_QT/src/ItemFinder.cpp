#include "ItemFinder.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "ItemManager.h"
#include "Selection.h"
#include <QDebug>

// ItemFinder implementation

// Static search methods for items on tiles
QList<Item*> ItemFinder::findItemsOnTile(Tile* tile, const SearchCriteria& criteria) {
    QList<Item*> results;
    
    if (!tile) {
        return results;
    }
    
    // Check ground item
    Item* ground = tile->getGround();
    if (ground && itemMatchesCriteria(ground, criteria)) {
        results.append(ground);
    }
    
    // Check all items on tile
    const QVector<Item*>& items = tile->items();
    for (Item* item : items) {
        if (item && itemMatchesCriteria(item, criteria)) {
            results.append(item);
        }
    }
    
    return results;
}

QList<Item*> ItemFinder::findItemsOnTile(Tile* tile, quint16 serverId) {
    SearchCriteria criteria = createServerIdCriteria(serverId);
    return findItemsOnTile(tile, criteria);
}

QList<Item*> ItemFinder::findItemsOnTile(Tile* tile, const QSet<quint16>& serverIds) {
    SearchCriteria criteria = createServerIdsCriteria(serverIds);
    return findItemsOnTile(tile, criteria);
}

QList<Item*> ItemFinder::findItemsOnTile(Tile* tile, ItemTypes_t itemType) {
    SearchCriteria criteria = createItemTypeCriteria(itemType);
    return findItemsOnTile(tile, criteria);
}

QList<Item*> ItemFinder::findItemsOnTile(Tile* tile, const ItemPredicate& predicate) {
    QList<Item*> results;
    
    if (!tile || !predicate) {
        return results;
    }
    
    // Check ground item
    Item* ground = tile->getGround();
    if (ground && predicate(ground)) {
        results.append(ground);
    }
    
    // Check all items on tile
    const QVector<Item*>& items = tile->items();
    for (Item* item : items) {
        if (item && predicate(item)) {
            results.append(item);
        }
    }
    
    return results;
}

// Static search methods for items in maps
QList<ItemFinder::ItemResult> ItemFinder::findAllItemsInMap(Map* map, const SearchCriteria& criteria) {
    QList<ItemResult> results;
    
    if (!map) {
        return results;
    }
    
    iterateMapTiles(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;
        
        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }
        
        QList<Item*> tileItems = findItemsOnTile(tile, criteria);
        for (Item* item : tileItems) {
            results.append(ItemResult(item, tile, pos));
        }
    });
    
    return results;
}

QList<ItemFinder::ItemResult> ItemFinder::findAllItemsInMap(Map* map, quint16 serverId) {
    SearchCriteria criteria = createServerIdCriteria(serverId);
    return findAllItemsInMap(map, criteria);
}

QList<ItemFinder::ItemResult> ItemFinder::findAllItemsInMap(Map* map, const QSet<quint16>& serverIds) {
    SearchCriteria criteria = createServerIdsCriteria(serverIds);
    return findAllItemsInMap(map, criteria);
}

QList<ItemFinder::ItemResult> ItemFinder::findAllItemsInMap(Map* map, ItemTypes_t itemType) {
    SearchCriteria criteria = createItemTypeCriteria(itemType);
    return findAllItemsInMap(map, criteria);
}

QList<ItemFinder::ItemResult> ItemFinder::findAllItemsInMap(Map* map, const ItemPredicate& predicate) {
    QList<ItemResult> results;
    
    if (!map || !predicate) {
        return results;
    }
    
    iterateMapTiles(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;
        
        QList<Item*> tileItems = findItemsOnTile(tile, predicate);
        for (Item* item : tileItems) {
            results.append(ItemResult(item, tile, pos));
        }
    });
    
    return results;
}

// Specialized search methods (replacing wxwidgets find*Brush methods)
Item* ItemFinder::findGroundItem(Tile* tile) {
    if (!tile) {
        return nullptr;
    }
    
    return tile->getGround();
}

QList<Item*> ItemFinder::findWallItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isWall();
    });
}

QList<Item*> ItemFinder::findDoodadItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isDoodad();
    });
}

QList<Item*> ItemFinder::findContainerItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isContainer();
    });
}

QList<Item*> ItemFinder::findTeleportItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isTeleport();
    });
}

QList<Item*> ItemFinder::findDoorItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isDoor();
    });
}

QList<Item*> ItemFinder::findTableItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isTable();
    });
}

QList<Item*> ItemFinder::findCarpetItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isCarpet();
    });
}

QList<Item*> ItemFinder::findBorderItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isBorder();
    });
}

// Property-based searches
QList<Item*> ItemFinder::findItemsWithProperty(Tile* tile, const QString& propertyKey, const QVariant& value) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [propertyKey, value](const Item* item) {
        return item && item->getAttribute(propertyKey) == value;
    });
}

QList<Item*> ItemFinder::findItemsWithAttribute(Tile* tile, const QString& attributeKey, const QVariant& value) {
    return findItemsWithProperty(tile, attributeKey, value);
}

QList<Item*> ItemFinder::findMovableItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isMoveable();
    });
}

QList<Item*> ItemFinder::findBlockingItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isBlocking();
    });
}

QList<Item*> ItemFinder::findStackableItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isStackable();
    });
}

QList<Item*> ItemFinder::findSelectedItems(Tile* tile) {
    if (!tile) {
        return QList<Item*>();
    }
    
    return findItemsOnTile(tile, [](const Item* item) {
        return item && item->isSelected();
    });
}

// Map-wide property searches
QList<ItemFinder::ItemResult> ItemFinder::findItemsWithPropertyInMap(Map* map, const QString& propertyKey, const QVariant& value) {
    SearchCriteria criteria;
    criteria.requiredAttributes[propertyKey] = value;
    return findAllItemsInMap(map, criteria);
}

QList<ItemFinder::ItemResult> ItemFinder::findItemsWithAttributeInMap(Map* map, const QString& attributeKey, const QVariant& value) {
    return findItemsWithPropertyInMap(map, attributeKey, value);
}

QList<ItemFinder::ItemResult> ItemFinder::findDuplicateItems(Map* map, const QVector<QPair<quint16, quint16>>& ranges) {
    QList<ItemResult> results;
    
    if (!map) {
        return results;
    }
    
    QMap<quint16, QList<ItemResult>> itemsByServerId;
    
    // Collect all items grouped by server ID
    iterateMapTiles(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;
        
        // Check ground item
        Item* ground = tile->getGround();
        if (ground) {
            quint16 serverId = ground->getServerId();
            
            // Check if item is in specified ranges (if any)
            bool inRange = ranges.isEmpty();
            if (!inRange) {
                for (const auto& range : ranges) {
                    if (serverId >= range.first && serverId <= range.second) {
                        inRange = true;
                        break;
                    }
                }
            }
            
            if (inRange) {
                itemsByServerId[serverId].append(ItemResult(ground, tile, pos));
            }
        }
        
        // Check all items on tile
        const QVector<Item*>& items = tile->items();
        for (Item* item : items) {
            if (!item) continue;
            
            quint16 serverId = item->getServerId();
            
            // Check if item is in specified ranges (if any)
            bool inRange = ranges.isEmpty();
            if (!inRange) {
                for (const auto& range : ranges) {
                    if (serverId >= range.first && serverId <= range.second) {
                        inRange = true;
                        break;
                    }
                }
            }
            
            if (inRange) {
                itemsByServerId[serverId].append(ItemResult(item, tile, pos));
            }
        }
    });
    
    // Find duplicates (items that appear more than once on the same tile)
    for (auto it = itemsByServerId.begin(); it != itemsByServerId.end(); ++it) {
        const QList<ItemResult>& itemList = it.value();
        
        // Group by tile to find duplicates on same tile
        QMap<Tile*, QList<ItemResult>> itemsByTile;
        for (const ItemResult& result : itemList) {
            itemsByTile[result.tile].append(result);
        }
        
        // Add duplicates to results (keep first, mark rest as duplicates)
        for (auto tileIt = itemsByTile.begin(); tileIt != itemsByTile.end(); ++tileIt) {
            const QList<ItemResult>& tileItems = tileIt.value();
            if (tileItems.size() > 1) {
                // Add all but the first as duplicates
                for (int i = 1; i < tileItems.size(); ++i) {
                    results.append(tileItems[i]);
                }
            }
        }
    }
    
    return results;
}

QList<ItemFinder::ItemResult> ItemFinder::findInvalidItems(Map* map) {
    if (!map) {
        return QList<ItemResult>();
    }

    ItemManager* itemManager = ItemManager::getInstancePtr();
    if (!itemManager) {
        return QList<ItemResult>();
    }

    return findAllItemsInMap(map, [itemManager](const Item* item) {
        return item && !itemManager->itemTypeExists(item->getServerId());
    });
}

// Advanced search methods
QList<ItemFinder::ItemResult> ItemFinder::findItemsInArea(Map* map, const QRect& area, const SearchCriteria& criteria) {
    QList<ItemResult> results;

    if (!map) {
        return results;
    }

    iterateMapTilesInArea(map, area, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }

        QList<Item*> tileItems = findItemsOnTile(tile, criteria);
        for (Item* item : tileItems) {
            results.append(ItemResult(item, tile, pos));
        }
    });

    return results;
}

QList<ItemFinder::ItemResult> ItemFinder::findItemsInSelection(Map* map, const SearchCriteria& criteria) {
    QList<ItemResult> results;

    if (!map) {
        return results;
    }

    iterateMapTilesInSelection(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }

        QList<Item*> tileItems = findItemsOnTile(tile, criteria);
        for (Item* item : tileItems) {
            results.append(ItemResult(item, tile, pos));
        }
    });

    return results;
}

QList<ItemFinder::ItemResult> ItemFinder::findItemsOnLayer(Map* map, int layer, const SearchCriteria& criteria) {
    QList<ItemResult> results;

    if (!map) {
        return results;
    }

    iterateMapTilesOnLayer(map, layer, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        QList<Item*> tileItems = findItemsOnTile(tile, criteria);
        for (Item* item : tileItems) {
            results.append(ItemResult(item, tile, pos));
        }
    });

    return results;
}

// Count methods
int ItemFinder::countItemsOnTile(Tile* tile, const SearchCriteria& criteria) {
    return findItemsOnTile(tile, criteria).size();
}

int ItemFinder::countItemsInMap(Map* map, const SearchCriteria& criteria) {
    return findAllItemsInMap(map, criteria).size();
}

// Utility methods
bool ItemFinder::itemMatchesCriteria(const Item* item, const SearchCriteria& criteria) {
    if (!item) {
        return false;
    }

    // Check server IDs
    if (!criteria.serverIds.isEmpty() && !matchesServerIds(item, criteria.serverIds)) {
        return false;
    }

    // Check client IDs
    if (!criteria.clientIds.isEmpty() && !matchesClientIds(item, criteria.clientIds)) {
        return false;
    }

    // Check item types
    if (!criteria.itemTypes.isEmpty() && !matchesItemTypes(item, criteria.itemTypes)) {
        return false;
    }

    // Check item groups
    if (!criteria.itemGroups.isEmpty() && !matchesItemGroups(item, criteria.itemGroups)) {
        return false;
    }

    // Check type names
    if (!criteria.typeNames.isEmpty() && !matchesTypeNames(item, criteria.typeNames)) {
        return false;
    }

    // Check required attributes
    if (!criteria.requiredAttributes.isEmpty() && !matchesAttributes(item, criteria.requiredAttributes, true)) {
        return false;
    }

    // Check excluded attributes
    if (!criteria.excludedAttributes.isEmpty() && matchesAttributes(item, criteria.excludedAttributes, true)) {
        return false;
    }

    // Check properties
    if (!matchesProperties(item, criteria)) {
        return false;
    }

    // Check custom predicate
    if (criteria.customPredicate && !criteria.customPredicate(item)) {
        return false;
    }

    return true;
}

bool ItemFinder::tileContainsItem(Tile* tile, const SearchCriteria& criteria) {
    return !findItemsOnTile(tile, criteria).isEmpty();
}

// Search criteria builders
ItemFinder::SearchCriteria ItemFinder::createServerIdCriteria(quint16 serverId) {
    SearchCriteria criteria;
    criteria.serverIds.insert(serverId);
    return criteria;
}

ItemFinder::SearchCriteria ItemFinder::createServerIdsCriteria(const QSet<quint16>& serverIds) {
    SearchCriteria criteria;
    criteria.serverIds = serverIds;
    return criteria;
}

ItemFinder::SearchCriteria ItemFinder::createItemTypeCriteria(ItemTypes_t itemType) {
    SearchCriteria criteria;
    criteria.itemTypes.insert(itemType);
    return criteria;
}

ItemFinder::SearchCriteria ItemFinder::createItemGroupCriteria(ItemGroup_t itemGroup) {
    SearchCriteria criteria;
    criteria.itemGroups.insert(itemGroup);
    return criteria;
}

ItemFinder::SearchCriteria ItemFinder::createAttributeCriteria(const QString& key, const QVariant& value) {
    SearchCriteria criteria;
    criteria.requiredAttributes[key] = value;
    return criteria;
}

ItemFinder::SearchCriteria ItemFinder::createPropertyCriteria(const QString& propertyName, bool required) {
    SearchCriteria criteria;

    if (propertyName == "ground") {
        criteria.requiresGround = required;
    } else if (propertyName == "wall") {
        criteria.requiresWall = required;
    } else if (propertyName == "container") {
        criteria.requiresContainer = required;
    } else if (propertyName == "moveable") {
        criteria.requiresMoveable = required;
    } else if (propertyName == "blocking") {
        criteria.requiresBlocking = required;
    } else if (propertyName == "stackable") {
        criteria.requiresStackable = required;
    }

    return criteria;
}

// Internal helper methods
bool ItemFinder::matchesServerIds(const Item* item, const QSet<quint16>& serverIds) {
    return serverIds.contains(item->getServerId());
}

bool ItemFinder::matchesClientIds(const Item* item, const QSet<quint16>& clientIds) {
    return clientIds.contains(item->getClientId());
}

bool ItemFinder::matchesItemTypes(const Item* item, const QSet<ItemTypes_t>& itemTypes) {
    return itemTypes.contains(item->itemType());
}

bool ItemFinder::matchesItemGroups(const Item* item, const QSet<ItemGroup_t>& itemGroups) {
    return itemGroups.contains(item->itemGroup());
}

bool ItemFinder::matchesTypeNames(const Item* item, const QStringList& typeNames) {
    QString itemTypeName = item->typeName();
    for (const QString& typeName : typeNames) {
        if (itemTypeName.contains(typeName, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool ItemFinder::matchesAttributes(const Item* item, const QMap<QString, QVariant>& attributes, bool required) {
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        const QString& key = it.key();
        const QVariant& expectedValue = it.value();
        QVariant actualValue = item->getAttribute(key);

        if (required) {
            // For required attributes, the value must match exactly
            if (actualValue != expectedValue) {
                return false;
            }
        } else {
            // For excluded attributes, if any match, return true (meaning it should be excluded)
            if (actualValue == expectedValue) {
                return true;
            }
        }
    }

    return required; // If required=true and we got here, all matched. If required=false, none matched.
}

bool ItemFinder::matchesProperties(const Item* item, const SearchCriteria& criteria) {
    if (criteria.requiresGround && !item->isGroundTile()) {
        return false;
    }

    if (criteria.requiresWall && !item->isWall()) {
        return false;
    }

    if (criteria.requiresContainer && !item->isContainer()) {
        return false;
    }

    if (criteria.requiresMoveable && !item->isMoveable()) {
        return false;
    }

    if (criteria.requiresBlocking && !item->isBlocking()) {
        return false;
    }

    if (criteria.requiresStackable && !item->isStackable()) {
        return false;
    }

    return true;
}

// Map iteration helpers
void ItemFinder::iterateMapTiles(Map* map, const std::function<void(Tile*, const MapPos&)>& callback) {
    if (!map || !callback) {
        return;
    }

    int width = map->getWidth();
    int height = map->getHeight();
    int layers = map->getLayers();

    for (int z = 0; z < layers; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (tile) {
                    callback(tile, MapPos(x, y, z));
                }
            }
        }
    }
}

void ItemFinder::iterateMapTilesInArea(Map* map, const QRect& area, const std::function<void(Tile*, const MapPos&)>& callback) {
    if (!map || !callback) {
        return;
    }

    int layers = map->getLayers();
    int startX = qMax(0, area.left());
    int endX = qMin(map->getWidth() - 1, area.right());
    int startY = qMax(0, area.top());
    int endY = qMin(map->getHeight() - 1, area.bottom());

    for (int z = 0; z < layers; ++z) {
        for (int y = startY; y <= endY; ++y) {
            for (int x = startX; x <= endX; ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (tile) {
                    callback(tile, MapPos(x, y, z));
                }
            }
        }
    }
}

void ItemFinder::iterateMapTilesInSelection(Map* map, const std::function<void(Tile*, const MapPos&)>& callback) {
    if (!map || !callback) {
        return;
    }

    Selection* selection = map->getSelection();
    if (!selection) {
        return;
    }

    const QList<MapPos>& selectedTiles = selection->getSelectedTiles();
    for (const MapPos& pos : selectedTiles) {
        Tile* tile = map->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            callback(tile, pos);
        }
    }
}

void ItemFinder::iterateMapTilesOnLayer(Map* map, int layer, const std::function<void(Tile*, const MapPos&)>& callback) {
    if (!map || !callback || layer < 0 || layer >= map->getLayers()) {
        return;
    }

    int width = map->getWidth();
    int height = map->getHeight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Tile* tile = map->getTile(x, y, layer);
            if (tile) {
                callback(tile, MapPos(x, y, layer));
            }
        }
    }
}

// TileFinder implementation

// Static search methods for tiles
QList<TileFinder::TileResult> TileFinder::findTilesInMap(Map* map, const TileSearchCriteria& criteria) {
    QList<TileResult> results;

    if (!map) {
        return results;
    }

    ItemFinder::iterateMapTiles(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }

        if (tileMatchesCriteria(tile, criteria)) {
            results.append(TileResult(tile, pos));
        }
    });

    return results;
}

QList<TileFinder::TileResult> TileFinder::findTilesWithItem(Map* map, quint16 serverId) {
    TileSearchCriteria criteria;
    criteria.requiresSpecificItem = true;
    criteria.itemCriteria = ItemFinder::createServerIdCriteria(serverId);
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findTilesWithItems(Map* map, const QSet<quint16>& serverIds) {
    TileSearchCriteria criteria;
    criteria.requiresSpecificItem = true;
    criteria.itemCriteria = ItemFinder::createServerIdsCriteria(serverIds);
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findTilesWithItemType(Map* map, ItemTypes_t itemType) {
    TileSearchCriteria criteria;
    criteria.requiresSpecificItem = true;
    criteria.itemCriteria = ItemFinder::createItemTypeCriteria(itemType);
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findTilesWithProperty(Map* map, const QString& propertyKey, const QVariant& value) {
    TileSearchCriteria criteria;
    criteria.requiresSpecificItem = true;
    criteria.itemCriteria = ItemFinder::createAttributeCriteria(propertyKey, value);
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findTilesWithPredicate(Map* map, const TilePredicate& predicate) {
    QList<TileResult> results;

    if (!map || !predicate) {
        return results;
    }

    ItemFinder::iterateMapTiles(map, [&](Tile* tile, const MapPos& pos) {
        if (tile && predicate(tile)) {
            results.append(TileResult(tile, pos));
        }
    });

    return results;
}

// Specialized tile searches
QList<TileFinder::TileResult> TileFinder::findGroundTiles(Map* map) {
    TileSearchCriteria criteria;
    criteria.requiresGround = true;
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findWallTiles(Map* map) {
    TileSearchCriteria criteria;
    criteria.requiresWall = true;
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findBlockingTiles(Map* map) {
    TileSearchCriteria criteria;
    criteria.requiresBlocking = true;
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findPZTiles(Map* map) {
    TileSearchCriteria criteria;
    criteria.requiresPZ = true;
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findHouseTiles(Map* map, quint32 houseId) {
    TileSearchCriteria criteria;
    criteria.requiresHouse = true;
    if (houseId != 0) {
        criteria.houseId = houseId;
    }
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findSelectedTiles(Map* map) {
    TileSearchCriteria criteria;
    criteria.requiresSelected = true;
    return findTilesInMap(map, criteria);
}

QList<TileFinder::TileResult> TileFinder::findEmptyTiles(Map* map) {
    return findTilesWithPredicate(map, [](const Tile* tile) {
        return tile && tile->isEmpty();
    });
}

// Area-based searches
QList<TileFinder::TileResult> TileFinder::findTilesInArea(Map* map, const QRect& area, const TileSearchCriteria& criteria) {
    QList<TileResult> results;

    if (!map) {
        return results;
    }

    ItemFinder::iterateMapTilesInArea(map, area, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }

        if (tileMatchesCriteria(tile, criteria)) {
            results.append(TileResult(tile, pos));
        }
    });

    return results;
}

QList<TileFinder::TileResult> TileFinder::findTilesInSelection(Map* map, const TileSearchCriteria& criteria) {
    QList<TileResult> results;

    if (!map) {
        return results;
    }

    ItemFinder::iterateMapTilesInSelection(map, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        // Check if layer filtering is needed
        if (!criteria.layers.isEmpty() && !criteria.layers.contains(pos.z)) {
            return;
        }

        if (tileMatchesCriteria(tile, criteria)) {
            results.append(TileResult(tile, pos));
        }
    });

    return results;
}

QList<TileFinder::TileResult> TileFinder::findTilesOnLayer(Map* map, int layer, const TileSearchCriteria& criteria) {
    QList<TileResult> results;

    if (!map) {
        return results;
    }

    ItemFinder::iterateMapTilesOnLayer(map, layer, [&](Tile* tile, const MapPos& pos) {
        if (!tile) return;

        if (tileMatchesCriteria(tile, criteria)) {
            results.append(TileResult(tile, pos));
        }
    });

    return results;
}

// Count methods
int TileFinder::countTilesInMap(Map* map, const TileSearchCriteria& criteria) {
    return findTilesInMap(map, criteria).size();
}

int TileFinder::countTilesWithItem(Map* map, quint16 serverId) {
    return findTilesWithItem(map, serverId).size();
}

// Utility methods
bool TileFinder::tileMatchesCriteria(const Tile* tile, const TileSearchCriteria& criteria) {
    if (!tile) {
        return false;
    }

    // Check tile properties
    if (!matchesTileProperties(tile, criteria)) {
        return false;
    }

    // Check item criteria
    if (criteria.requiresAnyItem && tile->isEmpty()) {
        return false;
    }

    if (criteria.requiresSpecificItem && !matchesItemCriteria(tile, criteria.itemCriteria)) {
        return false;
    }

    // Check zone criteria
    if (!criteria.zoneIds.isEmpty() && !matchesZoneCriteria(tile, criteria.zoneIds)) {
        return false;
    }

    // Check house ID
    if (criteria.houseId != 0 && tile->getHouseId() != criteria.houseId) {
        return false;
    }

    // Check custom predicate
    if (criteria.customPredicate && !criteria.customPredicate(tile)) {
        return false;
    }

    return true;
}

// Search criteria builders
TileFinder::TileSearchCriteria TileFinder::createPropertyCriteria(const QString& propertyName, bool required) {
    TileSearchCriteria criteria;

    if (propertyName == "ground") {
        criteria.requiresGround = required;
    } else if (propertyName == "wall") {
        criteria.requiresWall = required;
    } else if (propertyName == "blocking") {
        criteria.requiresBlocking = required;
    } else if (propertyName == "pz") {
        criteria.requiresPZ = required;
    } else if (propertyName == "nopvp") {
        criteria.requiresNoPVP = required;
    } else if (propertyName == "nologout") {
        criteria.requiresNoLogout = required;
    } else if (propertyName == "pvpzone") {
        criteria.requiresPVPZone = required;
    } else if (propertyName == "house") {
        criteria.requiresHouse = required;
    } else if (propertyName == "selected") {
        criteria.requiresSelected = required;
    }

    return criteria;
}

TileFinder::TileSearchCriteria TileFinder::createItemCriteria(const ItemFinder::SearchCriteria& itemCriteria) {
    TileSearchCriteria criteria;
    criteria.requiresSpecificItem = true;
    criteria.itemCriteria = itemCriteria;
    return criteria;
}

TileFinder::TileSearchCriteria TileFinder::createHouseCriteria(quint32 houseId) {
    TileSearchCriteria criteria;
    criteria.requiresHouse = true;
    criteria.houseId = houseId;
    return criteria;
}

TileFinder::TileSearchCriteria TileFinder::createZoneCriteria(quint16 zoneId) {
    TileSearchCriteria criteria;
    criteria.zoneIds.insert(zoneId);
    return criteria;
}

// Internal helper methods
bool TileFinder::matchesTileProperties(const Tile* tile, const TileSearchCriteria& criteria) {
    if (criteria.requiresGround && !tile->hasGround()) {
        return false;
    }

    if (criteria.requiresWall && !tile->hasWall()) {
        return false;
    }

    if (criteria.requiresBlocking && !tile->isBlocking()) {
        return false;
    }

    if (criteria.requiresPZ && !tile->isPZ()) {
        return false;
    }

    if (criteria.requiresNoPVP && !tile->isNoPVP()) {
        return false;
    }

    if (criteria.requiresNoLogout && !tile->isNoLogout()) {
        return false;
    }

    if (criteria.requiresPVPZone && !tile->isPVPZone()) {
        return false;
    }

    if (criteria.requiresHouse && !tile->isHouseTile()) {
        return false;
    }

    if (criteria.requiresSelected && !tile->isSelected()) {
        return false;
    }

    return true;
}

bool TileFinder::matchesItemCriteria(const Tile* tile, const ItemFinder::SearchCriteria& itemCriteria) {
    return ItemFinder::tileContainsItem(const_cast<Tile*>(tile), itemCriteria);
}

bool TileFinder::matchesZoneCriteria(const Tile* tile, const QSet<quint16>& zoneIds) {
    const QVector<quint16>& tileZoneIds = tile->getZoneIds();

    for (quint16 zoneId : zoneIds) {
        if (tileZoneIds.contains(zoneId)) {
            return true;
        }
    }

    return false;
}
