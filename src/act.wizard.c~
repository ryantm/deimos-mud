/*
************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "assemblies.h"
#include "dg_scripts.h"
#include "genolc.h"
#include "genmob.h"
#include "oasis.h"
#include "improved-edit.h"
#include "diskio.h"
#define MOB 1
#define OBJ 2

/*   external vars  */
extern FILE *player_fl;
extern socket_t mother_desc;
extern ush_int port;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct char_data *mob_proto;  
extern struct obj_data *obj_proto;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct attack_hit_type attack_hit_text[];
extern char *class_abbrevs[];
extern time_t boot_time;
extern zone_rnum top_of_zone_table;
extern int circle_shutdown, circle_reboot;
extern int circle_restrict;
extern int load_into_inventory;
extern int buf_switches, buf_largecount, buf_overflows;
extern mob_rnum top_of_mobt;
extern obj_rnum top_of_objt;
extern int top_of_p_table;
extern int compute_armor_class(struct char_data *ch);
extern struct player_index_element *player_table;/* for reimbursal*/
extern int frozen_start_room;
extern void delete_eq(struct char_data *ch);
extern void set_lname(struct char_data * ch, char * lname);
extern time_t LAST_ECON_RESET;
extern int file_to_string_alloc(const char *name, char **buf);
extern char *econrank;
extern struct clan_type *clan_info;
extern char *happytimes;

/* for chars */
extern const char *pc_class_types[];

/* extern functions */
struct index_data **trig_index;
int level_exp(int chclass, int level);
void show_shops(struct char_data * ch, char *value);
void hcontrol_list_houses(struct char_data *ch);
void do_start(struct char_data *ch);
void appear(struct char_data *ch);
void reset_zone(zone_rnum zone);
void roll_real_abils(struct char_data *ch);
int parse_class(char arg);
struct char_data *find_char(int n);
int zdelete_check(int zone);
void Read_Invalid_List(void);
void Crash_rentsave(struct char_data * ch, int cost);
void medit_setup_existing(struct descriptor_data *d, int rmob_num);
int is_name(const char *str, const char *namelist); 
void hedit_save_to_disk(void);
int save_shops(zone_rnum zone_num);
int save_mobiles(zone_rnum rznum);
int save_rooms(zone_rnum rzone);
int save_objects(zone_rnum ozone);
int save_zone(zone_rnum zone_num);
zone_rnum real_zone(zone_vnum vnum);
struct question_index *find_question(char *keyword);
int copy_object(struct obj_data *to, struct obj_data *from);

/* local functions */
int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg);
void perform_immort_invis(struct char_data *ch, int level);
ACMD(do_echo);
int search_mobile(struct char_data *ch, int mode, int value);
int search_object(struct char_data *ch, int mode, int value);
ACMD(do_find);
ACMD(do_send);
room_rnum find_target_room(struct char_data * ch, char *rawroomstr);
ACMD(do_at);
ACMD(do_goto);
ACMD(do_slay);
ACMD(do_trans);
ACMD(do_teleport);
ACMD(do_vnum);
void do_stat_room(struct char_data * ch);
void do_stat_object(struct char_data * ch, struct obj_data * j);
void do_stat_character(struct char_data * ch, struct char_data * k);
ACMD(do_stat);
ACMD(do_shutdown);
void stop_snooping(struct char_data * ch);
void perform_assassin_invis(struct char_data *ch, int level);
void perform_assassin_vis(struct char_data *ch);
ACMD(do_snoop);
ACMD(do_switch);
ACMD(do_return);
ACMD(do_load);
ACMD(do_vstat);
ACMD(do_purge);
ACMD(do_syslog);
ACMD(do_advance);
ACMD(do_deathtoall);
ACMD(do_restore);
void perform_immort_vis(struct char_data *ch);
ACMD(do_invis);
ACMD(do_gecho);
ACMD(do_poofin);
ACMD(do_poofout);
ACMD(do_dc);
ACMD(do_dig);
ACMD(do_copyto);
ACMD(do_wizlock);
ACMD(do_date);
ACMD(do_last);
ACMD(do_force);
ACMD(do_wiznet);
ACMD(do_zreset);
ACMD(do_wizutil);
void print_zone_to_buf(char *bufptr, zone_rnum zone);
ACMD(do_saveall);
ACMD(do_show);
ACMD(do_set);
ACMD(do_zsave);
ACMD(do_understand);
ACMD(do_haffinate);
ACMD(do_zdelete);
ACMD(do_pyoink);
ACMD(do_xname);
ACMD(do_verify);
ACMD(do_roomlink);
ACMD(do_undig);
ACMD(do_whoinvis);
ACMD(do_vlist);
ACMD(do_statlist);
ACMD(do_peace);
ACMD(do_lag);
ACMD(do_builderize);
void make_exit(int dir, int from_room, int to_room);
void clear_exit(int dir, int from_room);
ACMD(do_doubletog);

/* Special Quest Stuff */
byte ISHAPPY = 0; // 0 off, 1 exp, 2 gold, 3 razors
bool DOUBLEEXP = FALSE;
bool DOUBLEGOLD = FALSE;
bool GOLDCHIPPER = FALSE;
bool QUESTON = FALSE;
bool QUOTAOK = TRUE;
bool IMPSON = FALSE;
bool CANMULTIPLAY = FALSE;
bool CANASSASSINATE = TRUE;

ACMD(do_zclear);
void clearzone(zone_rnum z);


ACMD(do_liblist)
  {
    extern struct room_data *world;
    extern struct index_data *mob_index;
    extern struct char_data *mob_proto;
    extern struct index_data *obj_index;
    extern struct obj_data *obj_proto;
    extern struct zone_data *zone_table;

    int first, last, nr, found = 0;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
      switch (subcmd) {
        case SCMD_RLIST:
          send_to_char("Usage: rlist  \r\n", ch);
          break;
        case SCMD_OLIST:
          send_to_char("Usage: olist  \r\n", ch);
          break;
        case SCMD_MLIST:
          send_to_char("Usage: mlist  \r\n", ch);
          break;
        case SCMD_ZLIST:
          send_to_char("Usage: zlist  \r\n", ch);
          break;
        default:
          sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
          mudlog(buf, BRF, LVL_GOD, TRUE);
          break;
      }
      return;
    }

    first = atoi(buf);
    last = atoi(buf2);

    if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
      send_to_char("Values must be between 0 and 99999.\n\r", ch);
      return;
    }

    if (first >= last) {
      send_to_char("Second value must be greater than first.\n\r", ch);
      return;
    }

    switch (subcmd) {
      case SCMD_RLIST:
        sprintf(buf, "Room List From Vnum %d to %d\r\n", first, last);
        for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++) {
          if (world[nr].number >= first) {
            sprintf(buf, "%5d. [%5d] (%3d) %s\r\n", ++found,
                    world[nr].number, world[nr].zone,
                    world[nr].name);
            send_to_char(buf,ch);
          }
        }
        break;
      case SCMD_OLIST:
        sprintf(buf, "Object List From Vnum %d to %d\r\n", first, last);
        for (nr = 0; nr <= top_of_objt && (obj_index[nr].vnum <= last); nr++) {
          if (obj_index[nr].vnum >= first) {
            sprintf(buf, "%5d. [%5d] %s\r\n", ++found,
                    obj_index[nr].vnum,
                    obj_proto[nr].short_description);
            send_to_char(buf,ch);
          }
        }
        break;
      case SCMD_MLIST:
        sprintf(buf, "Mob List From Vnum %d to %d\r\n", first, last);
        for (nr = 0; nr <= top_of_mobt && (mob_index[nr].vnum <= last); nr++)  {
          if (mob_index[nr].vnum >= first) {
            sprintf(buf, "%5d. [%5d] %s\r\n", ++found,
                    mob_index[nr].vnum,
                    mob_proto[nr].player.short_descr);
            send_to_char(buf, ch);
          }
        }
        break;
      case SCMD_ZLIST:
        sprintf(buf, "Zone List From Vnum %d to %d\r\n", first, last);
        for (nr = 0; nr <= top_of_zone_table && (zone_table[nr].number <= last); nr++) {
          if (zone_table[nr].number >= first) {
            sprintf(buf, "%5d. [%5d] (%3d) %s\r\n", ++found,
                    zone_table[nr].number, zone_table[nr].lifespan,
                    zone_table[nr].name);
            send_to_char(buf, ch);
          }
        }
        break;
      default:
        sprintf(buf, "SYSERR:: invalid SCMD passed to ACMD do_build_list!");
        mudlog(buf, BRF, LVL_GOD, TRUE);
        return;
    }

    if (!found) {
      switch (subcmd) {
        case SCMD_RLIST:
          send_to_char("No rooms found within those parameters.\r\n", ch);
          break;
        case SCMD_OLIST:
          send_to_char("No objects found within those parameters.\r\n",ch);
          break;
        case SCMD_MLIST:
          send_to_char("No mobiles found within those parameters.\r\n",ch);
          break;
        case SCMD_ZLIST:
          send_to_char("No zones found within those parameters.\r\n", ch);
          break;
        default:
          sprintf(buf, "SYSERR:: invalid SCMD passed to do_build_list!");
          mudlog(buf, BRF, LVL_GOD, TRUE);
          break;
      }
      return;
    }
    
  }



ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes.. but what?\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE)
      sprintf(buf, "&W$n&W %s&n", argument);
    else
      strcpy(buf, argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data * ch, char *rawroomstr)
{
  room_vnum tmp;
  room_rnum location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return (NOWHERE);
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return (NOWHERE);
    }
  } else if ((target_mob = get_char_vis(ch, roomstr, FIND_CHAR_WORLD)) != NULL)
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr)) != NULL) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return (NOWHERE);
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return (NOWHERE);
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_LEVEL(ch) < LVL_GRGOD) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return (NOWHERE);
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return (NOWHERE);
    }
    if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
	!House_can_enter(ch, GET_ROOM_VNUM(location))) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return (NOWHERE);
    }
  }
  return (location);
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  room_rnum location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  room_rnum location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (GET_LEVEL(ch) < LVL_IMMORT && !PLR_FLAGGED2(ch, PLR2_REVIEW)) {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
 
  if (POOFOUT(ch))
    sprintf(buf, "$n %s", POOFOUT(ch));
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (POOFIN(ch))
    sprintf(buf, "$n %s", POOFIN(ch));
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}


ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
     if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
        return;
      act("&M$n is suddenly called to leave and fades from existence.&n", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      
      if (GET_LEVEL(victim) < LVL_IMMORT)
      {
        act("&M$n materializes here in front of $N.&n", FALSE, victim, 0, ch, TO_ROOM);
        act("&M$n grabs you with $s large ethereal hand, and places you in front of $m!&n", FALSE, ch, 0, victim, TO_VICT);
      }
      else
      {
         act("&M$n materializes here in response to $N's call.&n", FALSE, victim, 0, ch, TO_ROOM);
         act("&M$n calls you from your tasks and calls you to stand by $s side.&n", FALSE, ch, 0, victim, TO_VICT);
      }
      look_at_room(victim, 0);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char("I think not.\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
 	  continue;
        if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
          return;

	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  room_rnum target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch) && !IS_NPC(victim))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
        return;
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
  }
}

int find_attack_type(char *arg)
{
  int nr;

  for (nr=0; nr < NUM_ATTACK_TYPES; nr++) {
    if (is_abbrev(arg, attack_hit_text[nr].singular))
      break;
  }

  return (nr);
}

ACMD(do_vnum)
{
  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj") && !is_abbrev(buf, "weapon"))) {
    send_to_char("Usage: vnum { obj | mob | weapon } <name | attack-type>\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char("No mobiles by that name.\r\n", ch);

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char("No objects by that name.\r\n", ch);

  
  if (is_abbrev(buf, "weapon"))
    if (!vnum_weapon(find_attack_type(buf2), ch))
      send_to_char("No weapons with that attack-type.\r\n", ch);
}



void do_stat_room(struct char_data * ch)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[ch->in_room];
  int i, found;
  struct obj_data *j;
  struct char_data *k;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
	  zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
	  CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

  sprintbit(rm->room_flags, room_bits, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
	  (rm->func == NULL) ? "None" : "Exists", buf2);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
		GET_ROOM_VNUM(rm->dir_option[i]->to_room), CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }

  /* check the room for a script */
  do_sstat_room(ch);
}

/* Check the previous command in the list to determine whether
 * it was used in the same room or not
 */
bool check_same_room(int zone, int subcmd, room_rnum rnum)
{
//	sprintf(buf, "subcmd is %d", subcmd);
  // log(buf);

	if (subcmd <= 0)  /* Looks like we're already at the bottom, tough luck */
   	return FALSE;

	if (ZCMD(zone, (subcmd-1)).if_flag) {  /* Need to go back another command */
   	if (check_same_room(zone, (subcmd - 1), rnum) != TRUE)
      	return FALSE; /* Checking previous command found no match to the rnum */
      else
      	return TRUE;  /* Found it! */
 } else {
  	switch (ZCMD(zone, (subcmd-1)).command){
			case 'M':   /* Mobile */
     case 'V':		/* Variable - Used with DG Scripts, omit this case if you don't use them*/
	    case 'O':		/* Object */
	      	if (ZCMD(zone, (subcmd-1)).arg3 == rnum)
            	return TRUE;
	      	break;
  	   case 'R':	/* Room */
	     case 'D':	/* Door */
	      	if (ZCMD(zone, (subcmd-1)).arg1 == rnum)
            	return TRUE;
      		break;
   	 default: break;
	   }
  }
	 return FALSE;
}

/* Called from vstat - show the zone reset information for
 * the specified room
 */
void do_zstat_room(struct char_data *ch, zone_rnum rnum)
{
  int zone = world[rnum].zone;
  int subcmd = 0, counter = 0;
  room_vnum rvnum = GET_ROOM_VNUM(rnum);
  extern struct char_data *mob_proto;
  extern struct obj_data *obj_proto;
  char output[MAX_STRING_LENGTH];
  bool found;

  sprintf(buf,
    "Zone number : %d [%4d]\r\n"
    "Zone name   : %s\r\n"
    "Builders    : %s\r\n",
    zone_table[zone].number, rvnum, zone_table[zone].name ? zone_table[zone].name : "<NONE!>",
    zone_table[zone].builders ? zone_table[zone].builders : "<NONE!>");
    send_to_char(buf, ch);

  if (!(ZCMD(zone, subcmd).command)){
    send_to_char("No zone loading information.\r\n", ch);
    return;
  }

  sprintf(output, "--------------------------------------------------\r\n");
  while ((ZCMD(zone, subcmd).command) && (ZCMD(zone, subcmd).command != 'S')) {
 	 found = FALSE;
    /*
     * Translate what the command means.
     */

   /* If this is a "then do this" command, we need to check that the previous
    * command was issued in this room
    */
   if (ZCMD(zone, subcmd).if_flag == TRUE){
		  if (check_same_room(zone, subcmd, rnum) == FALSE){
       	subcmd++;
         continue; /* not this room */
      }
    }
    switch (ZCMD(zone, subcmd).command) {
    case 'M':
       if (ZCMD(zone, subcmd).arg3 != rnum)
       	break; /* not this room */
      sprintf(buf2, "%sLoad %s [%d], Max : %d",
              ZCMD(zone, subcmd).if_flag ? " then " : "",
              mob_proto[ZCMD(zone, subcmd).arg1].player.short_descr,
              mob_index[ZCMD(zone, subcmd).arg1].vnum, ZCMD(zone, subcmd).arg2);
      found = TRUE;
      break;
    case 'G':
      sprintf(buf2, "%sGive it %s [%d], Max : %d",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      obj_proto[ZCMD(zone, subcmd).arg1].short_description,
	      obj_index[ZCMD(zone, subcmd).arg1].vnum,
	      ZCMD(zone, subcmd).arg2);
      found = TRUE;
      break;
    case 'O':
      if (ZCMD(zone, subcmd).arg3 != rnum)
      	break; /* not this room */
      sprintf(buf2, "%sLoad %s [%d], Max : %d",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      obj_proto[ZCMD(zone, subcmd).arg1].short_description,
	      obj_index[ZCMD(zone, subcmd).arg1].vnum,
	      ZCMD(zone, subcmd).arg2);
      found = TRUE;
      break;
    case 'E':
     sprintf(buf2, "%sEquip with %s [%d], %s, Max : %d",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      obj_proto[ZCMD(zone, subcmd).arg1].short_description,
	      obj_index[ZCMD(zone, subcmd).arg1].vnum,
	      equipment_types[ZCMD(zone, subcmd).arg3],
	      ZCMD(zone, subcmd).arg2);

      found = TRUE;
      break;
    case 'P':
      sprintf(buf2, "%sPut %s [%d] in %s [%d], Max : %d",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      obj_proto[ZCMD(zone, subcmd).arg1].short_description,
	      obj_index[ZCMD(zone, subcmd).arg1].vnum,
	      obj_proto[ZCMD(zone, subcmd).arg3].short_description,
	      obj_index[ZCMD(zone, subcmd).arg3].vnum,
	      ZCMD(zone, subcmd).arg2);
      found = TRUE;
      break;
    case 'R':
       if (ZCMD(zone, subcmd).arg1 != rnum)
         break; // not this room
       sprintf(buf2, "%sRemove %s [%d] from room.",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      obj_proto[ZCMD(zone, subcmd).arg2].short_description,
	      obj_index[ZCMD(zone, subcmd).arg2].vnum);
      found = TRUE;
      break;
    case 'D':
       if (ZCMD(zone, subcmd).arg1 != rnum)
       	break; /* not this room */
     sprintf(buf2, "%sSet door %s as %s.",
	      ZCMD(zone, subcmd).if_flag ? " then " : "",
	      dirs[ZCMD(zone, subcmd).arg2],
	      ZCMD(zone, subcmd).arg3 ? ((ZCMD(zone, subcmd).arg3 == 1) ? ((ZCMD(zone, 
subcmd).arg3 == 2) ? "hidden" : "closed") : "locked") : "open");
      found = TRUE;
      break;
    case 'T': /* Omit this case if the latest version of DGScripts is not installed */
      sprintf(buf2, "%sAttach trigger %s [%d] to %s",
        ZCMD(zone, subcmd).if_flag ? " then " : "",
        trig_index[real_trigger(ZCMD(zone, subcmd).arg2)]->proto->name,
        ZCMD(zone, subcmd).arg2,
        ((ZCMD(zone, subcmd).arg1 == MOB_TRIGGER) ? "mobile" :
          ((ZCMD(zone, subcmd).arg1 == OBJ_TRIGGER) ? "object" :
            ((ZCMD(zone, subcmd).arg1 == WLD_TRIGGER)? "room" : "????"))));
      found = TRUE;
      break;
    case 'V':  /* Omit this case if the latest version of DGScripts is not installed */
      if (ZCMD(zone, subcmd).arg1 != WLD_TRIGGER)
      	break; /* not a world trigger then it doesn't concern us. */
      if (ZCMD(zone, subcmd).arg3 != rnum)
      	break; /* not this room */
      sprintf(buf2, "%sAssign global %s:%d to %s = %s",
        ZCMD(zone, subcmd).if_flag ? " then " : "",
        ZCMD(zone, subcmd).sarg1, ZCMD(zone, subcmd).arg2,
        ((ZCMD(zone, subcmd).arg1 == MOB_TRIGGER) ? "mobile" :
          ((ZCMD(zone, subcmd).arg1 == OBJ_TRIGGER) ? "object" :
            ((ZCMD(zone, subcmd).arg1 == WLD_TRIGGER)? "room" : "????"))),
        ZCMD(zone, subcmd).sarg2);
      found = TRUE;
      break;
    default: /* Catch all */
      sprintf(buf2, "<Unknown Command>");
      found = TRUE;
      break;
    }
    /*
     * Build the display buffer for this command.
     * Only show things that apply to this room.
     */
    if (found == TRUE){
	    sprintf(buf1, "%d - %s\r\n", counter++, buf2);
   	 strcat(output, buf1);
    }
    subcmd++;
  }
  send_to_char(output, ch);

}


