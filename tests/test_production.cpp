#include "production.cpp"
#include <cstdio>

int main()
{
  ConstructionQueueFIFO cityFifo;

  cityFifo.Add(CONSTRUCTION::SCOUT_UNIT);
  cityFifo.Add(CONSTRUCTION::FORGE);
  cityFifo.Add(CONSTRUCTION::MELEE_UNIT);

  cityFifo.Simulation();

  cityFifo.PrintQueue();

  std::cout << "(60) turns will pass, Press enter" << std::endl;
  getchar();

  for (int i = 0; i < 60; ++i) {
    cityFifo.Simulation();
    if (i == 20) {
      cityFifo.Move(0, 1);
    }
  }

  std::cout << "(60) turns will pass, Press enter" << std::endl;
  getchar();

  for (int i = 0; i < 60; ++i) {
    cityFifo.Simulation();
  }

  std::cout << "(15) turns will pass, Press enter" << std::endl;
  getchar();

  cityFifo.Add(CONSTRUCTION::GRANARY);

  for (int i = 0; i < 15; ++i) {
    cityFifo.Simulation();
  }

  cityFifo.PrintState();

  return 0;
}

