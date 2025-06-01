#ifndef RAWBRUSH_H
#define RAWBRUSH_H

#include "Brush.h"

class RAWBrush : public Brush {
    Q_OBJECT

public:
    explicit RAWBrush(QObject *parent = nullptr);

    // Brush type identification
    Type type() const override;
    QString name() const override;
    bool isRaw() const override;
    RAWBrush* asRaw() override;
    const RAWBrush* asRaw() const override;

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

    // RAWBrush specific methods
    void setItemId(quint16 itemId);
    quint16 getItemId() const;

private:
    quint16 itemId_ = 0;
    int brushSize_ = 1;
    BrushShape brushShape_ = BrushShape::Square;
};

#endif // RAWBRUSH_H
