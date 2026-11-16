// only include main function
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "simulation.h"
#include "world_type.h"

int main(int argc, char* argv[]) {
  // check inputs
  if (argc < 4) {
    std::cout << "Error: Missing arguments!\n";
    std::cout
        << "Usage: ./p3 <species-summary> <world-file> <rounds> [v|verbose]\n";
    return 0;
  }
  int rounds = atoi(argv[3]);
  if (rounds < 0) {
    std::cout << "Error: Number of simulation rounds is negative!\n";
    return 0;
  }
  bool verbose = false;
  if (argc >= 5 &&
      (std::string(argv[4]) == "v" || std::string(argv[4]) == "verbose")) {
    verbose = true;
  }

  // initialize world
  world_t world;
  if (!initWorld(world, argv[1], argv[2])) {
    return 0;
  }
  std::cout << "Initial state\n";
  printGrid(world.grid);

  // perform rounds
  for (int i = 0; i < rounds; i++) {
    std::cout << "Round " << i + 1 << "\n";
    for (unsigned int j = 0; j < world.numCreatures; j++) {
      simulateCreature(world.creatures[j], world.grid, verbose);

      if (verbose) {
        printGrid(world.grid);
      }
    }
    if (!verbose) {
      printGrid(world.grid);
    }
  }
}