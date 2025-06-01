// ImageSpace.cpp - Implementation of ImageSpace concept for Task 35

#include "ImageSpace.h"
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <QtMath>

// Image base class implementation
ImageSpace::Image::Image(quint32 id, ImageType type)
    : id_(id), type_(type), loaded_(false), lastAccess_(0) {
    visit(); // Set initial access time
}

ImageSpace::Image::~Image() {
    // Base destructor
}

void ImageSpace::Image::unload() {
    QMutexLocker locker(&mutex_);
    loaded_ = false;
}

void ImageSpace::Image::clean(qint64 currentTime, qint64 maxAge) {
    if (loaded_ && (currentTime - lastAccess_) > maxAge) {
        unload();
    }
}

void ImageSpace::Image::visit() {
    QMutexLocker locker(&mutex_);
    lastAccess_ = QDateTime::currentMSecsSinceEpoch();
}

// NormalImage implementation
ImageSpace::NormalImage::NormalImage(quint32 id)
    : Image(id, ImageType::Normal), imageCacheValid_(false) {
}

ImageSpace::NormalImage::~NormalImage() {
    unload();
}

QByteArray ImageSpace::NormalImage::getRawData() const {
    QMutexLocker locker(&mutex_);
    visit();
    return rawData_;
}

QImage ImageSpace::NormalImage::getQImage() const {
    QMutexLocker locker(&mutex_);
    visit();
    
    if (!imageCacheValid_ || cachedImage_.isNull()) {
        if (!rawData_.isEmpty()) {
            // Convert raw RGBA data to QImage
            cachedImage_ = QImage(reinterpret_cast<const uchar*>(rawData_.constData()), 
                                32, 32, QImage::Format_RGBA8888);
        } else if (!rleData_.isEmpty()) {
            // Decode RLE data to QImage
            cachedImage_ = decodeRleData(rleData_, true);
        } else {
            // Create transparent image
            cachedImage_ = QImage(32, 32, QImage::Format_RGBA8888);
            cachedImage_.fill(Qt::transparent);
        }
        imageCacheValid_ = true;
    }
    
    return cachedImage_;
}

bool ImageSpace::NormalImage::loadFromData(const QByteArray& data) {
    QMutexLocker locker(&mutex_);
    
    if (data.size() != 32 * 32 * 4) { // Expected RGBA data size
        qWarning() << "ImageSpace::NormalImage::loadFromData: Invalid data size" << data.size() 
                   << "expected" << (32 * 32 * 4);
        return false;
    }
    
    rawData_ = data;
    rleData_.clear();
    imageCacheValid_ = false;
    loaded_ = true;
    visit();
    
    return true;
}

bool ImageSpace::NormalImage::loadFromRleData(const QByteArray& rleData, bool hasAlpha) {
    QMutexLocker locker(&mutex_);
    
    rleData_ = rleData;
    rawData_.clear();
    imageCacheValid_ = false;
    loaded_ = true;
    visit();
    
    return true;
}

void ImageSpace::NormalImage::unload() {
    QMutexLocker locker(&mutex_);
    rawData_.clear();
    rleData_.clear();
    cachedImage_ = QImage();
    imageCacheValid_ = false;
    Image::unload();
}

void ImageSpace::NormalImage::clean(qint64 currentTime, qint64 maxAge) {
    Image::clean(currentTime, maxAge);
    
    // Clean cached image if not accessed recently
    if ((currentTime - lastAccess_) > (maxAge / 2)) {
        QMutexLocker locker(&mutex_);
        cachedImage_ = QImage();
        imageCacheValid_ = false;
    }
}

