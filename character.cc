#include "character.h"
#include <cmath>
#include <string>
using namespace std;

// initializes onBottom to a new Floor
Character::Character(int row, int col, char REPR, Object::Type type, int BASE_HP, int BASE_ATK, int BASE_DEF, int gold, string name) 
    : Object(REPR, true, row, col, type, new Floor(row, col)), BASE_HP(BASE_HP), BASE_ATK(BASE_ATK), BASE_DEF(BASE_DEF), gold(gold), name(name) {
  hp = BASE_HP;
  atk = BASE_ATK;
  def = BASE_DEF;
  grid = Grid::getInstance();
}

Character::~Character() {}

// deals damage to an enemy
void Character::attack(Character *target) {
  // cast from double to int
  int damage = static_cast<int>(ceil(100.0 / (100 + target->getDef()) * getAtk()));
  grid->buffer << name << " deals " << damage << " damage to " << target->getName() << " (" << max(0, target->getHp() - damage) << " HP). ";
  int targetHP = target->getHp();
  target->modifyHealth(-damage);
  if (targetHP > 0) {
    sideEffect(target);
  }
}

int Character::getHp() const { return hp; }
int Character::getAtk() const { return atk; }
int Character::getDef() const { return def; }
int Character::getGold() const { return gold; }
string Character::getName() const { return name; }

void Character::modifyHealth(int delta) {
  hp = min(max(hp + delta, 0), BASE_HP);
}

void Character::modifyGold(int delta) {
  gold = max(gold + delta, 0);
}
