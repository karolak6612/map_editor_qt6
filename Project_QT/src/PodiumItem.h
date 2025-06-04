#ifndef PODIUMITEM_H
#define PODIUMITEM_H

#include "Item.h"
#include "Outfit.h" // For outfit display

class PodiumItem : public Item {
    Q_OBJECT

public:
    explicit PodiumItem(quint16 serverId, QObject *parent = nullptr);
    ~PodiumItem() override;

    // Podium-specific functionality
    bool hasOutfit() const;
    Outfit getOutfit() const;
    void setOutfit(const Outfit& outfit);
    void clearOutfit();

    // Display properties
    bool isVisible() const;
    void setVisible(bool visible);

    // Override base class methods
    Item* deepCopy() const override;
    QString getDescription() const override;

    // Serialization support
    bool unserializeOtbmAttributes(QDataStream& stream, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) override;
    bool serializeOtbmAttributes(QDataStream& stream, quint32 mapOtbmFormatVersion, quint32 otbItemsMajorVersion, quint32 otbItemsMinorVersion) const override;

    // Type identification
    static bool isPodiumType(quint16 serverId);

signals:
    void outfitChanged(const Outfit& outfit);

private:
    Outfit outfit_;
    bool hasOutfit_;
    bool isVisible_;

    // Podium outfit serialization helpers (Task 24, Task 55)
    QByteArray serializePodiumOutfit() const;
    bool deserializePodiumOutfit(const QByteArray& data);
};

#endif // PODIUMITEM_H