void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int i, found;
  obj_vnum vnum;
  struct obj_data *j2;
  struct extra_descr_data *desc;

  vnum = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
   CCGRN(ch, C_NRM), vnum, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2);
  send_to_char(buf, ch);
  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on: ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d, Min Level: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j), GET_OBJ_LEVEL(j));
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%d", GET_ROOM_VNUM(IN_ROOM(j)));
    strcat(buf, buf2);
  }
  /*
   * NOTE: In order to make it this far, we must already be able to see the
   *       character holding the object. Therefore, we do not need CAN_SEE().
   */
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "Hours left: Infinite");
    else
      sprintf(buf, "Hours left: [%d]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Spells: (Level %d) %s, %s, %s", GET_OBJ_VAL(j, 0),
	    skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	    skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "Spell: %s at level %d, %d (of %d) charges remaining",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprintf(buf, "Todam: %dd%d, Message type: %d",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  case ITEM_ARMOR:
    sprintf(buf, "AC-apply: [%d]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    sprintf(buf, "Spell: %d, - Hitpoints: %d",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Tongue: %d", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    strcpy(buf, "");
    break;
  case ITEM_FOOD:
    sprintf(buf, "Makes full: %d, Poisoned: %s", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Coins: %d", GET_OBJ_VAL(j, 0));
    break;
  default:
    sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    sprintf(buf, "\r\nContents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);

  /* check the object for a script */
  do_sstat_object(ch, j);
}


void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int i, i2, found = 0;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;

  sprinttype(GET_SEX(k), genders, buf);
  sprintf(buf2, "&n %s '%s'  IDNum: [%5ld], In room [%5d]\r\n",
	  (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), GET_IDNUM(k), GET_ROOM_VNUM(IN_ROOM(k)));
  send_to_char(strcat(buf, buf2), ch);
  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
	    k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    send_to_char(buf, ch);
  }

  sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
  send_to_char(buf, ch);

  sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
  send_to_char(buf, ch);

  if (IS_NPC(k)) {	/* Use GET_CLASS() macro? */
    strcpy(buf, "Monster Class: ");
    sprinttype(k->player.chclass, npc_class_types, buf2);
  } else {
    strcpy(buf, "Class: ");
    sprinttype(k->player.chclass, pc_class_types, buf2);
  }
  strcat(buf, buf2);

 if (IS_NPC(k)) {
   if (!MOB_FLAGGED(k, MOB_EDIT)) {
  sprintf(buf2, ", Lev: [%s%2d%s]\r\nXP: [%s%7d%s], Align: [%4d]\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	  CCYEL(ch, C_NRM), (GET_LEVEL(k) * GET_LEVEL(k) * 1000), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k));
  strcat(buf, buf2);
  send_to_char(buf, ch);
 } else {
  sprintf(buf2, ", Lev: [%s%2d%s]\r\nXP: [%s%7d%s], Align: [%4d]\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	  CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k));
  strcat(buf, buf2);
  send_to_char(buf, ch);
  }
 } else {
  sprintf(buf2, ", Lev: [%s%2d%s], TLev: [%s%2d%s], WLev: [%s%2d%s], CLev: [%s%2d%s], MLev: [%s%2d%s]\r\nGMLev: [%2d] XP: [%s%7d%s], Align: [%4d]\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_THIEF_LEVEL(k), CCNRM(ch, C_NRM),  
          CCYEL(ch, C_NRM), GET_WARRIOR_LEVEL(k), CCNRM(ch, C_NRM),  
          CCYEL(ch, C_NRM), GET_CLERIC_LEVEL(k), CCNRM(ch, C_NRM),  
          CCYEL(ch, C_NRM), GET_MAGE_LEVEL(k), CCNRM(ch, C_NRM),
          GET_GM_LEVEL(k),
	  CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k));
  strcat(buf, buf2);
  send_to_char(buf, ch);
  }
  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played % 3600) / 60), age(k)->year);
    send_to_char(buf, ch);

    sprintf(buf, "Hometown: [%d], Speaks: [%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])",
	 k->player.hometown, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
	    GET_PRACTICES(k), int_app[GET_INT(k)].learn,
	    wis_app[GET_WIS(k)].bonus);
    /*. Display OLC zone for immorts .*/
    if (GET_LEVEL(k) >= LVL_IMMORT)
      sprintf(buf + strlen(buf), ", OLC[%d]", GET_OLC_ZONE(k));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
  }
  sprintf(buf, "Str: [%s%d/%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Coins: [%9d], Bank: [%9d] (Total: %d)\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
  send_to_char(buf, ch);

  if (IS_NPC(k) && !MOB_FLAGGED(k, MOB_EDIT)) {
  sprintf(buf, "AC: [%d], Hitroll: [%2d], Damroll: [%2d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  GET_AC(k), (GET_LEVEL(k) / 2 + 10),
	  k->points.damroll, GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2),
	  GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);
  } else {
  sprintf(buf, "AC: [%d], Hitroll: [%2d], Damroll: [%2d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  GET_AC(k), k->points.hitroll,
	  k->points.damroll, GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2),
	  GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);
  }

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	  (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, ", Attack type: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(STATE(k->desc), connected_types, buf2);
    strcat(buf, ", Connected: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  strcpy(buf, "Default position: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PLR_FLAGS2(k), player_bits2, buf2);
    sprintf(buf, "PLR2: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS2(k), preference_bits2, buf2);
    sprintf(buf, "PRF2: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (PLR_FLAGGED(k, PLR_HASHORSE)) {
    sprintf(buf, "Horse: %-8s HorseMV: %d/%d\r\n", GET_HORSENAME(k), GET_HORSEHAPPY(k), 
GET_MAXHAPPY(k));
    send_to_char(buf, ch);
  }

  if (!IS_NPC(k)) {
    sprintf(buf, "Host: %s\r\n", GET_HOST(k));
    send_to_char(buf, ch);
    sprintf(buf, "Sparring rank: %d  ", GET_SPARRANK(k));
    send_to_char(buf, ch);
    sprintf(buf, "Economy rank: %d  ", GET_ECONRANK(k));
    send_to_char(buf, ch);
    sprintf(buf, "Religion: %d\r\n", GET_RELIGION(k));
    send_to_char(buf, ch);
    sprintf(buf, "Sacrifices: %d  ", GET_SACRIFICE(k));
    send_to_char(buf, ch);

    sprintf(buf, "Kills: %d      Deaths: %d    Ratio: %f \r\n", GET_KILLS(k), GET_DEATHS(k), (float)GET_KILLS(k)/(float)GET_DEATHS(k));
    send_to_char(buf, ch);
//    sprintf(buf, "TetherRoom: %d  \r\n", GET_TETHER(k, 1));
 //   send_to_char(buf, ch);

  }

  
  if (IS_MOB(k)) {
   if (!MOB_FLAGGED(k, MOB_EDIT)) {
    sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
	    (GET_LEVEL(k) / 4), (GET_LEVEL(k) / 5));
    send_to_char(buf, ch);
   } else {
    sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
    }
  }

  sprintf(buf, "Carried: weight: %d, items: %d; ",
	  IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf + strlen(buf), "Items in: inventory: %d, ", i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
    sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
	  GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
    send_to_char(buf, ch);
  }

  sprintf(buf, "Master is: %s, Followers are:",
	  ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

     /* Show player on mobs piss list */

   {
    memory_rec *names;
    if (IS_NPC(k) && (MOB_FLAGGED(k, MOB_MEMORY))) {
      sprintf(buf, "Pissed List:\r\n");
      for (names = MEMORY(k); names && !found; names = names->next) {
        sprintf(buf2, "  %s\r\n", names->name);
        strcat(buf, buf2);
       }
       send_to_char(strcat(buf, "\r\n"), ch);
      }
   }

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
	      CCCYN(ch, C_NRM), skill_name(aff->type), CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	sprintbit(aff->bitvector, affected_bits, buf2);
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
  /*Compute Armor Class */
  sprintf(buf, "ArmorClass: %d\r\n", compute_armor_class(k));
  send_to_char(buf,ch);
  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char("Script memory:\r\n  Remember             Command\r\n", ch);
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc) send_to_char("  ** Corrupted!\r\n", ch);
        else {
          if (mem->cmd) sprintf(buf,"  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else sprintf(buf,"  %-20.20s <default>\r\n",GET_NAME(mc));
          send_to_char(buf, ch);
        }
      mem = mem->next;
      }
    }
  } else {
    /* this is a PC, display their global variables */
    if (k->script && k->script->global_vars) {
      struct trig_var_data *tv;
      char name[MAX_INPUT_LENGTH];
      void find_uid_name(char *uid, char *name);

      send_to_char("Global Variables:\r\n", ch);

      /* currently, variable context for players is always 0, so it is */
      /* not displayed here. in the future, this might change */
      for (tv = k->script->global_vars; tv; tv = tv->next) {
        if (*(tv->value) == UID_CHAR) {
          find_uid_name(tv->value, name);
          sprintf(buf, "    %10s:  [UID]: %s\r\n", tv->name, name);
        } else
          sprintf(buf, "    %10s:  %s\r\n", tv->name, tv->value);
        send_to_char(buf, ch);
      }
    }
  }

}


ACMD(do_stat)
{
  struct char_data *victim;
  struct obj_data *object;
  int tmp;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
    do_zstat_room(ch, ch->in_room);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
     if (load_char(buf2, victim) > -1) { 
	char_to_room(victim, 0);
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
	  do_stat_character(ch, victim);
	extract_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)) != NULL)
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)) != NULL)
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1, FIND_CHAR_ROOM)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1, FIND_CHAR_WORLD)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)) != NULL)
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "now")) {
    log("(GC) Shutdown NOW by %s.", GET_NAME(ch));
    send_to_all("The world in front of you collapses into nothing.\r\n");
    circle_shutdown = 1;
    circle_reboot = 2;
  } else if (!str_cmp(arg, "reboot")) {
    log("(GC) Reboot by %s.", GET_NAME(ch));
    send_to_all("The world in front of you collapses into nothing.\r\n");
    touch(FASTBOOT_FILE);
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(KILLSCRIPT_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(PAUSE_FILE);
    circle_shutdown = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
    send_to_char("No such person around.\r\n", ch);
  else if (!victim->desc)
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Don't be stupid.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char("You can't.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("You're already switched.\r\n", ch);
  else if (!*arg)
    send_to_char("Switch with who?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
    send_to_char("No such character.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  else if (victim->desc)
    send_to_char("You can't do that, the body is already in use!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_GODROOM))
    send_to_char("You are not godly enough to use that room!\r\n", ch);
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_HOUSE)
		&& !House_can_enter(ch, GET_ROOM_VNUM(IN_ROOM(victim))))
    send_to_char("That's private property -- no trespassing!\r\n", ch);
  else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    /*
     * If someone switched into your original body, disconnect them.
     *   - JE 2/22/95
     *
     * Zmey: here we put someone switched in our body to disconnect state
     * but we must also NULL his pointer to our character, otherwise
     * close_socket() will damage our character's pointer to our descriptor
     * (which is assigned below in this function). 12/17/99
     */
    if (ch->desc->original->desc) {
      ch->desc->original->desc->character = NULL;
      STATE(ch->desc->original->desc) = CON_DISCONNECT;
    }

    /* Now our descriptor points to our original body. */
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    /* And our body's pointer to descriptor now points to our descriptor. */
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_load)
{
  struct char_data *mob;
  struct obj_data *obj;
  mob_vnum number;
  mob_rnum r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: load { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < LVL_DEITY 
      && !is_name(GET_NAME(ch), zone_table[world[ch->in_room].zone].builders)) {
  send_to_char("You can't load stuff not from your zone, Sorry.\r\n", ch);
  return;
  }

  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    load_mtrigger(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    if (load_into_inventory)
      obj_to_char(obj, ch);
    else
      obj_to_room(obj, ch->in_room);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    load_otrigger(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}

ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  mob_vnum number;	/* or obj_vnum ... */
  mob_rnum r_num;	/* or obj_rnum ... */

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob | room } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
 } else if (is_abbrev(buf, "room")){
     if ((r_num = real_room(number)) < 0) {
       send_to_char("That room does not exist.\r\n", ch);
       return;
     }
     do_zstat_room(ch, r_num);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob' or 'room'.\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if (*buf) {			/* argument supplied. destroy single object
				 * or char */
    if ((vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)) != NULL) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
	if (vict->desc) {
	  STATE(vict->desc) = CON_CLOSE;
	  vict->desc->character = NULL;
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents)) != NULL) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }

    send_to_char(OK, ch);
  } else {			/* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
	extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}



const char *logtypes[] = {
  "off", "brief", "normal", "complete", "\n"
};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}

#define EXE_FILE "bin/circle" /* maybe use argv[0] but it's not reliable */

/* (c) 1996-97 Erwin S. Andreasen <erwin@pip.dknet.dk> */
ACMD(do_copyover)
{
	FILE *fp;
	struct descriptor_data *d, *d_next;
	char buf [100], buf2[100];
        struct char_data *tch;

        for (tch = character_list; tch; tch = tch->next)
        {
            if (GET_LEVEL(tch) >= LVL_IMMORT) 
              continue;
 
            if (GET_HIT(tch) < 0 || PLR_FLAGGED(tch, PLR_DEAD))
            {
              send_to_char("&RSomeone is dead, Don't copyover now!&n\r\n", ch);
              return;
            }
            if (FIGHTING(tch))
            {
              send_to_char("&RSomeone is fighting, Don't copyover now!&n\r\n", ch);
              return;
            }
        }
        

	fp = fopen (COPYOVER_FILE, "w");

	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		return;
	}

	/* Consider changing all saved areas here, if you use OLC */
	sprintf (buf, "\t\x1B[1;31m \007\007\007Time stops for a moment as %s folds space and time.\x1B[0;0m\r\n", GET_NAME(ch));
	/* For each playing descriptor, save its state */
	Crash_save_all();
                log("Copyover");

        for (d = descriptor_list; d ; d = d_next)
	{
		struct char_data * och = d->character;
		d_next = d->next; /* We delete from the list , so need to save this */
		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
            write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few seconds.\n\r");
			close_socket (d); /* throw'em out */
		}
		else
		{
		fprintf (fp, "%d %s %s\n", d->descriptor, GET_NAME(och), d->host);
//(d->character->master) ? GET_NAME(d->character->master) : " ");
            /* save och */
            Crash_rentsave(och,0);
            save_char(och, och->in_room);
			write_to_descriptor (d->descriptor, buf);
		}
	}

	fprintf (fp, "-1\n");
	fclose (fp);

	/* Close reserve and other always-open files and release other resources
           since we are now using ASCII pfiles, closing the player_fl would crash
           the game, since it's no longer around, so I commented it out. I'll
           leave the code here, for historical reasons -spl

     fclose(player_fl); */

	/* exec - descriptors are inherited */

	sprintf (buf, "%d", port);
       send_to_char(buf,ch);
    sprintf (buf2, "-C%d", mother_desc);
    /* Ugh, seems it is expected we are 1 step above lib - this may be dangerous! */
    chdir ("..");


	execl (EXE_FILE, "circle", buf2, buf, (char *) NULL);
	/* Failed - sucessful exec will not return */

	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);

    exit (1); /* too much trouble to try to recover! */
}

ACMD(do_advance)
{
  struct char_data *victim;
  char *name = arg, *level = buf2;
  int newlevel, oldlevel;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name, FIND_CHAR_WORLD))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char("They are already at that level.\r\n", ch);
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char("You are momentarily enveloped by darkness!\r\n"
		 "You feel somewhat diminished.\r\n", victim);
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(OK, ch);

  if (newlevel < oldlevel)
    log("(GC) %s demoted %s from level %d to %d.",
		GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  else
    log("(GC) %s has advanced %s to level %d (from %d)",
		GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);

  save_char(victim, NOWHERE);
}


ACMD(do_restore)
{
  struct char_data *vict;
  struct descriptor_data *i;
  int c;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("&RWhom do you wish to restore?&n\r\n", ch);
  else if (str_cmp("all", buf)) {
       if (!(vict = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
    send_to_char(NOPERSON, ch);
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);
    GET_HORSEHAPPY(vict) = GET_MAXHAPPY(vict);
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEAD);
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADI); 
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADII); 
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADIII); 

    if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
      for (c = 1; c <= MAX_SKILLS; c++)
	SET_SKILL(vict, c, 1);

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
	vict->real_abils.intel = 25;
	vict->real_abils.wis = 25;
	vict->real_abils.dex = 25;
	vict->real_abils.str = 25;
	vict->real_abils.con = 25;
	vict->real_abils.cha = 25;
      }
      vict->aff_abils = vict->real_abils;
    }
    update_pos(vict);
    send_to_char(OK, ch);
    act("&CYou feel $N's hand come down from the heavens and &Grestore&C your health!&n", FALSE, vict, 0, ch, TO_CHAR);
  }
  } else {
    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
	vict = i->character;
	if (GET_LEVEL(vict) >= GET_LEVEL(ch))
	  continue;
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);
    GET_HORSEHAPPY(vict) = GET_MAXHAPPY(vict);
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEAD); 
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADI); 
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADII); 
    REMOVE_BIT(PLR_FLAGS(vict), PLR_DEADIII); 

    if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
      for (c = 1; c <= MAX_SKILLS; c++)
	SET_SKILL(vict, c, 100);

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
	vict->real_abils.intel = 25;
	vict->real_abils.wis = 25;
	vict->real_abils.dex = 25;
	vict->real_abils.str = 25;
	vict->real_abils.con = 25;
	vict->real_abils.cha = 25;
      }
      vict->aff_abils = vict->real_abils;
    }
    update_pos(vict);
    act("&CYou feel $N's hand come down from the heavens and &Grestore&C your health!&n", FALSE, vict, 0, ch, TO_CHAR);
      }
    send_to_char(OK, ch);
  }
}


