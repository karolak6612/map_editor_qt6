#ifndef CREATURE_H
#define CREATURE_H

#include <QObject> 
#include <QString> // For name or type later

class Creature : public QObject { 
    Q_OBJECT 
public:
    explicit Creature(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Creature() {}

    // Example methods
    // virtual QString getName() const { return "Creature"; }
    // virtual int getHealth() const { return 100; }
};

#endif // CREATURE_H
