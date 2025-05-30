#ifndef PLACEHOLDERPROPERTIESWIDGET_H
#define PLACEHOLDERPROPERTIESWIDGET_H

#include <QWidget>

class QLabel; // Forward declaration

class PlaceholderPropertiesWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlaceholderPropertiesWidget(const QString& name, QWidget *parent = nullptr);
    ~PlaceholderPropertiesWidget() override;
private:
    QLabel *label_;
};

#endif // PLACEHOLDERPROPERTIESWIDGET_H
