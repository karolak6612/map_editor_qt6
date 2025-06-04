#ifndef QT_EDITORSPRITE_H
#define QT_EDITORSPRITE_H

#include "Sprite.h" // Abstract base class
#include "EditorSpriteIds.h"
#include <QPixmap>
#include <QString>
#include <QMap>

// Forward declaration
class QPainter;

class EditorSprite : public Sprite {
    Q_OBJECT

public:
    // Constructor that loads pixmaps from resource paths or file paths
    explicit EditorSprite(const QString& path16x16, const QString& path32x32, const QString& path64x64 = QString(), QObject *parent = nullptr);

    // Constructor that accepts already loaded pixmaps
    explicit EditorSprite(const QPixmap& pixmap16x16, const QPixmap& pixmap32x32, const QPixmap& pixmap64x64 = QPixmap(), QObject *parent = nullptr);

    // Constructor that loads from editor sprite ID (uses resource mapping)
    explicit EditorSprite(EditorSpriteId spriteId, QObject *parent = nullptr);

    // Constructor that loads from resource structure
    explicit EditorSprite(const EditorSpriteResource& resource, QObject *parent = nullptr);

    ~EditorSprite() override;

    // Overridden pure virtual methods from Sprite
    void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize size) override;

    void unload() override;

    int width() const override;  // Returns width of the default/primary pixmap (e.g., 32x32)
    int height() const override; // Returns height of the default/primary pixmap
    int width(SpriteSize size) const override;   // Returns width for specific size
    int height(SpriteSize size) const override;  // Returns height for specific size

    bool isLoaded() const override;

    // Get specific sized pixmap, if available
    QPixmap getPixmap(SpriteSize sz) const;
    bool hasPixmap(SpriteSize sz) const;

    // Convenience methods
    QPixmap getPixmap() const { return getPixmap(m_defaultSize); }

    // Scaling and caching
    void enableScalingCache(bool enable = true) { m_scalingCacheEnabled = enable; }
    bool isScalingCacheEnabled() const { return m_scalingCacheEnabled; }
    void clearScalingCache();

    // Resource management
    static EditorSprite* createFromId(EditorSpriteId spriteId, QObject* parent = nullptr);
    static bool isValidSpriteId(EditorSpriteId spriteId);

private:
    QMap<SpriteSize, QPixmap> m_pixmaps;
    SpriteSize m_defaultSize;

    // Scaling cache
    mutable QMap<SpriteSize, QPixmap> m_scaledPixmaps;
    bool m_scalingCacheEnabled;

    // Helper methods
    void loadFromPaths(const QString& path16x16, const QString& path32x32, const QString& path64x64);
    QPixmap loadPixmapFromPath(const QString& path) const;
    const QPixmap* getBestFitPixmap(SpriteSize requestedSize) const;
    const QPixmap* getBestFitPixmap(const QSize& targetSize) const;
    QPixmap getScaledPixmap(SpriteSize targetSize, SpriteSize sourceSize) const;

    // Task 014: Missing validation and fallback methods
    bool validateDrawingParameters(QPainter* painter, const QPoint& targetPos) const;
    QPixmap createFallbackPixmap(SpriteSize size) const;
    void drawFallbackPlaceholder(QPainter* painter, const QRect& targetRect) const;
};

#endif // QT_EDITORSPRITE_H
