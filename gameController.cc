#include "gameController.h"
#include "human.h"
#include "elf.h"
#include "dwarf.h"
#include "orc.h"
#include "hero.h"
#include "grid.h"
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

GameController::GameController(std::string mapFile/*=DEFAULT_FILE*/) {
  grid = Grid::getInstance(mapFile);
  hero = 0;
  gameStarted = false;
  race = "";
}

// Outputs map and player stats
void GameController::outputMap() {
  cout << *grid << endl;
  cout << "Race: " << race << "  Gold: " << hero->getGold() << setw(45) << right << "Floor: " << grid->getFloorNum() << endl;
  cout << "HP: " << hero->getHp() << endl;
  cout << "Atk: " << hero->getAtk() << endl;
  cout << "Def: " << hero->getDef() << endl;
  cout << "Action: ";
}

// will throw an exception if input is not defined
Grid::Direction GameController::parseDirection(std::string dir) {
  if (dir == "no")
    return Grid::NO;
  else if (dir == "so")
    return Grid::SO;
  else if (dir == "ea")
    return Grid::EA;
  else if (dir == "we")
    return Grid::WE;
  else if (dir == "ne")
    return Grid::NE;
  else if (dir == "nw")
    return Grid::NW;
  else if (dir == "se")
    return Grid::SE;
  else if (dir == "sw")
    return Grid::SW;
  else 
    throw -1;
}

// returns true on the event a win condition occurs
bool GameController::checkWin() {
  if (grid->getFloorNum() == 9) {
    return true;
  }
  return false;
}

// returns true on the event a lose condition occurs
bool GameController::checkLose() {
  if (hero->getHp() <= 0) {
    return true;
  }
  return false;
}

// selects a character and starts the game
void GameController::startGame(HeroType herotype) {
  if (herotype == HUMAN) {
    hero = new Human(0, 0);
    race = "Human";
  } else if (herotype == ORC) {
    hero = new Orc(0, 0);
    race = "Orc";
  } else if (herotype == ELF) {
    hero = new Elf(0, 0);
    race = "Elf";
  } else if (herotype == DWARF) {
    hero = new Dwarf(0, 0);
    race = "Dwarf";
  }

  grid->setHero(hero);
  grid->nextFloor();
}

// Contains interaction between player and program
bool GameController::mainProg() {
  
  // Initialize hero
  string command;
  cout << "CC3K" << endl;
  cout << "Choose a race: (h)uman, (e)lf, (d)warf, (o)rc" << endl;
  while (cin >> command) {
    if (command == "h") {
      startGame(HUMAN);
    } else if (command == "e") {
      startGame(ELF);
    } else if (command == "d") {
      startGame(DWARF);
    } else if (command == "o") {
      startGame(ORC);
    } else  {
      cout << "Please enter a valid race" << endl;
      continue;
    }
    break;
  }
  if (cin.fail()) return false;

  outputMap();
  cout << "Player Character has spawned." << endl;

  // Commands after spawning
  
  // q to quit
  while (cin >> command) {
    if (command == "q") {
      return false;
      // r to restart
    } else if (command == "r") {
      return true;
      // a to attack
    } else if (command == "a"){
      cin >> command;
      try {
        hero->attack(parseDirection(command));
        grid->enemiesAttackOrMove();
      } catch (...) {}
      // u to use potion
    } else if (command == "u"){
      cin >> command;
      try {
        hero->usePotion(parseDirection(command));
        grid->enemiesAttackOrMove();
      } catch (...) {}
    } else {
      try {
        // enemies try to attack first if the hero is trying to run
        grid->enemiesAttackAll();
        hero->move(parseDirection(command));
        grid->enemiesMoveAll();
      } catch (...) {}
    }  // end if

    // win and lose messages
    if (checkWin()) {
      cout << "You have reached floor 9 and won the game!" << endl;
      cout << "Your final score is: " << hero->getGold() << endl;
      return false;
    } else {
      outputMap();
      cout << grid->dumpBuffer() << endl;
      cout << endl;
      if (checkLose()) {
        cout << "You have lost." << endl;
        cout << "Your final score is: " << hero->getGold() << endl;
        return false;
      }
    }
  }
  return false;
}

// loops game so it can be restarted
void GameController::execute(){
  bool cont = true;
  
  while (cont){
    cont = mainProg();
  }
}
