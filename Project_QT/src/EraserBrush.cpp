#include "EraserBrush.h"
#include <QDebug>
#include <QMouseEvent> // For event->button()

EraserBrush::EraserBrush(QObject *parent) : Brush(parent) {
}

void EraserBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mousePressEvent at" << mapPos << "button:" << event->button();
    // Placeholder for actual erasing logic
}

void EraserBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    if (event->buttons() != Qt::NoButton) {
        qDebug() << name() << "::mouseMoveEvent at" << mapPos;
        // Placeholder for continuous erasing
    }
}

void EraserBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mouseReleaseEvent at" << mapPos << "button:" << event->button();
    // Placeholder for finalizing erasing
}

QString EraserBrush::name() const {
    return QStringLiteral("Eraser Brush");
}

bool EraserBrush::isEraser() const {
    return true;
}

// Example of overriding canSmear if Eraser should always smear
// bool EraserBrush::canSmear() const {
//     return true; 
// }
