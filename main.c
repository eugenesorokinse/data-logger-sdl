/*
 *  Serial data plotter / logger
 *  Platform: linux, SDL2
 *
 *  By Eugene Sorokin
 *
 */

#include "main.h"
#include "basic.h"

// ----------------------------------------------------------------------------

main_window_data_t main_window =
{
    .win_w = 1280,
    .win_h = 720,

    .color_background = MAIN_WINDOW_COLOUR_BACKGROUND,
};

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // some preps
  main_window.serial.dev_p = (argc > 1) ? argv[1] : "/dev/ttyACM0";
  main_window.serial.baud = (argc > 2) ? atoi(argv[2]) : 115200;

  if(main_create(&main_window) == MM_OK)
  {
    main_loop(&main_window);
  }
  main_cleanup(&main_window);

  return 0;
}
