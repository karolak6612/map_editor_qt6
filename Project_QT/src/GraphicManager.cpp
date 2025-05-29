#include "GraphicManager.h"
#include "Sprite.h"
#include "GameSprite.h"
#include "EditorSprite.h"
#include <QFile>
#include <QBuffer>
#include <QDebug> // For warnings/errors
#include <QStringList> // For QStringList

// Constants for editor sprite IDs (example, actual values might differ)
// These would correspond to enums like EDITOR_SPRITE_SELECTION_MARKER from wxwidgets
enum EditorSpriteId {
    EDITOR_SPRITE_SELECTION_MARKER = -1,
    EDITOR_SPRITE_BRUSH_CD_1x1 = -2,
    // ... other editor sprite IDs
};


GraphicManager::GraphicManager() {
    // Initialize client version with some default if necessary
    // m_client_version = ...;
}

GraphicManager::~GraphicManager() {
    clear();
}

void GraphicManager::clear() {
    m_sprite_cache.clear();
    m_item_sprite_cache.clear();
    m_creature_sprite_cache.clear();
    // m_editor_sprite_cache.clear(); // If separate cache for editor sprites

    m_item_count = 0;
    m_creature_count = 0;
    // Reset other relevant members
}

void GraphicManager::cleanSoftwareSprites() {
    // In Qt, QImage uses implicit sharing and QSharedPointer handles memory.
    // This function might be less relevant or could be used to clear caches
    // more aggressively if memory profiling shows issues.
    // For now, it can be a no-op or call clear().
    qDebug() << "GraphicManager::cleanSoftwareSprites() called - currently a no-op or consider cache clearing.";
}

QSharedPointer<Sprite> GraphicManager::getSprite(int id) {
    if (m_sprite_cache.contains(id)) {
        return m_sprite_cache.value(id);
    }
    // Potentially load on demand or return null if not found
    return QSharedPointer<Sprite>(); 
}

QSharedPointer<GameSprite> GraphicManager::getGameSprite(int id) {
    // Game sprites might be for items or creatures, original wx had separate counts
    // This is a simplified lookup, assumes ID is unique across items/creatures or
    // that the calling context knows which type of ID it is.
    // A more robust system might use prefixes or separate maps.
    if (id >= 0 && id < m_item_count) { // Assuming item IDs are 0 to m_item_count-1
         if (m_item_sprite_cache.contains(id)) {
            return m_item_sprite_cache.value(id);
        }
    } else { // Assuming creature IDs follow item IDs
        int creature_id_offset = id - m_item_count; // Adjust ID if creatures are in a separate range
         if (m_creature_sprite_cache.contains(creature_id_offset)) {
            return m_creature_sprite_cache.value(creature_id_offset);
        }
    }
    // TODO: Load on demand if not found, based on 'id' mapping to file/resource
    return QSharedPointer<GameSprite>();
}

QSharedPointer<EditorSprite> GraphicManager::getEditorSprite(int editor_sprite_id) {
    // Editor sprites are often identified by negative numbers or specific enums
    if (m_sprite_cache.contains(editor_sprite_id)) {
        // Attempt to cast Sprite to EditorSprite. This is safe if only EditorSprites are stored with these IDs.
        QSharedPointer<Sprite> sprite = m_sprite_cache.value(editor_sprite_id);
        QSharedPointer<EditorSprite> editorSprite = qSharedPointerCast<EditorSprite>(sprite);
        if (editorSprite) {
            return editorSprite;
        }
    }
    // TODO: Load on demand if not found
    return QSharedPointer<EditorSprite>();
}


bool GraphicManager::loadEditorSprites() {
    // This function would load predefined editor sprites (icons, markers, etc.)
    // In wxwidgets, these were often from XPM files or embedded PNG data.
    // Example:
    // QMap<EditorSpriteSize, QString> paths_selection_marker;
    // paths_selection_marker.insert(EDITOR_SPRITE_SIZE_16x16, ":/icons/selection_marker_16.png");
    // paths_selection_marker.insert(EDITOR_SPRITE_SIZE_32x32, ":/icons/selection_marker_32.png");
    // QSharedPointer<EditorSprite> sel_marker = QSharedPointer<EditorSprite>(new EditorSprite(paths_selection_marker));
    // m_sprite_cache.insert(EDITOR_SPRITE_SELECTION_MARKER, sel_marker);

    // For each editor sprite:
    // 1. Determine its ID (e.g., EDITOR_SPRITE_SELECTION_MARKER)
    // 2. Find its image data (e.g., from Qt resource file :/)
    // 3. Create an EditorSprite object
    // 4. Store it in m_sprite_cache or a dedicated editor sprite cache

    qDebug() << "GraphicManager::loadEditorSprites() - Placeholder. Implement loading from resources.";
    // Example of loading a single editor sprite (assuming it's a simple image, not multiple sizes for now)
    // QSharedPointer<EditorSprite> eraser_sprite = QSharedPointer<EditorSprite>(new EditorSprite(":/icons/eraser.png"));
    // if (!eraser_sprite->getImage().isNull()) {
    //    m_sprite_cache.insert(SOME_ERASER_ID_ENUM, eraser_sprite);
    // } else {
    //    qWarning() << "Failed to load eraser.png for editor sprite.";
    //    return false;
    // }
    return true; // Placeholder
}

