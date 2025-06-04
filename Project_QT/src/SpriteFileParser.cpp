#include "SpriteFileParser.h"
#include "SpriteManager.h"
#include <QDebug>

SpriteFileParser::SpriteFileParser(SpriteManager* spriteManager, QObject *parent)
    : QObject(parent)
    , spriteManager_(spriteManager)
    , versionData_(nullptr)
    , parsedItemCount_(0)
    , parsedOutfitCount_(0)
    , parsedEffectCount_(0)
    , parsedMissileCount_(0)
    , parsedSpriteCount_(0)
    , progressReporting_(true)
    , strictValidation_(true)
{
    connectSignals();
    resetStatistics();
}

SpriteFileParser::~SpriteFileParser()
{
    disconnectSignals();
    clearSpriteAddresses();
}

// Main parsing methods
bool SpriteFileParser::parseDatFile(QFile& file, QString& error, QStringList& warnings)
{
    if (!spriteManager_) {
        error = "SpriteFileParser: SpriteManager not available";
        return false;
    }

    versionData_ = spriteManager_->getCurrentVersionData();
    if (!versionData_) {
        error = "SpriteFileParser: Version data not available";
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    if (!parseDatHeader(stream, error)) {
        logParsingError("DAT Header", error);
        return false;
    }

    if (!loadDatContents(stream, error, warnings)) {
        logParsingError("DAT Contents", error);
        return false;
    }

    emit parsingCompleted("DAT File", true);
    return true;
}

bool SpriteFileParser::parseSprFile(QFile& file, QString& error)
{
    if (!spriteManager_) {
        error = "SpriteFileParser: SpriteManager not available";
        return false;
    }

    versionData_ = spriteManager_->getCurrentVersionData();
    if (!versionData_) {
        error = "SpriteFileParser: Version data not available";
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    if (!parseSprHeader(stream, error)) {
        logParsingError("SPR Header", error);
        return false;
    }

    if (!loadSpriteAddresses(stream, error)) {
        logParsingError("SPR Addresses", error);
        return false;
    }

    emit parsingCompleted("SPR File", true);
    return true;
}

// Header parsing methods
bool SpriteFileParser::parseDatHeader(QDataStream& stream, QString& error)
{
    if (!isValidStream(stream, "DAT Header", error)) {
        return false;
    }

    quint32 signature;
    stream >> signature;
    
    if (!validateDatSignature(signature, error)) {
        return false;
    }

    quint16 itemCount, outfitCount, effectCount, missileCount;
    stream >> itemCount >> outfitCount >> effectCount >> missileCount;

    if (!validateDatCounts(itemCount, outfitCount, effectCount, missileCount, error)) {
        return false;
    }

    // Store counts for later use
    updateStatistics("items", itemCount);
    updateStatistics("outfits", outfitCount);
    updateStatistics("effects", effectCount);
    updateStatistics("missiles", missileCount);

    logParsingProgress("DAT Header", 1, 1);
    return true;
}

bool SpriteFileParser::parseSprHeader(QDataStream& stream, QString& error)
{
    if (!isValidStream(stream, "SPR Header", error)) {
        return false;
    }

    quint32 signature;
    stream >> signature;
    
    if (!validateSprSignature(signature, error)) {
        return false;
    }

    quint32 spriteCount;
    if (isExtendedSprFormat()) {
        stream >> spriteCount;
    } else {
        quint16 spriteCount16;
        stream >> spriteCount16;
        spriteCount = spriteCount16;
    }

    if (!validateSpriteCount(spriteCount, error)) {
        return false;
    }

    updateStatistics("sprites", spriteCount);
    logParsingProgress("SPR Header", 1, 1);
    return true;
}

// Content loading methods
bool SpriteFileParser::loadDatContents(QDataStream& stream, QString& error, QStringList& warnings)
{
    if (!isValidStream(stream, "DAT Contents", error)) {
        return false;
    }

    int totalEntries = parsedItemCount_ + parsedOutfitCount_ + parsedEffectCount_ + parsedMissileCount_;
    int currentEntry = 0;

    // Load items
    for (int i = 0; i < parsedItemCount_; ++i) {
        quint32 clientItemId = getItemIdOffset() + i;
        
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading item %1").arg(i);
            return false;
        }

        if (!readDatEntry(stream, clientItemId, error, warnings)) {
            warnings.append(QString("Error reading item %1: %2").arg(i).arg(error));
        }

        currentEntry++;
        if (progressReporting_ && (currentEntry % 100 == 0 || currentEntry == totalEntries)) {
            logParsingProgress("DAT Items", currentEntry, totalEntries);
        }
    }

    // Load outfits
    for (int i = 0; i < parsedOutfitCount_; ++i) {
        quint32 outfitId = getOutfitIdOffset() + i;
        
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading outfit %1").arg(i);
            return false;
        }

        if (!readDatEntry(stream, outfitId, error, warnings)) {
            warnings.append(QString("Error reading outfit %1: %2").arg(i).arg(error));
        }

        currentEntry++;
        if (progressReporting_ && (currentEntry % 100 == 0 || currentEntry == totalEntries)) {
            logParsingProgress("DAT Outfits", currentEntry, totalEntries);
        }
    }

    // Load effects
    for (int i = 0; i < parsedEffectCount_; ++i) {
        quint32 effectId = getEffectIdOffset() + i;
        
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading effect %1").arg(i);
            return false;
        }

        if (!readDatEntry(stream, effectId, error, warnings)) {
            warnings.append(QString("Error reading effect %1: %2").arg(i).arg(error));
        }

        currentEntry++;
        if (progressReporting_ && (currentEntry % 100 == 0 || currentEntry == totalEntries)) {
            logParsingProgress("DAT Effects", currentEntry, totalEntries);
        }
    }

    // Load missiles
    for (int i = 0; i < parsedMissileCount_; ++i) {
        quint32 missileId = getMissileIdOffset() + i;
        
        if (stream.atEnd()) {
            error = QString("Unexpected end of DAT file while reading missile %1").arg(i);
            return false;
        }

        if (!readDatEntry(stream, missileId, error, warnings)) {
            warnings.append(QString("Error reading missile %1: %2").arg(i).arg(error));
        }

        currentEntry++;
        if (progressReporting_ && (currentEntry % 100 == 0 || currentEntry == totalEntries)) {
            logParsingProgress("DAT Missiles", currentEntry, totalEntries);
        }
    }

    return true;
}

