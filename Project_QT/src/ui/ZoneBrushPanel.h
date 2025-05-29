#ifndef ZONEBRUSHPANEL_H
#define ZONEBRUSHPANEL_H

#include <QWidget>

// Forward declarations
class QPushButton;
class QSpinBox;
class QHBoxLayout;

class ZoneBrushPanel : public QWidget {
    Q_OBJECT

public:
    explicit ZoneBrushPanel(QWidget *parent = nullptr);
    ~ZoneBrushPanel() override;

private slots:
    void onZoneButtonToggled(bool checked);
    void onZoneIdChanged(int value);

private:
    void setupUi();

    QPushButton* m_zoneButton;
    QSpinBox* m_zoneIdSpin;
};

#endif // ZONEBRUSHPANEL_H
