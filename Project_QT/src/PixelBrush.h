#ifndef PIXELBRUSH_H
#define PIXELBRUSH_H

#include "Brush.h" // Base class
#include <QColor>

class PixelBrush : public Brush {
    Q_OBJECT
public:
    explicit PixelBrush(QObject *parent = nullptr);
    ~PixelBrush() override;

    QString name() const override;
    int getBrushSize() const override;      // From Brush base
    BrushShape getBrushShape() const override; // From Brush base

    // New methods from Brush interface
    Type type() const override;
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Mouse interaction methods (signatures should already match Brush.h)
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
    QColor drawColor_ = Qt::red; // Color this brush applies
};

#endif // PIXELBRUSH_H
