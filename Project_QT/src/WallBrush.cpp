#include "WallBrush.h"
#include "PlaceWallCommand.h" // For creating wall placement/removal commands
#include "Map.h"              // For Map* type hint
#include "Tile.h"             // For Tile* type hint (though not directly used here)
#include "QUndoCommand.h"     // For QUndoCommand* return type
#include <QMouseEvent>        // For QMouseEvent type
#include <QDebug>
#include <QObject>            // For tr()

WallBrush::WallBrush(QObject* parent)
    : TerrainBrush(parent),
      currentWallItemId_(0),
      currentDoorType_(DoorType::None),
      currentAlignment_(WallAlignment::Undefined) {
    setSpecificName(tr("Wall Brush"));

    // Initialize default wall item mappings (Task 39 requirement)
    initializeDefaultWallMappings();

    qDebug() << "WallBrush: Created wall brush with ID" << getID();
}

WallBrush::~WallBrush() {
}

// --- Overrides from Brush/TerrainBrush ---
Brush::Type WallBrush::type() const {
    return Brush::Type::Wall;
}

bool WallBrush::isWall() const {
    // This brush is specifically for walls.
    return true;
}

bool WallBrush::canSmear() const {
    // wxWallBrush returned false, meaning it typically doesn't "smear" like terrain.
    // Actions usually happen on distinct clicks or drags that define segments.
    return false;
}

bool WallBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(tilePos);      // Not used in this very basic check for now
    Q_UNUSED(drawingContext); // Not used in this very basic check for now

    if (!map) {
        qWarning() << "WallBrush::canDraw: Map pointer is null.";
        return false;
    }
    // For placing a wall (applyBrush): canDraw is true if an item ID is configured.
    // For removing a wall (removeBrush): canDraw is generally always true.
    // This method is usually checked before an "apply" action.
    // If currentWallItemId_ is 0, this brush isn't configured to place a specific wall.
    return currentWallItemId_ != 0;
}

QUndoCommand* WallBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) {
        qWarning() << "WallBrush::applyBrush: Map pointer is null.";
        return nullptr;
    }
    if (currentWallItemId_ == 0) {
        qWarning() << "WallBrush::applyBrush: No currentWallItemId_ set. Brush not configured to place a wall.";
        return nullptr;
    }

    // Calculate wall alignment based on neighbors (Task 39 requirement)
    WallAlignment alignment = calculateWallAlignment(map, tilePos);
    quint16 wallItemId = getWallItemForAlignment(alignment);

    // Use calculated wall item ID if available, otherwise use default
    if (wallItemId == 0) {
        wallItemId = currentWallItemId_;
    }

    qDebug() << "WallBrush: Applying wall ID" << wallItemId << "at" << tilePos << "with alignment" << static_cast<int>(alignment);

    // Request wall update for this position and neighbors (Task 39 requirement)
    requestWallUpdate(map, tilePos);

    return new PlaceWallCommand(map, tilePos, wallItemId, parentCommand);
}

QUndoCommand* WallBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) {
        qWarning() << "WallBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }

    qDebug() << "WallBrush: Removing wall at" << tilePos;

    // Request wall update for this position and neighbors (Task 39 requirement)
    requestWallUpdate(map, tilePos);

    return new PlaceWallCommand(map, tilePos, 0, parentCommand); // 0 signifies wall removal
}

int WallBrush::getBrushSize() const {
    // Walls are typically placed one tile at a time by default (size 0).
    // Larger "wall brush sizes" for drawing lines/rectangles of walls would be handled by
    // MapViewInputHandler's getAffectedTiles and iterating calls to applyBrush.
    return 0;
}

Brush::BrushShape WallBrush::getBrushShape() const {
    // Shape is not particularly relevant if size is 0.
    return Brush::BrushShape::Square;
}

QUndoCommand* WallBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (ctrlPressed) {
        // Ctrl+Click with Wall Brush attempts to remove a wall.
        // No canDraw check here for removal, as removeBrush might be able to act even if currentWallItemId_ is 0.
        return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    } else {
        // Normal click attempts to place a wall.
        if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
            return nullptr;
        }
        return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    }
}

