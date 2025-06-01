#ifndef CONTAINERITEM_H
#define CONTAINERITEM_H

#include "Item.h"
#include <QList>

class ContainerItem : public Item {
    Q_OBJECT

public:
    explicit ContainerItem(quint16 serverId, QObject *parent = nullptr);
    ~ContainerItem() override;

    // Container-specific functionality
    void addItem(Item* item);
    void removeItem(Item* item);
    void removeItem(int index);
    Item* getItem(int index) const;
    QList<Item*> getItems() const;
    void clearItems();

    // Container properties
    int getItemCount() const;
    int getVolume() const;
    int getMaxVolume() const;
    double getTotalWeight() const;
    bool isFull() const;
    bool canAddItem(Item* item) const;

    // Override base class methods
    Item* deepCopy() const override;
    QString getDescription() const override;
    double getWeight() const override; // Include contents weight

    // Serialization support
    bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) override;
    bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const override;

    // Type identification
    static bool isContainerType(quint16 serverId);

signals:
    void itemAdded(Item* item);
    void itemRemoved(Item* item);
    void contentsChanged();

private:
    QList<Item*> contents_;
    void updateTotalWeight();
};

#endif // CONTAINERITEM_H
