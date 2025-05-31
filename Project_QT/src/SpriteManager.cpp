#include "SpriteManager.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

// Constructor
SpriteManager::SpriteManager(QObject* parent)
    : QObject(parent),
      assetsLoaded_(false),
      sprSignature_(0),
      sprSpriteCount_(0),
      datSignature_(0),
      datItemCount_(0),
      datOutfitCount_(0),
      datEffectCount_(0),
      datMissileCount_(0) {
}

// Destructor
SpriteManager::~SpriteManager() {
    unloadAssets();
}

// Unload Assets
void SpriteManager::unloadAssets() {
    gameSpriteMetadataCache_.clear();
    sprSheetRleDataCache_.clear();
    sprSheetAddresses_.clear();
    assetsLoaded_ = false;

    sprSignature_ = 0;
    sprSpriteCount_ = 0;
    datSignature_ = 0;
    datItemCount_ = 0;
    datOutfitCount_ = 0;
    datEffectCount_ = 0;
    datMissileCount_ = 0;

    qDebug() << "SpriteManager: Assets unloaded.";
}

// Load Assets Shell
bool SpriteManager::loadAssets(const ClientVersionData& clientVersion, QString& error, QStringList& warnings) {
    if (assetsLoaded_) {
        qDebug() << "SpriteManager: Assets already loaded. Unloading first.";
        unloadAssets();
    }

    versionData_ = clientVersion;
    error.clear();
    warnings.clear();

    qDebug() << "SpriteManager: Loading assets for client version" << versionData_.clientVersionNumber;
    qDebug() << "DAT path:" << versionData_.datPath;
    qDebug() << "SPR path:" << versionData_.sprPath;

    // Open and parse DAT file
    QFile datFile(versionData_.datPath);
    if (!datFile.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open DAT file: %1").arg(versionData_.datPath);
        qCritical() << "SpriteManager:" << error;
        return false;
    }
    qDebug() << "SpriteManager: DAT file opened successfully.";
    if (!parseDatFile(datFile, error, warnings)) { // Pass warnings by reference
        datFile.close();
        // error string is set by parseDatFile or parseDatHeader
        qCritical() << "SpriteManager: Failed to parse DAT file:" << error;
        return false;
    }
    datFile.close();
    qDebug() << "SpriteManager: DAT file parsed successfully.";

    // Open and parse SPR file
    QFile sprFile(versionData_.sprPath);
    if (!sprFile.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open SPR file: %1").arg(versionData_.sprPath);
        qCritical() << "SpriteManager:" << error;
        return false;
    }
    qDebug() << "SpriteManager: SPR file opened successfully.";
    if (!parseSprFile(sprFile, error)) {
        sprFile.close();
        // error string is set by parseSprFile or parseSprHeader
        qCritical() << "SpriteManager: Failed to parse SPR file:" << error;
        return false;
    }
    sprFile.close();
    qDebug() << "SpriteManager: SPR file parsed successfully.";

    assetsLoaded_ = true;
    qInfo() << "SpriteManager: Assets loaded successfully for client version" << versionData_.clientVersionNumber;
    return true;
}

// Parse DAT File (Header and Contents Shell)
bool SpriteManager::parseDatFile(QFile& file, QString& error, QStringList& warnings) {
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    if (!parseDatHeader(stream, error)) {
        return false;
    }

    // Stub for loadDatContents for Part 1
    // qDebug() << "SpriteManager::loadDatContents called, actual content parsing deferred to Part 2.";
    // In a full implementation, this would loop from 1 to (itemCount + outfitCount + effectCount + missileCount)
    // and call readDatEntry for each.
    if (!loadDatContents(stream, error, warnings)) {
        qCritical() << "SpriteManager: Failed during loadDatContents:" << error;
        return false;
    }

    return true;
}

