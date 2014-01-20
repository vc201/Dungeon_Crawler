#include "blank.h"
#include "door.h"
#include "enemy.h"
#include "enemyfactory.h"
#include "dragon.h"
#include "floor.h"
#include "grid.h"
#include "hero.h"
#include "human.h"
#include "itemfactory.h"
#include "object.h"
#include "passage.h"
#include "phoenix.h"
#include "stairs.h"
#include "wall.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

Grid *Grid::instance = 0;

Grid::Grid(string mapFile) : enemies(0), enemyfactory(new EnemyFactory), 
                             itemfactory(new ItemFactory), floorNum(0) {
  loadMap(mapFile);
}

Grid::~Grid() {
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      delete grid[row][col];
    }
  }
  delete enemyfactory;
  delete itemfactory;
}

// call to get the instance of Grid::grid
Grid *Grid::getInstance(std::string mapFile/*=DEFAULT_FILE*/) {
  if (!instance) {
    instance = new Grid(mapFile);
    atexit(cleanup);
  }
  return instance;
}

// to be called with atexit()
void Grid::cleanup() {
  delete instance;
}

// returns the current floor number
int Grid::getFloorNum() const {
  return floorNum;
}

// parses a file and stores in the grid
void Grid::loadMap(string mapFile/*=DEFAULT_FILE*/) {
  ifstream fileStream(mapFile.c_str());
  string line;
  for (int row = 0; row < MAX_ROWS; ++row) {
    getline(fileStream, line);
    for (int col = 0; col < MAX_COLS; ++col) {
      switch (line[col]) {
        case ' ':
          grid[row][col] = new Blank(row, col);
          break;
        case '.':
          grid[row][col] = new Floor(row, col);
          break;
        case '#':
          grid[row][col] = new Passage(row, col);
          break;
        case '|':
          grid[row][col] = new Wall(row, col, Wall::VERTICAL);
          break;
        case '-':
          grid[row][col] = new Wall(row, col, Wall::HORIZONTAL);
          break;
        case '+':
          grid[row][col] = new Door(row, col);
          break;
        case '\\':
          grid[row][col] = new Stairs(row, col);
          break;
        case '@':
          grid[row][col] = hero;
          break;
        case '0':
          grid[row][col] = new Potion(row, col, Potion::PHEALTH, 5);
          break;
        case '1':
          grid[row][col] = new Potion(row, col, Potion::PATK, 5);
          break;
        case '2':
          grid[row][col] = new Potion(row, col, Potion::PDEF, 5);
          break;
        case '3':
          grid[row][col] = new Potion(row, col, Potion::NHEALTH, 5);
          break;
        case '4':
          grid[row][col] = new Potion(row, col, Potion::NATK, 5);
          break;
        case '5':
          grid[row][col] = new Potion(row, col, Potion::NDEF, 5);
          break;
        case '6':
          grid[row][col] = new Gold(row, col, Gold::NORMAL);
          break;
        case '7':
          grid[row][col] = new Gold(row, col, Gold::SMALL);
          break;
        case '8':
          grid[row][col] = new Gold(row, col, Gold::DRAGON);
          break;
        default:
          cerr << line[col] << " WAS NOT FOUND." << endl;
      }  // end switch
    }  // end for
  }  // end for
}

// returns a vector of Object* denoting a full chamber
vector<Object *> determineChamber(Object *cell, bool inChamber[MAX_ROWS][MAX_COLS], map<Object::Type, bool> &nonChamber, Object *grid[MAX_ROWS][MAX_COLS]) {
  vector<Object *> chamber;
  vector<Object *> stack;
  Object *currentCell;
  int currentRow, currentCol;
  int nextRow, nextCol;

  stack.push_back(cell);
  inChamber[cell->getRow()][cell->getCol()] = true;
  while (!stack.empty()) {
    currentCell = stack.back();
    stack.pop_back();
    // add to chamber
    chamber.push_back(currentCell);
    currentRow = currentCell->getRow();
    currentCol = currentCell->getCol();
    // check neighbours and add to queue
    for (int dx = -1; dx <= 1; ++dx) {
      nextRow = currentRow + dx;
      // skip if out of bounds
      if (nextRow < 0 || nextRow > MAX_ROWS)  continue;
      for (int dy = -1; dy <= 1; ++dy) {
        nextCol = currentCol + dy;
        // skip if out of bounds
        if (nextCol < 0 || nextCol > MAX_COLS)  continue;
        // if not seen before and is part of the chamber, add to queue
        if (!inChamber[nextRow][nextCol] && !nonChamber[grid[nextRow][nextCol]->getType()]) {
          stack.push_back(grid[nextRow][nextCol]);
          inChamber[nextRow][nextCol] = true;
        }  // end if
      }  // end for (dy)
    }  // end for (dx)
  }  // end while
  return chamber;
}

