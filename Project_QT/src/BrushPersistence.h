#ifndef BRUSHPERSISTENCE_H
#define BRUSHPERSISTENCE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDomDocument>
#include <QDomElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QSet>

// Include Brush.h for Brush::Type access
#include "Brush.h"

// Forward declarations
class BrushManager;
class QIODevice;

/**
 * @brief Comprehensive brush persistence system for Task 81
 * 
 * Complete implementation of brush loading/saving functionality including:
 * - XML-based brush serialization/deserialization (wxwidgets compatible)
 * - JSON-based brush serialization/deserialization (modern format)
 * - User-defined brush support with custom properties
 * - Brush state persistence and restoration
 * - File format detection and migration
 * - Integration with BrushManager for seamless operation
 */

/**
 * @brief Brush serialization data structure
 */
struct BrushSerializationData {
    QString name;
    QString type;
    quint32 id;
    QVariantMap properties;
    QVariantMap customData;
    QStringList dependencies;
    QString version;
    qint64 timestamp;
    bool isUserDefined;
    bool isModified;
    
    BrushSerializationData() 
        : id(0), timestamp(0), isUserDefined(false), isModified(false) {}
};

/**
 * @brief Brush collection data for saving/loading multiple brushes
 */
struct BrushCollectionData {
    QString name;
    QString description;
    QString author;
    QString version;
    qint64 timestamp;
    QList<BrushSerializationData> brushes;
    QVariantMap metadata;
    
    BrushCollectionData() : timestamp(0) {}
};

/**
 * @brief Main brush persistence manager
 */
class BrushPersistence : public QObject
{
    Q_OBJECT

public:
    enum FileFormat {
        AUTO_DETECT,        // Auto-detect format from file
        XML_FORMAT,         // XML format (wxwidgets compatible)
        JSON_FORMAT,        // JSON format (modern)
        BINARY_FORMAT       // Binary format (future)
    };

    enum SaveMode {
        SAVE_ALL,           // Save all brushes
        SAVE_USER_DEFINED,  // Save only user-defined brushes
        SAVE_MODIFIED,      // Save only modified brushes
        SAVE_SELECTED       // Save selected brushes
    };

    explicit BrushPersistence(BrushManager* brushManager, QObject* parent = nullptr);
    ~BrushPersistence() override;

    // Main save/load operations
    bool saveBrushes(const QString& filePath, FileFormat format = AUTO_DETECT, SaveMode mode = SAVE_ALL);
    bool loadBrushes(const QString& filePath, FileFormat format = AUTO_DETECT);
    
    // Collection operations
    bool saveBrushCollection(const QString& filePath, const BrushCollectionData& collection, FileFormat format = AUTO_DETECT);
    bool loadBrushCollection(const QString& filePath, BrushCollectionData& collection, FileFormat format = AUTO_DETECT);
    
    // Individual brush operations
    bool saveBrush(const QString& filePath, Brush* brush, FileFormat format = AUTO_DETECT);
    bool loadBrush(const QString& filePath, FileFormat format = AUTO_DETECT);
    
    // Serialization operations
    BrushSerializationData serializeBrush(Brush* brush) const;
    Brush* deserializeBrush(const BrushSerializationData& data) const;
    
    // Format detection and validation
    FileFormat detectFileFormat(const QString& filePath) const;
    bool validateBrushFile(const QString& filePath, FileFormat format = AUTO_DETECT) const;
    
    // Brush state management
    void markBrushAsModified(const QString& brushName);
    void markBrushAsUserDefined(const QString& brushName);
    bool isBrushModified(const QString& brushName) const;
    bool isBrushUserDefined(const QString& brushName) const;
    
    // Collection management
    QStringList getAvailableCollections(const QString& directory) const;
    BrushCollectionData createCollectionFromBrushes(const QStringList& brushNames, const QString& collectionName) const;
    
    // Settings and preferences
    void setDefaultSaveFormat(FileFormat format) { defaultSaveFormat_ = format; }
    FileFormat getDefaultSaveFormat() const { return defaultSaveFormat_; }
    void setAutoBackup(bool enabled) { autoBackupEnabled_ = enabled; }
    bool isAutoBackupEnabled() const { return autoBackupEnabled_; }

signals:
    void brushSaved(const QString& brushName, const QString& filePath);
    void brushLoaded(const QString& brushName, const QString& filePath);
    void collectionSaved(const QString& collectionName, const QString& filePath);
    void collectionLoaded(const QString& collectionName, const QString& filePath);
    void saveProgress(int current, int total, const QString& currentItem);
    void loadProgress(int current, int total, const QString& currentItem);
    void errorOccurred(const QString& error, const QString& details);

private:
    // XML serialization methods
    bool saveToXML(const QString& filePath, const QList<BrushSerializationData>& brushes, const QVariantMap& metadata = QVariantMap());
    bool loadFromXML(const QString& filePath, QList<BrushSerializationData>& brushes, QVariantMap& metadata);
    QDomElement serializeBrushToXML(const BrushSerializationData& data, QDomDocument& doc) const;
    BrushSerializationData deserializeBrushFromXML(const QDomElement& element) const;
    
