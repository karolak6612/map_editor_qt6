#ifndef DEPOTITEM_H
#define DEPOTITEM_H

#include "ContainerItem.h"

class DepotItem : public ContainerItem {
    Q_OBJECT

public:
    explicit DepotItem(quint16 serverId, QObject *parent = nullptr);
    ~DepotItem() override;

    // Depot-specific functionality
    quint32 getDepotId() const;
    void setDepotId(quint32 depotId);

    // Override base class methods
    Item* deepCopy() const override;
    QString getDescription() const override;

    // Serialization support
    bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) override;
    bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const override;

    // Type identification
    static bool isDepotType(quint16 serverId);

private:
    quint32 depotId_;
};

#endif // DEPOTITEM_H
