/* Capture the flag */
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "dg_scripts.h"

void do_message_world(char *argument, int type);
void return_blue_flag();
void return_red_flag();


extern int freeze_game_running;
extern int freeze_game_timer;
extern int freeze_game_blue;
extern int freeze_game_red;
extern int freeze_game_iblue;
extern int freeze_game_ired;
extern int freeze_game_redpnt;
extern int freeze_game_bluepnt;
extern room_vnum blue_room;
extern room_vnum red_room;


extern zone_rnum top_of_zone_table;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;

void endgame()
{
  struct descriptor_data *d;

  sprintf(buf, "&MThe CTF match has ended! %d red, %d blue", 
          freeze_game_redpnt, freeze_game_bluepnt);
  do_message_world(buf, MSG_FREEZE);
  if (freeze_game_bluepnt == freeze_game_redpnt) {
    sprintf(buf, "&yThere has been a tie in the CTF match!");
    do_message_world(buf, MSG_FREEZE);
  }
  else if (freeze_game_bluepnt > freeze_game_redpnt) {
    sprintf(buf, "&bThe Blue Team has won the match!");
    do_message_world(buf, MSG_FREEZE);
  }
  else {
    sprintf(buf, "&rThe Red Team has won the match!");
    do_message_world(buf, MSG_FREEZE);
  }

  freeze_game_red = 0;
  freeze_game_redpnt = 0;
  freeze_game_ired = 0;
  freeze_game_blue = 0;
  freeze_game_bluepnt = 0;
  freeze_game_iblue = 0;
  freeze_game_running = 0;


  for (d = descriptor_list; d; d = d->next)
    if (STATE(d) == CON_PLAYING)
       if (GET_FREEZE_TEAM(d->character)) {
         GET_FREEZE_TEAM(d->character) = TEAM_NONE; 
         GET_POS(d->character) = POS_STANDING; 
       }
}

void checkgame()
{
  if (freeze_game_red == 0 || freeze_game_blue == 0)
     endgame();
}

void return_blue_flag()
{
  struct obj_data *blueflag;
  obj_rnum thing = 5001;
  zone_rnum i;
  int num = 0;
    
 for (i = 0; i <= top_of_zone_table; i++) {
   for (num = 0, blueflag = object_list; blueflag; blueflag = blueflag->next)
   {
   if (world[blueflag->in_room].zone == i && IS_OBJ_STAT(blueflag, ITEM_BLUEFLAG))
       extract_obj(blueflag);
     }
   }
  blueflag = read_object(thing, VIRTUAL);
  obj_to_room(blueflag, real_room(blue_room));
}

void return_red_flag()
{
  struct obj_data *redflag;
  obj_rnum thing = 5000;
  zone_rnum i;
  int num = 0;
    
 for (i = 0; i <= top_of_zone_table; i++) {
   for (num = 0, redflag = object_list; redflag; redflag = redflag->next)
   {
   if (world[redflag->in_room].zone == i && IS_OBJ_STAT(redflag, ITEM_REDFLAG))
       extract_obj(redflag);
     }
   }
  redflag = read_object(thing, VIRTUAL);
  obj_to_room(redflag, real_room(red_room));
}

ACMD(do_endgame)
{
  if (!freeze_game_running) { 
     send_to_char("There is no CTF match running!\r\n", ch);
     return;
  }
  send_to_char("You terminate the existing game!\r\n", ch);
  endgame();

}

ACMD(do_startgame)
{
  struct descriptor_data *d;

  two_arguments(argument, buf, buf1);
  
  if (!*buf || !*buf1) {
    send_to_char("Usage: startgame (blue_room) (red_room) \r\n", ch);
    return;
  }

  blue_room = atoi(buf);
  red_room = atoi(buf1);

  if (freeze_game_running) { 
     send_to_char("A CTF match has already started!\r\n", ch);
     return;
  }
  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_PLAYING) {
       if (GET_FREEZE_TEAM(d->character) == TEAM_RED) freeze_game_red++;
       else if (GET_FREEZE_TEAM(d->character) == TEAM_BLUE) freeze_game_blue++;
    }
  }
  if (!freeze_game_blue || !freeze_game_red) {
     send_to_char("Do the teams even exist?\r\n", ch);
     freeze_game_red = 0;
     freeze_game_blue = 0;
     return;
  }

  freeze_game_iblue = freeze_game_blue;
  freeze_game_ired  = freeze_game_red;
  freeze_game_timer = 10;	// These games will last a max of 10 ticks
  freeze_game_running = 1;
  freeze_game_redpnt = 0;
  freeze_game_bluepnt = 0;
  sprintf(buf, "&MA CTF match has started, %d blues, %d reds, for %d tics!&n",
          freeze_game_blue, freeze_game_red, freeze_game_timer);
  do_message_world(buf, MSG_FREEZE);
  send_to_char(buf, ch);
}


ACMD(do_addteam)
{
  char name[MAX_STRING_LENGTH], team[MAX_STRING_LENGTH];
  struct char_data *vict = NULL;

  two_arguments(argument, name, team);

  if (!name || !team) {
     send_to_char("Usage: addteam name team\r\n", ch);
     return;
  }
  if ( !(vict = get_player_vis(ch, name, FIND_CHAR_WORLD)) ) {
     send_to_char("They don't seem to be around here.\r\n", ch);
     return;
  }
  if ( IS_NPC(vict) ) {
     send_to_char("No monsters, only players.\r\n", ch);
     return;
  }
  if ( is_abbrev(team, "red") ) {
    send_to_char("They are now on the red team.\r\n", ch);
    send_to_char("You are now on the red team.\r\n", vict);
    GET_FREEZE_TEAM(vict) = TEAM_RED;
  }
  else if ( is_abbrev(team, "blue") ) {
    send_to_char("They are now on the blue team.\r\n", ch);
    send_to_char("You are now on the blue team.\r\n", vict);
    GET_FREEZE_TEAM(vict) = TEAM_BLUE;
  }
  else {
     send_to_char("Red or blue teams only.\r\n", ch);
     return;
  }
}

