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

    // Mouse interaction methods
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