// stores all chambers in a separate vector
void Grid::determineChambers() {
  // sets inChamber to {{false}}
  memset(inChamber, false, sizeof(**inChamber) * MAX_ROWS * MAX_COLS);
  map<Object::Type, bool> nonChamber;
  nonChamber[Object::BLANK] = true;
  nonChamber[Object::DOOR] = true;
  nonChamber[Object::PASSAGEWAY] = true;
  nonChamber[Object::WALL] = true;
  
  // iteratively scan through each grid coordinate
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      Object *currentCell = grid[row][col];
      if (!inChamber[row][col] && !nonChamber[currentCell->getType()]) {
        chambers.push_back(determineChamber(currentCell, inChamber, nonChamber, grid));
      }  // end if
    }  // end for
  }  // end for
}

// completely replace toReplace with replaceWith
// will also change replaceWith's row and col to match the new coordinates
void Grid::replace(Object *toReplace, Object *replaceWith) {
  if (toReplace == replaceWith) {
    return;
  }
  int row = toReplace->getRow();
  int col = toReplace->getCol();
  delete toReplace;
  grid[row][col] = replaceWith;
  replaceWith->setRow(row);
  replaceWith->setCol(col);
}

// call to destroy the given Object and replace it with the object on the bottom
// assumes the object is in a chamber
void Grid::kill(Object *obj) {
  Object *onBottom = obj->getOnBottom();
  obj->setOnBottom(0);  // set to null
  replace(obj, onBottom);
}

// returns all neighbours of a given Object in the grid
vector<Object *> Grid::getNeighbours(Object *obj) {
  vector<Object *> neighbours;
  int currentRow = obj->getRow();
  int currentCol = obj->getCol();
  for (int dx = -1; dx <= 1; ++dx) {
    int nextRow = currentRow + dx;
    // skip if out of bounds
    if (nextRow < 0 || nextRow > MAX_ROWS)  continue;
    for (int dy = -1; dy <= 1; ++dy) {
      int nextCol = currentCol + dy;
      // skip if out of bounds
      if (nextCol < 0 || nextCol > MAX_COLS)  continue;
      if (dx == dy && dy == 0)  continue;
      neighbours.push_back(grid[nextRow][nextCol]);
    }
  }
  return neighbours;
}

// returns NULL if no object can be found at the direction
Object *Grid::getObjectAt(Object *obj, Direction dir) {
  Object *objAt = 0;
  int row = obj->getRow(), col = obj->getCol();
  int dx = 0, dy = 0;
  switch (dir) {
    case Grid::NW:
      dx = -1; dy = -1;
      break;
    case Grid::NO:
      dx = 0; dy = -1;
      break;
    case Grid::NE:
      dx = 1; dy = -1;
      break;
    case Grid::EA:
      dx = 1; dy = 0;
      break;
    case Grid::SE:
      dx = 1; dy = 1;
      break;
    case Grid::SO:
      dx = 0; dy = 1;
      break;
    case Grid::SW:
      dx = -1; dy = 1;
      break;
    case Grid::WE:
      dx = -1; dy = 0;
      break;
  }
  objAt = grid[row + dy][col + dx];
  return objAt;
}

// moves (if possible) an object to the location directed by dir
// returns true if moved successfully
bool Grid::move(Object *obj, Direction dir) {
  Object *targetObj = getObjectAt(obj, dir);
  if (targetObj == NULL || targetObj->isCollidable()) {
    return false;
  }
  return move(obj, targetObj);
}