bool SpriteManager::loadDatContents(QDataStream& stream, QString& error, QStringList& warnings) {
    qDebug() << "SpriteManager: Starting to load DAT contents...";
    quint32 datEntriesRead = 0;

    // Items (Client IDs typically start from 100)
    // The DAT file lists items sequentially starting from the first item (often ID 100).
    for (quint16 i = 0; i < datItemCount_; ++i) {
        quint32 clientItemId = 100 + i;
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading item %1 (Client ID: %2). Expected %3 items.")
                        .arg(i).arg(clientItemId).arg(datItemCount_);
            return false;
        }
        if (!readDatEntry(stream, clientItemId, error, warnings)) {
            // readDatEntry sets the error, but we add context
            warnings.append(QString("Error reading DAT entry for item (sequential index %1, clientID %2): %3")
                                .arg(datEntriesRead).arg(clientItemId).arg(error));
            // Depending on severity, might choose to continue or stop.
            // For now, let's try to continue to gather more potential errors.
        }
        datEntriesRead++;
    }
    qDebug() << "SpriteManager: Finished reading" << datItemCount_ << "item entries.";

    // Outfits (Creatures)
    // These follow items in the .dat file. Their client-side identification might be more complex
    // (e.g., not simple sequential IDs, or might overlap with item ID space if not careful).
    // For RME internal representation, a distinct ID range is often used.
    // The prompt suggested a placeholder scheme: datItemCount_ + 100 + i
    // Let's assume a base ID for outfits for now, e.g., 20000, or just use a sequential internal one.
    // For simplicity in this step, let's use a gameSpriteId that is just a continuation,
    // but acknowledge this needs proper mapping for a real editor.
    // Let's use a distinct range for cache key, e.g. outfits start after items using a large offset.
    // For now, using the sequential approach and the cache key will be this sequential id.
    // This means getGameSpriteData will need to know how to map game types to these ranges.
    // This mapping is complex and outside this subtask's scope.
    // For now, just read sequentially. The gameSpriteId passed to readDatEntry will be this sequential one.

    quint32 outfitStartId = 1; // This is an internal sequential ID for outfits, not client ID
    for (quint16 i = 0; i < datOutfitCount_; ++i) {
        // The key for gameSpriteMetadataCache_ needs to be unique across types.
        // A common way is to use a type prefix or large offset.
        // E.g. items 100-19999, outfits 20000-29999 etc.
        // For this generic loader, let's pass the sequential index + type offset as gameSpriteId.
        // This ID is for caching, specific game logic will map game IDs to these.
        quint32 cacheKeyForOutfit = 20000 + outfitStartId + i; // Example offset
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading outfit %1. Expected %2 outfits.")
                        .arg(i).arg(datOutfitCount_);
            return false;
        }
        if (!readDatEntry(stream, cacheKeyForOutfit, error, warnings)) {
            warnings.append(QString("Error reading DAT entry for outfit (sequential index %1, cache key %2): %3")
                                .arg(datEntriesRead).arg(cacheKeyForOutfit).arg(error));
        }
        datEntriesRead++;
    }
    qDebug() << "SpriteManager: Finished reading" << datOutfitCount_ << "outfit entries.";

    // Effects
    quint32 effectStartId = 1;
    for (quint16 i = 0; i < datEffectCount_; ++i) {
        quint32 cacheKeyForEffect = 30000 + effectStartId + i; // Example offset
         if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading effect %1. Expected %2 effects.")
                        .arg(i).arg(datEffectCount_);
            return false;
        }
        if (!readDatEntry(stream, cacheKeyForEffect, error, warnings)) {
             warnings.append(QString("Error reading DAT entry for effect (sequential index %1, cache key %2): %3")
                                .arg(datEntriesRead).arg(cacheKeyForEffect).arg(error));
        }
        datEntriesRead++;
    }
    qDebug() << "SpriteManager: Finished reading" << datEffectCount_ << "effect entries.";

    // Missiles (Distances)
    quint32 missileStartId = 1;
    for (quint16 i = 0; i < datMissileCount_; ++i) {
        quint32 cacheKeyForMissile = 40000 + missileStartId + i; // Example offset
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading missile %1. Expected %2 missiles.")
                        .arg(i).arg(datMissileCount_);
            return false;
        }
        if (!readDatEntry(stream, cacheKeyForMissile, error, warnings)) {
            warnings.append(QString("Error reading DAT entry for missile (sequential index %1, cache key %2): %3")
                                .arg(datEntriesRead).arg(cacheKeyForMissile).arg(error));
        }
        datEntriesRead++;
    }
    qDebug() << "SpriteManager: Finished reading" << datMissileCount_ << "missile entries.";
    qDebug() << "SpriteManager: Total DAT entries processed:" << datEntriesRead;

    return true;
}

// Parse DAT Header
bool SpriteManager::parseDatHeader(QDataStream& stream, QString& error) {
    stream >> datSignature_;
    qDebug() << "SpriteManager: DAT Signature read:" << Qt::hex << datSignature_;

    if (versionData_.expectedDatSignature != 0 && datSignature_ != versionData_.expectedDatSignature) {
        error = QString("DAT file signature mismatch. Expected %1, got %2.")
                    .arg(versionData_.expectedDatSignature, 0, 16)
                    .arg(datSignature_, 0, 16);
        return false;
    }

    stream >> datItemCount_ >> datOutfitCount_ >> datEffectCount_ >> datMissileCount_;
    qDebug() << "SpriteManager: DAT Counts - Items:" << datItemCount_
             << "Outfits:" << datOutfitCount_
             << "Effects:" << datEffectCount_
             << "Missiles:" << datMissileCount_;

    // Basic validation
    if (datItemCount_ == 0 || datItemCount_ > 50000) { // Arbitrary upper limit
        error = QString("Invalid item count in DAT file: %1").arg(datItemCount_);
        return false;
    }

    return true;
}

