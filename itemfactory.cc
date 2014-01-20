#include <cstdlib>
#include "itemfactory.h"
#include "potion.h"
#include "gold.h"

Gold *ItemFactory::createGold(int row, int col) {
  Gold *gold;
  // Generates random value from 1-8 representing the different gold types
  // Generation takes into account their probability of spawning
  int value = rand() % 8 + 1;

  if (value <= NORM)
    gold = new Gold(row, col, Gold::NORMAL);

  else if (value <= SM)
    gold = new Gold(row, col, Gold::SMALL);

  else
    gold = new Gold(row, col, Gold::DRAGON);

  return gold;
}

Potion *ItemFactory::createPotion(int row, int col) {
  Potion *potion;
  // Generates random value from 1-6 representing the different potion type
  int type = rand() % 6 + 1;
  // how much the potion will boost/weaken of a stat
  int potionStrength = (rand() % 2) ? 10 : 5;

  switch (type){
    case (PATK):
      potion = new Potion(row, col, Potion::PATK, potionStrength);
      break;
    case (NATK):
      potion = new Potion(row, col, Potion::NATK, -potionStrength);
      break;
    case (PDEF):
      potion = new Potion(row, col, Potion::PDEF, potionStrength);
      break;
    case (NDEF):
      potion = new Potion(row, col, Potion::NDEF, -potionStrength);
      break;
    case (PHP):
      potion = new Potion(row, col, Potion::PHEALTH, potionStrength);
      break;
    case (NHP):
    default:
      potion = new Potion(row, col, Potion::NHEALTH, -potionStrength);
      break;
  }
  return potion;
}