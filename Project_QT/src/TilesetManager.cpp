#include "TilesetManager.h"
#include "Brush.h"
#include "BrushManager.h"
#include "ItemManager.h"
#include "ResourceManager.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QMutexLocker>
#include <QIcon>
#include <QColor>
#include <QRegularExpression>

// TilesetCategory implementation
TilesetCategory::TilesetCategory(const QString& name, TilesetCategoryType type)
    : name_(name), type_(type) {
}

TilesetCategory::~TilesetCategory() {
    clear();
}

bool TilesetCategory::isTrivial() const {
    return (type_ == TilesetCategoryType::Item) || (type_ == TilesetCategoryType::Raw);
}

void TilesetCategory::addBrush(Brush* brush) {
    if (brush && !brushes_.contains(brush)) {
        brushes_.append(brush);
    }
}

void TilesetCategory::removeBrush(Brush* brush) {
    brushes_.removeAll(brush);
}

bool TilesetCategory::containsBrush(Brush* brush) const {
    return brushes_.contains(brush);
}

void TilesetCategory::addItemId(quint16 itemId) {
    if (itemId > 0) {
        itemIds_.insert(itemId);
    }
}

void TilesetCategory::removeItemId(quint16 itemId) {
    itemIds_.remove(itemId);
}

bool TilesetCategory::containsItemId(quint16 itemId) const {
    return itemIds_.contains(itemId);
}

void TilesetCategory::addCreatureName(const QString& creatureName) {
    if (!creatureName.isEmpty() && !creatureNames_.contains(creatureName)) {
        creatureNames_.append(creatureName);
    }
}

void TilesetCategory::removeCreatureName(const QString& creatureName) {
    creatureNames_.removeAll(creatureName);
    creatureTypes_.remove(creatureName);
    creatureLookTypes_.remove(creatureName);
}

bool TilesetCategory::containsCreatureName(const QString& creatureName) const {
    return creatureNames_.contains(creatureName);
}

int TilesetCategory::size() const {
    return brushes_.size() + itemIds_.size() + creatureNames_.size();
}

bool TilesetCategory::isEmpty() const {
    return brushes_.isEmpty() && itemIds_.isEmpty() && creatureNames_.isEmpty();
}

void TilesetCategory::clear() {
    brushes_.clear();
    itemIds_.clear();
    creatureNames_.clear();
    creatureTypes_.clear();
    creatureLookTypes_.clear();
}

bool TilesetCategory::loadFromXml(const QDomElement& element, QStringList& warnings) {
    QString categoryName = element.tagName();
    
    // Load brushes
    QDomNodeList brushNodes = element.elementsByTagName("brush");
    for (int i = 0; i < brushNodes.count(); ++i) {
        QDomElement brushElement = brushNodes.at(i).toElement();
        if (!brushElement.isNull()) {
            QString brushName = brushElement.attribute("name");
            if (!brushName.isEmpty()) {
                // Note: Actual brush lookup would be done during integration
                // For now, we store the brush name as metadata
                qDebug() << "TilesetCategory: Found brush reference:" << brushName;
            }
        }
    }
    
    // Load items
    QDomNodeList itemNodes = element.elementsByTagName("item");
    for (int i = 0; i < itemNodes.count(); ++i) {
        QDomElement itemElement = itemNodes.at(i).toElement();
        if (!itemElement.isNull()) {
            bool ok;
            quint16 itemId = itemElement.attribute("id").toUShort(&ok);
            if (ok && itemId > 0) {
                addItemId(itemId);
            } else {
                warnings << QString("Invalid item ID in category %1: %2").arg(name_).arg(itemElement.attribute("id"));
            }
        }
    }
    
    // Load creatures
    QDomNodeList creatureNodes = element.elementsByTagName("creature");
    for (int i = 0; i < creatureNodes.count(); ++i) {
        QDomElement creatureElement = creatureNodes.at(i).toElement();
        if (!creatureElement.isNull()) {
            QString creatureName = creatureElement.attribute("name");
            QString creatureType = creatureElement.attribute("type");
            bool ok;
            quint16 lookType = creatureElement.attribute("looktype").toUShort(&ok);
            
            if (!creatureName.isEmpty()) {
                addCreatureName(creatureName);
                if (!creatureType.isEmpty()) {
                    creatureTypes_[creatureName] = creatureType;
                }
                if (ok && lookType > 0) {
                    creatureLookTypes_[creatureName] = lookType;
                }
            } else {
                warnings << QString("Empty creature name in category %1").arg(name_);
            }
        }
    }
    
    return true;
}

