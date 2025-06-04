#include "Sprite.h"
#include <QPainter>
#include <QDebug>

bool Sprite::validateDrawingParameters(QPainter* painter, const QPoint& targetPos) const {
    if (!painter) {
        qWarning() << "Sprite::validateDrawingParameters: Null painter provided";
        return false;
    }

    if (!painter->isActive()) {
        qWarning() << "Sprite::validateDrawingParameters: Painter is not active";
        return false;
    }

    // Check if target position is reasonable (not extremely negative)
    if (targetPos.x() < -10000 || targetPos.y() < -10000) {
        qWarning() << "Sprite::validateDrawingParameters: Target position is unreasonable:" << targetPos;
        return false;
    }

    return true;
}

QRect Sprite::calculateSourceRect(int sourceX, int sourceY, int sourceWidth, int sourceHeight, int defaultWidth, int defaultHeight) const {
    // Use provided values or defaults
    int finalWidth = (sourceWidth > 0) ? sourceWidth : defaultWidth;
    int finalHeight = (sourceHeight > 0) ? sourceHeight : defaultHeight;

    // Ensure non-negative coordinates
    int finalX = qMax(0, sourceX);
    int finalY = qMax(0, sourceY);

    // Ensure positive dimensions
    finalWidth = qMax(1, finalWidth);
    finalHeight = qMax(1, finalHeight);

    return QRect(finalX, finalY, finalWidth, finalHeight);
}


