#ifndef HOUSEBRUSH_H
#define HOUSEBRUSH_H

#include "Brush.h"
#include <QString>

// Forward declarations
class House;
class Map;
class Tile;

class HouseBrush : public Brush {
    Q_OBJECT

public:
    explicit HouseBrush(QObject* parent = nullptr);
    ~HouseBrush() override;

    // Brush interface implementation
    Type type() const override;
    QString name() const override;
    void setName(const QString& newName) override;
    int getLookID() const override;

    // Type identification
    bool isHouse() const override;
    HouseBrush* asHouse() override;
    const HouseBrush* asHouse() const override;

    // Core brush actions
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // House-specific properties
    House* getHouse() const;
    void setHouse(House* house);
    quint32 getHouseId() const;
    void setHouseId(quint32 houseId);

    // Brush configuration
    bool getRemoveItems() const;
    void setRemoveItems(bool remove);
    bool getAutoAssignDoorId() const;
    void setAutoAssignDoorId(bool autoAssign);

    // Drawing options
    bool canDrag() const override;
    bool canSmear() const override;
    bool oneSizeFitsAll() const override;

    // Brush loading
    bool load(const QDomElement& element, QStringList& warnings) override;

    // Task 73: Enhanced house operations
    void applyToArea(Map* map, const QRect& area, int floor = -1);
    void removeFromArea(Map* map, const QRect& area, int floor = -1);
    void applyToSelection(Map* map, const QList<QPointF>& positions);
    void removeFromSelection(Map* map, const QList<QPointF>& positions);

    // Task 73: Cleanup operations
    void cleanupHouseTiles(Map* map);
    void validateHouseTiles(Map* map);
    void updateHouseReferences(Map* map);

    // Task 73: Door management
    void assignDoorIDs(Map* map, const QList<QPointF>& positions);
    void clearDoorIDs(Map* map, const QList<QPointF>& positions);
    quint8 getNextAvailableDoorID(Map* map) const;

    // Task 73: House utilities
    QList<QPointF> getHouseTiles(Map* map) const;
    int getHouseTileCount(Map* map) const;
    QRectF getHouseBounds(Map* map) const;
    bool isHouseConnected(Map* map) const;

    // Task 73: Validation
    bool isValidHouseTile(Map* map, const QPointF& tilePos) const;
    bool hasBlockingItems(Map* map, const QPointF& tilePos) const;
    QStringList getValidationErrors(Map* map) const;

signals:
    void houseChanged(House* house);

    // Task 73: Enhanced signals
    void tileAdded(const QPointF& position);
    void tileRemoved(const QPointF& position);
    void doorAssigned(const QPointF& position, quint8 doorId);
    void houseValidated(bool isValid);
    void cleanupCompleted();

private:
    QString name_;
    House* currentHouse_;
    quint32 currentHouseId_;
    bool removeItems_;
    bool autoAssignDoorId_;

    // Helper methods
    void updateTileHouseProperties(Map* map, Tile* tile, bool adding) const;
    void removeLooseItems(Tile* tile) const;
    void assignDoorIds(Map* map, Tile* tile) const;
    void removeDoorIds(Tile* tile) const;
};

// Undo command for house brush operations
class HouseBrushCommand : public QUndoCommand {
public:
    HouseBrushCommand(Map* map, const QPointF& tilePos, House* house, bool isAdding, QUndoCommand* parent = nullptr);
    ~HouseBrushCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    House* house_;
    quint32 houseId_;
    bool isAdding_;
    bool hadHouse_;
    quint32 previousHouseId_;
    
    // Store removed items for undo
    QList<QPair<quint16, QVariant>> removedItems_;
    
    void executeOperation(bool adding);
    void storeRemovedItems(Tile* tile);
    void restoreRemovedItems(Tile* tile);
};

// Task 73: House management utility class
class HouseManager : public QObject {
    Q_OBJECT

public:
    explicit HouseManager(QObject* parent = nullptr);
    ~HouseManager() override;

    // Task 73: House operations
    static void cleanupHouse(Map* map, House* house);
    static void validateHouse(Map* map, House* house);
    static void removeInvalidHouses(Map* map);
    static void updateAllHouseReferences(Map* map);

    // Task 73: Batch operations
    static void cleanupAllHouses(Map* map);
    static void validateAllHouses(Map* map);
    static void assignAllDoorIDs(Map* map);
    static void clearAllDoorIDs(Map* map);

    // Task 73: House utilities
    static QList<House*> getHousesInArea(Map* map, const QRect& area);
    static QList<House*> getInvalidHouses(Map* map);
    static QList<House*> getDisconnectedHouses(Map* map);
    static House* getHouseAt(Map* map, const QPointF& position);

    // Task 73: Statistics
    static int getTotalHouseTiles(Map* map);
    static int getTotalHouseDoors(Map* map);
    static QMap<quint32, int> getHouseSizeStatistics(Map* map);
    static QStringList getHouseValidationReport(Map* map);

signals:
    void houseCleanupCompleted(House* house);
    void houseValidationCompleted(House* house, bool isValid);
    void batchOperationCompleted(const QString& operation, int housesProcessed);
};

#endif // HOUSEBRUSH_H
