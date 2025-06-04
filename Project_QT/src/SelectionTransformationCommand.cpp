#include "SelectionTransformationCommand.h"
#include "ItemManager.h"
#include "BrushManager.h"
#include "AutoBorderManager.h"
#include <QDebug>
#include <QtMath>
#include <algorithm>

// Initialize static members
QMap<quint16, quint16> ItemTransformationHelper::clockwiseRotationMap_;
QMap<quint16, quint16> ItemTransformationHelper::counterClockwiseRotationMap_;
QMap<quint16, quint16> ItemTransformationHelper::horizontalFlipMap_;
QMap<quint16, quint16> ItemTransformationHelper::verticalFlipMap_;
bool ItemTransformationHelper::mapsInitialized_ = false;

// Base SelectionTransformationCommand implementation
SelectionTransformationCommand::SelectionTransformationCommand(Map* map, Selection* selection, 
                                                             const QString& description,
                                                             QUndoCommand* parent)
    : QUndoCommand(description, parent)
    , map_(map)
    , selection_(selection)
    , executed_(false)
    , automagicEnabled_(true) {
    
    if (!map_ || !selection_) {
        qWarning() << "SelectionTransformationCommand: Invalid map or selection";
        return;
    }
    
    // Store original selection
    for (const MapPos& pos : *selection_) {
        originalSelection_.insert(pos);
    }
    
    // Initialize transformation helper
    ItemTransformationHelper::initializeTransformationMaps();
}

SelectionTransformationCommand::~SelectionTransformationCommand() {
    // Clean up stored tiles
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    for (auto it = transformedTiles_.begin(); it != transformedTiles_.end(); ++it) {
        delete it.value();
    }
}

void SelectionTransformationCommand::undo() {
    if (!executed_) {
        return;
    }
    
    reverseTransformation();
    restoreTileData();
    
    // Restore original selection
    selection_->clear();
    for (const MapPos& pos : originalSelection_) {
        selection_->addTile(pos);
    }
    
    if (automagicEnabled_) {
        triggerAutomagicBordering();
    }
    
    executed_ = false;
}

void SelectionTransformationCommand::redo() {
    if (executed_) {
        return;
    }
    
    storeTileData();
    executeTransformation();
    applyTileChanges();
    updateSelectionAfterTransformation();
    
    if (automagicEnabled_) {
        triggerAutomagicBordering();
    }
    
    executed_ = true;
}

void SelectionTransformationCommand::storeTileData() {
    // Clear existing data
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();
    
    // Store original tiles
    for (const MapPos& pos : originalSelection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            originalTiles_[pos] = tile->deepCopy();
        }
    }
}

void SelectionTransformationCommand::restoreTileData() {
    // Restore original tiles
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        const MapPos& pos = it.key();
        Tile* originalTile = it.value();
        
        if (originalTile) {
            Tile* restoredTile = originalTile->deepCopy();
            map_->setTile(pos.x, pos.y, pos.z, restoredTile);
        } else {
            map_->removeTile(pos.x, pos.y, pos.z);
        }
    }
}

void SelectionTransformationCommand::applyTileChanges() {
    // Apply transformed tiles to map
    for (auto it = transformedTiles_.begin(); it != transformedTiles_.end(); ++it) {
        const MapPos& pos = it.key();
        Tile* transformedTile = it.value();
        
        if (transformedTile) {
            Tile* newTile = transformedTile->deepCopy();
            map_->setTile(pos.x, pos.y, pos.z, newTile);
        }
    }
}

void SelectionTransformationCommand::updateSelectionAfterTransformation() {
    // Update selection to reflect transformed positions
    selection_->clear();
    for (const MapPos& pos : transformedSelection_) {
        selection_->addTile(pos);
    }
}

void SelectionTransformationCommand::triggerAutomagicBordering() {
    // Trigger automagic bordering for affected areas
    QList<MapPos> affectedPositions = getAffectedPositions();
    
    // This would integrate with AutoBorderManager when available
    qDebug() << "SelectionTransformationCommand: Triggering automagic bordering for" 
             << affectedPositions.size() << "positions";
}

