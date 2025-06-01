#ifndef ERASERBRUSH_H
#define ERASERBRUSH_H

#include "Brush.h"

class EraserBrush : public Brush {
    Q_OBJECT

public:
    explicit EraserBrush(QObject *parent = nullptr);

    // Brush type identification
    Type type() const override;
    QString name() const override;
    bool isEraser() const override;
    EraserBrush* asEraser() override;
    const EraserBrush* asEraser() const override;

    // Brush properties
    int getBrushSize() const override;
    BrushShape getBrushShape() const override;
    bool needBorders() const override;
    bool canDrag() const override;
    bool canSmear() const override;
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

private:
    int brushSize_ = 1;
    BrushShape brushShape_ = BrushShape::Square;
};

#endif // ERASERBRUSH_H
