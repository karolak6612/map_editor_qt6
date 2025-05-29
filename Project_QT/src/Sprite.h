#ifndef SPRITE_H
#define SPRITE_H

#include <QImage>
#include <QPainter> // For QPaintDevice, which QImage is
#include <QSize>
#include <QColor> // Added for QColor

// Forward declaration
class QPaintDevice;

class Sprite {
public:
    Sprite();
    virtual ~Sprite();

    virtual void draw(QPainter* painter, int x, int y, int width = -1, int height = -1) = 0;
    virtual bool load(const QString& path); // Generic load, might be from file or data
    virtual bool loadFromData(const QByteArray& data, const char* format = nullptr);

    virtual QImage getImage() const;
    virtual void setImage(const QImage& image);

    virtual QImage scale(const QSize& size, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio, Qt::TransformationMode transformMode = Qt::SmoothTransformation);
    virtual void setTransparency(int alpha); // Simple alpha channel setting
    
    virtual void setMaskColor(const QColor& color, bool enable = true);


protected:
    QImage m_image;
};

#endif // SPRITE_H
