#ifndef DOORBRUSH_H
#define DOORBRUSH_H

#include "Brush.h"

// Door types matching wxWidgets implementation
enum class DoorType {
    Normal,
    Locked,
    Magic,
    Quest,
    Hatch,
    Archway,
    NormalAlt,
    Window
};

class DoorBrush : public Brush {
    Q_OBJECT

public:
    explicit DoorBrush(DoorType doorType, QObject *parent = nullptr);

    // Brush type identification
    Type type() const override;
    QString name() const override;
    bool isDoor() const override;
    DoorBrush* asDoor() override;
    const DoorBrush* asDoor() const override;

    // Brush properties
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;
    bool canDrag() const override;
    bool canSmear() const override;
    bool oneSizeFitsAll() const override;
    int getLookID() const override;

    // Core brush action interface
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Mouse event handlers with proper signatures
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

    // DoorBrush specific methods
    DoorType getDoorType() const;
    void setDoorType(DoorType doorType);
    quint16 getDoorItemId() const;

    // Static utility method (matching wxWidgets)
    static void switchDoor(void* door); // TODO: Replace void* with proper Item* when available

private:
    DoorType doorType_;
    int brushSize_ = 1;
    BrushShape brushShape_ = BrushShape::Square;
};

#endif // DOORBRUSH_H
