// contains declarations of functions 
#ifndef SIMULATION_H
#define SIMULATION_H

#include "world_type.h"

std::string readSpeciesFile(const std::string &speciesFile, world_t &world);
// REQUIRES: speciesFile is a valid file 
// MODIFIES: world 
// EFFECTS: inputs how many species and what they are called in world array

bool readProgram(const std::string &programFile, species_t &species);
// REQUIRES: programFile is a valid file
// MODIFIES: species
// EFFECTS: inputs instructions into species and sets size of species program

bool readCreatures(const std::string &creaturesFile, world_t &world);
// REQUIRES: creaturesfile is valid
// MODIFIES: world
// EFFECTS: reads world file

bool initWorld(world_t &world, const std::string &speciesFile, 
const std::string &creaturesFile); 
// MODIFIES: world 
// 
// EFFECTS: Initialize "world" given the species summary file 
//          "speciesFile" and the world description file 
//          "creaturesFile". This initializes all the components of 
//          "world". Returns true if initialization is successful. 

void simulateCreature(creature_t &creature, grid_t &grid, bool 
verbose); 
// REQUIRES: creature is inside the grid. 
// 
// MODIFIES: creature, grid, cout. 
// 
// EFFECTS: Simulate one turn of "creature" and update the creature, 
//          the infected creature, and the grid if necessary. 
//          The creature programID is always updated. The function 
//          also prints to the stdout the procedure. If verbose is 
//          true, it prints more information. 

void printGrid(const grid_t &grid); 
// MODIFIES: cout. 
// 
// EFFECTS: print a grid representation of the creature world. 

point_t adjacentPoint(point_t pt, direction_t dir); 
// EFFECTS: Returns a point that results from moving one square 
//          in the direction "dir" from the point "pt". 

direction_t leftFrom(direction_t dir); 
// EFFECTS: Returns the direction that results from turning 
//          left from the given direction "dir". 

direction_t rightFrom(direction_t dir); 
// EFFECTS: Returns the direction that results from turning 
//          right from the given direction "dir". 

instruction_t getInstruction(const creature_t &creature); 
// EFFECTS: Returns the current instruction of "creature". 

creature_t *getCreature(const grid_t &grid, point_t location); 
// REQUIRES: location is inside the grid. 
// 
// EFFECTS: Returns a pointer to the creature at "location" in "grid".

bool isInside(point_t pt, const grid_t& grid);
// EFFECTS: checks if a point is inside the grid

#endif