ACMD(do_tag)
{
  struct char_data *vict = NULL;
  
  one_argument(argument, buf);

  if (!freeze_game_running) {
     send_to_char("There isn't a CTF match running!\r\n", ch);
     return;
  }

  if (!buf) {
     send_to_char("You who?\r\n", ch);
     return;
  }
  GET_WAIT_STATE(ch) += PULSE_VIOLENCE/2;
  if (!(vict = get_char_room_vis(ch, buf))) {
     send_to_char("They don't seem to be around here.\r\n", ch);
     return;
  }
  if (ch == vict) {
     send_to_char("Ha ha ha ha...\r\n", ch);
     return;
  }
  if (IS_NPC(vict) ) {
     send_to_char("You can only tag other players!\r\n", ch);
     return;
  }
  if (!GET_FREEZE_TEAM(ch)) {
     send_to_char("You aren't even playing CTF!\r\n", ch);
     return;
  }
  if (!GET_FREEZE_TEAM(vict)) {
     send_to_char("They aren't even playing CTF!\r\n", ch);
     return;
  }
  if (GET_FREEZE_TEAM(vict) != GET_FREEZE_TEAM(ch) && GET_POS(vict) == POS_FROZEN) {
     send_to_char("They are already frozen, stop touching them.\r\n", ch);
     return;
  }
  if (GET_FREEZE_TEAM(vict) == GET_FREEZE_TEAM(ch) && GET_POS(vict) == POS_FROZEN) {
     send_to_char("You tag your fellow team mate and restore them!\r\n", ch);
     send_to_char("You have been restored from frozen status\r\n", vict);
     GET_POS(vict) = POS_STANDING;
     if (GET_FREEZE_TEAM(vict) == TEAM_BLUE) freeze_game_blue++;
     else if (GET_FREEZE_TEAM(vict) == TEAM_RED) freeze_game_red++;
  }
  else if (GET_FREEZE_TEAM(vict) != GET_FREEZE_TEAM(ch)  && GET_POS(vict) != POS_FROZEN) {
   if (PLR_FLAGGED2(vict, PLR2_FLAGGER)) {
     send_to_char("You tag the flagger! Flag is returned!", ch);
    if (GET_FREEZE_TEAM(vict) == TEAM_RED) {
     REMOVE_BIT(PLR_FLAGS2(vict), PLR2_FLAGGER);
     return_blue_flag();      
     return;
     }
    if (GET_FREEZE_TEAM(vict) == TEAM_BLUE) {
     REMOVE_BIT(PLR_FLAGS2(vict), PLR2_FLAGGER);
     return_red_flag();      
     return;
     }
   }
     send_to_char("You tag your enemy, they are frozen!\r\n", ch);
     GET_POS(vict) = POS_FROZEN;
     GET_WAIT_STATE(ch) -= PULSE_VIOLENCE/2;
     if(GET_WAIT_STATE(ch) < 0) { GET_WAIT_STATE(ch) = 0;}

     if (GET_FREEZE_TEAM(vict) == TEAM_BLUE) {
       freeze_game_redpnt++;
       freeze_game_blue--;

     }
     else if (GET_FREEZE_TEAM(vict) == TEAM_RED) {
       freeze_game_bluepnt++;
       freeze_game_red--;
     }
     sprintf(buf, "&M%s has tagged %s!", GET_NAME(ch), GET_NAME(vict));
     do_message_world(buf, MSG_FREEZE);
  }
  else
     send_to_char("That won't accomplish anything at all!\r\n", ch);
  checkgame();
}

ACMD(do_tsay)
{
  struct descriptor_data *d;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!freeze_game_running) {
     send_to_char("There isn't a CTF match running!\r\n", ch);
     return;
  }
  if (!GET_FREEZE_TEAM(ch)) {
     send_to_char("You aren't even playing CTF!\r\n", ch);
     return;
  }

  if (!*argument) {
    sprintf(buf, "&m#&M----- CTF -----&m#&n\r\n"
                 "&m|&nBlue Members: &b%-2d/%-2d&m|&n\r\n"
                 "&m|&nBlue Score: &B%-2d&m|&n\r\n"
                 "&m|&nRed Members: &r%-2d/%-2d&m|&n\r\n"
                 "&m|&nRed Score : &R%-2d&m|&n\r\n"
                 "&m#&M----------------&m#&n\r\n",
                freeze_game_blue, 
                freeze_game_iblue, 
                freeze_game_bluepnt, 
                freeze_game_red,
                freeze_game_ired,
                freeze_game_redpnt);
    send_to_char(buf, ch);

  }
  else {
    if (GET_FREEZE_TEAM(ch) == TEAM_RED) sprintf(buf2, "&r[RED] %s:&n ", GET_NAME(ch));
    if (GET_FREEZE_TEAM(ch) == TEAM_BLUE) sprintf(buf2, "&b[BLUE] %s:&n ", GET_NAME(ch));
    sprintf(buf, "%s", argument);
    strcat(buf2, buf);

    for (d = descriptor_list; d; d = d->next) {
       if (STATE(d) == CON_PLAYING)
         if (GET_FREEZE_TEAM(d->character) == GET_FREEZE_TEAM(ch)) { 
           act(buf2, FALSE, ch, 0, d->character, TO_VICT | TO_SLEEP);
         }
      }
    }
  }

