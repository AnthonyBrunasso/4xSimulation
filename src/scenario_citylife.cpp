#include "scenario_citylife.h"


namespace scenario_citylife {
  bool s_active;

  bool active()
  {
    return s_active;
  }

  void start()
  {
    s_active = true;
  }

  void process()
  {

  }

  void reset() {
    s_active = false;
  }
}

