#include <stdio.h>
#include "radar_pi.h"
using namespace RadarPlugin;

int main(int argc, const char* argv[]){
  opencpn_plugin* plugin = new radar_pi(nullptr);

  plugin->Init();


  printf("radar_exe\n");
}
