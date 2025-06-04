#include "MapVersionConverter.h"
#include <QDebug>

MapVersionConverter::MapVersionConverter(QObject* parent)
    : QObject(parent)
{
    qDebug() << "MapVersionConverter: Initialized";
}

MapVersionConverter::~MapVersionConverter() = default;

bool MapVersionConverter::canConvert(const QString& fromFormat, const QString& toFormat) const {
    QString fromLower = fromFormat.toLower();
    QString toLower = toFormat.toLower();
    
    // All supported formats can be converted between each other
    QStringList supportedFormats = {"otbm", "otmm", "xml", "json"};
    
    return supportedFormats.contains(fromLower) && supportedFormats.contains(toLower);
}

bool MapVersionConverter::canConvertVersion(const QString& format, const QString& fromVersion, const QString& toVersion) const {
    QString formatLower = format.toLower();
    
    if (formatLower == "otbm") {
        QStringList supportedVersions = {"1.0", "2.0", "3.0"};
        return supportedVersions.contains(fromVersion) && supportedVersions.contains(toVersion);
    } else if (formatLower == "otmm") {
        QStringList supportedVersions = {"1.0"};
        return supportedVersions.contains(fromVersion) && supportedVersions.contains(toVersion);
    }
    
    // XML and JSON don't have versions
    return fromVersion == toVersion;
}

QStringList MapVersionConverter::getSupportedFormats() const {
    return QStringList() << "otbm" << "otmm" << "xml" << "json";
}

QStringList MapVersionConverter::getSupportedVersions(const QString& format) const {
    QString formatLower = format.toLower();
    
    if (formatLower == "otbm") {
        return QStringList() << "1.0" << "2.0" << "3.0";
    } else if (formatLower == "otmm") {
        return QStringList() << "1.0";
    }
    
    return QStringList();
}

QString MapVersionConverter::getDefaultVersion(const QString& format) const {
    QString formatLower = format.toLower();
    
    if (formatLower == "otbm") {
        return "3.0"; // Latest OTBM version
    } else if (formatLower == "otmm") {
        return "1.0"; // Only OTMM version
    }
    
    return "1.0"; // Default for XML/JSON
}

bool MapVersionConverter::isVersionSupported(const QString& format, const QString& version) const {
    QStringList supportedVersions = getSupportedVersions(format);
    return supportedVersions.contains(version);
}

ConversionInfo MapVersionConverter::getConversionInfo(const QString& fromFormat, const QString& fromVersion, 
                                                     const QString& toFormat, const QString& toVersion) const {
    ConversionInfo info;
    info.fromFormat = fromFormat;
    info.fromVersion = fromVersion;
    info.toFormat = toFormat;
    info.toVersion = toVersion;
    info.isSupported = canConvert(fromFormat, toFormat) && 
                      canConvertVersion(fromFormat, fromVersion, toVersion) &&
                      canConvertVersion(toFormat, fromVersion, toVersion);
    
    // Determine conversion complexity
    if (fromFormat.toLower() == toFormat.toLower()) {
        if (fromVersion == toVersion) {
            info.complexity = ConversionComplexity::None;
        } else {
            info.complexity = ConversionComplexity::Simple;
        }
    } else {
        QString fromLower = fromFormat.toLower();
        QString toLower = toFormat.toLower();
        
        if ((fromLower == "otbm" && toLower == "otmm") || 
            (fromLower == "otmm" && toLower == "otbm")) {
            info.complexity = ConversionComplexity::Moderate;
        } else if ((fromLower == "xml" && toLower == "json") || 
                   (fromLower == "json" && toLower == "xml")) {
            info.complexity = ConversionComplexity::Simple;
        } else {
            info.complexity = ConversionComplexity::Complex;
        }
    }
    
    // Set potential data loss warnings
    if (fromFormat.toLower() == "otbm" && toFormat.toLower() == "xml") {
        info.warnings.append("Binary-specific data may be lost in XML conversion");
    }
    if (fromFormat.toLower() == "otmm" && toFormat.toLower() != "otbm") {
        info.warnings.append("Memory map optimizations will be lost");
    }
    
    return info;
}

QList<ConversionPath> MapVersionConverter::getConversionPaths(const QString& fromFormat, const QString& toFormat) const {
    QList<ConversionPath> paths;
    
    QString fromLower = fromFormat.toLower();
    QString toLower = toFormat.toLower();
    
    if (fromLower == toLower) {
        // Same format, direct conversion
        ConversionPath directPath;
        directPath.steps.append({fromFormat, toFormat});
        directPath.complexity = ConversionComplexity::None;
        paths.append(directPath);
    } else {
        // Different formats, may need intermediate steps
        ConversionPath directPath;
        directPath.steps.append({fromFormat, toFormat});
        
        if ((fromLower == "otbm" && toLower == "otmm") || 
            (fromLower == "otmm" && toLower == "otbm")) {
            directPath.complexity = ConversionComplexity::Moderate;
        } else if ((fromLower == "xml" && toLower == "json") || 
                   (fromLower == "json" && toLower == "xml")) {
            directPath.complexity = ConversionComplexity::Simple;
        } else {
            directPath.complexity = ConversionComplexity::Complex;
        }
        
        paths.append(directPath);
        
        // For complex conversions, suggest intermediate paths via OTBM
        if (directPath.complexity == ConversionComplexity::Complex && 
            fromLower != "otbm" && toLower != "otbm") {
            ConversionPath intermediatePath;
            intermediatePath.steps.append({fromFormat, "otbm"});
            intermediatePath.steps.append({"otbm", toFormat});
            intermediatePath.complexity = ConversionComplexity::Complex;
            paths.append(intermediatePath);
        }
    }
    
    return paths;
}

bool MapVersionConverter::validateConversion(const QString& fromFormat, const QString& fromVersion,
                                           const QString& toFormat, const QString& toVersion) const {
    // Check if formats are supported
    if (!getSupportedFormats().contains(fromFormat.toLower()) ||
        !getSupportedFormats().contains(toFormat.toLower())) {
        return false;
    }
    
    // Check if versions are supported
    if (!isVersionSupported(fromFormat, fromVersion) ||
        !isVersionSupported(toFormat, toVersion)) {
        return false;
    }
    
    // Check if conversion is possible
    return canConvert(fromFormat, toFormat) && 
           canConvertVersion(fromFormat, fromVersion, toVersion);
}

QString MapVersionConverter::getLastError() const {
    return lastError_;
}

void MapVersionConverter::clearError() {
    lastError_.clear();
}

void MapVersionConverter::setError(const QString& error) {
    lastError_ = error;
    qWarning() << "MapVersionConverter Error:" << error;
}