QImage ImageSpace::NormalImage::decodeRleData(const QByteArray& rleData, bool hasAlpha) const {
    if (rleData.isEmpty()) {
        QImage image(32, 32, QImage::Format_RGBA8888);
        image.fill(Qt::transparent);
        return image;
    }
    
    QImage image(32, 32, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);
    
    uchar* imageData = image.bits();
    const uchar* rleBytes = reinterpret_cast<const uchar*>(rleData.constData());
    int rleSize = rleData.size();
    
    int rleIdx = 0;
    int currentPixel = 0;
    int bytesPerPixel = hasAlpha ? 4 : 3;
    
    while (rleIdx < rleSize && currentPixel < 1024) {
        // Read transparent pixel count
        if (rleIdx + 1 >= rleSize) break;
        quint16 transparentPixels = static_cast<quint16>(rleBytes[rleIdx]) | 
                                  (static_cast<quint16>(rleBytes[rleIdx+1]) << 8);
        rleIdx += 2;
        currentPixel += transparentPixels;
        
        if (currentPixel >= 1024 || rleIdx >= rleSize) break;
        
        // Read colored pixel count
        if (rleIdx + 1 >= rleSize) break;
        quint16 coloredPixels = static_cast<quint16>(rleBytes[rleIdx]) | 
                              (static_cast<quint16>(rleBytes[rleIdx+1]) << 8);
        rleIdx += 2;
        
        // Read colored pixels
        for (quint16 i = 0; i < coloredPixels && currentPixel < 1024; ++i) {
            if (rleIdx + bytesPerPixel > rleSize) break;
            
            int imgByteIdx = currentPixel * 4;
            
            // BGRA order in file, convert to RGBA for QImage
            imageData[imgByteIdx + 0] = rleBytes[rleIdx + 2]; // Red
            imageData[imgByteIdx + 1] = rleBytes[rleIdx + 1]; // Green
            imageData[imgByteIdx + 2] = rleBytes[rleIdx + 0]; // Blue
            imageData[imgByteIdx + 3] = hasAlpha ? rleBytes[rleIdx + 3] : 255; // Alpha
            
            rleIdx += bytesPerPixel;
            currentPixel++;
        }
    }
    
    return image;
}

// TemplateImage implementation
ImageSpace::TemplateImage::TemplateImage(quint32 id, const QSharedPointer<NormalImage>& baseImage)
    : Image(id, ImageType::Template), baseImage_(baseImage), 
      outfitHead_(0), outfitBody_(0), outfitLegs_(0), outfitFeet_(0),
      colorizedCacheValid_(false) {
}

ImageSpace::TemplateImage::~TemplateImage() {
    unload();
}

QByteArray ImageSpace::TemplateImage::getRawData() const {
    if (!baseImage_) return QByteArray();
    return baseImage_->getRawData();
}

QImage ImageSpace::TemplateImage::getQImage() const {
    QMutexLocker locker(&mutex_);
    visit();
    
    if (!colorizedCacheValid_ || cachedColorizedImage_.isNull()) {
        if (baseImage_) {
            cachedColorizedImage_ = getColorizedImage(outfitHead_, outfitBody_, outfitLegs_, outfitFeet_);
        } else {
            cachedColorizedImage_ = QImage(32, 32, QImage::Format_RGBA8888);
            cachedColorizedImage_.fill(Qt::transparent);
        }
        colorizedCacheValid_ = true;
    }
    
    return cachedColorizedImage_;
}

bool ImageSpace::TemplateImage::loadFromData(const QByteArray& data) {
    // Template images don't load data directly, they use base images
    Q_UNUSED(data);
    return false;
}

void ImageSpace::TemplateImage::setOutfitColors(quint8 head, quint8 body, quint8 legs, quint8 feet) {
    QMutexLocker locker(&mutex_);
    
    if (outfitHead_ != head || outfitBody_ != body || outfitLegs_ != legs || outfitFeet_ != feet) {
        outfitHead_ = head;
        outfitBody_ = body;
        outfitLegs_ = legs;
        outfitFeet_ = feet;
        colorizedCacheValid_ = false;
        visit();
    }
}

QImage ImageSpace::TemplateImage::getColorizedImage(quint8 head, quint8 body, quint8 legs, quint8 feet) const {
    if (!baseImage_) {
        QImage image(32, 32, QImage::Format_RGBA8888);
        image.fill(Qt::transparent);
        return image;
    }
    
    QImage baseImg = baseImage_->getQImage();
    if (baseImg.isNull()) {
        QImage image(32, 32, QImage::Format_RGBA8888);
        image.fill(Qt::transparent);
        return image;
    }
    
    QImage colorized = baseImg.copy();
    
    // Apply outfit coloring (simplified implementation)
    // This would need to be enhanced based on the actual Tibia outfit coloring algorithm
    for (int y = 0; y < colorized.height(); ++y) {
        for (int x = 0; x < colorized.width(); ++x) {
            QRgb pixel = colorized.pixel(x, y);
            quint8 r = qRed(pixel);
            quint8 g = qGreen(pixel);
            quint8 b = qBlue(pixel);
            quint8 a = qAlpha(pixel);
            
            if (a > 0) { // Only colorize non-transparent pixels
                // Simple colorization based on grayscale value
                quint8 gray = (r + g + b) / 3;
                
                // Apply outfit colors based on pixel position or other criteria
                // This is a simplified implementation
                if (gray > 200) {
                    colorizePixel(head, r, g, b);
                } else if (gray > 150) {
                    colorizePixel(body, r, g, b);
                } else if (gray > 100) {
                    colorizePixel(legs, r, g, b);
                } else {
                    colorizePixel(feet, r, g, b);
                }
                
                colorized.setPixel(x, y, qRgba(r, g, b, a));
            }
        }
    }
    
    return colorized;
}

