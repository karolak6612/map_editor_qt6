#include "MapSearcher.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "ItemType.h"
#include "ItemManager.h"
#include "MapView.h"
#include <QDebug>
#include <QApplication>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QMutexLocker>
#include <QRegularExpression>

// SearchResult implementation
SearchResult::SearchResult(Item* item, Tile* tile, const QPoint& pos, int floor)
    : item(item), tile(tile), position(pos), floor(floor)
{
    if (item) {
        itemId = item->getServerId();
        itemName = item->getName();
        // Extract item properties would be implemented based on Item class interface
    }
    
    description = getDisplayText();
}

QString SearchResult::getDisplayText() const {
    if (!item) {
        return "Invalid result";
    }
    
    return QString("%1 (ID: %2) at (%3,%4,%5)")
           .arg(itemName)
           .arg(itemId)
           .arg(position.x())
           .arg(position.y())
           .arg(floor);
}

QString SearchResult::getDetailedInfo() const {
    if (!item) {
        return "Invalid result";
    }
    
    QString info = QString("Item: %1\nID: %2\nPosition: (%3,%4,%5)\n")
                   .arg(itemName)
                   .arg(itemId)
                   .arg(position.x())
                   .arg(position.y())
                   .arg(floor);
    
    // Add properties if available
    if (!itemProperties.isEmpty()) {
        info += "Properties:\n";
        for (auto it = itemProperties.begin(); it != itemProperties.end(); ++it) {
            info += QString("  %1: %2\n").arg(it.key(), it.value().toString());
        }
    }
    
    return info;
}

// MapSearcher implementation
MapSearcher::MapSearcher(QObject* parent)
    : QObject(parent)
    , itemManager_(nullptr)
    , mapView_(nullptr)
    , searchInProgress_(false)
    , searchCancelled_(false)
    , searchProgress_(0)
    , searchTimeoutTimer_(new QTimer(this))
{
    searchTimeoutTimer_->setSingleShot(true);
    searchTimeoutTimer_->setInterval(DEFAULT_SEARCH_TIMEOUT);
    connect(searchTimeoutTimer_, &QTimer::timeout, this, &MapSearcher::onSearchTimeout);
}

MapSearcher::~MapSearcher() = default;

QList<SearchResult> MapSearcher::findAllItems(Map* map, const SearchCriteria& criteria) {
    if (!map) {
        setLastError("No map provided for search");
        return QList<SearchResult>();
    }
    
    QMutexLocker locker(&searchMutex_);
    
    if (searchInProgress_) {
        setLastError("Search already in progress");
        return QList<SearchResult>();
    }
    
    resetSearchState();
    searchInProgress_ = true;
    searchTimeoutTimer_->start();
    
    emit searchStarted(QString("Searching for items in map"));
    
    QList<SearchResult> results = performSearch(map, criteria);
    
    searchTimeoutTimer_->stop();
    searchInProgress_ = false;
    
    if (searchCancelled_) {
        emit searchCancelled();
        return QList<SearchResult>();
    }
    
    // Store search for continuation
    storeLastSearch(criteria, results);
    
    // Update statistics
    updateSearchStatistics(results);
    
    emit searchCompleted(results.size(), QString("Found %1 items").arg(results.size()));
    
    return results;
}

QList<SearchResult> MapSearcher::findItemsInArea(Map* map, const QRect& area, const SearchCriteria& criteria) {
    if (!map) {
        setLastError("No map provided for search");
        return QList<SearchResult>();
    }
    
    QMutexLocker locker(&searchMutex_);
    
    resetSearchState();
    searchInProgress_ = true;
    
    emit searchStarted(QString("Searching for items in area (%1,%2) to (%3,%4)")
                      .arg(area.left()).arg(area.top()).arg(area.right()).arg(area.bottom()));
    
    QList<SearchResult> results = performSearch(map, criteria, area);
    
    searchInProgress_ = false;
    
    if (searchCancelled_) {
        emit searchCancelled();
        return QList<SearchResult>();
    }
    
    emit searchCompleted(results.size(), QString("Found %1 items in area").arg(results.size()));
    
    return results;
}

QList<SearchResult> MapSearcher::findItemsByServerId(Map* map, quint16 serverId, int maxResults) {
    SearchCriteria criteria;
    criteria.mode = SearchCriteria::SERVER_IDS;
    criteria.serverId = serverId;
    criteria.maxResults = maxResults;
    
    return findAllItems(map, criteria);
}