QUndoCommand* WallBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                     Map* map, QUndoStack* undoStack,
                                     bool shiftPressed, bool ctrlPressed, bool altPressed,
                                     QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (event->buttons() & Qt::LeftButton) { // If dragging with left button
        if (ctrlPressed) { // Erasing on drag with Ctrl
             return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        } else { // Drawing on drag
            if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
                return nullptr;
            }
            return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* WallBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    // Most wall drawing actions happen on press or move (for dragging lines).
    // Release typically doesn't do anything further for simple wall brushes.
    return nullptr;
}

void WallBrush::cancel() {
    qDebug() << "WallBrush::cancel called";
    // Reset any internal state specific to WallBrush if it had a multi-step operation.
    // For this basic version, no specific state to reset.
}

// --- WallBrush specific methods ---
void WallBrush::setCurrentWallItemId(quint16 itemId) {
    currentWallItemId_ = itemId;
    // Potentially update lookId_ from TerrainBrush if the brush icon should change:
    // if (itemId != 0) setLookID(itemId);
    // else setLookID(DEFAULT_WALL_BRUSH_ICON_ID_OR_SIMILAR);
}

quint16 WallBrush::getCurrentWallItemId() const {
    return currentWallItemId_;
}

// Initialization helper (Task 39 requirement)
void WallBrush::initializeDefaultWallMappings() {
    // Initialize default wall item mappings for different alignments
    // These would typically be loaded from XML configuration

    // Example default mappings (would be configurable)
    if (currentWallItemId_ > 0) {
        wallItemsByAlignment_[WallAlignment::Horizontal] = currentWallItemId_;
        wallItemsByAlignment_[WallAlignment::Vertical] = currentWallItemId_ + 1;
        wallItemsByAlignment_[WallAlignment::Corner] = currentWallItemId_ + 2;
        wallItemsByAlignment_[WallAlignment::TJunction] = currentWallItemId_ + 3;
        wallItemsByAlignment_[WallAlignment::Cross] = currentWallItemId_ + 4;
    }

    // Initialize default door item ranges (example ranges)
    doorItemRanges_.append({1000, 1099}); // Normal doors
    doorItemRanges_.append({1100, 1199}); // Locked doors
    doorItemRanges_.append({1200, 1299}); // Quest doors
    doorItemRanges_.append({1300, 1399}); // Magic doors

    // Initialize default wall item ranges (example ranges)
    wallItemRanges_.append({2000, 2999}); // Stone walls
    wallItemRanges_.append({3000, 3999}); // Wood walls
    wallItemRanges_.append({4000, 4999}); // Metal walls

    qDebug() << "WallBrush: Initialized default wall mappings";
}

// Auto-connection stubs (Task 39 requirement - placeholders for full implementation)
void WallBrush::requestWallUpdate(Map* map, const QPointF& tilePos) const {
    if (!map) {
        return;
    }

    // Placeholder for wall connection system integration
    // This will request wall recalculation for the affected tile and its neighbors
    map->requestWallUpdate(tilePos);

    // Also update neighboring tiles for proper wall connections
    map->requestWallUpdate(tilePos + QPointF(0, -1)); // North
    map->requestWallUpdate(tilePos + QPointF(0, 1));  // South
    map->requestWallUpdate(tilePos + QPointF(-1, 0)); // West
    map->requestWallUpdate(tilePos + QPointF(1, 0));  // East

    qDebug() << "WallBrush::requestWallUpdate: Requested wall update for tile at" << tilePos << "and neighbors";
}

WallBrush::WallAlignment WallBrush::calculateWallAlignment(Map* map, const QPointF& tilePos) const {
    if (!map) {
        return WallAlignment::Undefined;
    }

    // Placeholder for wall connection algorithm (Task 39 requirement)
    // This would analyze neighboring tiles to determine the correct wall alignment

    // Check neighboring tiles for walls
    bool hasNorth = false, hasSouth = false, hasWest = false, hasEast = false;

    // Check north neighbor
    Tile* northTile = map->getTile(tilePos + QPointF(0, -1));
    if (northTile && !northTile->getWallItems().isEmpty()) {
        hasNorth = true;
    }

    // Check south neighbor
    Tile* southTile = map->getTile(tilePos + QPointF(0, 1));
    if (southTile && !southTile->getWallItems().isEmpty()) {
        hasSouth = true;
    }

    // Check west neighbor
    Tile* westTile = map->getTile(tilePos + QPointF(-1, 0));
    if (westTile && !westTile->getWallItems().isEmpty()) {
        hasWest = true;
    }

    // Check east neighbor
    Tile* eastTile = map->getTile(tilePos + QPointF(1, 0));
    if (eastTile && !eastTile->getWallItems().isEmpty()) {
        hasEast = true;
    }

    // Determine alignment based on neighbors
    int connectionCount = (hasNorth ? 1 : 0) + (hasSouth ? 1 : 0) + (hasWest ? 1 : 0) + (hasEast ? 1 : 0);

    if (connectionCount == 0) {
        return WallAlignment::Undefined;
    } else if (connectionCount == 1) {
        return WallAlignment::Undefined; // Single connection, use default
    } else if (connectionCount == 2) {
        if ((hasNorth && hasSouth) || (hasWest && hasEast)) {
            return (hasNorth && hasSouth) ? WallAlignment::Vertical : WallAlignment::Horizontal;
        } else {
            return WallAlignment::Corner;
        }
    } else if (connectionCount == 3) {
        return WallAlignment::TJunction;
    } else {
        return WallAlignment::Cross;
    }
}

