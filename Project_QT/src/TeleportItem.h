#ifndef TELEPORTITEM_H
#define TELEPORTITEM_H

#include "Item.h"
#include "Map.h" // For MapPos

class TeleportItem : public Item {
    Q_OBJECT

public:
    explicit TeleportItem(quint16 serverId, QObject *parent = nullptr);
    ~TeleportItem() override;

    // Teleport-specific functionality
    MapPos getDestination() const;
    void setDestination(const MapPos& destination);
    void setDestination(int x, int y, int z);

    // Convenience getters
    int getDestX() const;
    int getDestY() const;
    int getDestZ() const;

    // Override base class methods
    Item* deepCopy() const override;
    QString getDescription() const override;

    // Serialization support
    bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) override;
    bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const override;

    // Type identification
    static bool isTeleportType(quint16 serverId);

private:
    MapPos destination_;
};

#endif // TELEPORTITEM_H
