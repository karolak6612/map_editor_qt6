#ifndef DOODADBRUSH_H
#define DOODADBRUSH_H

#include "Brush.h"

class DoodadBrush : public Brush {
    Q_OBJECT

public:
    explicit DoodadBrush(QObject *parent = nullptr);

    // Task 020: Implement pure virtual methods from Brush.h with correct signatures
    // Pure virtual mouse events (complex signatures)
    QUndoCommand* mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView, Map* map, QUndoStack* undoStack, bool shiftPressed, bool ctrlPressed, bool altPressed, QUndoCommand* parentCommand = nullptr) override;

    // Pure virtual brush properties
    Type type() const override;
    void cancel() override;
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;
    QString name() const override;
    int getLookID() const override;

    // Pure virtual core brush actions
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Type identification
    bool isDoodad() const override;
};

#endif // DOODADBRUSH_H
