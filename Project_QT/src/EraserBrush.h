#ifndef ERASERBRUSH_H
#define ERASERBRUSH_H

#include "Brush.h"

class EraserBrush : public Brush {
    Q_OBJECT

public:
    explicit EraserBrush(QObject *parent = nullptr);

    void mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;

    QString name() const override;
    bool isEraser() const override;
    // EraserBrush might also override canSmear() to return true by default
    // bool canSmear() const override; 
};

#endif // ERASERBRUSH_H