bool TilesetCategory::saveToXml(QDomDocument& doc, QDomElement& parent) const {
    QDomElement categoryElement = doc.createElement(TilesetManager::categoryTypeToString(type_).toLower());
    
    // Save brushes
    for (Brush* brush : brushes_) {
        if (brush) {
            QDomElement brushElement = doc.createElement("brush");
            brushElement.setAttribute("name", brush->getName());
            categoryElement.appendChild(brushElement);
        }
    }
    
    // Save items
    for (quint16 itemId : itemIds_) {
        QDomElement itemElement = doc.createElement("item");
        itemElement.setAttribute("id", itemId);
        categoryElement.appendChild(itemElement);
    }
    
    // Save creatures
    for (const QString& creatureName : creatureNames_) {
        QDomElement creatureElement = doc.createElement("creature");
        creatureElement.setAttribute("name", creatureName);
        
        if (creatureTypes_.contains(creatureName)) {
            creatureElement.setAttribute("type", creatureTypes_[creatureName]);
        }
        if (creatureLookTypes_.contains(creatureName)) {
            creatureElement.setAttribute("looktype", creatureLookTypes_[creatureName]);
        }
        
        categoryElement.appendChild(creatureElement);
    }
    
    parent.appendChild(categoryElement);
    return true;
}

// Tileset implementation
Tileset::Tileset(const QString& name) : name_(name) {
}

Tileset::~Tileset() {
    clear();
}

TilesetCategory* Tileset::getCategory(TilesetCategoryType type) {
    auto it = categoryMap_.find(type);
    if (it != categoryMap_.end()) {
        return it.value();
    }
    
    // Create new category if it doesn't exist
    QString categoryName = TilesetManager::categoryTypeToString(type);
    TilesetCategory* category = new TilesetCategory(categoryName, type);
    categories_.append(category);
    categoryMap_[type] = category;
    
    return category;
}

const TilesetCategory* Tileset::getCategory(TilesetCategoryType type) const {
    auto it = categoryMap_.find(type);
    return (it != categoryMap_.end()) ? it.value() : nullptr;
}

TilesetCategory* Tileset::createCategory(const QString& categoryName, TilesetCategoryType type) {
    // Remove existing category of this type if it exists
    removeCategory(type);
    
    TilesetCategory* category = new TilesetCategory(categoryName, type);
    categories_.append(category);
    categoryMap_[type] = category;
    
    return category;
}

void Tileset::removeCategory(TilesetCategoryType type) {
    auto it = categoryMap_.find(type);
    if (it != categoryMap_.end()) {
        TilesetCategory* category = it.value();
        categories_.removeAll(category);
        categoryMap_.erase(it);
        delete category;
    }
}

bool Tileset::containsBrush(Brush* brush) const {
    for (const TilesetCategory* category : categories_) {
        if (category && category->containsBrush(brush)) {
            return true;
        }
    }
    return false;
}

bool Tileset::containsItemId(quint16 itemId) const {
    for (const TilesetCategory* category : categories_) {
        if (category && category->containsItemId(itemId)) {
            return true;
        }
    }
    return false;
}

bool Tileset::containsCreatureName(const QString& creatureName) const {
    for (const TilesetCategory* category : categories_) {
        if (category && category->containsCreatureName(creatureName)) {
            return true;
        }
    }
    return false;
}

int Tileset::totalSize() const {
    int total = 0;
    for (const TilesetCategory* category : categories_) {
        if (category) {
            total += category->size();
        }
    }
    return total;
}

bool Tileset::isEmpty() const {
    for (const TilesetCategory* category : categories_) {
        if (category && !category->isEmpty()) {
            return false;
        }
    }
    return true;
}

void Tileset::clear() {
    for (TilesetCategory* category : categories_) {
        delete category;
    }
    categories_.clear();
    categoryMap_.clear();
}

