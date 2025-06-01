// ScreenshotManager.cpp - Qt-based screenshot system implementation

#include "ScreenshotManager.h"
#include "MapView.h"
#include <QPixmap>
#include <QScreen>
#include <QApplication>
#include <QWidget>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

ScreenshotManager::ScreenshotManager(QObject* parent)
    : QObject(parent)
    , defaultFormat_(Format::PNG)
    , defaultQuality_(Quality::High)
    , screenshotCount_(0)
{
    // Set default directory to Pictures/Screenshots
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    defaultDirectory_ = QDir(picturesPath).absoluteFilePath("Screenshots");
    
    qDebug() << "ScreenshotManager: Qt-based screenshot system initialized";
    qDebug() << "Default directory:" << defaultDirectory_;
}

ScreenshotManager::~ScreenshotManager() {
    qDebug() << "ScreenshotManager: Destroyed, took" << screenshotCount_ << "screenshots";
}

// Task 76: Main screenshot methods (replaces OpenGL TakeScreenshot)
bool ScreenshotManager::takeScreenshot(QWidget* widget, const QString& filePath, 
                                      Format format, Quality quality) {
    if (!widget) {
        emit screenshotFailed("Widget is null");
        return false;
    }

    try {
        QPixmap pixmap = captureWidget(widget);
        if (pixmap.isNull()) {
            emit screenshotFailed("Failed to capture widget");
            return false;
        }

        bool success = savePixmap(pixmap, filePath, format, quality);
        if (success) {
            screenshotCount_++;
            lastScreenshotTime_ = QDateTime::currentDateTime();
            emit screenshotTaken(filePath, true);
            qDebug() << "ScreenshotManager::takeScreenshot: Saved to" << filePath;
        } else {
            emit screenshotFailed("Failed to save screenshot");
        }
        
        return success;
    } catch (const std::exception& e) {
        QString error = QString("Screenshot failed: %1").arg(e.what());
        emit screenshotFailed(error);
        return false;
    }
}

bool ScreenshotManager::takeMapViewScreenshot(MapView* mapView, const QString& filePath,
                                             Format format, Quality quality) {
    if (!mapView) {
        emit screenshotFailed("MapView is null");
        return false;
    }

    try {
        QPixmap pixmap = captureMapView(mapView);
        if (pixmap.isNull()) {
            emit screenshotFailed("Failed to capture MapView");
            return false;
        }

        bool success = savePixmap(pixmap, filePath, format, quality);
        if (success) {
            screenshotCount_++;
            lastScreenshotTime_ = QDateTime::currentDateTime();
            emit screenshotTaken(filePath, true);
            qDebug() << "ScreenshotManager::takeMapViewScreenshot: Saved to" << filePath;
        } else {
            emit screenshotFailed("Failed to save MapView screenshot");
        }
        
        return success;
    } catch (const std::exception& e) {
        QString error = QString("MapView screenshot failed: %1").arg(e.what());
        emit screenshotFailed(error);
        return false;
    }
}

bool ScreenshotManager::takeFullScreenScreenshot(const QString& filePath,
                                                Format format, Quality quality) {
    try {
        QPixmap pixmap = captureScreen();
        if (pixmap.isNull()) {
            emit screenshotFailed("Failed to capture screen");
            return false;
        }

        bool success = savePixmap(pixmap, filePath, format, quality);
        if (success) {
            screenshotCount_++;
            lastScreenshotTime_ = QDateTime::currentDateTime();
            emit screenshotTaken(filePath, true);
            qDebug() << "ScreenshotManager::takeFullScreenScreenshot: Saved to" << filePath;
        } else {
            emit screenshotFailed("Failed to save fullscreen screenshot");
        }
        
        return success;
    } catch (const std::exception& e) {
        QString error = QString("Fullscreen screenshot failed: %1").arg(e.what());
        emit screenshotFailed(error);
        return false;
    }
}

// Task 76: Screenshot with automatic naming
QString ScreenshotManager::takeScreenshotAuto(QWidget* widget, const QString& directory,
                                             Format format, Quality quality) {
    QString dir = directory.isEmpty() ? defaultDirectory_ : directory;
    QString dirPath = ensureDirectory(dir);
    
    if (dirPath.isEmpty()) {
        emit screenshotFailed("Failed to create directory: " + dir);
        return QString();
    }
    
    QString filename = generateTimestampFilename("screenshot", format);
    QString filePath = QDir(dirPath).absoluteFilePath(filename);
    filePath = getUniqueFilePath(filePath);
    
    if (takeScreenshot(widget, filePath, format, quality)) {
        return filePath;
    }
    
    return QString();
}

