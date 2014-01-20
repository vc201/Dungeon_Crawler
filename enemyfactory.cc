#include <cstdlib>
#include "enemyfactory.h"
#include "werewolf.h"
#include "vampire.h"
#include "goblin.h"
#include "troll.h"
#include "phoenix.h"
#include "merchant.h"

Enemy* EnemyFactory::createEnemy(int row, int col){
  Enemy * enemy;
  
  // Generates random value from 1-18 representing the different enemies
  // Generation takes into account their probability of spawning
  int monster = rand() % 18 + 1;
  
  if (monster <= WEREWOLF)
    enemy = new Werewolf(row, col);

  else if (monster <= VAMPIRE)
    enemy = new Vampire(row, col);
    
  else if (monster <= GOBLIN)
    enemy = new Goblin(row, col);
    
  else if (monster <= TROLL)
    enemy = new Troll(row, col);
    
  else if (monster <= PHOENIX)
    enemy = new Phoenix(row, col);
    
  else
    enemy = new Merchant(row, col);
    
  return enemy;
}
