#ifndef ITEM_H
#define ITEM_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QtGlobal> // For quint16

// Forward declarations for drawText parameters
class QPainter;
class QRectF;
// QVariantMap is typedef for QMap<QString, QVariant> - no need to forward declare if QMap is included

class Item : public QObject {
    Q_OBJECT

public:
    // Constructors & Destructor
    explicit Item(quint16 serverId, QObject *parent = nullptr);
    ~Item() override;

    // Core Properties
    quint16 getServerId() const;
    void setServerId(quint16 id); // Typically fixed after creation, but providing setter for flexibility

    quint16 getClientId() const;
    void setClientId(quint16 id);

    QString name() const;
    void setName(const QString& name);

    QString typeName() const; 
    void setTypeName(const QString& typeName);

    // Generic Attribute System
    void setAttribute(const QString& key, const QVariant& value);
    QVariant getAttribute(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool hasAttribute(const QString& key) const;
    void clearAttribute(const QString& key);
    const QMap<QString, QVariant>& getAttributes() const; 

    // Specific Attribute Accessors (examples, more can be added)
    int getCount() const;
    void setCount(int count);

    QString getText() const;
    void setText(const QString& text);

    int getActionId() const;
    void setActionId(int id);

    int getUniqueId() const;
    void setUniqueId(int id);
    
    // Add more specific attribute getters/setters as identified from wxItem
    // e.g., charges, duration, fluidType etc.

    // Boolean Flags (derived from ItemType, set by ItemManager or specific subclasses)
    // Getters
    bool isMoveable() const;
    bool isBlocking() const;      
    bool blocksMissiles() const;  
    bool blocksPathfind() const;  
    bool isStackable() const;
    bool isGroundTile() const;
    bool isAlwaysOnTop() const;   
    int getTopOrder() const;      
    bool isTeleport() const;      
    bool isContainer() const;     
    bool isReadable() const;      
    bool canWriteText() const;    
    bool isPickupable() const;
    bool isRotatable() const;
    bool isHangable() const;
    bool hasHookSouth() const;
    bool hasHookEast() const;
    bool hasHeight() const;       
    // Add more as per ItemPropertyFlag in wxItem and ItemType properties

    // Setters for these flags (mainly for ItemManager/subclass setup)
    void setMoveable(bool on);
    void setBlocking(bool on);
    void setBlocksMissiles(bool on);
    void setBlocksPathfind(bool on);
    void setStackable(bool on);
    void setGroundTile(bool on);
    void setAlwaysOnTop(bool on);
    void setTopOrder(int order);
    void setIsTeleport(bool on); 
    void setIsContainer(bool on); 
    void setReadable(bool on);
    void setCanWriteText(bool on);
    void setPickupable(bool on);
    void setRotatable(bool on);
    void setHangable(bool on);
    void setHasHookSouth(bool on);
    void setHasHookEast(bool on);
    void setHasHeight(bool on);


    // Other methods
    virtual QString getDescription() const; 
    virtual void drawText(QPainter* painter, const QRectF& targetRect, const QMap<QString, QVariant>& options); // Changed QVariantMap to QMap
    virtual Item* deepCopy() const; 

signals:
    void attributeChanged(const QString& key, const QVariant& newValue);
    void propertyChanged(); 

private:
    quint16 serverId_ = 0;
    quint16 clientId_ = 0;
    QString name_;
    QString itemTypeName_; 

    QMap<QString, QVariant> attributes_;

    // Direct member bool flags for common properties
    bool isMoveable_ = false;
    bool isBlocking_ = false;
    bool blocksMissiles_ = false;
    bool blocksPathfind_ = false;
    bool isStackable_ = false;
    bool isGroundTile_ = false;
    bool isAlwaysOnTop_ = false; 
    int topOrder_ = 1;          
    bool isTeleport_ = false;   
    bool isContainer_ = false;  
    bool isReadable_ = false;
    bool canWriteText_ = false;
    bool isPickupable_ = false;
    bool isRotatable_ = false;
    bool isHangable_ = false;
    bool hasHookSouth_ = false;
    bool hasHookEast_ = false;
    bool hasHeight_ = false;
};

#endif // ITEM_H
