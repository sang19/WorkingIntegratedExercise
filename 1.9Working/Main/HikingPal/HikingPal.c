/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "graphics.h"
#include "SDCard_Test_Program.h"
#include "gps.h"
#include "wifi.h"

int main()
{
  printf("Hello from Nios II!\n");
  TestShapes();
  DrawFilledRectangle(0, 800, 0, 480, 0);
  getLocationData();
  TestSDCard();
  Wifi_Init();

  return 0;
}