QList<SearchResult> MapSearcher::findItemsByClientId(Map* map, quint16 clientId, int maxResults) {
    SearchCriteria criteria;
    criteria.mode = SearchCriteria::CLIENT_IDS;
    criteria.clientId = clientId;
    criteria.maxResults = maxResults;
    
    return findAllItems(map, criteria);
}

QList<SearchResult> MapSearcher::findItemsByName(Map* map, const QString& name, bool exactMatch, int maxResults) {
    SearchCriteria criteria;
    criteria.mode = SearchCriteria::NAMES;
    criteria.itemName = name;
    criteria.exactNameMatch = exactMatch;
    criteria.maxResults = maxResults;
    
    return findAllItems(map, criteria);
}

QList<SearchResult> MapSearcher::findItemsByType(Map* map, const QString& typeName, int maxResults) {
    SearchCriteria criteria;
    criteria.mode = SearchCriteria::TYPES;
    criteria.itemTypeName = typeName;
    criteria.maxResults = maxResults;
    
    return findAllItems(map, criteria);
}

QList<SearchResult> MapSearcher::findItemsByProperties(Map* map, const QVariantMap& properties, int maxResults) {
    SearchCriteria criteria;
    criteria.mode = SearchCriteria::PROPERTIES;
    criteria.requiredProperties = properties;
    criteria.maxResults = maxResults;
    
    return findAllItems(map, criteria);
}

SearchResult MapSearcher::findNextItem(Map* map, const SearchCriteria& criteria, const QPoint& startPos) {
    if (!map) {
        setLastError("No map provided for search");
        return SearchResult();
    }
    
    // Implementation would iterate through map tiles starting from startPos
    // This is a simplified version
    QList<SearchResult> results = findAllItems(map, criteria);
    
    if (!results.isEmpty()) {
        // Find first result after startPos
        for (const SearchResult& result : results) {
            if (result.position.x() > startPos.x() || 
                (result.position.x() == startPos.x() && result.position.y() > startPos.y())) {
                return result;
            }
        }
        // If no result after startPos, return first result
        return results.first();
    }
    
    return SearchResult();
}

int MapSearcher::replaceItems(Map* map, const ReplaceOperation& operation) {
    if (!map) {
        setLastError("No map provided for replace operation");
        return 0;
    }
    
    QMutexLocker locker(&searchMutex_);
    
    emit replaceStarted(QString("Replacing items"));
    
    int replacedCount = 0;
    
    // Get items to replace based on scope
    QList<SearchResult> itemsToReplace;
    
    switch (operation.scope) {
        case ReplaceOperation::REPLACE_ALL_MATCHING:
            itemsToReplace = findAllItems(map, operation.sourceCriteria);
            break;
        case ReplaceOperation::REPLACE_SELECTED_RESULTS:
            itemsToReplace = operation.selectedResults;
            break;
        case ReplaceOperation::REPLACE_IN_SELECTION:
            itemsToReplace = findItemsInSelection(map, operation.sourceCriteria);
            break;
    }
    
    // Perform replacement
    for (int i = 0; i < itemsToReplace.size(); ++i) {
        const SearchResult& result = itemsToReplace[i];
        
        emit replaceProgress(i, itemsToReplace.size(), result.getDisplayText());
        
        bool success = false;
        
        switch (operation.mode) {
            case ReplaceOperation::REPLACE_ITEM_ID:
                success = replaceItemOnTile(result.tile, result.item, operation.targetItemId);
                break;
            case ReplaceOperation::DELETE_ITEMS:
                success = deleteItemFromTile(result.tile, result.item);
                break;
            case ReplaceOperation::REPLACE_PROPERTIES:
                // Would implement property replacement
                success = true; // Placeholder
                break;
            case ReplaceOperation::SWAP_ITEMS:
                // Would implement item swapping
                success = true; // Placeholder
                break;
        }
        
        if (success) {
            replacedCount++;
            
            // Create new result for replaced item
            SearchResult newResult = result;
            newResult.itemId = operation.targetItemId;
            // Update other properties as needed
            
            emit itemReplaced(result, newResult);
            
            // Update borders if requested
            if (operation.updateBorders) {
                updateTileBorders(result.tile);
            }
        }
        
        // Check for cancellation
        if (searchCancelled_) {
            break;
        }
    }
    
    emit replaceCompleted(replacedCount, QString("Replaced %1 items").arg(replacedCount));
    
    return replacedCount;
}