void perform_immort_vis(struct char_data *ch)
{
  if (GET_INVIS_LEV(ch) == 0 && !AFF_FLAGGED(ch, AFF_HIDE | AFF_INVISIBLE | AFF_SHADE)) {
    send_to_char("You are already fully visible.\r\n", ch);
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Your invisibility level is %d.\r\n", level);
  send_to_char(buf, ch);
}
  

ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char("You can't go invisible above your own level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch)
	send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      send_to_char(buf, ch);
  }
}


ACMD(do_poofin)
  {
    skip_spaces(&argument);

    if (!*argument)
    {
      sprintf(buf, "Current Poofin: %s %s\r\n", GET_NAME(ch), POOFIN(ch));
      send_to_char(buf, ch);
      return;
    }

    if (POOFIN(ch))
      free(POOFIN(ch));

    POOFIN(ch) = str_dup(argument);
    sprintf(buf, "Poofin set to: %s %s\r\n", GET_NAME(ch), POOFIN(ch));
    send_to_char(buf, ch);
  }


  ACMD(do_poofout)
  {
    skip_spaces(&argument);

    if (!*argument)
    {
      sprintf(buf, "Current Poofout: %s %s\r\n", GET_NAME(ch), POOFOUT(ch));
      send_to_char(buf, ch);
      return;
    }

    if (POOFOUT(ch))
      free(POOFOUT(ch));

    POOFOUT(ch) = str_dup(argument);
    sprintf(buf, "Poofout set to: %s %s\r\n", GET_NAME(ch), POOFOUT(ch));
    send_to_char(buf, ch);
  }



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <user number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    if (!CAN_SEE(ch, d->character))
      send_to_char("No such connection.\r\n", ch);
    else
      send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }

  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor. -je
   *
   * It is a much more logical extension for a CON_DISCONNECT to be used
   * for in-game socket closes and CON_CLOSE for out of game closings.
   * This will retain the stability of the close_me hack while being
   * neater in appearance. -gg 12/1/97
   *
   * For those unlucky souls who actually manage to get disconnected
   * by two different immortals in the same 1/10th of a second, we have
   * the below 'if' check. -gg 12/17/99
   */
  if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
    send_to_char("They're already being disconnected.\r\n", ch);
  else {
    /*
     * Remember that we can disconnect people not in the game and
     * that rather confuses the code when it expected there to be
     * a character context.
     */
    if (STATE(d) == CON_PLAYING)
      STATE(d) = CON_DISCONNECT;
    else
      STATE(d) = CON_CLOSE;

    sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
    send_to_char(buf, ch);
    log("(GC) Connection closed by %s.", GET_NAME(ch));
  }
}



ACMD(do_wizlock)
{
  int value;
  const char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    circle_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (circle_restrict) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    circle_restrict, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	    ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}

struct char_data *is_in_game(int idnum) {
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i, *next_i;
 
  for (i = descriptor_list; i; i = next_i) {
    next_i = i->next;
    if (GET_IDNUM(i->character) == idnum) {
      return i->character;
    }
  }
  return NULL;
}

/* altered from stock to the following:
   last [name] [#]
   last without arguments displays the last 10 entries.
   last with a name only displays the 'stock' last entry.
   last with a number displays that many entries (combines with name)
*/


ACMD(do_last)
{
  struct char_data *vict = NULL;
 
  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  CREATE(vict, struct char_data, 1);
  clear_char(vict);
  CREATE(vict->player_specials, struct player_special_data, 1);
  if (load_char(arg, vict) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    free_char(vict);
    return;
  }
 if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
    send_to_char("You are not sufficiently godly for that!\r\n", ch);
    return;
  }     
  sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
          GET_IDNUM(vict), (int) GET_LEVEL(vict),
          class_abbrevs[(int) GET_CLASS(vict)], GET_NAME(vict),
          vict->player_specials->host && *vict->player_specials->host
          ? vict->player_specials->host : "(NOHOST)",
          ctime(&vict->player.time.logon));
  send_to_char(buf, ch);                                                 
  free_char(vict);
}


ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
      send_to_char(NOPERSON, ch);
    else if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char("No, no, no!\r\n", ch);
    else {
      send_to_char(OK, ch);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced room %d to %s",
		GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (STATE(i) != CON_PLAYING || !(vict = i->character) || (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch)))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		 "       wiznet @<level> *<emotetext> | wiz @\r\n", ch);
    return;
  }

  switch(subcmd)
  {
  case SCMD_WIZARCHI:
    level = LVL_IMMORT; break;
  case SCMD_WIZCREATOR:
    level = LVL_CREATOR; break;
  case SCMD_WIZDEITY:
    level = LVL_DEITY; break;
  case SCMD_WIZCODER:
    level = LVL_CODER; break;
  case SCMD_WIZCOIMP:
    level = LVL_COIMP; break;
  case SCMD_WIZIMP:
    level = LVL_IMPL; break;
  }
  
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) {
	if (!any) {
	  strcpy(buf1, "Gods online:\r\n");
	  any = TRUE;
	}
	sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
	if (PLR_FLAGGED(d->character, PLR_WRITING))
	  strcat(buf1, " (Writing)\r\n");
	else if (PLR_FLAGGED(d->character, PLR_MAILING))
	  strcat(buf1, " (Writing mail)\r\n");
	else
	  strcat(buf1, "\r\n");

      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  CAN_SEE(ch, d->character)) {
	if (!any) {
	  strcat(buf1, "Gods offline:\r\n");
	  any = TRUE;
	}
	sprintf(buf1 + strlen(buf1), "  %s\r\n", GET_NAME(d->character));
      }
    }
    send_to_char(buf1, ch);
    return;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if (level > LVL_IMMORT) {
    sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
	    emote ? "<--- " : "", argument);
    sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
	    argument);
  } else {
    sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
	    argument);
    sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((STATE(d) == CON_PLAYING) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(CCCYN(d->character, C_NRM), d->character);
      if (CAN_SEE(d->character, ch))
	send_to_char(buf1, d->character);
      else
	send_to_char(buf2, d->character);
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}



ACMD(do_zreset)
{
  zone_rnum i = 0;
  zone_vnum j = 0;
  struct char_data *d;  
  struct obj_data *k, *next_obj;
  int num = 0;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    if (GET_LEVEL(ch) == LVL_IMPL)
    {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    return;
    }
    else
     send_to_char("You don't have the power to do this.\r\n", ch);
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
    if (i > top_of_zone_table || i < 0)
    {
       send_to_char("Zone out of range!\r\n", ch);
       return;
    }
  }
  if (GET_LEVEL(ch) < LVL_DEITY && (!is_name(GET_NAME(ch), zone_table[i].builders))) 
  {
    send_to_char("You can only reset your zones!", ch);
    return;
  } 
  if (i >= 0 && i <= top_of_zone_table) {
    for (d = character_list; d; d = d->next)
      {
	if (IS_NPC(d) && d->in_room != NOWHERE && world[d->in_room].zone == i)
	  extract_char(d);
      }

    for (num = 0, k = object_list; k; k = next_obj)
      {
        next_obj = k->next;
	if (world[k->in_room].zone == i && k->carried_by == NULL && k->worn_by == NULL)
          if (GET_OBJ_TYPE(k) != ITEM_PCCORPSE)
            extract_obj(k);
      }
    reset_zone(i);
    sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  struct char_data *vict;
  long result;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Yes, but for whom?!?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
    send_to_char("There is no such player.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("You can't do that to a mob!\r\n", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    send_to_char("Hmmm...you'd better not.\r\n", ch);
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      send_to_char("Rerolled...\r\n", ch);
      roll_real_abils(vict);
      log("(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      sprintf(buf, "New stats: Str %d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
	      GET_STR(vict), GET_INT(vict), GET_WIS(vict),
	      GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
      send_to_char(buf, ch);
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER)) {
	send_to_char("Your victim is not flagged.\r\n", ch);
	return;
      }
      REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER);
      send_to_char("Pardoned.\r\n", ch);
      send_to_char("You have been pardoned by the Gods!\r\n", vict);
      sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      if (result)
        send_to_char("&RYour use of titles has been disallowed due to abuse.&n\r\n", vict);
      else
        send_to_char("&RYour use of titles has been reallowed.&n\r\n", vict);
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      if (result)
        send_to_char("&RYou have been muted for public communication offenses.&n\r\n", vict);
      else
        send_to_char("&RYou have been unmuted.&n\r\n", vict);
      break;
    case SCMD_NORENT:
      result = PLR_TOG_CHK2(vict, PLR2_NORENT);
      sprintf(buf, "(GC) No Rent %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      if (result)
        send_to_char("&RRenting has been disallowed for you.&n\r\n", vict);
      else
        send_to_char("&RRenting has been allowed for you.&n\r\n", vict);
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Your victim is already pretty cold.\r\n", ch);
	return;
      }
      SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	   GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	send_to_char(buf, ch);
	return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected) {
	while (vict->affected)
	  affect_remove(vict, vict->affected);
	send_to_char("There is a brief flash of light!\r\n"
		     "You feel slightly different.\r\n", vict);
	send_to_char("All spells removed.\r\n", ch);
      } else {
	send_to_char("Your victim does not have any affections!\r\n", ch);
	return;
      }
      break;
    case SCMD_JAIL:
      char_from_room(vict);
      char_to_room(vict, real_room(frozen_start_room));

      act("&CBy your decree, $N has been ICED!&n", FALSE, ch, 0, vict,TO_CHAR);
      act("&CBy decree of $n you have been ICED!&n",FALSE, ch, 0,vict,TO_VICT);
      break;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
      break;
    }
    save_char(vict, NOWHERE);
  }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, zone_rnum zone)
{
  /*
  sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name,
	  zone_table[zone].age, zone_table[zone].lifespan,
	  zone_table[zone].reset_mode, zone_table[zone].top);
  */
}


ACMD(do_show)
{
  int i, j, k, l, con;		/* i, j, k to specifics? */
  zone_rnum zrn;
  zone_vnum zvn;
  zone_rnum tempzone;
  char self = 0;
  struct char_data *vict = NULL;
  struct obj_data *obj;
  struct descriptor_data *d;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];

  struct show_struct {
    const char *cmd;
    const char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "zones",		1  },	        		/* 1 */
    { "player",		LVL_GOD },
    { "rent",		LVL_GOD },
    { "stats",		LVL_IMMORT },
    { "errors",		LVL_IMPL },			/* 5 */
    { "death",		LVL_GOD },
    { "godrooms",	LVL_GOD },
    { "shops",		LVL_IMMORT },
    { "houses",		LVL_GOD },
    { "snoop",		LVL_GRGOD },			/* 10 */
    { "assemblies",     LVL_DEITY },
    { "teleport",       LVL_GRGOD },
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	sprintf(buf + strlen(buf), "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1:			/* zone */
    /* tightened up by JE 4/6/93 */
   if (PLR_FLAGGED2(ch, PLR2_REVIEW) || GET_LEVEL(ch) >= LVL_IMMORT) {
    if (self)
    {
    tempzone = world[ch->in_room].zone;
    sprintf(buf, "%3d %-30.30s\r\n",
	  zone_table[tempzone].number, zone_table[tempzone].name);
  //    print_zone_to_buf(buf, world[ch->in_room].zone);
      send_to_char(buf,ch);
    }
    else if (*value && is_number(value)) {
      for (zvn = atoi(value), zrn = 0; zone_table[zrn].number != zvn && zrn <= top_of_zone_table; zrn++);
      if (zrn <= top_of_zone_table)
      { 
    sprintf(buf, "&n%3d %-30.30s\r\n",
	   zone_table[zrn].number, zone_table[zrn].name);

//	print_zone_to_buf(buf, zrn);
        send_to_char(buf, ch);
      } 
      else {
	send_to_char("That is not a valid zone.\r\n", ch);
	return;
      }
    } else
      for (zrn = 0; zrn <= top_of_zone_table; zrn++)
      {
    sprintf(buf, "%3d %-30.30s\r\n",
	   zone_table[zrn].number, zone_table[zrn].name);

	//print_zone_to_buf(buf, zrn);
        send_to_char(buf, ch);
      }
    }
    else
    send_to_char("I don't think so...\r\n", ch);
    break;
  case 2:	         		/* player */
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }

    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
    if (load_char(value, vict) < 0) {    
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    sprintf(buf + strlen(buf), "Player: %-12s (%s) [%2d %s]\r\n", GET_NAME(vict),
      genders[(int) GET_SEX(vict)], GET_LEVEL(vict), class_abbrevs[(int)
        GET_CLASS(vict)]);  
    sprintf(buf + strlen(buf),
         "Au: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
          GET_GOLD(vict), GET_BANK_GOLD(vict), GET_EXP(vict),
          GET_ALIGNMENT(vict), GET_PRACTICES(vict));
    strcpy(birth, ctime(&vict->player.time.birth));     
    sprintf(buf + strlen(buf),
            "Started: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
          birth, ctime(&vict->player.time.logon), (int)
          (vict->player.time.played / 3600),
          (int) (vict->player.time.played / 60 % 60));   
    send_to_char(buf, ch);
    free_char(vict);
    break;
  case 3:
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    strcpy(buf, "Current stats:\r\n");
    sprintf(buf + strlen(buf), "  %5d players in game  %5d connected\r\n",
		i, con);
    sprintf(buf + strlen(buf), "  %5d registered\r\n",
		top_of_p_table + 1);
    sprintf(buf + strlen(buf), "  %5d mobiles          %5d prototypes\r\n",
		j, top_of_mobt + 1);
    sprintf(buf + strlen(buf), "  %5d objects          %5d prototypes\r\n",
		k, top_of_objt + 1);
    sprintf(buf + strlen(buf), "  %5d rooms            %5d zones\r\n",
		top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf + strlen(buf), "  %5d large bufs\r\n",
		buf_largecount);
    sprintf(buf + strlen(buf), "  %5d buf switches     %5d overflows\r\n",
		buf_switches, buf_overflows);
    send_to_char(buf, ch);
    break;
  case 5:
    strcpy(buf, "Errant Rooms\r\n------------\r\n");
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
	if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
	  sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++k, GET_ROOM_VNUM(i),
		  world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 6:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH))
	sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++j,
		GET_ROOM_VNUM(i), world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 7:
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
    if (ROOM_FLAGGED(i, ROOM_GODROOM))
      sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
		++j, GET_ROOM_VNUM(i), world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 8:
    show_shops(ch, value);
    break;
  case 9:
    hcontrol_list_houses(ch);
    break;
  case 10:
    *buf = '\0';
    send_to_char("People currently snooping:\r\n", ch);
    send_to_char("--------------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next) {
      if (d->snooping == NULL || d->character == NULL)
	continue;
      if (STATE(d) != CON_PLAYING || GET_LEVEL(ch) < GET_LEVEL(d->character))
	continue;
      if (!CAN_SEE(ch, d->character) || IN_ROOM(d->character) == NOWHERE)
	continue;
      sprintf(buf + strlen(buf), "%-10s - snooped by %s.\r\n",
               GET_NAME(d->snooping->character), GET_NAME(d->character));
    }
    send_to_char(*buf ? buf : "No one is currently snooping.\r\n", ch);
    break; /* snoop */
  case 11:
    assemblyListToChar(ch);
    break;
  case 12:
    strcpy(buf, "Teleport Rooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
    if (ROOM_FLAGGED(i, ROOM_TELEPORT))
      sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
		++j, GET_ROOM_VNUM(i), world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;
  }
}


/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


/* The set options available */
  struct set_struct {
    const char *cmd;
    const char level;
    const char pcnpc;
    const char type;
  } set_fields[] = {
   { "brief",		LVL_IMMORT, 	PC, 	BINARY },  /* 0 */
   { "invstart", 	LVL_DEITY, 	PC, 	BINARY },  /* 1 */
   { "title",		LVL_GRGOD, 	PC, 	MISC },
   { "nosummon", 	LVL_IMMORT, 	PC, 	BINARY },
   { "maxhit",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "maxmana", 	LVL_IMPL, 	BOTH, 	NUMBER },  /* 5 */
   { "maxmove", 	LVL_IMPL, 	BOTH, 	NUMBER },
   { "hit", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "mana",		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "move",		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "align",		LVL_IMMORT, 	BOTH, 	NUMBER },  /* 10 */
   { "str",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "stradd",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "int", 		LVL_IMPL, 	BOTH, 	NUMBER },
   { "wis", 		LVL_IMPL, 	BOTH, 	NUMBER },
   { "dex", 		LVL_IMPL, 	BOTH, 	NUMBER },  /* 15 */
   { "con", 		LVL_IMPL, 	BOTH, 	NUMBER },
   { "cha",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "ac", 		LVL_IMPL, 	BOTH, 	NUMBER },
   { "gold",		LVL_IMMORT, 	BOTH, 	NUMBER },
   { "bank",		LVL_IMMORT, 	PC, 	NUMBER },  /* 20 */
   { "exp", 		LVL_IMPL, 	BOTH, 	NUMBER },
   { "hitroll", 	LVL_IMPL, 	BOTH, 	NUMBER },
   { "damroll", 	LVL_IMPL, 	BOTH, 	NUMBER },
   { "invis",		LVL_IMMORT, 	PC, 	NUMBER },
   { "nohassle", 	LVL_CREATOR, 	PC, 	BINARY },  /* 25 */
   { "frozen",		LVL_FREEZE, 	PC, 	BINARY },
   { "practices", 	LVL_DEITY, 	PC, 	NUMBER },
   { "lessons", 	LVL_DEITY, 	PC, 	NUMBER },
   { "drunk",		LVL_IMMORT, 	BOTH, 	MISC },
   { "hunger",		LVL_IMMORT, 	BOTH, 	MISC },    /* 30 */
   { "thirst",		LVL_IMMORT, 	BOTH, 	MISC },
   { "killer",		LVL_IMPL, 	PC, 	BINARY },
   { "thief",		LVL_IMPL, 	PC, 	BINARY },
   { "level",		LVL_DEITY, 	BOTH, 	NUMBER },
   { "room",		LVL_IMPL, 	BOTH, 	NUMBER },  /* 35 */
   { "roomflag", 	LVL_CREATOR, 	PC, 	BINARY },
   { "siteok",		LVL_IMPL, 	PC, 	BINARY },
   { "deleted", 	LVL_IMPL, 	PC, 	BINARY },
   { "class",		LVL_IMPL, 	BOTH, 	MISC },
   { "nowizlist", 	LVL_IMPL, 	PC, 	BINARY },  /* 40 */
   { "quest",		LVL_IMMORT, 	PC, 	BINARY },
   { "loadroom", 	LVL_IMMORT, 	PC, 	MISC },
   { "color",		LVL_IMMORT, 	PC, 	BINARY },
   { "idnum",		LVL_IMPL, 	PC, 	NUMBER },
   { "passwd",		LVL_IMPL, 	PC, 	MISC },    /* 45 */
   { "nodelete", 	LVL_IMPL, 	PC, 	BINARY },
   { "sex", 		LVL_IMPL, 	BOTH, 	MISC },
   { "age",		LVL_IMMORT,	BOTH,	NUMBER },
   { "height",		LVL_CREATOR,	BOTH,	NUMBER },
   { "weight",		LVL_CREATOR,	BOTH,	NUMBER },  /* 50 */
   { "olc",		LVL_DEITY,	PC,	NUMBER },
   { "assassin",        LVL_GRGOD,      PC,     BINARY },
   { "hbuilder",        LVL_IMPL,       PC,     BINARY },
   { "dead",            LVL_CREATOR,     PC,     BINARY },
   { "tlevel",          LVL_IMPL,       PC,     NUMBER },
   { "wlevel",          LVL_IMPL,       PC,     NUMBER }, 
   { "mlevel",          LVL_IMPL,       PC,     NUMBER }, 
   { "clevel",          LVL_IMPL,       PC,     NUMBER }, 
   { "multione",        LVL_IMPL,       PC,     BINARY },
   { "multitwo",        LVL_IMPL,       PC,     BINARY },  
   { "multithree",      LVL_IMPL,       PC,     BINARY },  
   { "romance",         LVL_IMPL,       PC,     NUMBER },
   { "anon",            LVL_IMMORT,     PC,     BINARY },
   { "whoinvis",        LVL_DEITY,      PC,	BINARY },
   { "fedhorse",        LVL_IMPL,       PC,     BINARY },
   { "review",          LVL_IMPL,       PC,	BINARY },
   { "pretitle",        LVL_DEITY,      PC,     MISC   },
   { "hlevel",          LVL_IMPL,       PC,     NUMBER }, 
   { "hexp",            LVL_IMPL,       PC,     NUMBER }, 
   { "heq",             LVL_IMPL,       PC,     NUMBER }, 
   { "sparrank",        LVL_IMPL,       PC,     NUMBER }, 
   { "writer",          LVL_IMPL,       PC,     BINARY },
   { "kills",           LVL_DEITY,      PC,     NUMBER },
   { "deaths",          LVL_DEITY,      PC,     NUMBER },
   { "gmlevel",         LVL_IMPL,       PC,     NUMBER },
   { "lastname",        LVL_IMPL,       PC,     MISC   },
   { "islord",          LVL_IMPL,       PC,     BINARY },
   { "econrank",        LVL_IMPL,       PC,     NUMBER }, 
   { "sacrifices",      LVL_IMPL,       PC,     NUMBER }, 
   { "religion",        LVL_IMPL,       PC,     NUMBER }, 
   { "mute",            LVL_CREATOR,    PC,     BINARY },
   { "realimm",         LVL_IMPL,       PC,     BINARY },
   { "ipmask",          LVL_IMPL,       PC,     BINARY },
   { "cantrigedit",     LVL_DEITY,      PC,     BINARY },        
   { "clan",            LVL_IMPL,       PC,     NUMBER },
   { "idle",            LVL_DEITY,      PC,     NUMBER },
   { "haff",            LVL_IMPL,       PC,     BINARY },
   { "hearall",         LVL_IMPL,       PC,     BINARY },
   { "\n", 0, BOTH, MISC }
  };