QList<MapPos> SelectionTransformationCommand::getAffectedPositions() const {
    QSet<MapPos> allPositions = originalSelection_;
    allPositions.unite(transformedSelection_);
    return allPositions.toList();
}

// MoveSelectionCommand implementation
MoveSelectionCommand::MoveSelectionCommand(Map* map, Selection* selection, const QPoint& offset,
                                         QUndoCommand* parent)
    : SelectionTransformationCommand(map, selection, QString("Move Selection (%1, %2)")
                                   .arg(offset.x()).arg(offset.y()), parent)
    , moveOffset_(offset)
    , reverseOffset_(-offset.x(), -offset.y()) {
}

MoveSelectionCommand::~MoveSelectionCommand() {
}

void MoveSelectionCommand::executeTransformation() {
    performMove(moveOffset_);
}

void MoveSelectionCommand::reverseTransformation() {
    performMove(reverseOffset_);
}

void MoveSelectionCommand::performMove(const QPoint& offset) {
    // Clear transformed data
    for (auto it = transformedTiles_.begin(); it != transformedTiles_.end(); ++it) {
        delete it.value();
    }
    transformedTiles_.clear();
    transformedSelection_.clear();
    
    // Move each tile in the selection
    for (const MapPos& originalPos : originalSelection_) {
        MapPos newPos(originalPos.x + offset.x(), originalPos.y + offset.y(), originalPos.z);
        
        if (isValidMoveDestination(newPos)) {
            Tile* originalTile = map_->getTile(originalPos.x, originalPos.y, originalPos.z);
            if (originalTile) {
                // Create moved tile
                Tile* movedTile = originalTile->deepCopy();
                movedTile->setPosition(newPos);
                
                transformedTiles_[newPos] = movedTile;
                transformedSelection_.insert(newPos);
            }
        }
    }
    
    // Clear original positions (for move operation)
    for (const MapPos& pos : originalSelection_) {
        if (!transformedSelection_.contains(pos)) {
            map_->removeTile(pos.x, pos.y, pos.z);
        }
    }
}

bool MoveSelectionCommand::isValidMoveDestination(const MapPos& pos) const {
    // Basic validation - check map bounds
    if (!map_->isValidPosition(pos.x, pos.y, pos.z)) {
        return false;
    }
    
    // Additional validation could be added here
    return true;
}

// RotateSelectionCommand implementation
RotateSelectionCommand::RotateSelectionCommand(Map* map, Selection* selection, RotationDirection direction,
                                             QUndoCommand* parent)
    : SelectionTransformationCommand(map, selection, QString("Rotate Selection %1")
                                   .arg(direction == Clockwise90 ? "90° CW" : 
                                        direction == CounterClockwise90 ? "90° CCW" : "180°"), parent)
    , rotationDirection_(direction)
    , reverseDirection_(getOppositeDirection(direction)) {
}

RotateSelectionCommand::~RotateSelectionCommand() {
}

void RotateSelectionCommand::executeTransformation() {
    performRotation(rotationDirection_);
}

void RotateSelectionCommand::reverseTransformation() {
    performRotation(reverseDirection_);
}

void RotateSelectionCommand::performRotation(RotationDirection direction) {
    // Clear transformed data
    for (auto it = transformedTiles_.begin(); it != transformedTiles_.end(); ++it) {
        delete it.value();
    }
    transformedTiles_.clear();
    transformedSelection_.clear();
    
    MapPos center = getSelectionCenter();
    
    // Rotate each tile in the selection
    for (const MapPos& originalPos : originalSelection_) {
        MapPos rotatedPos = rotatePosition(originalPos, direction, center);
        
        Tile* originalTile = map_->getTile(originalPos.x, originalPos.y, originalPos.z);
        if (originalTile) {
            // Create rotated tile
            Tile* rotatedTile = originalTile->deepCopy();
            rotatedTile->setPosition(rotatedPos);
            
            // Rotate items within the tile
            rotateItems(rotatedTile, direction);
            
            transformedTiles_[rotatedPos] = rotatedTile;
            transformedSelection_.insert(rotatedPos);
        }
    }
}