    // JSON serialization methods
    bool saveToJSON(const QString& filePath, const QList<BrushSerializationData>& brushes, const QVariantMap& metadata = QVariantMap());
    bool loadFromJSON(const QString& filePath, QList<BrushSerializationData>& brushes, QVariantMap& metadata);
    QJsonObject serializeBrushToJSON(const BrushSerializationData& data) const;
    BrushSerializationData deserializeBrushFromJSON(const QJsonObject& object) const;
    
    // Brush data extraction methods
    QVariantMap extractBrushProperties(Brush* brush) const;
    void applyBrushProperties(Brush* brush, const QVariantMap& properties) const;
    QStringList extractBrushDependencies(Brush* brush) const;
    
    // File operations
    bool createBackup(const QString& filePath) const;
    bool restoreBackup(const QString& filePath) const;
    QString generateBackupPath(const QString& filePath) const;
    
    // Validation methods
    bool validateXMLStructure(const QDomDocument& doc) const;
    bool validateJSONStructure(const QJsonDocument& doc) const;
    bool validateBrushData(const BrushSerializationData& data) const;
    
    // Helper methods
    QString brushTypeToString(Brush::Type type) const;
    Brush::Type stringToBrushType(const QString& typeString) const;
    QVariantMap variantMapFromDomElement(const QDomElement& element) const;
    QDomElement variantMapToDomElement(const QVariantMap& map, QDomDocument& doc, const QString& tagName) const;
    
    // Error handling
    void setLastError(const QString& error, const QString& details = QString());
    void clearLastError();

private:
    BrushManager* brushManager_;
    FileFormat defaultSaveFormat_;
    bool autoBackupEnabled_;
    
    // State tracking
    QSet<QString> modifiedBrushes_;
    QSet<QString> userDefinedBrushes_;
    QMap<QString, qint64> brushTimestamps_;
    
    // Error handling
    QString lastError_;
    QString lastErrorDetails_;
    
    // File format signatures
    static const QString XML_ROOT_ELEMENT;
    static const QString JSON_FORMAT_IDENTIFIER;
    static const QString BRUSH_FILE_VERSION;
    
    // Supported brush types for serialization
    static const QStringList SERIALIZABLE_BRUSH_TYPES;
};

/**
 * @brief Brush import/export utility class
 */
class BrushImportExport : public QObject
{
    Q_OBJECT

public:
    explicit BrushImportExport(BrushPersistence* persistence, QObject* parent = nullptr);
    ~BrushImportExport() override;

    // Import operations
    bool importBrushesFromDirectory(const QString& directory, const QStringList& filters = QStringList());
    bool importBrushCollection(const QString& filePath);
    bool importLegacyBrushes(const QString& filePath); // wxwidgets format
    
    // Export operations
    bool exportBrushesToDirectory(const QString& directory, const QStringList& brushNames, BrushPersistence::FileFormat format);
    bool exportBrushCollection(const QString& filePath, const QString& collectionName, const QStringList& brushNames);
    bool exportForLegacy(const QString& filePath, const QStringList& brushNames); // wxwidgets format
    
    // Batch operations
    bool batchImport(const QStringList& filePaths);
    bool batchExport(const QString& directory, const QMap<QString, QStringList>& collections);
    
    // Migration utilities
    bool migrateLegacyBrushes(const QString& legacyDirectory, const QString& targetDirectory);
    QStringList findLegacyBrushFiles(const QString& directory) const;
    
    // Validation and analysis
    QStringList validateImportFiles(const QStringList& filePaths) const;
    QMap<QString, QVariant> analyzeBrushFile(const QString& filePath) const;

signals:
    void importProgress(int current, int total, const QString& currentFile);
    void exportProgress(int current, int total, const QString& currentFile);
    void importCompleted(int successCount, int failureCount);
    void exportCompleted(int successCount, int failureCount);
    void migrationProgress(int current, int total, const QString& currentFile);
    void migrationCompleted(int migratedCount, int skippedCount);

private:
    BrushPersistence* persistence_;
    
    // Import/export state
    int currentOperation_;
    int totalOperations_;
    QStringList failedOperations_;
    
    // Helper methods
    bool processImportFile(const QString& filePath);
    bool processExportBrush(const QString& brushName, const QString& targetPath, BrushPersistence::FileFormat format);
    QString generateUniqueFileName(const QString& basePath, const QString& name, const QString& extension) const;
};

#endif // BRUSHPERSISTENCE_H
