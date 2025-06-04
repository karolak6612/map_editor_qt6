#include "MapFormatManager.h"
#include "OTBMMapLoader.h"
#include "OTMMMapLoader.h"
#include "MapVersionConverter.h"
#include <QDebug>
#include <QFileInfo>
#include <QFile>

MapFormatManager::MapFormatManager(QObject* parent)
    : QObject(parent)
    , otbmLoader_(new OTBMMapLoader(this))
    , otmmLoader_(new OTMMMapLoader(this))
    , versionConverter_(new MapVersionConverter(this))
{
    qDebug() << "MapFormatManager: Initialized with format support for OTBM and OTMM";
}

MapFormatManager::~MapFormatManager() = default;

bool MapFormatManager::canLoadFormat(const QString& format) const {
    QString lowerFormat = format.toLower();
    return lowerFormat == "otbm" || lowerFormat == "otmm" || 
           lowerFormat == "xml" || lowerFormat == "json";
}

bool MapFormatManager::canSaveFormat(const QString& format) const {
    QString lowerFormat = format.toLower();
    return lowerFormat == "otbm" || lowerFormat == "otmm" || 
           lowerFormat == "xml" || lowerFormat == "json";
}

QString MapFormatManager::detectFormat(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    // First try by extension
    if (extension == "otbm") {
        return "otbm";
    } else if (extension == "otmm") {
        return "otmm";
    } else if (extension == "xml") {
        return "xml";
    } else if (extension == "json") {
        return "json";
    }
    
    // Try to detect by content
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray header = file.read(16);
        file.close();
        
        if (header.startsWith("<?xml")) {
            return "xml";
        } else if (header.startsWith("{") || header.startsWith("[")) {
            return "json";
        } else if (header.size() >= 4) {
            // Check for OTBM magic bytes
            quint32 magic = *reinterpret_cast<const quint32*>(header.constData());
            if (magic == 0x00000000) { // OTBM magic number
                return "otbm";
            }
        }
    }
    
    // Default to OTBM for unknown binary files
    return "otbm";
}

QStringList MapFormatManager::getSupportedFormats() const {
    return QStringList() << "otbm" << "otmm" << "xml" << "json";
}

QStringList MapFormatManager::getLoadableFormats() const {
    return getSupportedFormats();
}

QStringList MapFormatManager::getSavableFormats() const {
    return getSupportedFormats();
}

QString MapFormatManager::getFormatDescription(const QString& format) const {
    QString lowerFormat = format.toLower();
    
    if (lowerFormat == "otbm") {
        return "OpenTibia Binary Map (*.otbm)";
    } else if (lowerFormat == "otmm") {
        return "OpenTibia Memory Map (*.otmm)";
    } else if (lowerFormat == "xml") {
        return "XML Map Format (*.xml)";
    } else if (lowerFormat == "json") {
        return "JSON Map Format (*.json)";
    }
    
    return "Unknown Format";
}

QString MapFormatManager::getFormatExtension(const QString& format) const {
    QString lowerFormat = format.toLower();
    
    if (lowerFormat == "otbm") {
        return "otbm";
    } else if (lowerFormat == "otmm") {
        return "otmm";
    } else if (lowerFormat == "xml") {
        return "xml";
    } else if (lowerFormat == "json") {
        return "json";
    }
    
    return "";
}

bool MapFormatManager::isFormatVersioned(const QString& format) const {
    QString lowerFormat = format.toLower();
    return lowerFormat == "otbm" || lowerFormat == "otmm";
}

QStringList MapFormatManager::getSupportedVersions(const QString& format) const {
    QString lowerFormat = format.toLower();
    
    if (lowerFormat == "otbm") {
        return QStringList() << "1.0" << "2.0" << "3.0";
    } else if (lowerFormat == "otmm") {
        return QStringList() << "1.0";
    }
    
    return QStringList();
}

bool MapFormatManager::validateFormat(const QString& filePath, const QString& expectedFormat) const {
    QString detectedFormat = detectFormat(filePath);
    return detectedFormat.toLower() == expectedFormat.toLower();
}

MapFormatInfo MapFormatManager::getFormatInfo(const QString& format) const {
    MapFormatInfo info;
    info.name = format;
    info.description = getFormatDescription(format);
    info.extension = getFormatExtension(format);
    info.canLoad = canLoadFormat(format);
    info.canSave = canSaveFormat(format);
    info.isVersioned = isFormatVersioned(format);
    info.supportedVersions = getSupportedVersions(format);
    
    return info;
}

QList<MapFormatInfo> MapFormatManager::getAllFormatsInfo() const {
    QList<MapFormatInfo> formats;
    QStringList supportedFormats = getSupportedFormats();
    
    for (const QString& format : supportedFormats) {
        formats.append(getFormatInfo(format));
    }
    
    return formats;
}

OTBMMapLoader* MapFormatManager::getOTBMLoader() const {
    return otbmLoader_;
}

OTMMMapLoader* MapFormatManager::getOTMMLoader() const {
    return otmmLoader_;
}

MapVersionConverter* MapFormatManager::getVersionConverter() const {
    return versionConverter_;
}

QString MapFormatManager::getLastError() const {
    return lastError_;
}

void MapFormatManager::clearError() {
    lastError_.clear();
}

void MapFormatManager::setError(const QString& error) {
    lastError_ = error;
    qWarning() << "MapFormatManager Error:" << error;
}
