#include "object.h"

Object::Object(char repr, bool collidable, int row, int col, Type type, Object *onBottom/*=0*/) 
    : REPR(repr), collidable(collidable), row(row), col(col), type(type), onBottom(onBottom) {}

Object::~Object() {
  delete onBottom;
}

// Used to check if characters can walk through items
bool Object::isCollidable() const { return collidable; }
void Object::setCollidable(bool newState) {
  collidable = newState;
}


Object::Type Object::getType() const { return type; }

int Object::getRow() const {
  return row;
}

// also changes "onBottom"'s row (if not NULL)
void Object::setRow(int newRow) {
  row = newRow;
  if (onBottom) {
    onBottom->setRow(newRow);
  }
}

int Object::getCol() const {
  return col;
}

// also changes "onBottom"'s col (if not NULL)
void Object::setCol(int newCol) {
  col = newCol;
  if (onBottom) {
    onBottom->setCol(newCol);
  }
}

// Get what characters and items (consumables) are on top of
Object *Object::getOnBottom() const { return onBottom; }
void Object::setOnBottom(Object *newOnBottom) { onBottom = newOnBottom; }

std::ostream & operator<<(std::ostream &out, const Object &object) {
  return out << object.REPR;
}