// Parse SPR File (Header)
bool SpriteManager::parseSprFile(QFile& file, QString& error) {
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    if (!parseSprHeader(stream, error)) {
        return false;
    }
    return true;
}

// Parse SPR Header
bool SpriteManager::parseSprHeader(QDataStream& stream, QString& error) {
    stream >> sprSignature_;
    qDebug() << "SpriteManager: SPR Signature read:" << Qt::hex << sprSignature_;

    if (versionData_.expectedSprSignature != 0 && sprSignature_ != versionData_.expectedSprSignature) {
        error = QString("SPR file signature mismatch. Expected %1, got %2.")
                    .arg(versionData_.expectedSprSignature, 0, 16)
                    .arg(sprSignature_, 0, 16);
        return false;
    }

    if (versionData_.isExtendedSpr) {
        stream >> sprSpriteCount_;
    } else {
        quint16 totalSprites_u16;
        stream >> totalSprites_u16;
        sprSpriteCount_ = totalSprites_u16;
    }
    qDebug() << "SpriteManager: SPR Sprite Count:" << sprSpriteCount_;

    if (sprSpriteCount_ == 0 || sprSpriteCount_ > 150000) { // Arbitrary upper limit, Tibia 12.x has ~90k
         error = QString("Invalid sprite count in SPR file: %1").arg(sprSpriteCount_);
        return false;
    }

    sprSheetAddresses_.reserve(sprSpriteCount_); // Pre-allocate for efficiency
    // Sprite IDs are 1-based in typical usage, but addresses are stored per sprite.
    // The map can use 0 to N-1 or 1 to N. Let's use 1 to N for consistency with game IDs.
    for (quint32 i = 1; i <= sprSpriteCount_; ++i) {
        quint32 spriteAddress;
        stream >> spriteAddress;
        if (spriteAddress == 0) { // Address 0 indicates an empty sprite (transparent)
            // Still store it, as it's a valid "empty" sprite reference.
            // Or, could skip and handle lookups for missing IDs as transparent.
            // For now, store as is.
        }
        sprSheetAddresses_.insert(i, spriteAddress);
        if (stream.atEnd() && i < sprSpriteCount_) {
             error = QString("SPR file ended prematurely while reading sprite addresses. Read %1 of %2.").arg(i).arg(sprSpriteCount_);
             return false;
        }
    }

    qDebug() << "SpriteManager: Read" << sprSheetAddresses_.size() << "sprite addresses from SPR header.";
    if (sprSheetAddresses_.size() != sprSpriteCount_) {
        // This case should be caught by stream.atEnd() above, but as a safeguard:
        error = QString("Mismatch between sprite count and addresses read. Count: %1, Addresses: %2")
                    .arg(sprSpriteCount_).arg(sprSheetAddresses_.size());
        return false;
    }

    return true;
}

// --- Basic Accessors ---
quint32 SpriteManager::getSpriteCount() const { return sprSpriteCount_; }
quint16 SpriteManager::getItemTypeCount() const { return datItemCount_; }
quint16 SpriteManager::getOutfitCount() const { return datOutfitCount_; }
quint16 SpriteManager::getEffectCount() const { return datEffectCount_; }
quint16 SpriteManager::getMissileCount() const { return datMissileCount_; }
const ClientVersionData* SpriteManager::getCurrentVersionData() const { return &versionData_; }

// Stubs for methods to be implemented in later parts
QSharedPointer<const GameSpriteData> SpriteManager::getGameSpriteData(quint32 gameSpriteId) const {
    Q_UNUSED(gameSpriteId);
    // To be implemented in Part 2 (DAT content parsing)
    // return gameSpriteMetadataCache_.value(gameSpriteId, nullptr);
    return nullptr;
}

QImage SpriteManager::getSpriteImage(quint32 actualSprId) {
    Q_UNUSED(actualSprId);
    // To be implemented in Part 3 (SPR reading and RLE decoding)
    return QImage();
}

QImage SpriteManager::getFrameImage(quint32 gameSpriteId, int frame, int patternX, int patternY, int patternZ, int layer) {
    Q_UNUSED(gameSpriteId);
    Q_UNUSED(frame);
    Q_UNUSED(patternX);
    Q_UNUSED(patternY);
    Q_UNUSED(patternZ);
    Q_UNUSED(layer);
    // To be implemented in Part 3 or 4 (Frame composition)
    return QImage();
}