bool GraphicManager::loadSpriteAssets(const QString& dataPath, QString& error, QStringList& warnings) {
    // This is a high-level function that would orchestrate the loading of all game assets.
    // 1. Call loadOTFI (or its equivalent for finding .dat and .spr paths)
    // 2. Call loadSpriteMetadata (to parse .dat file)
    // 3. Call loadSpriteData (to parse .spr file and link image data to GameSprites)

    // For now, this is a stub.
    // The original wxGraphicManager had complex logic for parsing these custom binary formats.
    // That logic needs to be ported carefully.

    // Example steps:
    // if (!loadOTFI(dataPath + "/some_config_or_dir", error, warnings)) return false;
    // if (!loadSpriteMetadata(m_metadata_file_path, error, warnings)) return false;
    // if (!loadSpriteData(m_sprites_file_path, error, warnings)) return false;
    
    qDebug() << "GraphicManager::loadSpriteAssets() - Placeholder for complex asset loading pipeline.";
    error = "Asset loading not yet implemented.";
    return false; // Placeholder
}

QImage GraphicManager::loadImageFromData(const QByteArray& data, const char* format) {
    QImage image;
    if (image.loadFromData(data, format)) {
        return image;
    }
    qWarning() << "GraphicManager::loadImageFromData - Failed to load image from data. Format:" << (format ? format : "autodetect");
    return QImage(); // Return a null image on failure
}

bool GraphicManager::loadGameSpriteData(QSharedPointer<GameSprite> gameSprite, const QByteArray& sprite_data) {
    if (!gameSprite) {
        qWarning() << "GraphicManager::loadGameSpriteData - Null GameSprite provided.";
        return false;
    }

    // This function's role depends on how sprite data is stored and structured.
    // If sprite_data is a spritesheet:
    //   - gameSprite->loadFromSpriteSheet(QImage::fromData(sprite_data), ...);
    // If sprite_data points to individual files (less likely for one QByteArray):
    //   - This would need a different structure, perhaps a list of file paths.
    
    // The original wxGraphicManager::loadSpriteData and GameSprite::NormalImage::getRGBData
    // handled reading from a .spr file which contained many small, potentially compressed, sprite pieces.
    // The 'sprite_id' from the .dat file would map to an offset in the .spr file.
    // That data was then decompressed and turned into pixel data for wxImage.

    // For a Qt port, this means:
    // 1. Reading the correct chunk from the .spr file (based on sprite_id).
    // 2. Decompressing it (if the format uses RLE or similar, as hinted by transparent/colored runs).
    // 3. Constructing a QImage from this raw pixel data.
    // 4. This QImage then becomes one of the 'sprite_parts' in the GameSprite.

    // This is a highly simplified placeholder. The actual implementation will be complex.
    QImage whole_sheet;
    if (whole_sheet.loadFromData(sprite_data)) {
        // Assuming gameSprite has its dimensions (part_width, part_height, sheet_width, sheet_height) set
        // from metadata (.dat file) previously.
        // return gameSprite->loadFromSpriteSheet(whole_sheet, 
        //                                       gameSprite->width * SPRITE_PART_WIDTH, // sheet total width
        //                                       gameSprite->height * SPRITE_PART_HEIGHT, // sheet total height
        //                                       SPRITE_PART_WIDTH, 
        //                                       SPRITE_PART_HEIGHT);
        qDebug() << "GraphicManager::loadGameSpriteData() - Placeholder. Needs spritesheet definition.";
        // For now, just set the whole data as the base image for the GameSprite.
        if(gameSprite) { // Check if gameSprite is not null
             gameSprite->setImage(whole_sheet);
        }
        return true;
    }
    
    qWarning() << "GraphicManager::loadGameSpriteData - Failed to load image from provided data.";
    return false;
}


// QString GraphicManager::getMetadataFileName() const { return m_metadata_file_path; }
// QString GraphicManager::getSpritesFileName() const { return m_sprites_file_path; }

void GraphicManager::setClientVersion(const ClientVersion& version) {
    m_client_version = version;
}

const ClientVersion* GraphicManager::getClientVersion() const {
    return &m_client_version;
}