int ImageSpace::TemplateImage::getDataSize() const {
    return baseImage_ ? baseImage_->getDataSize() : 0;
}

void ImageSpace::TemplateImage::unload() {
    QMutexLocker locker(&mutex_);
    cachedColorizedImage_ = QImage();
    colorizedCacheValid_ = false;
    Image::unload();
}

void ImageSpace::TemplateImage::colorizePixel(quint8 color, quint8& r, quint8& g, quint8& b) const {
    // Simplified outfit color mapping
    // This would need to be replaced with the actual Tibia color palette
    switch (color) {
        case 0: r = 255; g = 255; b = 255; break; // White
        case 1: r = 255; g = 0; b = 0; break;     // Red
        case 2: r = 0; g = 255; b = 0; break;     // Green
        case 3: r = 0; g = 0; b = 255; break;     // Blue
        case 4: r = 255; g = 255; b = 0; break;   // Yellow
        case 5: r = 255; g = 0; b = 255; break;   // Magenta
        case 6: r = 0; g = 255; b = 255; break;   // Cyan
        case 7: r = 0; g = 0; b = 0; break;       // Black
        default: break; // Keep original color
    }
}

// ImageSpace main class implementation
ImageSpace::ImageSpace(QObject* parent)
    : QObject(parent), maxCacheSize_(100 * 1024 * 1024), // 100MB default
      cachedMemoryUsage_(0), memoryUsageDirty_(true) {

    // Setup cleanup timer
    cleanupTimer_ = new QTimer(this);
    connect(cleanupTimer_, &QTimer::timeout, this, &ImageSpace::onCleanupTimer);
    cleanupTimer_->start(30000); // Cleanup every 30 seconds

    qDebug() << "ImageSpace: Initialized with max cache size" << maxCacheSize_ << "bytes";
}

ImageSpace::~ImageSpace() {
    unloadAllImages();
}

QSharedPointer<ImageSpace::Image> ImageSpace::getImage(quint32 id) const {
    QMutexLocker locker(&imagesMutex_);
    return images_.value(id, nullptr);
}

QSharedPointer<ImageSpace::NormalImage> ImageSpace::getNormalImage(quint32 id) const {
    auto image = getImage(id);
    return qSharedPointerDynamicCast<NormalImage>(image);
}

QSharedPointer<ImageSpace::TemplateImage> ImageSpace::getTemplateImage(quint32 id) const {
    auto image = getImage(id);
    return qSharedPointerDynamicCast<TemplateImage>(image);
}

QSharedPointer<ImageSpace::NormalImage> ImageSpace::createNormalImage(quint32 id) {
    QMutexLocker locker(&imagesMutex_);

    auto existing = images_.value(id, nullptr);
    if (existing && existing->getType() == ImageType::Normal) {
        return qSharedPointerDynamicCast<NormalImage>(existing);
    }

    auto normalImage = QSharedPointer<NormalImage>::create(id);
    images_[id] = normalImage;
    memoryUsageDirty_ = true;

    emit imageLoaded(id);
    return normalImage;
}

QSharedPointer<ImageSpace::TemplateImage> ImageSpace::createTemplateImage(quint32 id, const QSharedPointer<NormalImage>& baseImage) {
    QMutexLocker locker(&imagesMutex_);

    auto existing = images_.value(id, nullptr);
    if (existing && existing->getType() == ImageType::Template) {
        return qSharedPointerDynamicCast<TemplateImage>(existing);
    }

    auto templateImage = QSharedPointer<TemplateImage>::create(id, baseImage);
    images_[id] = templateImage;
    memoryUsageDirty_ = true;

    emit imageLoaded(id);
    return templateImage;
}

bool ImageSpace::loadImageFromRleData(quint32 id, const QByteArray& rleData, bool hasAlpha) {
    auto normalImage = createNormalImage(id);
    if (!normalImage) return false;

    bool success = normalImage->loadFromRleData(rleData, hasAlpha);
    if (success) {
        memoryUsageDirty_ = true;
        enforceMemoryLimit();
    }

    return success;
}

bool ImageSpace::loadImageFromRawData(quint32 id, const QByteArray& rawData) {
    auto normalImage = createNormalImage(id);
    if (!normalImage) return false;

    bool success = normalImage->loadFromData(rawData);
    if (success) {
        memoryUsageDirty_ = true;
        enforceMemoryLimit();
    }

    return success;
}