// Placeholder for readDatEntry, will be implemented in Part 2 (actually, implementing now)
bool SpriteManager::readDatEntry(QDataStream& stream, quint32 gameSpriteId, QString& error, QStringList& warnings) {
    QSharedPointer<GameSpriteData> spriteData = QSharedPointer<GameSpriteData>(new GameSpriteData());
    spriteData->id = gameSpriteId;

    quint8 flag_value;
    forever {
        if (stream.atEnd()) { error = "Unexpected end of stream while reading flags for DAT entry " + QString::number(gameSpriteId); return false; }
        stream >> flag_value;
        if (flag_value == 0xFF) break; // End of flags marker

        // Mapping from raw flag byte to SpriteDatFlags enum member
        // This switch is based on common DAT flags. Specific client versions might vary.
        // The values (cases) are from common Tibia .dat specifications.
        switch (flag_value) {
            case 0: spriteData->flags |= SpriteDatFlags::Ground;
                    if(versionData_.datFormat >= DatFormat::Format_755) { // Ground speed is part of ground flag in 7.55+
                         quint16 groundSpeed; stream >> groundSpeed; /* store if needed, e.g. spriteData->groundSpeed = groundSpeed; */
                         if (stream.atEnd()) { error = "Unexpected end of stream after ground speed flag for " + QString::number(gameSpriteId); return false; }
                    }
                    break;
            case 1: spriteData->flags |= SpriteDatFlags::GroundBorder; break; // Clip / Ground Border
            case 2: spriteData->flags |= SpriteDatFlags::OnBottom; break;     // OnBottom / DrawOnTop (depends on interpretation)
            case 3: spriteData->flags |= SpriteDatFlags::OnTop; break;        // OnTop / DrawOnBottom
            case 4: spriteData->flags |= SpriteDatFlags::Container; break;
            case 5: spriteData->flags |= SpriteDatFlags::Stackable; break;
            case 6: spriteData->flags |= SpriteDatFlags::ForceUse; break; // Corpse / ForceUse
            case 7: spriteData->flags |= SpriteDatFlags::MultiUse; break;
            case 8: // Writable / Usable / Chargeable (context dependent)
                if (versionData_.clientVersionNumber >= 780 && versionData_.clientVersionNumber <= 792) { // Chargeable for 7.8-7.92
                     spriteData->flags |= SpriteDatFlags::Chargeable_780;
                } else { // Default to Writable
                     spriteData->flags |= SpriteDatFlags::Writable;
                }
                break;
            case 9: spriteData->flags |= SpriteDatFlags::WritableOnce; break; // Readable
            case 10: spriteData->flags |= SpriteDatFlags::FluidContainer; break;
            case 11: spriteData->flags |= SpriteDatFlags::Splash; break;
            case 12: spriteData->flags |= SpriteDatFlags::NotWalkable; break;
            case 13: spriteData->flags |= SpriteDatFlags::NotMoveable; break;
            case 14: spriteData->flags |= SpriteDatFlags::BlockProjectile; break;
            case 15: spriteData->flags |= SpriteDatFlags::NotPathable; break; // NoPathArrow
            case 16: // Pickupable or NoMoveAnimation
                if (versionData_.clientVersionNumber >= 1010) {
                    spriteData->flags |= SpriteDatFlags::NoMoveAnimation_1010;
                } else {
                    spriteData->flags |= SpriteDatFlags::Pickupable;
                }
                break;
            case 17: spriteData->flags |= SpriteDatFlags::Hangable; break; // Hang / Horizontal / Vertical
            case 18: spriteData->flags |= SpriteDatFlags::HookSouth; break;
            case 19: spriteData->flags |= SpriteDatFlags::HookEast; break;
            case 20: spriteData->flags |= SpriteDatFlags::Rotateable; break;
            case 21: { // Light info
                spriteData->flags |= SpriteDatFlags::Light;
                quint16 intensity_read, color_read; // As per wxRME, these were quint16 in file for some versions
                stream >> intensity_read >> color_read;
                if (stream.atEnd()) { error = "Unexpected end of stream after light flag for " + QString::number(gameSpriteId); return false; }
                spriteData->light.intensity = static_cast<quint8>(intensity_read); // Assuming data fits quint8
                spriteData->light.color = static_cast<quint8>(color_read);
                spriteData->light.hasLight = true;
                break;
            }
            case 22: spriteData->flags |= SpriteDatFlags::DontHide; break; // FloorChange / DontHide / IgnoreLook
            case 23: spriteData->flags |= SpriteDatFlags::Translucent; break; // Translucent / Offset
            case 24: { // Displacement / Shift
                spriteData->flags |= SpriteDatFlags::Displacement;
                quint16 x_offset, y_offset;
                stream >> x_offset >> y_offset;
                if (stream.atEnd()) { error = "Unexpected end of stream after displacement flag for " + QString::number(gameSpriteId); return false; }
                spriteData->drawOffset = QPoint(static_cast<int>(x_offset), static_cast<int>(y_offset));
                break;
            }
            case 25: { // Elevation / Height
                spriteData->flags |= SpriteDatFlags::Elevation;
                stream >> spriteData->drawHeight;
                if (stream.atEnd()) { error = "Unexpected end of stream after elevation flag for " + QString::number(gameSpriteId); return false; }
                break;
            }
            case 26: spriteData->flags |= SpriteDatFlags::LyingCorpse; break; // Lying Object
            case 27: spriteData->flags |= SpriteDatFlags::AnimateAlways; break;
            case 28: { // Minimap Color
                spriteData->flags |= SpriteDatFlags::MinimapColor;
                stream >> spriteData->minimapColor;
                if (stream.atEnd()) { error = "Unexpected end of stream after minimap color flag for " + QString::number(gameSpriteId); return false; }
                break;
            }
            case 29: { // Lens Help / Action data
                spriteData->flags |= SpriteDatFlags::LensHelp;
                quint16 lensHelpId; stream >> lensHelpId; /* store if needed spriteData->lensHelpId = lensHelpId; */
                if (stream.atEnd()) { error = "Unexpected end of stream after lens help flag for " + QString::number(gameSpriteId); return false; }
                break;
            }
            case 30: spriteData->flags |= SpriteDatFlags::FullGround; break; // Full Ground / IgnoreLook
            case 31: spriteData->flags |= SpriteDatFlags::Look; break; // Indicates "look through" / no default action
            // case 32: // Clothing (newer clients)
            // case 33: // Market (newer clients)
            // etc. Add more flags as needed based on versionData_.clientVersionNumber
            default:
                warnings.append(QString("Unknown DAT flag %1 for gameSpriteId %2. Skipping.").arg(flag_value).arg(gameSpriteId));
                // If a flag has variable length arguments, skipping unknown ones is risky.
                // This basic parser assumes unknown flags have no arguments.
                break;
        }
    }

    // Read Dimensions and Animation Info
    if (stream.atEnd()) { error = "Unexpected end of stream before dimensions for " + QString::number(gameSpriteId); return false; }
    stream >> spriteData->spriteWidth >> spriteData->spriteHeight;
    if (spriteData->spriteWidth == 0) spriteData->spriteWidth = 1; // Avoid division by zero later
    if (spriteData->spriteHeight == 0) spriteData->spriteHeight = 1;

    if (spriteData->spriteWidth > 1 || spriteData->spriteHeight > 1) {
        if (stream.atEnd()) { error = "Unexpected end of stream before exact size for " + QString::number(gameSpriteId); return false; }
        quint8 exactSize; stream >> exactSize; // Read and discard for now, could store if useful
    }

    if (stream.atEnd()) { error = "Unexpected end of stream before layers for " + QString::number(gameSpriteId); return false; }
    stream >> spriteData->layers >> spriteData->patternX >> spriteData->patternY;

    if (versionData_.datFormat < DatFormat::Format_780) {
        spriteData->patternZ = 1; // Default for older versions
    } else {
        if (stream.atEnd()) { error = "Unexpected end of stream before patternZ for " + QString::number(gameSpriteId); return false; }
        stream >> spriteData->patternZ;
    }
    if (stream.atEnd()) { error = "Unexpected end of stream before frames for " + QString::number(gameSpriteId); return false; }
    stream >> spriteData->frames;
    spriteData->isAnimated = spriteData->frames > 1;

    if (spriteData->isAnimated) {
        if (versionData_.hasFrameDurations) { // Typically for 9.60+ or as indicated by client version data
            if (stream.atEnd()) { error = "Unexpected end of stream before animation async byte for " + QString::number(gameSpriteId); return false; }
            quint8 async; stream >> async; // Read 'is_async' byte (used as 'loop_type' in some contexts)

            if (stream.atEnd()) { error = "Unexpected end of stream before animation loop count for " + QString::number(gameSpriteId); return false; }
            stream >> spriteData->animationLoopCount;

            if (stream.atEnd()) { error = "Unexpected end of stream before animation start frame for " + QString::number(gameSpriteId); return false; }
            qint8 startFrame; stream >> startFrame; spriteData->animationStartFrame = startFrame;

            spriteData->frameDurations.reserve(spriteData->frames);
            for (quint8 i = 0; i < spriteData->frames; ++i) {
                if (stream.atEnd()) { error = QString("Unexpected end of stream reading frame duration %1/%2 for ").arg(i+1).arg(spriteData->frames) + QString::number(gameSpriteId); return false; }
                quint32 min, max; stream >> min >> max;
                spriteData->frameDurations.append(qMakePair(min, max));
            }
        } else { // Older clients without explicit frame durations
            spriteData->animationLoopCount = -1; // Default to continuous loop (or often 0 means continuous)
            spriteData->animationStartFrame = 0; // Default start frame
            // Frame durations might be fixed (e.g., 100ms) or handled by an animator class.
            // For now, leave frameDurations vector empty.
        }
    }

    // Read SPR Sheet IDs
    quint32 numSprSheetsToRead = static_cast<quint32>(spriteData->spriteWidth) * spriteData->spriteHeight *
                                spriteData->layers * spriteData->patternX *
                                spriteData->patternZ * spriteData->frames;

    if (numSprSheetsToRead == 0 && (spriteData->spriteWidth > 0 || spriteData->spriteHeight > 0)) { // If dimensions suggest sprites but calculation is 0
        warnings.append(QString("Calculated 0 SPR sheets for gameSpriteId %1 with non-zero dimensions (W:%2 H:%3 L:%4 X:%5 Y:%6 Z:%7 F:%8). Assuming 1 sheet.")
                        .arg(gameSpriteId).arg(spriteData->spriteWidth).arg(spriteData->spriteHeight).arg(spriteData->layers)
                        .arg(spriteData->patternX).arg(spriteData->patternY).arg(spriteData->patternZ).arg(spriteData->frames));
        numSprSheetsToRead = 1; // Fallback for items that are not "null" but have 0 in some pattern/layer fields
    }

    // Sanity check numSprSheetsToRead
    if (numSprSheetsToRead > 4096) { // Arbitrary limit for a single game object's sprites
        error = QString("Excessive number of sprite sheets (%1) calculated for gameSpriteId %2. DAT entry likely corrupt.")
                    .arg(numSprSheetsToRead).arg(gameSpriteId);
        return false;
    }

    if (numSprSheetsToRead > 0) {
        spriteData->sprSheetIDs.reserve(static_cast<int>(numSprSheetsToRead));
        for (quint32 i = 0; i < numSprSheetsToRead; ++i) {
            if (stream.atEnd()) {
                error = QString("Unexpected end of stream reading SPR sheet ID %1/%2 for gameSpriteId %3.")
                            .arg(i+1).arg(numSprSheetsToRead).arg(gameSpriteId);
                return false;
            }
            quint32 sprSheetId;
            if (versionData_.isExtendedSpr) {
                stream >> sprSheetId;
            } else {
                quint16 sprSheetId_u16;
                stream >> sprSheetId_u16;
                sprSheetId = sprSheetId_u16;
            }
            spriteData->sprSheetIDs.append(sprSheetId);
        }
    }

    gameSpriteMetadataCache_.insert(gameSpriteId, spriteData);
    return true;
}

