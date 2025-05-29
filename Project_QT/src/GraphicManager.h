#ifndef GRAPHICMANAGER_H
#define GRAPHICMANAGER_H

#include <QImage>
#include <QString>
#include <QMap>
#include <QVector>
#include <QSharedPointer> // For managing sprites
#include <QStringList> // For QStringList

// Forward declarations
class Sprite;
class GameSprite;
class EditorSprite;
class QFile; // For file operations if needed directly

// If ClientVersion, Outfit, etc., are complex classes, they would need their own headers.
// For now, assume basic or forward-declared.
struct ClientVersion { // Placeholder
    // Define members based on wxwidgets/client_version.h if complex logic is needed
    // For GraphicManager, it might influence how .dat/.spr files are parsed
    QString versionString;
    // enum DatFormat { DAT_FORMAT_UNKNOWN, DAT_FORMAT_74, ... };
    // DatFormat getDatFormatForSignature(uint32_t signature);
};

struct SpriteLight { // Placeholder
    uint8_t intensity = 0;
    uint8_t color = 0; // Assuming color can be represented by a simple type for now
};


class GraphicManager {
public:
    GraphicManager();
    ~GraphicManager();

    void clear(); // Clears loaded graphics
    void cleanSoftwareSprites(); // May not be directly applicable if QImage handles its own memory well

    QSharedPointer<Sprite> getSprite(int id); // Generic sprite getter
    virtual QSharedPointer<GameSprite> getGameSprite(int id); // Made virtual for Mocking
    QSharedPointer<EditorSprite> getEditorSprite(int editor_sprite_id); // For editor-specific UI icons

    // Corresponds to GraphicManager::loadEditorSprites
    bool loadEditorSprites(); 

    // Corresponds to GraphicManager::loadOTFI, loadSpriteMetadata, loadSpriteData
    // These will be complex and involve parsing custom binary formats.
    // Stubs for now.
    bool loadSpriteAssets(const QString& dataPath, QString& error, QStringList& warnings);
    // bool loadOTFI(const QString& directoryPath, QString& error, QStringList& warnings);
    // bool loadSpriteMetadata(const QString& datPath, QString& error, QStringList& warnings);
    // bool loadSpriteData(const QString& sprPath, QString& error, QStringList& warnings);

    // Helper for loading an image from raw data (akin to _wxGetBitmapFromMemory)
    QImage loadImageFromData(const QByteArray& data, const char* format = "PNG");

    // Manages sprite sheets or individual sprite files
    // This is a high-level function that might call internal helpers
    // to populate GameSprite objects with their QImage parts.
    bool loadGameSpriteData(QSharedPointer<GameSprite> gameSprite, const QByteArray& sprite_sheet_data /* or paths */);


    // Getters for paths (if still relevant)
    // QString getMetadataFileName() const;
    // QString getSpritesFileName() const;

    // Client version handling
    void setClientVersion(const ClientVersion& version);
    const ClientVersion* getClientVersion() const;


private:
    // Caches for different types of sprites
    // Using QSharedPointer for automatic memory management of sprites
    QMap<int, QSharedPointer<Sprite>> m_sprite_cache; // Generic sprites, could include editor sprites
    QMap<int, QSharedPointer<GameSprite>> m_item_sprite_cache;
    QMap<int, QSharedPointer<GameSprite>> m_creature_sprite_cache;
    // QMap<int, QSharedPointer<EditorSprite>> m_editor_sprite_cache; // Or merge with m_sprite_cache

    // Store sprite metadata if not directly in GameSprite objects
    // (e.g. raw data pointers if not immediately converted to QImages)

    ClientVersion m_client_version; // Current client version for asset loading logic

    // Paths for sprite data - might be set during loadSpriteAssets
    QString m_metadata_file_path;
    QString m_sprites_file_path;

    // Internal counters and flags from wxwidgets version
    uint16_t m_item_count = 0;
    uint16_t m_creature_count = 0;
    // bool m_otfi_found = false;
    // bool m_is_extended_format = false; // e.g. for .spr files
    // bool m_has_transparency_in_spr = false; // for .spr files

    // Helper methods for parsing .dat and .spr files would go here
    // bool parseDatEntry(...);
    // bool parseSprEntry(...);
};

#endif // GRAPHICMANAGER_H
