// BrushManagerHelpers.cpp - Helper methods for BrushManager
// This file contains the remaining helper methods that couldn't fit in the main file

#include "BrushManager.h"
#include <QDebug>
#include <QMetaObject>

// Internal helper methods implementation

void BrushManager::connectBrushSignals(Brush* brush) {
    if (!brush) return;
    
    // Connect to brush property change signals if the brush supports them
    // Note: This assumes Brush class has property change signals
    // The actual signal connections will depend on the Brush implementation
    
    // Example connections (uncomment when Brush signals are implemented):
    // connect(brush, &Brush::propertiesChanged, this, &BrushManager::onBrushPropertiesChanged);
    // connect(brush, &Brush::sizeChanged, this, &BrushManager::brushSizeChanged);
    // connect(brush, &Brush::shapeChanged, this, &BrushManager::brushShapeChanged);
    
    qDebug() << "BrushManager: Connected signals for brush" << brush->name();
}

void BrushManager::disconnectBrushSignals(Brush* brush) {
    if (!brush) return;
    
    // Disconnect all signals from this brush
    disconnect(brush, nullptr, this, nullptr);
    
    qDebug() << "BrushManager: Disconnected signals for brush" << brush->name();
}

void BrushManager::initializeDefaultParameters() {
    // Initialize default parameters for each brush type
    
    // Ground brush defaults
    QVariantMap groundDefaults;
    groundDefaults["groundId"] = 100;
    groundDefaults["size"] = 1;
    groundDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Ground] = groundDefaults;
    
    // Door brush defaults
    QVariantMap doorDefaults;
    doorDefaults["doorType"] = static_cast<int>(DoorType::Normal);
    doorDefaults["size"] = 1;
    doorDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Door] = doorDefaults;
    
    // Pixel brush defaults
    QVariantMap pixelDefaults;
    pixelDefaults["color"] = QColor(Qt::black);
    pixelDefaults["size"] = 1;
    pixelDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Pixel] = pixelDefaults;
    
    // Raw brush defaults
    QVariantMap rawDefaults;
    rawDefaults["itemId"] = 0;
    rawDefaults["size"] = 1;
    rawDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Raw] = rawDefaults;
    
    // Terrain brush defaults
    QVariantMap terrainDefaults;
    terrainDefaults["terrainId"] = 0;
    terrainDefaults["size"] = 3;
    terrainDefaults["shape"] = static_cast<int>(Brush::BrushShape::Circle);
    terrainDefaults["needBorders"] = true;
    defaultBrushParameters_[Brush::Type::Terrain] = terrainDefaults;
    
    // Wall brush defaults
    QVariantMap wallDefaults;
    wallDefaults["wallId"] = 0;
    wallDefaults["size"] = 1;
    wallDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    wallDefaults["needBorders"] = true;
    defaultBrushParameters_[Brush::Type::Wall] = wallDefaults;
    
    // Creature brush defaults
    QVariantMap creatureDefaults;
    creatureDefaults["creatureId"] = 0;
    creatureDefaults["size"] = 1;
    creatureDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Creature] = creatureDefaults;
    
    // Eraser brush defaults
    QVariantMap eraserDefaults;
    eraserDefaults["size"] = 1;
    eraserDefaults["shape"] = static_cast<int>(Brush::BrushShape::Square);
    defaultBrushParameters_[Brush::Type::Eraser] = eraserDefaults;
    
    qDebug() << "BrushManager: Initialized default parameters for" << defaultBrushParameters_.size() << "brush types";
}

void BrushManager::updateCurrentBrushProperties(const QVariantMap& properties) {
    if (!currentBrush_) return;
    
    // Update brush properties using Qt's property system
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        const QString& propertyName = it.key();
        const QVariant& propertyValue = it.value();
        
        // Try to set the property on the brush object
        if (currentBrush_->metaObject()->indexOfProperty(propertyName.toUtf8().constData()) != -1) {
            currentBrush_->setProperty(propertyName.toUtf8().constData(), propertyValue);
            qDebug() << "BrushManager: Set property" << propertyName << "to" << propertyValue << "on brush" << currentBrush_->name();
        } else {
            qDebug() << "BrushManager: Property" << propertyName << "not found on brush" << currentBrush_->name();
        }
    }
    
    // Emit properties changed signal
    emit brushPropertiesChanged(currentBrush_);
}

QString BrushManager::generateBrushName(Brush::Type type, const QVariantMap& parameters) const {
    QString baseName;
    
    // Generate base name from type
    switch (type) {
        case Brush::Type::Ground:
            baseName = QString("Ground_%1").arg(parameters.value("groundId", 100).toUInt());
            break;
        case Brush::Type::Door:
            baseName = QString("Door_%1").arg(parameters.value("doorType", 0).toInt());
            break;
        case Brush::Type::Pixel:
            baseName = QString("Pixel_%1").arg(parameters.value("color", QColor(Qt::black)).value<QColor>().name());
            break;
        case Brush::Type::Raw:
            baseName = QString("Raw_%1").arg(parameters.value("itemId", 0).toUInt());
            break;
        case Brush::Type::Terrain:
            baseName = QString("Terrain_%1").arg(parameters.value("terrainId", 0).toUInt());
            break;
        case Brush::Type::Wall:
            baseName = QString("Wall_%1").arg(parameters.value("wallId", 0).toUInt());
            break;
        case Brush::Type::Creature:
            baseName = QString("Creature_%1").arg(parameters.value("creatureId", 0).toUInt());
            break;
        case Brush::Type::Eraser:
            baseName = "Eraser";
            break;
        default:
            baseName = QString("Brush_%1").arg(static_cast<int>(type));
            break;
    }
    
    // Add size and shape if specified
    if (parameters.contains("size")) {
        baseName += QString("_S%1").arg(parameters.value("size").toInt());
    }
    
    if (parameters.contains("shape")) {
        int shape = parameters.value("shape").toInt();
        baseName += QString("_%1").arg(shape == static_cast<int>(Brush::BrushShape::Circle) ? "Circle" : "Square");
    }
    
    return baseName;
}

void BrushManager::registerBrushFactories() {
    // This method can be used to register custom brush factories
    // For now, we use the built-in factory in createBrushInternal
    qDebug() << "BrushManager: Brush factories registered";
}

bool BrushManager::isValidBrushType(Brush::Type type) const {
    // Check if the brush type is valid and implemented
    switch (type) {
        // Task 53: Implemented brush types
        case Brush::Type::Ground:
        case Brush::Type::Door:
        case Brush::Type::Pixel:
        case Brush::Type::Wall:
        case Brush::Type::WallDecoration:
        case Brush::Type::Table:
        case Brush::Type::Carpet:
        case Brush::Type::Creature:
        case Brush::Type::Spawn:
        case Brush::Type::Flag:
        case Brush::Type::Eraser:
            return true; // These are implemented

        case Brush::Type::Raw:
        case Brush::Type::Doodad:
        case Brush::Type::Terrain:
        case Brush::Type::OptionalBorder:
        case Brush::Type::House:
        case Brush::Type::HouseExit:
        case Brush::Type::Waypoint:
            return false; // These are not yet implemented

        case Brush::Type::Unknown:
        default:
            return false;
    }
}

// Slot implementation
void BrushManager::onBrushPropertiesChanged() {
    Brush* senderBrush = qobject_cast<Brush*>(sender());
    if (senderBrush) {
        emit brushPropertiesChanged(senderBrush);
        qDebug() << "BrushManager: Brush properties changed for" << senderBrush->name();
    }
}
