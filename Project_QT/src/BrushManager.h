#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "Brush.h" // Assuming Brush.h is in the same directory

// Forward declaration can be used if Brush.h includes BrushManager.h to avoid circular dependency
// class Brush;

class BrushManager : public QObject {
    Q_OBJECT

public:
    explicit BrushManager(QObject *parent = nullptr);
    ~BrushManager();

    void addBrush(Brush* brush);
    void removeBrush(const QString& name);
    Brush* getBrush(const QString& name) const;
    QMap<QString, Brush*> getBrushes() const; // Returns a copy of the map

    void setCurrentBrush(const QString& name);
    void setCurrentBrush(Brush* brush); // Overload
    Brush* getCurrentBrush() const;

signals:
    void currentBrushChanged(Brush* newBrush);

private:
    QMap<QString, Brush*> brushes_;
    Brush* currentBrush_ = nullptr;
};

#endif // BRUSHMANAGER_H