bool Tileset::loadFromXml(const QDomElement& element, QStringList& warnings) {
    name_ = element.attribute("name", name_);
    
    // Load different category types
    QStringList categoryTypes = {"terrain", "creatures", "doodads", "items", "raw", "house", "waypoint", "borders", "walls"};
    
    for (const QString& categoryTypeName : categoryTypes) {
        QDomNodeList categoryNodes = element.elementsByTagName(categoryTypeName);
        for (int i = 0; i < categoryNodes.count(); ++i) {
            QDomElement categoryElement = categoryNodes.at(i).toElement();
            if (!categoryElement.isNull()) {
                TilesetCategoryType categoryType = TilesetManager::stringToCategoryType(categoryTypeName);
                TilesetCategory* category = getCategory(categoryType);
                if (category) {
                    category->loadFromXml(categoryElement, warnings);
                }
            }
        }
    }
    
    return true;
}

bool Tileset::saveToXml(QDomDocument& doc, QDomElement& parent) const {
    QDomElement tilesetElement = doc.createElement("tileset");
    tilesetElement.setAttribute("name", name_);
    
    // Save all categories
    for (const TilesetCategory* category : categories_) {
        if (category && !category->isEmpty()) {
            category->saveToXml(doc, tilesetElement);
        }
    }
    
    parent.appendChild(tilesetElement);
    return true;
}

// TilesetManager implementation
TilesetManager::TilesetManager(QObject* parent)
    : QObject(parent)
    , initialized_(false)
    , brushManager_(nullptr)
    , itemManager_(nullptr)
    , totalCategories_(0)
    , totalItems_(0)
    , statisticsValid_(false) {
}

TilesetManager::~TilesetManager() {
    shutdown();
}

TilesetManager& TilesetManager::instance() {
    static TilesetManager instance;
    return instance;
}

bool TilesetManager::initialize(BrushManager* brushManager, ItemManager* itemManager) {
    QMutexLocker locker(&mutex_);
    
    if (initialized_) {
        qWarning() << "TilesetManager: Already initialized";
        return true;
    }
    
    if (!brushManager || !itemManager) {
        qCritical() << "TilesetManager: Invalid managers provided";
        return false;
    }
    
    brushManager_ = brushManager;
    itemManager_ = itemManager;
    
    // Connect to brush manager signals
    connect(brushManager_, &BrushManager::brushCreated, this, &TilesetManager::onBrushCreated);
    connect(brushManager_, &BrushManager::brushDestroyed, this, &TilesetManager::onBrushDestroyed);
    
    initialized_ = true;
    invalidateStatistics();
    
    qDebug() << "TilesetManager: Initialized successfully";
    return true;
}

void TilesetManager::shutdown() {
    QMutexLocker locker(&mutex_);
    
    if (!initialized_) {
        return;
    }
    
    // Disconnect signals
    if (brushManager_) {
        disconnect(brushManager_, nullptr, this, nullptr);
    }
    
    // Clear all data
    clear();
    
    brushManager_ = nullptr;
    itemManager_ = nullptr;
    initialized_ = false;
    
    qDebug() << "TilesetManager: Shutdown completed";
}

Tileset* TilesetManager::createTileset(const QString& name) {
    QMutexLocker locker(&mutex_);
    
    if (name.isEmpty()) {
        qWarning() << "TilesetManager: Cannot create tileset with empty name";
        return nullptr;
    }
    
    if (tilesetMap_.contains(name)) {
        qWarning() << "TilesetManager: Tileset already exists:" << name;
        return tilesetMap_[name];
    }
    
    Tileset* tileset = new Tileset(name);
    tilesets_.append(tileset);
    tilesetMap_[name] = tileset;
    
    invalidateStatistics();
    emit tilesetAdded(name);
    
    qDebug() << "TilesetManager: Created tileset:" << name;
    return tileset;
}

void TilesetManager::removeTileset(const QString& name) {
    QMutexLocker locker(&mutex_);
    
    auto it = tilesetMap_.find(name);
    if (it == tilesetMap_.end()) {
        qWarning() << "TilesetManager: Tileset not found:" << name;
        return;
    }
    
    Tileset* tileset = it.value();
    tilesets_.removeAll(tileset);
    tilesetMap_.erase(it);
    
    delete tileset;
    
    invalidateStatistics();
    emit tilesetRemoved(name);
    
    qDebug() << "TilesetManager: Removed tileset:" << name;
}

