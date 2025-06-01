#include "WaypointBrush.h"
#include "Waypoint.h"
#include "Waypoints.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "ItemManager.h"
#include <QDebug>
#include <QUndoCommand>
#include <QDomElement>
#include <QDateTime>

// WaypointBrush implementation
WaypointBrush::WaypointBrush(QObject* parent)
    : Brush(parent),
      name_("Waypoint Brush"),
      waypointName_(""),
      waypointType_("generic"),
      waypointScript_(""),
      waypointRadius_(1),
      waypointColor_(Qt::blue),
      waypointIcon_("generic"),
      autoGenerateName_(true),
      markWithItem_(true),
      markerItemId_(1387), // Default waypoint marker item
      replaceExisting_(true) {
}

WaypointBrush::~WaypointBrush() {
    // No cleanup needed
}

// Brush interface implementation
Brush::Type WaypointBrush::type() const {
    return Type::Waypoint;
}

QString WaypointBrush::name() const {
    return name_;
}

void WaypointBrush::setName(const QString& newName) {
    if (name_ != newName) {
        name_ = newName;
        emit brushChanged();
    }
}

int WaypointBrush::getLookID() const {
    // Return the marker item ID for UI display
    return markerItemId_;
}

// Type identification
bool WaypointBrush::isWaypoint() const {
    return true;
}

WaypointBrush* WaypointBrush::asWaypoint() {
    return this;
}

const WaypointBrush* WaypointBrush::asWaypoint() const {
    return this;
}

// Core brush actions
bool WaypointBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    
    if (!map) {
        return false;
    }
    
    return isValidWaypointPosition(map, tilePos);
}

QUndoCommand* WaypointBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!canDraw(map, tilePos)) {
        return nullptr;
    }
    
    QString finalWaypointName = waypointName_;
    if (autoGenerateName_ || finalWaypointName.isEmpty()) {
        finalWaypointName = generateWaypointName(map, tilePos);
    }
    
    return new WaypointBrushCommand(map, tilePos, finalWaypointName, waypointType_, 
                                   waypointScript_, waypointRadius_, waypointColor_, 
                                   waypointIcon_, true, parentCommand);
}

QUndoCommand* WaypointBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!map) {
        return nullptr;
    }
    
    return new WaypointBrushCommand(map, tilePos, "", "", "", 0, QColor(), "", false, parentCommand);
}

// Waypoint-specific properties
QString WaypointBrush::getWaypointName() const {
    return waypointName_;
}

