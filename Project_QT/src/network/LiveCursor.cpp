#include "LiveCursor.h"
#include "../Position.h"

LiveCursor::LiveCursor() : id(0), color(Qt::white), pos() {
}

LiveCursor::LiveCursor(quint32 id_, const QColor& color_, const Position& pos_) 
    : id(id_), color(color_), pos(pos_) {
}

bool LiveCursor::operator==(const LiveCursor& other) const {
    return id == other.id && color == other.color && pos == other.pos;
}

bool LiveCursor::operator!=(const LiveCursor& other) const {
    return !(*this == other);
}
