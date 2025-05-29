#include "DoodadBrush.h"
#include <QDebug>
#include <QMouseEvent> // For event->button()

DoodadBrush::DoodadBrush(QObject *parent) : Brush(parent) {
}

void DoodadBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mousePressEvent at" << mapPos << "button:" << event->button();
    // Placeholder for actual doodad placement logic
    // Typically, a doodad is placed on press, or a preview starts.
}

void DoodadBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    // Doodad brushes typically don't do anything on mouse move unless they are drag-placing
    // or showing a preview that follows the mouse.
    if (event->buttons() != Qt::NoButton) { // Only if a button is held (e.g. dragging a preview)
        qDebug() << name() << "::mouseMoveEvent at" << mapPos;
    }
}

void DoodadBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView) {
    qDebug() << name() << "::mouseReleaseEvent at" << mapPos << "button:" << event->button();
    // Placeholder for finalizing doodad placement
}

QString DoodadBrush::name() const {
    return QStringLiteral("Doodad Brush");
}

bool DoodadBrush::isDoodad() const {
    return true;
}
