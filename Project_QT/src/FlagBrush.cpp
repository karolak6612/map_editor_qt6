#include "FlagBrush.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>
#include <QMouseEvent>
#include <QUndoCommand>
#include <QDomElement>

// FlagBrush implementation
FlagBrush::FlagBrush(quint32 flag, QObject *parent)
    : Brush(parent),
      name_(""),
      flag_(flag),
      zoneId_(0),
      brushSize_(1),
      brushShape_(BrushShape::Square) {
    name_ = getFlagName(flag_);
}

FlagBrush::~FlagBrush() {
    // No cleanup needed
}

// Brush type identification
Brush::Type FlagBrush::type() const {
    return Type::Flag;
}

QString FlagBrush::name() const {
    return name_.isEmpty() ? getFlagName(flag_) : name_;
}

void FlagBrush::setName(const QString& newName) {
    if (name_ != newName) {
        name_ = newName;
        emit brushChanged();
    }
}

bool FlagBrush::isFlag() const {
    return true;
}

FlagBrush* FlagBrush::asFlag() {
    return this;
}

const FlagBrush* FlagBrush::asFlag() const {
    return this;
}

// Brush properties
int FlagBrush::getBrushSize() const {
    return brushSize_;
}

Brush::BrushShape FlagBrush::getBrushShape() const {
    return brushShape_;
}

bool FlagBrush::canDrag() const {
    return true; // Matching wxWidgets FlagBrush behavior
}

bool FlagBrush::canSmear() const {
    return true; // Allow continuous flag setting on mouse move
}

bool FlagBrush::oneSizeFitsAll() const {
    return true; // Flags typically ignore brush size
}

int FlagBrush::getLookID() const {
    return getFlagLookID(flag_);
}

// Core brush action interface
bool FlagBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);

    if (!map) {
        return false;
    }

    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());

    // Check if position is valid
    if (!map->isValidCoordinate(x, y, 0)) {
        return false;
    }

    // Get tile (create if needed for flag operations)
    Tile* tile = map->getTile(x, y, 0);
    if (!tile) {
        return true; // Can create tile for flag
    }

    // Flag brush can only be applied to tiles with ground
    return tileHasGround(tile);
}

QUndoCommand* FlagBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!canDraw(map, tilePos)) {
        return nullptr;
    }

    return new FlagBrushCommand(map, tilePos, flag_, zoneId_, true, parentCommand);
}

QUndoCommand* FlagBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        return nullptr;
    }

    return new FlagBrushCommand(map, tilePos, flag_, zoneId_, false, parentCommand);
}

// Mouse event handlers with proper signatures
QUndoCommand* FlagBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "FlagBrush::mousePressEvent at" << mapPos 
             << "button:" << event->button()
             << "flag:" << flag_
             << "zoneId:" << zoneId_
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Identify target tile(s) based on brush size and shape
    qDebug() << "Target tile(s): [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
    qDebug() << "Brush size:" << getBrushSize() << "shape:" << static_cast<int>(getBrushShape());
    
    if (ctrlPressed) {
        // Ctrl+click typically removes flags
        return removeBrush(map, mapPos, nullptr, parentCommand);
    } else {
        // Normal click sets flags
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }
}

QUndoCommand* FlagBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                       Map* map, QUndoStack* undoStack,
                                       bool shiftPressed, bool ctrlPressed, bool altPressed,
                                       QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    if (event->buttons() != Qt::NoButton && canSmear()) {
        qDebug() << "FlagBrush::mouseMoveEvent at" << mapPos 
                 << "flag:" << flag_
                 << "zoneId:" << zoneId_
                 << "shift:" << shiftPressed 
                 << "ctrl:" << ctrlPressed 
                 << "alt:" << altPressed;
        
        // Placeholder: Continuous flag setting
        qDebug() << "Continuous flag setting at tile: [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
        
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr, parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr, parentCommand);
        }
    }
    
    return nullptr;
}

QUndoCommand* FlagBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                          Map* map, QUndoStack* undoStack,
                                          bool shiftPressed, bool ctrlPressed, bool altPressed,
                                          QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "FlagBrush::mouseReleaseEvent at" << mapPos 
             << "button:" << event->button()
             << "flag:" << flag_
             << "zoneId:" << zoneId_
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Finalize flag operation
    qDebug() << "Finalizing flag operation";
    
    return nullptr; // No additional command needed on release for flag brush
}