int perform_set(struct char_data *ch, struct char_data *vict, int mode,
		char *val_arg)
{
  int i, on = 0, off = 0, value = 0;
  room_rnum rnum;
  room_vnum rvnum;
  char output[MAX_STRING_LENGTH];
  struct clan_type *cptr = NULL;

  /* Check to make sure all the levels are correct */
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return (0);
    }
  }
  if (GET_LEVEL(ch) < set_fields[mode].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return (0);
  }

  /* Make sure the PC/NPC is correct */
  if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return (0);
  } else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return (0);
  }

  /* Find the value of the argument */
  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be 'on' or 'off'.\r\n", ch);
      return (0);
    }
    sprintf(output, "%s %s for %s.", set_fields[mode].cmd, ONOFF(on),
	    GET_NAME(vict));
  } else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
    sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
	    set_fields[mode].cmd, value);
  } else {
    strcpy(output, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  }

  switch (mode) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    set_title(vict, val_arg);
    sprintf(output, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    sprintf(output, "Nosummon %s for %s.\r\n", ONOFF(!on), GET_NAME(vict));
    break;
  case 4:
    vict->points.max_hit = RANGE(1, 2000000000);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_mana = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 6:
    vict->points.max_move = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 7:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.str = value;
    vict->real_abils.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    vict->real_abils.str_add = RANGE(0, 100);
    if (value > 0)
      vict->real_abils.str = 18;
    affect_total(vict);
    break;
  case 13:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.wis = value;
    affect_total(vict);
    break;
  case 15:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17:
    if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
      RANGE(3, 25);
    else
      RANGE(3, 18);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 18:
    vict->points.armor = RANGE(-100, 100);
    affect_total(vict);
    break;
  case 19:
    if (ch != vict && GET_LEVEL(ch) < LVL_DEITY)
    {
      send_to_char("You can only set your own gold!\r\n", ch);
      return 0;
    }
    GET_GOLD(vict) = RANGE(0, 2147483647);
    break;
  case 20:
    if (ch != vict && GET_LEVEL(ch) < LVL_DEITY)
    {
      send_to_char("You can only set your own gold!\r\n", ch);
      return 0;
    }
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    vict->points.exp = RANGE(0, 2147483647);
    break;
  case 22:
    vict->points.hitroll = RANGE(-20, 100);
    affect_total(vict);
    break;
  case 23:
    vict->points.damroll = RANGE(-20, 100);
    affect_total(vict);
    break;
  case 24:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return (0);
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return (0);
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26:
    if (ch == vict && on) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return (0);
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27:
  case 28:
    GET_PRACTICES(vict) = RANGE(-100, 1000);
    break;
  case 29:
  case 30:
  case 31:
    if (ch != vict && GET_LEVEL(ch) < LVL_DEITY)
    {
      send_to_char("You can only set your own hunger!\r\n", ch);
      return 0;
    }
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (mode - 29)) = (char) -1; /* warning: magic number here */
      sprintf(output, "%s's %s now off.", GET_NAME(vict), set_fields[mode].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (mode - 29)) = (char) value; /* and here too */
      sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
	      set_fields[mode].cmd, value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return (0);
    }
    break;
  case 32:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 33:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 34:
    if (PLR_FLAGGED(ch, PLR_HBUILDER))
    {
      if (value != 61 && value != 1)
      {
         send_to_char("You can't do that.\r\n", ch);
         return 0;
      }
      else if (PLR_FLAGGED2(vict, PLR2_REALIMM))
      {
         send_to_char("You can't do that.\r\n", ch);
         return 0;
      }
      else if (GET_LEVEL(ch) < 61 && GET_LEVEL(vict) > 1)
      {
         send_to_char("You can't do that.\r\n", ch);
         return 0;
      }
      RANGE(0, 1000);
      vict->player.level = (byte) value;
    }
    else if (GET_LEVEL(ch) == LVL_IMPL)
    {
     if ((value > GET_LEVEL(ch) || value > LVL_IMPL) && !IS_NPC(vict)) {
       send_to_char("You can't do that.\r\n", ch);
       return (0);
     }
     RANGE(0, 1000);
     vict->player.level = (byte) value;
    }
    break;
  case 35:
    if ((rnum = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return (0);
    }
    if (IN_ROOM(vict) != NOWHERE)	/* Another Eric Green special. */
      char_from_room(vict);
    char_to_room(vict, rnum);
    break;
  case 36:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 37:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 38:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 39:
    if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
      send_to_char("That is not a class.\r\n", ch);
      return (0);
    }
    GET_CLASS(vict) = i;
    break;
  case 40:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 41:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 42:
    if (ch != vict && GET_LEVEL(ch) < LVL_DEITY)
    {
      send_to_char("You can only set your own loadroom!\r\n", ch);
      return 0;
    }
    if (!str_cmp(val_arg, "off")) {
      REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    } else if (is_number(val_arg)) {
      rvnum = atoi(val_arg);
      if (real_room(rvnum) != NOWHERE) {
        SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
	GET_LOADROOM(vict) = rvnum;
	sprintf(output, "%s will enter at room #%d.", GET_NAME(vict),
		GET_LOADROOM(vict));
      } else {
	send_to_char("That room does not exist!\r\n", ch);
	return (0);
      }
    } else {
      send_to_char("Must be 'off' or a room's virtual number.\r\n", ch);
      return (0);
    }
    break;
  case 43:
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 44:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return (0);
    GET_IDNUM(vict) = value;
    break;
  case 45:
    if (GET_LEVEL(vict) >= LVL_GRGOD) {
      send_to_char("You cannot change that.\r\n", ch);
      return (0);
    }
    strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);
    *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
    sprintf(output, "Password changed to '%s'.", val_arg);
    break;
  case 46:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;

  case 47:
    if ((i = search_block(val_arg, genders, FALSE)) < 0) {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return (0);
    }
    GET_SEX(vict) = i;
    break;

  case 48:	/* set age */
    if (value < 2 || value > 1000) {	/* Arbitrary limits. */
      send_to_char("Ages 2 to 1000 accepted.\r\n", ch);
      return (0);
    }
    /*
     * NOTE: May not display the exact age specified due to the integer
     * division used elsewhere in the code.  Seems to only happen for
     * some values below the starting age (17) anyway. -gg 5/27/98
     */
    vict->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
    break;

  case 49:	/* Blame/Thank Rick Glover. :) */
    GET_HEIGHT(vict) = value;
    affect_total(vict);
    break;

  case 50:
    GET_WEIGHT(vict) = value;
    affect_total(vict);
    break;

  case 51:
    GET_OLC_ZONE(vict) = value;
    break;

  case 52:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_ASSASSIN);
    break;
  
  case 53:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_HBUILDER);
    break;

  case 54:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DEAD);
    break;

  case 55:
    GET_THIEF_LEVEL(vict) = value;
    break;

  case 56:
    GET_WARRIOR_LEVEL(vict) = value;
    break;

  case 57:
    GET_MAGE_LEVEL(vict) = value;
    break;

  case 58:
    GET_CLERIC_LEVEL(vict) = value;
    break;

  case 59:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MULTI);
    break;

  case 60:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MULTII);
    break; 

  case 61:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MULTIII);
    break; 

 case 62:
    ROMANCE(vict) = value;
   if (value == 0) {
    free(PARTNER(vict));
    PARTNER(vict) = NULL;
    send_to_char("Player set to single, partner NULLED\r\n", ch);
    }
    break;

 case 63:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ANON);
    break;

 case 64:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_WHOINVIS);
    break;

 case 65:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FEDHORSE);
    break;

 case 66:
    SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_REVIEW);
    break;

  case 67:
    set_pretitle(vict, val_arg);
    sprintf(output, "%s's pretitle is now: %s", GET_NAME(vict), GET_PRETITLE(vict));
    break;

 case 68:
    GET_HORSELEVEL(vict) = RANGE(0, 10);
    break;

 case 69:
    GET_HORSEEXP(vict) = RANGE(0, 100000000);
    break;

 case 70:
    GET_HORSEEQ(vict) = RANGE(0, 5);
    break;

 case 71:
    GET_SPARRANK(vict) = RANGE(1, 1000);
    break;

 case 72:
    SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_WRITER);
    break;
 
 case 73:
    GET_KILLS(vict) = RANGE(1,2000000000);
    break;

 case 74:
    GET_DEATHS(vict) = RANGE(1,2000000000);
    break;

 case 75:
    GET_GM_LEVEL(vict) = RANGE(1, MAX_GM_LEVEL);
    break;

 case 76:
    set_lname(vict, val_arg);
    sprintf(output, "%s's title is now: %s", GET_NAME(vict), GET_LNAME(vict));
    break;

 case 77:
    IS_LORD(vict) = !IS_LORD(vict);
    sprintf(output, "%s's lordship is %s.\r\n", GET_NAME(vict),
         IS_LORD(vict) ? "On" : "Off");
    send_to_char(output, ch);
    break;

 case 78:
    GET_ECONRANK(vict) = RANGE(1,2000000000);
    break;

 case 79:
    GET_SACRIFICE(vict) = RANGE(1,2000000000);
    break;

 case 80:
    GET_RELIGION(vict) = RANGE(0, 4);
    break;
 case 81:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOSHOUT);
    break;
 case 82:
    SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_REALIMM);
    break;
 case 83:
    SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_IPMASK);
    break;
 case 84:
    SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_CANTRIGEDIT);
    break;
 case 85:
    for (cptr = clan_info; cptr && cptr->number != value; cptr = cptr->next);

    if (cptr == NULL || cptr->number != value)
    { 
       send_to_char("&RThat's not a valid clan!\r\n", ch);
       return FALSE;
    }

    GET_CLAN(vict) = RANGE(0, 1000);
    break;
  case 86:
      vict->char_specials.timer = RANGE(0, 100);
    break;
  case 87:
      SET_OR_REMOVE(PLR_FLAGS2(vict), PLR2_HAFF);
    break;
  case 88:
      SET_OR_REMOVE(PRF_FLAGS2(vict), PRF2_HEARALLTELL);
    break;

  default:
    send_to_char("Can't set that!\r\n", ch);
    return (0);
  }

  strcat(output, "\r\n");
  send_to_char(CAP(output), ch);
  return (1);
}


ACMD(do_set)
{
  struct char_data *vict = NULL, *cbuf = NULL;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
	val_arg[MAX_INPUT_LENGTH];
  int mode, len, player_i = 0, retval;
  char is_file = 0, is_player = 0;

  half_chop(argument, name, buf);

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob"))
    half_chop(buf, name, buf);

  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) {
    send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
    return;
  }

  /* find the target */
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, FIND_CHAR_WORLD))) {
	send_to_char("There is no such player.\r\n", ch);
	return;
      }
    } else { /* is_mob */
      if (!(vict = get_char_vis(ch, name, FIND_CHAR_WORLD))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  } else if (is_file) {
    /* try to load the player off disk */
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    CREATE(cbuf->player_specials, struct player_special_data, 1);
    if ((player_i = load_char(name, cbuf)) > -1) {  
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }

  /* find the command in the list */
  len = strlen(field);
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, len))
      break;

  /* perform the set */
  retval = perform_set(ch, vict, mode, val_arg);

  /* save the character if a change was made */
  if (retval) {
    if (!is_file && !IS_NPC(vict))
      save_char(vict, NOWHERE);
    if (is_file) {
      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf, GET_LOADROOM(cbuf));
      send_to_char("Saved in file.\r\n", ch);
    }
  }

  /* free the memory if we allocated it earlier */
  if (is_file)
    free_char(cbuf);
}

void out_rent(char *name) {
  FILE *fl,*fp;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH],fname2[MAX_INPUT_LENGTH];
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  if (!get_filename(name, fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    return;
  }

  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if (!get_filename(name, fname2, NEW_OBJ_FILES)) {
    log("SYSERR: Unable to complete conversion - unable to get new object filename.\r\n");
    return;
  }
  if (!(fp = fopen(fname2, "w+"))) {
    log("SYSERR: Unable to open new object file.\r\n");
    return;
  }

  fprintf(fp,"%d %d %d %d %d %d\r\n",rent.rentcode,rent.time,
       rent.net_cost_per_diem,rent.gold,rent.account,rent.nitems);
  /* for rent code */

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);

    if (ferror(fl) || ferror(fp)) {
      fclose(fl);
      fclose(fp);
      log("SYSERR: Error in conversion of rent files.");
      return;
    }

    if (!feof(fl)) {
      if (real_object(object.item_number) > 0) {
       /* none of these will be unique items. just can't happen */
          obj = read_object(object.item_number, VIRTUAL);
          my_obj_save_to_disk(fp, obj,0);
          extract_obj(obj);
      }
    }
  }
  fclose(fl);

  /* write final line - this is never actually read.. but hey! */
  fprintf(fp, "\n");
  fclose(fp);
}

/* Xap - To keep old rent files (binary) and not lose all objects, you'll
   have to convert to ascii files.  Guess what?  This doesn't work as well
   as you'd think.  Well, It works, but here's some limitations:
    1. Sucks up memory. I don't know why. Maybe its specific to my mud,
        but I'm pretty sure I'm freeing all that needs it.
    2. It takes time.  You'll notice I use the process output command
        a few times.  This is because with something like 2500 rent files,
        it can take up to 2 or 3 minutes.
                I guess, use at your own risk. */

ACMD(do_objconv) {
  int counter;
  float percent;
  struct char_data *victim;
  extern struct player_index_element *player_table;
  int flag=0;
  int process_output(struct descriptor_data *t);
  struct descriptor_data *d;


  if(GET_LEVEL(ch) != LVL_IMPL && !IS_NPC(ch)) {
    send_to_char("You may not.\r\n",ch);
    return;
  }

  send_to_all("Please hold on - object conversion taking place.\r\n");

  for (d = descriptor_list; d; d = d->next) {
    process_output(d);
  }

  /* okay, this is where we load every char, apparently.. but we'll
     do it one at a time, thank you very much */
  for (counter = 0; counter <= top_of_p_table; counter++) {

   /* individual load of characters so that way you may use this
      opportunity to use the rest of the character information..
	(like deleted, etc.. to choose if you want) */

    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    if (load_char((player_table + counter)->name, victim) > -1)
     save_char(victim, NOWHERE);
    out_rent(GET_NAME(victim));
    free_char(victim);
    percent = (float)((float) counter/(float) top_of_p_table);
    if(percent > .25 && flag==0) {
      send_to_char("25%...",ch);
      flag=1;
      process_output(ch->desc);
    } else if(percent > .50 && flag==1) {
      send_to_char("50%...",ch);
      flag=2;
      process_output(ch->desc);
    } else if(percent > .75 && flag==2) {
      send_to_char("75%...",ch);
      flag=3;
      process_output(ch->desc);
    } else if(percent > .90 && flag==3) {
      send_to_char("90%...",ch); 
      flag=4;
      process_output(ch->desc);
    }
  }
  send_to_char("100%. Done.\r\n",ch);
}

