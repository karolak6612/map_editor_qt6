#ifndef TILESETMANAGER_H
#define TILESETMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>
#include <QMutex>
#include <memory>

// Forward declarations
class Brush;
class BrushManager;
class ItemManager;

// Task 67: Tileset category types (from wxwidgets)
enum class TilesetCategoryType : quint8 {
    Unknown = 0,
    Terrain = 1,
    Creature = 2,
    Doodad = 3,
    Collection = 4,
    Item = 5,
    Raw = 6,
    House = 7,
    Waypoint = 8,
    Border = 9,
    Wall = 10
};

// Task 67: Tileset category class
class TilesetCategory {
public:
    explicit TilesetCategory(const QString& name, TilesetCategoryType type);
    ~TilesetCategory();

    // Basic properties
    QString getName() const { return name_; }
    TilesetCategoryType getType() const { return type_; }
    bool isTrivial() const;
    
    // Content management
    void addBrush(Brush* brush);
    void removeBrush(Brush* brush);
    bool containsBrush(Brush* brush) const;
    const QList<Brush*>& getBrushes() const { return brushes_; }
    
    void addItemId(quint16 itemId);
    void removeItemId(quint16 itemId);
    bool containsItemId(quint16 itemId) const;
    const QSet<quint16>& getItemIds() const { return itemIds_; }
    
    void addCreatureName(const QString& creatureName);
    void removeCreatureName(const QString& creatureName);
    bool containsCreatureName(const QString& creatureName) const;
    const QStringList& getCreatureNames() const { return creatureNames_; }
    
    // Size and state
    int size() const;
    bool isEmpty() const;
    void clear();
    
    // XML serialization
    bool loadFromXml(const QDomElement& element, QStringList& warnings);
    bool saveToXml(QDomDocument& doc, QDomElement& parent) const;

private:
    QString name_;
    TilesetCategoryType type_;
    QList<Brush*> brushes_;
    QSet<quint16> itemIds_;
    QStringList creatureNames_;
    
    // Additional metadata for creatures
    QMap<QString, QString> creatureTypes_;     // creature name -> type (monster, npc, etc.)
    QMap<QString, quint16> creatureLookTypes_; // creature name -> looktype ID
};

// Task 67: Tileset class
class Tileset {
public:
    explicit Tileset(const QString& name);
    ~Tileset();

    // Basic properties
    QString getName() const { return name_; }
    void setName(const QString& name) { name_ = name; }
    
    // Category management
    TilesetCategory* getCategory(TilesetCategoryType type);
    const TilesetCategory* getCategory(TilesetCategoryType type) const;
    TilesetCategory* createCategory(const QString& categoryName, TilesetCategoryType type);
    void removeCategory(TilesetCategoryType type);
    const QList<TilesetCategory*>& getCategories() const { return categories_; }
    
    // Content queries
    bool containsBrush(Brush* brush) const;
    bool containsItemId(quint16 itemId) const;
    bool containsCreatureName(const QString& creatureName) const;
    
    // Size and state
    int totalSize() const;
    bool isEmpty() const;
    void clear();
    
    // XML serialization
    bool loadFromXml(const QDomElement& element, QStringList& warnings);
    bool saveToXml(QDomDocument& doc, QDomElement& parent) const;

private:
    QString name_;
    QList<TilesetCategory*> categories_;
    QMap<TilesetCategoryType, TilesetCategory*> categoryMap_;
};

// Task 67: Main tileset manager class
class TilesetManager : public QObject {
    Q_OBJECT

public:
    // Singleton access
    static TilesetManager& instance();
    
    // Initialization and cleanup
    bool initialize(BrushManager* brushManager, ItemManager* itemManager);
    void shutdown();
    bool isInitialized() const { return initialized_; }
    
    // Tileset management
    Tileset* createTileset(const QString& name);
    void removeTileset(const QString& name);
    Tileset* getTileset(const QString& name) const;
    const QStringList getTilesetNames() const;
    const QList<Tileset*>& getTilesets() const { return tilesets_; }
    
    // Category management across all tilesets
    TilesetCategory* createTilesetCategory(const QString& tilesetName, const QString& categoryName, TilesetCategoryType type);
    QList<TilesetCategory*> getCategoriesByType(TilesetCategoryType type) const;
    QStringList getCategoryNames(TilesetCategoryType type) const;
    