bool SpriteFileParser::loadSpriteAddresses(QDataStream& stream, QString& error)
{
    if (!isValidStream(stream, "SPR Addresses", error)) {
        return false;
    }

    clearSpriteAddresses();
    spriteAddresses_.reserve(parsedSpriteCount_);

    for (quint32 i = 1; i <= static_cast<quint32>(parsedSpriteCount_); ++i) {
        if (stream.atEnd()) {
            error = QString("Unexpected end of SPR file while reading sprite address %1").arg(i);
            return false;
        }

        quint32 address;
        stream >> address;

        if (!storeSpriteAddress(i, address)) {
            error = QString("Failed to store sprite address for sprite %1").arg(i);
            return false;
        }

        if (progressReporting_ && (i % 1000 == 0 || i == static_cast<quint32>(parsedSpriteCount_))) {
            logParsingProgress("SPR Addresses", i, parsedSpriteCount_);
        }
    }

    return true;
}

// Entry reading methods
bool SpriteFileParser::readDatEntry(QDataStream& stream, quint32 gameSpriteId, QString& error, QStringList& warnings)
{
    if (!isValidStream(stream, "DAT Entry", error)) {
        return false;
    }

    QSharedPointer<GameSpriteData> spriteData = createGameSpriteData(gameSpriteId);
    if (!spriteData) {
        error = "Failed to create GameSpriteData";
        return false;
    }

    // Parse flags
    if (!parseDatFlags(stream, spriteData, error, warnings)) {
        return false;
    }

    // Parse dimensions
    if (!parseDatDimensions(stream, spriteData, error)) {
        return false;
    }

    // Parse animation data
    if (!parseDatAnimation(stream, spriteData, error)) {
        return false;
    }

    // Parse sprite IDs
    if (!parseDatSpriteIds(stream, spriteData, error)) {
        return false;
    }

    // Store the sprite data in SpriteManager
    if (spriteManager_) {
        // This would need to be implemented in SpriteManager
        // spriteManager_->storeSpriteData(gameSpriteId, spriteData);
    }

    return true;
}

bool SpriteFileParser::readSpriteData(QDataStream& stream, quint32 spriteId, QString& error)
{
    if (!isValidStream(stream, "SPR Data", error)) {
        return false;
    }

    // This would implement actual sprite pixel data reading
    // For now, this is a placeholder
    Q_UNUSED(spriteId);
    
    return true;
}

// Validation methods
bool SpriteFileParser::validateDatSignature(quint32 signature, QString& error) const
{
    if (versionData_ && versionData_->expectedDatSignature != 0 && signature != versionData_->expectedDatSignature) {
        error = QString("DAT file signature mismatch. Expected %1, got %2")
                    .arg(versionData_->expectedDatSignature, 0, 16)
                    .arg(signature, 0, 16);
        return false;
    }
    return true;
}