quint16 WallBrush::getWallItemForAlignment(WallAlignment alignment) const {
    auto it = wallItemsByAlignment_.find(alignment);
    if (it != wallItemsByAlignment_.end()) {
        return it.value();
    }

    // Return default wall item if no specific alignment mapping exists
    return currentWallItemId_;
}

// Door information handling (Task 39 requirement)
bool WallBrush::hasWall(quint16 itemId) const {
    // Check if the item ID is in any of the wall item ranges
    for (const auto& range : wallItemRanges_) {
        if (itemId >= range.first && itemId <= range.second) {
            return true;
        }
    }

    // Also check if it's the current wall item
    return itemId == currentWallItemId_;
}

WallBrush::DoorType WallBrush::getDoorTypeFromID(quint16 itemId) const {
    // Check door item ranges to determine door type
    if (itemId >= 1000 && itemId <= 1099) {
        return DoorType::Normal;
    } else if (itemId >= 1100 && itemId <= 1199) {
        return DoorType::Locked;
    } else if (itemId >= 1200 && itemId <= 1299) {
        return DoorType::Quest;
    } else if (itemId >= 1300 && itemId <= 1399) {
        return DoorType::Magic;
    } else if (itemId >= 1400 && itemId <= 1499) {
        return DoorType::Level;
    } else if (itemId >= 1500 && itemId <= 1599) {
        return DoorType::Key;
    }

    return DoorType::None;
}

bool WallBrush::isDoorItem(quint16 itemId) const {
    // Check if the item ID is in any of the door item ranges
    for (const auto& range : doorItemRanges_) {
        if (itemId >= range.first && itemId <= range.second) {
            return true;
        }
    }

    return false;
}

void WallBrush::setDoorType(DoorType doorType) {
    currentDoorType_ = doorType;
}

WallBrush::DoorType WallBrush::getDoorType() const {
    return currentDoorType_;
}

// Wall selection/deselection stubs (Task 39 requirement)
void WallBrush::selectWall(Map* map, const QPointF& tilePos) {
    if (!map) {
        return;
    }

    // Placeholder for wall selection logic
    // This would typically highlight the wall or mark it for special operations
    qDebug() << "WallBrush::selectWall: Selected wall at" << tilePos;

    // Could emit a signal or update map state to show selection
    // map->selectWall(tilePos);
}

void WallBrush::deselectWall(Map* map, const QPointF& tilePos) {
    if (!map) {
        return;
    }

    // Placeholder for wall deselection logic
    qDebug() << "WallBrush::deselectWall: Deselected wall at" << tilePos;

    // Could emit a signal or update map state to remove selection
    // map->deselectWall(tilePos);
}