MapPos RotateSelectionCommand::rotatePosition(const MapPos& pos, RotationDirection direction, const MapPos& center) const {
    int dx = pos.x - center.x;
    int dy = pos.y - center.y;
    
    int newDx, newDy;
    
    switch (direction) {
        case Clockwise90:
            newDx = -dy;
            newDy = dx;
            break;
        case CounterClockwise90:
            newDx = dy;
            newDy = -dx;
            break;
        case Rotate180:
            newDx = -dx;
            newDy = -dy;
            break;
        default:
            newDx = dx;
            newDy = dy;
            break;
    }
    
    return MapPos(center.x + newDx, center.y + newDy, pos.z);
}

void RotateSelectionCommand::rotateItems(Tile* tile, RotationDirection direction) {
    if (!tile) return;
    
    for (Item* item : tile->getItems()) {
        if (item) {
            rotateItem(item, direction);
        }
    }
}

void RotateSelectionCommand::rotateItem(Item* item, RotationDirection direction) {
    if (!item || !ItemTransformationHelper::canRotateItem(item->getServerId())) {
        return;
    }
    
    quint16 newItemId = getRotatedItemId(item->getServerId(), direction);
    if (newItemId != item->getServerId()) {
        item->setServerId(newItemId);
    }
}

MapPos RotateSelectionCommand::getSelectionCenter() const {
    if (originalSelection_.isEmpty()) {
        return MapPos(0, 0, 0);
    }
    
    int minX = INT_MAX, maxX = INT_MIN;
    int minY = INT_MAX, maxY = INT_MIN;
    int z = originalSelection_.begin()->z;
    
    for (const MapPos& pos : originalSelection_) {
        minX = qMin(minX, pos.x);
        maxX = qMax(maxX, pos.x);
        minY = qMin(minY, pos.y);
        maxY = qMax(maxY, pos.y);
    }
    
    return MapPos((minX + maxX) / 2, (minY + maxY) / 2, z);
}

quint16 RotateSelectionCommand::getRotatedItemId(quint16 itemId, RotationDirection direction) const {
    switch (direction) {
        case Clockwise90:
            return ItemTransformationHelper::rotateItemClockwise(itemId);
        case CounterClockwise90:
            return ItemTransformationHelper::rotateItemCounterClockwise(itemId);
        case Rotate180:
            return ItemTransformationHelper::rotateItem180(itemId);
        default:
            return itemId;
    }
}

RotateSelectionCommand::RotationDirection RotateSelectionCommand::getOppositeDirection(RotationDirection direction) const {
    switch (direction) {
        case Clockwise90:
            return CounterClockwise90;
        case CounterClockwise90:
            return Clockwise90;
        case Rotate180:
            return Rotate180; // 180° is its own opposite
        default:
            return direction;
    }
}

// FlipSelectionCommand implementation
FlipSelectionCommand::FlipSelectionCommand(Map* map, Selection* selection, FlipDirection direction,
                                         QUndoCommand* parent)
    : SelectionTransformationCommand(map, selection, QString("Flip Selection %1")
                                   .arg(direction == Horizontal ? "Horizontally" : "Vertically"), parent)
    , flipDirection_(direction) {
}

FlipSelectionCommand::~FlipSelectionCommand() {
}

void FlipSelectionCommand::executeTransformation() {
    performFlip(flipDirection_);
}

void FlipSelectionCommand::reverseTransformation() {
    // Flipping is its own reverse operation
    performFlip(flipDirection_);
}

