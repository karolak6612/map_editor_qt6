#ifndef ITEMPROPERTYMATCHER_H
#define ITEMPROPERTYMATCHER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QRegularExpression>
#include <QList>
#include <QSet>
#include <functional>

/**
 * @brief Task 89: Item Property Matcher for advanced property-based searching
 * 
 * Complete property matching system:
 * - Advanced property comparison and filtering
 * - Support for all item attributes and flags
 * - Regular expression matching for text properties
 * - Numeric range matching for numeric properties
 * - Boolean logic for complex criteria
 * - Integration with ItemManager and ItemTypes
 * - 1:1 compatibility with wxwidgets property matching
 */

/**
 * @brief Property Match Criteria
 */
struct PropertyMatchCriteria {
    enum MatchType {
        EXACT_MATCH,        // Exact value match
        CONTAINS,           // String contains substring
        STARTS_WITH,        // String starts with prefix
        ENDS_WITH,          // String ends with suffix
        REGEX_MATCH,        // Regular expression match
        NUMERIC_RANGE,      // Numeric value in range
        GREATER_THAN,       // Numeric value greater than
        LESS_THAN,          // Numeric value less than
        BOOLEAN_MATCH,      // Boolean value match
        LIST_CONTAINS,      // List contains value
        CUSTOM_FUNCTION     // Custom matching function
    };
    
    QString propertyName;
    MatchType matchType = EXACT_MATCH;
    QVariant expectedValue;
    QVariant minValue;
    QVariant maxValue;
    QRegularExpression regex;
    std::function<bool(const QVariant&)> customFunction;
    bool caseSensitive = false;
    bool wholeWordOnly = false;
    
    PropertyMatchCriteria() = default;
    PropertyMatchCriteria(const QString& name, const QVariant& value, MatchType type = EXACT_MATCH);
    
    bool matches(const QVariant& actualValue) const;
    QString getDescription() const;
};

/**
 * @brief Complex Property Criteria with boolean logic
 */
struct ComplexPropertyCriteria {
    enum LogicOperator {
        AND,    // All criteria must match
        OR,     // Any criteria must match
        NOT,    // Criteria must not match
        XOR     // Exactly one criteria must match
    };
    
    LogicOperator logicOperator = AND;
    QList<PropertyMatchCriteria> criteria;
    QList<ComplexPropertyCriteria> subCriteria;
    
    ComplexPropertyCriteria() = default;
    
    bool matches(const QVariantMap& properties) const;
    QString getDescription() const;
    void addCriteria(const PropertyMatchCriteria& criteria);
    void addSubCriteria(const ComplexPropertyCriteria& subCriteria);
};

/**
 * @brief Item Property Matcher
 */
class ItemPropertyMatcher : public QObject
{
    Q_OBJECT

public:
    explicit ItemPropertyMatcher(QObject* parent = nullptr);
    ~ItemPropertyMatcher() override = default;

    // Configuration
    void setItemManager(class ItemManager* itemManager);
    class ItemManager* getItemManager() const { return itemManager_; }

    // Basic property matching
    bool matchesProperty(class Item* item, const QString& propertyName, const QVariant& expectedValue) const;
    bool matchesProperties(class Item* item, const QVariantMap& expectedProperties) const;
    bool matchesCriteria(class Item* item, const PropertyMatchCriteria& criteria) const;
    bool matchesComplexCriteria(class Item* item, const ComplexPropertyCriteria& criteria) const;

    // Advanced matching
    bool matchesItemFlags(class Item* item, const QVariantMap& flagCriteria) const;
    bool matchesItemAttributes(class Item* item, const QVariantMap& attributeCriteria) const;
    bool matchesItemType(class Item* item, const QString& typeName) const;
    bool matchesItemCategory(class Item* item, const QString& categoryName) const;

    // Specialized matching methods
    bool isMoveable(class Item* item) const;
    bool isBlocking(class Item* item) const;
    bool isStackable(class Item* item) const;
    bool isPickupable(class Item* item) const;
    bool isReadable(class Item* item) const;
    bool isWriteable(class Item* item) const;
    bool isRotateable(class Item* item) const;
    bool isHangable(class Item* item) const;
    bool isVertical(class Item* item) const;
    bool isHorizontal(class Item* item) const;
    bool isContainer(class Item* item) const;
    bool isDoor(class Item* item) const;
    bool isGround(class Item* item) const;
    bool isWall(class Item* item) const;
    bool isDecoration(class Item* item) const;

    // Property extraction
    QVariantMap getItemProperties(class Item* item) const;
    QVariantMap getItemFlags(class Item* item) const;
    QVariantMap getItemAttributes(class Item* item) const;
    QString getItemTypeName(class Item* item) const;
    QStringList getItemCategories(class Item* item) const;

    // Property validation
    bool isValidProperty(const QString& propertyName) const;
    bool isValidPropertyValue(const QString& propertyName, const QVariant& value) const;
    QStringList getAvailableProperties() const;
    QStringList getAvailableFlags() const;
    QStringList getAvailableAttributes() const;

    // Criteria building helpers
    PropertyMatchCriteria createExactMatch(const QString& propertyName, const QVariant& value) const;
    PropertyMatchCriteria createContainsMatch(const QString& propertyName, const QString& substring, bool caseSensitive = false) const;
    PropertyMatchCriteria createRegexMatch(const QString& propertyName, const QString& pattern) const;
    PropertyMatchCriteria createRangeMatch(const QString& propertyName, const QVariant& minValue, const QVariant& maxValue) const;
    PropertyMatchCriteria createBooleanMatch(const QString& propertyName, bool expectedValue) const;

