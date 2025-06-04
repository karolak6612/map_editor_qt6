#ifndef ITEMRENDERER_H
#define ITEMRENDERER_H

#include <QObject>
#include <QRectF>
#include <QColor>
#include <QSharedPointer>

// Forward declarations
class QPainter;
class Item;
class DrawingOptions;
class GameSpriteData;

/**
 * @brief ItemRenderer - Helper class for rendering Item objects
 * 
 * This class extracts all rendering logic from Item.cpp to comply with mandate M6.
 * It handles placeholder rendering, sprite rendering, special flags, and debug information.
 * 
 * Task 011: Refactor large source files - Extract rendering logic from Item.cpp
 */
class ItemRenderer : public QObject
{
    Q_OBJECT

public:
    explicit ItemRenderer(QObject *parent = nullptr);
    ~ItemRenderer() override;

    // Main rendering methods
    static void draw(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawText(const Item* item, QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options);

    // Sprite rendering methods
    static void drawWithSprites(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawSpriteLayer(const Item* item, QPainter* painter, const QRectF& targetRect,
                               QSharedPointer<const GameSpriteData> spriteData,
                               int frame, int patternX, int patternY, int patternZ,
                               int layer, const DrawingOptions& options);
    static void drawMultiTileSprite(const Item* item, QPainter* painter, const QRectF& baseRect,
                                   const QImage& frameImage,
                                   QSharedPointer<const GameSpriteData> spriteData,
                                   const DrawingOptions& options);

    // Placeholder rendering methods
    static void drawPlaceholder(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawItemIdText(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawTypeIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

    // Debug rendering methods
    static void drawDebugInfo(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawBoundingBox(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

    // Special flag rendering methods
    static void drawSpecialFlags(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawSelectionHighlight(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawLockedDoorHighlight(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawWallHookIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);
    static void drawBlockingIndicator(const Item* item, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options);

    // Utility methods
    static QColor generateItemColor(const Item* item);
    static QColor getItemTypeColor(const Item* item);

private:
    // Private helper methods
    static void applyItemOpacity(QPainter* painter, const Item* item, const DrawingOptions& options);
    static QRectF calculateDrawRect(const QRectF& targetRect, const QImage& image, const DrawingOptions& options);
    static QString getItemTypeIndicator(const Item* item);
    static QColor getSelectionColor(const Item* item);
    static QColor getHighlightColor(const Item* item, const DrawingOptions& options);
};

#endif // ITEMRENDERER_H