Tileset* TilesetManager::getTileset(const QString& name) const {
    QMutexLocker locker(&mutex_);
    
    auto it = tilesetMap_.find(name);
    return (it != tilesetMap_.end()) ? it.value() : nullptr;
}

const QStringList TilesetManager::getTilesetNames() const {
    QMutexLocker locker(&mutex_);
    return tilesetMap_.keys();
}

void TilesetManager::clear() {
    for (Tileset* tileset : tilesets_) {
        delete tileset;
    }
    tilesets_.clear();
    tilesetMap_.clear();
    invalidateStatistics();
}

void TilesetManager::invalidateStatistics() const {
    statisticsValid_ = false;
}

void TilesetManager::updateStatistics() const {
    if (statisticsValid_) {
        return;
    }
    
    totalCategories_ = 0;
    totalItems_ = 0;
    
    for (const Tileset* tileset : tilesets_) {
        if (tileset) {
            totalCategories_ += tileset->getCategories().size();
            totalItems_ += tileset->totalSize();
        }
    }
    
    statisticsValid_ = true;
}

int TilesetManager::getTotalCategories() const {
    QMutexLocker locker(&mutex_);
    updateStatistics();
    return totalCategories_;
}

int TilesetManager::getTotalItems() const {
    QMutexLocker locker(&mutex_);
    updateStatistics();
    return totalItems_;
}

// Category type utilities
QString TilesetManager::categoryTypeToString(TilesetCategoryType type) {
    switch (type) {
        case TilesetCategoryType::Unknown: return "Unknown";
        case TilesetCategoryType::Terrain: return "Terrain";
        case TilesetCategoryType::Creature: return "Creatures";
        case TilesetCategoryType::Doodad: return "Doodads";
        case TilesetCategoryType::Collection: return "Collections";
        case TilesetCategoryType::Item: return "Items";
        case TilesetCategoryType::Raw: return "Raw";
        case TilesetCategoryType::House: return "House";
        case TilesetCategoryType::Waypoint: return "Waypoint";
        case TilesetCategoryType::Border: return "Borders";
        case TilesetCategoryType::Wall: return "Walls";
        default: return "Unknown";
    }
}

TilesetCategoryType TilesetManager::stringToCategoryType(const QString& typeStr) {
    QString lower = typeStr.toLower();
    if (lower == "terrain") return TilesetCategoryType::Terrain;
    if (lower == "creatures" || lower == "creature") return TilesetCategoryType::Creature;
    if (lower == "doodads" || lower == "doodad") return TilesetCategoryType::Doodad;
    if (lower == "collections" || lower == "collection") return TilesetCategoryType::Collection;
    if (lower == "items" || lower == "item") return TilesetCategoryType::Item;
    if (lower == "raw") return TilesetCategoryType::Raw;
    if (lower == "house") return TilesetCategoryType::House;
    if (lower == "waypoint") return TilesetCategoryType::Waypoint;
    if (lower == "borders" || lower == "border") return TilesetCategoryType::Border;
    if (lower == "walls" || lower == "wall") return TilesetCategoryType::Wall;
    return TilesetCategoryType::Unknown;
}

QStringList TilesetManager::getAllCategoryTypeNames() {
    return {
        "Terrain", "Creatures", "Doodads", "Collections", "Items", 
        "Raw", "House", "Waypoint", "Borders", "Walls"
    };
}

// Slots
void TilesetManager::onBrushCreated(Brush* brush) {
    // Auto-categorize brush based on its type
    if (!brush) return;
    
    // This would be implemented based on brush properties
    qDebug() << "TilesetManager: New brush created:" << brush->getName();
}

void TilesetManager::onBrushDestroyed(Brush* brush) {
    // Remove brush from all categories
    QMutexLocker locker(&mutex_);
    
    for (Tileset* tileset : tilesets_) {
        if (tileset) {
            for (TilesetCategory* category : tileset->getCategories()) {
                if (category) {
                    category->removeBrush(brush);
                }
            }
        }
    }
    
    invalidateStatistics();
}

