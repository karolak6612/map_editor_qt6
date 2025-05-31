#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QSharedPointer>
#include <QPoint> // For GameSpriteData::drawOffset

// Forward declarations
class QFile;

// --- Enums from wxwidgets/client_version.h ---
enum class DatFormat {
    Unknown,
    Format_740,  // DAT_FORMAT_74
    Format_755,  // DAT_FORMAT_755
    Format_780,  // DAT_FORMAT_78
    Format_860,  // DAT_FORMAT_86
    Format_960,  // DAT_FORMAT_96
    Format_1010, // DAT_FORMAT_1010
    Format_1050, // DAT_FORMAT_1050 (unused in wx RME for DatFormat enum, but good for completeness)
    Format_1057  // DAT_FORMAT_1057
};
// Q_ENUM(DatFormat) // Add if SpriteManager becomes QObject and this needs meta-type system

enum class SpriteDatFlags : quint32 {
    None = 0,
    Ground = 1 << 0,
    GroundBorder = 1 << 1,
    OnBottom = 1 << 2,
    OnTop = 1 << 3,
    Container = 1 << 4,
    Stackable = 1 << 5,
    ForceUse = 1 << 6,
    MultiUse = 1 << 7,
    Writable = 1 << 8,            // Original value 8
    Chargeable_780 = 1 << 8,      // For 7.80+, also value 8
    WritableOnce = 1 << 9,
    FluidContainer = 1 << 10,
    Splash = 1 << 11,
    NotWalkable = 1 << 12,
    NotMoveable = 1 << 13,
    BlockProjectile = 1 << 14,
    NotPathable = 1 << 15,
    Pickupable = 1 << 16,         // Original value 16
    NoMoveAnimation_1010 = 1 << 16, // For 10.10+, also value 16
    Hangable = 1 << 17,
    HookSouth = 1 << 18,
    HookEast = 1 << 19,
    Rotateable = 1 << 20,
    Light = 1 << 21,
    DontHide = 1 << 22,
    Translucent = 1 << 23,
    Displacement = 1 << 24,
    Elevation = 1 << 25,
    LyingCorpse = 1 << 26,
    AnimateAlways = 1 << 27,
    MinimapColor = 1 << 28,
    LensHelp = 1 << 29,
    FullGround = 1 << 30,
    Look = 1U << 31 // Ensure unsigned for bit 31
};
Q_DECLARE_FLAGS(SpriteDatFlagValues, SpriteDatFlags)
// Q_FLAG(SpriteDatFlags) // Add if SpriteManager becomes QObject

// --- ClientVersionData Struct ---
struct ClientVersionData {
    QString sprPath;
    QString datPath;
    DatFormat datFormat = DatFormat::Unknown;
    quint32 clientVersionNumber = 0;
    bool isExtendedSpr = false;
    bool hasAlphaChannel = false;
    bool hasFrameDurations = false;
    quint32 expectedDatSignature = 0;
    quint32 expectedSprSignature = 0;
};

// --- SpriteLightData Struct ---
struct SpriteLightData {
    quint8 intensity = 0;
    quint8 color = 0;
    bool hasLight = false;
};

// --- GameSpriteData Class/Struct ---
class GameSpriteData {
public:
    GameSpriteData() = default;

    quint32 id = 0;
    quint8 spriteWidth = 1;  // Default to 1 to avoid division by zero if uninitialized
    quint8 spriteHeight = 1; // Default to 1
    quint8 layers = 0;
    quint8 patternX = 0;
    quint8 patternY = 0;
    quint8 patternZ = 0;
    quint8 frames = 0;

    SpriteLightData light;
    quint16 minimapColor = 0;
    QPoint drawOffset;
    quint16 drawHeight = 0;
    SpriteDatFlagValues flags;

    bool isAnimated = false;
    qint32 animationLoopCount = 0;
    qint8 animationStartFrame = 0;
    QVector<QPair<quint32, quint32>> frameDurations;

    QVector<quint32> sprSheetIDs;
    QMap<quint32, QByteArray> rawSpriteRleData; // Consider changing to offsets/sizes later
};

// --- SpriteManager Class ---
class SpriteManager : public QObject {
    Q_OBJECT
public:
    explicit SpriteManager(QObject* parent = nullptr);
    ~SpriteManager();

    bool loadAssets(const ClientVersionData& clientVersion, QString& error, QStringList& warnings);
    void unloadAssets();

    QSharedPointer<const GameSpriteData> getGameSpriteData(quint32 gameSpriteId) const;
    QImage getSpriteImage(quint32 actualSprId);
    QImage getFrameImage(quint32 gameSpriteId, int frame = 0, int patternX = 0, int patternY = 0, int patternZ = 0, int layer = 0);

    quint32 getSpriteCount() const;
    quint16 getItemTypeCount() const;
    quint16 getOutfitCount() const;
    quint16 getEffectCount() const;
    quint16 getMissileCount() const;

    const ClientVersionData* getCurrentVersionData() const;

    // Helper to declare Q_ENUMs if they are moved inside SpriteManager
    // static void declareQtEnums();

private:
    bool parseSprFile(QFile& file, QString& error);
    bool parseDatFile(QFile& file, QString& error, QStringList& warnings);
    bool readDatEntry(QFile& file, quint32 gameSpriteId, QString& error);
    QImage decodeSpriteRleData(const QByteArray& rleData, bool hasAlpha) const;

    ClientVersionData versionData_;
    bool assetsLoaded_ = false;

    QMap<quint32, QSharedPointer<GameSpriteData>> gameSpriteMetadataCache_;
    QMap<quint32, QByteArray> sprSheetRleDataCache_; // Alternative: QMap<quint32, quint32> sprSheetAddresses_;

    quint32 sprSignature_ = 0;
    quint32 sprSpriteCount_ = 0;

    quint32 datSignature_ = 0;
    quint16 datItemCount_ = 0;
    quint16 datOutfitCount_ = 0;
    quint16 datEffectCount_ = 0;
    quint16 datMissileCount_ = 0;
};

// Needed if DatFormat and SpriteDatFlags are Q_ENUM/Q_FLAG'd within SpriteManager
// Q_DECLARE_METATYPE(SpriteManager::DatFormat)
// Q_DECLARE_METATYPE(SpriteManager::SpriteDatFlags)


#endif // SPRITEMANAGER_H
