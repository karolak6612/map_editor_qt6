#include "TileRenderer.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include "DrawingOptions.h"
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <algorithm>

TileRenderer::TileRenderer(QObject* parent) : QObject(parent) {
    // Constructor implementation
}

TileRenderer::~TileRenderer() {
    // Destructor implementation
}

void TileRenderer::draw(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    if (!painter || !tile) return;

    // Save painter state
    painter->save();

    // Early exit for high zoom levels - only show ground
    if (options.zoom > options.groundOnlyZoomThreshold) {
        drawGroundOnly(tile, painter, targetRect, options);
        painter->restore();
        return;
    }

    // Apply color modulation if needed
    applyColorModulation(painter, options);

    // Draw selection highlight if this tile is selected
    if (options.highlightSelectedTile && tile->isSelected()) {
        drawSelectionHighlight(tile, painter, targetRect, options);
    }

    // Draw ground layer
    if (options.showGround) {
        drawGround(tile, painter, targetRect, options);
    }

    // Skip items and creatures at high zoom levels
    if (options.hideItemsWhenZoomed && options.zoom > options.itemDisplayZoomThreshold) {
        drawZoneFlags(tile, painter, targetRect, options);
        drawDebugInfo(tile, painter, targetRect, options);
        painter->restore();
        return;
    }

    // Draw items with proper Z-ordering
    if (options.showItems) {
        drawItems(tile, painter, targetRect, options);
    }

    // Draw creatures
    if (options.showCreatures) {
        drawCreatures(tile, painter, targetRect, options);
    }

    // Draw spawn indicators
    if (options.showSpawns && tile->spawn()) {
        drawSpawn(tile, painter, targetRect, options);
    }

    // Draw zone flags and effects
    drawZoneFlags(tile, painter, targetRect, options);
    
    // Draw debug information
    drawDebugInfo(tile, painter, targetRect, options);

    // Restore painter state
    painter->restore();
}

void TileRenderer::drawGroundOnly(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    if (options.showGround) {
        drawGround(tile, painter, targetRect, options);
    }
    
    // Still show zone flags at high zoom
    if (options.alwaysShowZones) {
        drawZoneFlags(tile, painter, targetRect, options);
    }
}

void TileRenderer::drawSelectionHighlight(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    Q_UNUSED(tile)
    Q_UNUSED(options)
    
    painter->save();
    
    QColor selectionColor = Qt::yellow;
    selectionColor.setAlpha(80);
    painter->fillRect(targetRect, selectionColor);
    
    QPen pen(Qt::yellow, 2);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    painter->drawRect(targetRect);
    
    painter->restore();
}

void TileRenderer::drawGround(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    Item* ground = tile->getGround();
    if (ground) {
        DrawingOptions groundOptions = options;
        
        // Apply transparency for higher/lower floors
        if (tile->z() > options.currentFloor && options.showHigherFloorsTransparent) {
            groundOptions.itemOpacity *= 0.5f;
        } else if (tile->z() < options.currentFloor && options.showLowerFloorsTransparent) {
            groundOptions.itemOpacity *= 0.3f;
        }
        
        ground->draw(painter, targetRect, groundOptions);
    } else {
        // Enhanced fallback: draw a comprehensive ground placeholder
        drawGroundPlaceholder(tile, painter, targetRect, options);
    }
}

void TileRenderer::drawItems(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    const QVector<Item*>& items = tile->items();
    if (items.isEmpty()) return;

    // Sort items by Z-order for proper rendering
    QList<Item*> sortedItems = items.toList();
    sortItemsByZOrder(sortedItems);

    for (Item* item : sortedItems) {
        if (!item) continue;

        DrawingOptions itemOptions = options;

        // Apply floor transparency
        if (tile->z() > options.currentFloor && options.showHigherFloorsTransparent) {
            itemOptions.itemOpacity *= 0.5f;
        } else if (tile->z() < options.currentFloor && options.showLowerFloorsTransparent) {
            itemOptions.itemOpacity *= 0.3f;
        }

        // Apply transparent items option
        if (options.transparentItems) {
            itemOptions.itemOpacity *= 0.7f;
        }

        // Skip invisible items unless in GM mode
        if (!options.showInvisibleItems && !item->isVisible()) {
            continue;
        }

        // Highlight locked doors if enabled
        if (options.highlightLockedDoors && item->isDoor() && item->isLocked()) {
            painter->save();
            painter->fillRect(targetRect, QColor(255, 0, 0, 50)); // Red highlight
            painter->restore();
        }

        item->draw(painter, targetRect, itemOptions);
    }
}