// XML loading support (structure for Task 81)
bool WallBrush::load(const QDomElement& element, QStringList& warnings) {
    // Call base class load first
    if (!TerrainBrush::load(element, warnings)) {
        return false;
    }

    // Load wall-specific properties
    bool ok;

    // Load wall item ID
    quint16 wallId = element.attribute("wallId", "0").toUShort(&ok);
    if (ok && wallId > 0) {
        setCurrentWallItemId(wallId);
        setLookID(wallId);
        setSpecificName(QString("Wall %1").arg(wallId));
    }

    // Load door type
    QString doorTypeStr = element.attribute("doorType", "None");
    if (doorTypeStr == "Normal") {
        setDoorType(DoorType::Normal);
    } else if (doorTypeStr == "Locked") {
        setDoorType(DoorType::Locked);
    } else if (doorTypeStr == "Quest") {
        setDoorType(DoorType::Quest);
    } else if (doorTypeStr == "Magic") {
        setDoorType(DoorType::Magic);
    } else if (doorTypeStr == "Level") {
        setDoorType(DoorType::Level);
    } else if (doorTypeStr == "Key") {
        setDoorType(DoorType::Key);
    } else {
        setDoorType(DoorType::None);
    }

    // Load wall alignment mappings
    QDomElement alignmentElement = element.firstChildElement("alignments");
    if (!alignmentElement.isNull()) {
        QDomElement child = alignmentElement.firstChildElement();
        while (!child.isNull()) {
            QString alignmentName = child.tagName();
            quint16 itemId = child.text().toUShort(&ok);

            if (ok && itemId > 0) {
                if (alignmentName == "horizontal") {
                    wallItemsByAlignment_[WallAlignment::Horizontal] = itemId;
                } else if (alignmentName == "vertical") {
                    wallItemsByAlignment_[WallAlignment::Vertical] = itemId;
                } else if (alignmentName == "corner") {
                    wallItemsByAlignment_[WallAlignment::Corner] = itemId;
                } else if (alignmentName == "tjunction") {
                    wallItemsByAlignment_[WallAlignment::TJunction] = itemId;
                } else if (alignmentName == "cross") {
                    wallItemsByAlignment_[WallAlignment::Cross] = itemId;
                }
            } else {
                warnings.append(QString("Invalid wall alignment item ID: %1 for %2").arg(child.text()).arg(alignmentName));
            }

            child = child.nextSiblingElement();
        }
    }

    // Load door item ranges
    QDomElement doorRangesElement = element.firstChildElement("doorRanges");
    if (!doorRangesElement.isNull()) {
        doorItemRanges_.clear();
        QDomElement rangeElement = doorRangesElement.firstChildElement("range");
        while (!rangeElement.isNull()) {
            quint16 start = rangeElement.attribute("start", "0").toUShort(&ok);
            if (!ok) {
                warnings.append(QString("Invalid door range start: %1").arg(rangeElement.attribute("start")));
                rangeElement = rangeElement.nextSiblingElement("range");
                continue;
            }

            quint16 end = rangeElement.attribute("end", "0").toUShort(&ok);
            if (!ok) {
                warnings.append(QString("Invalid door range end: %1").arg(rangeElement.attribute("end")));
                rangeElement = rangeElement.nextSiblingElement("range");
                continue;
            }

            if (start <= end) {
                doorItemRanges_.append({start, end});
            } else {
                warnings.append(QString("Invalid door range: start (%1) > end (%2)").arg(start).arg(end));
            }

            rangeElement = rangeElement.nextSiblingElement("range");
        }
    }

    // Load wall item ranges
    QDomElement wallRangesElement = element.firstChildElement("wallRanges");
    if (!wallRangesElement.isNull()) {
        wallItemRanges_.clear();
        QDomElement rangeElement = wallRangesElement.firstChildElement("range");
        while (!rangeElement.isNull()) {
            quint16 start = rangeElement.attribute("start", "0").toUShort(&ok);
            if (!ok) {
                warnings.append(QString("Invalid wall range start: %1").arg(rangeElement.attribute("start")));
                rangeElement = rangeElement.nextSiblingElement("range");
                continue;
            }

            quint16 end = rangeElement.attribute("end", "0").toUShort(&ok);
            if (!ok) {
                warnings.append(QString("Invalid wall range end: %1").arg(rangeElement.attribute("end")));
                rangeElement = rangeElement.nextSiblingElement("range");
                continue;
            }

            if (start <= end) {
                wallItemRanges_.append({start, end});
            } else {
                warnings.append(QString("Invalid wall range: start (%1) > end (%2)").arg(start).arg(end));
            }

            rangeElement = rangeElement.nextSiblingElement("range");
        }
    }

    qDebug() << "WallBrush::load: Loaded wall brush with ID" << getCurrentWallItemId()
             << "door type:" << static_cast<int>(getDoorType())
             << "alignment mappings:" << wallItemsByAlignment_.size()
             << "door ranges:" << doorItemRanges_.size()
             << "wall ranges:" << wallItemRanges_.size();

    return true;
}