// moves (if possible) an object to the dest object
// does not necessarily have to move to a dest next to obj
bool Grid::move(Object *obj, Object *dest) {
  if (dest->isCollidable()) {
    return false;
  }
  int oldRow = obj->getRow();
  int oldCol = obj->getCol();
  int newRow = dest->getRow();
  int newCol = dest->getCol();

  // swap what the current object is on top of
  grid[oldRow][oldCol] = obj->getOnBottom();
  obj->setOnBottom(dest);
  // assign new coords
  grid[newRow][newCol] = obj;
  obj->setRow(newRow);
  obj->setCol(newCol);
  return true;
}

// returns true if the given (row, col) is in a chamber
bool Grid::isInChamber(int row, int col) {
  return inChamber[row][col];
}

// removes the tile in validSpaces given by validSpaces[chamberID][tileID]
// will also remove the entire chamber if the tile removed was the last tile
void removeValidSpace(vector<vector<Object *> > &validSpaces, int chamberID, int tileID) {
  validSpaces[chamberID].erase(validSpaces[chamberID].begin() + tileID);
  if (validSpaces[chamberID].size() == 0) {
    validSpaces.erase(validSpaces.begin() + chamberID);
  }
}

void Grid::spawnHeroAndStairs(vector<vector<Object *> > &validSpaces) {
  // generate hero first
  int heroChamber = rand() % validSpaces.size();
  int heroTile = rand() % validSpaces[heroChamber].size();
  replace(validSpaces[heroChamber][heroTile], hero);
  removeValidSpace(validSpaces, heroChamber, heroTile);
  // generate stairs
  int stairsChamber = (heroChamber + 1 + (rand() % (validSpaces.size() - 1))) % validSpaces.size();
  int stairsTile = rand() % validSpaces[stairsChamber].size();
  replace(validSpaces[stairsChamber][stairsTile], new Stairs(0, 0));
  removeValidSpace(validSpaces, stairsChamber, stairsTile);
}

void Grid::spawnPotions(vector<vector<Object *> > &validSpaces) {
  const int maxPotions = 10;
  int potionChamber, potionTile;
  Potion *toPut;

  for (int i = 0; (i < maxPotions) && (validSpaces.size() > 0); ++i) {
    toPut = itemfactory->createPotion(0, 0);
    potionChamber = rand() % validSpaces.size();
    potionTile = rand() % validSpaces[potionChamber].size();
    replace(validSpaces[potionChamber][potionTile], toPut);
    removeValidSpace(validSpaces, potionChamber, potionTile);
  }
}

// returns true if placed a dragon hoard next to the given gold successfully
bool Grid::placeDHoard(Gold *gold, int chamberID, int tileID, std::vector<std::vector<Object *> > &validSpaces) {
  Object *target = validSpaces[chamberID][tileID];
  vector<Object *> neighbours = getNeighbours(target);
  vector<Object *> validNeighbours;
  // create a vector of valid neighbours
  for (vector<Object *>::iterator i = neighbours.begin(); i != neighbours.end(); ++i) {
    Object *current = (*i);
    if (((current->getType()) == Object::FLOOR) && (current != target)) {
      validNeighbours.push_back(current);
    }
  }
  if (validNeighbours.size()) {
    // choose a random location
    int dragonLocation = rand() % validNeighbours.size();
    Dragon *dragon = new Dragon(0, 0, gold);
    enemies.push_back(dragon);
    replace(validSpaces[chamberID][tileID], gold);
    removeValidSpace(validSpaces, chamberID, tileID);
    for (vector<vector<Object *> >::size_type tile = 0; tile < validSpaces[chamberID].size(); ++tile) {
      if (validSpaces[chamberID][tile] == validNeighbours[dragonLocation]) {
        replace(validSpaces[chamberID][tile], dragon);
        removeValidSpace(validSpaces, chamberID, tile);
        return true;
      }
    }
  }
  return false;
}