ACMD(do_deathtoall)
{
  struct char_data *i;
  struct obj_data *k;
  mob_vnum number;
  mob_rnum r_num;
  char type[10];
  int num = 0, found = 0;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: deathtoall { obj | mob } \r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A negative number?\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
  {
    if ((r_num = real_mobile(number)) < 0)
    {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && (GET_MOB_VNUM(i) == number))
      {
        found = 1;
        strcpy(type, "mobs");
        extract_char(i);
      }
  }
  else if (is_abbrev(buf, "obj"))
  {
    if ((r_num = real_object(number)) < 0)
    {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && (GET_OBJ_VNUM(k) == number))
      {
        found = 1;
        strcpy(type, "objs");
        extract_obj(k);
      }
  }
  if (!found)
  {
    send_to_char("Couldn't find any such thing.\r\n", ch);
    return;
  }
  act("$n gathers massive destructive energies around $s body...", FALSE, ch, 0, 0, TO_ROOM);
  act("$e releases them in a wild blast to all the corners of the world!", FALSE, ch, 0, 0, TO_ROOM);
  send_to_char("You release massive amounts of cleansing energies into the world!\r\n", ch);
  sprintf(buf1, "(GC) %s has eliminated the world of all %s with vnum #%d.", GET_NAME(ch), type, number);
  mudlog(buf1, BRF, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
}


ACMD(do_copyto)
{

/* Only works if you have Oasis OLC */
extern void olc_add_to_save_list(int zone, byte type);

  char buf2[10];
  char buf[80];
  int iroom = 0, rroom = 0;
  struct room_data *room;

  one_argument(argument, buf2);
  /* buf2 is room to copy to */

  CREATE (room, struct room_data, 1);
  iroom = atoi(buf2);
  rroom = real_room(atoi(buf2));
  *room = world[rroom];

 if (!*buf2) {
    send_to_char("Format: copyto <room number>\r\n", ch);
    return; }

 if (rroom <= 0) {
  sprintf(buf, "There is no room with the number %d.\r\n", iroom);
  send_to_char(buf, ch);
  return;
  }

 /* if (GET_LEVEL(ch) < LVL_IMPL && zone_table[room->zone].number != ch->player_specials->saved.olc_zone){
  send_to_char("You are not allowed to access that room to copy to it!", ch);
  return;
  } 
 */

 if (GET_LEVEL(ch) < LVL_IMPL && (!is_name(GET_NAME(ch), zone_table[room->zone].builders))) {
  send_to_char("You are not allowed to access that room to copy to it!", ch);
  return;
  } 

/* Main stuff */

  if (world[ch->in_room].description) {
    world[rroom].description = str_dup(world[ch->in_room].description);

/* Only works if you have Oasis OLC */
  add_to_save_list((iroom/100), SL_WLD);

 sprintf(buf, "You copy the description to room %d.\r\n", iroom);
 send_to_char(buf, ch);
 }

 else
   send_to_char("This room has no description!\r\n", ch);

}

ACMD(do_dig)
{
/* Only works if you have Oasis OLC */

  char buf2[10];
  char buf3[10];
  char buf[80];
  int iroom = 0, rroom = 0;
  int dir = 0;
  struct room_data *room;

  two_arguments(argument, buf2, buf3);
  /* buf2 is the direction, buf3 is the room */

  CREATE (room, struct room_data, 1);
  iroom = atoi(buf3);
  rroom = real_room(iroom);
  *room = world[rroom];

 if (!*buf2) {
    send_to_char("Format: dig <dir> <room number>\r\n", ch);
    return; }
 else if (!*buf3) {
    send_to_char("Format: dig <dir> <room number>\r\n", ch);
    return; }
 if (rroom <= 0) {
    sprintf(buf, "There is no room with the number %d", iroom);
    send_to_char(buf, ch);
    return;
   }

 if (GET_LEVEL(ch) < LVL_IMPL && !is_name(GET_NAME(ch), zone_table[room->zone].builders)) {
    send_to_char("You cannot dig to a room outside your zone!", ch);
    return;
   }

/* Main stuff */
    switch (*buf2) {
    case 'n':
    case 'N':
      dir = NORTH;
      break;
    case 'e':
    case 'E':
      dir = EAST;
      break;
    case 's':
    case 'S':
      dir = SOUTH;
      break;
    case 'w':
    case 'W':
      dir = WEST;
      break;
    case 'u':
    case 'U':
      dir = UP;
      break;
    case 'd':
    case 'D':
      dir = DOWN;
      break; }

  CREATE(world[rroom].dir_option[rev_dir[dir]], struct room_direction_data,1);
  world[rroom].dir_option[rev_dir[dir]]->general_description = NULL;
  world[rroom].dir_option[rev_dir[dir]]->keyword = NULL;
  world[rroom].dir_option[rev_dir[dir]]->to_room = ch->in_room;

  CREATE(world[ch->in_room].dir_option[dir], struct room_direction_data,1);
  world[ch->in_room].dir_option[dir]->general_description = NULL;
  world[ch->in_room].dir_option[dir]->keyword = NULL;
  world[ch->in_room].dir_option[dir]->to_room = rroom;

 /* Only works if you have Oasis OLC */
  add_to_save_list((iroom/100), SL_WLD);

  sprintf(buf, "You make an exit %s to room %d.\r\n", buf2, iroom);
  send_to_char(buf, ch);
}


ACMD(do_saveall) {
int can_edit_zone(struct char_data *ch, int number);
int x, j = 0, zonenum = 0;

  for (x = 0;x <= top_of_zone_table;x++)
    if ((!zonenum) && (GET_LEVEL(ch) >= LVL_IMPL)) {
      if (!j) {
        sprintf(buf, "Saved mob, room, obj, zone, and shop info for zone%s",
          (zonenum == 0 ? "s:\r\n" : ": "));
        send_to_char(buf, ch);
      }
      j++;
      medit_save_to_disk(x);
      redit_save_to_disk(x);
      oedit_save_to_disk(x);
      zedit_save_to_disk(x);
      sedit_save_to_disk(x);

      if (!zone_table[x].name)  /* this should never happen, but try to catch
everything */
        zone_table[x].name = str_dup("New Zone");

      sprintf(buf, "[%3d] %s&n\r\n", zone_table[x].number, zone_table[x].name);
      send_to_char(buf, ch);
    }

  if (!j) {
    if (!zonenum)
      send_to_char("You have to have some zones to save first.\r\n", ch);
    else
      send_to_char("The zone number must exist, and you must be able to edit it.\r\n", ch);
    return;
  }
  sprintf(buf, "\r\nYou saved info for %d zone%s.\r\n", j, (j == 1 ? "" : "s"));
  send_to_char(buf, ch);
  sprintf(buf, "%s saved info for %d zone%s.", GET_NAME(ch), j, (j == 1
? "" : "s"));
  mudlog(buf, CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
}

ACMD(do_understand) {

  struct char_data *tch;
  char tmp[256];
  int count = 0;

  send_to_char("Lvl  Class  Room   Name                "
               "Health  Position\r\n", ch);
  for (tch = character_list; tch; tch = tch->next)
    if (!IS_NPC(tch) && GET_LEVEL(tch) < LVL_IMMORT) {
      sprintf(tmp, "%2d  %4s    %-5d  %-20s  %3d%%  %s\r\n",
              GET_LEVEL(tch), CLASS_ABBR(tch), world[IN_ROOM(tch)].number,
              GET_NAME(tch), 
              (int)(100. * (double)GET_HIT(tch)/ GET_MAX_HIT(tch)),
              position_types[(int)GET_POS(tch)]);
      send_to_char(tmp, ch);
      count++;
    }
  if (!count)
    send_to_char("  No mortals are on!\r\n", ch);

}

ACMD(do_haffinate)
{
  struct char_data *vict, *d, *next_ch;
  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to make as stupid as Haff?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
    send_to_char(NOPERSON, ch);
  else {
    SET_BIT(PLR_FLAGS2(vict), PLR2_HAFF);
    SET_BIT(PLR_FLAGS(vict), PLR_NOTITLE);
 
    GET_ALIGNMENT(vict) = -666;      
    GET_TITLE(vict) = "is a HUGE dumbass.";
  
    update_pos(vict);
    send_to_char(OK, ch);
    act("&RThou art has been HAFFINATED!&n", FALSE, vict, 0, ch, TO_CHAR);
    act("&G$N has been HAFFINATED by $n.&n\r\n",0,ch,0,vict,TO_WORLD);

    for (d = character_list; d ; d = next_ch)
    {
      next_ch = d->next;
      if (!IS_MOB(d) || !MOB_FLAGGED(d, MOB_MEMORY) || !MEMORY(d)) 
         continue;
      remember(d, vict);
    }

  }
}

ACMD(do_zdelete)
{
  FILE *index2;
  zone_vnum i;
  zone_vnum j;
  int real = 0;

  one_argument(argument, buf);

  if (!*buf) {
   send_to_char("Well you need to give the zone number to delete!", ch);
   return;
  }
  j = atoi(buf);
  for (i = 0; i <= top_of_zone_table; i++)
   if (zone_table[i].number == j) {
    real = TRUE;
    break;
   }

  if (!real) {
   send_to_char("No such zone.\r\n", ch);
   return;
  }

  sprintf(buf2, "%s%s", WLD_PREFIX, INDEX_FILE);
  if (!(index2 = fopen(buf2, "w"))) {
    log("SYSERR: Error opening an index file in zdelete");
    send_to_char("Error. Please Delete zone manually then reboot.\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_zone_table; i++) {
   if (zone_table[i].number != j)
    fprintf(index2, "%d.wld\n", zone_table[i].number);
  }
  fprintf(index2, "$\n");
  fclose(index2);

  sprintf(buf2, "%s%s", MOB_PREFIX, INDEX_FILE);
  if (!(index2 = fopen(buf2, "w"))) {
    log("SYSERR: Error opening an index file in zdelete");
    send_to_char("Error. Please Delete zone manually then reboot.\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_zone_table; i++) {
   if (zone_table[i].number != j)
    fprintf(index2, "%d.mob\n", zone_table[i].number);
  }
  fprintf(index2, "$\n");
  fclose(index2);

  sprintf(buf2, "%s%s", OBJ_PREFIX, INDEX_FILE);
  if (!(index2 = fopen(buf2, "w"))) {
    log("SYSERR: Error opening an index file in zdelete");
    send_to_char("Error. Please Delete zone manually then reboot.\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_zone_table; i++) {
   if (zone_table[i].number != j)
    fprintf(index2, "%d.obj\n", zone_table[i].number);
  }
  fprintf(index2, "$\n");
  fclose(index2);

  sprintf(buf2, "%s%s", ZON_PREFIX, INDEX_FILE);
  if (!(index2 = fopen(buf2, "w"))) {
    log("SYSERR: Error opening an index file in zdelete");
    send_to_char("Error. Please Delete zone manually then reboot.\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_zone_table; i++) {
   if (zone_table[i].number != j)
    fprintf(index2, "%d.zon\n", zone_table[i].number);
  }
  fprintf(index2, "$\n");
  fclose(index2);

  sprintf(buf2, "%s%s", SHP_PREFIX, INDEX_FILE);
  if (!(index2 = fopen(buf2, "w"))) {
    log("SYSERR: Error opening an index file in zdelete");
    send_to_char("Error. Please Delete zone manually then reboot.\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_zone_table; i++) {
   if ((zone_table[i].number != j) && zdelete_check(zone_table[i].number))
    fprintf(index2, "%d.shp\n", zone_table[i].number);
  }
  fprintf(index2, "$\n");
  fclose(index2);

  log("(GC) Zone Delete #%d successful now rebooting.", j);
  send_to_char("Zone Delete Successful.\r\n", ch);
  send_to_all("Rebooting.. come back in a minute or two.\r\n");
  do_copyover(ch, "", 0, 0);
}

ACMD(do_pyoink)
{
  struct char_data *victim;
  struct obj_data *obj;
  char buf2[80];
  char buf3[80];
  int i, k = 0;

  two_arguments(argument, buf2, buf3);

  if (!*buf2)
    send_to_char("Syntax: pyoink <object> <character>.\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf3, FIND_CHAR_ROOM)))
    send_to_char("No one by that name here.\r\n", ch);
  else if (victim == ch)
    send_to_char("Are you sure you're feeling ok?\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch) && !IS_NPC(victim))
    send_to_char("That's really not such a good idea.\r\n", ch);
  else if (!*buf3)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else {
    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
         isname(buf2, GET_EQ(victim, i)->name)) {
        obj_to_char(unequip_char(victim, i), victim);
        k = 1;
      }
    }

  if (!(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
    if (!k && !(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
      sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim), buf2);
      send_to_char(buf, ch);
      return;
    }
  }

  act("&n$n makes a magical gesture and $p&n flies from $N to $m.",FALSE,ch,obj,
       victim,TO_NOTVICT);
  act("&n$n makes a magical gesture and $p&n flies away from you to $m.",FALSE,ch,
       obj,victim,TO_VICT);
  act("&nYou make a magical gesture and $p&n flies away from $N to you.",FALSE,ch,
       obj, victim,TO_CHAR);

  obj_from_char(obj);
  obj_to_char(obj, ch);
  save_char(ch, NOWHERE);
  save_char(victim, NOWHERE);
  }
}

ACMD(do_xname)
{
   char tempname[MAX_INPUT_LENGTH+1];
   int i = 0;
   FILE *fp;
   *buf = '\0';

   one_argument(argument, buf);
   
   if(!*buf) {
      send_to_char("Xname which name?\r\n", ch);
      return;
   }
      
   if (strlen(buf) >= MAX_INPUT_LENGTH) {
     send_to_char("Too long name for xname!\r\n", ch);
     return;
   }
      
   if(!(fp = fopen(XNAME_FILE, "a"))) {
      perror("Problems opening xname file for do_xname");
      return;
   }
   
   strcpy(tempname, buf);
   for (i = 0; tempname[i]; i++)
      tempname[i] = LOWER(tempname[i]);
   fprintf(fp, "%s\n", tempname);
   fclose(fp);
   sprintf(buf1, "%s has been xnamed!\r\n", tempname);
   send_to_char(buf1, ch);
   Read_Invalid_List();
}

typedef const char *(*verify_func)(int);

#define VALID_EXIT(x, y) (world[(x)].dir_option[(y)] && \
                          (TOROOM(x, y) != NOWHERE))
#define IS_DOOR(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_ISDOOR))
#define IS_CLOSED(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))
#define MAX_REPORT_ERR		50

const char *check_exits(int rnum)
{
  static char retbuf[MAX_STRING_LENGTH];
  int i;

  if (rnum == NOWHERE) return NULL;

  retbuf[0] = 0;

  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (VALID_EXIT(rnum, i)) {
      if (TOROOM(TOROOM(rnum, i), rev_dir[i]) != rnum) {
        sprintf(retbuf + strlen(retbuf), "Room %d: going %s then %s doesn't return to starting room\r\n",
                world[rnum].number, dirs[i], dirs[rev_dir[i]]);
      }
      if (TOROOM(rnum, i) == NOWHERE) {
        sprintf(retbuf + strlen(retbuf), "Room %d: door to the %s leads nowhere\r\n",
                world[rnum].number, dirs[i]);
      }
      if (IS_DOOR(rnum, i) && (!VALID_EXIT(TOROOM(rnum, i), rev_dir[i]) || !IS_DOOR(TOROOM(rnum, i), rev_dir[i]))) {
        sprintf(retbuf + strlen(retbuf), "Room %d: door to the %s doesn't have a door on the other side\r\n",
                world[rnum].number, dirs[i]);
      }
      if (IS_CLOSED(rnum, i) && (!VALID_EXIT(TOROOM(rnum, i), rev_dir[i]) || !IS_CLOSED(TOROOM(rnum, i), rev_dir[i]))) {
        sprintf(retbuf + strlen(retbuf), "Room %d: door to the %s is closed, but other side isn't\r\n",
                world[rnum].number, dirs[i]);
      }
    }
  }
  if (retbuf[0])
    return retbuf;
  else
    return NULL;
}

ACMD(do_verify)
{
  int a = -1, b = -1, i, vfn;
  verify_func vfs[] = {check_exits, NULL};
  const char *s;
  int cnt;

  sscanf(argument, "%d %d", &a, &b);

  buf[0] = 0;

  if (a < 0 || b < 0 || a == b) {
    send_to_char("Usage: verify <first room> <last room>\r\n", ch);
    return;
  }
  cnt = 0;
  for (vfn = 0; vfs[vfn]; vfn++) {
    for (i = a; i <= b; i++) {
      s = vfs[vfn](real_room(i));
      if (s && (cnt > MAX_REPORT_ERR)) break;
      
      if (s) {
        strcat(buf, s);
        cnt++;
      }
    }
  }

  if (buf[0]) {
    if (cnt > MAX_REPORT_ERR)
      strcat(buf, "&ROVERFLOW.&w\r\n");
    page_string(ch->desc, buf, 1);
  } else
    send_to_char("No errors found.\r\n", ch);
}

ACMD(do_roomlink)         /* Shows all exits to a given room */
{
  int door, i, room_num;

  one_argument(argument, buf);
  
  if (!buf || !*buf)
    room_num = ch->in_room;
  else
    room_num = real_room(atoi(buf));

  if (room_num == NOWHERE) {
    send_to_char("There is no room with that number.\r\n", ch);
    return;
  } else {
    for (i = 0; i <= top_of_world; i++) {
      for (door = 0; door < NUM_OF_DIRS; door++) {
        if (world[i].dir_option[door] &&
	    world[i].dir_option[door]->to_room == room_num) {
          sprintf(buf2, "Exit %s from room %d.\r\n", dirs[door], 
		world[i].number);
          send_to_char(buf2, ch);
        }
      }
    }
  }
}



  ACMD(do_undig)
  {
    char buf2[10];
    char buf[80];
    int dir = 0;
    struct room_direction_data *exit1 = NULL, *exit2 = NULL;
    room_rnum toroom = 0;

    /* buf2 is the direction */
    one_argument(argument, buf2);

    if (!*buf2) {
      send_to_char("Format: unmake \r\n", ch);
      return;
    }

    if(!strcmp(buf2, "n"))
      dir = NORTH;
    else if(!strcmp(buf2, "e"))
      dir = EAST;
    else if(!strcmp(buf2, "s"))
      dir = SOUTH;
    else if(!strcmp(buf2, "w"))
      dir = WEST;
    else if(!strcmp(buf2, "u"))
      dir = UP;
    else if(!strcmp(buf2, "d"))
      dir = DOWN;
    else
    {
      send_to_char("What direction? (n s e w u d)\r\n", ch);
      return;
    }

      exit1 = world[ch->in_room].dir_option[dir];
      if (exit1)
      {
        toroom = exit1->to_room;
        exit2 = world[toroom].dir_option[rev_dir[dir]];
      }

      if (exit1)
      {
        if (exit1->general_description)
          free(exit1->general_description);
        if (exit1->keyword)
          free(exit1->keyword);
        free(exit1);
        world[ch->in_room].dir_option[dir] = NULL;
      }
      else
      {
        send_to_char("There is no exit in that direction!", ch);
        return;
      }
      if (exit2)
      {
        if (exit2->general_description)
          free(exit2->general_description);
        if (exit2->keyword)
          free(exit2->keyword);
        free(exit2);
        world[toroom].dir_option[rev_dir[dir]] = NULL;
      }
      else
      {
        send_to_char("No returning exit from the other side, only one side deleted!\r\n", ch);
      }

    /* Only works if you have Oasis OLC */
    add_to_save_list((GET_ROOM_VNUM(toroom) / 100), SL_WLD);
    add_to_save_list((IN_ROOM_VNUM(ch) / 100), SL_WLD);

    sprintf(buf, "You purge the exit to the %s.\r\n", dirs[dir]);
    send_to_char(buf, ch);
    sprintf(buf1, "%s gestures to the %s...what once was an exit has now vanished.", GET_NAME(ch), dirs[dir]);
    act(buf1, FALSE, ch, 0, 0, TO_ROOM);
  }

ACMD(do_zsave) {
  int can_edit_zone(struct char_data *ch, int number);
  int x, j = 0, zonenum = 0;

    one_argument(argument, arg);

    if ((arg && *arg) && (!(zonenum = atoi(arg)))) {
      send_to_char("Usage: zsave [zone number]\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) >= LVL_CREATOR && !zonenum) {
      hedit_save_to_disk();
      send_to_char("Saved help.\r\n", ch);
    }

    for (x = 0;x <= top_of_zone_table;x++) {
     if (zonenum == zone_table[x].number) {
           if (can_edit_zone(ch, x)) {

        j++;
	save_mobiles(real_zone(zonenum));
	save_objects(real_zone(zonenum));
	save_rooms(real_zone(zonenum));
	save_shops(real_zone(zonenum));
	save_zone(real_zone(zonenum));
        if (!zone_table[x].name)  /* this should never happen, but try to catch everything */
          zone_table[x].name = str_dup("New Zone");
        sprintf(buf, "[%3d] %s&n\r\n", zone_table[x].number, zone_table[x].name);
        send_to_char(buf, ch);
      }
    }
  }

    if (!j) {
      if (!zonenum)
        send_to_char("You have to have some zones to save first.\r\n", ch);
      else
        send_to_char("The zone number must exist, and you must be able to edit it.\r\n", ch);
      return;
    }

    sprintf(buf, "\r\nYou saved info for %d zone%s.\r\n", j, (j == 1 ? "" : "s"));
    send_to_char(buf, ch);
    sprintf(buf, "%s saved info for %d zone%s.", GET_NAME(ch), j, (j == 1 ? "" : "s"));
    mudlog(buf, CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
  }


void perform_assassin_vis(struct char_data *ch)
{
  if (GET_INVIS_LEV(ch) == 0 && !PRF_FLAGGED(ch, PRF_WHOINVIS)) {
    send_to_char("You are already fully visible.\r\n", ch);
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}


void perform_assassin_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
  }

  sprintf(buf, "You are now invisible on the who list.\r\n");
  send_to_char(buf, ch);
 }
  

ACMD(do_whoinvis)
{

   if (PLR_FLAGGED(ch, PLR_ASSASSIN)) {
      if (PRF_FLAGGED(ch, PRF_WHOINVIS)) {
      REMOVE_BIT(PRF_FLAGS(ch), PRF_WHOINVIS);
      send_to_char("You are now visible on the who list.\r\n", ch);
      return;
      }  else 
      SET_BIT(PRF_FLAGS(ch), PRF_WHOINVIS);
      send_to_char("You are now invisible on the who list.\r\n", ch);
        
   /*if (GET_INVIS_LEV(ch) > 0)
      perform_assassin_vis(ch);
    else
      perform_assassin_invis(ch, GET_LEVEL(ch));*/
   }
   else { 
   send_to_char("Only assassin's can do that!\r\n", ch);
  }
}

ACMD(do_depiss)
  {
    void forget(struct char_data * ch, struct char_data * victim);
    char buf3[80];
    struct char_data *vic = 0;
    struct char_data *mob = 0;
    two_arguments(argument, buf1, buf2);
    if (!*buf1 || !*buf2) {
      send_to_char("Usage: depiss <player> <mobile>\r\n", ch);
      return;
    }
    if (((vic = get_char_vis(ch, buf1, FIND_CHAR_WORLD))) && (!IS_NPC(vic))) {
      if (((mob = get_char_vis(ch, buf2, FIND_CHAR_WORLD))) && (IS_NPC(mob))) {
        if (MOB_FLAGGED(mob, MOB_MEMORY))
          forget(mob, vic);
        else {
          send_to_char("Mobile does not have the memory flag set!\r\n", ch);
          return;
        }
      }
      else {
        send_to_char("Sorry, Player Not Found!\r\n", ch);
        return;
      }
    }
    else {
      send_to_char("Sorry, Mobile Not Found!\r\n", ch);
      return;
    }
    sprintf(buf3, "%s has been removed from %s pissed list.\r\n",
            GET_NAME(vic), GET_NAME(mob));
    send_to_char(buf3, ch);
  }



  ACMD(do_repiss)
  {
    void remember(struct char_data * ch, struct char_data * victim);
    char buf3[80];
    struct char_data *vic = 0;
    struct char_data *mob = 0;
    two_arguments(argument, buf1, buf2);
    if (!*buf1 || !*buf2) {
      send_to_char("Usage: repiss <player> <mobile>\r\n", ch);
      return;
    }
    if (((vic = get_char_vis(ch, buf1, FIND_CHAR_WORLD))) && (!IS_NPC(vic))) {
      if (((mob = get_char_vis(ch, buf2, FIND_CHAR_WORLD))) && (IS_NPC(mob))) {
        if (MOB_FLAGGED(mob, MOB_MEMORY))
          remember(mob, vic);
        else {
          send_to_char("Mobile does not have the memory flag set!\r\n", ch);
          return;
        }
      }
      else {
        send_to_char("Sorry, Player Not Found!\r\n", ch);
        return;
      }
    }
    else {
      send_to_char("Sorry, Mobile Not Found!\r\n", ch);
      return;
    }
    sprintf(buf3, "%s has been added to %s pissed list.\r\n",
            GET_NAME(vic), GET_NAME(mob));
    send_to_char(buf3, ch);
  }

ACMD(do_vlist)
{
  int found = 0, choice = 0, nr = 0;
  zone_rnum zone;
  zone_vnum j;
  
  two_arguments(argument, buf, buf2);

        if (is_abbrev(buf, "objects"))
                choice = 1;
        if (is_abbrev(buf, "mobiles"))
                choice = 2;
        if (is_abbrev(buf, "rooms"))
                choice = 3;

        if (!*buf || !*buf2 || !is_number(buf2) || !choice) {
                 send_to_char("Usage: vlist { obj | mob } <zone number>\r\n", ch);
                return;
         }


    j = atoi(buf2);
    
        for (zone = 0; zone <= top_of_zone_table; zone++)
      if (zone_table[zone].number == j)
          {
                j = zone;
                break;
          }

        if (j > top_of_zone_table)
        {
                send_to_char("That is not a vlid zone number..", ch);
                return;
        }

        sprintf(buf, "Zone: %s\r\n", zone_table[j].name);
        send_to_char(buf, ch);
        
        switch (choice)
        {
        case 1:
                {
                        for (nr = 0; nr <= top_of_objt; nr++) {
                          if (obj_index[nr].vnum <= zone_table[j].top
                                  && obj_index[nr].vnum >= (zone_table[j].number * 100)) {

                                sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                                obj_index[nr].vnum, obj_proto[nr].short_description);
                                send_to_char(buf, ch);

                                }
                        }
                        
                        break;
                }
        case 2:
                {
                        for (nr = 0; nr <= top_of_mobt; nr++) {
                          if (mob_index[nr].vnum <= zone_table[j].top
                                  && mob_index[nr].vnum >= (zone_table[j].number * 100)) {

                                sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                                mob_index[nr].vnum, GET_NAME(&mob_proto[nr]));
                                send_to_char(buf, ch);

                                }
                        }
                        
                        break;
                }
        case 3:
                {
                        for (nr = 0; nr <= top_of_world; nr++) {
                          if (world[nr].zone == j) {

                                sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                                world[nr].number, world[nr].name);
                                send_to_char(buf, ch);

                                }
                        }
                        
                        break;
                }
        default:
                {
                        send_to_char("Come again?! :P", ch);
                        return;
                }
        }
}

ACMD(do_statlist)
{
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        int aff = 0, pos = -1, nr, count = 0, object, size = 0;

        *buf = '\0';

        two_arguments(argument, arg1, arg2);

        if(!*arg1)
        {
                send_to_char("You must specify which affect you want. Options are:", ch);
                return;
        }
        
        for(aff = 0; *apply_types[aff] != '\n'; aff++)
                if(is_abbrev(arg1, apply_types[aff]))
                        break;

        if(*apply_types[aff] == '\n')
        {
                send_to_char("Unrecognized apply type, options are:", ch);
                return;
        }
        
        if(*arg2)
        {
                for(pos = 0; *wear_bits[pos] != '\n'; pos++)
                        if(is_abbrev(arg2, wear_bits[pos]))
                                break;  

                if(*wear_bits[pos] == '\n')
                {
                        send_to_char("Unrecognized position, options are:", ch);
                        return;
                }
        }

        for(object = 0; object <= top_of_objt; object++)
        {
                
                if(pos != -1 && !CAN_WEAR(&obj_proto[object], 1 << pos))
                        continue;               
                
                for(nr = 0; nr < MAX_OBJ_AFFECT; nr++)
                {
                        if(obj_proto[object].affected[nr].location == aff)
                        {
                                count++;
                                sprintf(buf, "[%d] %s - ", obj_index[object].vnum, obj_proto[object].short_description);
                                sprintbit(obj_proto[object].obj_flags.wear_flags, wear_bits, buf + strlen(buf));
                                sprintf(buf + strlen(buf), "- ");

                                size = obj_proto[object].affected[nr].modifier;

                                if(size < 0)
                                        sprintf(buf + strlen(buf), "&b%d&n\r\n", size);
                                else
                                {
                                        switch(size)
                                        {
                                                case 0:
                                                        sprintf(buf + strlen(buf), "&r0&n\r\n");
                                                        break;
                                                case 1:
                                                        sprintf(buf + strlen(buf), "&y1&n\r\n");
                                                        break;
                                                case 2:
                                                        sprintf(buf + strlen(buf), "&g2&n\r\n");
                                                        break;
                                                case 3:
                                                        sprintf(buf + strlen(buf), "&G3&n\r\n");
                                                        break;
                                                case 4:
                                                        sprintf(buf + strlen(buf), "&C4&n\r\n");
                                                        break;
                                                default:
                                                        sprintf(buf + strlen(buf), "&W%d&n\r\n", size);
                                                        break;
                                        }                                       
                                
                                }

                                send_to_char(buf, ch);
                        }

                }
        }

        if(!count)
                send_to_char("No items that affect that stat.\r\n", ch);
}               

ACMD(do_peace)
  {
        struct char_data *vict, *next_v;
        act ("$n decides that everyone should just be friends.",
                FALSE,ch,0,0,TO_ROOM);
        send_to_room("Everything is quite peaceful now.\r\n",ch->in_room);
        for(vict=world[ch->in_room].people;vict;vict=next_v)
        {
                next_v=vict->next_in_room;
              if (FIGHTING(vict))
                {
                if(FIGHTING(FIGHTING(vict))==vict)
                        stop_fighting(FIGHTING(vict));
                stop_fighting(vict);

                }
        }
  }

ACMD(do_lag)
{
struct char_data *vict;

char *name = arg, *timetolag = buf2;

 two_arguments(argument, name, timetolag);

  if (!*timetolag || !*name)
  {
   send_to_char("usage: lag target length_in_seconds\r\n", ch);
   return;
  }


vict = get_char_vis(ch, name, FIND_CHAR_WORLD);

if (vict == 0)
{
send_to_char("Cannot find your target!\r\n", ch);
return;
}

if (IS_NPC(vict))
{
  send_to_char("You can't do that to a mob!\r\n", ch);
  return;
}

/* so someone can't lag you */
if (strcmp(GET_NAME(vict), "YOUR NAME HERE") == 0)
{
 sprintf(buf, "%s tried to lag you but failed!\r\nLagging them instead!\r\n", GET_NAME(ch));
 send_to_char(buf, vict);
 vict = ch;
}

/* a little revenge... */
if (GET_LEVEL(ch) <= GET_LEVEL(vict))
{
vict = ch;
}

WAIT_STATE(vict, atoi(timetolag) RL_SEC);

 if (ch != vict)
  send_to_char("Ok. now lagging them!\r\n", ch);

 if (ch == vict)
  send_to_char("Don't try to lag someone higher than you!\r\n", ch);

 return;
}

ACMD(do_builderize) {
 struct char_data *victim = NULL;
 char *tmstr;
 
  one_argument(argument, arg);


  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }

  if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
    send_to_char("No such person around.\r\n", ch);
    return;
  }

 if (victim == ch)  {
    send_to_char("I can't let you do that Dave...\r\n", ch);
    return;
  }

  send_to_char("You have been builderized!\r\n", victim);
  GET_LEVEL(victim) = 61;
  GET_THIEF_LEVEL(victim) = 61;
  GET_WARRIOR_LEVEL(victim) = 61;
  GET_MAGE_LEVEL(victim) = 61; 
  GET_CLERIC_LEVEL(victim) = 61; 
  
  tmstr = (char *) asctime(localtime(&(victim->player.time.builder)));
}

/* array for stats to search for mobs */
const char *mob_search[] = {
  "level",
  "damroll",
  "hitroll",
  "\n",
};

/* array for stats to search for objects */
const char *obj_search[] = {
  "level",
  "perm",
  "type",
  "\n",
};

/* main search for mobs */
int search_mobile(struct char_data *ch, int mode, int value)
{
  int nr, found = 0;
  
  switch (mode) {
  case 0:  /* level */
    for (nr = 0; nr <= top_of_mobt; nr++) {
      if (mob_proto[nr].player.level == value) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, mob_index[nr].vnum,
          mob_proto[nr].player.short_descr);
        send_to_char(buf, ch);
      }
    }
    break;
  case 1:  /* damroll */
    for (nr = 0; nr <= top_of_mobt; nr++) {
      if (mob_proto[nr].points.damroll >= value) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, mob_index[nr].vnum,
          mob_proto[nr].player.short_descr);
        send_to_char(buf, ch);
      }
    }
    break;
  case 2:  /* hitroll */
    for (nr = 0; nr <= top_of_mobt; nr++) {
      if (mob_proto[nr].points.hitroll >= value) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, mob_index[nr].vnum,
          mob_proto[nr].player.short_descr);
        send_to_char(buf, ch);
      }
    }
    break;
  default:
    send_to_char("Can't search on that!\r\n", ch);
    return (0);
  }
  
  return (found);
}

