#include "enemy.h"
#include "grid.h"
#include <cstdlib>
#include <vector>
using namespace std;

Enemy::Enemy(int row, int col, char REPR, int BASE_HP, int BASE_ATK, int BASE_DEF, int gold, string name) 
    : Character(row, col, REPR, Object::ENEMY, BASE_HP, BASE_ATK, BASE_DEF, gold, name), attacked(false) {}

Enemy::~Enemy() {}

// will not move if hero is near, will not move out of grid
void Enemy::move() {
  // do not move if already attacked
  if (attacked) {
    attacked = false;
    return;
  }
  
  // Finds valid empty spaces for enemy to move to
  vector<Object *> neighbours = grid->getNeighbours(this);
  vector<Object *> validNeighbours;
  Object::Type neighbourType;
  int row, col;
  for (vector<Object *>::iterator i = neighbours.begin(); i != neighbours.end(); ++i) {
    row = (*i)->getRow();
    col = (*i)->getCol();
    neighbourType = (*i)->getType();
    if ((neighbourType == Object::FLOOR) && (grid->isInChamber(row, col))) {
      validNeighbours.push_back(*i);
    }
  }
  // select random spot
  if (validNeighbours.size() > 0) {
    Object *destination = validNeighbours[rand() % validNeighbours.size()];
    grid->move(this, destination);
  }
}

// locates and attacks the hero, if is within range
// returns true if attempted an attack
bool Enemy::attack() {
  vector<Object *> targets = grid->getNeighbours(this);
  for (vector<Object *>::iterator i = targets.begin(); i != targets.end(); ++i) {
    if ((*i)->getType() == Object::HERO) {
      if ((rand() % 2) == 0) {
        grid->buffer << getName() << " tries attacking, but misses! ";
      } else {
        Character::attack(static_cast<Character *>(*i));
      }  // end if
      attacked = true;
      return true;
    }  // end if
  }  // end for
  attacked = false;
  return false;
}

void Enemy::modifyHealth(int delta) {
  Character::modifyHealth(delta);
  if (getHp() <= 0) {
    grid->buffer << getName() << " died! ";
    grid->kill(this);
  }
}

bool Enemy::getAttacked() const {
  return attacked;
}

void Enemy::setAttacked(bool attacked) {
  this->attacked = attacked;
}
