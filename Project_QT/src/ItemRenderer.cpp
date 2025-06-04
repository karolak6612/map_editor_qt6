#include "ItemRenderer.h"
#include "Item.h"
#include "DrawingOptions.h"
#include "SpriteManager.h"
#include "GameSprite.h"
#include "ItemManager.h"
#include <QPainter>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QtMath>

ItemRenderer::ItemRenderer(QObject *parent)
    : QObject(parent)
{
}

ItemRenderer::~ItemRenderer()
{
}

// Main rendering methods
void ItemRenderer::draw(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item) return;

    painter->save();

    // Task 54: Full sprite integration for production-quality rendering
    if (options.useSprites && item->getClientId() > 0) {
        drawWithSprites(item, painter, targetRect, options);
    } else {
        // Fallback to placeholder rendering
        drawPlaceholder(item, painter, targetRect, options);
    }

    // Task 76: Draw special item flags
    drawSpecialFlags(item, painter, targetRect, options);

    // Draw debug information if enabled
    if (options.drawDebugInfo) {
        drawDebugInfo(item, painter, targetRect, options);
    }

    // Draw text overlay (count for stackable items)
    QMap<QString, QVariant> textOptions;
    drawText(item, painter, targetRect, textOptions);

    painter->restore();
}

void ItemRenderer::drawText(const Item* item, QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options)
{
    Q_UNUSED(options)
    
    if (painter && item && item->isStackable() && item->getCount() > 1) {
        QString countStr = QString::number(item->getCount());
        painter->save();
        QFont font = painter->font();
        font.setPointSize(font.pointSize() - 2 > 0 ? font.pointSize() - 2 : 6);
        painter->setFont(font);
        painter->setPen(Qt::red); 
        QRectF textRect = painter->fontMetrics().boundingRect(countStr);
        textRect.moveBottomRight(targetRect.bottomRight() - QPointF(1,1));
        painter->drawText(textRect, countStr);
        painter->restore();
    }
}

// Sprite rendering methods
void ItemRenderer::drawWithSprites(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item || item->getClientId() == 0) {
        return;
    }

    // Get SpriteManager instance
    SpriteManager* spriteManager = SpriteManager::getInstance();
    if (!spriteManager) {
        qWarning() << "ItemRenderer::drawWithSprites: SpriteManager not available";
        drawPlaceholder(item, painter, targetRect, options);
        return;
    }

    // Get GameSprite data for this item
    QSharedPointer<const GameSpriteData> spriteData = spriteManager->getGameSpriteData(item->getClientId());
    if (!spriteData) {
        qWarning() << "ItemRenderer::drawWithSprites: No sprite data for client ID" << item->getClientId();
        drawPlaceholder(item, painter, targetRect, options);
        return;
    }

    // Apply item opacity
    qreal opacity = painter->opacity();
    applyItemOpacity(painter, item, options);

    // Calculate animation frame
    int frame = 0;
    if (spriteData->animationPhases > 1) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        frame = (currentTime / 100) % spriteData->animationPhases; // 100ms per frame
    }

    // Draw all layers
    for (int layer = 0; layer < spriteData->layers; ++layer) {
        drawSpriteLayer(item, painter, targetRect, spriteData, frame, 0, 0, 0, layer, options);
    }

    // Restore opacity
    painter->setOpacity(opacity);
}

void ItemRenderer::drawSpriteLayer(const Item* item, QPainter* painter, const QRectF& targetRect,
                                  QSharedPointer<const GameSpriteData> spriteData,
                                  int frame, int patternX, int patternY, int patternZ,
                                  int layer, const DrawingOptions& options)
{
    if (!painter || !spriteData || !item) {
        return;
    }

    // Get frame image from sprite data
    QImage frameImage = spriteData->getFrameImage(frame, patternX, patternY, patternZ, layer);
    if (frameImage.isNull()) {
        return;
    }

    // Handle multi-tile sprites
    if (spriteData->width > 1 || spriteData->height > 1) {
        drawMultiTileSprite(item, painter, targetRect, frameImage, spriteData, options);
    } else {
        // Single tile sprite - direct draw
        QRectF drawRect = calculateDrawRect(targetRect, frameImage, options);
        painter->drawImage(drawRect, frameImage);
    }
}

void ItemRenderer::drawMultiTileSprite(const Item* item, QPainter* painter, const QRectF& baseRect,
                                      const QImage& frameImage,
                                      QSharedPointer<const GameSpriteData> spriteData,
                                      const DrawingOptions& options)
{
    Q_UNUSED(item)
    Q_UNUSED(options)

    if (!painter || !spriteData || frameImage.isNull()) {
        return;
    }

    // Calculate the total size of the multi-tile sprite
    int totalWidth = spriteData->width * 32;  // 32 pixels per tile
    int totalHeight = spriteData->height * 32;

    // Calculate the position to draw the sprite
    // Multi-tile sprites are typically anchored at their bottom-right tile
    QRectF drawRect = baseRect;
    drawRect.setWidth(totalWidth);
    drawRect.setHeight(totalHeight);
    drawRect.moveBottomRight(baseRect.bottomRight());

    painter->drawImage(drawRect, frameImage);
}

