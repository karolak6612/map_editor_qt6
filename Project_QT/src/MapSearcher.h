#ifndef MAPSEARCHER_H
#define MAPSEARCHER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QList>
#include <QRect>
#include <QPoint>
#include <QSet>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QProgressDialog>
#include <functional>

// Forward declarations
class Map;
class Tile;
class Item;
class ItemType;
class ItemManager;
class MapView;

/**
 * @brief Comprehensive map search functionality for Task 82
 * 
 * Complete implementation of search functionality including:
 * - Enhanced backend search logic with ItemTypes integration
 * - "Find All" functionality with results display
 * - Replace functionality with QUndoCommand support
 * - Advanced search criteria and filtering
 * - Progress tracking and cancellation support
 * - Integration with FindItemDialog and MapView
 */

/**
 * @brief Enhanced search criteria structure
 */
struct SearchCriteria {
    // Basic search parameters
    enum SearchMode {
        SERVER_IDS,
        CLIENT_IDS,
        NAMES,
        TYPES,
        PROPERTIES
    };
    
    SearchMode mode = SERVER_IDS;
    
    // ID-based search
    quint16 serverId = 0;
    quint16 clientId = 0;
    QList<quint16> serverIdList;
    QList<quint16> clientIdList;
    QPair<quint16, quint16> serverIdRange = {0, 0};
    QPair<quint16, quint16> clientIdRange = {0, 0};
    
    // Name-based search
    QString itemName;
    bool exactNameMatch = false;
    bool caseSensitive = false;
    bool wholeWordOnly = false;
    
    // Type-based search
    QString itemTypeName;
    QStringList itemTypeCategories;
    
    // Property-based search
    QVariantMap requiredProperties;
    QVariantMap excludedProperties;
    
    // Advanced filtering
    QStringList layers; // Floor layers to search
    QRect searchArea; // Specific area to search
    bool searchInSelection = false;
    bool includeGround = true;
    bool includeItems = true;
    bool includeCreatures = false;
    bool includeSpawns = false;
    
    // Ignored items
    QSet<quint16> ignoredIds;
    QList<QPair<quint16, quint16>> ignoredRanges;
    
    // Result limits
    int maxResults = 1000;
    bool stopOnFirstMatch = false;
    
    SearchCriteria() = default;
};

/**
 * @brief Search result structure
 */
struct SearchResult {
    Item* item = nullptr;
    Tile* tile = nullptr;
    QPoint position;
    int floor = 0;
    QString description;
    quint16 itemId = 0;
    QString itemName;
    QVariantMap itemProperties;
    
    SearchResult() = default;
    SearchResult(Item* item, Tile* tile, const QPoint& pos, int floor);
    
    bool isValid() const { return item != nullptr && tile != nullptr; }
    QString getDisplayText() const;
    QString getDetailedInfo() const;
};

/**
 * @brief Replace operation parameters
 */
struct ReplaceOperation {
    enum ReplaceMode {
        REPLACE_ITEM_ID,        // Replace with different item ID
        REPLACE_PROPERTIES,     // Replace specific properties
        DELETE_ITEMS,           // Delete matching items
        SWAP_ITEMS             // Swap two item types
    };
    
    ReplaceMode mode = REPLACE_ITEM_ID;
    
    // Source criteria (what to replace)
    SearchCriteria sourceCriteria;
    
    // Target parameters (what to replace with)
    quint16 targetItemId = 0;
    QVariantMap targetProperties;
    
    // Replace scope
    enum ReplaceScope {
        REPLACE_ALL_MATCHING,   // Replace all items matching criteria
        REPLACE_SELECTED_RESULTS, // Replace only selected search results
        REPLACE_IN_SELECTION    // Replace only in current map selection
    };
    
    ReplaceScope scope = REPLACE_ALL_MATCHING;
    QList<SearchResult> selectedResults;
    
    // Options
    bool createBackup = true;
    bool updateBorders = true;
    bool preserveAttributes = false;
    
    ReplaceOperation() = default;
};

/**
 * @brief Main map searcher class
 */
class MapSearcher : public QObject
{
    Q_OBJECT

public:
    explicit MapSearcher(QObject* parent = nullptr);
    ~MapSearcher() override;

    // Main search operations
    QList<SearchResult> findAllItems(Map* map, const SearchCriteria& criteria);
    QList<SearchResult> findItemsInArea(Map* map, const QRect& area, const SearchCriteria& criteria);
    QList<SearchResult> findItemsInSelection(Map* map, const SearchCriteria& criteria);
    SearchResult findNextItem(Map* map, const SearchCriteria& criteria, const QPoint& startPos = QPoint());
    
    // Specialized search methods (1:1 wxwidgets compatibility)
    QList<SearchResult> findItemsByServerId(Map* map, quint16 serverId, int maxResults = 1000);
    QList<SearchResult> findItemsByClientId(Map* map, quint16 clientId, int maxResults = 1000);
    QList<SearchResult> findItemsByName(Map* map, const QString& name, bool exactMatch = false, int maxResults = 1000);
    QList<SearchResult> findItemsByType(Map* map, const QString& typeName, int maxResults = 1000);
    QList<SearchResult> findItemsByProperties(Map* map, const QVariantMap& properties, int maxResults = 1000);
    
