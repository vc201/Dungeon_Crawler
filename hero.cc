#include "hero.h"
#include "object.h"
#include "consumable.h"
#include "gold.h"
#include "potion.h"
#include <string>
#include <iostream>
using namespace std;

// fills in some default values
Hero::Hero(int row, int col, int BASE_HP, int BASE_ATK, int BASE_DEF)
  : Character(row, col, '@', Object::HERO, BASE_HP, BASE_ATK, BASE_DEF, 0, "PC"),
  attackModifier(0), defenceModifier(0) {}

Hero::~Hero() {}

// no side effect for heroes (by default)
void Hero::sideEffect(Character *target) {}

int Hero::getAtk() const {
  return attackModifier + Character::getAtk();
}

int Hero::getDef() const {
  return defenceModifier + Character::getDef();
}

void Hero::resetModifiers() {
  attackModifier = 0;
  defenceModifier = 0;
}

// Checks if the Object at the given direction is a valid target.
// If so, attacks the target.
void Hero::attack(Grid::Direction dir) {
  Character *target = dynamic_cast<Character *>(grid->getObjectAt(this, dir));
  if (target == NULL) {
    grid->buffer << getName() << " attacks nothing! ";
  } else {  
    int worth = target->getGold();
    Character::attack(target);
    // if the enemy was slain
    if (grid->getObjectAt(this, dir) != target) {
      modifyGold(worth);
    }
  }
}

//**NOTE this function may look long but it's only because of switch cases**
void Hero::move(Grid::Direction dir) {
  bool moved = grid->move(this, dir);
  string direction;

  // Part of action statement
  switch (dir) {
    case Grid::NW:
      direction = "NorthWest";
      break;
    case Grid::NO:
      direction = "North";
      break;
    case Grid::NE:
      direction = "NorthEast";
      break;
    case Grid::EA:
      direction = "East";
      break;
    case Grid::SE:
      direction = "SouthEast";
      break;
    case Grid::SO:
      direction = "South";
      break;
    case Grid::SW:
      direction = "SouthWest";
      break;
    case Grid::WE:
      direction = "West";
      break;
  }

  // General move statements
  if (moved) {
    grid->buffer << getName() << " moves " << direction << " to (" << getRow() << ", " << getCol() << ")";
  } else {
    Object *obstacle = grid->getObjectAt(this, dir);
    grid->buffer << getName() << " could not move " << direction << " to (" << obstacle->getRow() << ", " << obstacle->getCol() << ")";
  }

  // For moving on gold and stairs
  Object::Type type = getOnBottom()->getType();

  if (type == Object::GOLD) {
    Gold *gold = static_cast<Gold *>(getOnBottom());
    gold->consume(this);

  } else if (type == Object::STAIRS){
    grid->nextFloor();
  }

  // For potions seen  
  vector<Object *> neighbours = grid->getNeighbours(this);
  int potions[6] = {0};

  // figures out how many and of each type of potion is in surrounding spaces
  for (unsigned int i = 0; i < neighbours.size(); i++){
    if (neighbours[i]->getType() == Object::POTION){
      Potion* potion = static_cast<Potion*> (neighbours[i]);     
      potions[potion->getPotionType()]++;
    }
  }

  int unknown = 0;

  for (int pLearned = 0; pLearned < 6; pLearned++){
    bool hasLearned = false;
    for (unsigned int k = 0; k < learned.size();k++){
      if (learned[k] == pLearned){
        hasLearned = true;
        break;
      }
    }
    
    // Will be part of action statement
    if (hasLearned){
      switch (pLearned) {
        case Potion::PATK:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a +Attack potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " +Attack potions"; 
          break;
        case Potion::NATK:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a -Attack potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " -Attack potions"; 
          break;
        case Potion::PDEF:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a +Defence potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " +Defence potions";
          break;
        case Potion::NDEF:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a -Defence potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " -Defence potions";
          break;
        case Potion::PHEALTH:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a +Health potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " +Health potions";
          break;
        case Potion::NHEALTH:
          if (potions[pLearned] == 1)
            grid->buffer << " and sees a -Health potion";
          else if (potions[pLearned] > 1)
            grid->buffer << " and sees " << potions[pLearned] << " -Health potions";
          break;
      }
    } else {
      unknown += potions[pLearned];
    }
  }

  // If type of potion never seen before
  if (unknown == 1)
    grid->buffer << " and sees an unknown potion";
  else if (unknown > 1)
    grid->buffer << " and sees " << unknown << " potions";

  grid->buffer << ". ";
}

// attempts to use a potion in the specified direction
// if successful, will destroy the potion afterwards
void Hero::usePotion(Grid::Direction dir) {
  Object *target = grid->getObjectAt(this, dir);
  if (target->getType() == Object::POTION) {
    Potion *potion = static_cast<Potion *>(target);
    potion->consume(this);
  }
  else {
    grid->buffer << "There is no potion in the specified direction. ";
  }
}

// consumes a gold pile; replaces the gold with what is under it
void Hero::consume(Gold *gold) {
  modifyGold(gold->getValue());
  grid->buffer << "and picked up " << gold->getValue() << " gold ";
  Object *onBottom = gold->getOnBottom();
  gold->setOnBottom(0);
  delete gold;
  this->setOnBottom(onBottom);
}

// consumes a potion; replaces the potion with what is under it
void Hero::consume(Potion *potion) {
  grid->buffer << getName() << " uses " << potion->getName() << ", changing " << potion->getValue() << " points! ";
  int modifier = potion->getValue();
  Potion::PotionType ptype = potion->getPotionType();
  
  // Potion type determines what stat is augmented
  switch (ptype) {
    case Potion::PATK:
    case Potion::NATK:
      attackModifier = max(0, attackModifier + modifier);
      break;
    case Potion::PDEF:
    case Potion::NDEF:
      defenceModifier = max(0, defenceModifier + modifier);
      break;
    case Potion::PHEALTH:
    case Potion::NHEALTH:
      modifyHealth(modifier);
      break;
  }
  learned.push_back(ptype);
  grid->kill(potion);
}
