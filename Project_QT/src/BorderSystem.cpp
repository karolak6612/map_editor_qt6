#include "BorderSystem.h"
#include "SettingsManager.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "AutoBorder.h"
#include "ItemManager.h"
#include <QDebug>
#include <QRect>
#include <QPoint>
#include <QList>

BorderSystem* BorderSystem::instance_ = nullptr;

BorderSystem* BorderSystem::getInstance() {
    if (!instance_) {
        instance_ = new BorderSystem();
    }
    return instance_;
}

void BorderSystem::destroyInstance() {
    delete instance_;
    instance_ = nullptr;
}

BorderSystem::BorderSystem(QObject* parent)
    : QObject(parent), settingsManager_(SettingsManager::getInstance()) {
    
    // Connect to settings changes
    connect(settingsManager_, &SettingsManager::automagicSettingsChanged,
            this, &BorderSystem::onAutomagicSettingsChanged);
    
    initializeFromSettings();
    
    qDebug() << "BorderSystem: Initialized with automagic enabled:" << automagicEnabled_;
}

BorderSystem::~BorderSystem() {
    // Cleanup
}

void BorderSystem::initializeFromSettings() {
    automagicEnabled_ = settingsManager_->isAutomagicEnabled();
    sameGroundTypeBorder_ = settingsManager_->isSameGroundTypeBorderEnabled();
    wallsRepelBorders_ = settingsManager_->isWallsRepelBordersEnabled();
    layerCarpets_ = settingsManager_->isLayerCarpetsEnabled();
    borderizeDelete_ = settingsManager_->isBorderizeDeleteEnabled();
    customBorderEnabled_ = settingsManager_->isCustomBorderEnabled();
    customBorderId_ = settingsManager_->getCustomBorderId();
}

// Task 52: Enhanced border functionality with Qt rendering integration
void BorderSystem::applyAutomagicBorders(Map* map, Tile* tile) {
    if (!automagicEnabled_ || !map || !tile) {
        return;
    }

    logBorderAction("Applying automagic borders", map, tile);

    emit borderingStarted();

    QList<QPoint3D> affectedTiles;

    // Get tile position
    int x = tile->x();
    int y = tile->y();
    int z = tile->z();

    // Analyze surrounding tiles for border placement
    QList<Tile*> neighbors = getNeighborTiles(map, tile);

    // Process border logic for this tile and its neighbors
    for (Tile* neighborTile : neighbors) {
        if (!neighborTile) continue;

        // Determine if borders are needed between this tile and neighbor
        if (analyzeGroundCompatibility(tile, neighborTile)) {
            // Calculate border type needed
            BorderType borderType = determineBorderType(map, tile, neighborTile->x(), neighborTile->y());

            if (borderType != BorderType::BORDER_NONE) {
                // Apply border item to tile
                applyBorderItem(map, tile, borderType);
                affectedTiles.append(QPoint3D(x, y, z));
            }
        }
    }

    // Apply custom borders if enabled
    if (customBorderEnabled_) {
        applyCustomBorders(map, tile, customBorderId_);
        affectedTiles.append(QPoint3D(x, y, z));
    }

    // Emit signals for Qt rendering system integration
    if (!affectedTiles.isEmpty()) {
        emit borderItemsChanged(map, affectedTiles);
        emit tilesNeedVisualUpdate(affectedTiles);
    }

    emit borderingFinished();
}

void BorderSystem::applyCustomBorders(Map* map, Tile* tile, int borderId) {
    if (!map || !tile || borderId <= 0) {
        return;
    }
    
    logBorderAction(QString("Applying custom border ID %1").arg(borderId), map, tile);
    
    // TODO: Implement custom border logic
    // This would include:
    // - Looking up the custom border definition
    // - Applying the specific border pattern
    // - Handling border placement rules
    
    qDebug() << "BorderSystem: Custom border logic not yet implemented for ID" << borderId;
}

void BorderSystem::removeBorders(Map* map, Tile* tile) {
    if (!map || !tile) {
        return;
    }
    
    logBorderAction("Removing borders", map, tile);
    
    // TODO: Implement border removal logic
    // This would include:
    // - Identifying border items on the tile
    // - Removing them appropriately
    // - Handling cleanup
    
    qDebug() << "BorderSystem: Border removal logic not yet implemented";
}

