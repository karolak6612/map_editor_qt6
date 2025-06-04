#ifndef TILEPROPERTYCOMMAND_H
#define TILEPROPERTYCOMMAND_H

#include <QUndoCommand>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

// Forward declarations
class Tile;
class Map;

/**
 * @brief Task 93: Tile Property Command for undo/redo functionality
 * 
 * Complete undo/redo system for tile property editing:
 * - Single property changes with full undo/redo support
 * - Batch property changes for multiple properties
 * - Map integration for automatic updates
 * - Property validation and error handling
 * - Efficient memory usage and command merging
 * - Support for all tile properties (flags, house ID, zone IDs, minimap color)
 */

/**
 * @brief Single Tile Property Change Command
 */
class SetTilePropertyCommand : public QUndoCommand
{
public:
    explicit SetTilePropertyCommand(Tile* tile, const QString& property, 
                                   const QVariant& newValue, const QVariant& oldValue,
                                   Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTilePropertyCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    QString getProperty() const { return property_; }
    QVariant getOldValue() const { return oldValue_; }
    QVariant getNewValue() const { return newValue_; }

private:
    void applyProperty(const QVariant& value);
    void updateMap();
    bool isValidProperty(const QString& property) const;
    bool isValidValue(const QString& property, const QVariant& value) const;

private:
    Tile* tile_;
    Map* map_;
    QString property_;
    QVariant oldValue_;
    QVariant newValue_;
    
    static const int COMMAND_ID = 1001;
};

/**
 * @brief Multiple Tile Properties Change Command
 */
class SetTilePropertiesCommand : public QUndoCommand
{
public:
    explicit SetTilePropertiesCommand(Tile* tile, const QMap<QString, QVariant>& newValues,
                                     const QMap<QString, QVariant>& oldValues,
                                     Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTilePropertiesCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    QMap<QString, QVariant> getOldValues() const { return oldValues_; }
    QMap<QString, QVariant> getNewValues() const { return newValues_; }

private:
    void applyProperties(const QMap<QString, QVariant>& values);
    void updateMap();
    bool validateProperties(const QMap<QString, QVariant>& values) const;

private:
    Tile* tile_;
    Map* map_;
    QMap<QString, QVariant> oldValues_;
    QMap<QString, QVariant> newValues_;
    
    static const int COMMAND_ID = 1002;
};

/**
 * @brief Batch Tile Properties Change Command (Multiple Tiles)
 */
class SetMultipleTilePropertiesCommand : public QUndoCommand
{
public:
    struct TilePropertyChange {
        Tile* tile;
        QString property;
        QVariant oldValue;
        QVariant newValue;
        
        TilePropertyChange(Tile* t = nullptr, const QString& p = QString(),
                          const QVariant& ov = QVariant(), const QVariant& nv = QVariant())
            : tile(t), property(p), oldValue(ov), newValue(nv) {}
    };

