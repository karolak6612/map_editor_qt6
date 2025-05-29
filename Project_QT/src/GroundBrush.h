#ifndef GROUNDBRUSH_H
#define GROUNDBRUSH_H

#include "Brush.h"

class GroundBrush : public Brush {
    Q_OBJECT 

public:
    explicit GroundBrush(QObject *parent = nullptr);

    void mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;
    void mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) override;

    QString name() const override;
    bool isGround() const override;
    // Add other GroundBrush specific overrides or methods here later
};

#endif // GROUNDBRUSH_H
