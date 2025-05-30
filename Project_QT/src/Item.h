#ifndef ITEM_H
#define ITEM_H

#include <QObject> 
#include <QString>
#include <QtGlobal> // For quint16

class Item : public QObject { 
    Q_OBJECT 
public:
    explicit Item(quint16 id, QObject* parent = nullptr) : QObject(parent), id_(id) {}
    virtual ~Item() {}

    quint16 getID() const { return id_; }
    
    // Placeholder methods that Tile might call based on wxwidgets analysis
    virtual bool isGroundTile() const { return false; } 
    virtual bool isAlwaysOnBottom() const { return false; } 
    virtual int getTopOrder() const { return 1; } // Example: 0=ground, 1=on-ground, 2=creature/top, 3=always-on-top
    virtual bool isBlocking() const { return true; } // Most items are blocking by default
    virtual bool isPathBlocking() const { return isBlocking(); } // Path blocking often same as physical blocking
    virtual bool isWalkBlocking() const { return isBlocking(); } // Walk blocking often same as physical blocking
    virtual bool isMissileBlocking() const { return isBlocking(); } // Missile blocking often same as physical blocking
    virtual bool isSelectable() const { return true; } // Most items are selectable
    virtual bool isUseable() const { return true; } // Most items are useable

private:
    quint16 id_;
};

#endif // ITEM_H