// Spawns 10 gold items on the map
void Grid::spawnGold(vector<vector<Object *> > &validSpaces) {
  const int maxGold = 10;
  int goldChamber, goldTile;
  Gold *toPut;

  for (int i = 0; (i < maxGold) && (validSpaces.size() > 0); ++i) {
    toPut = itemfactory->createGold(0, 0);
    goldChamber = rand() % validSpaces.size();
    goldTile = rand() % validSpaces[goldChamber].size();
    if (toPut->getGoldType() == Gold::DRAGON) {
      while (!placeDHoard(toPut, goldChamber, goldTile, validSpaces)) {
        goldChamber = rand() % validSpaces.size();
        goldTile = rand() % validSpaces[goldChamber].size();
      }
    } else {
      replace(validSpaces[goldChamber][goldTile], toPut);
      removeValidSpace(validSpaces, goldChamber, goldTile);
    }
  }
}

// Spwans 20 enemies total on the map
void Grid::spawnEnemies(vector<vector<Object *> > &validSpaces) {
  const unsigned int maxEnemies = 20;
  int enemyChamber, enemyTile;
  Enemy *toPut;

  while ((enemies.size() < maxEnemies) && validSpaces.size()) {
    toPut = enemyfactory->createEnemy(0, 0);
    enemyChamber = rand() % validSpaces.size();
    enemyTile = rand() % validSpaces[enemyChamber].size();
    replace(validSpaces[enemyChamber][enemyTile], toPut);
    removeValidSpace(validSpaces, enemyChamber, enemyTile);
    enemies.push_back(toPut);
  }
}

// deletes everything in the map, except for the hero
void Grid::clearChambers() {
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      if (grid[row][col] == hero) {
        delete hero->getOnBottom();
        hero->setOnBottom(new Floor(0, 0));
        grid[row][col] = 0;
      } else {
        delete grid[row][col];
      }  // end if
    }  // end for (col)
  }  // end for (row)
}

// clears the grid and spawns all objects
void Grid::spawn() {
  clearChambers();
  loadMap();
  determineChambers();
  // valid spaces in each chamber
  vector<vector<Object *> > validSpaces(chambers);
  spawnHeroAndStairs(validSpaces);
  spawnPotions(validSpaces);
  spawnGold(validSpaces);
  spawnEnemies(validSpaces);
}

// go to the next floor (i.e. spawn new map elements, replace old)
void Grid::nextFloor() {
  hero->resetModifiers();
  chambers.clear();
  enemies.clear();
  spawn();
  ++floorNum;
}

// sets the hero
void Grid::setHero(Hero *hero) {
  this->hero = hero;
}

// rebuilds the enemy array
void Grid::rescanEnemies() {
  enemies.clear();
  // rescan enemies
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      Object *current = grid[row][col];
      if (current->getType() == Object::ENEMY) {
        enemies.push_back(static_cast<Enemy *>(current));
      }
    }
  }
}

// Determines whether enemies should attack or move
void Grid::enemiesAttackOrMove() {
  rescanEnemies();
  for (vector<Enemy *>::iterator i = enemies.begin(); i != enemies.end(); ++i) {
    if ((*i)->getAttacked()) {
      (*i)->setAttacked(false);
      return;
    }
    bool successAttack = (*i)->attack();
    if (successAttack && (hero->getHp() == 0)) {
      buffer << "The hero has died! ";
      return;
    } else {
      (*i)->move();
    }
  }
}

// all enemies attempt to attack
void Grid::enemiesAttackAll() {
  rescanEnemies();
  for (vector<Enemy *>::iterator i = enemies.begin(); i != enemies.end(); ++i) {
    if ((*i)->getAttacked()) {
      (*i)->setAttacked(false);
      return;
    }
    bool successAttack = (*i)->attack();
    if (successAttack && (hero->getHp() == 0)) {
      buffer << "The hero has died! ";
      return;
    }
  }
}

// all enemies attempt to move
void Grid::enemiesMoveAll() {
  rescanEnemies();
  for (vector<Enemy *>::iterator i = enemies.begin(); i != enemies.end(); ++i) {
    (*i)->move();
  }
}

// clears buffer and returns the contents it contained
string Grid::dumpBuffer() {
  string output = buffer.str();
  buffer.clear();
  buffer.str("");
  return output;
}

// outputs all elements in the grid
ostream &operator<<(ostream &out, const Grid &g) {
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      out << *(g.grid[row][col]);
    }
    out << endl;
  }
  return out;
}
