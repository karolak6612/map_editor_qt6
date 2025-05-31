#include "TilePropertyEditor.h" // Should be first
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QDebug>
// No Tile.h needed here yet, only in displayTileProperties method later

TilePropertyEditor::TilePropertyEditor(QWidget *parent) : QWidget(parent) {
    setObjectName("TilePropertyEditor"); // For styling or debugging
    qDebug() << "TilePropertyEditor constructor started.";

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setContentsMargins(5, 5, 5, 5); // Add some margins
    formLayout->setSpacing(4);                 // Set spacing

    // Position
    m_positionLineEdit = new QLineEdit(this);
    m_positionLineEdit->setReadOnly(true);
    m_positionLineEdit->setObjectName("positionLineEdit");
    formLayout->addRow(new QLabel(tr("Position:")), m_positionLineEdit);

    // House ID
    m_houseIdLineEdit = new QLineEdit(this);
    m_houseIdLineEdit->setReadOnly(true);
    m_houseIdLineEdit->setObjectName("houseIdLineEdit");
    formLayout->addRow(new QLabel(tr("House ID:")), m_houseIdLineEdit);

    // Protection Zone (PZ) CheckBox
    m_pzCheckBox = new QCheckBox(tr("Is Protection Zone"), this);
    m_pzCheckBox->setEnabled(false); // Display only
    m_pzCheckBox->setObjectName("pzCheckBox");
    formLayout->addRow(m_pzCheckBox);

    // NoPVP Zone CheckBox
    m_noPvpCheckBox = new QCheckBox(tr("Is No-PVP Zone"), this);
    m_noPvpCheckBox->setEnabled(false);
    m_noPvpCheckBox->setObjectName("noPvpCheckBox");
    formLayout->addRow(m_noPvpCheckBox);

    // NoLogout Zone CheckBox
    m_noLogoutCheckBox = new QCheckBox(tr("Is No-Logout Zone"), this);
    m_noLogoutCheckBox->setEnabled(false);
    m_noLogoutCheckBox->setObjectName("noLogoutCheckBox");
    formLayout->addRow(m_noLogoutCheckBox);

    // PVP Zone CheckBox
    m_pvpZoneCheckBox = new QCheckBox(tr("Is PVP Zone"), this);
    m_pvpZoneCheckBox->setEnabled(false);
    m_pvpZoneCheckBox->setObjectName("pvpZoneCheckBox");
    formLayout->addRow(m_pvpZoneCheckBox);

    // Zone IDs
    m_zoneIdsLineEdit = new QLineEdit(this);
    m_zoneIdsLineEdit->setReadOnly(true);
    m_zoneIdsLineEdit->setObjectName("zoneIdsLineEdit");
    formLayout->addRow(new QLabel(tr("Zone IDs:")), m_zoneIdsLineEdit);

    // Item Count
    m_itemCountLineEdit = new QLineEdit(this);
    m_itemCountLineEdit->setReadOnly(true);
    m_itemCountLineEdit->setObjectName("itemCountLineEdit");
    formLayout->addRow(new QLabel(tr("Total Items (incl. ground):")), m_itemCountLineEdit);

    // Raw Map Flags
    m_mapFlagsLineEdit = new QLineEdit(this);
    m_mapFlagsLineEdit->setReadOnly(true);
    m_mapFlagsLineEdit->setObjectName("mapFlagsLineEdit");
    formLayout->addRow(new QLabel(tr("Map Flags (Hex):")), m_mapFlagsLineEdit);

    // Raw State Flags
    m_stateFlagsLineEdit = new QLineEdit(this);
    m_stateFlagsLineEdit->setReadOnly(true);
    m_stateFlagsLineEdit->setObjectName("stateFlagsLineEdit");
    formLayout->addRow(new QLabel(tr("State Flags (Hex):")), m_stateFlagsLineEdit);

    // Example of another field if needed:
    // m_tileHashLineEdit = new QLineEdit(this);
    // m_tileHashLineEdit->setReadOnly(true);
    // m_tileHashLineEdit->setObjectName("tileHashLineEdit");
    // formLayout->addRow(new QLabel(tr("Tile Hash (Debug):")), m_tileHashLineEdit);

    setLayout(formLayout);
    qDebug() << "TilePropertyEditor UI structure created with QFormLayout.";
}

TilePropertyEditor::~TilePropertyEditor() {
    // Child QObjects (QLineEdit, QCheckBox, QLabel, QFormLayout) are automatically deleted
    // when the TilePropertyEditor (their parent QWidget) is destroyed.
    qDebug() << "TilePropertyEditor destroyed.";
}

void TilePropertyEditor::displayTileProperties(Tile* tile) {
    if (!tile) {
        qDebug() << "TilePropertyEditor: displayTileProperties called with null tile. Clearing fields.";
        m_positionLineEdit->setText("");
        m_houseIdLineEdit->setText("");
        m_pzCheckBox->setChecked(false);
        m_noPvpCheckBox->setChecked(false);
        m_noLogoutCheckBox->setChecked(false);
        m_pvpZoneCheckBox->setChecked(false);
        m_zoneIdsLineEdit->setText("");
        m_itemCountLineEdit->setText("");
        m_mapFlagsLineEdit->setText("");
        m_stateFlagsLineEdit->setText("");

        // Optionally, disable all controls (though they are already read-only/non-interactive for editing)
        // setEnabled(false); // Disables the whole widget
        return;
    }

    // If previously disabled due to null tile, re-enable the widget
    // if (!isEnabled()) {
    //     setEnabled(true);
    // }

    qDebug() << "TilePropertyEditor: Displaying properties for tile at" << tile->mapPos();

    m_positionLineEdit->setText(QString("(%1, %2, %3)").arg(tile->x()).arg(tile->y()).arg(tile->z()));
    m_houseIdLineEdit->setText(tile->getHouseId() == 0 ? tr("None") : QString::number(tile->getHouseId()));

    m_pzCheckBox->setChecked(tile->isPZ());
    m_noPvpCheckBox->setChecked(tile->isNoPVP());
    m_noLogoutCheckBox->setChecked(tile->isNoLogout());
    m_pvpZoneCheckBox->setChecked(tile->isPVPZone());

    QString zoneIdsStr;
    const QVector<quint16>& zoneIds = tile->getZoneIds();
    for (int i = 0; i < zoneIds.size(); ++i) {
        zoneIdsStr += QString::number(zoneIds.at(i));
        if (i < zoneIds.size() - 1) {
            zoneIdsStr += ", ";
        }
    }
    if (zoneIdsStr.isEmpty()) {
        zoneIdsStr = tr("None");
    }
    m_zoneIdsLineEdit->setText(zoneIdsStr);

    m_itemCountLineEdit->setText(QString::number(tile->itemCount()));

    // Displaying flags as hex. Tile::TileMapFlags and Tile::TileStateFlags are QFlags<Enum>
    m_mapFlagsLineEdit->setText(QString("0x%1").arg(static_cast<quint16>(tile->getMapFlags()), 4, 16, QChar('0')));
    m_stateFlagsLineEdit->setText(QString("0x%1").arg(static_cast<quint16>(tile->getStateFlags()), 4, 16, QChar('0')));

    // Example for the new field, if it were added:
    // if (m_tileHashLineEdit) { // Check if member exists
    //     m_tileHashLineEdit->setText(QString("DEBUG_HASH_PLACEHOLDER"));
    // }
}
