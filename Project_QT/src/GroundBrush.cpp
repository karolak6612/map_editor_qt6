#include "GroundBrush.h"
#include <QDebug>
#include <QMouseEvent> // For event->button()

GroundBrush::GroundBrush(QObject *parent) : Brush(parent) {
}

void GroundBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mousePressEvent at" << mapPos << "button:" << event->button();
    // Placeholder for actual ground drawing logic
}

void GroundBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    // Only log if buttons are pressed, similar to original smearing behavior
    if (event->buttons() != Qt::NoButton) {
        qDebug() << name() << "::mouseMoveEvent at" << mapPos;
        // Placeholder for continuous drawing
    }
}

void GroundBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mouseReleaseEvent at" << mapPos << "button:" << event->button();
    // Placeholder for finalizing drawing
}

QString GroundBrush::name() const {
    return QStringLiteral("Ground Brush");
}

bool GroundBrush::isGround() const {
    return true;
}
