#ifndef ITEMFINDER_H
#define ITEMFINDER_H

#include <QList>
#include <QVector>
#include <QSet>
#include <QString>
#include <QVariant>
#include <functional>
#include "MapPos.h"
#include "Tile.h"
#include "Item.h"
#include "ItemManager.h"

// Forward declarations
class Map;
class Tile;
class Item;

/**
 * @brief ItemFinder - Comprehensive utility for searching items within maps and tiles
 * 
 * This class provides efficient and flexible methods for searching items based on
 * various criteria including item IDs, types, attributes, and custom predicates.
 * It replaces the individual findXXXBrush() methods from wxwidgets with a more
 * centralized and powerful search mechanism.
 */
class ItemFinder {
public:
    // Search criteria types
    using ItemPredicate = std::function<bool(const Item*)>;
    using TilePredicate = std::function<bool(const Tile*)>;
    
    // Search result structures
    struct ItemResult {
        Item* item;
        Tile* tile;
        MapPos position;
        
        ItemResult(Item* i, Tile* t, const MapPos& pos) 
            : item(i), tile(t), position(pos) {}
    };
    
    struct SearchCriteria {
        // Item ID criteria
        QSet<quint16> serverIds;
        QSet<quint16> clientIds;
        
        // Item type criteria
        QSet<ItemTypes_t> itemTypes;
        QSet<ItemGroup_t> itemGroups;
        QStringList typeNames;
        
        // Attribute criteria
        QMap<QString, QVariant> requiredAttributes;
        QMap<QString, QVariant> excludedAttributes;
        
        // Property criteria
        bool requiresGround = false;
        bool requiresWall = false;
        bool requiresContainer = false;
        bool requiresMoveable = false;
        bool requiresBlocking = false;
        bool requiresStackable = false;
        
        // Custom predicate
        ItemPredicate customPredicate;
        
        // Layer/floor filtering
        QSet<int> layers; // Empty means all layers
        
        SearchCriteria() = default;
    };

    // Static search methods for items on tiles
    static QList<Item*> findItemsOnTile(Tile* tile, const SearchCriteria& criteria);
    static QList<Item*> findItemsOnTile(Tile* tile, quint16 serverId);
    static QList<Item*> findItemsOnTile(Tile* tile, const QSet<quint16>& serverIds);
    static QList<Item*> findItemsOnTile(Tile* tile, ItemTypes_t itemType);
    static QList<Item*> findItemsOnTile(Tile* tile, const ItemPredicate& predicate);
    
    // Static search methods for items in maps
    static QList<ItemResult> findAllItemsInMap(Map* map, const SearchCriteria& criteria);
    static QList<ItemResult> findAllItemsInMap(Map* map, quint16 serverId);
    static QList<ItemResult> findAllItemsInMap(Map* map, const QSet<quint16>& serverIds);
    static QList<ItemResult> findAllItemsInMap(Map* map, ItemTypes_t itemType);
    static QList<ItemResult> findAllItemsInMap(Map* map, const ItemPredicate& predicate);
    
    // Specialized search methods (replacing wxwidgets find*Brush methods)
    static Item* findGroundItem(Tile* tile);
    static QList<Item*> findWallItems(Tile* tile);
    static QList<Item*> findDoodadItems(Tile* tile);
    static QList<Item*> findContainerItems(Tile* tile);
    static QList<Item*> findTeleportItems(Tile* tile);
    static QList<Item*> findDoorItems(Tile* tile);
    static QList<Item*> findTableItems(Tile* tile);
    static QList<Item*> findCarpetItems(Tile* tile);
    static QList<Item*> findBorderItems(Tile* tile);
    
    // Property-based searches
    static QList<Item*> findItemsWithProperty(Tile* tile, const QString& propertyKey, const QVariant& value);
    static QList<Item*> findItemsWithAttribute(Tile* tile, const QString& attributeKey, const QVariant& value);
    static QList<Item*> findMovableItems(Tile* tile);
    static QList<Item*> findBlockingItems(Tile* tile);
    static QList<Item*> findStackableItems(Tile* tile);
    static QList<Item*> findSelectedItems(Tile* tile);
    
    // Map-wide property searches
    static QList<ItemResult> findItemsWithPropertyInMap(Map* map, const QString& propertyKey, const QVariant& value);
    static QList<ItemResult> findItemsWithAttributeInMap(Map* map, const QString& attributeKey, const QVariant& value);
    static QList<ItemResult> findDuplicateItems(Map* map, const QVector<QPair<quint16, quint16>>& ranges = QVector<QPair<quint16, quint16>>());
    static QList<ItemResult> findInvalidItems(Map* map);
    
    // Advanced search methods
    static QList<ItemResult> findItemsInArea(Map* map, const QRect& area, const SearchCriteria& criteria);
    static QList<ItemResult> findItemsInSelection(Map* map, const SearchCriteria& criteria);
    static QList<ItemResult> findItemsOnLayer(Map* map, int layer, const SearchCriteria& criteria);
    
    // Count methods
    static int countItemsOnTile(Tile* tile, const SearchCriteria& criteria);
    static int countItemsInMap(Map* map, const SearchCriteria& criteria);
    
    // Utility methods
    static bool itemMatchesCriteria(const Item* item, const SearchCriteria& criteria);
    static bool tileContainsItem(Tile* tile, const SearchCriteria& criteria);
    