bool SpriteFileParser::validateSprSignature(quint32 signature, QString& error) const
{
    if (versionData_ && versionData_->expectedSprSignature != 0 && signature != versionData_->expectedSprSignature) {
        error = QString("SPR file signature mismatch. Expected %1, got %2")
                    .arg(versionData_->expectedSprSignature, 0, 16)
                    .arg(signature, 0, 16);
        return false;
    }
    return true;
}

bool SpriteFileParser::validateDatCounts(quint16 itemCount, quint16 outfitCount, quint16 effectCount, quint16 missileCount, QString& error) const
{
    if (itemCount == 0 || itemCount > 50000) {
        error = QString("Invalid item count in DAT file: %1").arg(itemCount);
        return false;
    }

    if (strictValidation_) {
        if (outfitCount > 10000) {
            error = QString("Invalid outfit count in DAT file: %1").arg(outfitCount);
            return false;
        }
        if (effectCount > 5000) {
            error = QString("Invalid effect count in DAT file: %1").arg(effectCount);
            return false;
        }
        if (missileCount > 1000) {
            error = QString("Invalid missile count in DAT file: %1").arg(missileCount);
            return false;
        }
    }

    return true;
}

bool SpriteFileParser::validateSpriteCount(quint32 spriteCount, QString& error) const
{
    if (spriteCount == 0 || spriteCount > 150000) {
        error = QString("Invalid sprite count in SPR file: %1").arg(spriteCount);
        return false;
    }
    return true;
}

// Data extraction helpers
QSharedPointer<GameSpriteData> SpriteFileParser::createGameSpriteData(quint32 gameSpriteId) const
{
    QSharedPointer<GameSpriteData> spriteData = QSharedPointer<GameSpriteData>(new GameSpriteData());
    spriteData->id = gameSpriteId;
    return spriteData;
}

bool SpriteFileParser::parseDatFlags(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error, QStringList& warnings)
{
    quint8 flagValue;
    while (true) {
        if (stream.atEnd()) {
            error = "Unexpected end of stream while reading flags";
            return false;
        }

        stream >> flagValue;
        if (flagValue == 0xFF) {
            break; // End of flags marker
        }

        if (!parseDatFlag(stream, flagValue, spriteData, error, warnings)) {
            return false;
        }
    }
    return true;
}

bool SpriteFileParser::parseDatDimensions(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error)
{
    if (stream.atEnd()) {
        error = "Unexpected end of stream before dimensions";
        return false;
    }

    stream >> spriteData->spriteWidth >> spriteData->spriteHeight;

    if (spriteData->spriteWidth == 0) spriteData->spriteWidth = 1;
    if (spriteData->spriteHeight == 0) spriteData->spriteHeight = 1;

    if (spriteData->spriteWidth > 1 || spriteData->spriteHeight > 1) {
        if (stream.atEnd()) {
            error = "Unexpected end of stream before exact size";
            return false;
        }
        quint8 exactSize;
        stream >> exactSize; // Read and store if needed
    }

    return true;
}

bool SpriteFileParser::parseDatAnimation(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error)
{
    if (stream.atEnd()) {
        error = "Unexpected end of stream before animation data";
        return false;
    }

    stream >> spriteData->layers >> spriteData->patternX >> spriteData->patternY;

    if (supportsPatternZ()) {
        if (stream.atEnd()) {
            error = "Unexpected end of stream before patternZ";
            return false;
        }
        stream >> spriteData->patternZ;
    } else {
        spriteData->patternZ = 1;
    }

    if (stream.atEnd()) {
        error = "Unexpected end of stream before frames";
        return false;
    }
    stream >> spriteData->frames;
    spriteData->isAnimated = spriteData->frames > 1;

    if (spriteData->isAnimated && hasFrameDurations()) {
        // Parse frame duration data
        if (stream.atEnd()) {
            error = "Unexpected end of stream before animation data";
            return false;
        }

        quint8 async;
        stream >> async;
        stream >> spriteData->animationLoopCount;

        qint8 startFrame;
        stream >> startFrame;
        spriteData->animationStartFrame = startFrame;

        spriteData->frameDurations.reserve(spriteData->frames);
        for (quint8 i = 0; i < spriteData->frames; ++i) {
            if (stream.atEnd()) {
                error = QString("Unexpected end of stream reading frame duration %1").arg(i);
                return false;
            }
            quint32 min, max;
            stream >> min >> max;
            spriteData->frameDurations.append(QPair<quint32, quint32>(min, max));
        }
    }

    return true;
}