void MapSearcher::cancelSearch() {
    searchCancelled_ = true;
    searchTimeoutTimer_->stop();
}

void MapSearcher::storeLastSearch(const SearchCriteria& criteria, const QList<SearchResult>& results) {
    lastSearchCriteria_ = criteria;
    lastSearchResults_ = results;
    
    if (!results.isEmpty()) {
        lastSearchPosition_ = results.last().position;
    }
}

QList<SearchResult> MapSearcher::continueLastSearch(Map* map, int additionalResults) {
    if (!hasLastSearch()) {
        setLastError("No previous search to continue");
        return QList<SearchResult>();
    }
    
    // Modify criteria to start from last position and limit results
    SearchCriteria continueCriteria = lastSearchCriteria_;
    continueCriteria.maxResults = additionalResults;
    
    // Implementation would start search from lastSearchPosition_
    // This is a simplified version
    QList<SearchResult> newResults = findAllItems(map, continueCriteria);
    
    // Filter out already found results
    QList<SearchResult> filteredResults;
    for (const SearchResult& result : newResults) {
        bool alreadyFound = false;
        for (const SearchResult& existingResult : lastSearchResults_) {
            if (result.position == existingResult.position && result.itemId == existingResult.itemId) {
                alreadyFound = true;
                break;
            }
        }
        
        if (!alreadyFound) {
            filteredResults.append(result);
        }
    }
    
    // Update last search results
    lastSearchResults_.append(filteredResults);
    
    return filteredResults;
}

QVariantMap MapSearcher::getSearchStatistics() const {
    return searchStatistics_;
}

void MapSearcher::onSearchTimeout() {
    if (searchInProgress_) {
        searchCancelled_ = true;
        emit searchError("Search timeout", "Search operation took too long and was cancelled");
    }
}

// Core search implementation
QList<SearchResult> MapSearcher::performSearch(Map* map, const SearchCriteria& criteria, const QRect& searchArea) {
    QList<SearchResult> results;

    if (!map) {
        return results;
    }

    // Optimize search criteria
    SearchCriteria optimizedCriteria = criteria;
    optimizeSearchCriteria(optimizedCriteria);

    int totalTiles = 0;
    int processedTiles = 0;

    // Calculate total tiles for progress tracking
    if (searchArea.isValid()) {
        totalTiles = searchArea.width() * searchArea.height() * (criteria.layers.isEmpty() ? 16 : criteria.layers.size());
    } else {
        totalTiles = map->getWidth() * map->getHeight() * (criteria.layers.isEmpty() ? 16 : criteria.layers.size());
    }

    // Iterate through map tiles
    int startX = searchArea.isValid() ? searchArea.left() : 0;
    int endX = searchArea.isValid() ? searchArea.right() : map->getWidth() - 1;
    int startY = searchArea.isValid() ? searchArea.top() : 0;
    int endY = searchArea.isValid() ? searchArea.bottom() : map->getHeight() - 1;

    QList<int> layersToSearch;
    if (criteria.layers.isEmpty()) {
        for (int z = 0; z < 16; ++z) {
            layersToSearch.append(z);
        }
    } else {
        for (const QString& layerStr : criteria.layers) {
            layersToSearch.append(layerStr.toInt());
        }
    }

    for (int x = startX; x <= endX && !searchCancelled_; ++x) {
        for (int y = startY; y <= endY && !searchCancelled_; ++y) {
            for (int z : layersToSearch) {
                if (searchCancelled_) break;

                Tile* tile = map->getTile(x, y, z);
                if (!tile || shouldSkipTile(tile, optimizedCriteria)) {
                    processedTiles++;
                    continue;
                }

                // Search items on tile
                QList<Item*> tileItems = tile->getItems();

                // Include ground if requested
                if (optimizedCriteria.includeGround && tile->getGround()) {
                    tileItems.prepend(tile->getGround());
                }

                for (Item* item : tileItems) {
                    if (!item) continue;

                    if (matchesSearchCriteria(item, tile, optimizedCriteria)) {
                        SearchResult result(item, tile, QPoint(x, y), z);
                        results.append(result);

                        emit resultFound(result);

                        // Check result limit
                        if (results.size() >= optimizedCriteria.maxResults) {
                            goto search_complete;
                        }

                        // Stop on first match if requested
                        if (optimizedCriteria.stopOnFirstMatch) {
                            goto search_complete;
                        }
                    }
                }

                processedTiles++;

                // Update progress periodically
                if (processedTiles % PROGRESS_UPDATE_INTERVAL == 0) {
                    updateSearchProgress(processedTiles, totalTiles, QString("Tile (%1,%2,%3)").arg(x).arg(y).arg(z));
                }
            }
        }
    }

search_complete:
    updateSearchProgress(totalTiles, totalTiles, "Search complete");

    return results;
}