void FlipSelectionCommand::performFlip(FlipDirection direction) {
    // Clear transformed data
    for (auto it = transformedTiles_.begin(); it != transformedTiles_.end(); ++it) {
        delete it.value();
    }
    transformedTiles_.clear();
    transformedSelection_.clear();

    MapPos center = getSelectionCenter();

    // Flip each tile in the selection
    for (const MapPos& originalPos : originalSelection_) {
        MapPos flippedPos = flipPosition(originalPos, direction, center);

        Tile* originalTile = map_->getTile(originalPos.x, originalPos.y, originalPos.z);
        if (originalTile) {
            // Create flipped tile
            Tile* flippedTile = originalTile->deepCopy();
            flippedTile->setPosition(flippedPos);

            // Flip items within the tile
            flipItems(flippedTile, direction);

            transformedTiles_[flippedPos] = flippedTile;
            transformedSelection_.insert(flippedPos);
        }
    }
}

MapPos FlipSelectionCommand::flipPosition(const MapPos& pos, FlipDirection direction, const MapPos& center) const {
    int dx = pos.x - center.x;
    int dy = pos.y - center.y;

    int newDx, newDy;

    switch (direction) {
        case Horizontal:
            newDx = -dx;
            newDy = dy;
            break;
        case Vertical:
            newDx = dx;
            newDy = -dy;
            break;
        default:
            newDx = dx;
            newDy = dy;
            break;
    }

    return MapPos(center.x + newDx, center.y + newDy, pos.z);
}

void FlipSelectionCommand::flipItems(Tile* tile, FlipDirection direction) {
    if (!tile) return;

    for (Item* item : tile->getItems()) {
        if (item) {
            flipItem(item, direction);
        }
    }
}

void FlipSelectionCommand::flipItem(Item* item, FlipDirection direction) {
    if (!item || !ItemTransformationHelper::canFlipItem(item->getServerId())) {
        return;
    }

    quint16 newItemId = getFlippedItemId(item->getServerId(), direction);
    if (newItemId != item->getServerId()) {
        item->setServerId(newItemId);
    }
}

MapPos FlipSelectionCommand::getSelectionCenter() const {
    if (originalSelection_.isEmpty()) {
        return MapPos(0, 0, 0);
    }

    int minX = INT_MAX, maxX = INT_MIN;
    int minY = INT_MAX, maxY = INT_MIN;
    int z = originalSelection_.begin()->z;

    for (const MapPos& pos : originalSelection_) {
        minX = qMin(minX, pos.x);
        maxX = qMax(maxX, pos.x);
        minY = qMin(minY, pos.y);
        maxY = qMax(maxY, pos.y);
    }

    return MapPos((minX + maxX) / 2, (minY + maxY) / 2, z);
}

quint16 FlipSelectionCommand::getFlippedItemId(quint16 itemId, FlipDirection direction) const {
    switch (direction) {
        case Horizontal:
            return ItemTransformationHelper::flipItemHorizontal(itemId);
        case Vertical:
            return ItemTransformationHelper::flipItemVertical(itemId);
        default:
            return itemId;
    }
}

// CompositeTransformationCommand implementation
CompositeTransformationCommand::CompositeTransformationCommand(const QString& description,
                                                             QUndoCommand* parent)
    : QUndoCommand(description, parent) {
}

CompositeTransformationCommand::~CompositeTransformationCommand() {
    qDeleteAll(transformationCommands_);
}

void CompositeTransformationCommand::addMoveCommand(Map* map, Selection* selection, const QPoint& offset) {
    MoveSelectionCommand* moveCommand = new MoveSelectionCommand(map, selection, offset, this);
    transformationCommands_.append(moveCommand);
}

void CompositeTransformationCommand::addRotateCommand(Map* map, Selection* selection,
                                                     RotateSelectionCommand::RotationDirection direction) {
    RotateSelectionCommand* rotateCommand = new RotateSelectionCommand(map, selection, direction, this);
    transformationCommands_.append(rotateCommand);
}

void CompositeTransformationCommand::addFlipCommand(Map* map, Selection* selection,
                                                   FlipSelectionCommand::FlipDirection direction) {
    FlipSelectionCommand* flipCommand = new FlipSelectionCommand(map, selection, direction, this);
    transformationCommands_.append(flipCommand);
}

void CompositeTransformationCommand::executeTransformations() {
    // All child commands will be executed automatically by QUndoCommand
    // This method is for any additional setup if needed
}

