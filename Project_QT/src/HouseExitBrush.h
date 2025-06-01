#ifndef HOUSEEXITBRUSH_H
#define HOUSEEXITBRUSH_H

#include "Brush.h"
#include <QString>

// Forward declarations
class House;
class Map;
class Tile;

class HouseExitBrush : public Brush {
    Q_OBJECT

public:
    explicit HouseExitBrush(QObject* parent = nullptr);
    ~HouseExitBrush() override;

    // Brush interface implementation
    Type type() const override;
    QString name() const override;
    void setName(const QString& newName) override;
    int getLookID() const override;

    // Type identification
    bool isHouseExit() const override;
    HouseExitBrush* asHouseExit() override;
    const HouseExitBrush* asHouseExit() const override;

    // Core brush actions
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // House exit specific properties
    House* getHouse() const;
    void setHouse(House* house);
    quint32 getHouseId() const;
    void setHouseId(quint32 houseId);

    // Exit configuration
    bool getAutoSetAsEntry() const;
    void setAutoSetAsEntry(bool autoSet);
    bool getMarkWithItem() const;
    void setMarkWithItem(bool mark);
    quint16 getMarkerItemId() const;
    void setMarkerItemId(quint16 itemId);

    // Drawing options
    bool canDrag() const override;
    bool canSmear() const override;
    bool oneSizeFitsAll() const override;

    // Brush loading
    bool load(const QDomElement& element, QStringList& warnings) override;

signals:
    void houseChanged(House* house);
    void exitConfigChanged();

private:
    QString name_;
    House* currentHouse_;
    quint32 currentHouseId_;
    bool autoSetAsEntry_;
    bool markWithItem_;
    quint16 markerItemId_;

    // Helper methods
    void setHouseExit(Map* map, Tile* tile, House* house) const;
    void removeHouseExit(Map* map, Tile* tile) const;
    void addExitMarker(Tile* tile) const;
    void removeExitMarker(Tile* tile) const;
    bool isValidExitPosition(Map* map, const QPointF& tilePos) const;
};

// Undo command for house exit brush operations
class HouseExitBrushCommand : public QUndoCommand {
public:
    HouseExitBrushCommand(Map* map, const QPointF& tilePos, House* house, bool isAdding, QUndoCommand* parent = nullptr);
    ~HouseExitBrushCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    House* house_;
    quint32 houseId_;
    bool isAdding_;
    bool hadExit_;
    quint32 previousHouseId_;
    MapPos previousExitPos_;
    bool hadMarkerItem_;
    quint16 markerItemId_;
    
    void executeOperation(bool adding);
    void storeCurrentState();
    void restorePreviousState();
};

#endif // HOUSEEXITBRUSH_H
