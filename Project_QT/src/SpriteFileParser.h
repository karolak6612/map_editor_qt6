#ifndef SPRITEFILEPARSER_H
#define SPRITEFILEPARSER_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QMap>

// Forward declarations
class SpriteManager;
struct ClientVersionData;
struct GameSpriteData;

/**
 * @brief SpriteFileParser - Helper class for parsing sprite files
 * 
 * This class extracts all file parsing logic from SpriteManager.cpp to comply with mandate M6.
 * It handles DAT and SPR file parsing, header validation, and data extraction.
 * 
 * Task 011: Refactor large source files - Extract file parsing from SpriteManager.cpp
 */
class SpriteFileParser : public QObject
{
    Q_OBJECT

public:
    explicit SpriteFileParser(SpriteManager* spriteManager, QObject *parent = nullptr);
    ~SpriteFileParser() override;

    // Main parsing methods
    bool parseDatFile(QFile& file, QString& error, QStringList& warnings);
    bool parseSprFile(QFile& file, QString& error);

    // Header parsing methods
    bool parseDatHeader(QDataStream& stream, QString& error);
    bool parseSprHeader(QDataStream& stream, QString& error);

    // Content loading methods
    bool loadDatContents(QDataStream& stream, QString& error, QStringList& warnings);
    bool loadSpriteAddresses(QDataStream& stream, QString& error);

    // Entry reading methods
    bool readDatEntry(QDataStream& stream, quint32 gameSpriteId, QString& error, QStringList& warnings);
    bool readSpriteData(QDataStream& stream, quint32 spriteId, QString& error);

    // Validation methods
    bool validateDatSignature(quint32 signature, QString& error) const;
    bool validateSprSignature(quint32 signature, QString& error) const;
    bool validateDatCounts(quint16 itemCount, quint16 outfitCount, quint16 effectCount, quint16 missileCount, QString& error) const;
    bool validateSpriteCount(quint32 spriteCount, QString& error) const;

    // Data extraction helpers
    QSharedPointer<GameSpriteData> createGameSpriteData(quint32 gameSpriteId) const;
    bool parseDatFlags(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error, QStringList& warnings);
    bool parseDatDimensions(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseDatAnimation(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseDatSpriteIds(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);

    // Sprite address management
    bool storeSpriteAddress(quint32 spriteId, quint32 address);
    quint32 getSpriteAddress(quint32 spriteId) const;
    void clearSpriteAddresses();

    // Error handling and logging
    void logParsingProgress(const QString& operation, int current, int total) const;
    void logParsingError(const QString& operation, const QString& error) const;
    void logParsingWarning(const QString& operation, const QString& warning) const;

    // Statistics and information
    int getParsedItemCount() const { return parsedItemCount_; }
    int getParsedOutfitCount() const { return parsedOutfitCount_; }
    int getParsedEffectCount() const { return parsedEffectCount_; }
    int getParsedMissileCount() const { return parsedMissileCount_; }
    int getParsedSpriteCount() const { return parsedSpriteCount_; }

    // Configuration
    void setProgressReporting(bool enabled) { progressReporting_ = enabled; }
    bool isProgressReporting() const { return progressReporting_; }
    void setValidationStrict(bool strict) { strictValidation_ = strict; }
    bool isValidationStrict() const { return strictValidation_; }

signals:
    void parsingProgress(const QString& operation, int current, int total);
    void parsingError(const QString& operation, const QString& error);
    void parsingWarning(const QString& operation, const QString& warning);
    void parsingCompleted(const QString& operation, bool success);

private slots:
    void onParsingProgress(const QString& operation, int current, int total);

private:
    SpriteManager* spriteManager_;
    const ClientVersionData* versionData_;

    // Parsing statistics
    int parsedItemCount_;
    int parsedOutfitCount_;
    int parsedEffectCount_;
    int parsedMissileCount_;
    int parsedSpriteCount_;

    // Configuration flags
    bool progressReporting_;
    bool strictValidation_;

    // Sprite address storage
    QMap<quint32, quint32> spriteAddresses_;

    // Helper methods
    void resetStatistics();
    void updateStatistics(const QString& type, int count);
    bool isValidStream(QDataStream& stream, const QString& operation, QString& error) const;
    void connectSignals();
    void disconnectSignals();

    // DAT flag parsing helpers
    bool parseDatFlag(QDataStream& stream, quint8 flagValue, QSharedPointer<GameSpriteData> spriteData, QString& error, QStringList& warnings);
    bool parseGroundFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseLightFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseDisplacementFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseElevationFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseMinimapColorFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);
    bool parseLensHelpFlag(QDataStream& stream, QSharedPointer<GameSpriteData> spriteData, QString& error);

    // Version-specific parsing
    bool isExtendedSprFormat() const;
    bool hasFrameDurations() const;
    bool supportsPatternZ() const;
    quint32 getItemIdOffset() const;
    quint32 getOutfitIdOffset() const;
    quint32 getEffectIdOffset() const;
    quint32 getMissileIdOffset() const;
};

#endif // SPRITEFILEPARSER_H
