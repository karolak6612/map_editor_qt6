#include "ItemManager.h"
#include "Item.h" // Item class from Task 13
#include "SpriteManager.h" // Task 64: For core properties access
#include <QFile>
#include <QDataStream>
#include <QXmlStreamReader> // For optional XML parsing
#include <QDebug>
#include <QCoreApplication> // For QCoreApplication::instance()
#include <QVariant> // For QVariant::fromValue()

// Static member initialization
ItemManager* ItemManager::s_instance = nullptr;
ItemProperties ItemManager::defaultProperties_; 

// OTB specific enums (mirror from wxwidgets/items.h or define appropriately)
// These might be better in a private section of ItemManager.h or a dedicated OTB parser helper later
enum RootAttrib_OTB : quint8 { // Changed from int to quint8 for consistency with QDataStream reads
    ROOT_ATTR_OTB_VERSION = 0x01 
};

enum ItemAttrib_OTB : quint8 { 
    ITEM_ATTR_FIRST_OTB = 0x10, // Start of item attributes
    ITEM_ATTR_SERVERID_OTB = ITEM_ATTR_FIRST_OTB, // deprecated, use group based id
    ITEM_ATTR_CLIENTID_OTB,
    ITEM_ATTR_NAME_OTB,         //deprecated
    ITEM_ATTR_DESCR_OTB,        //deprecated
    ITEM_ATTR_SPEED_OTB,
    ITEM_ATTR_SLOT_OTB,         //deprecated
    ITEM_ATTR_MAXITEMS_OTB,     //deprecated count of items in container
    ITEM_ATTR_WEIGHT_OTB,       //deprecated
    ITEM_ATTR_WEAPON_OTB,       //deprecated
    ITEM_ATTR_AMU_OTB,          //deprecated
    ITEM_ATTR_ARMOR_OTB,        //deprecated
    ITEM_ATTR_MAGLEVEL_OTB,     //deprecated
    ITEM_ATTR_MAGFIELDTYPE_OTB, //deprecated
    ITEM_ATTR_WRITEABLE_OTB,    //deprecated
    ITEM_ATTR_ROTATETO_OTB,     //deprecated
    ITEM_ATTR_DECAY_OTB,        //deprecated
    ITEM_ATTR_SPRITEHASH_OTB,
    ITEM_ATTR_MINIMAPCOLOR_OTB,
    ITEM_ATTR_07_OTB,           //deprecated
    ITEM_ATTR_08_OTB,           //deprecated
    ITEM_ATTR_LIGHT_OTB,        //deprecated old light

    // 1-byte aligned
    ITEM_ATTR_DECAY2_OTB,           //deprecated
    ITEM_ATTR_WEAPON2_OTB,          //deprecated
    ITEM_ATTR_AMU2_OTB,             //deprecated
    ITEM_ATTR_ARMOR2_OTB,           //deprecated
    ITEM_ATTR_WRITEABLE2_OTB,       //deprecated
    ITEM_ATTR_LIGHT2_OTB,           //new light
    ITEM_ATTR_TOPORDER_OTB,
    ITEM_ATTR_WRITEABLE3_OTB,       //deprecated

    ITEM_ATTR_WAREID_OTB,           // Market
    ITEM_ATTR_CLASSIFICATION_OTB,   // Tibia 12.81 item classification
    ITEM_ATTR_LAST_OTB
};