    explicit SetMultipleTilePropertiesCommand(const QList<TilePropertyChange>& changes,
                                             Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetMultipleTilePropertiesCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QList<TilePropertyChange> getChanges() const { return changes_; }
    int getChangeCount() const { return changes_.size(); }

private:
    void applyChanges(bool forward);
    void updateMap();
    bool validateChanges(const QList<TilePropertyChange>& changes) const;

private:
    QList<TilePropertyChange> changes_;
    Map* map_;
    
    static const int COMMAND_ID = 1003;
};

/**
 * @brief Tile Map Flag Change Command
 */
class SetTileMapFlagCommand : public QUndoCommand
{
public:
    explicit SetTileMapFlagCommand(Tile* tile, quint32 flag, bool enabled,
                                  Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTileMapFlagCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    quint32 getFlag() const { return flag_; }
    bool isEnabled() const { return enabled_; }

private:
    void applyFlag(bool enable);
    void updateMap();

private:
    Tile* tile_;
    Map* map_;
    quint32 flag_;
    bool enabled_;
    bool wasEnabled_;
    
    static const int COMMAND_ID = 1004;
};

/**
 * @brief Tile State Flag Change Command
 */
class SetTileStateFlagCommand : public QUndoCommand
{
public:
    explicit SetTileStateFlagCommand(Tile* tile, quint32 flag, bool enabled,
                                    Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTileStateFlagCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    quint32 getFlag() const { return flag_; }
    bool isEnabled() const { return enabled_; }

private:
    void applyFlag(bool enable);
    void updateMap();

private:
    Tile* tile_;
    Map* map_;
    quint32 flag_;
    bool enabled_;
    bool wasEnabled_;
    
    static const int COMMAND_ID = 1005;
};

/**
 * @brief Tile House ID Change Command
 */
class SetTileHouseIdCommand : public QUndoCommand
{
public:
    explicit SetTileHouseIdCommand(Tile* tile, quint32 newHouseId, quint32 oldHouseId,
                                  Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTileHouseIdCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    quint32 getOldHouseId() const { return oldHouseId_; }
    quint32 getNewHouseId() const { return newHouseId_; }

private:
    void applyHouseId(quint32 houseId);
    void updateMap();
    bool isValidHouseId(quint32 houseId) const;

private:
    Tile* tile_;
    Map* map_;
    quint32 oldHouseId_;
    quint32 newHouseId_;
    
    static const int COMMAND_ID = 1006;
};

/**
 * @brief Tile Zone ID Change Command
 */
class SetTileZoneIdCommand : public QUndoCommand
{
public:
    explicit SetTileZoneIdCommand(Tile* tile, const QList<quint16>& newZoneIds,
                                 const QList<quint16>& oldZoneIds,
                                 Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTileZoneIdCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    QList<quint16> getOldZoneIds() const { return oldZoneIds_; }
    QList<quint16> getNewZoneIds() const { return newZoneIds_; }

private:
    void applyZoneIds(const QList<quint16>& zoneIds);
    void updateMap();
    bool isValidZoneId(quint16 zoneId) const;

private:
    Tile* tile_;
    Map* map_;
    QList<quint16> oldZoneIds_;
    QList<quint16> newZoneIds_;
    
    static const int COMMAND_ID = 1007;
};

/**
 * @brief Tile Minimap Color Change Command
 */
class SetTileMinimapColorCommand : public QUndoCommand
{
public:
    explicit SetTileMinimapColorCommand(Tile* tile, quint8 newColor, quint8 oldColor,
                                       Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~SetTileMinimapColorCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Tile* getTile() const { return tile_; }
    quint8 getOldColor() const { return oldColor_; }
    quint8 getNewColor() const { return newColor_; }

private:
    void applyColor(quint8 color);
    void updateMap();
    bool isValidColor(quint8 color) const;

private:
    Tile* tile_;
    Map* map_;
    quint8 oldColor_;
    quint8 newColor_;
    
    static const int COMMAND_ID = 1008;
};

/**
 * @brief Tile Property Command Factory
 */
class TilePropertyCommandFactory
{
public:
    // Single property commands
    static SetTilePropertyCommand* createPropertyCommand(Tile* tile, const QString& property,
                                                        const QVariant& newValue, const QVariant& oldValue,
                                                        Map* map = nullptr);
    
    // Multiple properties command
    static SetTilePropertiesCommand* createPropertiesCommand(Tile* tile,
                                                            const QMap<QString, QVariant>& newValues,
                                                            const QMap<QString, QVariant>& oldValues,
                                                            Map* map = nullptr);
    
    // Batch command
    static SetMultipleTilePropertiesCommand* createBatchCommand(const QList<SetMultipleTilePropertiesCommand::TilePropertyChange>& changes,
                                                               Map* map = nullptr);
    
    // Specific property commands
    static SetTileMapFlagCommand* createMapFlagCommand(Tile* tile, quint32 flag, bool enabled, Map* map = nullptr);
    static SetTileStateFlagCommand* createStateFlagCommand(Tile* tile, quint32 flag, bool enabled, Map* map = nullptr);
    static SetTileHouseIdCommand* createHouseIdCommand(Tile* tile, quint32 newHouseId, quint32 oldHouseId, Map* map = nullptr);
    static SetTileZoneIdCommand* createZoneIdCommand(Tile* tile, const QList<quint16>& newZoneIds, const QList<quint16>& oldZoneIds, Map* map = nullptr);
    static SetTileMinimapColorCommand* createMinimapColorCommand(Tile* tile, quint8 newColor, quint8 oldColor, Map* map = nullptr);

private:
    TilePropertyCommandFactory() = default;
};

#endif // TILEPROPERTYCOMMAND_H
