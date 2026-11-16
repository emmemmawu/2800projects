// contains implementation of simulation.h functions
#include "simulation.h"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::string readSpeciesFile(const std::string& speciesFile, world_t& world) {
  // REQUIRES: speciesFile is a valid file
  // MODIFIES: world
  // EFFECTS: inputs how many species and what they are called in world array
  std::ifstream inputFile(speciesFile);
  if (!inputFile) {
    std::cout << "Error: Cannot open file" << speciesFile << "!\n";
    return "";
  }
  // read the directory name
  std::string directory;
  std::getline(inputFile, directory);
  if (directory.empty()) {
    std::cout << "Error: Species file is empty.\n";
    return "";
  }
  // reads remaining species
  std::string name;
  unsigned int count = 0;
  while (std::getline(inputFile, name)) {
    if (count >= MAXSPECIES) {
      std::cout << "Error: Too many species!\n";
      std::cout << "Maximal number of species is " << MAXSPECIES << ".\n";
      return "";
    }
    world.species[count].name = name;
    count++;
  }
  world.numSpecies = count;
  return directory;
}

bool readProgram(const std::string& programFile, species_t& species) {
  // REQUIRES: programFile is a valid file
  // MODIFIES: species
  // EFFECTS: inputs instructions into species and sets size of species program
  std::ifstream inputFile(programFile);
  if (!inputFile) {
    std::cout << "Error: Cannot open file" << programFile << "!\n";
    return false;
  }

  std::string line;
  species.programSize = 0;
  while (std::getline(inputFile, line)) {
    if (line.empty()) {
      break;
    }
    if (species.programSize >= MAXPROGRAM) {
      std::cout << "Error: Too many instructions for species " << species.name
                << "!\n";
      std::cout << "Maximal number of instructions is " << MAXPROGRAM << ".\n";
      return false;
    }
    // get the instruction and address separately
    std::istringstream iss(line);
    std::string instruction;
    unsigned int address = 0;
    iss >> instruction;
    // find corresponding enum value
    opcode_t op = HOP;
    bool found = false;
    for (unsigned int i = 0; i < sizeof(opName) / sizeof(opName[0]); i++) {
      if (instruction == opName[i]) {
        op = static_cast<opcode_t>(i);
        found = true;
        break;
      }
    }
    if (!found) {
      std::cout << "Error: Instruction " << instruction
                << " is not recognized!\n";
      return false;
    }
    // has address, and is in range
    if (op == IFEMPTY || op == IFENEMY || op == IFSAME || op == IFWALL ||
        op == GO) {
      if (!(iss >> address)) {
        std::cout << "Error: Missing address.\n";
        return false;
      } else if (address >= MAXPROGRAM) {
        std::cout << "Error: Address is out of bounds.\n";
        return false;
      }
    }
    // no extra address
    if (op == HOP || op == LEFT || op == RIGHT || op == INFECT) {
      if (iss >> address) {
        std::cout << "Error: Unexpected address for instruction.\n";
        return false;
      }
    }
    // store it in the species program
    species.program[species.programSize].op = op;
    species.program[species.programSize].address = address;
    // iterate
    species.programSize++;
  }
  return true;
}