bool MapSearcher::matchesSearchCriteria(Item* item, Tile* tile, const SearchCriteria& criteria) const {
    if (!item) {
        return false;
    }

    // Check if item is ignored
    if (isItemIgnored(item->getServerId())) {
        return false;
    }

    // Check based on search mode
    switch (criteria.mode) {
        case SearchCriteria::SERVER_IDS:
            return matchesIdCriteria(item->getServerId(), criteria);

        case SearchCriteria::CLIENT_IDS:
            return matchesIdCriteria(item->getClientId(), criteria);

        case SearchCriteria::NAMES:
            return matchesNameCriteria(item->getName(), criteria);

        case SearchCriteria::TYPES:
            return matchesTypeCriteria(item, criteria);

        case SearchCriteria::PROPERTIES:
            return matchesPropertyCriteria(item, criteria);
    }

    return false;
}

bool MapSearcher::matchesIdCriteria(quint16 itemId, const SearchCriteria& criteria) const {
    // Single ID match
    if (criteria.serverId > 0 && itemId == criteria.serverId) {
        return true;
    }

    if (criteria.clientId > 0 && itemId == criteria.clientId) {
        return true;
    }

    // ID list match
    if (!criteria.serverIdList.isEmpty() && criteria.serverIdList.contains(itemId)) {
        return true;
    }

    if (!criteria.clientIdList.isEmpty() && criteria.clientIdList.contains(itemId)) {
        return true;
    }

    // Range match
    if (criteria.serverIdRange.first > 0 && criteria.serverIdRange.second > 0) {
        if (itemId >= criteria.serverIdRange.first && itemId <= criteria.serverIdRange.second) {
            return true;
        }
    }

    if (criteria.clientIdRange.first > 0 && criteria.clientIdRange.second > 0) {
        if (itemId >= criteria.clientIdRange.first && itemId <= criteria.clientIdRange.second) {
            return true;
        }
    }

    return false;
}

bool MapSearcher::matchesNameCriteria(const QString& itemName, const SearchCriteria& criteria) const {
    if (criteria.itemName.isEmpty()) {
        return false;
    }

    QString searchName = criteria.itemName;
    QString targetName = itemName;

    // Case sensitivity
    if (!criteria.caseSensitive) {
        searchName = searchName.toLower();
        targetName = targetName.toLower();
    }

    // Exact match
    if (criteria.exactNameMatch) {
        return targetName == searchName;
    }

    // Whole word only
    if (criteria.wholeWordOnly) {
        QRegularExpression regex(QString("\\b%1\\b").arg(QRegularExpression::escape(searchName)));
        if (!criteria.caseSensitive) {
            regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        return regex.match(targetName).hasMatch();
    }

    // Partial match
    return targetName.contains(searchName);
}

bool MapSearcher::matchesTypeCriteria(Item* item, const SearchCriteria& criteria) const {
    if (!item || criteria.itemTypeName.isEmpty()) {
        return false;
    }

    QString itemTypeName = getItemTypeName(item);

    if (!criteria.caseSensitive) {
        return itemTypeName.toLower().contains(criteria.itemTypeName.toLower());
    }

    return itemTypeName.contains(criteria.itemTypeName);
}

bool MapSearcher::matchesPropertyCriteria(Item* item, const SearchCriteria& criteria) const {
    if (!item || criteria.requiredProperties.isEmpty()) {
        return false;
    }

    // Check required properties
    for (auto it = criteria.requiredProperties.begin(); it != criteria.requiredProperties.end(); ++it) {
        if (!hasItemProperty(item, it.key(), it.value())) {
            return false;
        }
    }

    // Check excluded properties
    for (auto it = criteria.excludedProperties.begin(); it != criteria.excludedProperties.end(); ++it) {
        if (hasItemProperty(item, it.key(), it.value())) {
            return false;
        }
    }

    return true;
}