void BorderSystem::reborderizeTile(Map* map, Tile* tile) {
    if (!map || !tile) {
        return;
    }
    
    logBorderAction("Reborderizing tile", map, tile);
    
    // First remove existing borders
    if (borderizeDelete_) {
        removeBorders(map, tile);
    }
    
    // Then apply new borders
    applyAutomagicBorders(map, tile);
}

// Border validation and checking
bool BorderSystem::shouldApplyBorders(Map* map, Tile* tile) const {
    if (!automagicEnabled_ || !map || !tile) {
        return false;
    }
    
    // TODO: Implement validation logic
    // This would check:
    // - If the tile needs borders
    // - If surrounding tiles are compatible
    // - If walls repel borders (if enabled)
    
    return true; // Placeholder
}

bool BorderSystem::isValidBorderPosition(Map* map, int x, int y, int z) const {
    if (!map) {
        return false;
    }

    // Check map boundaries
    if (x < 0 || y < 0 || z < 0 ||
        x >= map->getWidth() || y >= map->getHeight() || z >= map->getFloors()) {
        return false;
    }

    // Check if tile exists
    Tile* tile = map->getTile(x, y, z);
    if (!tile) {
        return false;
    }

    // Additional border placement rules
    if (wallsRepelBorders_ && tile->hasWall()) {
        return false;
    }

    return true;
}

// Task 52: Area-based border processing for efficiency
void BorderSystem::processBorderArea(Map* map, const QRect& area) {
    if (!map || !automagicEnabled_) {
        return;
    }

    emit borderingStarted();

    QList<QPoint3D> affectedTiles;
    int totalTiles = area.width() * area.height();
    int processedTiles = 0;

    // Process all tiles in the area
    for (int x = area.left(); x <= area.right(); ++x) {
        for (int y = area.top(); y <= area.bottom(); ++y) {
            for (int z = 0; z < map->getFloors(); ++z) {
                Tile* tile = map->getTile(x, y, z);
                if (tile && shouldApplyBorders(map, tile)) {
                    applyAutomagicBorders(map, tile);
                    affectedTiles.append(QPoint3D(x, y, z));
                }

                processedTiles++;
                if (totalTiles > 0) {
                    int percentage = (processedTiles * 100) / totalTiles;
                    emit borderingProgress(percentage);
                }
            }
        }
    }

    // Emit completion signals
    if (!affectedTiles.isEmpty()) {
        emit borderItemsChanged(map, affectedTiles);
        emit tilesNeedVisualUpdate(affectedTiles);
    }

    emit borderProcessingComplete(map, area);
    emit borderingFinished();
}

void BorderSystem::processBorderArea(Map* map, const QList<QPoint>& tilePositions) {
    if (!map || !automagicEnabled_ || tilePositions.isEmpty()) {
        return;
    }

    emit borderingStarted();

    QList<QPoint3D> affectedTiles;
    int totalTiles = tilePositions.size();
    int processedTiles = 0;

    // Process specified tile positions
    for (const QPoint& pos : tilePositions) {
        for (int z = 0; z < map->getFloors(); ++z) {
            Tile* tile = map->getTile(pos.x(), pos.y(), z);
            if (tile && shouldApplyBorders(map, tile)) {
                applyAutomagicBorders(map, tile);
                affectedTiles.append(QPoint3D(pos.x(), pos.y(), z));
            }
        }

        processedTiles++;
        if (totalTiles > 0) {
            int percentage = (processedTiles * 100) / totalTiles;
            emit borderingProgress(percentage);
        }
    }

    // Emit completion signals
    if (!affectedTiles.isEmpty()) {
        emit borderItemsChanged(map, affectedTiles);
        emit tilesNeedVisualUpdate(affectedTiles);
    }

    emit borderingFinished();
}