bool SpriteFileParser::parseDatSpriteIds(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error)
{
    // Calculate total sprites needed
    quint32 totalSprites = spriteData->spriteWidth * spriteData->spriteHeight *
                          spriteData->layers * spriteData->patternX *
                          spriteData->patternY * spriteData->patternZ *
                          spriteData->frames;

    spriteData->spriteIds.reserve(totalSprites);

    for (quint32 i = 0; i < totalSprites; ++i) {
        if (stream.atEnd()) {
            error = QString("Unexpected end of stream reading sprite ID %1/%2").arg(i).arg(totalSprites);
            return false;
        }

        quint32 spriteId;
        stream >> spriteId;
        spriteData->spriteIds.append(spriteId);
    }

    return true;
}

// Sprite address management
bool SpriteFileParser::storeSpriteAddress(quint32 spriteId, quint32 address)
{
    spriteAddresses_.insert(spriteId, address);
    return true;
}

quint32 SpriteFileParser::getSpriteAddress(quint32 spriteId) const
{
    return spriteAddresses_.value(spriteId, 0);
}

void SpriteFileParser::clearSpriteAddresses()
{
    spriteAddresses_.clear();
}

// Error handling and logging
void SpriteFileParser::logParsingProgress(const QString& operation, int current, int total) const
{
    if (progressReporting_) {
        qDebug() << "SpriteFileParser:" << operation << "progress:" << current << "/" << total;
        emit parsingProgress(operation, current, total);
    }
}

void SpriteFileParser::logParsingError(const QString& operation, const QString& error) const
{
    qCritical() << "SpriteFileParser:" << operation << "error:" << error;
    emit parsingError(operation, error);
}

void SpriteFileParser::logParsingWarning(const QString& operation, const QString& warning) const
{
    qWarning() << "SpriteFileParser:" << operation << "warning:" << warning;
    emit parsingWarning(operation, warning);
}

// Private slots
void SpriteFileParser::onParsingProgress(const QString& operation, int current, int total)
{
    logParsingProgress(operation, current, total);
}

// Private helper methods
void SpriteFileParser::resetStatistics()
{
    parsedItemCount_ = 0;
    parsedOutfitCount_ = 0;
    parsedEffectCount_ = 0;
    parsedMissileCount_ = 0;
    parsedSpriteCount_ = 0;
}

void SpriteFileParser::updateStatistics(const QString& type, int count)
{
    if (type == "items") parsedItemCount_ = count;
    else if (type == "outfits") parsedOutfitCount_ = count;
    else if (type == "effects") parsedEffectCount_ = count;
    else if (type == "missiles") parsedMissileCount_ = count;
    else if (type == "sprites") parsedSpriteCount_ = count;
}

bool SpriteFileParser::isValidStream(QDataStream& stream, const QString& operation, QString& error) const
{
    if (stream.atEnd()) {
        error = QString("Stream ended unexpectedly during %1").arg(operation);
        return false;
    }
    return true;
}

void SpriteFileParser::connectSignals()
{
    connect(this, &SpriteFileParser::parsingProgress, this, &SpriteFileParser::onParsingProgress);
}

void SpriteFileParser::disconnectSignals()
{
    disconnect(this, nullptr, this, nullptr);
}

// Version-specific parsing helpers
bool SpriteFileParser::isExtendedSprFormat() const
{
    return versionData_ ? versionData_->isExtendedSpr : false;
}

bool SpriteFileParser::hasFrameDurations() const
{
    return versionData_ ? versionData_->hasFrameDurations : false;
}

bool SpriteFileParser::supportsPatternZ() const
{
    return versionData_ ? (versionData_->datFormat >= DatFormat::Format_780) : false;
}

quint32 SpriteFileParser::getItemIdOffset() const
{
    return 100; // Standard Tibia item ID offset
}

quint32 SpriteFileParser::getOutfitIdOffset() const
{
    return 20000; // Outfit cache key offset
}

quint32 SpriteFileParser::getEffectIdOffset() const
{
    return 30000; // Effect cache key offset
}

quint32 SpriteFileParser::getMissileIdOffset() const
{
    return 40000; // Missile cache key offset
}

// Placeholder for DAT flag parsing - would need full implementation
bool SpriteFileParser::parseDatFlag(QDataStream& stream, quint8 flagValue, QSharedPointer<GameSpriteData> spriteData, QString& error, QStringList& warnings)
{
    // This is a simplified version - full implementation would handle all flag types
    Q_UNUSED(stream);
    Q_UNUSED(flagValue);
    Q_UNUSED(spriteData);
    Q_UNUSED(error);
    Q_UNUSED(warnings);
    return true;
}