    // Replace operations
    int replaceItems(Map* map, const ReplaceOperation& operation);
    int replaceSelectedItems(Map* map, const QList<SearchResult>& results, quint16 targetItemId);
    int deleteItems(Map* map, const QList<SearchResult>& results);
    int swapItems(Map* map, quint16 itemId1, quint16 itemId2);
    
    // Search state management
    void cancelSearch();
    bool isSearchInProgress() const { return searchInProgress_; }
    int getSearchProgress() const { return searchProgress_; }
    QString getCurrentSearchStatus() const { return currentSearchStatus_; }
    
    // Integration with managers
    void setItemManager(ItemManager* itemManager) { itemManager_ = itemManager; }
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    
    // Search history and continuation
    void storeLastSearch(const SearchCriteria& criteria, const QList<SearchResult>& results);
    bool hasLastSearch() const { return !lastSearchCriteria_.itemName.isEmpty() || lastSearchCriteria_.serverId > 0; }
    QList<SearchResult> continueLastSearch(Map* map, int additionalResults = 1000);
    
    // Ignored items management
    void setIgnoredIds(const QSet<quint16>& ignoredIds) { ignoredIds_ = ignoredIds; }
    void setIgnoredRanges(const QList<QPair<quint16, quint16>>& ignoredRanges) { ignoredRanges_ = ignoredRanges; }
    QSet<quint16> getIgnoredIds() const { return ignoredIds_; }
    QList<QPair<quint16, quint16>> getIgnoredRanges() const { return ignoredRanges_; }
    
    // Statistics and analysis
    QVariantMap getSearchStatistics() const;
    QStringList getFoundItemTypes() const;
    QMap<quint16, int> getItemIdCounts() const;

signals:
    void searchStarted(const QString& description);
    void searchProgress(int current, int total, const QString& currentItem);
    void searchCompleted(int resultCount, const QString& summary);
    void searchCancelled();
    void searchError(const QString& error, const QString& details);
    
    void replaceStarted(const QString& description);
    void replaceProgress(int current, int total, const QString& currentItem);
    void replaceCompleted(int replacedCount, const QString& summary);
    void replaceError(const QString& error, const QString& details);
    
    void resultFound(const SearchResult& result);
    void itemReplaced(const SearchResult& oldResult, const SearchResult& newResult);

private slots:
    void onSearchTimeout();

private:
    // Core search implementation
    QList<SearchResult> performSearch(Map* map, const SearchCriteria& criteria, const QRect& searchArea = QRect());
    bool matchesSearchCriteria(Item* item, Tile* tile, const SearchCriteria& criteria) const;
    bool matchesIdCriteria(quint16 itemId, const SearchCriteria& criteria) const;
    bool matchesNameCriteria(const QString& itemName, const SearchCriteria& criteria) const;
    bool matchesTypeCriteria(Item* item, const SearchCriteria& criteria) const;
    bool matchesPropertyCriteria(Item* item, const SearchCriteria& criteria) const;
    
    // ItemTypes integration
    bool hasItemProperty(Item* item, const QString& propertyName, const QVariant& expectedValue) const;
    QVariantMap getItemProperties(Item* item) const;
    QString getItemTypeName(Item* item) const;
    QStringList getItemTypeCategories(Item* item) const;
    
    // Replace implementation
    bool replaceItemOnTile(Tile* tile, Item* oldItem, quint16 newItemId);
    bool deleteItemFromTile(Tile* tile, Item* item);
    void updateTileBorders(Tile* tile);
    
    // Search optimization
    void optimizeSearchCriteria(SearchCriteria& criteria) const;
    bool shouldSkipTile(Tile* tile, const SearchCriteria& criteria) const;
    bool isItemIgnored(quint16 itemId) const;
    
    // Progress tracking
    void updateSearchProgress(int current, int total, const QString& currentItem);
    void resetSearchState();
    
    // Error handling
    void setLastError(const QString& error, const QString& details = QString());
    void clearLastError();

private:
    ItemManager* itemManager_;
    MapView* mapView_;
    
    // Search state
    bool searchInProgress_;
    bool searchCancelled_;
    int searchProgress_;
    QString currentSearchStatus_;
    QTimer* searchTimeoutTimer_;
    
    // Last search for continuation
    SearchCriteria lastSearchCriteria_;
    QList<SearchResult> lastSearchResults_;
    QPoint lastSearchPosition_;
    
    // Ignored items
    QSet<quint16> ignoredIds_;
    QList<QPair<quint16, quint16>> ignoredRanges_;
    
    // Statistics
    mutable QVariantMap searchStatistics_;
    mutable QStringList foundItemTypes_;
    mutable QMap<quint16, int> itemIdCounts_;
    
    // Error handling
    QString lastError_;
    QString lastErrorDetails_;
    
    // Thread safety
    mutable QMutex searchMutex_;
    
    // Constants
    static const int DEFAULT_SEARCH_TIMEOUT = 30000; // 30 seconds
    static const int PROGRESS_UPDATE_INTERVAL = 100; // Update progress every 100 items
};

#endif // MAPSEARCHER_H