    // Content management
    void addItemToCategory(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void addBrushToCategory(const QString& tilesetName, const QString& categoryName, Brush* brush);
    void addCreatureToCategory(const QString& tilesetName, const QString& categoryName, 
                              const QString& creatureName, const QString& type = QString(), quint16 lookType = 0);
    
    // Content queries
    QList<quint16> getItemsInCategory(const QString& tilesetName, const QString& categoryName) const;
    QList<Brush*> getBrushesInCategory(const QString& tilesetName, const QString& categoryName) const;
    QStringList getCreaturesInCategory(const QString& tilesetName, const QString& categoryName) const;
    
    // Global content queries
    bool isInTileset(Brush* brush, const QString& tilesetName) const;
    bool isInTileset(quint16 itemId, const QString& tilesetName) const;
    bool isInTileset(const QString& creatureName, const QString& tilesetName) const;
    
    // Lookup by content
    QStringList getTilesetsContaining(Brush* brush) const;
    QStringList getTilesetsContaining(quint16 itemId) const;
    QStringList getTilesetsContaining(const QString& creatureName) const;
    
    // XML file operations
    bool loadMaterials(const QString& materialsPath, QStringList& errors, QStringList& warnings);
    bool saveMaterials(const QString& materialsPath, QStringList& errors) const;
    bool loadTilesetFile(const QString& filePath, QStringList& errors, QStringList& warnings);
    bool saveTilesetFile(const QString& filePath, const QString& tilesetName, QStringList& errors) const;
    
    // Utility methods
    void clear();
    int getTotalTilesets() const { return tilesets_.size(); }
    int getTotalCategories() const;
    int getTotalItems() const;
    
    // Category type utilities
    static QString categoryTypeToString(TilesetCategoryType type);
    static TilesetCategoryType stringToCategoryType(const QString& typeStr);
    static QStringList getAllCategoryTypeNames();

signals:
    void tilesetAdded(const QString& tilesetName);
    void tilesetRemoved(const QString& tilesetName);
    void tilesetChanged(const QString& tilesetName);
    void categoryAdded(const QString& tilesetName, const QString& categoryName);
    void categoryRemoved(const QString& tilesetName, const QString& categoryName);
    void categoryChanged(const QString& tilesetName, const QString& categoryName);

private:
    // Private constructor for singleton
    explicit TilesetManager(QObject* parent = nullptr);
    ~TilesetManager();
    
    // Disable copy and assignment
    TilesetManager(const TilesetManager&) = delete;
    TilesetManager& operator=(const TilesetManager&) = delete;
    
    // Internal helper methods
    void connectTilesetSignals(Tileset* tileset);
    void disconnectTilesetSignals(Tileset* tileset);
    bool loadIncludeFile(const QString& basePath, const QString& includeFile, QStringList& errors, QStringList& warnings);
    bool processTilesetElement(const QDomElement& tilesetElement, QStringList& warnings);
    bool processCategoryElement(Tileset* tileset, const QDomElement& categoryElement, 
                               TilesetCategoryType categoryType, QStringList& warnings);
    
    // Member variables
    bool initialized_;
    BrushManager* brushManager_;
    ItemManager* itemManager_;
    
    QList<Tileset*> tilesets_;
    QMap<QString, Tileset*> tilesetMap_;
    
    // Thread safety
    mutable QMutex mutex_;
    
    // Statistics
    mutable int totalCategories_;
    mutable int totalItems_;
    mutable bool statisticsValid_;
    
    void invalidateStatistics() const;
    void updateStatistics() const;

private slots:
    void onBrushCreated(Brush* brush);
    void onBrushDestroyed(Brush* brush);
};

// Task 67: Utility functions
namespace TilesetUtils {
    // Category type conversion utilities
    QString categoryTypeToDisplayName(TilesetCategoryType type);
    QIcon getCategoryIcon(TilesetCategoryType type);
    QColor getCategoryColor(TilesetCategoryType type);
    
    // Validation utilities
    bool isValidTilesetName(const QString& name);
    bool isValidCategoryName(const QString& name);
    QString sanitizeTilesetName(const QString& name);
    QString sanitizeCategoryName(const QString& name);
}

#endif // TILESETMANAGER_H