// Placeholder for decodeSpriteRleData, will be implemented in Part 3 (actually, implementing now)
QImage SpriteManager::decodeSpriteRleData(const QByteArray& rleData, bool hasAlpha) const {
    if (rleData.isEmpty()) {
        return QImage(32, 32, QImage::Format_ARGB32_Premultiplied); // Return transparent image for empty RLE
    }

    QImage image(32, 32, QImage::Format_ARGB32_Premultiplied); // Use ARGB32_Premultiplied for better QPainter performance
    image.fill(Qt::transparent); // Initialize with transparency

    uchar* imageData = image.bits();
    const uchar* rleBytes = reinterpret_cast<const uchar*>(rleData.constData());
    int rleSize = rleData.size();

    int rleIdx = 0;
    int currentPixel = 0; // Current pixel index in the 32x32 grid (0 to 1023)
    int bytesPerPixel = hasAlpha ? 4 : 3;

    while (rleIdx < rleSize && currentPixel < 1024) {
        if (rleIdx + 1 >= rleSize) { // Not enough data for count
            qWarning() << "SpriteManager::decodeSpriteRleData: Unexpected end of RLE data (reading count). Offset:" << rleIdx << "Size:" << rleSize;
            break;
        }
        quint16 transparentPixels = static_cast<quint16>(rleBytes[rleIdx]) | (static_cast<quint16>(rleBytes[rleIdx+1]) << 8);
        rleIdx += 2;
        currentPixel += transparentPixels;

        if (currentPixel > 1024 && transparentPixels > 0) { // Check if transparent run overshot
             qWarning() << "SpriteManager::decodeSpriteRleData: Transparent run overshot image buffer. currentPixel:" << currentPixel;
             currentPixel = 1024; // Clamp to end
        }
        if (currentPixel >= 1024 || rleIdx >= rleSize) { // Re-check after transparent run
            break;
        }

        if (rleIdx + 1 >= rleSize) { // Not enough data for count
            qWarning() << "SpriteManager::decodeSpriteRleData: Unexpected end of RLE data (reading colored count). Offset:" << rleIdx << "Size:" << rleSize;
            break;
        }
        quint16 coloredPixels = static_cast<quint16>(rleBytes[rleIdx]) | (static_cast<quint16>(rleBytes[rleIdx+1]) << 8);
        rleIdx += 2;

        for (quint16 i = 0; i < coloredPixels; ++i) {
            if (currentPixel >= 1024) {
                qWarning() << "SpriteManager::decodeSpriteRleData: Colored pixel run trying to write past image buffer. currentPixel:" << currentPixel;
                break;
            }
            if (rleIdx + bytesPerPixel > rleSize) { // Check for enough data for one pixel
                qWarning() << "SpriteManager::decodeSpriteRleData: Unexpected end of RLE data (reading pixel data). Offset:" << rleIdx << "Required:" << bytesPerPixel << "Size:" << rleSize;
                rleIdx = rleSize; // Force break outer loop
                break;
            }

            int imgByteIdx = currentPixel * 4; // Each pixel is 4 bytes in ARGB32_Premultiplied

            // BGRA order in file for RGB or RGBA sprites
            imageData[imgByteIdx + 2] = rleBytes[rleIdx + 0]; // Red
            imageData[imgByteIdx + 1] = rleBytes[rleIdx + 1]; // Green
            imageData[imgByteIdx + 0] = rleBytes[rleIdx + 2]; // Blue
            imageData[imgByteIdx + 3] = hasAlpha ? rleBytes[rleIdx + 3] : 255; // Alpha

            rleIdx += bytesPerPixel;
            currentPixel++;
        }
    }
    if (rleIdx < rleSize && currentPixel < 1024) {
        qWarning() << "SpriteManager::decodeSpriteRleData: RLE data not fully consumed or image not fully populated. RLE Idx:" << rleIdx << "/" << rleSize << "Pixel Idx:" << currentPixel << "/1024";
    }

    return image;
}

