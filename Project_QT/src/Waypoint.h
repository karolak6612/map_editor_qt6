#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QObject>
#include <QString>
#include "Map.h" // For MapPos

class Waypoint : public QObject {
    Q_OBJECT

public:
    explicit Waypoint(QObject* parent = nullptr);
    Waypoint(const QString& name, const MapPos& position, const QString& type = QString(), const QString& scriptOrText = QString(), QObject* parent = nullptr);
    ~Waypoint() override;

    QString name() const;
    void setName(const QString& name);

    MapPos position() const;
    void setPosition(const MapPos& position);

    QString type() const; // E.g., "generic", "npc_path", "quest_marker"
    void setType(const QString& type);

    QString scriptOrText() const; // Could be a script snippet, a note, or dialogue line
    void setScriptOrText(const QString& scriptOrText);

signals:
    void waypointChanged();

private:
    QString name_;
    MapPos position_;
    QString type_;
    QString scriptOrText_;
};

#endif // WAYPOINT_H