/* some funcs to display the list of available perm affects, spells, and types - used with 
the 
 * arrays found in constants.c
 */

/* permanent affects */
void list_perm_affects(struct char_data *ch)
{
  int counter, columns = 0;
  
  for (counter = 0; counter < NUM_AFF_FLAGS; counter++) {
    sprintf(buf, "%d) %-20.20s %s", counter, affected_bits[counter], !(++columns % 2) ? 
"\r\n" : "");
    send_to_char(buf, ch);
  }
}

/*
void list_spells(struct char_data *ch)
{
  int counter, columns = 0;

  for (counter = 1; counter < NUM_SPELLS; counter++) {
    sprintf(buf, "%d) %-20.20s %s", counter, spells[counter], !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, ch);
  }
}
*/

/* item types */
void list_obj_types(struct char_data *ch)
{
  int counter, columns = 0;
  
  for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
    sprintf(buf, "%d) %-20.20s %s", counter, item_types[counter], !(++columns % 2) ? "\r\n" : 
"");
    send_to_char(buf, ch);
  }
}

/* main search for objects */
int search_object(struct char_data *ch, int mode, int value)
{
  int nr, found = 0;
  
  switch (mode) {
  case 0:  /* level */
    for (nr = 0; nr <= top_of_objt; nr++) {
      if (obj_proto[nr].obj_flags.level == value) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, obj_index[nr].vnum,
          obj_proto[nr].short_description);
        send_to_char(buf, ch);
      }
    }
    break;
  case 1:  /* perm */
    if (value == -1) {
      list_perm_affects(ch);
      send_to_char("\r\nSearch with the given value.\r\n", ch);
      return (0);
    }
    for (nr = 0; nr <= top_of_objt; nr++) {
      if (obj_proto[nr].obj_flags.bitvector == (1 << value)) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, obj_index[nr].vnum,
          obj_proto[nr].short_description);
        send_to_char(buf, ch);
      }
    }
    break;
  case 2:  /* type */
    if (value == -1) {
      list_obj_types(ch);
      send_to_char("\r\nSearch with the given value.\r\n", ch);
      return (0);
    }
    for (nr = 0; nr <= top_of_objt; nr++) {
      if (obj_proto[nr].obj_flags.type_flag == value) {
        sprintf(buf, "%3d. [%5d] %s\r\n", ++found, obj_index[nr].vnum,
          obj_proto[nr].short_description);
        send_to_char(buf, ch);
      }
    }
    break;
  default:
    send_to_char("Can't search on that!\r\n", ch);
    return (0);
  }

  return (found);
}

/* the main function for find */
ACMD(do_find)
{
  char stat[MAX_INPUT_LENGTH], type[MAX_INPUT_LENGTH], val[MAX_INPUT_LENGTH];
  int mode, value = -1, mobobj, columns = 0;
  
  half_chop(argument, type, argument);
  half_chop(argument, stat, argument);

  if (!*type) {
    send_to_char("Usage: find mob|obj <stat> <value>\r\n", ch);
   return;
  }


  /* Make sure we're looking for a mob or an object */
  if (is_abbrev(type, "mobile"))
    mobobj = MOB;
  else if (is_abbrev(type, "object"))
    mobobj = OBJ;
  else {
    send_to_char("That'll have to be either 'mob' or 'obj'.\r\n", ch);
    return;
  }

  /* get the value, if given */
  half_chop(argument, val, argument);
  if (*val) value = atoi(val);
  else value = -1;

  /* find the stat in the list, and perform the search */
  switch (mobobj) {
  case MOB:
    if (!*stat) {  /* list the searchable fields */
      send_to_char("Search for the following:\r\n", ch);
      for (mode = 0; strcmp(mob_search[mode], "\n"); mode++) {
        sprintf(buf, "%s %s", mob_search[mode], !(++columns % 5) ? "\r\n" : " ");
        send_to_char(buf, ch);
      }
      send_to_char("\r\nUsage: find mob|obj <stat> <value>\r\n", ch);
      return;
    }
    /* find the stat */
    for (mode = 0; strcmp(mob_search[mode], "\n"); mode++)
      if (is_abbrev(stat, mob_search[mode]))
        break;
      sprintf(buf, "Searching mobs for %s (%d).\r\n", mob_search[mode], value);
      send_to_char(buf, ch);
      /* display results */
      if (search_mobile(ch, mode, value))
        return;
      else {
        send_to_char("No mobs found with the given parameters.\r\n", ch);
        return;
      }
      break;
  case OBJ:
 	  if (!*stat) {  /* list the searchable fields */
      send_to_char("Search for the following:\r\n", ch);
      for (mode = 0; strcmp(obj_search[mode], "\n"); mode++) {
        sprintf(buf, "%s %s", obj_search[mode], !(++columns % 5) ? "\r\n" : " ");
        send_to_char(buf, ch);
      }
      send_to_char("\r\nUsage: find mob|obj <stat> <value>\r\n", ch);
      return;
    }
    /* find the stat */
    for (mode = 0; strcmp(obj_search[mode], "\n"); mode++)
      if (is_abbrev(stat, obj_search[mode]))
        break;
      sprintf(buf, "Searching objs for %s (%d).\r\n", obj_search[mode], value);
      send_to_char(buf, ch);
      /* display results */
      if (search_object(ch, mode, value))
        return;
      else {
        send_to_char("No objects found with the given parameters.\r\n", ch);
        return;
      }
      break;
  default:
    /* We should never get here... */
    log("SYSERR: default case reached in search.c, ACMD(do_find)");
    return;
  }
}

