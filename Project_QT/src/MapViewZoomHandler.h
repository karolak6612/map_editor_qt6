#ifndef MAPVIEWZOOMHANDLER_H
#define MAPVIEWZOOMHANDLER_H

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QWheelEvent>

// Forward declarations
class MapView;
class QGraphicsView;

/**
 * @brief MapViewZoomHandler - Helper class for handling MapView zoom operations
 * 
 * This class extracts all zoom-related logic from MapView.cpp to comply with mandate M6.
 * It handles zoom in/out, zoom to level, zoom to fit, wheel zoom, and zoom constraints.
 * 
 * Task 011: Refactor large source files - Extract zoom logic from MapView.cpp
 */
class MapViewZoomHandler : public QObject
{
    Q_OBJECT

public:
    // Zoom constants
    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 10.0;
    static constexpr double DEFAULT_ZOOM = 1.0;
    static constexpr double ZOOM_IN_FACTOR = 1.25;
    static constexpr double ZOOM_OUT_FACTOR = 0.8;

    explicit MapViewZoomHandler(MapView* mapView, QObject *parent = nullptr);
    ~MapViewZoomHandler() override;

    // Core zoom operations
    void zoom(qreal factor, const QPointF& centerScreenPos);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void zoomToLevel(double level, const QPointF& centerPoint = QPointF());
    void zoomToFit(const QRectF& rect);

    // Wheel zoom handling
    void handleWheelZoom(QWheelEvent* event);

    // Zoom state management
    double getCurrentZoom() const { return currentZoom_; }
    void setCurrentZoom(double zoom);
    bool isZoomAtMinimum() const { return currentZoom_ <= MIN_ZOOM; }
    bool isZoomAtMaximum() const { return currentZoom_ >= MAX_ZOOM; }

    // Zoom constraints
    double constrainZoom(double zoom) const;
    void setZoomConstraints(double minZoom, double maxZoom);

    // Center point management
    void centerOnMap();
    void centerOnPoint(const QPointF& point);

    // Zoom animation support (for future enhancement)
    void setAnimatedZoom(bool enabled) { animatedZoom_ = enabled; }
    bool isAnimatedZoom() const { return animatedZoom_; }

signals:
    void zoomChanged(double newZoom);
    void zoomLevelChanged(double level);
    void zoomConstraintsChanged(double minZoom, double maxZoom);

private slots:
    void onZoomAnimationFinished();

private:
    MapView* mapView_;
    double currentZoom_;
    double minZoom_;
    double maxZoom_;
    bool animatedZoom_;

    // Helper methods
    void applyZoomToView(double scaleFactor, const QPointF& centerScreenPos);
    void updateScrollBarsAfterZoom(const QPointF& centerScreenPos, const QPointF& sceneCenterPos);
    QPointF getViewportCenter() const;
    void emitZoomChanged();

    // Wheel zoom settings
    double wheelZoomFactor_;
    bool invertWheelZoom_;
};

#endif // MAPVIEWZOOMHANDLER_H