// Flags from OTB, matching RME's interpretation
enum ItemFlags_OTB : quint32 {
    FLAG_UNPASSABLE_OTB         = 1 << 0,  // block passage
    FLAG_BLOCK_MISSILES_OTB     = 1 << 1,
    FLAG_BLOCK_PATHFINDER_OTB   = 1 << 2,
    FLAG_HAS_ELEVATION_OTB      = 1 << 3,
    FLAG_USEABLE_OTB            = 1 << 4,  // can be used
    FLAG_PICKUPABLE_OTB         = 1 << 5,
    FLAG_MOVEABLE_OTB           = 1 << 6,
    FLAG_STACKABLE_OTB          = 1 << 7,
    FLAG_FLOORCHANGEDOWN_OTB    = 1 << 8,  // is a floor changer going down
    FLAG_FLOORCHANGENORTH_OTB   = 1 << 9,  // is a floor changer going north
    FLAG_FLOORCHANGEEAST_OTB    = 1 << 10, // is a floor changer going east
    FLAG_FLOORCHANGESOUTH_OTB   = 1 << 11, // is a floor changer going south
    FLAG_FLOORCHANGEWEST_OTB    = 1 << 12, // is a floor changer going west
    FLAG_ALWAYSONTOP_OTB        = 1 << 13, // is always on top (draw last) -> RME: means always on bottom
    FLAG_READABLE_OTB           = 1 << 14,
    FLAG_ROTABLE_OTB            = 1 << 15,
    FLAG_HANGABLE_OTB           = 1 << 16,
    FLAG_HOOK_EAST_OTB          = 1 << 17,
    FLAG_HOOK_SOUTH_OTB         = 1 << 18,
    FLAG_CANNOTDECAY_OTB        = 1 << 19, // deprecated
    FLAG_ALLOWDISTREAD_OTB      = 1 << 20,
    FLAG_UNUSED_OTB             = 1 << 21, // was animation
    FLAG_CLIENTCHARGES_OTB      = 1 << 22, // show charges type and count like potions, deprecated
    FLAG_IGNORE_LOOK_OTB        = 1 << 23, // look through
    FLAG_CLOTHING_OTB           = 1 << 24, // is clothing
    FLAG_MARKET_OTB             = 1 << 25, // can be sold on market
    FLAG_DEFAULTACTION_OTB      = 1 << 26, // new default action for items.
    FLAG_WRAPABLE_OTB           = 1 << 27, // can be wrapped
    FLAG_UNWRAPABLE_OTB         = 1 << 28, // can be unwrapped
    FLAG_TOPITEM_OTB            = 1 << 29, // another always on top flag
};


// Singleton Implementation
ItemManager* ItemManager::instance() {
    if (!s_instance) {
        // Parent to QApplication for automatic cleanup
        QObject* appParent = QCoreApplication::instance();
        s_instance = new ItemManager(appParent);

        // Store reference in QApplication for easy access
        if (appParent) {
            appParent->setProperty("ItemManager", QVariant::fromValue(s_instance));
        }
    }
    return s_instance;
}

ItemManager::ItemManager(QObject *parent) : QObject(parent) {
    defaultProperties_.name = "Unknown Item Type"; // Provide a default name
    defaultProperties_.isBlocking = true; // Default unknown items to blocking
}