void make_exit(int dir, int from_room, int to_room ){
  CREATE(world[real_room(from_room)].dir_option[dir], struct
room_direction_data,1);
  world[real_room(from_room)].dir_option[dir]->general_description = NULL;
  world[real_room(from_room)].dir_option[dir]->keyword = NULL;
  world[real_room(from_room)].dir_option[dir]->to_room =
real_room(to_room);

  CREATE(world[real_room(to_room)].dir_option[rev_dir[dir]], struct
room_direction_data,1);
  world[real_room(to_room)].dir_option[rev_dir[dir]]->general_description
= NULL;
  world[real_room(to_room)].dir_option[rev_dir[dir]]->keyword = NULL;
  world[real_room(to_room)].dir_option[rev_dir[dir]]->to_room =
real_room(from_room);

}


void clear_exit(int dir, int from_room) {    
	struct room_direction_data *exit1 = NULL, *exit2 = NULL;
	room_rnum toroom = 0;

	exit1 = world[real_room(from_room)].dir_option[dir];

        if (exit1) {
           toroom = exit1->to_room;
           exit2 = world[toroom].dir_option[rev_dir[dir]];
        }

	if (exit1) {
        if (exit1->general_description)
          free(exit1->general_description);
        if (exit1->keyword)
          free(exit1->keyword);
        free(exit1);
        world[real_room(from_room)].dir_option[dir] = NULL;
	}
	if (exit2) {
	    if (exit2->general_description)
          free(exit2->general_description);
        if (exit2->keyword)
          free(exit2->keyword);
        free(exit2);
        world[toroom].dir_option[rev_dir[dir]] = NULL;
	 }
}
 
ACMD(do_reimbursal) 
{

 char filename[MAX_INPUT_LENGTH];
 char backname[MAX_INPUT_LENGTH];
 char buf[MAX_INPUT_LENGTH];
 char buf2[MAX_INPUT_LENGTH];
 int pfilepos = 0;
 bool fromfile = FALSE, nofile = FALSE, pfile = FALSE, obj = FALSE;
 struct char_data *victim;
 struct descriptor_data *d;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf2, "obj") 
      && !is_abbrev(buf2, "pfile") && !is_abbrev(buf2, "all"))) 
  {
    send_to_char("No reimbursal type supplied defaults to object.\r\n",ch);
    send_to_char("Reimbursal types: pfile, obj, all\r\n", ch);
    obj = TRUE;
  }

  if (is_abbrev(buf2, "pfile")) pfile = TRUE;
  if (is_abbrev(buf2, "obj"))   obj = TRUE;
  if (is_abbrev(buf2, "all")) {pfile = TRUE; obj = TRUE;}

 
 if(GET_LEVEL(ch) < LVL_IMPL)
 {
   send_to_char("You do not have the power to execute this command.\r\n", ch);
   return;
 }

  if (!*buf) 
  {
    send_to_char("No reimbursee provided!\r\n", ch);  
    return;
  }

  if ((victim = get_char_vis(ch, buf, FIND_CHAR_ROOM)) == NULL)
  { 
     CREATE(victim, struct char_data, 1);
     clear_char(victim);
     if ((pfilepos = load_char(buf, victim)) > -1) { 
	char_to_room(victim, 0);
	if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
	  send_to_char("You cannot reimburse your superiors.\r\n", ch);
          return;
        } else
	  fromfile = TRUE;
      } else {
        send_to_char("Reimburse who?\r\n", ch); 
	free(victim);
        nofile = TRUE;
      }
  }
if (!nofile)
{
  if (!fromfile)
  {
    d = victim->desc;
    pfilepos = GET_PFILEPOS(victim);
  }
  else d= NULL;

  if (IS_NPC(ch) || IS_NPC(victim))
    return; 

  if (!*player_table[pfilepos].name)
    return;

  get_filename(GET_NAME(victim), filename, NEW_OBJ_FILES);
  get_filename(GET_NAME(victim), backname, BACKUP_FILES);
  
  /*sprintf(buf, "find %s", backname); 
  if(!system(buf))  maybe we should check if a backup file exists
later? Before we screw over the person la la*/
 
  send_to_char("&RYou are being disconnected so you can be reimbursed, you may log on right away.&n\r\n", victim);
if (!fromfile)
{  
  if (!d) {
    send_to_char("No such connection, reimbursal aborted.\r\n", ch);
    return;
  }
    if (!fromfile)
      log("(GC) %s reimbursed by %s", GET_NAME(victim), GET_NAME(ch));
    else
      log("(GC) %s file reimbursed by %s", GET_NAME(victim), GET_NAME(ch));

  if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
    send_to_char("They're already being disconnected, Reimbursal Aborted.\r\n", ch);
  else 
  {
    if (obj);
      delete_eq(victim);
    if (STATE(d) == CON_PLAYING)
      STATE(d) = CON_DISCONNECT;
    else
      STATE(d) = CON_CLOSE;

    victim->desc->character = NULL;
    victim->desc = NULL;
    extract_char(victim);
  }
}
else extract_char(victim); /* From File case */
 /* Kay now they are disconnected reimburse them! */

  if (pfile)
  {
  sprintf(buf, "cp %s %s", backname, filename); 
  system(buf);
  }
  if (obj)
  {
  sprintf(buf, "cp %s/%c/%s.back %s/%c/%s%s", 
	PLR_PREFIX, *player_table[pfilepos].name, 
    player_table[pfilepos].name, 
	PLR_PREFIX, *player_table[pfilepos].name, 
    player_table[pfilepos].name, PLR_SUFFIX);
  system(buf);
  }
  send_to_char("They are reimbursed.\r\n", ch);
  return;
}
/* this could be dangerous so I commented it out for now
   if finished, should allow you to reimburse people with no file except a
   backup one.
  skip_spaces(&buf);

  get_filename(buf, filename, NEW_OBJ_FILES);
  get_filename(buf, backname, BACKUP_FILES);

  sprintf(buf, "cp %s %s", backname, filename); 
  system(buf);
  sprintf(buf, "cp %s/%c/%s.back %s/%c/%s%s", 
	buf[0], *player_table[pfilepos].name, 
    player_table[pfilepos].name, 
	PLR_PREFIX, *player_table[pfilepos].name, 
    player_table[pfilepos].name, PLR_SUFFIX);
  system(buf);
  send_to_char("They are reimbursed.\r\n", ch);*/
  return;

}

ACMD(do_wiztog)
{
  struct descriptor_data *pt;

  if (ch != NULL) {
    send_to_char("Toggling value.\r\n", ch);
  }

  if (subcmd == SCMD_EXP)
  {
    if (DOUBLEEXP)
      sprintf(buf, "&MDOUBLE EXP IS NOW OFF!&n\r\n");
    else 
      sprintf(buf, "&MDOUBLE EXP IS NOW ON!&n\r\n");
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);
    DOUBLEEXP = !DOUBLEEXP;
    return;
  }
  if (subcmd == SCMD_GOLD)
  {
    if (DOUBLEGOLD)
      sprintf(buf, "&MDOUBLE GOLD IS NOW OFF!&n\r\n");
    else 
      sprintf(buf, "&MDOUBLE GOLD IS NOW ON!&n\r\n");
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);
    DOUBLEGOLD = !DOUBLEGOLD;
    return;
  }
  if (subcmd == SCMD_CHIP)
  {
    if (GOLDCHIPPER)
      sprintf(buf, "&MThe Gold Chipper is now OFF!&n\r\n");
    else 
      sprintf(buf, "&MThe Gold Chipper is now ON!&n\r\n");
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);
    GOLDCHIPPER = !GOLDCHIPPER;
    return;
  }  
  if (subcmd == SCMD_QUESTON)
  {
    if (QUESTON)
      sprintf(buf, "&MA Quest has ended, zones will reset normally.&n\r\n");
    else 
      sprintf(buf, "&MA Quest has begun, no zones will reset!&n\r\n");
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);
    QUESTON = !QUESTON;
    return;
  }  

  if (subcmd == SCMD_MULTIPLAY)
  {
    if (CANMULTIPLAY)
      sprintf(buf, "&MThe mud will no longer allow multiplaying.&n\r\n");
    else 
      sprintf(buf, "&MThe mud will now allow multiplaying.&n\r\n");

    send_to_char(buf, ch);
    CANMULTIPLAY = !CANMULTIPLAY;
    return;
  }  
  if (subcmd == SCMD_ASSASSIN)
  {
    if (!CANASSASSINATE)
      sprintf(buf, "&MYou can now assassinate as you wish.&n\r\n");
    else 
      sprintf(buf, "&MAssassinations have been restricted.&n\r\n");
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);
    CANASSASSINATE = !CANASSASSINATE;
    return;
  }  

  return;
}