// ItemTransformationHelper implementation
bool ItemTransformationHelper::canRotateItem(quint16 itemId) {
    initializeTransformationMaps();

    // Check if item has rotation mappings
    return clockwiseRotationMap_.contains(itemId) ||
           counterClockwiseRotationMap_.contains(itemId);
}

bool ItemTransformationHelper::canFlipItem(quint16 itemId) {
    initializeTransformationMaps();

    // Check if item has flip mappings
    return horizontalFlipMap_.contains(itemId) ||
           verticalFlipMap_.contains(itemId);
}

quint16 ItemTransformationHelper::rotateItemClockwise(quint16 itemId) {
    initializeTransformationMaps();

    if (clockwiseRotationMap_.contains(itemId)) {
        return clockwiseRotationMap_[itemId];
    }

    // Try using ItemManager's rotateTo property (from wxwidgets)
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        if (props.rotatable && props.rotateTo != 0) {
            return props.rotateTo;
        }
    }

    return itemId; // No rotation available
}

quint16 ItemTransformationHelper::rotateItemCounterClockwise(quint16 itemId) {
    initializeTransformationMaps();

    if (counterClockwiseRotationMap_.contains(itemId)) {
        return counterClockwiseRotationMap_[itemId];
    }

    // For counter-clockwise, we need to find what rotates TO this item
    for (auto it = clockwiseRotationMap_.begin(); it != clockwiseRotationMap_.end(); ++it) {
        if (it.value() == itemId) {
            return it.key();
        }
    }

    return itemId; // No rotation available
}

quint16 ItemTransformationHelper::rotateItem180(quint16 itemId) {
    // 180° rotation is two 90° rotations
    quint16 firstRotation = rotateItemClockwise(itemId);
    if (firstRotation != itemId) {
        return rotateItemClockwise(firstRotation);
    }

    return itemId; // No rotation available
}

quint16 ItemTransformationHelper::flipItemHorizontal(quint16 itemId) {
    initializeTransformationMaps();

    if (horizontalFlipMap_.contains(itemId)) {
        return horizontalFlipMap_[itemId];
    }

    return itemId; // No flip available
}

quint16 ItemTransformationHelper::flipItemVertical(quint16 itemId) {
    initializeTransformationMaps();

    if (verticalFlipMap_.contains(itemId)) {
        return verticalFlipMap_[itemId];
    }

    return itemId; // No flip available
}

bool ItemTransformationHelper::isDirectionalItem(quint16 itemId) {
    // Check if item has directional properties
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        return props.rotatable || isWallItem(itemId) || isDoorItem(itemId);
    }

    return false;
}

bool ItemTransformationHelper::isWallItem(quint16 itemId) {
    // Check if item is a wall-type item
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        return props.isWall || props.name.contains("wall", Qt::CaseInsensitive);
    }

    return false;
}

bool ItemTransformationHelper::isDoorItem(quint16 itemId) {
    // Check if item is a door-type item
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        return props.isDoor || props.name.contains("door", Qt::CaseInsensitive);
    }

    return false;
}

bool ItemTransformationHelper::isTableItem(quint16 itemId) {
    // Check if item is a table-type item
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        return props.name.contains("table", Qt::CaseInsensitive);
    }

    return false;
}

bool ItemTransformationHelper::isCarpetItem(quint16 itemId) {
    // Check if item is a carpet-type item
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        const ItemProperties& props = itemManager->getItemProperties(itemId);
        return props.name.contains("carpet", Qt::CaseInsensitive) ||
               props.name.contains("rug", Qt::CaseInsensitive);
    }

    return false;
}

bool ItemTransformationHelper::isTransformationValid(quint16 fromId, quint16 toId) {
    // Check if transformation from one item to another is valid
    initializeTransformationMaps();

    // Check rotation mappings
    if (clockwiseRotationMap_.value(fromId) == toId ||
        counterClockwiseRotationMap_.value(fromId) == toId) {
        return true;
    }

    // Check flip mappings
    if (horizontalFlipMap_.value(fromId) == toId ||
        verticalFlipMap_.value(fromId) == toId) {
        return true;
    }

    return false;
}