void WaypointBrush::setWaypointName(const QString& name) {
    if (waypointName_ != name) {
        waypointName_ = name;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

QString WaypointBrush::getWaypointType() const {
    return waypointType_;
}

void WaypointBrush::setWaypointType(const QString& type) {
    if (waypointType_ != type) {
        waypointType_ = type;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

QString WaypointBrush::getWaypointScript() const {
    return waypointScript_;
}

void WaypointBrush::setWaypointScript(const QString& script) {
    if (waypointScript_ != script) {
        waypointScript_ = script;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

int WaypointBrush::getWaypointRadius() const {
    return waypointRadius_;
}

void WaypointBrush::setWaypointRadius(int radius) {
    if (waypointRadius_ != radius) {
        waypointRadius_ = radius;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

QColor WaypointBrush::getWaypointColor() const {
    return waypointColor_;
}

void WaypointBrush::setWaypointColor(const QColor& color) {
    if (waypointColor_ != color) {
        waypointColor_ = color;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

QString WaypointBrush::getWaypointIcon() const {
    return waypointIcon_;
}

void WaypointBrush::setWaypointIcon(const QString& icon) {
    if (waypointIcon_ != icon) {
        waypointIcon_ = icon;
        emit waypointPropertiesChanged();
        emit brushChanged();
    }
}

// Brush configuration
bool WaypointBrush::getAutoGenerateName() const {
    return autoGenerateName_;
}

void WaypointBrush::setAutoGenerateName(bool autoGenerate) {
    if (autoGenerateName_ != autoGenerate) {
        autoGenerateName_ = autoGenerate;
        emit brushChanged();
    }
}

bool WaypointBrush::getMarkWithItem() const {
    return markWithItem_;
}

void WaypointBrush::setMarkWithItem(bool mark) {
    if (markWithItem_ != mark) {
        markWithItem_ = mark;
        emit brushChanged();
    }
}

quint16 WaypointBrush::getMarkerItemId() const {
    return markerItemId_;
}

void WaypointBrush::setMarkerItemId(quint16 itemId) {
    if (markerItemId_ != itemId) {
        markerItemId_ = itemId;
        emit brushChanged();
    }
}

bool WaypointBrush::getReplaceExisting() const {
    return replaceExisting_;
}

void WaypointBrush::setReplaceExisting(bool replace) {
    if (replaceExisting_ != replace) {
        replaceExisting_ = replace;
        emit brushChanged();
    }
}

// Drawing options
bool WaypointBrush::canDrag() const {
    return false; // Waypoint brush doesn't support dragging
}

bool WaypointBrush::canSmear() const {
    return false; // Waypoint brush doesn't support smearing
}

bool WaypointBrush::oneSizeFitsAll() const {
    return true; // Waypoint brush doesn't have size variations
}

// Brush loading
bool WaypointBrush::load(const QDomElement& element, QStringList& warnings) {
    if (!Brush::load(element, warnings)) {
        return false;
    }
    
    // Load waypoint-specific properties
    name_ = element.attribute("name", "Waypoint Brush");
    waypointName_ = element.attribute("waypoint_name", "");
    waypointType_ = element.attribute("waypoint_type", "generic");
    waypointScript_ = element.attribute("waypoint_script", "");
    waypointRadius_ = element.attribute("waypoint_radius", "1").toInt();
    waypointColor_ = QColor(element.attribute("waypoint_color", "#0000FF"));
    waypointIcon_ = element.attribute("waypoint_icon", "generic");
    autoGenerateName_ = element.attribute("auto_generate_name", "true").toLower() == "true";
    markWithItem_ = element.attribute("mark_with_item", "true").toLower() == "true";
    markerItemId_ = element.attribute("marker_item_id", "1387").toUShort();
    replaceExisting_ = element.attribute("replace_existing", "true").toLower() == "true";
    
    return true;
}

// Helper methods
QString WaypointBrush::generateWaypointName(Map* map, const QPointF& tilePos) const {
    if (!map) return "Waypoint";
    
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    
    // Generate name based on position and timestamp
    QString baseName = QString("Waypoint_%1_%2").arg(x).arg(y);
    
    // Check if name already exists and add suffix if needed
    QString finalName = baseName;
    int suffix = 1;
    while (map->getWaypoint(finalName) != nullptr) {
        finalName = QString("%1_%2").arg(baseName).arg(suffix);
        ++suffix;
    }
    
    return finalName;
}

void WaypointBrush::addWaypointMarker(Tile* tile) const {
    if (!tile || !markWithItem_) return;
    
    // Check if marker item already exists
    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->getServerId() == markerItemId_) {
            return; // Marker already exists
        }
    }
    
    // Create and add marker item
    ItemManager* itemManager = ItemManager::getInstancePtr();
    if (itemManager) {
        Item* marker = itemManager->createItem(markerItemId_);
        if (marker) {
            tile->addItem(marker);
            qDebug() << "Added waypoint marker item" << markerItemId_ << "to tile";
        }
    }
}

void WaypointBrush::removeWaypointMarker(Tile* tile) const {
    if (!tile) return;
    
    // Find and remove marker items
    QVector<Item*> itemsToRemove;
    const QVector<Item*>& items = tile->getItems();
    
    for (Item* item : items) {
        if (item && item->getServerId() == markerItemId_) {
            itemsToRemove.append(item);
        }
    }
    
    for (Item* item : itemsToRemove) {
        tile->removeItem(item);
        delete item;
        qDebug() << "Removed waypoint marker item" << markerItemId_ << "from tile";
    }
}

bool WaypointBrush::isValidWaypointPosition(Map* map, const QPointF& tilePos) const {
    if (!map) return false;
    
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    
    // Check if position is valid
    return map->isValidCoordinate(x, y, 0);
}

Waypoint* WaypointBrush::createWaypoint(Map* map, const QPointF& tilePos) const {
    if (!map) return nullptr;
    
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    
    QString finalName = waypointName_;
    if (autoGenerateName_ || finalName.isEmpty()) {
        finalName = generateWaypointName(map, tilePos);
    }
    
    MapPos position(x, y, 0);
    Waypoint* waypoint = new Waypoint(finalName, position, waypointType_, waypointScript_);
    waypoint->setRadius(waypointRadius_);
    waypoint->setColor(waypointColor_);
    waypoint->setIconType(waypointIcon_);
    
    return waypoint;
}

// WaypointBrushCommand implementation
WaypointBrushCommand::WaypointBrushCommand(Map* map, const QPointF& tilePos,
                                          const QString& waypointName, const QString& waypointType,
                                          const QString& waypointScript, int radius,
                                          const QColor& color, const QString& icon,
                                          bool isAdding, QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      waypointName_(waypointName),
      waypointType_(waypointType),
      waypointScript_(waypointScript),
      waypointRadius_(radius),
      waypointColor_(color),
      waypointIcon_(icon),
      isAdding_(isAdding),
      hadWaypoint_(false),
      previousWaypointName_(""),
      hadMarkerItem_(false),
      markerItemId_(1387) {

    if (isAdding) {
        setText(QString("Add waypoint '%1'").arg(waypointName));
    } else {
        setText("Remove waypoint");
    }

    storeCurrentState();
}

WaypointBrushCommand::~WaypointBrushCommand() {
    // Clean up if needed
}

void WaypointBrushCommand::undo() {
    restorePreviousState();
}

void WaypointBrushCommand::redo() {
    executeOperation(isAdding_);
}

void WaypointBrushCommand::executeOperation(bool adding) {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    if (!tile) {
        // Create tile if it doesn't exist
        tile = map_->createTile(x, y, 0);
        if (!tile) return;
    }

    if (adding) {
        // Create and add waypoint
        MapPos position(x, y, 0);
        Waypoint* waypoint = new Waypoint(waypointName_, position, waypointType_, waypointScript_);
        waypoint->setRadius(waypointRadius_);
        waypoint->setColor(waypointColor_);
        waypoint->setIconType(waypointIcon_);

        map_->addWaypoint(waypoint);

        // Add marker item
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (itemManager) {
            Item* marker = itemManager->createItem(markerItemId_);
            if (marker) {
                tile->addItem(marker);
            }
        }
    } else {
        // Remove waypoint at this position
        const QList<Waypoint*> waypoints = map_->getWaypoints();
        for (Waypoint* waypoint : waypoints) {
            if (waypoint && waypoint->position().x == x && waypoint->position().y == y) {
                map_->removeWaypoint(waypoint);
                break;
            }
        }

        // Remove marker items
        QVector<Item*> itemsToRemove;
        const QVector<Item*>& items = tile->getItems();

        for (Item* item : items) {
            if (item && item->getServerId() == markerItemId_) {
                itemsToRemove.append(item);
            }
        }

        for (Item* item : itemsToRemove) {
            tile->removeItem(item);
            delete item;
        }
    }
}

void WaypointBrushCommand::storeCurrentState() {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());

    // Check if waypoint exists at this position
    const QList<Waypoint*> waypoints = map_->getWaypoints();
    for (Waypoint* waypoint : waypoints) {
        if (waypoint && waypoint->position().x == x && waypoint->position().y == y) {
            hadWaypoint_ = true;
            previousWaypointName_ = waypoint->name();
            break;
        }
    }

    // Check if tile has marker item
    Tile* tile = map_->getTile(x, y, 0);
    if (tile) {
        const QVector<Item*>& items = tile->getItems();
        for (Item* item : items) {
            if (item && item->getServerId() == markerItemId_) {
                hadMarkerItem_ = true;
                break;
            }
        }
    }
}

void WaypointBrushCommand::restorePreviousState() {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    if (!tile) return;

    if (isAdding_) {
        // Remove the waypoint we added
        map_->removeWaypoint(waypointName_);

        // Remove marker item if it wasn't there before
        if (!hadMarkerItem_) {
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();

            for (Item* item : items) {
                if (item && item->getServerId() == markerItemId_) {
                    itemsToRemove.append(item);
                }
            }

            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
        }
    } else {
        // Restore waypoint if it was there before
        if (hadWaypoint_) {
            MapPos position(x, y, 0);
            Waypoint* waypoint = new Waypoint(previousWaypointName_, position, "generic", "");
            map_->addWaypoint(waypoint);
        }

        // Restore marker item if it was there before
        if (hadMarkerItem_) {
            ItemManager* itemManager = ItemManager::getInstancePtr();
            if (itemManager) {
                Item* marker = itemManager->createItem(markerItemId_);
                if (marker) {
                    tile->addItem(marker);
                }
            }
        }
    }
}

#include "WaypointBrush.moc"