void TileRenderer::drawCreatures(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    Creature* creature = tile->creature();
    if (!creature) return;

    DrawingOptions creatureOptions = options;

    // Apply floor transparency
    if (tile->z() > options.currentFloor && options.showHigherFloorsTransparent) {
        creatureOptions.creatureOpacity *= 0.5f;
    } else if (tile->z() < options.currentFloor && options.showLowerFloorsTransparent) {
        creatureOptions.creatureOpacity *= 0.3f;
    }

    creature->draw(painter, targetRect, creatureOptions);
}

void TileRenderer::drawSpawn(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    Spawn* spawn = tile->spawn();
    if (!spawn) return;

    painter->save();

    // Different spawn visualization based on zoom level
    if (options.zoom < 2.0f) {
        // Detailed spawn visualization at low zoom
        painter->setBrush(QColor(128, 0, 128, 150));
        painter->setPen(QPen(QColor(255, 0, 255), 1));
        painter->drawEllipse(targetRect.center(), 8, 8);

        // Draw spawn radius if available
        if (spawn->getRadius() > 0) {
            painter->setPen(QPen(QColor(128, 0, 128, 100), 1, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            float radius = spawn->getRadius() * targetRect.width();
            painter->drawEllipse(targetRect.center(), radius, radius);
        }
    } else {
        // Simple spawn indicator at high zoom
        painter->setBrush(QColor(128, 0, 128, 100));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(targetRect.topLeft() + QPointF(2, 2), 4, 4);
    }

    painter->restore();
}

void TileRenderer::drawZoneFlags(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    if (!options.showTileFlags && !options.alwaysShowZones) return;

    QString flagsText;
    QColor flagColor = Qt::white;

    // Collect zone flags
    if (tile->hasMapFlag(Tile::TileMapFlag::ProtectionZone)) {
        flagsText += "PZ ";
        flagColor = QColor(0, 255, 0); // Green for PZ
    }
    if (tile->hasMapFlag(Tile::TileMapFlag::NoPVP)) {
        flagsText += "NoPvP ";
        flagColor = QColor(0, 0, 255); // Blue for NoPvP
    }
    if (tile->hasMapFlag(Tile::TileMapFlag::PVPZone)) {
        flagsText += "PvP ";
        flagColor = QColor(255, 0, 0); // Red for PvP
    }
    if (tile->hasMapFlag(Tile::TileMapFlag::NoLogout)) {
        flagsText += "NoLogout ";
    }

    if (!flagsText.isEmpty()) {
        painter->save();

        // Adjust font size based on zoom
        QFont font = painter->font();
        int fontSize = qMax(6, static_cast<int>(8 / options.zoom));
        font.setPointSize(fontSize);
        painter->setFont(font);

        // Use zone-specific colors or white
        painter->setPen(flagColor);

        // Position text based on zoom level
        Qt::Alignment alignment = (options.zoom > 2.0f) ?
            (Qt::AlignCenter) : (Qt::AlignBottom | Qt::AlignHCenter);

        painter->drawText(targetRect, alignment, flagsText.trimmed());
        painter->restore();
    }
}

void TileRenderer::drawDebugInfo(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    if (!options.drawDebugInfo) return;

    painter->save();

    QFont font = painter->font();
    font.setPointSize(7);
    painter->setFont(font);
    painter->setPen(Qt::cyan);

    QString coordText = QString("%1,%2,%3").arg(tile->x()).arg(tile->y()).arg(tile->z());

    // Add additional debug info
    if (options.zoom < 3.0f) {
        QStringList debugInfo;
        debugInfo << coordText;

        if (!tile->items().isEmpty()) {
            debugInfo << QString("I:%1").arg(tile->items().size());
        }
        if (tile->creature()) {
            debugInfo << "C";
        }
        if (tile->spawn()) {
            debugInfo << "S";
        }
        if (tile->getGround()) {
            debugInfo << QString("G:%1").arg(tile->getGround()->getItemId());
        }

        coordText = debugInfo.join(" ");
    }

    painter->drawText(targetRect.adjusted(2, 2, 0, 0),
                     Qt::AlignTop | Qt::AlignLeft | Qt::TextDontClip, coordText);

    painter->restore();
}

void TileRenderer::drawGroundPlaceholder(const Tile* tile, QPainter* painter, const QRectF& targetRect, const DrawingOptions& options) {
    if (!painter || !tile) return;

    painter->save();

    // Determine placeholder color based on tile properties
    QColor groundColor;
    if (options.showAsMinimapColors) {
        groundColor = QColor(100, 100, 100); // Gray for minimap
    } else {
        // Use different colors based on tile flags for visual distinction
        if (tile->isPz()) {
            groundColor = QColor(0, 255, 0, 80); // Green for PZ
        } else if (tile->isNoPvp()) {
            groundColor = QColor(0, 0, 255, 80); // Blue for No PVP
        } else if (tile->isNoLogout()) {
            groundColor = QColor(255, 255, 0, 80); // Yellow for No Logout
        } else if (tile->hasProperty(TILESTATE_PROTECTIONZONE)) {
            groundColor = QColor(0, 255, 255, 80); // Cyan for protection
        } else {
            groundColor = QColor(50, 50, 50, 100); // Default dark gray
        }
    }

    painter->fillRect(targetRect, groundColor);

    // Draw a subtle grid pattern for empty ground
    if (!options.showAsMinimapColors && options.zoom >= 1.0f) {
        QPen gridPen(QColor(255, 255, 255, 30), 1, Qt::DotLine);
        painter->setPen(gridPen);

        // Draw grid lines
        painter->drawLine(targetRect.topLeft(), targetRect.topRight());
        painter->drawLine(targetRect.topLeft(), targetRect.bottomLeft());
        painter->drawLine(targetRect.topRight(), targetRect.bottomRight());
        painter->drawLine(targetRect.bottomLeft(), targetRect.bottomRight());

        // Draw diagonal for visual interest
        painter->drawLine(targetRect.topLeft(), targetRect.bottomRight());
    }

    // Draw coordinate text for debugging if enabled and there's space
    if (options.drawDebugInfo && targetRect.width() > 30 && targetRect.height() > 20) {
        QFont font = painter->font();
        font.setPointSize(qMax(6, font.pointSize() - 3));
        painter->setFont(font);
        painter->setPen(Qt::white);

        QString coordText = QString("(%1,%2,%3)").arg(tile->x()).arg(tile->y()).arg(tile->z());
        QRectF textRect = targetRect.adjusted(2, 2, -2, -2);
        painter->drawText(textRect, Qt::AlignCenter, coordText);
    }

    painter->restore();
}

void TileRenderer::sortItemsByZOrder(QList<Item*>& items) {
    std::sort(items.begin(), items.end(), [](const Item* a, const Item* b) {
        // Items with higher Z-order (stackpos) should be drawn last (on top)
        return a->getStackPos() < b->getStackPos();
    });
}

void TileRenderer::applyColorModulation(QPainter* painter, const DrawingOptions& options) {
    if (options.redModulation != 255 || options.greenModulation != 255 ||
        options.blueModulation != 255 || options.alphaModulation != 255) {
        painter->setCompositionMode(QPainter::CompositionMode_Multiply);
        painter->setOpacity(options.alphaModulation / 255.0f);
    }
}

#include "TileRenderer.moc"
