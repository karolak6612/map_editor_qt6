#ifndef WALLBRUSH_H
#define WALLBRUSH_H

#include "TerrainBrush.h"
#include "BrushCommon.h" // For WallAlignment, DoorTypeQt
#include <QMap>
#include <QVector>
#include <QString>

// Forward declarations (Map, QUndoCommand, etc. are in Brush.h/TerrainBrush.h's scope for virtual methods)

// Helper structs for configuration (can be defined here or in BrushCommon.h if widely used)
struct WallItemConfig {
    quint16 itemId = 0;
    int chance = 100;
};

struct WallDoorConfig {
    DoorTypeQt doorType = DoorTypeQt::Undefined;
    quint16 itemId = 0;
    bool isLocked = false;
};


class WallBrush : public TerrainBrush {
    Q_OBJECT
public:
    explicit WallBrush(QObject* parent = nullptr);
    ~WallBrush() override;

    // --- Overrides from Brush/TerrainBrush ---
    // Name can be set via TerrainBrush::setSpecificName in constructor
    Type type() const override;
    bool isWall() const override; // Convenience, already in Brush.h (will return true)

    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    int getBrushSize() const override;
    BrushShape getBrushShape() const override;

    QUndoCommand* mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                  Map* map, QUndoStack* undoStack,
                                  bool shiftPressed, bool ctrlPressed, bool altPressed,
                                  QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                 Map* map, QUndoStack* undoStack,
                                 bool shiftPressed, bool ctrlPressed, bool altPressed,
                                 QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                    Map* map, QUndoStack* undoStack,
                                    bool shiftPressed, bool ctrlPressed, bool altPressed,
                                    QUndoCommand* parentCommand = nullptr) override;
    void cancel() override;

    bool canSmear() const override; // wxWallBrush returned false

    // --- WallBrush specific methods ---
    void setCurrentWallItemId(quint16 itemId);
    quint16 getCurrentWallItemId() const;

    // Methods for more detailed configuration (implementation deferred)
    // void addWallItemConfig(WallAlignment alignment, quint16 itemId, int chance = 100);
    // void addDoorConfig(WallAlignment alignment, DoorTypeQt doorType, quint16 itemId, bool isLocked = false);
    // const QVector<WallItemConfig>* getWallItemsFor(WallAlignment alignment) const;
    // const QVector<WallDoorConfig>* getDoorItemsFor(WallAlignment alignment) const;

private:
    quint16 currentWallItemId_ = 0; // Primary item ID to place for this wall type

    // Complex configuration deferred to later tasks / XML loading
    // QMap<WallAlignment, QVector<WallItemConfig>> wallItemConfigs_;
    // QMap<WallAlignment, QVector<WallDoorConfig>> doorItemConfigs_;
    // WallBrush* redirectTo_ = nullptr;
};

#endif // WALLBRUSH_H