QList<quint16> ItemTransformationHelper::getTransformationChain(quint16 itemId) {
    QList<quint16> chain;
    chain.append(itemId);

    // Follow rotation chain
    quint16 currentId = itemId;
    QSet<quint16> visited;

    while (true) {
        visited.insert(currentId);
        quint16 nextId = rotateItemClockwise(currentId);

        if (nextId == currentId || visited.contains(nextId)) {
            break;
        }

        chain.append(nextId);
        currentId = nextId;
    }

    return chain;
}

void ItemTransformationHelper::initializeTransformationMaps() {
    if (mapsInitialized_) {
        return;
    }

    // Clear existing maps
    clockwiseRotationMap_.clear();
    counterClockwiseRotationMap_.clear();
    horizontalFlipMap_.clear();
    verticalFlipMap_.clear();

    // Load transformation data from ItemManager
    ItemManager* itemManager = ItemManager::instance();
    if (itemManager) {
        loadTransformationsFromItemManager(itemManager);
    }

    // Add hardcoded transformations for common items
    addCommonItemTransformations();

    // Build reverse mappings
    buildReverseMappings();

    mapsInitialized_ = true;
}

void ItemTransformationHelper::loadTransformationsFromItemManager(ItemManager* itemManager) {
    if (!itemManager) return;

    // Get all item IDs from ItemManager
    const auto& itemMap = itemManager->getItemMap();

    for (auto it = itemMap.begin(); it != itemMap.end(); ++it) {
        quint16 itemId = it.key();
        const ItemProperties& props = it.value();

        // Handle rotatable items
        if (props.rotatable && props.rotateTo != 0 && props.rotateTo != itemId) {
            clockwiseRotationMap_[itemId] = props.rotateTo;
        }

        // Handle items with directional variants (walls, doors, etc.)
        if (props.isWall || props.isDoor) {
            addDirectionalItemTransformations(itemId, props);
        }

        // Handle carpet and table items with directional variants
        if (props.name.contains("carpet", Qt::CaseInsensitive) ||
            props.name.contains("table", Qt::CaseInsensitive) ||
            props.name.contains("chair", Qt::CaseInsensitive)) {
            addFurnitureTransformations(itemId, props);
        }
    }
}

void ItemTransformationHelper::addCommonItemTransformations() {
    // Common wall transformations (Tibia standard walls)
    // Stone walls
    clockwiseRotationMap_[1234] = 1235; // horizontal -> vertical
    clockwiseRotationMap_[1235] = 1234; // vertical -> horizontal

    // Wooden walls
    clockwiseRotationMap_[1240] = 1241;
    clockwiseRotationMap_[1241] = 1240;

    // Common door transformations
    // Wooden doors
    clockwiseRotationMap_[1209] = 1210; // north -> east
    clockwiseRotationMap_[1210] = 1211; // east -> south
    clockwiseRotationMap_[1211] = 1212; // south -> west
    clockwiseRotationMap_[1212] = 1209; // west -> north

    // Stone doors
    clockwiseRotationMap_[1249] = 1250;
    clockwiseRotationMap_[1250] = 1251;
    clockwiseRotationMap_[1251] = 1252;
    clockwiseRotationMap_[1252] = 1249;

    // Table transformations
    clockwiseRotationMap_[1728] = 1729; // table north-south -> table east-west
    clockwiseRotationMap_[1729] = 1728; // table east-west -> table north-south

    // Stairs transformations
    horizontalFlipMap_[1385] = 1386; // stairs east -> west
    horizontalFlipMap_[1386] = 1385; // stairs west -> east
    verticalFlipMap_[1387] = 1388;   // stairs north -> south
    verticalFlipMap_[1388] = 1387;   // stairs south -> north

    // Ramp transformations
    clockwiseRotationMap_[1395] = 1396; // ramp north -> east
    clockwiseRotationMap_[1396] = 1397; // ramp east -> south
    clockwiseRotationMap_[1397] = 1398; // ramp south -> west
    clockwiseRotationMap_[1398] = 1395; // ramp west -> north
}

