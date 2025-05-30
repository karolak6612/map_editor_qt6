#ifndef QT_EDITORSPRITE_H
#define QT_EDITORSPRITE_H

#include "Sprite.h" // Abstract base class
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

    ~EditorSprite() override;

    // Overridden pure virtual methods from Sprite
    void drawTo(QPainter* painter, const QRect& targetScreenRect, const QRect& sourceSpriteRect) override;
    void drawTo(QPainter* painter, const QPoint& targetPos, int sourceX = 0, int sourceY = 0, int sourceWidth = -1, int sourceHeight = -1) override;
    
    // Specific draw method for EditorSprite, allowing to request a specific size
    // This is closer to the original wxEditorSprite::DrawTo(wxDC* dc, SpriteSize sz, ...)
    void drawTo(QPainter* painter, const QPoint& targetPos, SpriteSize sz);

    void unload() override;

    int width() const override;  // Returns width of the default/primary pixmap (e.g., 32x32)
    int height() const override; // Returns height of the default/primary pixmap

    // Get specific sized pixmap, if available
    QPixmap getPixmap(SpriteSize sz) const;
    bool hasPixmap(SpriteSize sz) const;

private:
    QMap<SpriteSize, QPixmap> m_pixmaps;
    SpriteSize m_defaultSize;

    // Helper to determine which pixmap to use based on target size or requested SpriteSize
    const QPixmap* getBestFitPixmap(SpriteSize requestedSize) const;
    const QPixmap* getBestFitPixmap(const QSize& targetSize) const;
};

#endif // QT_EDITORSPRITE_H