QString ScreenshotManager::takeMapViewScreenshotAuto(MapView* mapView, const QString& directory,
                                                    Format format, Quality quality) {
    QString dir = directory.isEmpty() ? defaultDirectory_ : directory;
    QString dirPath = ensureDirectory(dir);
    
    if (dirPath.isEmpty()) {
        emit screenshotFailed("Failed to create directory: " + dir);
        return QString();
    }
    
    QString filename = generateTimestampFilename("mapview", format);
    QString filePath = QDir(dirPath).absoluteFilePath(filename);
    filePath = getUniqueFilePath(filePath);
    
    if (takeMapViewScreenshot(mapView, filePath, format, quality)) {
        return filePath;
    }
    
    return QString();
}

// Task 76: Internal screenshot methods
QPixmap ScreenshotManager::captureWidget(QWidget* widget) {
    if (!widget) {
        return QPixmap();
    }

    // Use QWidget::render for high-quality capture (replaces glReadPixels)
    QPixmap pixmap(widget->size());
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    widget->render(&painter);
    
    return pixmap;
}

QPixmap ScreenshotManager::captureMapView(MapView* mapView) {
    if (!mapView) {
        return QPixmap();
    }

    // Capture the entire MapView including scene content
    QPixmap pixmap(mapView->size());
    pixmap.fill(Qt::white); // White background for map screenshots
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // Render the MapView
    mapView->render(&painter);
    
    return pixmap;
}

QPixmap ScreenshotManager::captureScreen() {
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }

    // Use QScreen::grabWindow for fullscreen capture (replaces glReadPixels)
    return screen->grabWindow(0);
}

bool ScreenshotManager::savePixmap(const QPixmap& pixmap, const QString& filePath, 
                                  Format format, Quality quality) {
    if (pixmap.isNull()) {
        return false;
    }

    // Ensure directory exists
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "ScreenshotManager::savePixmap: Failed to create directory" << dir.path();
        return false;
    }

    // Convert format to Qt format string
    const char* qtFormat = nullptr;
    int qtQuality = static_cast<int>(quality);
    
    switch (format) {
        case Format::PNG:
            qtFormat = "PNG";
            qtQuality = -1; // PNG ignores quality
            break;
        case Format::JPEG:
            qtFormat = "JPEG";
            break;
        case Format::BMP:
            qtFormat = "BMP";
            qtQuality = -1; // BMP ignores quality
            break;
        case Format::TIFF:
            qtFormat = "TIFF";
            break;
    }

    bool success = pixmap.save(filePath, qtFormat, qtQuality);
    if (!success) {
        qWarning() << "ScreenshotManager::savePixmap: Failed to save" << filePath;
    }
    
    return success;
}

// Task 76: Utility methods
QString ScreenshotManager::formatToString(Format format) {
    switch (format) {
        case Format::PNG: return "PNG";
        case Format::JPEG: return "JPEG";
        case Format::BMP: return "BMP";
        case Format::TIFF: return "TIFF";
    }
    return "PNG";
}

QString ScreenshotManager::formatToExtension(Format format) {
    switch (format) {
        case Format::PNG: return ".png";
        case Format::JPEG: return ".jpg";
        case Format::BMP: return ".bmp";
        case Format::TIFF: return ".tiff";
    }
    return ".png";
}

ScreenshotManager::Format ScreenshotManager::stringToFormat(const QString& formatStr) {
    QString upper = formatStr.toUpper();
    if (upper == "JPEG" || upper == "JPG") return Format::JPEG;
    if (upper == "BMP") return Format::BMP;
    if (upper == "TIFF" || upper == "TIF") return Format::TIFF;
    return Format::PNG; // Default
}

QString ScreenshotManager::generateTimestampFilename(const QString& prefix, Format format) {
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyy-MM-dd_hh-mm-ss");
    return QString("%1_%2%3").arg(prefix, timestamp, formatToExtension(format));
}

QString ScreenshotManager::ensureDirectory(const QString& directory) {
    QDir dir(directory);
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "ScreenshotManager::ensureDirectory: Failed to create" << directory;
        return QString();
    }
    return dir.absolutePath();
}

QString ScreenshotManager::getUniqueFilePath(const QString& basePath) {
    if (!QFile::exists(basePath)) {
        return basePath;
    }
    
    QFileInfo fileInfo(basePath);
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    QString dir = fileInfo.absolutePath();
    
    int counter = 1;
    QString uniquePath;
    
    do {
        uniquePath = QDir(dir).absoluteFilePath(
            QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
        counter++;
    } while (QFile::exists(uniquePath) && counter < 1000);
    
    return uniquePath;
}
