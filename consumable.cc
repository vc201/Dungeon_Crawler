#include "consumable.h"
#include "floor.h"
using namespace std;

Consumable::Consumable(int row, int col, char REPR, bool collidable, Object::Type type, int value, string name) 
    : Object(REPR, collidable, row, col, type, new Floor(row, col)), name(name), value(value) {}

Consumable::~Consumable() {}

string Consumable::getName() const {
  return name;
}

void Consumable::setName(string newName) {
  name = newName;
}

int Consumable::getValue() {
  return value;
}