bool readCreatures(const std::string& creaturesFile, world_t& world) {
  // REQUIRES: creaturesfile is valid
  // MODIFIES: world
  // EFFECTS: reads world file
  std::ifstream inputFile(creaturesFile);
  if (!inputFile) {
    std::cout << "Error: Cannot open file " << creaturesFile << "!\n";
    return false;
  }
  // read grid dimensions
  if (!(inputFile >> world.grid.height >> world.grid.width)) {
    std::cout << "Error: Invalid grid dimensions.\n";
    return false;
  }
  if (world.grid.height > MAXHEIGHT || world.grid.height < 1) {
    std::cout << "Error: The grid height is illegal!\n";
    return false;
  } else if (world.grid.width > MAXWIDTH || world.grid.width < 1) {
    std::cout << "Error: The grid width is illegal!\n";
    return false;
  }
  // initialize empty grid
  for (unsigned int r = 0; r < world.grid.height; r++) {
    for (unsigned int c = 0; c < world.grid.width; c++) {
      world.grid.squares[r][c] = nullptr;
    }
  }
  // get creature info
  world.numCreatures = 0;
  std::string line;
  std::getline(inputFile, line);
  while (std::getline(inputFile, line)) {
    std::istringstream iss(line);
    int row = 0;
    int col = 0;
    std::string speciesName;
    std::string speciesDirection;
    iss >> speciesName >> speciesDirection >> row >> col;
    if (row < 0 || row >= static_cast<int>(world.grid.height) || col < 0 ||
        col >= static_cast<int>(world.grid.width)) {
      std::cout << "Error: Creature (" << speciesName << " " << speciesDirection
                << " " << row << " " << col << ") is out of bound!\n";
      std::cout << "The grid size is " << world.grid.height << "-by-"
                << world.grid.width << ".\n";
      return false;
    }
    species_t* species = nullptr;
    for (unsigned int i = 0; i < world.numSpecies; i++) {
      if (world.species[i].name == speciesName) {
        species = &world.species[i];
        break;
      }
    }
    if (species == nullptr) {
      std::cout << "Error: Species " << speciesName << " not found!\n";
      return false;
    }
    direction_t dir = EAST;
    bool found = false;
    for (unsigned int i = 0; i < 4; i++) {
      if (speciesDirection == directName[i]) {
        dir = static_cast<direction_t>(i);
        found = true;
        break;
      }
    }
    if (!found) {
      std::cout << "Error: Direction " << speciesDirection
                << " is not recognized!\n";
      return false;
    }
    // initialize creature
    if (world.numCreatures >= MAXCREATURES) {
      std::cout << "Error: Too many creatures!\n";
      std::cout << "Maximal number of creatures is " << MAXCREATURES << ".\n";
      return false;
    }
    if (world.grid.squares[row][col] != nullptr) {
      std::cout << "Error: Creature (" << speciesName << " " << speciesDirection
                << " " << row << " " << col << ") overlaps with creature ("
                << world.grid.squares[row][col]->species->name << " "
                << directName[world.grid.squares[row][col]->direction] << " "
                << world.grid.squares[row][col]->location.r << " "
                << world.grid.squares[row][col]->location.c << ")!\n";
      return false;
    }
    creature_t& creature = world.creatures[world.numCreatures];
    creature.species = species;
    creature.direction = dir;
    creature.location.r = row;
    creature.location.c = col;
    creature.programID = 0;
    // place creature on grid
    world.grid.squares[row][col] = &creature;
    // iterate
    world.numCreatures++;
  }
  return true;
}

bool initWorld(world_t& world, const std::string& speciesFile,
               const std::string& creaturesFile) {
  // MODIFIES: world
  //
  // EFFECTS: Initialize "world" given the species summary file
  //          "speciesFile" and the world description file
  //          "creaturesFile". This initializes all the components of
  //          "world". Returns true if initialization is successful.
  // speciesFile read
  std::string directory = readSpeciesFile(speciesFile, world);
  if (directory.empty()) {
    return false;
  }
  // program files read
  for (unsigned int i = 0; i < world.numSpecies; i++) {
    std::string programFile = directory + "/" + world.species[i].name;
    if (!readProgram(programFile, world.species[i])) {
      return false;
    }
  }
  // creaturesFile read
  if (!readCreatures(creaturesFile, world)) {
    return false;
  }
  return true;
}

