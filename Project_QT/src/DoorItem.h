#ifndef DOORITEM_H
#define DOORITEM_H

#include "Item.h"

class DoorItem : public Item {
    Q_OBJECT

public:
    explicit DoorItem(quint16 serverId, QObject *parent = nullptr);
    ~DoorItem() override;

    // Door-specific functionality
    bool isOpen() const;
    void setOpen(bool open);
    
    bool isLocked() const;
    void setLocked(bool locked);
    
    quint16 getDoorId() const;
    void setDoorId(quint16 doorId);
    
    quint16 getOpenDoorId() const;
    void setOpenDoorId(quint16 openDoorId);
    
    quint16 getClosedDoorId() const;
    void setClosedDoorId(quint16 closedDoorId);

    // Door state management
    void toggle();
    quint16 getAlternateDoorId() const;

    // Override base class methods
    Item* deepCopy() const override;
    QString getDescription() const override;

    // Serialization support
    bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) override;
    bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const override;

    // Type identification
    static bool isDoorType(quint16 serverId);

signals:
    void doorStateChanged(bool isOpen);

private:
    bool isOpen_;
    bool isLocked_;
    quint16 doorId_;
    quint16 openDoorId_;
    quint16 closedDoorId_;
};

#endif // DOORITEM_H