// Placeholder rendering methods
void ItemRenderer::drawPlaceholder(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item) return;

    painter->save();

    // Generate color based on item type and ID for visual distinction
    QColor itemColor = generateItemColor(item);
    painter->fillRect(targetRect, itemColor);

    // Draw border
    painter->setPen(QPen(itemColor.darker(150), 1));
    painter->drawRect(targetRect);

    // Draw item ID text
    drawItemIdText(item, painter, targetRect, options);

    // Draw type indicator
    if (options.showItemTypes) {
        drawTypeIndicator(item, painter, targetRect, options);
    }

    painter->restore();
}

void ItemRenderer::drawItemIdText(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (!painter || !item) return;

    QString idText = QString::number(item->getServerId());
    QFont font = painter->font();
    font.setPointSize(qMax(6, font.pointSize() - 1));
    painter->setFont(font);
    painter->setPen(Qt::black);

    QRectF textRect = targetRect.adjusted(2, 2, -2, -2);
    painter->drawText(textRect, Qt::AlignCenter, idText);
}

void ItemRenderer::drawTypeIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (!painter || !item) return;

    QString typeIndicator = getItemTypeIndicator(item);
    if (typeIndicator.isEmpty()) return;

    QFont font = painter->font();
    font.setPointSize(qMax(6, font.pointSize() - 2));
    painter->setFont(font);
    painter->setPen(Qt::white);

    QRectF textRect = targetRect.adjusted(1, 1, -1, -1);
    painter->drawText(textRect, Qt::AlignTop | Qt::AlignLeft, typeIndicator);
}

// Debug rendering methods
void ItemRenderer::drawDebugInfo(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item || !options.drawDebugInfo) return;

    painter->save();

    QFont debugFont = painter->font();
    debugFont.setPointSize(qMax(6, debugFont.pointSize() - 2));
    painter->setFont(debugFont);
    painter->setPen(Qt::yellow);

    QString debugText = QString("ID:%1\nCID:%2\nPos:%3")
                       .arg(item->getServerId())
                       .arg(item->getClientId())
                       .arg(item->getStackPos());

    QRectF textRect = targetRect.adjusted(2, 2, -2, -2);
    painter->drawText(textRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, debugText);

    painter->restore();
}

// Special flag rendering methods
void ItemRenderer::drawSpecialFlags(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item) return;

    // Draw selection highlight if item is selected
    if (item->isSelected() && options.highlightSelectedTile) {
        drawSelectionHighlight(item, painter, targetRect, options);
    }

    // Draw locked door highlight
    if (options.showLockedDoors && item->hasAttribute(Item::AttrDoorLocked) &&
        item->getAttribute(Item::AttrDoorLocked).toBool()) {
        drawLockedDoorHighlight(item, painter, targetRect, options);
    }

    // Draw wall hook indicator
    if (options.showWallHooks && (item->hasHookSouth() || item->hasHookEast())) {
        drawWallHookIndicator(item, painter, targetRect, options);
    }

    // Draw blocking indicator
    if (options.showBlocking && item->isBlocking()) {
        drawBlockingIndicator(item, painter, targetRect, options);
    }
}

void ItemRenderer::drawSelectionHighlight(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item) return;

    painter->save();

    // Draw animated selection highlight
    QColor selectionColor = getSelectionColor(item);

    // Add animation effect
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double alpha = 0.3 + 0.2 * qSin(currentTime * 0.005); // Pulsing effect
    selectionColor.setAlphaF(alpha);

    painter->fillRect(targetRect, selectionColor);

    // Draw selection border
    painter->setPen(QPen(selectionColor.darker(150), 2));
    painter->drawRect(targetRect);

    painter->restore();
}

void ItemRenderer::drawLockedDoorHighlight(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (!painter || !item) return;

    painter->save();

    // Draw red highlight for locked doors
    QColor lockedColor(255, 0, 0, 100);
    painter->fillRect(targetRect, lockedColor);

    // Draw lock icon
    QPen lockPen(Qt::red, 2);
    painter->setPen(lockPen);

    QRectF lockRect = targetRect.adjusted(targetRect.width() * 0.6, targetRect.height() * 0.1,
                                         -targetRect.width() * 0.1, -targetRect.height() * 0.6);

    // Draw simple lock shape
    painter->drawRect(lockRect);
    QRectF keyhole = lockRect.adjusted(lockRect.width() * 0.3, lockRect.height() * 0.4,
                                      -lockRect.width() * 0.3, -lockRect.height() * 0.2);
    painter->fillRect(keyhole, Qt::red);

    painter->restore();
}

