#include "PlaceholderPropertiesWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPalette> 
#include <QColor>   

PlaceholderPropertiesWidget::PlaceholderPropertiesWidget(const QString& name, QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    label_ = new QLabel(QString("%1 Placeholder").arg(name), this);
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_);
    setLayout(layout);
    
    setAutoFillBackground(true);
    QPalette pal = palette();
    if (name.compare("Palette", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(220, 220, 240)); 
    } else if (name.compare("Minimap", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(220, 240, 220)); 
    } else if (name.compare("Properties", Qt::CaseInsensitive) == 0) {
        pal.setColor(QPalette::Window, QColor(240, 220, 220)); 
    } else {
        pal.setColor(QPalette::Window, QColor(Qt::lightGray)); 
    }
    setPalette(pal);

    qDebug() << QString("Placeholder%1Widget created.").arg(name);
}

PlaceholderPropertiesWidget::~PlaceholderPropertiesWidget() {
    qDebug() << QString("Placeholder%1Widget (specifically '%2') destroyed.").arg(label_->text().split(" ").first()).arg(label_->text());
}
