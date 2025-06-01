#ifndef CLIPBOARDDATA_H
#define CLIPBOARDDATA_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QByteArray> // For serialize/deserialize
#include <QSet>       // For populateFromSelection argument

#include "Map.h" // For MapPos (assuming MapPos is defined here and small enough)

// Forward declarations for classes from other modules
class Item;
class Creature;
class Spawn;
class Tile;
class QJsonObject; // For JSON helper function signatures

// --- Data Structures for Clipboard Content ---

struct ClipboardItemData {
    int id = 0;
    int countOrSubType = 0; // Or other key attribute like actionID
    QVariantMap properties;   // For other serializable item properties (e.g., text, charges)
};

struct ClipboardCreatureData {
    QString name;
    MapPos offset; // Relative offset if creature is not exactly on tile origin
    QVariantMap properties; // For other serializable creature properties
};

struct ClipboardSpawnData {
    int radius = 0;
    int interval = 0;
    int maxCreatures = 0;
    QStringList creatureNames;
    QVariantMap properties; // For other serializable spawn properties
};

struct ClipboardTileData {
    MapPos relativePosition; // Relative to the top-left-most tile of the selection
    ClipboardItemData ground;
    QList<ClipboardItemData> items;
    ClipboardCreatureData creature;
    ClipboardSpawnData spawn;
    quint32 tileFlags = 0; // Or specific bool flags like isPz, isNoPvp etc.

    bool hasGround = false;
    bool hasCreature = false;
    bool hasSpawn = false;
};

// --- Main ClipboardData Class ---

class ClipboardData {
public:
    ClipboardData();
    ~ClipboardData();

    // Populates this ClipboardData from a selection.
    void populateFromSelection(const QSet<MapPos>& selection, const Map& map);

    // Serializes the clipboard content to a JSON byte array.
    QByteArray serializeToJson() const;
    // Deserializes from JSON byte array to populate this ClipboardData object. Returns true on success.
    bool deserializeFromJson(const QByteArray& jsonData);

    // Getters
    int getSelectionWidth() const { return selectionWidth_; }
    int getSelectionHeight() const { return selectionHeight_; }
    int getSelectionDepth() const { return selectionDepth_; } // If handling multiple Z levels
    const QList<ClipboardTileData>& getTilesData() const { return copiedTiles_; }
    bool isEmpty() const { return copiedTiles_.isEmpty(); }
    bool isCutOperation() const { return isCutOperation_; }

    // Enhanced operations
    void populateFromSelectionCopy(const QSet<MapPos>& selection, const Map& map);
    void populateFromSelectionCut(const QSet<MapPos>& selection, const Map& map);

    // System clipboard integration
    void copyToSystemClipboard() const;
    bool pasteFromSystemClipboard();

    // Utility methods
    void clear();
    ClipboardData* deepCopy() const;
    int getTileCount() const;
    int getItemCount() const;

private:
    QList<ClipboardTileData> copiedTiles_;
    int selectionWidth_ = 0;
    int selectionHeight_ = 0;
    int selectionDepth_ = 0; // For Z-axis extent if selection can span multiple floors
    bool isCutOperation_ = false; // Track whether this was a cut or copy operation

    // Helper method to convert Project_QT Tile to ClipboardTileData
    ClipboardTileData tileToClipboardTileData(const Tile* tile, const MapPos& selectionOrigin);

    // Helper methods for JSON serialization/deserialization of nested structs
    QJsonObject itemDataToJsonObject(const ClipboardItemData& itemData) const;
    ClipboardItemData jsonObjectToItemData(const QJsonObject& itemJson);

    QJsonObject creatureDataToJsonObject(const ClipboardCreatureData& creatureData) const;
    ClipboardCreatureData jsonObjectToCreatureData(const QJsonObject& creatureJson);

    QJsonObject spawnDataToJsonObject(const ClipboardSpawnData& spawnData) const;
    ClipboardSpawnData jsonObjectToSpawnData(const QJsonObject& spawnJson);
    
    QJsonObject tileDataToJsonObject(const ClipboardTileData& tileData) const;
    ClipboardTileData jsonObjectToTileData(const QJsonObject& tileJson);
};

#endif // CLIPBOARDDATA_H
