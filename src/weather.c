/*
************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"

extern struct time_info_data time_info;

extern void make_exit(int dir, int from_room, int to_room);
extern void clear_exit(int dir, int from_room);

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void modify_boats(int time);

void weather_and_time(int mode)
{
  another_hour(mode);
  if (mode)
    weather_change();
}


void another_hour(int mode)
{
  time_info.hours++;
  modify_boats(time_info.hours);
  if (mode) {
    switch (time_info.hours) {
    case 5:
      weather_info.sunlight = SUN_RISE;
      send_to_outdoor("&RThe sun of Deimos rises in the east with orange radiance.&n\r\n");
      break;
    case 6:
      weather_info.sunlight = SUN_LIGHT;
      send_to_outdoor("&YThe sun begins to shine its light over the realm of Deimos.&n\r\n");
      break;
    case 21:
      weather_info.sunlight = SUN_SET;
      send_to_outdoor("&MThe sun begins to set in the west as its light dims to a beautiful purple and red.&n\r\n");
      break;
    case 22:
      weather_info.sunlight = SUN_DARK;
      send_to_outdoor("&RNight sets in over Deimos leaving the world in darkness.&n\r\n");
      break;
    default:
      break;
    }
  }
  if (time_info.hours > 23) {	/* Changed by HHS due to bug ??? */
    time_info.hours -= 24;
    time_info.day++;

    if (time_info.day > 34) {
      time_info.day = 0;
      time_info.month++;

      if (time_info.month > 16) {
	time_info.month = 0;
	time_info.year++;
      }
    }
  }
}


void weather_change(void)
{
  int diff, change;
  if ((time_info.month >= 9) && (time_info.month <= 16))
    diff = (weather_info.pressure > 985 ? -2 : 2);
  else
    diff = (weather_info.pressure > 1015 ? -2 : 2);

  weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);

  change = 0;

  switch (weather_info.sky) {
  case SKY_CLOUDLESS:
    if (weather_info.pressure < 990)
      change = 1;
    else if (weather_info.pressure < 1010)
      if (dice(1, 4) == 1)
	change = 1;
    break;
  case SKY_CLOUDY:
    if (weather_info.pressure < 970)
      change = 2;
    else if (weather_info.pressure < 990) {
      if (dice(1, 4) == 1)
	change = 2;
      else
	change = 0;
    } else if (weather_info.pressure > 1030)
      if (dice(1, 4) == 1)
	change = 3;

    break;
  case SKY_RAINING:
    if (weather_info.pressure < 970) {
      if (dice(1, 4) == 1)
	change = 4;
      else
	change = 0;
    } else if (weather_info.pressure > 1030)
      change = 5;
    else if (weather_info.pressure > 1010)
      if (dice(1, 4) == 1)
	change = 5;

    break;
  case SKY_LIGHTNING:
    if (weather_info.pressure > 1010)
      change = 6;
    else if (weather_info.pressure > 990)
      if (dice(1, 4) == 1)
	change = 6;

    break;
  default:
    change = 0;
    weather_info.sky = SKY_CLOUDLESS;
    break;
  }

  switch (change) {
  case 0:
    break;
  case 1:
    send_to_outdoor("&DClouds fill the sky with impending doom.&n\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 2:
    send_to_outdoor("&BDroplets of rain begin to fall from the sky.&n\r\n");
    weather_info.sky = SKY_RAINING;
    break;
  case 3:
    send_to_outdoor("&wThe clouds leave the sky slowly.&n\r\n");
    weather_info.sky = SKY_CLOUDLESS;
    break;
  case 4:
    send_to_outdoor("&YThe earth reverberates with thunder and flashes with lightning.&n\r\n");
    weather_info.sky = SKY_LIGHTNING;
    break;
  case 5:
    send_to_outdoor("&cThe rain slowly begins to let up and the sky begins to clear.&n\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 6:
    send_to_outdoor("&yThe thunder and lightning flashes grow less frequent.&n\r\n");
    weather_info.sky = SKY_RAINING;
    break;
  default:
    break;
  }
}


void modify_boats(int time) {
    int dockone = 4222, docktwo = 8564, ship = 1519;
	switch (time) { /* Phobos to Arenburg Route */
	case 5: case 11: case 17: case 23:
		make_exit(EAST, ship ,dockone);
		clear_exit(WEST, ship);
		send_to_room("&WThe ship has arrived at port.&n\r\n", real_room(ship));
		send_to_room("&WA great elven sailing ship has arrived at port.&n\r\n", real_room(dockone));
      break;
	case 6: case 12: case 18: case 24:
		clear_exit(EAST, ship);
		clear_exit(WEST, ship);
		send_to_room("&WThe ship casts off its lines and begins its journey.&n\r\n", real_room(ship));
		send_to_room("&WA great elven frigate leaves for it's homeland.&n\r\n", real_room(dockone));
	  break;
	case 8: case 14: case 20: case 2:
		clear_exit(EAST, ship);
		make_exit(WEST, ship , docktwo);
		send_to_room("&WThe ship has arrived at port.&n\r\n", real_room(ship));
		send_to_room("&WA great frigate ruturns home.&n\r\n", real_room(docktwo));
	  break;
	case 9: case 15: case 21: case 3:
		clear_exit(WEST, ship);
		clear_exit(EAST, ship);
		send_to_room("&WThe ship casts off its lines and begins its journey.&n\r\n", real_room(ship));
		send_to_room("&WA great elven frigate leaves for Phobos.&n\r\n", real_room(docktwo));
	  break;
    }
}