void ImageSpace::cleanupImages(qint64 maxAge) {
    QMutexLocker locker(&imagesMutex_);

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QList<quint32> toRemove;

    for (auto it = images_.begin(); it != images_.end(); ++it) {
        auto image = it.value();
        if (image) {
            image->clean(currentTime, maxAge);

            // Remove completely unloaded images
            if (!image->isLoaded()) {
                toRemove.append(it.key());
            }
        }
    }

    // Remove unloaded images
    for (quint32 id : toRemove) {
        images_.remove(id);
        emit imageUnloaded(id);
    }

    if (!toRemove.isEmpty()) {
        memoryUsageDirty_ = true;
        qDebug() << "ImageSpace: Cleaned up" << toRemove.size() << "images";
    }
}

void ImageSpace::unloadImage(quint32 id) {
    QMutexLocker locker(&imagesMutex_);

    auto image = images_.value(id, nullptr);
    if (image) {
        image->unload();
        images_.remove(id);
        memoryUsageDirty_ = true;
        emit imageUnloaded(id);
    }
}

void ImageSpace::unloadAllImages() {
    QMutexLocker locker(&imagesMutex_);

    for (auto image : images_) {
        if (image) {
            image->unload();
        }
    }

    images_.clear();
    cachedMemoryUsage_ = 0;
    memoryUsageDirty_ = false;

    emit cacheCleared();
    qDebug() << "ImageSpace: Unloaded all images";
}

void ImageSpace::clearCache() {
    unloadAllImages();
}

int ImageSpace::getImageCount() const {
    QMutexLocker locker(&imagesMutex_);
    return images_.size();
}

int ImageSpace::getLoadedImageCount() const {
    QMutexLocker locker(&imagesMutex_);

    int count = 0;
    for (auto image : images_) {
        if (image && image->isLoaded()) {
            count++;
        }
    }
    return count;
}

qint64 ImageSpace::getTotalMemoryUsage() const {
    if (memoryUsageDirty_) {
        cachedMemoryUsage_ = calculateMemoryUsage();
        memoryUsageDirty_ = false;
    }
    return cachedMemoryUsage_;
}

QList<quint32> ImageSpace::getLoadedImageIds() const {
    QMutexLocker locker(&imagesMutex_);

    QList<quint32> loadedIds;
    for (auto it = images_.begin(); it != images_.end(); ++it) {
        if (it.value() && it.value()->isLoaded()) {
            loadedIds.append(it.key());
        }
    }
    return loadedIds;
}

void ImageSpace::performCleanup() {
    cleanupImages(5000); // 5 second default max age
    enforceMemoryLimit();
}

void ImageSpace::onCleanupTimer() {
    performCleanup();
}

void ImageSpace::enforceMemoryLimit() {
    qint64 currentUsage = getTotalMemoryUsage();

    if (currentUsage > maxCacheSize_) {
        emit memoryLimitReached(currentUsage, maxCacheSize_);

        // Remove oldest images until under limit
        int imagesToRemove = qMax(1, getLoadedImageCount() / 10); // Remove 10% of images
        removeOldestImages(imagesToRemove);

        qDebug() << "ImageSpace: Memory limit exceeded, removed" << imagesToRemove << "images";
    }
}

void ImageSpace::removeOldestImages(int count) {
    QMutexLocker locker(&imagesMutex_);

    // Create list of images sorted by last access time
    QList<QPair<qint64, quint32>> imagesByAge;
    for (auto it = images_.begin(); it != images_.end(); ++it) {
        auto image = it.value();
        if (image && image->isLoaded()) {
            imagesByAge.append(qMakePair(image->getLastAccess(), it.key()));
        }
    }

    // Sort by access time (oldest first)
    std::sort(imagesByAge.begin(), imagesByAge.end());

    // Remove oldest images
    int removed = 0;
    for (auto& pair : imagesByAge) {
        if (removed >= count) break;

        quint32 id = pair.second;
        auto image = images_.value(id, nullptr);
        if (image) {
            image->unload();
            images_.remove(id);
            emit imageUnloaded(id);
            removed++;
        }
    }

    if (removed > 0) {
        memoryUsageDirty_ = true;
    }
}

qint64 ImageSpace::calculateMemoryUsage() const {
    QMutexLocker locker(&imagesMutex_);

    qint64 totalUsage = 0;
    for (auto image : images_) {
        if (image && image->isLoaded()) {
            totalUsage += image->getDataSize();
        }
    }
    return totalUsage;
}