void ItemRenderer::drawWallHookIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (!painter || !item) return;

    painter->save();

    // Draw wall hook indicator
    QPen hookPen(Qt::blue, 2);
    painter->setPen(hookPen);

    QRectF hookRect = targetRect.adjusted(targetRect.width() * 0.7, targetRect.height() * 0.7,
                                         -targetRect.width() * 0.1, -targetRect.height() * 0.1);

    painter->drawArc(hookRect, 45 * 16, 180 * 16); // Quarter circle hook

    painter->restore();
}

void ItemRenderer::drawBlockingIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (!painter || !item) return;

    painter->save();

    // Draw blocking indicator with cross pattern
    QPen blockPen(Qt::red, 1, Qt::DashLine);
    painter->setPen(blockPen);

    // Draw diagonal cross
    painter->drawLine(targetRect.topLeft(), targetRect.bottomRight());
    painter->drawLine(targetRect.topRight(), targetRect.bottomLeft());

    painter->restore();
}

// Utility methods
QColor ItemRenderer::generateItemColor(const Item* item)
{
    if (!item) return Qt::gray;

    // Generate color based on item ID for consistent visual distinction
    quint16 id = item->getServerId();

    // Use item type for base color if available
    QColor baseColor = getItemTypeColor(item);

    // Modify hue based on item ID for variation
    int hue = (baseColor.hue() + (id * 137) % 360) % 360; // 137 is a prime for good distribution
    baseColor.setHsv(hue, baseColor.saturation(), baseColor.value());

    return baseColor;
}

QColor ItemRenderer::getItemTypeColor(const Item* item)
{
    if (!item) return Qt::gray;

    // Color coding based on item properties
    if (item->isGroundTile()) return QColor(139, 69, 19);    // Brown for ground
    if (item->isContainer()) return QColor(160, 82, 45);     // Saddle brown for containers
    if (item->isTeleport()) return QColor(138, 43, 226);     // Blue violet for teleports
    if (item->isReadable()) return QColor(255, 255, 224);    // Light yellow for books
    if (item->isBlocking()) return QColor(105, 105, 105);    // Dim gray for blocking items
    if (item->isPickupable()) return QColor(144, 238, 144);  // Light green for pickupable

    return QColor(176, 196, 222); // Light steel blue for default
}

// Private helper methods
void ItemRenderer::applyItemOpacity(QPainter* painter, const Item* item, const DrawingOptions& options)
{
    if (!painter || !item) return;

    qreal opacity = 1.0;

    // Apply transparency based on item properties
    if (options.useTransparency) {
        // Items on different floors might have reduced opacity
        // This would integrate with TransparencyManager if available
        if (item->hasAttribute("floorTransparency")) {
            opacity *= item->getAttribute("floorTransparency").toDouble();
        }
    }

    // Apply selection opacity
    if (item->isSelected() && options.selectionOpacity < 1.0) {
        opacity *= options.selectionOpacity;
    }

    painter->setOpacity(painter->opacity() * opacity);
}

QRectF ItemRenderer::calculateDrawRect(const QRectF& targetRect, const QImage& image, const DrawingOptions& options)
{
    Q_UNUSED(options)

    if (image.isNull()) return targetRect;

    // For now, just return the target rect
    // In a full implementation, this would handle:
    // - Image scaling based on zoom level
    // - Offset calculations for multi-tile sprites
    // - Alignment options

    return targetRect;
}

QString ItemRenderer::getItemTypeIndicator(const Item* item)
{
    if (!item) return QString();

    // Return single character indicators for item types
    if (item->isContainer()) return "C";
    if (item->isTeleport()) return "T";
    if (item->isReadable()) return "R";
    if (item->isGroundTile()) return "G";
    if (item->isBlocking()) return "B";

    return QString();
}

QColor ItemRenderer::getSelectionColor(const Item* item)
{
    Q_UNUSED(item)

    // Return standard selection color
    // Could be customized based on item type or user preferences
    return QColor(0, 120, 215, 128); // Windows-style selection blue
}

QColor ItemRenderer::getHighlightColor(const Item* item, const DrawingOptions& options)
{
    Q_UNUSED(item)
    Q_UNUSED(options)

    // Return standard highlight color
    return QColor(255, 255, 0, 100); // Yellow highlight
}

void ItemRenderer::drawBoundingBox(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options)
{
    if (!painter || !item || !options.drawDebugInfo) return;

    painter->save();

    QPen boundingPen(Qt::magenta, 1, Qt::DotLine);
    painter->setPen(boundingPen);
    painter->drawRect(targetRect);

    painter->restore();
}