void BorderSystem::processBorderUpdates(Map* map, const QList<QPoint3D>& affectedTiles) {
    if (!map || !automagicEnabled_ || affectedTiles.isEmpty()) {
        return;
    }

    emit borderingStarted();

    QList<QPoint3D> updatedTiles;

    // Process each affected tile and its neighbors
    for (const QPoint3D& tilePos : affectedTiles) {
        Tile* tile = map->getTile(tilePos.x, tilePos.y, tilePos.z);
        if (tile) {
            // Process the tile itself
            if (shouldApplyBorders(map, tile)) {
                applyAutomagicBorders(map, tile);
                updatedTiles.append(tilePos);
            }

            // Process neighboring tiles that might need border updates
            QList<Tile*> neighbors = getNeighborTiles(map, tile);
            for (Tile* neighbor : neighbors) {
                if (neighbor && shouldApplyBorders(map, neighbor)) {
                    reborderizeTile(map, neighbor);
                    updatedTiles.append(QPoint3D(neighbor->x(), neighbor->y(), neighbor->z()));
                }
            }
        }
    }

    // Emit visual update signals
    if (!updatedTiles.isEmpty()) {
        emit borderItemsChanged(map, updatedTiles);
        emit tilesNeedVisualUpdate(updatedTiles);
    }

    emit borderingFinished();
}

// Task 52: Neighbor analysis for border determination
QList<Tile*> BorderSystem::getNeighborTiles(Map* map, Tile* centerTile) const {
    if (!map || !centerTile) {
        return QList<Tile*>();
    }

    return getNeighborTiles(map, centerTile->x(), centerTile->y(), centerTile->z());
}

QList<Tile*> BorderSystem::getNeighborTiles(Map* map, int x, int y, int z) const {
    QList<Tile*> neighbors;

    if (!map) {
        return neighbors;
    }

    // Get orthogonal neighbors (north, south, east, west)
    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (isValidBorderPosition(map, nx, ny, z)) {
            Tile* neighbor = map->getTile(nx, ny, z);
            neighbors.append(neighbor);
        } else {
            neighbors.append(nullptr); // Maintain position correspondence
        }
    }

    // Optionally include diagonal neighbors for more complex border logic
    if (sameGroundTypeBorder_) {
        const int ddx[] = {-1, -1, 1, 1};
        const int ddy[] = {-1, 1, -1, 1};

        for (int i = 0; i < 4; ++i) {
            int nx = x + ddx[i];
            int ny = y + ddy[i];

            if (isValidBorderPosition(map, nx, ny, z)) {
                Tile* neighbor = map->getTile(nx, ny, z);
                neighbors.append(neighbor);
            } else {
                neighbors.append(nullptr);
            }
        }
    }

    return neighbors;
}

bool BorderSystem::analyzeGroundCompatibility(Tile* tile1, Tile* tile2) const {
    if (!tile1 || !tile2) {
        return false;
    }

    // Get ground items from both tiles
    Item* ground1 = tile1->getGroundItem();
    Item* ground2 = tile2->getGroundItem();

    // If either tile has no ground, no border needed
    if (!ground1 || !ground2) {
        return false;
    }

    // Check if ground types are different (requiring borders)
    if (ground1->getServerId() != ground2->getServerId()) {
        return true;
    }

    // If same ground type border is enabled, check for subtle differences
    if (sameGroundTypeBorder_) {
        // Check for different ground variations or attributes
        if (ground1->getCount() != ground2->getCount() ||
            ground1->getActionId() != ground2->getActionId()) {
            return true;
        }
    }

    return false;
}

BorderType BorderSystem::determineBorderType(Map* map, Tile* tile, int neighborX, int neighborY) const {
    if (!map || !tile) {
        return BorderType::BORDER_NONE;
    }

    int tileX = tile->x();
    int tileY = tile->y();

    // Determine relative position of neighbor
    int deltaX = neighborX - tileX;
    int deltaY = neighborY - tileY;

    // Map relative positions to border types
    if (deltaX == 0 && deltaY == -1) {
        return BorderType::NORTH_HORIZONTAL;
    } else if (deltaX == 1 && deltaY == 0) {
        return BorderType::EAST_HORIZONTAL;
    } else if (deltaX == 0 && deltaY == 1) {
        return BorderType::SOUTH_HORIZONTAL;
    } else if (deltaX == -1 && deltaY == 0) {
        return BorderType::WEST_HORIZONTAL;
    } else if (deltaX == -1 && deltaY == -1) {
        return BorderType::NORTHWEST_CORNER;
    } else if (deltaX == 1 && deltaY == -1) {
        return BorderType::NORTHEAST_CORNER;
    } else if (deltaX == -1 && deltaY == 1) {
        return BorderType::SOUTHWEST_CORNER;
    } else if (deltaX == 1 && deltaY == 1) {
        return BorderType::SOUTHEAST_CORNER;
    }

    return BorderType::BORDER_NONE;
}