// TilesetUtils namespace implementation
namespace TilesetUtils {

QString categoryTypeToDisplayName(TilesetCategoryType type) {
    return TilesetManager::categoryTypeToString(type);
}

QIcon getCategoryIcon(TilesetCategoryType type) {
    QString iconName;
    switch (type) {
        case TilesetCategoryType::Terrain: iconName = "terrain"; break;
        case TilesetCategoryType::Creature: iconName = "creature"; break;
        case TilesetCategoryType::Doodad: iconName = "doodad"; break;
        case TilesetCategoryType::Item: iconName = "item"; break;
        case TilesetCategoryType::House: iconName = "house"; break;
        default: iconName = "unknown"; break;
    }
    
    return ResourceManager::instance().getIcon(iconName + ".png");
}

QColor getCategoryColor(TilesetCategoryType type) {
    switch (type) {
        case TilesetCategoryType::Terrain: return QColor(139, 69, 19);   // Brown
        case TilesetCategoryType::Creature: return QColor(255, 165, 0);  // Orange
        case TilesetCategoryType::Doodad: return QColor(0, 128, 0);      // Green
        case TilesetCategoryType::Item: return QColor(0, 0, 255);        // Blue
        case TilesetCategoryType::House: return QColor(255, 0, 255);     // Magenta
        default: return QColor(128, 128, 128);                           // Gray
    }
}

bool isValidTilesetName(const QString& name) {
    static const QRegularExpression invalidChars("[<>:\"/\\\\|?*]");
    return !name.isEmpty() && !name.contains(invalidChars);
}

bool isValidCategoryName(const QString& name) {
    static const QRegularExpression invalidChars("[<>:\"/\\\\|?*]");
    return !name.isEmpty() && !name.contains(invalidChars);
}

QString sanitizeTilesetName(const QString& name) {
    static const QRegularExpression invalidChars("[<>:\"/\\\\|?*]");
    QString sanitized = name;
    sanitized.remove(invalidChars);
    return sanitized.trimmed();
}

QString sanitizeCategoryName(const QString& name) {
    static const QRegularExpression invalidChars("[<>:\"/\\\\|?*]");
    QString sanitized = name;
    sanitized.remove(invalidChars);
    return sanitized.trimmed();
}

} // namespace TilesetUtils

// Additional TilesetManager methods implementation

TilesetCategory* TilesetManager::createTilesetCategory(const QString& tilesetName, const QString& categoryName, TilesetCategoryType type) {
    QMutexLocker locker(&mutex_);

    Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        tileset = createTileset(tilesetName);
    }

    TilesetCategory* category = tileset->createCategory(categoryName, type);
    invalidateStatistics();
    emit categoryAdded(tilesetName, categoryName);

    return category;
}

QList<TilesetCategory*> TilesetManager::getCategoriesByType(TilesetCategoryType type) const {
    QMutexLocker locker(&mutex_);

    QList<TilesetCategory*> categories;
    for (const Tileset* tileset : tilesets_) {
        if (tileset) {
            const TilesetCategory* category = tileset->getCategory(type);
            if (category) {
                categories.append(const_cast<TilesetCategory*>(category));
            }
        }
    }
    return categories;
}

QStringList TilesetManager::getCategoryNames(TilesetCategoryType type) const {
    QMutexLocker locker(&mutex_);

    QStringList names;
    for (const Tileset* tileset : tilesets_) {
        if (tileset) {
            const TilesetCategory* category = tileset->getCategory(type);
            if (category) {
                names.append(category->getName());
            }
        }
    }
    return names;
}

void TilesetManager::addItemToCategory(const QString& tilesetName, const QString& categoryName, quint16 itemId) {
    QMutexLocker locker(&mutex_);

    Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        qWarning() << "TilesetManager: Tileset not found:" << tilesetName;
        return;
    }

    // Find category by name
    TilesetCategory* targetCategory = nullptr;
    for (TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            targetCategory = category;
            break;
        }
    }

    if (!targetCategory) {
        qWarning() << "TilesetManager: Category not found:" << categoryName << "in tileset:" << tilesetName;
        return;
    }

    targetCategory->addItemId(itemId);
    invalidateStatistics();
    emit categoryChanged(tilesetName, categoryName);
}

