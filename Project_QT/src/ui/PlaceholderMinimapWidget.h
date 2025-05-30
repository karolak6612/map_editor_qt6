#ifndef PLACEHOLDERMINIMAPWIDGET_H
#define PLACEHOLDERMINIMAPWIDGET_H

#include <QWidget>

class QLabel; // Forward declaration

class PlaceholderMinimapWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlaceholderMinimapWidget(const QString& name, QWidget *parent = nullptr);
    ~PlaceholderMinimapWidget() override;
private:
    QLabel *label_;
};

#endif // PLACEHOLDERMINIMAPWIDGET_H
