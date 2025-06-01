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
class ImageSpace;

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

// --- Core Item Properties from .dat files (Task 64) ---
struct CoreItemProperties {
    quint16 clientId = 0;           // Client ID (sprite ID)
    SpriteDatFlagValues flags;      // Core flags from .dat file

    // Core behavioral properties from .dat
    bool isGround = false;
    bool isGroundBorder = false;
    bool isOnBottom = false;
    bool isOnTop = false;
    bool isContainer = false;
    bool isStackable = false;
    bool isForceUse = false;
    bool isMultiUse = false;
    bool isWritable = false;
    bool isWritableOnce = false;
    bool isFluidContainer = false;
    bool isSplash = false;
    bool isNotWalkable = false;
    bool isNotMoveable = false;
    bool isBlockProjectile = false;
    bool isNotPathable = false;
    bool isPickupable = false;
    bool isHangable = false;
    bool hasHookSouth = false;
    bool hasHookEast = false;
    bool isRotateable = false;
    bool hasLight = false;
    bool isDontHide = false;
    bool isTranslucent = false;
    bool hasDisplacement = false;
    bool hasElevation = false;
    bool isLyingCorpse = false;
    bool isAnimateAlways = false;
    bool hasMinimapColor = false;
    bool hasLensHelp = false;
    bool isFullGround = false;
    bool hasLook = false;

    // Additional properties from .dat
    quint16 groundSpeed = 0;        // For ground items
    quint16 lightLevel = 0;         // Light intensity
    quint16 lightColor = 0;         // Light color
    quint16 minimapColor = 0;       // Minimap color
    QPoint displacement;            // Displacement offset
    quint16 elevation = 0;          // Elevation height
    quint8 topOrder = 1;           // Rendering order

    // Animation properties
    bool isAnimated = false;
    qint32 animationLoopCount = 0;
    qint8 animationStartFrame = 0;
    QVector<QPair<quint32, quint32>> frameDurations;
};

// --- GameSpriteData Class/Struct (Enhanced for Task 64) ---
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

    // Task 64: Core item properties from .dat file
    CoreItemProperties coreProperties;
};

// --- SpriteManager Class ---
class SpriteManager : public QObject {
    Q_OBJECT
public:
    explicit SpriteManager(QObject* parent = nullptr);
    ~SpriteManager();

    // Task 54: Singleton access for Item rendering
    static SpriteManager* getInstance();
    static void setInstance(SpriteManager* instance);

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

    // ImageSpace integration (Task 35 requirement)
    ImageSpace* getImageSpace() const { return imageSpace_; }

    // Task 64: Core item properties access (unified .dat/.spr loading)
    const CoreItemProperties* getCoreItemProperties(quint16 clientId) const;
    bool hasCoreItemProperties(quint16 clientId) const;
    QList<quint16> getAllClientIds() const;

    // Task 64: Client ID to Sprite ID mapping
    quint16 getClientIdForSprite(quint32 spriteId) const;
    QList<quint32> getSpriteIdsForClient(quint16 clientId) const;

    // Task 64: Core property queries (for ItemManager integration)
    bool isClientIdWalkable(quint16 clientId) const;
    bool isClientIdStackable(quint16 clientId) const;
    bool isClientIdMoveable(quint16 clientId) const;
    bool isClientIdPickupable(quint16 clientId) const;
    bool isClientIdGround(quint16 clientId) const;
    bool isClientIdContainer(quint16 clientId) const;
    quint8 getClientIdTopOrder(quint16 clientId) const;
    quint16 getClientIdLightLevel(quint16 clientId) const;
    quint16 getClientIdMinimapColor(quint16 clientId) const;

    // Helper to declare Q_ENUMs if they are moved inside SpriteManager
    // static void declareQtEnums();

private:
    // SPR file parsing (Task 35 requirement)
    bool parseSprFile(QFile& file, QString& error);
    bool parseSprHeader(QDataStream& stream, QString& error);
    bool loadSpriteAddresses(QDataStream& stream, QString& error);

    // DAT file parsing (Task 35 requirement)
    bool parseDatFile(QFile& file, QString& error, QStringList& warnings);
    bool parseDatHeader(QDataStream& stream, QString& error);
    bool loadDatContents(QDataStream& stream, QString& error, QStringList& warnings);
    bool readDatEntry(QDataStream& stream, quint32 gameSpriteId, QString& error, QStringList& warnings);

    // Sprite data processing (Task 35 requirement)
    QByteArray readRawSpriteData(quint32 actualSprId, QString& error);
    QImage decodeSpriteRleData(const QByteArray& rleData, bool hasAlpha) const;

    // Task 64: Core item properties loading from .dat files
    bool loadCoreItemProperties(QDataStream& stream, QString& error, QStringList& warnings);
    bool parseCoreItemFlags(QDataStream& stream, quint16 clientId, CoreItemProperties& properties, QString& error, QStringList& warnings);
    void applyCoreItemFlag(quint8 flagValue, CoreItemProperties& properties, QDataStream& stream, QString& error, QStringList& warnings);
    void buildClientIdMappings();
    CoreItemProperties extractCorePropertiesFromGameSprite(const GameSpriteData& spriteData) const;

    ClientVersionData versionData_;
    bool assetsLoaded_ = false;

    // ImageSpace for intermediate sprite storage (Task 35 requirement)
    ImageSpace* imageSpace_;

    // Sprite data caches (Task 35 requirement)
    QMap<quint32, QSharedPointer<GameSpriteData>> gameSpriteMetadataCache_;
    QMap<quint32, QByteArray> sprSheetRleDataCache_;
    QMap<quint32, quint32> sprSheetAddresses_;  // Sprite ID -> File offset mapping

    // SPR file data (Task 35 requirement)
    quint32 sprSignature_ = 0;
    quint32 sprSpriteCount_ = 0;

    // DAT file data (Task 35 requirement)
    quint32 datSignature_ = 0;
    quint16 datItemCount_ = 0;
    quint16 datOutfitCount_ = 0;
    quint16 datEffectCount_ = 0;
    quint16 datMissileCount_ = 0;

    // Task 64: Core item properties storage
    QMap<quint16, CoreItemProperties> coreItemPropertiesMap_;  // Client ID -> Core Properties
    QMap<quint32, quint16> spriteIdToClientId_;               // Sprite ID -> Client ID
    QMap<quint16, QList<quint32>> clientIdToSpriteIds_;       // Client ID -> List of Sprite IDs
};

// Needed if DatFormat and SpriteDatFlags are Q_ENUM/Q_FLAG'd within SpriteManager
// Q_DECLARE_METATYPE(SpriteManager::DatFormat)
// Q_DECLARE_METATYPE(SpriteManager::SpriteDatFlags)


#endif // SPRITEMANAGER_H
