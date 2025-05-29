#ifndef DOODADBRUSH_H
#define DOODADBRUSH_H

#include "Brush.h"

class DoodadBrush : public Brush {
    Q_OBJECT

public:
    explicit DoodadBrush(QObject *parent = nullptr);

    void mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;

    QString name() const override;
    bool isDoodad() const override;
    // DoodadBrush might override canDrag() to return true if it places a single doodad
    // and dragging could mean re-positioning before final placement, or selecting an area for multiple.
    // For now, default is fine.
};

#endif // DOODADBRUSH_H