void simulateCreature(creature_t& creature, grid_t& grid, bool verbose) {
  // REQUIRES: creature is inside the grid.
  //
  // MODIFIES: creature, grid, cout.
  //
  // EFFECTS: Simulate one turn of "creature" and update the creature,
  //          the infected creature, and the grid if necessary.
  //          The creature programID is always updated. The function
  //          also prints to the stdout the procedure. If verbose is
  //          true, it prints more information.
  std::cout << "Creature (" << creature.species->name << " "
            << directName[creature.direction] << " " << creature.location.r
            << " " << creature.location.c << ") takes action:";
  while (true) {
    instruction_t instruction = getInstruction(creature);
    if (verbose) {
      std::cout << "Instruction " << creature.programID + 1 << ": "
                << opName[instruction.op];
      if (instruction.op == IFEMPTY || instruction.op == IFENEMY ||
          instruction.op == IFSAME || instruction.op == IFWALL ||
          instruction.op == GO) {
        std::cout << " " << instruction.address;
      }
      std::cout << "\n";
    }
    if (instruction.op == HOP) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (isInside(next, grid) && getCreature(grid, next) == nullptr) {
        grid.squares[creature.location.r][creature.location.c] = nullptr;
        creature.location = next;
        grid.squares[next.r][next.c] = &creature;
      }
      creature.programID++;
      if (!verbose) {
        std::cout << " hop\n";
      }
      break;
    } else if (instruction.op == LEFT) {
      creature.direction = leftFrom(creature.direction);
      creature.programID++;
      if (!verbose) {
        std::cout << " left\n";
      }
      break;
    } else if (instruction.op == RIGHT) {
      creature.direction = rightFrom(creature.direction);
      creature.programID++;
      if (!verbose) {
        std::cout << " right\n";
      }
      break;
    } else if (instruction.op == INFECT) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (isInside(next, grid) && getCreature(grid, next) != nullptr &&
          getCreature(grid, next)->species != creature.species) {
        getCreature(grid, next)->species = creature.species;
        getCreature(grid, next)->programID = 0;
      }
      creature.programID++;
      if (!verbose) {
        std::cout << " infect\n";
      }
      break;
    } else if (instruction.op == IFEMPTY) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (isInside(next, grid) && getCreature(grid, next) == nullptr) {
        creature.programID = instruction.address - 1;
      } else {
        creature.programID++;
      }
    } else if (instruction.op == IFENEMY) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (isInside(next, grid) && getCreature(grid, next) != nullptr &&
          getCreature(grid, next)->species != creature.species) {
        creature.programID = instruction.address - 1;
      } else {
        creature.programID++;
      }
    } else if (instruction.op == IFSAME) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (isInside(next, grid) && getCreature(grid, next) != nullptr &&
          getCreature(grid, next)->species == creature.species) {
        creature.programID = instruction.address - 1;
      } else {
        creature.programID++;
      }
    } else if (instruction.op == IFWALL) {
      point_t next = adjacentPoint(creature.location, creature.direction);
      if (!isInside(next, grid)) {
        creature.programID = instruction.address - 1;
      } else {
        creature.programID++;
      }
    } else if (instruction.op == GO) {
      creature.programID = instruction.address - 1;
    }
  }
}

void printGrid(const grid_t& grid) {
  // MODIFIES: cout.
  //
  // EFFECTS: print a grid representation of the creature world.

  for (unsigned int r = 0; r < grid.height; r++) {
    for (unsigned int c = 0; c < grid.width; c++) {
      if (grid.squares[r][c] == nullptr) {
        std::cout << "____";
      } else {
        std::cout << grid.squares[r][c]->species->name.substr(0, 2) << "_"
                  << directShortName[grid.squares[r][c]->direction];
      }
      std::cout << " ";
    }
    std::cout << "\n";
  }
}

point_t adjacentPoint(point_t pt, direction_t dir) {
  // EFFECTS: Returns a point that results from moving one square
  //          in the direction "dir" from the point "pt".
  unsigned int c = 0;
  unsigned int r = 0;
  c = pt.c;
  r = pt.r;
  if (dir == EAST) {
    pt.c = c + 1;
  } else if (dir == SOUTH) {
    pt.r = r + 1;
  } else if (dir == WEST) {
    pt.c = c - 1;
  } else if (dir == NORTH) {
    pt.r = r - 1;
  }
  return pt;
}

direction_t leftFrom(direction_t dir) {
  // EFFECTS: Returns the direction that results from turning
  //          left from the given direction "dir".
  if (dir == EAST) {
    dir = NORTH;
  } else if (dir == SOUTH) {
    dir = EAST;
  } else if (dir == WEST) {
    dir = SOUTH;
  } else if (dir == NORTH) {
    dir = WEST;
  }
  return dir;
}

direction_t rightFrom(direction_t dir) {
  // EFFECTS: Returns the direction that results from turning
  //          right from the given direction "dir".
  if (dir == EAST) {
    dir = SOUTH;
  } else if (dir == SOUTH) {
    dir = WEST;
  } else if (dir == WEST) {
    dir = NORTH;
  } else if (dir == NORTH) {
    dir = EAST;
  }
  return dir;
}

instruction_t getInstruction(const creature_t& creature) {
  // EFFECTS: Returns the current instruction of "creature".
  return creature.species->program[creature.programID];
}

creature_t* getCreature(const grid_t& grid, point_t location) {
  // REQUIRES: location is inside the grid.
  //
  // EFFECTS: Returns a pointer to the creature at "location" in "grid".
  return grid.squares[location.r][location.c];
}

bool isInside(point_t pt, const grid_t& grid) {
  // EFFECTS: checks if a point is inside the grid
  if (static_cast<unsigned int>(pt.r) >= grid.height || pt.r < 0 || pt.c < 0 ||
      static_cast<unsigned int>(pt.c) >= grid.width) {
    return false;
  }
  return true;
}