    // Search criteria builders
    static SearchCriteria createServerIdCriteria(quint16 serverId);
    static SearchCriteria createServerIdsCriteria(const QSet<quint16>& serverIds);
    static SearchCriteria createItemTypeCriteria(ItemTypes_t itemType);
    static SearchCriteria createItemGroupCriteria(ItemGroup_t itemGroup);
    static SearchCriteria createAttributeCriteria(const QString& key, const QVariant& value);
    static SearchCriteria createPropertyCriteria(const QString& propertyName, bool required = true);
    
private:
    // Internal helper methods
    static bool matchesServerIds(const Item* item, const QSet<quint16>& serverIds);
    static bool matchesClientIds(const Item* item, const QSet<quint16>& clientIds);
    static bool matchesItemTypes(const Item* item, const QSet<ItemTypes_t>& itemTypes);
    static bool matchesItemGroups(const Item* item, const QSet<ItemGroup_t>& itemGroups);
    static bool matchesTypeNames(const Item* item, const QStringList& typeNames);
    static bool matchesAttributes(const Item* item, const QMap<QString, QVariant>& attributes, bool required);
    static bool matchesProperties(const Item* item, const SearchCriteria& criteria);
    
    // Map iteration helpers
    static void iterateMapTiles(Map* map, const std::function<void(Tile*, const MapPos&)>& callback);
    static void iterateMapTilesInArea(Map* map, const QRect& area, const std::function<void(Tile*, const MapPos&)>& callback);
    static void iterateMapTilesInSelection(Map* map, const std::function<void(Tile*, const MapPos&)>& callback);
    static void iterateMapTilesOnLayer(Map* map, int layer, const std::function<void(Tile*, const MapPos&)>& callback);
};

/**
 * @brief TileFinder - Comprehensive utility for searching tiles within maps
 * 
 * This class provides efficient methods for searching tiles based on various
 * criteria including tile properties, contained items, and custom predicates.
 */
class TileFinder {
public:
    using TilePredicate = std::function<bool(const Tile*)>;
    
    struct TileResult {
        Tile* tile;
        MapPos position;
        
        TileResult(Tile* t, const MapPos& pos) 
            : tile(t), position(pos) {}
    };
    
    struct TileSearchCriteria {
        // Tile property criteria
        bool requiresGround = false;
        bool requiresWall = false;
        bool requiresBlocking = false;
        bool requiresPZ = false;
        bool requiresNoPVP = false;
        bool requiresNoLogout = false;
        bool requiresPVPZone = false;
        bool requiresHouse = false;
        bool requiresSelected = false;
        
        // Item-based criteria
        ItemFinder::SearchCriteria itemCriteria;
        bool requiresAnyItem = false;
        bool requiresSpecificItem = false;
        
        // Zone criteria
        QSet<quint16> zoneIds;
        quint32 houseId = 0;
        
        // Custom predicate
        TilePredicate customPredicate;
        
        // Layer filtering
        QSet<int> layers; // Empty means all layers
        
        TileSearchCriteria() = default;
    };
    
    // Static search methods for tiles
    static QList<TileResult> findTilesInMap(Map* map, const TileSearchCriteria& criteria);
    static QList<TileResult> findTilesWithItem(Map* map, quint16 serverId);
    static QList<TileResult> findTilesWithItems(Map* map, const QSet<quint16>& serverIds);
    static QList<TileResult> findTilesWithItemType(Map* map, ItemTypes_t itemType);
    static QList<TileResult> findTilesWithProperty(Map* map, const QString& propertyKey, const QVariant& value);
    static QList<TileResult> findTilesWithPredicate(Map* map, const TilePredicate& predicate);
    
    // Specialized tile searches
    static QList<TileResult> findGroundTiles(Map* map);
    static QList<TileResult> findWallTiles(Map* map);
    static QList<TileResult> findBlockingTiles(Map* map);
    static QList<TileResult> findPZTiles(Map* map);
    static QList<TileResult> findHouseTiles(Map* map, quint32 houseId = 0);
    static QList<TileResult> findSelectedTiles(Map* map);
    static QList<TileResult> findEmptyTiles(Map* map);
    
    // Area-based searches
    static QList<TileResult> findTilesInArea(Map* map, const QRect& area, const TileSearchCriteria& criteria);
    static QList<TileResult> findTilesInSelection(Map* map, const TileSearchCriteria& criteria);
    static QList<TileResult> findTilesOnLayer(Map* map, int layer, const TileSearchCriteria& criteria);
    
    // Count methods
    static int countTilesInMap(Map* map, const TileSearchCriteria& criteria);
    static int countTilesWithItem(Map* map, quint16 serverId);
    
    // Utility methods
    static bool tileMatchesCriteria(const Tile* tile, const TileSearchCriteria& criteria);
    
    // Search criteria builders
    static TileSearchCriteria createPropertyCriteria(const QString& propertyName, bool required = true);
    static TileSearchCriteria createItemCriteria(const ItemFinder::SearchCriteria& itemCriteria);
    static TileSearchCriteria createHouseCriteria(quint32 houseId);
    static TileSearchCriteria createZoneCriteria(quint16 zoneId);
    
private:
    // Internal helper methods
    static bool matchesTileProperties(const Tile* tile, const TileSearchCriteria& criteria);
    static bool matchesItemCriteria(const Tile* tile, const ItemFinder::SearchCriteria& itemCriteria);
    static bool matchesZoneCriteria(const Tile* tile, const QSet<quint16>& zoneIds);
};

#endif // ITEMFINDER_H