void FlagBrush::cancel() {
    qDebug() << "FlagBrush::cancel - Canceling ongoing flag operation";
    // TODO: Implement cancellation logic when needed
}

// FlagBrush specific methods
quint32 FlagBrush::getFlag() const {
    return flag_;
}

void FlagBrush::setFlag(quint32 flag) {
    if (flag_ != flag) {
        flag_ = flag;
        name_ = getFlagName(flag_);
        emit flagChanged(flag);
        emit brushChanged();
    }
}

void FlagBrush::setZoneId(quint16 zoneId) {
    if (zoneId_ != zoneId) {
        zoneId_ = zoneId;
        emit zoneIdChanged(zoneId);
        emit brushChanged();
    }
}

quint16 FlagBrush::getZoneId() const {
    return zoneId_;
}

// Flag type helpers
bool FlagBrush::isProtectionZone() const {
    return flag_ == FlagType::ProtectionZone;
}

bool FlagBrush::isNoPVP() const {
    return flag_ == FlagType::NoPVP;
}

bool FlagBrush::isNoLogout() const {
    return flag_ == FlagType::NoLogout;
}

bool FlagBrush::isPVPZone() const {
    return flag_ == FlagType::PVPZone;
}

bool FlagBrush::isZoneBrush() const {
    return flag_ == FlagType::ZoneBrush;
}

// Static factory methods for common flag brushes
FlagBrush* FlagBrush::createProtectionZoneBrush(QObject* parent) {
    return new FlagBrush(FlagType::ProtectionZone, parent);
}

FlagBrush* FlagBrush::createNoPVPBrush(QObject* parent) {
    return new FlagBrush(FlagType::NoPVP, parent);
}

FlagBrush* FlagBrush::createNoLogoutBrush(QObject* parent) {
    return new FlagBrush(FlagType::NoLogout, parent);
}

FlagBrush* FlagBrush::createPVPZoneBrush(QObject* parent) {
    return new FlagBrush(FlagType::PVPZone, parent);
}

FlagBrush* FlagBrush::createZoneBrush(QObject* parent) {
    return new FlagBrush(FlagType::ZoneBrush, parent);
}

// Brush loading
bool FlagBrush::load(const QDomElement& element, QStringList& warnings) {
    if (!Brush::load(element, warnings)) {
        return false;
    }

    // Load flag-specific properties
    bool ok;
    quint32 flag = element.attribute("flag", "0").toUInt(&ok);
    if (ok) {
        setFlag(flag);
    } else {
        warnings << QString("Invalid flag value in FlagBrush: %1").arg(element.attribute("flag"));
    }

    quint16 zoneId = element.attribute("zone_id", "0").toUShort(&ok);
    if (ok) {
        setZoneId(zoneId);
    } else {
        warnings << QString("Invalid zone_id value in FlagBrush: %1").arg(element.attribute("zone_id"));
    }

    name_ = element.attribute("name", getFlagName(flag_));

    return true;
}

// Helper methods
void FlagBrush::applyFlagToTile(Tile* tile) const {
    if (!tile) return;

    if (flag_ & FlagType::ZoneBrush) {
        if (zoneId_ == 0) {
            // Remove zone flag and clear zone IDs
            tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
            tile->clearZoneIds();
        } else {
            // Set zone flag and add zone ID
            tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), true);
            tile->addZoneId(zoneId_);
        }
    } else {
        // Set regular flag
        tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), true);
    }
}

void FlagBrush::removeFlagFromTile(Tile* tile) const {
    if (!tile) return;

    if (flag_ & FlagType::ZoneBrush) {
        if (zoneId_ == 0) {
            // Remove zone flag and clear all zone IDs
            tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
            tile->clearZoneIds();
        } else {
            // Remove specific zone ID
            tile->removeZoneId(zoneId_);
            if (tile->getZoneIds().isEmpty()) {
                tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
            }
        }
    } else {
        // Remove regular flag
        tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
    }
}

bool FlagBrush::tileHasGround(Tile* tile) const {
    return tile && tile->hasGround();
}