    // Complex criteria building
    ComplexPropertyCriteria createAndCriteria(const QList<PropertyMatchCriteria>& criteria) const;
    ComplexPropertyCriteria createOrCriteria(const QList<PropertyMatchCriteria>& criteria) const;
    ComplexPropertyCriteria createNotCriteria(const PropertyMatchCriteria& criteria) const;

signals:
    void propertyMatchFound(class Item* item, const QString& propertyName, const QVariant& value);
    void complexMatchFound(class Item* item, const ComplexPropertyCriteria& criteria);

private:
    // Core matching implementation
    bool matchesExactValue(const QVariant& actual, const QVariant& expected) const;
    bool matchesStringContains(const QString& actual, const QString& expected, bool caseSensitive) const;
    bool matchesStringStartsWith(const QString& actual, const QString& expected, bool caseSensitive) const;
    bool matchesStringEndsWith(const QString& actual, const QString& expected, bool caseSensitive) const;
    bool matchesRegex(const QString& actual, const QRegularExpression& regex) const;
    bool matchesNumericRange(const QVariant& actual, const QVariant& min, const QVariant& max) const;
    bool matchesGreaterThan(const QVariant& actual, const QVariant& threshold) const;
    bool matchesLessThan(const QVariant& actual, const QVariant& threshold) const;
    bool matchesList(const QVariantList& actual, const QVariant& expected) const;

    // Property access helpers
    QVariant getPropertyValue(class Item* item, const QString& propertyName) const;
    bool hasProperty(class Item* item, const QString& propertyName) const;
    QVariant getItemFlag(class Item* item, const QString& flagName) const;
    QVariant getItemAttribute(class Item* item, const QString& attributeName) const;

    // Type and category helpers
    bool isItemOfType(class Item* item, const QString& typeName) const;
    bool isItemInCategory(class Item* item, const QString& categoryName) const;
    QStringList getItemTypeHierarchy(class Item* item) const;

    // Validation helpers
    bool isNumericProperty(const QString& propertyName) const;
    bool isStringProperty(const QString& propertyName) const;
    bool isBooleanProperty(const QString& propertyName) const;
    bool isListProperty(const QString& propertyName) const;

    // Conversion helpers
    QVariant convertToPropertyType(const QString& propertyName, const QVariant& value) const;
    QString normalizeStringValue(const QString& value, bool caseSensitive) const;

private:
    class ItemManager* itemManager_;
    
    // Cached property information
    mutable QStringList availableProperties_;
    mutable QStringList availableFlags_;
    mutable QStringList availableAttributes_;
    mutable QMap<QString, QVariant::Type> propertyTypes_;
    mutable bool propertiesCached_;
    
    void cachePropertyInformation() const;
    void clearPropertyCache();
};

/**
 * @brief Property Match Result for detailed matching information
 */
struct PropertyMatchResult {
    bool matched = false;
    QString propertyName;
    QVariant actualValue;
    QVariant expectedValue;
    PropertyMatchCriteria::MatchType matchType;
    QString description;
    
    PropertyMatchResult() = default;
    PropertyMatchResult(bool matched, const QString& propertyName, 
                       const QVariant& actualValue, const QVariant& expectedValue,
                       PropertyMatchCriteria::MatchType matchType);
    
    QString getDetailedDescription() const;
};

/**
 * @brief Property Matcher Factory for creating common matchers
 */
class PropertyMatcherFactory
{
public:
    // Common property criteria
    static PropertyMatchCriteria createMoveableCriteria(bool moveable = true);
    static PropertyMatchCriteria createBlockingCriteria(bool blocking = true);
    static PropertyMatchCriteria createStackableCriteria(bool stackable = true);
    static PropertyMatchCriteria createPickupableCriteria(bool pickupable = true);
    static PropertyMatchCriteria createReadableCriteria(bool readable = true);
    static PropertyMatchCriteria createWriteableCriteria(bool writeable = true);
    static PropertyMatchCriteria createRotateableCriteria(bool rotateable = true);
    static PropertyMatchCriteria createHangableCriteria(bool hangable = true);
    static PropertyMatchCriteria createVerticalCriteria(bool vertical = true);
    static PropertyMatchCriteria createHorizontalCriteria(bool horizontal = true);
    
    // Type-based criteria
    static PropertyMatchCriteria createContainerCriteria();
    static PropertyMatchCriteria createDoorCriteria();
    static PropertyMatchCriteria createGroundCriteria();
    static PropertyMatchCriteria createWallCriteria();
    static PropertyMatchCriteria createDecorationCriteria();
    
    // Attribute-based criteria
    static PropertyMatchCriteria createActionIdCriteria(quint32 actionId);
    static PropertyMatchCriteria createUniqueIdCriteria(quint32 uniqueId);
    static PropertyMatchCriteria createTextCriteria(const QString& text, bool exactMatch = false);
    static PropertyMatchCriteria createCountCriteria(int minCount, int maxCount = -1);
    
    // Complex criteria combinations
    static ComplexPropertyCriteria createFurnitureCriteria();
    static ComplexPropertyCriteria createInteractableCriteria();
    static ComplexPropertyCriteria createStorageCriteria();
    static ComplexPropertyCriteria createNavigationCriteria();

private:
    PropertyMatcherFactory() = default; // Static factory class
};

/**
 * @brief Property Match Statistics for analysis
 */
struct PropertyMatchStatistics {
    int totalItemsChecked = 0;
    int itemsMatched = 0;
    int propertiesChecked = 0;
    int propertiesMatched = 0;
    QMap<QString, int> propertyMatchCounts;
    QMap<QString, int> propertyMissCounts;
    qint64 matchingTime = 0;
    
    PropertyMatchStatistics() = default;
    
    double getMatchPercentage() const;
    QString getSummary() const;
    QVariantMap toVariantMap() const;
};

#endif // ITEMPROPERTYMATCHER_H