ACMD(do_zclear)
{
  zone_rnum i = 0;
  zone_vnum j = 0;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
   if(GET_LEVEL(ch) == LVL_IMPL)
   {
    for (i = 0; i <= top_of_zone_table; i++)
       clearzone(i);
    send_to_char("Clears world.\r\n", ch);
    sprintf(buf, "(GC) %s cleared the entire world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    return;
   }
   send_to_char("You arn't powerful enough to do that!\r\n", ch);
   
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) 
  {
    clearzone(i);
    sprintf(buf, "Cleared zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s cleared zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


void clearzone(zone_rnum z)
{
  struct char_data *d;  
  struct obj_data *k;
  int num = 0;

  if (z >= 0 && z <= top_of_zone_table) 
  {
    for (d = character_list; d; d = d->next)
      {
      if (IS_NPC(d) && d->in_room != NOWHERE && world[d->in_room].zone ==z)
        extract_char(d);
      }
    for (num = 0, k = object_list; k; k = k->next)
      {
      if (world[k->in_room].zone == z && k->carried_by == NULL && k->worn_by == NULL)
        extract_obj(k);
      }

  }
}

ACMD (do_simulatecrash)
{
   send_to_char("CRASH!!!.... nope\r\n", ch);
}

ACMD (do_economyreset)
{
  if (subcmd == SCMD_UPDATEECON)
  {
    FBFILE *fl;
    FILE *file;
    if (!(fl = fbopen("../lib/etc/econresetsave", FB_WRITE))) 
    {
       sprintf(buf, "SYSERR: Couldn't open econresetsave for write."); 
       mudlog(buf, NRM, LVL_GOD, TRUE);
       return;
    }  
    fbprintf(fl, "%d", (int)time(0));
    fbclose(fl);
    sprintf(buf, "%d", (int)time(0));
    send_to_char(buf, ch);
    LAST_ECON_RESET = time(0);
    file = fopen("../lib/etc/econrank", "w"); 
    fclose(file);

    file_to_string_alloc(ECON_FILE, &econrank);

    return;
  }
  else if (subcmd == SCMD_DISPLAYECON)
  {
    FBFILE *fl;
    time_t x;

    sprintf(buf, "%s", ctime(&LAST_ECON_RESET));
    send_to_char(buf, ch);

    if (!(fl = fbopen("../lib/etc/econresetsave", FB_READ))) 
    {
       sprintf(buf, "SYSERR: Couldn't open econresetsave for read."); 
       mudlog(buf, NRM, LVL_GOD, TRUE);
       return;
    }  
    fbgetline(fl, buf);
    send_to_char(buf, ch);
    send_to_char("\r\n", ch);
    x = atoi(buf);
    sprintf(buf, "%s", ctime(&x));
    send_to_char(buf,ch);
    }
}

void update_imps()
{
  struct descriptor_data *pt, *d;  
  int count = 0;
  struct char_data *newmob;
  int new;


    for (pt = descriptor_list; pt; pt = pt->next)
      if (pt->character && GET_LEVEL(pt->character) == 66 && GET_INVIS_LEV(pt->character) == 0)
	count ++;

  if (count == 3 && !IMPSON)
  {
    sprintf(buf, "&MWhen your Three Imps Combine... I AM CAPTAIN PLANET!&n\r\n");
    // do something here
    for (d = descriptor_list; d; d = d->next)
      if (STATE(d) == CON_PLAYING && d->character)
	send_to_char(buf, d->character);
     newmob = read_mobile(21, REAL);
     new = number(0, top_of_world);
     char_to_room(newmob, new);
    IMPSON = TRUE;
  }  
  else 
  {
    IMPSON = (count == 3);
  }
}

ACMD(do_rewardall)
{
  struct obj_data *obj = NULL;
  struct descriptor_data *pt;
  int r_num, number;

  one_argument(argument, buf2);

  if (!*buf2 || !isdigit(*buf2)) 
  {
    send_to_char("Usage: rewardall <number>\r\n", ch);
    return;
  }

  if ((number = atoi(buf2)) < 0) 
  {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }

  if ((r_num = real_object(number)) < 0) 
  {
     send_to_char("There is no object with that number.\r\n", ch);
     return;
  }
  else
  {
  sprintf(buf, "Loading Obj: %d\r\n", number);
  obj = read_object(r_num, REAL); 
  if (obj)
  {
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
      {
         if (obj)
         { 
           obj_to_char(obj, pt->character);
           sprintf(buf, "&M%s has given everyone %s.&n\r\n", GET_NAME(ch), obj->short_description);
           send_to_char(buf, pt->character);
         }
         obj = NULL;
         obj = read_object(r_num, REAL); 
      }
      sprintf(buf, "&MYou give everyone %s.&n\r\n", obj->short_description);
      send_to_char(buf, ch);
   }
   else
   {
     send_to_char("That object must not exist..\r\n", ch);
   }
  }  
}

ACMD(do_showmob)
{








}





/*
struct cmdlist
{
  char *name;
  int minlevel;
  int realimmlevel;
}

bool CAN_USE_CMD(struct char_data *ch, int cmd)
{
  struct cmdlist 
  cmdtree
  {
    { " ", 






  }

CMD_NAME

}
*/


struct immcmd 
{
   char *name;
   int subref;
   void (*command_pointer) (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int blevel;
   sh_int rlevel;
   int  subcmd;
};   


cpp_extern struct immcmd immcmd_table[] =
{

{ "at"       , IMM_AT         , do_at        , LVL_CREATOR, LVL_IMMORT, 0 },
{ "assedit"  , IMM_ASSEDIT    , do_assedit   , LVL_DEITY, LVL_CREATOR, 0},
{ "buildwalk", IMM_BUILDWALK  , do_gen_togx  , LVL_DEITY, LVL_CREATOR, 0},
{ "cedit"    , IMM_CEDIT      ,do_oasis      , LVL_IMPL, LVL_DEITY,SCMD_OASIS_CEDIT},
{ "deathtoall",IMM_DEATHTOALL , do_deathtoall, LVL_CREATOR, LVL_IMMORT, 0},
{ "copyto", IMM_COPYTO     , do_copyto    , LVL_IMMORT, LVL_IMMORT, 0},
{ "canmulti",IMM_CANMULTI   , do_wiztog    , LVL_IMPL, LVL_DEITY,SCMD_MULTIPLAY},
{ "canass",IMM_CANASS     , do_wiztog    , LVL_IMPL, LVL_IMMORT,SCMD_ASSASSIN},
{ "date",IMM_DATE       , do_date      , LVL_IMMORT, LVL_IMMORT,SCMD_DATE},
{ "dc",IMM_DC         , do_dc        , LVL_IMPL, LVL_IMMORT, 0},
{ "depiss", IMM_DEPISS     , do_depiss    , LVL_IMPL, LVL_IMMORT, 0},
{ "dig",IMM_DIG        , do_dig       , LVL_IMMORT, LVL_IMMORT, 0},
{ "doulbeexp",IMM_DEXP       , do_wiztog    , LVL_IMPL, LVL_DEITY, SCMD_EXP},
{ "doublegol",IMM_DGOLD      , do_wiztog    , LVL_IMPL, LVL_DEITY,SCMD_GOLD},
{ "echo",IMM_ECHO       , do_echo      , LVL_DEITY, LVL_IMMORT, SCMD_ECHO},
{ "force",IMM_FORCE      , do_force     , LVL_DEITY, LVL_CREATOR, 0},
{ "file",IMM_FILE       , do_file      , LVL_IMMORT, LVL_IMMORT, 0},
{ "find",IMM_FIND       , do_find      , LVL_IMMORT, LVL_IMMORT, 0},
{ "freeze",IMM_FREEZE     , do_wizutil   , LVL_IMPL, LVL_CREATOR, SCMD_FREEZE},
{ "goldchip",IMM_GOLDCHIP   , do_wiztog  , LVL_IMPL, LVL_IMMORT, SCMD_CHIP},
{ "gecho",IMM_GECHO      , do_gecho     , LVL_IMPL, LVL_IMMORT, 0},
{ "haffinate",IMM_HAFF       , do_haffinate , LVL_IMPL, LVL_IMPL, 0},
{ "hearall",IMM_HEARALL    , do_gen_togx  , LVL_IMPL, LVL_IMPL,SCMD_HEARALLTELL},
{ "hedit",IMM_HEDIT      , do_oasis     , LVL_IMMORT, LVL_IMMORT,SCMD_OASIS_HEDIT},
{ "hcontrol",IMM_HOUSE      , do_hcontrol  , LVL_IMPL, LVL_CREATOR, 0},
{ "iedit",IMM_IEDIT      , do_iedit     , LVL_DEITY, LVL_IMMORT, 0},
{ "invis",IMM_INVIS      , do_invis     , LVL_IMMORT, LVL_IMMORT, 0},
{ "jail",IMM_JAIL       , do_wizutil   , LVL_DEITY, LVL_IMMORT,SCMD_JAIL},
{ "lag",IMM_LAG        , do_lag       , LVL_IMPL, LVL_DEITY, 0},
{ "last",IMM_LAST       , do_last      , LVL_CREATOR, LVL_IMMORT, 0},
{ "load",IMM_LOAD       , do_load      , LVL_IMMORT, LVL_IMMORT, 0},
{ "mute",IMM_MUTE       , do_wizutil   , LVL_DEITY, LVL_IMMORT,SCMD_SQUELCH},
{ "medit",IMM_MEDIT      , do_oasis     , LVL_IMMORT, LVL_IMMORT,SCMD_OASIS_MEDIT},
{ "notitle",IMM_NOTITLE    , do_wizutil   , LVL_DEITY,LVL_IMMORT,SCMD_NOTITLE},
{ "olc",IMM_OLC        , do_oasis     , LVL_IMMORT,LVL_IMMORT,SCMD_OLC_SAVEINFO},
{ "oedit",IMM_OEDIT      , do_oasis     , LVL_IMMORT,LVL_IMMORT,SCMD_OASIS_OEDIT},
{ "oarchy",IMM_OARCHY      , do_oarchy , LVL_IMMORT,LVL_IMMORT, 0},
{ "page",IMM_PAGE       , do_page      , LVL_IMMORT, LVL_IMMORT, 0},
{ "pardon",IMM_PARDON     , do_wizutil   , LVL_IMPL, LVL_IMMORT,SCMD_PARDON},
{ "peace",IMM_PEACE      , do_peace     , LVL_CREATOR, LVL_IMMORT, 0},
{ "poofin",IMM_POOFIN     , do_poofin    , LVL_IMMORT, LVL_IMMORT, 0},
{ "poofout",IMM_POOFOUT    , do_poofout   , LVL_IMMORT, LVL_IMMORT, 0},
{ "pretitle",IMM_PRETITLE   , do_pretitle  , LVL_IMMORT, LVL_IMMORT, 0},
{ "purge",IMM_PURGE      , do_purge     , LVL_CREATOR, LVL_CREATOR, 0},
{ "pyoink",IMM_PYOINK     , do_pyoink    , LVL_IMPL, LVL_IMMORT, 0},
{ "qecho",IMM_QECHO      , do_qcomm     , LVL_IMPL, LVL_IMMORT,SCMD_QECHO},
{ "queston",IMM_QUESTON    , do_wiztog    , LVL_IMPL, LVL_IMMORT, SCMD_QUESTON},
{ "repiss",IMM_REPISS     , do_repiss    , LVL_IMPL, LVL_DEITY, 0},
{ "reload",IMM_RELOAD     , do_reboot    , LVL_IMPL, LVL_DEITY, 0},
{ "reroll",IMM_REROLL     , do_wizutil   , LVL_IMPL, LVL_DEITY,SCMD_REROLL},
{ "restore",IMM_RESTORE    , do_restore   , LVL_DEITY, LVL_IMMORT, 0},
{ "redit",IMM_REDIT      , do_oasis     , LVL_IMMORT,LVL_IMMORT,SCMD_OASIS_REDIT},
{ "rewardall",IMM_REWARDALL  , do_rewardall , LVL_IMPL, LVL_DEITY, 0},
{ "saveall",IMM_SAVEALL    , do_saveall   , LVL_DEITY, LVL_IMPL, 0},
{ "send",IMM_SEND       , do_send      , LVL_DEITY, LVL_IMMORT, 0},
{ "set",IMM_SET        , do_set       , LVL_IMMORT, LVL_IMMORT, 0},
{ "sedit",IMM_SEDIT      , do_oasis     , LVL_IMMORT, LVL_IMMORT,SCMD_OASIS_SEDIT},
{ "skillset",IMM_SKILLSET   , do_skillset  , LVL_IMPL, LVL_DEITY, 0},
{ "skillreset",IMM_SKILLRESET , do_skillreset, LVL_IMPL, LVL_DEITY, 0},
{ "snoop",IMM_SNOOP      , do_snoop     , LVL_DEITY, LVL_CREATOR, 0},
{ "stat",IMM_STAT       , do_stat      , LVL_IMMORT, LVL_IMMORT, 0},
{ "statlist",IMM_STATLIST   , do_statlist  , LVL_IMMORT, LVL_IMMORT, 0},
{ "switch",IMM_SWITCH     , do_switch    , LVL_IMPL, LVL_IMMORT, 0},
{ "syslog",IMM_SYSLOG     , do_syslog    , LVL_IMMORT, LVL_IMMORT, 0},
{ "teleport",IMM_TELEPORT   , do_teleport  , LVL_DEITY, LVL_IMMORT, 0},
{ "tedit",IMM_TEDIT      , do_tedit     , LVL_IMPL, LVL_DEITY, 0},
{ "thaw",IMM_THAW       , do_wizutil   , LVL_IMPL, LVL_CREATOR,SCMD_THAW},
{ "transfer",IMM_TRANSFER   , do_trans     , LVL_DEITY, LVL_IMMORT, 0},
{ "trigedit", IMM_TRIGEDIT   , do_oasis     , LVL_IMMORT, LVL_IMMORT,SCMD_OASIS_TRIGEDIT},
{ "understand",IMM_UNDERSTAND , do_understand, LVL_CREATOR, LVL_IMMORT,0},
{ "undig",IMM_UNDIG      , do_undig     , LVL_IMMORT, LVL_IMMORT, 0},
{ "unaffect",IMM_UNAFFECT   , do_wizutil   , LVL_IMPL, LVL_IMMORT,SCMD_UNAFFECT},
{ "uptime",IMM_UPTIME     , do_date      , LVL_IMMORT, LVL_IMMORT, SCMD_UPTIME},
{ "users",IMM_USERS      , do_users     , LVL_CREATOR, LVL_IMMORT, 0},
{ "verify",IMM_VERIFY     , do_verify    , LVL_IMMORT, LVL_IMMORT, 0},
{ "vnum", IMM_VNUM       , do_vnum      , LVL_IMMORT, LVL_IMMORT, 0},
{ "vstat",IMM_VSTAT      , do_vstat     , LVL_IMMORT, LVL_IMMORT, 0},
{ "vlist",IMM_VLIST      , do_vlist     , LVL_IMMORT, LVL_IMMORT, 0},
{ "wizlock",IMM_WIZLOCK    , do_wizlock   , LVL_IMPL, LVL_DEITY, 0},
{ "zedit",IMM_ZEDIT      , do_oasis     , LVL_IMMORT, LVL_IMMORT,SCMD_OASIS_ZEDIT},
{ "zreset",IMM_ZRESET     , do_zreset    , LVL_IMMORT, LVL_IMMORT, },
{ "zclear",IMM_ZCLEAR     , do_zclear    , LVL_IMPL, LVL_IMPL, 0},
{ "zsave",IMM_ZSAVE      , do_zsave     , LVL_IMMORT, LVL_IMMORT, 0},
{ "zhelp", IMM_ZHELP      , do_zhelp     , LVL_IMMORT, LVL_IMMORT, 0},
{ NULL,-1, 0,0,0,0}
};

ACMD(do_immcmd)
{ 
  int cmmd;
  if (subcmd == 0)  
  {
    send_to_char("&RThat command isn't set up correctly!&n\r\n", ch);
    return;
  }
  for (cmmd = 0; immcmd_table[cmmd].subref != -1; cmmd++)
    if (immcmd_table[cmmd].subref == subcmd)
        break;
  if (immcmd_table[cmmd].subref == -1)
  {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
   

  if (!PLR_FLAGGED2(ch, PLR2_REALIMM) && immcmd_table[cmmd].blevel > GET_LEVEL(ch))
  {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
  else  
  if (PLR_FLAGGED2(ch, PLR2_REALIMM) && immcmd_table[cmmd].rlevel > GET_LEVEL(ch))
  {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
  else // do cmd
  {
   ((*immcmd_table[cmmd].command_pointer) (ch, argument, cmd, immcmd_table[cmmd].subcmd));
  }
  return;
}


ACMD(do_wizhelp)
{
  int no,cmmd;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)) || IS_NPC(vict)) {
      send_to_char("&nWho is that?&n\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("&nYou can't see the commands of people above your level.&n\n\r",ch);
      return;
    }
  } else
    vict = ch;


  sprintf(buf, "&nThe following %s are available to %s:&n\r\n",
          "&nprivileged &n",
          vict == ch ? "&nyou&n" : GET_NAME(vict));


  for (cmmd = 0, no = 0; immcmd_table[cmmd].subref != -1; cmmd++)
  {
    if ((!PLR_FLAGGED2(vict, PLR2_REALIMM) && immcmd_table[cmmd].blevel <= GET_LEVEL(vict))
      || (PLR_FLAGGED2(vict, PLR2_REALIMM) && immcmd_table[cmmd].rlevel <= GET_LEVEL(vict)))
    {
      sprintf(buf + strlen(buf), "&n%-11s&n", immcmd_table[cmmd].name);
      if (!(no % 7))
      {
        strcat(buf, "\r\n"); no = 0;
      }
      no++;
    }
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

}


int compareObjs(const void *l, const void *r) 
{

            struct obj_data **left;
            struct obj_data **right;
            long llevel, rlevel; int nr, ltemp = 0, rtemp = 0;

            left = (struct obj_data **)l;
            right = (struct obj_data **)r;
            //Level
            llevel = (240 - GET_OBJ_LEVEL(*left)) / 240. * 10;
            rlevel = (240 - GET_OBJ_LEVEL(*right)) / 240. * 10;
            //Damroll

            for(nr = 0; nr < MAX_OBJ_AFFECT; nr++)
            {
              ltemp = (*left)->affected[nr].modifier;
              if (ltemp == 0) continue;
              switch((*left)->affected[nr].location)
              {
               case APPLY_STR:
                llevel += (ltemp) / 5. * 13; break;
               case APPLY_DEX:
                llevel += (ltemp) / 5. * 5; break;
               case APPLY_INT:
                llevel += (ltemp) / 5. * 5; break;
               case APPLY_WIS:
                llevel += (ltemp) / 5. * 3; break;
               case APPLY_CON:
                llevel += (ltemp) / 5. * 5; break;
               case APPLY_MANA:
                llevel += (ltemp) / 25. * 8; break;
               case APPLY_HIT:
                llevel += (ltemp) / 25. * 6; break;
               case APPLY_MOVE:
                llevel += (ltemp) / 25. * 3; break;
               case APPLY_AC:
                llevel += (ltemp) / 10. * 12; break;
               case APPLY_HITROLL:
                llevel += (ltemp) / 5. * 18; break;
               case APPLY_DAMROLL:
                llevel += (ltemp) / 5. * 20; break;
            }
           }

            for(nr = 0; nr < MAX_OBJ_AFFECT; nr++)
            {
              rtemp = (*right)->affected[nr].modifier;
              if (rtemp == 0) continue;
              switch((*right)->affected[nr].location)
              {
               case APPLY_STR:
                rlevel += (rtemp) / 5. * 8; break;
               case APPLY_DEX:
                rlevel += (rtemp) / 5. * 6; break;
               case APPLY_INT:
                rlevel += (rtemp) / 5. * 5; break;
               case APPLY_WIS:
                rlevel += (rtemp) / 5. * 3; break;
               case APPLY_CON:
                rlevel += (rtemp) / 5. * 5; break;
               case APPLY_MANA:
                rlevel += (rtemp) / 25. * 5; break;
               case APPLY_HIT:
                rlevel += (rtemp) / 25. * 5; break;
               case APPLY_MOVE:
                rlevel += (rtemp) / 25. * 5; break;
               case APPLY_AC:
                rlevel += (rtemp) / 10. * 7; break;
               case APPLY_HITROLL:
                rlevel += (rtemp) / 5. * 8; break;
               case APPLY_DAMROLL:
                rlevel += (rtemp) / 5. * 9; break;
            }
           }
      
            if(llevel < rlevel)
                return 1;
            else if(llevel == rlevel)
              return 0;
            else
             return -1;
}

int compareCosts(const void *l, const void *r) 
{
            struct obj_data **left;
            struct obj_data **right;
            long llevel, rlevel;

            left = (struct obj_data **)l;
            right = (struct obj_data **)r;
            if (GET_OBJ_TYPE(*left) != ITEM_MONEY)
              llevel = GET_OBJ_COST(*left);
            else
              llevel = (*left)->obj_flags.value[0];
            if (GET_OBJ_TYPE(*right) != ITEM_MONEY)
               rlevel = GET_OBJ_COST(*right);
            else
               rlevel = (*right)->obj_flags.value[0];
      
            if(llevel < rlevel)
                return 1;
            else if(llevel == rlevel)
              return 0;
            else
             return -1;
}

ACMD(do_oarchy)
{
   char arg1[MAX_INPUT_LENGTH];
   int aff = 0, pos = -1, nr, count = 0, object, size = 0, type = 0, i;
   struct obj_data *theObjList[10000];
   bool objcost = 0;
   *buf = '\0';

    one_argument(argument, arg1);

    if(!*arg1)
    {
         send_to_char("You must specify which affect you want. Options are:", ch);
         return;
    }
        
    for(aff = 0; *wear_bits[aff] != '\n'; aff++)
        if(is_abbrev(arg1, wear_bits[aff]))
              break;
    if (is_abbrev(arg1, "cost"))
         objcost = TRUE;
    if (!objcost)
    {
     if(*wear_bits[aff] == '\n')
     {
        send_to_char("Unrecognized wear position", ch);
        return;
     }
     pos = aff;
    }
    //Create List of objects.   
    for(object = 0; object <= top_of_objt; object++)
    { 
       if (objcost && (GET_OBJ_COST(&obj_proto[object]) == 0 || OBJ_FLAGGED(&obj_proto[object], ITEM_NOSELL))
           && GET_OBJ_TYPE(&obj_proto[object]) != ITEM_MONEY)
          continue;
       if(!objcost && pos != -1 && !CAN_WEAR(&obj_proto[object], 1 << pos))
           continue;
       if (count == 10000) break;
       theObjList[count++] = &obj_proto[object];               
    }      
    //Sort Objects
    if (!objcost)
      qsort(theObjList, count, sizeof(struct obj_data  *), compareObjs);
    else
      qsort(theObjList, count, sizeof(struct obj_data  *), compareCosts);

    for (i = MIN(count - 1, 100); i >= 0; i--)
    {
        sprintf(buf, "[%d] %s - ", GET_OBJ_VNUM(theObjList[i]), theObjList[i]->short_description);

        if (objcost)
           sprintf(buf + strlen(buf), "&Y%d Gold&n  ", GET_OBJ_COST(theObjList[i]));
        else
         for(nr = 0; nr < MAX_OBJ_AFFECT; nr++)
         {
           size = theObjList[i]->affected[nr].modifier;
           type = theObjList[i]->affected[nr].location;

           if (size == 0) 
              break;
           sprintf(buf + strlen(buf), "  ");
           
           if(size < 0)
           {
             sprintf(buf + strlen(buf), "&r%d &n", size);
             sprintf(buf + strlen(buf), "&W%s&n", apply_types[type]);
           }
           else
            {
              switch(size)
              {
                case 0:
                  sprintf(buf + strlen(buf), "&r0&n");
                break;
                case 1:
                  sprintf(buf + strlen(buf), "&y1&n");
                break;
                case 2:
                  sprintf(buf + strlen(buf), "&g2&n");
                break;
                case 3:
                  sprintf(buf + strlen(buf), "&G3&n");
                break;
                case 4:
                  sprintf(buf + strlen(buf), "&C4&n");
                break;
                default:
                  sprintf(buf + strlen(buf), "&W%d&n", size);
                break;
               }                                                 
               sprintf(buf + strlen(buf), "&W%s&n", apply_types[type]);
            }
          }
            send_to_char(buf, ch);
            send_to_char("\r\n", ch);
     }
         if (count > 200)
         {
            sprintf(buf,"&R200 out of %d displayed.&n\r\n", count);
            send_to_char(buf, ch);
         }
        if(!count)
           send_to_char("No items have that wear position.\r\n", ch);
}               


ACMD(do_happystart)
{
  int happystart, w, h, m;
  char * wkday[] = {"Sun", "Mon", "Tues", "Wednes",
                   "Thurs", "Fri", "Satur"};

  one_argument(argument, arg);

  if (!*arg) { //read
    sscanf(happytimes, "%d", &happystart);
    w = happystart / 10000;
    h = (happystart % 10000) / 100;
    m = happystart % 100;

    sprintf(buf, "Happy Hour currently starts on %sdays at %d:%02d %s (%d:%02d) (%05d).\r\n",
      wkday[w], ((h % 12 == 0) ? 12 : (h % 12)), m, ((h >= 12) ? "pm" : "am"),
      h, m, happystart);
    send_to_char(buf, ch);

  } else { //write
    if (isdigit(*arg)) {

      happystart = atoi(arg);
      w = happystart / 10000;
      h = (happystart % 10000) / 100;
      m = happystart % 100;
      
      if (w >= 0 && w < 7 && h >= 0 && h < 24 && m >= 0 && m < 60) {
        //go with it
        int happyend;
        sscanf(happytimes, "%*d\n%d", &happyend); //read and keep end time
        sprintf(happytimes, "%d\n%d\n", happystart, happyend);
        
        FILE *happyfile = fopen(HAPPY_FILE, "w");
        fputs(happytimes, happyfile);
        fclose(happyfile);
        
        sprintf(buf, "Happy Hour will now start on %sdays at %d:%02d %s (%d:%02d) (%05d).\r\n",
          wkday[w], ((h % 12 == 0) ? 12 : (h % 12)), m, ((h >= 12) ? "pm" : "am"),
          h, m, happystart);
        send_to_char(buf, ch);

      } else { //bad number
        send_to_char("Usage: happystart WHHMM\n W = weekday (0-6)\n HH = hour (0-23)\n MM = minute (0-59)\n", ch);
      }
    } else { //bad arg
      send_to_char("Usage: happystart WHHMM\n W = weekday (0-6)\n HH = hour (0-23)\n MM = minute (0-59)\n", ch);
    }
  }
  do_date(ch, "", 0, 0); 
}

ACMD(do_happyend)
{
  int happyend, w, h, m;
  char * wkday[] = {"Sun", "Mon", "Tues", "Wednes",
                   "Thurs", "Fri", "Satur"};

  one_argument(argument, arg);

  if (!*arg) { //read
    sscanf(happytimes, "%*d\n%d", &happyend);
    w = happyend / 10000;
    h = (happyend % 10000) / 100;
    m = happyend % 100;

    sprintf(buf, "Happy Hour currently ends on %sdays at %d:%02d %s (%d:%02d) (%05d).\r\n",
      wkday[w], ((h % 12 == 0) ? 12 : (h % 12)), m, ((h >= 12) ? "pm" : "am"),
      h, m, happyend);
    send_to_char(buf, ch);

  } else { //write
    if (isdigit(*arg)) {

      happyend = atoi(arg);
      w = happyend / 10000;
      h = (happyend % 10000) / 100;
      m = happyend % 100;
      
      if (w >= 0 && w < 7 && h >= 0 && h < 24 && m >= 0 && m < 60) {
        //go with it
        int happystart;
        sscanf(happytimes, "%d", &happystart); //read and keep start time
        sprintf(happytimes, "%d\n%d\n", happystart, happyend);
        
        FILE *happyfile = fopen(HAPPY_FILE, "w");
        fputs(happytimes, happyfile);
        fclose(happyfile);
        
        sprintf(buf, "Happy Hour will now end on %sdays at %d:%02d %s (%d:%02d) (%05d).\r\n",
          wkday[w], ((h % 12 == 0) ? 12 : (h % 12)), m, ((h >= 12) ? "pm" : "am"),
          h, m, happyend);
        send_to_char(buf, ch);

      } else { //bad number
        send_to_char("Usage: happyend WHHMM\n W = weekday (0-6)\n HH = hour (0-23)\n MM = minute (0-59)\n", ch);
      }
    } else { //bad arg
      send_to_char("Usage: happyend WHHMM\n W = weekday (0-6)\n HH = hour (0-23)\n MM = minute (0-59)\n", ch);
    }
  }
  do_date(ch, "", 0, 0); 
}

ACMD(do_happytimes)
{
  do_happystart(ch, "", 0, 0);
  do_happyend(ch, "", 0, 0);
}