QString FlagBrush::getFlagName(quint32 flag) const {
    switch (flag) {
        case FlagType::ProtectionZone:
            return QStringLiteral("PZ brush (0x01)");
        case FlagType::NoPVP:
            return QStringLiteral("No combat zone brush (0x04)");
        case FlagType::NoLogout:
            return QStringLiteral("No logout zone brush (0x08)");
        case FlagType::PVPZone:
            return QStringLiteral("PVP Zone brush (0x10)");
        case FlagType::ZoneBrush:
            return QStringLiteral("Zone brush (0x40)");
        default:
            return QStringLiteral("Unknown flag brush");
    }
}

int FlagBrush::getFlagLookID(quint32 flag) const {
    switch (flag) {
        case FlagType::ProtectionZone:
            return 1001; // PZ tool sprite ID
        case FlagType::NoPVP:
            return 1002; // No PVP tool sprite ID
        case FlagType::NoLogout:
            return 1003; // No logout tool sprite ID
        case FlagType::PVPZone:
            return 1004; // PVP zone tool sprite ID
        case FlagType::ZoneBrush:
            return 1005; // Zone brush tool sprite ID
        default:
            return 0;
    }
}

// FlagBrushCommand implementation
FlagBrushCommand::FlagBrushCommand(Map* map, const QPointF& tilePos, quint32 flag, quint16 zoneId,
                                  bool isAdding, QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      flag_(flag),
      zoneId_(zoneId),
      isAdding_(isAdding),
      previousMapFlags_(Tile::TileMapFlag::None),
      hadTile_(false) {

    if (isAdding) {
        setText(QString("Set flag 0x%1").arg(flag, 0, 16));
    } else {
        setText(QString("Remove flag 0x%1").arg(flag, 0, 16));
    }

    storeCurrentState();
}

FlagBrushCommand::~FlagBrushCommand() {
    // Clean up if needed
}

void FlagBrushCommand::undo() {
    restorePreviousState();
}

void FlagBrushCommand::redo() {
    executeOperation(isAdding_);
}

void FlagBrushCommand::executeOperation(bool adding) {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);

    if (!tile) {
        // Create tile if it doesn't exist and we're adding a flag
        if (adding) {
            tile = map_->createTile(x, y, 0);
            if (!tile) return;
        } else {
            return; // Can't remove flag from non-existent tile
        }
    }

    // Check if tile has ground (required for flags)
    if (!tile->hasGround()) {
        return;
    }

    if (adding) {
        // Apply flag to tile
        if (flag_ & FlagBrush::FlagType::ZoneBrush) {
            if (zoneId_ == 0) {
                // Remove zone flag and clear zone IDs
                tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
                tile->clearZoneIds();
            } else {
                // Set zone flag and add zone ID
                tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), true);
                tile->addZoneId(zoneId_);
            }
        } else {
            // Set regular flag
            tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), true);
        }
    } else {
        // Remove flag from tile
        if (flag_ & FlagBrush::FlagType::ZoneBrush) {
            if (zoneId_ == 0) {
                // Remove zone flag and clear all zone IDs
                tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
                tile->clearZoneIds();
            } else {
                // Remove specific zone ID
                tile->removeZoneId(zoneId_);
                if (tile->getZoneIds().isEmpty()) {
                    tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
                }
            }
        } else {
            // Remove regular flag
            tile->setMapFlag(static_cast<Tile::TileMapFlag>(flag_), false);
        }
    }
}

void FlagBrushCommand::storeCurrentState() {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);

    if (tile) {
        hadTile_ = true;
        previousMapFlags_ = tile->getMapFlags();
        previousZoneIds_ = tile->getZoneIds();
    } else {
        hadTile_ = false;
        previousMapFlags_ = Tile::TileMapFlag::None;
        previousZoneIds_.clear();
    }
}

void FlagBrushCommand::restorePreviousState() {
    if (!map_) return;

    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);

    if (!tile && hadTile_) {
        // Tile was deleted, recreate it
        tile = map_->createTile(x, y, 0);
        if (!tile) return;
    } else if (tile && !hadTile_) {
        // Tile was created, remove it if it's empty
        if (tile->isEmpty()) {
            map_->removeTile(x, y, 0);
            return;
        }
    }

    if (tile) {
        // Restore previous flags and zone IDs
        tile->setMapFlags(previousMapFlags_);
        tile->clearZoneIds();
        for (quint16 zoneId : previousZoneIds_) {
            tile->addZoneId(zoneId);
        }
    }
}