QByteArray SpriteManager::readRawSpriteData(quint32 actualSprId, QString& error) {
    error.clear();
    if (!assetsLoaded_) {
        error = "Assets not loaded.";
        return QByteArray();
    }
    if (actualSprId == 0 || actualSprId > sprSpriteCount_) { // Sprite ID 0 is often "empty" but might not have an address.
        // error = QString("Invalid actualSprId: %1. Max is %2.").arg(actualSprId).arg(sprSpriteCount_);
        // For ID 0, or invalid ID, return empty indicating transparency.
        return QByteArray();
    }

    if (!sprSheetAddresses_.contains(actualSprId)) {
        // This can happen if a DAT entry references a SPR ID that was not in the header's address list
        // or if actualSprId is 0 and we chose not to store address 0.
        // error = QString("Sprite address not found for actualSprId: %1").arg(actualSprId);
        return QByteArray(); // Treat as transparent
    }

    quint32 address = sprSheetAddresses_.value(actualSprId);
    if (address == 0) { // Address 0 explicitly means an empty/transparent sprite
        return QByteArray();
    }

    QFile file(versionData_.sprPath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QString("Failed to open SPR file for reading sprite ID %1: %2").arg(actualSprId).arg(versionData_.sprPath);
        return QByteArray();
    }

    if (!file.seek(address)) {
        error = QString("Failed to seek to address %1 for sprite ID %2 in SPR file.").arg(address).arg(actualSprId);
        file.close();
        return QByteArray();
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Skip 3-byte color key (R, G, B) - not used when alpha channel is present
    if (stream.skipRawData(3) != 3) {
        error = QString("Failed to skip color key for sprite ID %1.").arg(actualSprId);
        file.close();
        return QByteArray();
    }

    quint16 rleDataSize;
    stream >> rleDataSize;

    if (stream.status() != QDataStream::Ok) {
        error = QString("Failed to read RLE data size for sprite ID %1.").arg(actualSprId);
        file.close();
        return QByteArray();
    }

    if (rleDataSize == 0) {
        file.close();
        return QByteArray(); // Empty sprite
    }

    // QDataStream::readBytes reads a quint32 size first, then the bytes. We want to read exactly rleDataSize.
    // So, better use file.read directly or stream.readRawData.
    QByteArray rleData;
    rleData.resize(rleDataSize); // Pre-allocate
    int bytesRead = stream.readRawData(rleData.data(), rleDataSize);

    if (bytesRead != rleDataSize) {
        error = QString("Failed to read RLE data for sprite ID %1. Expected %2 bytes, got %3.")
                    .arg(actualSprId).arg(rleDataSize).arg(bytesRead);
        file.close();
        return QByteArray(); // Return empty or partially read data
    }

    file.close();
    return rleData;
}


// --- Public Accessors ---
QSharedPointer<const GameSpriteData> SpriteManager::getGameSpriteData(quint32 gameSpriteId) const {
    if (!assetsLoaded_) {
        qWarning("SpriteManager::getGameSpriteData - Assets not loaded.");
        return nullptr;
    }
    return gameSpriteMetadataCache_.value(gameSpriteId, nullptr);
}

QImage SpriteManager::getSpriteImage(quint32 actualSprId) {
    if (!assetsLoaded_) {
        qWarning("SpriteManager::getSpriteImage - Assets not loaded.");
        return QImage();
    }
    if (actualSprId == 0) return QImage(32,32,QImage::Format_ARGB32_Premultiplied); // common case for empty/transparent

    if (sprSheetRleDataCache_.contains(actualSprId)) {
        const QByteArray& rleData = sprSheetRleDataCache_.value(actualSprId);
        return decodeSpriteRleData(rleData, versionData_.hasAlphaChannel);
    }

    QString error;
    QByteArray rleData = readRawSpriteData(actualSprId, error);
    if (!error.isEmpty()) {
        qWarning() << "SpriteManager::getSpriteImage - Error reading raw sprite data for ID" << actualSprId << ":" << error;
        return QImage(); // Return empty or placeholder image
    }
    if (rleData.isEmpty() && actualSprId != 0) { // ID 0 is expected to be empty
         // This might be a valid empty sprite, or an error from readRawSpriteData if actualSprId was invalid but not 0.
         // readRawSpriteData already handles invalid IDs by returning empty QByteArray.
    }

    sprSheetRleDataCache_.insert(actualSprId, rleData); // Cache it
    return decodeSpriteRleData(rleData, versionData_.hasAlphaChannel);
}

QImage SpriteManager::getFrameImage(quint32 gameSpriteId, int frame, int patternX_arg, int patternY_arg, int patternZ_arg, int layer_arg) {
    if (!assetsLoaded_) {
        qWarning("SpriteManager::getFrameImage - Assets not loaded.");
        return QImage();
    }

    QSharedPointer<const GameSpriteData> data = getGameSpriteData(gameSpriteId);
    if (!data) {
        qWarning() << "SpriteManager::getFrameImage - No GameSpriteData found for ID" << gameSpriteId;
        return QImage(); // Or return a placeholder "unknown item" sprite
    }

    // Validate and wrap around input pattern/frame/layer values based on sprite's dimensions
    // Ensure denominators are not zero.
    int f = (data->frames > 0) ? (frame % data->frames) : 0;
    int pZ = (data->patternZ > 0) ? (patternZ_arg % data->patternZ) : 0;
    int pY = (data->patternY > 0) ? (patternY_arg % data->patternY) : 0;
    int pX = (data->patternX > 0) ? (patternX_arg % data->patternX) : 0;
    int l = (data->layers > 0) ? (layer_arg % data->layers) : 0;

    // The calculation for spriteSheetIndex needs to match the order sprites are stored in sprSheetIDs
    // Based on readDatEntry, the order is:
    // numSprSheetsToRead = width * height * layers * patternX * patternY * patternZ * frames;
    // So, reading is frame-outermost, then patternZ, then patternY, then patternX, then layers, then height, then width.
    // For a single 32x32 sheet, width and height components of this index are 0.
    // (This assumes spriteWidth/Height in GameSpriteData refer to how many 32x32 sheets make up one "layer-pattern-frame" instance)
    // For now, let's assume GameSpriteData's spriteWidth/Height are 1 for this calculation,
    // meaning each entry in sprSheetIDs is a distinct 32x32 sprite.
    // If a "thing" is e.g. 2x1 sheets (64x32 pixels), then spriteWidth would be 2, spriteHeight 1.
    // The current readDatEntry sets spriteWidth/Height from DAT, which is this sheet count.

    quint32 sheetsPerFrame = static_cast<quint32>(data->spriteWidth) * data->spriteHeight * data->layers * data->patternX * data->patternY * data->patternZ;
    quint32 sheetsPerPatternZ = static_cast<quint32>(data->spriteWidth) * data->spriteHeight * data->layers * data->patternX * data->patternY;
    quint32 sheetsPerPatternY = static_cast<quint32>(data->spriteWidth) * data->spriteHeight * data->layers * data->patternX;
    quint32 sheetsPerPatternX = static_cast<quint32>(data->spriteWidth) * data->spriteHeight * data->layers;
    quint32 sheetsPerLayer = static_cast<quint32>(data->spriteWidth) * data->spriteHeight;

    int spriteSheetIndex =
        f * sheetsPerFrame +
        pZ * sheetsPerPatternZ +
        pY * sheetsPerPatternY +
        pX * sheetsPerPatternX +
        l * sheetsPerLayer;
        // Assuming we always want the top-left sheet of a potential multi-sheet arrangement for this specific "thing"
        // If spriteWidth/Height > 1, this index points to the (0,0)th sheet of that larger sprite.
        // To get other parts of a large sprite, you'd add offsets: (sheet_y * data->spriteWidth + sheet_x)

    if (spriteSheetIndex < 0 || spriteSheetIndex >= data->sprSheetIDs.size()) {
        qWarning() << "SpriteManager::getFrameImage - Calculated spriteSheetIndex" << spriteSheetIndex
                   << "is out of bounds for sprSheetIDs (size" << data->sprSheetIDs.size() << ") for ID" << gameSpriteId;
        return QImage(); // Or placeholder
    }

    quint32 actualSprId = data->sprSheetIDs.value(spriteSheetIndex, 0);
    if (actualSprId == 0) {
        // This is a valid case for parts of an object being transparent.
        return QImage(32,32,QImage::Format_ARGB32_Premultiplied); // Return fully transparent image
    }

    return getSpriteImage(actualSprId);
}
