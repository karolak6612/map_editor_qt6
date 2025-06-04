#include "TilePropertyCommand.h"
#include "Tile.h"
#include "Map.h"
#include <QDebug>

// SetTilePropertyCommand implementation
SetTilePropertyCommand::SetTilePropertyCommand(Tile* tile, const QString& property, 
                                             const QVariant& newValue, const QVariant& oldValue,
                                             Map* map, QUndoCommand* parent)
    : QUndoCommand(QString("Set Tile %1").arg(property), parent),
      tile_(tile),
      map_(map),
      property_(property),
      oldValue_(oldValue),
      newValue_(newValue) {
}

void SetTilePropertyCommand::undo() {
    if (tile_ && isValidProperty(property_) && isValidValue(property_, oldValue_)) {
        applyProperty(oldValue_);
        updateMap();
        qDebug() << "SetTilePropertyCommand: Undid property" << property_;
    }
}

void SetTilePropertyCommand::redo() {
    if (tile_ && isValidProperty(property_) && isValidValue(property_, newValue_)) {
        applyProperty(newValue_);
        updateMap();
        qDebug() << "SetTilePropertyCommand: Applied property" << property_;
    }
}

int SetTilePropertyCommand::id() const {
    return COMMAND_ID;
}

bool SetTilePropertyCommand::mergeWith(const QUndoCommand* other) {
    const SetTilePropertyCommand* otherCommand = dynamic_cast<const SetTilePropertyCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_ || otherCommand->property_ != property_) {
        return false;
    }
    
    // Merge by updating the new value
    newValue_ = otherCommand->newValue_;
    return true;
}

void SetTilePropertyCommand::applyProperty(const QVariant& value) {
    if (!tile_) return;
    
    if (property_ == "houseId") {
        tile_->setHouseId(value.toUInt());
    } else if (property_ == "minimapColor") {
        tile_->setMinimapColor(value.toUInt());
    } else if (property_ == "mapFlags") {
        tile_->setMapFlags(value.toUInt());
    } else if (property_ == "stateFlags") {
        tile_->setStateFlags(value.toUInt());
    }
    // TODO: Add more properties as needed
}

void SetTilePropertyCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

bool SetTilePropertyCommand::isValidProperty(const QString& property) const {
    static const QStringList validProperties = {"houseId", "minimapColor", "mapFlags", "stateFlags"};
    return validProperties.contains(property);
}

bool SetTilePropertyCommand::isValidValue(const QString& property, const QVariant& value) const {
    Q_UNUSED(property);
    return value.isValid();
}

// SetTilePropertiesCommand implementation
SetTilePropertiesCommand::SetTilePropertiesCommand(Tile* tile, const QMap<QString, QVariant>& newValues,
                                                 const QMap<QString, QVariant>& oldValues,
                                                 Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile Properties", parent),
      tile_(tile),
      map_(map),
      oldValues_(oldValues),
      newValues_(newValues) {
}

void SetTilePropertiesCommand::undo() {
    if (tile_ && validateProperties(oldValues_)) {
        applyProperties(oldValues_);
        updateMap();
        qDebug() << "SetTilePropertiesCommand: Undid" << oldValues_.size() << "properties";
    }
}

void SetTilePropertiesCommand::redo() {
    if (tile_ && validateProperties(newValues_)) {
        applyProperties(newValues_);
        updateMap();
        qDebug() << "SetTilePropertiesCommand: Applied" << newValues_.size() << "properties";
    }
}

int SetTilePropertiesCommand::id() const {
    return COMMAND_ID;
}

bool SetTilePropertiesCommand::mergeWith(const QUndoCommand* other) {
    const SetTilePropertiesCommand* otherCommand = dynamic_cast<const SetTilePropertiesCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_) {
        return false;
    }
    
    // Merge by updating the new values
    for (auto it = otherCommand->newValues_.begin(); it != otherCommand->newValues_.end(); ++it) {
        newValues_[it.key()] = it.value();
    }
    return true;
}

void SetTilePropertiesCommand::applyProperties(const QMap<QString, QVariant>& values) {
    if (!tile_) return;
    
    for (auto it = values.begin(); it != values.end(); ++it) {
        const QString& property = it.key();
        const QVariant& value = it.value();
        
        if (property == "houseId") {
            tile_->setHouseId(value.toUInt());
        } else if (property == "minimapColor") {
            tile_->setMinimapColor(value.toUInt());
        } else if (property == "mapFlags") {
            tile_->setMapFlags(value.toUInt());
        } else if (property == "stateFlags") {
            tile_->setStateFlags(value.toUInt());
        }
        // TODO: Add more properties as needed
    }
}

void SetTilePropertiesCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

bool SetTilePropertiesCommand::validateProperties(const QMap<QString, QVariant>& values) const {
    static const QStringList validProperties = {"houseId", "minimapColor", "mapFlags", "stateFlags"};
    
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (!validProperties.contains(it.key()) || !it.value().isValid()) {
            return false;
        }
    }
    return true;
}

// SetMultipleTilePropertiesCommand implementation
SetMultipleTilePropertiesCommand::SetMultipleTilePropertiesCommand(const QList<TilePropertyChange>& changes,
                                                                 Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Multiple Tile Properties", parent),
      changes_(changes),
      map_(map) {
}

void SetMultipleTilePropertiesCommand::undo() {
    if (validateChanges(changes_)) {
        applyChanges(false);
        updateMap();
        qDebug() << "SetMultipleTilePropertiesCommand: Undid" << changes_.size() << "changes";
    }
}

void SetMultipleTilePropertiesCommand::redo() {
    if (validateChanges(changes_)) {
        applyChanges(true);
        updateMap();
        qDebug() << "SetMultipleTilePropertiesCommand: Applied" << changes_.size() << "changes";
    }
}

int SetMultipleTilePropertiesCommand::id() const {
    return COMMAND_ID;
}

bool SetMultipleTilePropertiesCommand::mergeWith(const QUndoCommand* other) {
    const SetMultipleTilePropertiesCommand* otherCommand = dynamic_cast<const SetMultipleTilePropertiesCommand*>(other);
    if (!otherCommand || otherCommand->map_ != map_) {
        return false;
    }
    
    // Merge changes
    changes_.append(otherCommand->changes_);
    return true;
}

void SetMultipleTilePropertiesCommand::applyChanges(bool forward) {
    for (const TilePropertyChange& change : changes_) {
        if (!change.tile) continue;
        
        const QVariant& value = forward ? change.newValue : change.oldValue;
        
        if (change.property == "houseId") {
            change.tile->setHouseId(value.toUInt());
        } else if (change.property == "minimapColor") {
            change.tile->setMinimapColor(value.toUInt());
        } else if (change.property == "mapFlags") {
            change.tile->setMapFlags(value.toUInt());
        } else if (change.property == "stateFlags") {
            change.tile->setStateFlags(value.toUInt());
        }
        // TODO: Add more properties as needed
    }
}

void SetMultipleTilePropertiesCommand::updateMap() {
    if (map_) {
        // Update all affected tiles
        for (const TilePropertyChange& change : changes_) {
            if (change.tile) {
                map_->updateTile(change.tile);
            }
        }
    }
}

bool SetMultipleTilePropertiesCommand::validateChanges(const QList<TilePropertyChange>& changes) const {
    static const QStringList validProperties = {"houseId", "minimapColor", "mapFlags", "stateFlags"};
    
    for (const TilePropertyChange& change : changes) {
        if (!change.tile || !validProperties.contains(change.property) || 
            !change.oldValue.isValid() || !change.newValue.isValid()) {
            return false;
        }
    }
    return true;
}

// SetTileMapFlagCommand implementation
SetTileMapFlagCommand::SetTileMapFlagCommand(Tile* tile, quint32 flag, bool enabled,
                                           Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile Map Flag", parent),
      tile_(tile),
      map_(map),
      flag_(flag),
      enabled_(enabled) {
    if (tile_) {
        wasEnabled_ = tile_->hasMapFlag(flag_);
    }
}

void SetTileMapFlagCommand::undo() {
    if (tile_) {
        applyFlag(wasEnabled_);
        updateMap();
        qDebug() << "SetTileMapFlagCommand: Restored flag" << flag_ << "to" << wasEnabled_;
    }
}

void SetTileMapFlagCommand::redo() {
    if (tile_) {
        applyFlag(enabled_);
        updateMap();
        qDebug() << "SetTileMapFlagCommand: Set flag" << flag_ << "to" << enabled_;
    }
}

int SetTileMapFlagCommand::id() const {
    return COMMAND_ID;
}

bool SetTileMapFlagCommand::mergeWith(const QUndoCommand* other) {
    const SetTileMapFlagCommand* otherCommand = dynamic_cast<const SetTileMapFlagCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_ || otherCommand->flag_ != flag_) {
        return false;
    }
    
    // Merge by updating the enabled state
    enabled_ = otherCommand->enabled_;
    return true;
}

void SetTileMapFlagCommand::applyFlag(bool enable) {
    if (tile_) {
        tile_->setMapFlag(flag_, enable);
    }
}

void SetTileMapFlagCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

// SetTileStateFlagCommand implementation
SetTileStateFlagCommand::SetTileStateFlagCommand(Tile* tile, quint32 flag, bool enabled,
                                               Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile State Flag", parent),
      tile_(tile),
      map_(map),
      flag_(flag),
      enabled_(enabled) {
    if (tile_) {
        wasEnabled_ = tile_->hasStateFlag(flag_);
    }
}

void SetTileStateFlagCommand::undo() {
    if (tile_) {
        applyFlag(wasEnabled_);
        updateMap();
        qDebug() << "SetTileStateFlagCommand: Restored flag" << flag_ << "to" << wasEnabled_;
    }
}

void SetTileStateFlagCommand::redo() {
    if (tile_) {
        applyFlag(enabled_);
        updateMap();
        qDebug() << "SetTileStateFlagCommand: Set flag" << flag_ << "to" << enabled_;
    }
}

int SetTileStateFlagCommand::id() const {
    return COMMAND_ID;
}

bool SetTileStateFlagCommand::mergeWith(const QUndoCommand* other) {
    const SetTileStateFlagCommand* otherCommand = dynamic_cast<const SetTileStateFlagCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_ || otherCommand->flag_ != flag_) {
        return false;
    }

    // Merge by updating the enabled state
    enabled_ = otherCommand->enabled_;
    return true;
}

void SetTileStateFlagCommand::applyFlag(bool enable) {
    if (tile_) {
        tile_->setStateFlag(flag_, enable);
    }
}

void SetTileStateFlagCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

// SetTileHouseIdCommand implementation
SetTileHouseIdCommand::SetTileHouseIdCommand(Tile* tile, quint32 newHouseId, quint32 oldHouseId,
                                           Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile House ID", parent),
      tile_(tile),
      map_(map),
      oldHouseId_(oldHouseId),
      newHouseId_(newHouseId) {
}

void SetTileHouseIdCommand::undo() {
    if (tile_ && isValidHouseId(oldHouseId_)) {
        applyHouseId(oldHouseId_);
        updateMap();
        qDebug() << "SetTileHouseIdCommand: Restored house ID to" << oldHouseId_;
    }
}

void SetTileHouseIdCommand::redo() {
    if (tile_ && isValidHouseId(newHouseId_)) {
        applyHouseId(newHouseId_);
        updateMap();
        qDebug() << "SetTileHouseIdCommand: Set house ID to" << newHouseId_;
    }
}

int SetTileHouseIdCommand::id() const {
    return COMMAND_ID;
}

bool SetTileHouseIdCommand::mergeWith(const QUndoCommand* other) {
    const SetTileHouseIdCommand* otherCommand = dynamic_cast<const SetTileHouseIdCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_) {
        return false;
    }

    // Merge by updating the new house ID
    newHouseId_ = otherCommand->newHouseId_;
    return true;
}

void SetTileHouseIdCommand::applyHouseId(quint32 houseId) {
    if (tile_) {
        tile_->setHouseId(houseId);
    }
}

void SetTileHouseIdCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

bool SetTileHouseIdCommand::isValidHouseId(quint32 houseId) const {
    // House ID 0 means no house, which is valid
    Q_UNUSED(houseId);
    return true;
}

// SetTileZoneIdCommand implementation
SetTileZoneIdCommand::SetTileZoneIdCommand(Tile* tile, const QList<quint16>& newZoneIds,
                                         const QList<quint16>& oldZoneIds,
                                         Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile Zone IDs", parent),
      tile_(tile),
      map_(map),
      oldZoneIds_(oldZoneIds),
      newZoneIds_(newZoneIds) {
}

void SetTileZoneIdCommand::undo() {
    if (tile_) {
        applyZoneIds(oldZoneIds_);
        updateMap();
        qDebug() << "SetTileZoneIdCommand: Restored zone IDs";
    }
}

void SetTileZoneIdCommand::redo() {
    if (tile_) {
        applyZoneIds(newZoneIds_);
        updateMap();
        qDebug() << "SetTileZoneIdCommand: Set zone IDs";
    }
}

int SetTileZoneIdCommand::id() const {
    return COMMAND_ID;
}

bool SetTileZoneIdCommand::mergeWith(const QUndoCommand* other) {
    const SetTileZoneIdCommand* otherCommand = dynamic_cast<const SetTileZoneIdCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_) {
        return false;
    }

    // Merge by updating the new zone IDs
    newZoneIds_ = otherCommand->newZoneIds_;
    return true;
}

void SetTileZoneIdCommand::applyZoneIds(const QList<quint16>& zoneIds) {
    if (tile_) {
        tile_->setZoneIds(zoneIds);
    }
}

void SetTileZoneIdCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

bool SetTileZoneIdCommand::isValidZoneId(quint16 zoneId) const {
    // Zone ID 0 means no zone, which is valid
    Q_UNUSED(zoneId);
    return true;
}

// SetTileMinimapColorCommand implementation
SetTileMinimapColorCommand::SetTileMinimapColorCommand(Tile* tile, quint8 newColor, quint8 oldColor,
                                                     Map* map, QUndoCommand* parent)
    : QUndoCommand("Set Tile Minimap Color", parent),
      tile_(tile),
      map_(map),
      oldColor_(oldColor),
      newColor_(newColor) {
}

void SetTileMinimapColorCommand::undo() {
    if (tile_ && isValidColor(oldColor_)) {
        applyColor(oldColor_);
        updateMap();
        qDebug() << "SetTileMinimapColorCommand: Restored color to" << oldColor_;
    }
}

void SetTileMinimapColorCommand::redo() {
    if (tile_ && isValidColor(newColor_)) {
        applyColor(newColor_);
        updateMap();
        qDebug() << "SetTileMinimapColorCommand: Set color to" << newColor_;
    }
}

int SetTileMinimapColorCommand::id() const {
    return COMMAND_ID;
}

bool SetTileMinimapColorCommand::mergeWith(const QUndoCommand* other) {
    const SetTileMinimapColorCommand* otherCommand = dynamic_cast<const SetTileMinimapColorCommand*>(other);
    if (!otherCommand || otherCommand->tile_ != tile_) {
        return false;
    }

    // Merge by updating the new color
    newColor_ = otherCommand->newColor_;
    return true;
}

void SetTileMinimapColorCommand::applyColor(quint8 color) {
    if (tile_) {
        tile_->setMinimapColor(color);
    }
}

void SetTileMinimapColorCommand::updateMap() {
    if (map_) {
        map_->updateTile(tile_);
    }
}

bool SetTileMinimapColorCommand::isValidColor(quint8 color) const {
    // All 8-bit color values are valid
    Q_UNUSED(color);
    return true;
}

// TilePropertyCommandFactory implementation
SetTilePropertyCommand* TilePropertyCommandFactory::createPropertyCommand(Tile* tile, const QString& property,
                                                                        const QVariant& newValue, const QVariant& oldValue,
                                                                        Map* map) {
    return new SetTilePropertyCommand(tile, property, newValue, oldValue, map);
}

SetTilePropertiesCommand* TilePropertyCommandFactory::createPropertiesCommand(Tile* tile,
                                                                            const QMap<QString, QVariant>& newValues,
                                                                            const QMap<QString, QVariant>& oldValues,
                                                                            Map* map) {
    return new SetTilePropertiesCommand(tile, newValues, oldValues, map);
}

SetMultipleTilePropertiesCommand* TilePropertyCommandFactory::createBatchCommand(const QList<SetMultipleTilePropertiesCommand::TilePropertyChange>& changes,
                                                                               Map* map) {
    return new SetMultipleTilePropertiesCommand(changes, map);
}

SetTileMapFlagCommand* TilePropertyCommandFactory::createMapFlagCommand(Tile* tile, quint32 flag, bool enabled, Map* map) {
    return new SetTileMapFlagCommand(tile, flag, enabled, map);
}

SetTileStateFlagCommand* TilePropertyCommandFactory::createStateFlagCommand(Tile* tile, quint32 flag, bool enabled, Map* map) {
    return new SetTileStateFlagCommand(tile, flag, enabled, map);
}

SetTileHouseIdCommand* TilePropertyCommandFactory::createHouseIdCommand(Tile* tile, quint32 newHouseId, quint32 oldHouseId, Map* map) {
    return new SetTileHouseIdCommand(tile, newHouseId, oldHouseId, map);
}

SetTileZoneIdCommand* TilePropertyCommandFactory::createZoneIdCommand(Tile* tile, const QList<quint16>& newZoneIds, const QList<quint16>& oldZoneIds, Map* map) {
    return new SetTileZoneIdCommand(tile, newZoneIds, oldZoneIds, map);
}

SetTileMinimapColorCommand* TilePropertyCommandFactory::createMinimapColorCommand(Tile* tile, quint8 newColor, quint8 oldColor, Map* map) {
    return new SetTileMinimapColorCommand(tile, newColor, oldColor, map);
}
