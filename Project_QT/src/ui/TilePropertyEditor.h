#ifndef TILEPROPERTYEDITOR_H
#define TILEPROPERTYEDITOR_H

#include <QWidget>

// Forward declarations
class QLabel;
class QLineEdit;
class QCheckBox;
class QFormLayout;
class Tile; // For future use with displayTileProperties

class TilePropertyEditor : public QWidget {
    Q_OBJECT
public:
    explicit TilePropertyEditor(QWidget *parent = nullptr);
    ~TilePropertyEditor() override;
    void displayTileProperties(Tile* tile);

private:
    QLineEdit* m_positionLineEdit;
    QLineEdit* m_houseIdLineEdit;
    QCheckBox* m_pzCheckBox;
    QCheckBox* m_noPvpCheckBox;
    QCheckBox* m_noLogoutCheckBox;
    QCheckBox* m_pvpZoneCheckBox;
    QLineEdit* m_zoneIdsLineEdit;
    QLineEdit* m_itemCountLineEdit;
    QLineEdit* m_mapFlagsLineEdit;
    QLineEdit* m_stateFlagsLineEdit;
    // QLineEdit* m_tileHashLineEdit; // Example for a new field
};

#endif // TILEPROPERTYEDITOR_H