// Helper method to apply border item to tile
void BorderSystem::applyBorderItem(Map* map, Tile* tile, BorderType borderType) {
    if (!map || !tile || borderType == BorderType::BORDER_NONE) {
        return;
    }

    // TODO: Get appropriate border item ID from AutoBorder system
    // For now, use placeholder logic
    ItemManager* itemManager = ItemManager::getInstancePtr();
    if (!itemManager) {
        return;
    }

    // Placeholder border item IDs (these would come from AutoBorder configuration)
    quint16 borderItemId = 0;

    switch (borderType) {
        case BorderType::NORTH_HORIZONTAL:
            borderItemId = 1000; // Placeholder
            break;
        case BorderType::EAST_HORIZONTAL:
            borderItemId = 1001; // Placeholder
            break;
        case BorderType::SOUTH_HORIZONTAL:
            borderItemId = 1002; // Placeholder
            break;
        case BorderType::WEST_HORIZONTAL:
            borderItemId = 1003; // Placeholder
            break;
        case BorderType::NORTHWEST_CORNER:
            borderItemId = 1004; // Placeholder
            break;
        case BorderType::NORTHEAST_CORNER:
            borderItemId = 1005; // Placeholder
            break;
        case BorderType::SOUTHWEST_CORNER:
            borderItemId = 1006; // Placeholder
            break;
        case BorderType::SOUTHEAST_CORNER:
            borderItemId = 1007; // Placeholder
            break;
        default:
            return;
    }

    if (borderItemId > 0) {
        Item* borderItem = itemManager->createItem(borderItemId);
        if (borderItem) {
            tile->addItem(borderItem);
            qDebug() << "BorderSystem: Applied border item" << borderItemId << "to tile at"
                     << tile->x() << "," << tile->y() << "," << tile->z();
        }
    }
}

// Settings integration
void BorderSystem::updateFromSettings() {
    initializeFromSettings();
    qDebug() << "BorderSystem: Updated settings - automagic enabled:" << automagicEnabled_;
}

bool BorderSystem::isAutomagicEnabled() const {
    return automagicEnabled_;
}

bool BorderSystem::isSameGroundTypeBorderEnabled() const {
    return sameGroundTypeBorder_;
}

bool BorderSystem::isWallsRepelBordersEnabled() const {
    return wallsRepelBorders_;
}

bool BorderSystem::isLayerCarpetsEnabled() const {
    return layerCarpets_;
}

bool BorderSystem::isBorderizeDeleteEnabled() const {
    return borderizeDelete_;
}

bool BorderSystem::isCustomBorderEnabled() const {
    return customBorderEnabled_;
}

int BorderSystem::getCustomBorderId() const {
    return customBorderId_;
}

// Slots
void BorderSystem::onAutomagicSettingsChanged() {
    updateFromSettings();
    qDebug() << "BorderSystem: Automagic settings changed";
}

void BorderSystem::onTileChanged(Map* map, Tile* tile) {
    if (shouldApplyBorders(map, tile)) {
        applyAutomagicBorders(map, tile);
    }
}

void BorderSystem::onSelectionBorderize() {
    qDebug() << "BorderSystem: Selection borderize requested";
    // TODO: Implement selection borderize
    // This would apply borders to all selected tiles
}

// Private methods
void BorderSystem::logBorderAction(const QString& action, Map* map, Tile* tile) {
    if (map && tile) {
        // Assuming we can get position from tile or map
        qDebug() << "BorderSystem:" << action << "for tile at position (placeholder coordinates)";
    } else {
        qDebug() << "BorderSystem:" << action << "(invalid map or tile)";
    }
}

#include "BorderSystem.moc"