void TilesetManager::addBrushToCategory(const QString& tilesetName, const QString& categoryName, Brush* brush) {
    QMutexLocker locker(&mutex_);

    Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        qWarning() << "TilesetManager: Tileset not found:" << tilesetName;
        return;
    }

    // Find category by name
    TilesetCategory* targetCategory = nullptr;
    for (TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            targetCategory = category;
            break;
        }
    }

    if (!targetCategory) {
        qWarning() << "TilesetManager: Category not found:" << categoryName << "in tileset:" << tilesetName;
        return;
    }

    targetCategory->addBrush(brush);
    invalidateStatistics();
    emit categoryChanged(tilesetName, categoryName);
}

void TilesetManager::addCreatureToCategory(const QString& tilesetName, const QString& categoryName,
                                          const QString& creatureName, const QString& type, quint16 lookType) {
    QMutexLocker locker(&mutex_);

    Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        qWarning() << "TilesetManager: Tileset not found:" << tilesetName;
        return;
    }

    // Find category by name
    TilesetCategory* targetCategory = nullptr;
    for (TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            targetCategory = category;
            break;
        }
    }

    if (!targetCategory) {
        qWarning() << "TilesetManager: Category not found:" << categoryName << "in tileset:" << tilesetName;
        return;
    }

    targetCategory->addCreatureName(creatureName);
    invalidateStatistics();
    emit categoryChanged(tilesetName, categoryName);
}

QList<quint16> TilesetManager::getItemsInCategory(const QString& tilesetName, const QString& categoryName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        return QList<quint16>();
    }

    for (const TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            return category->getItemIds().toList();
        }
    }

    return QList<quint16>();
}

QList<Brush*> TilesetManager::getBrushesInCategory(const QString& tilesetName, const QString& categoryName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        return QList<Brush*>();
    }

    for (const TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            return category->getBrushes();
        }
    }

    return QList<Brush*>();
}

QStringList TilesetManager::getCreaturesInCategory(const QString& tilesetName, const QString& categoryName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        return QStringList();
    }

    for (const TilesetCategory* category : tileset->getCategories()) {
        if (category && category->getName() == categoryName) {
            return category->getCreatureNames();
        }
    }

    return QStringList();
}

bool TilesetManager::isInTileset(Brush* brush, const QString& tilesetName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    return tileset ? tileset->containsBrush(brush) : false;
}

bool TilesetManager::isInTileset(quint16 itemId, const QString& tilesetName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    return tileset ? tileset->containsItemId(itemId) : false;
}

bool TilesetManager::isInTileset(const QString& creatureName, const QString& tilesetName) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    return tileset ? tileset->containsCreatureName(creatureName) : false;
}

QStringList TilesetManager::getTilesetsContaining(Brush* brush) const {
    QMutexLocker locker(&mutex_);

    QStringList tilesetNames;
    for (const Tileset* tileset : tilesets_) {
        if (tileset && tileset->containsBrush(brush)) {
            tilesetNames.append(tileset->getName());
        }
    }
    return tilesetNames;
}

QStringList TilesetManager::getTilesetsContaining(quint16 itemId) const {
    QMutexLocker locker(&mutex_);

    QStringList tilesetNames;
    for (const Tileset* tileset : tilesets_) {
        if (tileset && tileset->containsItemId(itemId)) {
            tilesetNames.append(tileset->getName());
        }
    }
    return tilesetNames;
}

QStringList TilesetManager::getTilesetsContaining(const QString& creatureName) const {
    QMutexLocker locker(&mutex_);

    QStringList tilesetNames;
    for (const Tileset* tileset : tilesets_) {
        if (tileset && tileset->containsCreatureName(creatureName)) {
            tilesetNames.append(tileset->getName());
        }
    }
    return tilesetNames;
}