ItemManager::~ItemManager() {
    clearDefinitions();
    // Clear static instance pointer when destroyed
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void ItemManager::clearDefinitions() {
    itemPropertiesMap_.clear();
    clientIdToServerIds_.clear();
    loaded_ = false;
    maxServerId_ = 0;
    majorVersion_ = 0;
    minorVersion_ = 0;
    buildNumber_ = 0;
    emit definitionsCleared();
    qDebug() << "Item definitions cleared.";
}

bool ItemManager::isLoaded() const {
    return loaded_;
}

quint16 ItemManager::getMaxServerId() const {
    return maxServerId_;
}

bool ItemManager::loadDefinitions(const QString& otbPath, const QString& xmlPath) {
    clearDefinitions();
    qDebug() << "Loading item definitions from OTB:" << otbPath;

    if (!parseOtb(otbPath)) {
        qWarning() << "Failed to parse OTB file:" << otbPath;
        // Clear again in case of partial load before failure
        clearDefinitions(); 
        return false;
    }

    if (!xmlPath.isEmpty()) {
        qDebug() << "Augmenting item definitions with XML:" << xmlPath;
        if (!parseXml(xmlPath)) {
            qWarning() << "Failed to parse or augment with XML file:" << xmlPath 
                       << ". Proceeding with OTB data only.";
            // Continue even if XML fails, OTB is primary
        }
    }

    // Task 64: Load core properties from SpriteManager if available
    if (spriteManager_) {
        if (!loadCorePropertiesFromSprites()) {
            qWarning() << "Failed to load core properties from SpriteManager, continuing with OTB/XML data only";
        }
    }

    loaded_ = true;
    emit definitionsLoaded();
    qDebug() << "Item definitions loaded. Max Server ID:" << maxServerId_ << "Total items:" << itemPropertiesMap_.size();
    return true;
}

bool ItemManager::parseOtb(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open OTB file:" << filePath << file.errorString();
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // OTB File Header - First 4 bytes are 0, then root attributes start
    quint32 magic; // Should be 0
    in >> magic; 
    if (in.status() != QDataStream::Ok || magic != 0) {
         qWarning() << "Invalid OTB magic number or read error. Expected 0, got:" << magic;
         return false;
    }
    
    // Root node attributes
    quint8 nodeMarker; // Should be 0x01 for root node start in RME's items.otb
                       // Actually, RME items.otb starts with 0x00 (type byte), then 0x00000000 (flags)
                       // then 0x01 (attribute: version)
    in >> nodeMarker; // This is the type of the root node, should be 0x00 for RME
    if (in.status() != QDataStream::Ok || nodeMarker != 0x00) {
         qWarning() << "Invalid OTB root node type. Expected 0x00, got:" << nodeMarker;
         return false;
    }

    quint32 rootFlags; // These are not used by RME's OTB parser for items.otb
    in >> rootFlags;
    if(in.status() != QDataStream::Ok) { qWarning() << "Error reading root flags."; return false; }

    quint8 attr; // First attribute key
    in >> attr;
    if(in.status() != QDataStream::Ok) { qWarning() << "Error reading first root attribute key."; return false; }

    if (attr == ROOT_ATTR_OTB_VERSION) {
        quint16 dataLen;
        in >> dataLen;
        if(in.status() != QDataStream::Ok) { qWarning() << "Error reading version data length."; return false; }

        // RME's items.otb version block is 12 bytes: major, minor, build (all uint32_t)
        if (dataLen < 12) { // Minimum expected length
            qWarning() << "OTB version data length too short:" << dataLen;
            return false;
        }

        quint32 otbVersion, clientVersion, buildNumber;
        in >> otbVersion >> clientVersion >> buildNumber;
        if(in.status() != QDataStream::Ok) { qWarning() << "Error reading version numbers."; return false; }

        // Store version information
        majorVersion_ = otbVersion;
        minorVersion_ = clientVersion;
        buildNumber_ = buildNumber;

        qDebug() << "OTB Version:" << otbVersion
                 << "Client Version:" << clientVersion
                 << "Build:" << buildNumber;
        
        // Skip any remaining CSD version string etc.
        if (dataLen > 12) {
            file.read(dataLen - 12); 
        }
    } else {
        qWarning() << "Expected ROOT_ATTR_OTB_VERSION as first attribute in OTB, got:" << attr;
        return false;
    }

    // Items data
    while (!in.atEnd()) {
        ItemProperties props; // Create new properties for each item
        quint8 nodeStartByte; // Should be 0xFE for RME items.otb item node start
                              // RME's loader doesn't use these node start/end markers for items.otb
                              // It directly reads item group, then flags, then attributes.
        
        in >> nodeStartByte; // This is the item group type
        if (in.status() == QDataStream::ReadPastEnd) break; // Normal end of file
        if (in.status() != QDataStream::Ok) { qWarning() << "Error reading item group type."; return false; }

        props.group = static_cast<ItemGroup_t>(nodeStartByte);

        if (props.group == ITEM_GROUP_NONE || props.group >= ITEM_GROUP_LAST) {
            // This can happen if we try to read past the end or if the file is corrupt.
            // Or it's an actual "NONE" group, which we might skip.
            // The original code checks if it's NONE and continues.
            qWarning() << "Skipping item with group NONE or invalid group:" << nodeStartByte;
            continue; 
        }
        
        // Assign ItemTypes_t based on group (can be overridden by XML)
        switch (props.group) {
            case ITEM_GROUP_DOOR: props.type = ITEM_TYPE_DOOR; break;
            case ITEM_GROUP_CONTAINER: props.type = ITEM_TYPE_CONTAINER; break;
            case ITEM_GROUP_TELEPORT: props.type = ITEM_TYPE_TELEPORT; break;
            case ITEM_GROUP_MAGICFIELD: props.type = ITEM_TYPE_MAGICFIELD; break;
            case ITEM_GROUP_PODIUM: props.type = ITEM_TYPE_PODIUM; break;
            default: props.type = ITEM_TYPE_NONE; break;
        }

        quint32 itemFlags;
        in >> itemFlags;
        if (in.status() != QDataStream::Ok) { qWarning() << "Error reading item flags."; return false; }

        props.isBlocking = ((itemFlags & FLAG_UNPASSABLE_OTB) != 0);
        props.blockMissiles = ((itemFlags & FLAG_BLOCK_MISSILES_OTB) != 0);
        props.blockPathfind = ((itemFlags & FLAG_BLOCK_PATHFINDER_OTB) != 0);
        props.hasElevation = ((itemFlags & FLAG_HAS_ELEVATION_OTB) != 0);
        props.isUseable = ((itemFlags & FLAG_USEABLE_OTB) != 0);
        props.isPickupable = ((itemFlags & FLAG_PICKUPABLE_OTB) != 0);
        props.isMoveable = ((itemFlags & FLAG_MOVEABLE_OTB) != 0);
        props.isStackable = ((itemFlags & FLAG_STACKABLE_OTB) != 0);
        props.floorChangeDown = ((itemFlags & FLAG_FLOORCHANGEDOWN_OTB) != 0);
        props.floorChangeNorth = ((itemFlags & FLAG_FLOORCHANGENORTH_OTB) != 0);
        props.floorChangeEast = ((itemFlags & FLAG_FLOORCHANGEEAST_OTB) != 0);
        props.floorChangeSouth = ((itemFlags & FLAG_FLOORCHANGESOUTH_OTB) != 0);
        props.floorChangeWest = ((itemFlags & FLAG_FLOORCHANGEWEST_OTB) != 0);
        props.floorChange = props.floorChangeDown || props.floorChangeNorth || props.floorChangeEast || props.floorChangeSouth || props.floorChangeWest;
        props.alwaysOnBottom = ((itemFlags & FLAG_ALWAYSONTOP_OTB) != 0); // Note: OTB "ALWAYSONTOP" means RME "alwaysOnBottom"
        props.isReadable = ((itemFlags & FLAG_READABLE_OTB) != 0);
        props.isRotatable = ((itemFlags & FLAG_ROTABLE_OTB) != 0);
        props.isHangable = ((itemFlags & FLAG_HANGABLE_OTB) != 0);
        props.hasHookEast = ((itemFlags & FLAG_HOOK_EAST_OTB) != 0);
        props.hasHookSouth = ((itemFlags & FLAG_HOOK_SOUTH_OTB) != 0);
        // props.cannotDecay = ((itemFlags & FLAG_CANNOTDECAY_OTB) != 0); // Deprecated flag
        props.allowDistRead = ((itemFlags & FLAG_ALLOWDISTREAD_OTB) != 0);
        props.clientCharges = ((itemFlags & FLAG_CLIENTCHARGES_OTB) != 0);
        props.ignoreLook = ((itemFlags & FLAG_IGNORE_LOOK_OTB) != 0);
        // Other flags like CLOTHING, MARKET, DEFAULTACTION, WRAPABLE, UNWRAPABLE, TOPITEM can be added if needed

        // Read attributes
        quint8 itemAttrKey;
        while(true) {
            in >> itemAttrKey;
            if (in.status() == QDataStream::ReadPastEnd && itemAttrKey == 0xFF) break; // Graceful end of attributes for this item
            if (in.status() != QDataStream::Ok) { qWarning() << "Error reading item attribute key."; return false; }
            if (itemAttrKey == 0xFF) break; 

            quint16 dataLen;
            in >> dataLen;
            if (in.status() != QDataStream::Ok) { qWarning() << "Error reading attribute data length."; return false; }
            
            QByteArray attrData = file.read(dataLen);
            if (attrData.size() != dataLen) { qWarning() << "OTB attribute data read failed for item" << props.serverId << "attr key" << itemAttrKey; return false; }
            QDataStream attrStream(attrData);
            attrStream.setByteOrder(QDataStream::LittleEndian);

            switch (static_cast<ItemAttrib_OTB>(itemAttrKey)) {
                case ITEM_ATTR_SERVERID_OTB: attrStream >> props.serverId; break; // Server ID is the primary key
                case ITEM_ATTR_CLIENTID_OTB: attrStream >> props.clientId; break;
                // Deprecated string attributes: NAME, DESCR
                // Deprecated numeric attributes: SPEED, SLOT, MAXITEMS, WEIGHT, WEAPON, AMU, ARMOR, MAGLEVEL, MAGFIELDTYPE, WRITEABLE, ROTATETO, DECAY
                case ITEM_ATTR_SPRITEHASH_OTB: /* QByteArray hash = attrData; props.spriteHash = hash; */ break;
                case ITEM_ATTR_MINIMAPCOLOR_OTB: /* quint16 minimapColor; attrStream >> minimapColor; props.minimapColor = minimapColor; */ break;
                case ITEM_ATTR_LIGHT2_OTB: 
                    attrStream >> props.lightLevel;
                    attrStream >> props.lightColor;
                    break;
                case ITEM_ATTR_TOPORDER_OTB:
                    quint8 topOrderVal; attrStream >> topOrderVal; props.topOrder = topOrderVal;
                    break;
                case ITEM_ATTR_WAREID_OTB: /* quint16 wareId; attrStream >> wareId; props.wareId = wareId; */ break;
                case ITEM_ATTR_CLASSIFICATION_OTB: quint8 classification; attrStream >> classification; props.classification = classification; break;
                default:
                    // qDebug() << "Unknown OTB item attribute:" << itemAttrKey << "for item ID:" << props.serverId << "dataLen:" << dataLen;
                    break; 
            }
        }
        if (props.serverId > 0) {
            if (itemPropertiesMap_.contains(props.serverId)) {
                qWarning() << "Duplicate server ID in OTB:" << props.serverId << ". Overwriting.";
            }
            itemPropertiesMap_.insert(props.serverId, props);

            // Build client ID to server ID mapping
            if (props.clientId > 0) {
                clientIdToServerIds_[props.clientId].append(props.serverId);
            }

            if (props.serverId > maxServerId_) {
                maxServerId_ = props.serverId;
            }
        } else {
            // This can happen if the OTB is structured with item nodes (0xFE, 0xFD, data, 0xFC, 0xFF)
            // and the loop isn't correctly consuming those markers.
            // RME's items.otb is a flat list of: group, flags, attributes, 0xFF.
            qWarning() << "Item with server ID 0 or invalid structure encountered in OTB after reading group" << props.group << "and flags" << itemFlags;
        }
    }

    file.close();
    return true;
}

bool ItemManager::parseXml(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open XML item file:" << filePath << file.errorString();
        return false;
    }

    QXmlStreamReader xml(&file);
    QString currentItemDebugName; 

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("item")) {
                quint16 currentServerId = 0;
                bool isRange = false;
                quint16 rangeToId = 0;

                if (xml.attributes().hasAttribute("id")) {
                    currentServerId = xml.attributes().value("id").toUShort();
                } else if (xml.attributes().hasAttribute("fromid") && xml.attributes().hasAttribute("toid")) {
                    currentServerId = xml.attributes().value("fromid").toUShort();
                    rangeToId = xml.attributes().value("toid").toUShort();
                    isRange = true;
                } else {
                    continue; 
                }
                
                currentItemDebugName = xml.attributes().value("name").toString();

                for (quint16 idToProcess = currentServerId; idToProcess <= (isRange ? rangeToId : currentServerId); ++idToProcess) {
                    if (!itemPropertiesMap_.contains(idToProcess)) {
                        // This means XML has an item that OTB didn't. Create a new entry.
                        // qDebug() << "XML defines new item ID" << idToProcess << " (" << currentItemDebugName << ").";
                        itemPropertiesMap_[idToProcess].serverId = idToProcess; // Initialize with serverId
                        if (idToProcess > maxServerId_) maxServerId_ = idToProcess;
                    }
                    ItemProperties& props = itemPropertiesMap_[idToProcess];
                    
                    if(xml.attributes().hasAttribute("name")) {
                        props.name = xml.attributes().value("name").toString();
                    }
                     if(xml.attributes().hasAttribute("editorsuffix")) { // RME specific for display name
                        props.editorSuffix = xml.attributes().value("editorsuffix").toString();
                    }
                    // ClientID might also be in XML, overriding OTB or setting if new
                    if(xml.attributes().hasAttribute("clientid")) {
                        quint16 oldClientId = props.clientId;
                        props.clientId = xml.attributes().value("clientid").toUShort();

                        // Update client ID mapping if changed
                        if (oldClientId != props.clientId) {
                            if (oldClientId > 0) {
                                clientIdToServerIds_[oldClientId].removeAll(idToProcess);
                                if (clientIdToServerIds_[oldClientId].isEmpty()) {
                                    clientIdToServerIds_.remove(oldClientId);
                                }
                            }
                            if (props.clientId > 0) {
                                clientIdToServerIds_[props.clientId].append(idToProcess);
                            }
                        }
                    }


                    QXmlStreamReader::TokenType innerToken;
                    // Need to properly manage the XML stream reader for nested elements
                    // This simplified loop might not correctly parse if attributes are deeply nested.
                    // RME items.xml has attributes as direct children of <item>
                    while(!xml.atEnd()){
                        innerToken = xml.readNext();
                        if(innerToken == QXmlStreamReader::EndElement && xml.name() == QLatin1String("item")) break;
                        if(innerToken == QXmlStreamReader::StartElement && xml.name() == QLatin1String("attribute")){
                            QString key = xml.attributes().value("key").toString().toLower();
                            QString valueStr = xml.attributes().value("value").toString();
                            bool ok = true;

                            // Mapping XML attributes to ItemProperties members
                            // This is a simplified version of wxItems::loadItemFromGameXml
                            if (key == "type") {
                                if (valueStr == "depot") props.type = ITEM_TYPE_DEPOT;
                                else if (valueStr == "mailbox") props.type = ITEM_TYPE_MAILBOX;
                                else if (valueStr == "trashholder") props.type = ITEM_TYPE_TRASHHOLDER;
                                else if (valueStr == "container") props.type = ITEM_TYPE_CONTAINER;
                                else if (valueStr == "door") props.type = ITEM_TYPE_DOOR;
                                else if (valueStr == "magicfield") props.type = ITEM_TYPE_MAGICFIELD;
                                else if (valueStr == "teleport") props.type = ITEM_TYPE_TELEPORT;
                                else if (valueStr == "bed") props.type = ITEM_TYPE_BED;
                                else if (valueStr == "key") props.type = ITEM_TYPE_KEY;
                                else if (valueStr == "podium") props.type = ITEM_TYPE_PODIUM;
                            } else if (key == "group") {
                                if (valueStr == "ground") props.group = ITEM_GROUP_GROUND;
                                else if (valueStr == "container") props.group = ITEM_GROUP_CONTAINER;
                                // ... etc for ItemGroup_t
                            } else if (key == "description") {
                                props.description = valueStr;
                            } else if (key == "weight") {
                                props.weight = valueStr.toFloat(&ok) / 100.0f; if (!ok) props.weight = 0.0f;
                            } else if (key == "armor") {
                                props.armor = valueStr.toShort(&ok); if (!ok) props.armor = 0;
                            } else if (key == "defense") {
                                props.defense = valueStr.toShort(&ok); if (!ok) props.defense = 0;
                            } else if (key == "attack") {
                                props.attack = valueStr.toShort(&ok); if (!ok) props.attack = 0;
                            } else if (key == "charges" || key == "maxcharges") {
                                props.charges = valueStr.toUInt(&ok); if (!ok) props.charges = 0;
                                if (props.charges > 0) props.extraChargeable = true; // If XML defines charges, it's likely extra chargeable
                            } else if (key == "maxtextlen" || key == "maxtextlength") {
                                props.maxTextLen = valueStr.toUShort(&ok); if(!ok) props.maxTextLen = 0;
                                if (props.maxTextLen > 0 && !props.isReadable) props.isReadable = true; 
                            } else if (key == "writeable" || key == "writeonceitemid") { 
                                props.canWriteText = QVariant(valueStr).toBool(); // writeonceitemid implies writeable
                                if (props.canWriteText || valueStr.toUInt(&ok) > 0) props.isReadable = true;
                            } else if (key == "rotateto") {
                                props.rotateTo = valueStr.toUShort(&ok); if(!ok) props.rotateTo = 0;
                            } else if (key == "containersize" || key == "volume") { 
                                props.volume = valueStr.toUShort(&ok); if(!ok) props.volume = 0;
                            } else if (key == "lightlevel") {
                                props.lightLevel = valueStr.toUShort(&ok); if(!ok) props.lightLevel = 0;
                            } else if (key == "lightcolor") {
                                props.lightColor = valueStr.toUShort(&ok); if(!ok) props.lightColor = 0;
                            } else if (key == "groundequivalent") {
                                props.groundEquivalent = valueStr.toUShort(&ok); if(!ok) props.groundEquivalent = 0;
                                if (props.groundEquivalent > 0) props.hasEquivalent = true;
                            } else if (key == "bordergroup") {
                                props.borderGroup = valueStr.toUShort(&ok); if(!ok) props.borderGroup = 0;
                            }
                            // Boolean flags often set by presence of attribute or specific value "1"
                            // Example: <attribute key="blockprojectile" value="1"/>
#define PARSE_XML_BOOL_FLAG(xml_key, prop_member) \
    else if (key == QLatin1String(xml_key)) props.prop_member = QVariant(valueStr).toBool();

                            PARSE_XML_BOOL_FLAG("blockprojectile", blockMissiles)
                            PARSE_XML_BOOL_FLAG("blockpathfind", blockPathfind)
                            PARSE_XML_BOOL_FLAG("pickupable", isPickupable)
                            PARSE_XML_BOOL_FLAG("moveable", isMoveable)
                            PARSE_XML_BOOL_FLAG("stackable", isStackable)
                            PARSE_XML_BOOL_FLAG("ontop", alwaysOnBottom) // XML "ontop" usually means RME "alwaysOnBottom" (rendered first)
                                                                        // OTB ALWAYSONTOP means RME alwaysOnBottom. XML "ontop" is ambiguous.
                                                                        // For now, assume XML "ontop" means "isAlwaysOnTop" (rendered last)
                                                                        // if (key == QLatin1String("ontop")) props.isAlwaysOnTop = QVariant(valueStr).toBool();
                            PARSE_XML_BOOL_FLAG("readable", isReadable)
                            PARSE_XML_BOOL_FLAG("rotatable", isRotatable)
                            PARSE_XML_BOOL_FLAG("hangable", isHangable)
                            PARSE_XML_BOOL_FLAG("hookeast", hasHookEast)
                            PARSE_XML_BOOL_FLAG("hooksouth", hasHookSouth)
                            PARSE_XML_BOOL_FLAG("allowdistread", allowDistRead)
                            PARSE_XML_BOOL_FLAG("clientcharges", clientCharges) // This means client handles charges display
                            PARSE_XML_BOOL_FLAG("lookthrough", ignoreLook) // lookthrough means ignorelook
                            PARSE_XML_BOOL_FLAG("hasheight", hasElevation)
                            PARSE_XML_BOOL_FLAG("wallhateme", wallHateMe)
                            PARSE_XML_BOOL_FLAG("canwritetext", canWriteText)
                            PARSE_XML_BOOL_FLAG("iswall", isWall)
                            PARSE_XML_BOOL_FLAG("isborder", isBorder)
                            PARSE_XML_BOOL_FLAG("istable", isTable)
                            PARSE_XML_BOOL_FLAG("iscarpet", isCarpet)
                            PARSE_XML_BOOL_FLAG("isoptionalborder", isOptionalBorder)
                            PARSE_XML_BOOL_FLAG("isbrushDoor", isBrushDoor)
                            PARSE_XML_BOOL_FLAG("isopen", isOpen)
                            PARSE_XML_BOOL_FLAG("islocked", isLocked)
                            // Add more specific XML attribute parsings here
                        }
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML parsing error for items.xml:" << xml.errorString() << "at line" << xml.lineNumber() << "col" << xml.columnNumber();
        file.close();
        return false; // XML parsing error
    }
    file.close();
    return true;
}


const ItemProperties& ItemManager::getItemProperties(quint16 serverId) const {
    auto it = itemPropertiesMap_.constFind(serverId);
    if (it != itemPropertiesMap_.constEnd()) {
        return it.value();
    }
    // qWarning() << "Request for unknown item ID:" << serverId << "; returning default properties for" << defaultProperties_.name;
    return defaultProperties_;
}

bool ItemManager::itemTypeExists(quint16 serverId) const {
    return itemPropertiesMap_.contains(serverId);
}



// --- Enhanced Lookup Methods ---

quint16 ItemManager::getItemIdByClientID(quint16 clientId) const {
    auto it = clientIdToServerIds_.constFind(clientId);
    if (it != clientIdToServerIds_.constEnd() && !it.value().isEmpty()) {
        return it.value().first(); // Return first server ID for this client ID
    }
    return 0; // Not found
}

QList<quint16> ItemManager::getItemsByClientID(quint16 clientId) const {
    auto it = clientIdToServerIds_.constFind(clientId);
    if (it != clientIdToServerIds_.constEnd()) {
        return it.value();
    }
    return QList<quint16>(); // Empty list if not found
}

// --- Meta Item Support ---

bool ItemManager::loadMetaItem(const QString& id, const QString& name) {
    bool ok;
    quint16 serverId = id.toUShort(&ok);
    if (!ok || serverId == 0) {
        qWarning() << "Invalid meta item ID:" << id;
        return false;
    }

    if (itemPropertiesMap_.contains(serverId)) {
        qWarning() << "Meta item ID already exists:" << serverId;
        return false;
    }

    ItemProperties props;
    props.serverId = serverId;
    props.name = name.isEmpty() ? QString("Meta Item %1").arg(serverId) : name;
    props.isMetaItem = true;

    itemPropertiesMap_.insert(serverId, props);
    if (serverId > maxServerId_) {
        maxServerId_ = serverId;
    }

    qDebug() << "Loaded meta item:" << serverId << "name:" << props.name;
    return true;
}

bool ItemManager::isMetaItem(quint16 serverId) const {
    auto it = itemPropertiesMap_.constFind(serverId);
    if (it != itemPropertiesMap_.constEnd()) {
        return it.value().isMetaItem;
    }
    return false;
}

// Task 64: SpriteManager integration methods
void ItemManager::setSpriteManager(SpriteManager* spriteManager) {
    spriteManager_ = spriteManager;
    qDebug() << "ItemManager: SpriteManager integration" << (spriteManager ? "enabled" : "disabled");
}

bool ItemManager::loadCorePropertiesFromSprites() {
    if (!spriteManager_) {
        qWarning() << "ItemManager::loadCorePropertiesFromSprites - No SpriteManager available";
        return false;
    }

    int propertiesApplied = 0;
    int itemsProcessed = 0;

    // Iterate through all loaded items and apply core properties from sprites
    for (auto it = itemPropertiesMap_.begin(); it != itemPropertiesMap_.end(); ++it) {
        quint16 serverId = it.key();
        ItemProperties& props = it.value();

        if (props.clientId > 0) {
            applyCorePropertiesToItem(serverId, props.clientId);
            propertiesApplied++;
        }
        itemsProcessed++;
    }

    qDebug() << "ItemManager: Applied core properties from SpriteManager to"
             << propertiesApplied << "items out of" << itemsProcessed << "total items";

    return true;
}

void ItemManager::applyCorePropertiesToItem(quint16 serverId, quint16 clientId) {
    if (!spriteManager_ || !spriteManager_->hasCoreItemProperties(clientId)) {
        return;
    }

    const CoreItemProperties* coreProps = spriteManager_->getCoreItemProperties(clientId);
    if (!coreProps) {
        return;
    }

    auto it = itemPropertiesMap_.find(serverId);
    if (it == itemPropertiesMap_.end()) {
        return;
    }

    ItemProperties& props = it.value();

    // Apply core properties from .dat file (only if not already set by OTB/XML)
    // OTB/XML takes precedence over .dat file properties

    // Walkability - use .dat if OTB didn't specify blocking
    if (!props.isBlocking && coreProps->isNotWalkable) {
        props.isBlocking = true;
    }

    // Stackability - use .dat if not set
    if (!props.isStackable && coreProps->isStackable) {
        props.isStackable = true;
    }

    // Moveability - use .dat if not set
    if (props.isMoveable && coreProps->isNotMoveable) {
        props.isMoveable = false;
    }

    // Pickupability - use .dat if not set
    if (!props.isPickupable && coreProps->isPickupable) {
        props.isPickupable = true;
    }

    // Ground items
    if (!props.isGroundTile && coreProps->isGround) {
        props.isGroundTile = true;
        props.alwaysOnBottom = true; // Ground items are always on bottom
    }

    // Container items
    if (props.type == ITEM_TYPE_NONE && coreProps->isContainer) {
        props.type = ITEM_TYPE_CONTAINER;
        props.group = ITEM_GROUP_CONTAINER;
    }

    // Top order from .dat
    if (props.topOrder == 1 && coreProps->topOrder != 1) { // Only if not explicitly set
        props.topOrder = coreProps->topOrder;
    }

    // Light properties from .dat
    if (props.lightLevel == 0 && coreProps->lightLevel > 0) {
        props.lightLevel = coreProps->lightLevel;
        props.lightColor = coreProps->lightColor;
    }

    // Minimap color from .dat
    if (props.minimapColor == 0xFF && coreProps->minimapColor > 0) {
        props.minimapColor = static_cast<quint8>(coreProps->minimapColor);
    }

    // Projectile blocking
    if (!props.blockMissiles && coreProps->isBlockProjectile) {
        props.blockMissiles = true;
    }

    // Pathfinding blocking
    if (!props.blockPathfind && coreProps->isNotPathable) {
        props.blockPathfind = true;
    }

    // Elevation
    if (!props.hasElevation && coreProps->hasElevation) {
        props.hasElevation = true;
    }

    // Hangable and hooks
    if (!props.isHangable && coreProps->isHangable) {
        props.isHangable = true;
    }
    if (!props.hasHookEast && coreProps->hasHookEast) {
        props.hasHookEast = true;
    }
    if (!props.hasHookSouth && coreProps->hasHookSouth) {
        props.hasHookSouth = true;
    }

    // Rotatable
    if (!props.isRotatable && coreProps->isRotateable) {
        props.isRotatable = true;
    }

    // Readable/Writable
    if (!props.isReadable && coreProps->isWritable) {
        props.isReadable = true;
        props.canWriteText = true;
    }

    // Fluid container
    if (coreProps->isFluidContainer) {
        props.group = ITEM_GROUP_FLUID;
    }

    // Splash
    if (coreProps->isSplash) {
        props.group = ITEM_GROUP_SPLASH;
    }
}

// Task 013: Brush registration methods implementation
void ItemManager::registerItemBrush(quint16 serverId, Brush* brush, bool isTable, bool isCarpet)
{
    if (!brush) {
        qWarning() << "ItemManager::registerItemBrush: Cannot register null brush for item" << serverId;
        return;
    }

    auto it = itemPropertiesMap_.find(serverId);
    if (it == itemPropertiesMap_.end()) {
        qWarning() << "ItemManager::registerItemBrush: Item" << serverId << "not found in properties map";
        return;
    }

    // Update the item properties
    it->brush = brush;
    it->isTable = isTable;
    it->isCarpet = isCarpet;

    qDebug() << "ItemManager::registerItemBrush: Registered brush for item" << serverId
             << "isTable:" << isTable << "isCarpet:" << isCarpet;
}

void ItemManager::unregisterItemBrush(quint16 serverId)
{
    auto it = itemPropertiesMap_.find(serverId);
    if (it == itemPropertiesMap_.end()) {
        qWarning() << "ItemManager::unregisterItemBrush: Item" << serverId << "not found in properties map";
        return;
    }

    // Clear the brush association
    it->brush = nullptr;
    it->isTable = false;
    it->isCarpet = false;

    qDebug() << "ItemManager::unregisterItemBrush: Unregistered brush for item" << serverId;
}