void ItemTransformationHelper::buildReverseMappings() {
    // Build counter-clockwise rotation map from clockwise map
    for (auto it = clockwiseRotationMap_.begin(); it != clockwiseRotationMap_.end(); ++it) {
        counterClockwiseRotationMap_[it.value()] = it.key();
    }

    // Flip maps are typically symmetric (flipping twice returns to original)
    for (auto it = horizontalFlipMap_.begin(); it != horizontalFlipMap_.end(); ++it) {
        if (!horizontalFlipMap_.contains(it.value())) {
            horizontalFlipMap_[it.value()] = it.key();
        }
    }

    for (auto it = verticalFlipMap_.begin(); it != verticalFlipMap_.end(); ++it) {
        if (!verticalFlipMap_.contains(it.value())) {
            verticalFlipMap_[it.value()] = it.key();
        }
    }
}

void ItemTransformationHelper::addDirectionalItemTransformations(quint16 itemId, const ItemProperties& props) {
    // This method would analyze item names and properties to determine
    // directional variants. For now, we'll use basic heuristics.

    QString name = props.name.toLower();

    // Wall items typically have horizontal/vertical variants
    if (props.isWall) {
        // Try to find the corresponding directional variant
        // This is a simplified approach - real implementation would use item database
        quint16 variantId = findDirectionalVariant(itemId, props);
        if (variantId != 0 && variantId != itemId) {
            clockwiseRotationMap_[itemId] = variantId;
        }
    }

    // Door items have 4-directional variants
    if (props.isDoor) {
        addDoorRotationChain(itemId, props);
    }
}

void ItemTransformationHelper::addFurnitureTransformations(quint16 itemId, const ItemProperties& props) {
    QString name = props.name.toLower();

    // Tables often have directional variants
    if (name.contains("table")) {
        // Add table rotation logic
        addTableRotations(itemId, props);
    }

    // Carpets often have corner and edge variants
    if (name.contains("carpet") || name.contains("rug")) {
        addCarpetTransformations(itemId, props);
    }
}

quint16 ItemTransformationHelper::findDirectionalVariant(quint16 itemId, const ItemProperties& props) {
    // Simplified implementation - would need proper item database lookup
    // For now, just try adjacent item IDs
    Q_UNUSED(props);

    // Try the next item ID as a potential variant
    return itemId + 1;
}

void ItemTransformationHelper::addDoorRotationChain(quint16 itemId, const ItemProperties& props) {
    // Simplified implementation for door rotations
    Q_UNUSED(props);

    // Doors typically come in sets of 4 (north, east, south, west)
    // This is a basic heuristic - real implementation would use item database
    quint16 baseId = (itemId / 4) * 4;

    clockwiseRotationMap_[baseId] = baseId + 1;     // north -> east
    clockwiseRotationMap_[baseId + 1] = baseId + 2; // east -> south
    clockwiseRotationMap_[baseId + 2] = baseId + 3; // south -> west
    clockwiseRotationMap_[baseId + 3] = baseId;     // west -> north
}

void ItemTransformationHelper::addTableRotations(quint16 itemId, const ItemProperties& props) {
    // Simplified implementation for table rotations
    Q_UNUSED(props);

    // Tables typically have 2 orientations (north-south, east-west)
    if (itemId % 2 == 0) {
        clockwiseRotationMap_[itemId] = itemId + 1;
    } else {
        clockwiseRotationMap_[itemId] = itemId - 1;
    }
}

void ItemTransformationHelper::addCarpetTransformations(quint16 itemId, const ItemProperties& props) {
    // Simplified implementation for carpet transformations
    Q_UNUSED(props);

    // Carpets often have horizontal/vertical flip variants
    // This is a basic heuristic
    horizontalFlipMap_[itemId] = itemId + 1;
    verticalFlipMap_[itemId] = itemId + 2;
}