// XML file operations implementation
bool TilesetManager::loadMaterials(const QString& materialsPath, QStringList& errors, QStringList& warnings) {
    QMutexLocker locker(&mutex_);

    QFile file(materialsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << QString("Cannot open materials file: %1").arg(materialsPath);
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        errors << QString("XML parse error in %1 at line %2, column %3: %4")
                  .arg(materialsPath).arg(errorLine).arg(errorColumn).arg(errorMsg);
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "materials") {
        errors << QString("Invalid materials XML file: root element should be 'materials'");
        return false;
    }

    QString basePath = QFileInfo(materialsPath).absolutePath();

    // Process include files
    QDomNodeList includeNodes = root.elementsByTagName("include");
    for (int i = 0; i < includeNodes.count(); ++i) {
        QDomElement includeElement = includeNodes.at(i).toElement();
        if (!includeElement.isNull()) {
            QString includeFile = includeElement.attribute("file");
            if (!includeFile.isEmpty()) {
                if (!loadIncludeFile(basePath, includeFile, errors, warnings)) {
                    qWarning() << "Failed to load include file:" << includeFile;
                }
            }
        }
    }

    // Process direct tileset elements
    QDomNodeList tilesetNodes = root.elementsByTagName("tileset");
    for (int i = 0; i < tilesetNodes.count(); ++i) {
        QDomElement tilesetElement = tilesetNodes.at(i).toElement();
        if (!tilesetElement.isNull()) {
            processTilesetElement(tilesetElement, warnings);
        }
    }

    invalidateStatistics();
    qDebug() << "TilesetManager: Loaded materials from" << materialsPath;
    return true;
}

bool TilesetManager::saveMaterials(const QString& materialsPath, QStringList& errors) const {
    QMutexLocker locker(&mutex_);

    QDomDocument doc;
    QDomElement root = doc.createElement("materials");
    doc.appendChild(root);

    // Save all tilesets
    for (const Tileset* tileset : tilesets_) {
        if (tileset && !tileset->isEmpty()) {
            tileset->saveToXml(doc, root);
        }
    }

    // Write to file
    QFile file(materialsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errors << QString("Cannot write materials file: %1").arg(materialsPath);
        return false;
    }

    QTextStream stream(&file);
    stream << doc.toString(4); // 4-space indentation
    file.close();

    qDebug() << "TilesetManager: Saved materials to" << materialsPath;
    return true;
}

bool TilesetManager::loadTilesetFile(const QString& filePath, QStringList& errors, QStringList& warnings) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << QString("Cannot open tileset file: %1").arg(filePath);
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        errors << QString("XML parse error in %1 at line %2, column %3: %4")
                  .arg(filePath).arg(errorLine).arg(errorColumn).arg(errorMsg);
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "materials") {
        errors << QString("Invalid tileset XML file: root element should be 'materials'");
        return false;
    }

    // Process tileset elements
    QDomNodeList tilesetNodes = root.elementsByTagName("tileset");
    for (int i = 0; i < tilesetNodes.count(); ++i) {
        QDomElement tilesetElement = tilesetNodes.at(i).toElement();
        if (!tilesetElement.isNull()) {
            processTilesetElement(tilesetElement, warnings);
        }
    }

    invalidateStatistics();
    qDebug() << "TilesetManager: Loaded tileset file" << filePath;
    return true;
}

bool TilesetManager::saveTilesetFile(const QString& filePath, const QString& tilesetName, QStringList& errors) const {
    QMutexLocker locker(&mutex_);

    const Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        errors << QString("Tileset not found: %1").arg(tilesetName);
        return false;
    }

    QDomDocument doc;
    QDomElement root = doc.createElement("materials");
    doc.appendChild(root);

    tileset->saveToXml(doc, root);

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errors << QString("Cannot write tileset file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream << doc.toString(4); // 4-space indentation
    file.close();

    qDebug() << "TilesetManager: Saved tileset" << tilesetName << "to" << filePath;
    return true;
}

// Internal helper methods
bool TilesetManager::loadIncludeFile(const QString& basePath, const QString& includeFile, QStringList& errors, QStringList& warnings) {
    QString fullPath = QDir(basePath).absoluteFilePath(includeFile);
    return loadTilesetFile(fullPath, errors, warnings);
}

bool TilesetManager::processTilesetElement(const QDomElement& tilesetElement, QStringList& warnings) {
    QString tilesetName = tilesetElement.attribute("name");
    if (tilesetName.isEmpty()) {
        warnings << "Tileset element missing name attribute";
        return false;
    }

    Tileset* tileset = getTileset(tilesetName);
    if (!tileset) {
        tileset = createTileset(tilesetName);
    }

    return tileset->loadFromXml(tilesetElement, warnings);
}
