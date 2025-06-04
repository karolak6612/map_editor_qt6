#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QObject>
#include <QString>
#include <QColor>
#include "MapPos.h" // Direct include for MapPos typedef

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

    // Additional attributes (extended from wxWidgets)
    int radius() const; // Waypoint influence radius
    void setRadius(int radius);

    QColor color() const; // Waypoint display color
    void setColor(const QColor& color);

    QString iconType() const; // Icon type for display
    void setIconType(const QString& iconType);

    bool isSelected() const;
    void setSelected(bool selected);

    // Utility methods
    Waypoint* deepCopy() const;
    bool isValid() const;
    QString getValidationError() const;
    quint32 memsize() const;

    // Case-insensitive name comparison (wxWidgets compatibility)
    QString normalizedName() const;
    static QString normalizeName(const QString& name);

signals:
    void waypointChanged();

private:
    QString name_;
    MapPos position_;
    QString type_;
    QString scriptOrText_;
    int radius_;
    QColor color_;
    QString iconType_;
    bool selected_;
};

#endif // WAYPOINT_H
