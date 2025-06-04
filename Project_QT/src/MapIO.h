#ifndef MAPIO_H
#define MAPIO_H

#include <QObject>
#include <QString>
#include <QDataStream>

// Forward declarations
class Map;

/**
 * @brief Map I/O operations handler
 * 
 * This class handles all map loading and saving operations,
 * extracted from Map.cpp to reduce file size and improve modularity.
 * Provides 1:1 functionality migration from wxwidgets.
 */
class MapIO : public QObject
{
    Q_OBJECT

public:
    explicit MapIO(Map* map, QObject* parent = nullptr);
    ~MapIO() override;

    // Format detection and loading
    bool loadByFormat(const QString& path, const QString& format);
    bool saveByFormat(const QString& path, const QString& format) const;

    // OTBM operations
    bool loadFromOTBM(QDataStream& stream);
    bool saveToOTBM(QDataStream& stream) const;

    // XML operations  
    bool loadFromXML(const QString& path);
    bool saveToXML(const QString& path) const;

    // JSON operations
    bool loadFromJSON(const QString& path);
    bool saveToJSON(const QString& path) const;

    // Error handling
    QString getLastError() const { return lastError_; }
    void clearError() { lastError_.clear(); }

signals:
    void loadingProgress(int current, int total, const QString& operation);
    void loadingCompleted(bool success);
    void savingProgress(int current, int total, const QString& operation);
    void savingCompleted(bool success);

private:
    Map* map_;
    mutable QString lastError_;

    // Helper methods for OTBM loading
    bool loadOTBMHeader(QDataStream& stream);
    bool loadOTBMMapData(QDataStream& stream);
    bool loadOTBMTileArea(QDataStream& stream);
    bool loadOTBMSpawns(QDataStream& stream);
    bool loadOTBMHouses(QDataStream& stream);
    bool loadOTBMWaypoints(QDataStream& stream);

    // Helper methods for OTBM saving
    bool saveOTBMHeader(QDataStream& stream) const;
    bool saveOTBMMapData(QDataStream& stream) const;
    bool saveOTBMTileAreas(QDataStream& stream) const;
    bool saveOTBMSpawns(QDataStream& stream) const;
    bool saveOTBMHouses(QDataStream& stream) const;
    bool saveOTBMWaypoints(QDataStream& stream) const;

    // Helper methods for XML
    bool parseXMLMapElement(QXmlStreamReader& xml);
    bool parseXMLTileElement(QXmlStreamReader& xml);
    bool parseXMLItemElement(QXmlStreamReader& xml);
    
    bool writeXMLMapElement(QXmlStreamWriter& xml) const;
    bool writeXMLTileElements(QXmlStreamWriter& xml) const;
    bool writeXMLItemElements(QXmlStreamWriter& xml) const;

    // Helper methods for JSON
    bool parseJSONMapObject(const QJsonObject& mapObj);
    bool parseJSONTileArray(const QJsonArray& tilesArray);
    bool parseJSONItemObject(const QJsonObject& itemObj);
    
    QJsonObject createJSONMapObject() const;
    QJsonArray createJSONTileArray() const;
    QJsonObject createJSONItemObject() const;

    // Utility methods
    void setError(const QString& error) const;
    void updateProgress(int current, int total, const QString& operation) const;
};

#endif // MAPIO_H
