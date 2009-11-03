/*
************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "constants.h"
#include "dg_scripts.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;

/* external functs */
void add_follower(struct char_data *ch, struct char_data *leader);
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);

/* MatingMod Defines */
#define NINE_MONTHS    6000 /* 6000 realtime minutes TO GO*/
#define MONTHS_8        5333
#define MONTHS_7        4666 /* Note: These are MONTHS REMAINING */
#define MONTHS_6        4000
#define MONTHS_5        3333
#define MONTHS_4        2666
#define MONTHS_3        2000
#define MONTHS_2        1333
#define MONTH_1         666

/* local functions */
int has_boat(struct char_data *ch);
int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname);
int has_key(struct char_data *ch, obj_vnum key);
void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd);
int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int scmd);
ACMD(do_gen_door);
ACMD(do_enter);
ACMD(do_leave);
ACMD(do_stand);
ACMD(do_sit);
ACMD(do_rest);
ACMD(do_sleep);
ACMD(do_wake);
ACMD(do_follow);
ACMD(do_dismount);

/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;
/*
  if (ROOM_IDENTITY(ch->in_room) == DEAD_SEA)
    return (1);
*/

  if (GET_LEVEL(ch) > LVL_IMMORT)
    return (1);

  if (AFF_FLAGGED(ch, AFF_WATERWALK))
    return (1);

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return (1);

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return (1);

  return (0);
}

  

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  room_rnum was_in;
  int need_movement;

  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, "")) /* XXX: Evaluate NULL */
    return (0);

  /* charmed? */
  if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {
    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    return (0);
  }

  /* if this room or the one we're going to needs a boat, check for one */
  if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
    if (!has_boat(ch)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return (0);
    }
  }

  /* move points needed is avg. move loss for src and destination sect type */
  if (GET_LEVEL(ch) < LVL_IMMORT)
  	need_movement = (movement_loss[SECT(ch->in_room)]);
  else
		need_movement = 0;

  if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);

    return (0);
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_ATRIUM)) {
    if (!House_can_enter(ch, GET_ROOM_VNUM(EXIT(ch, dir)->to_room))) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return (0);
    }
  }
/*  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
      AFF_FLAGGED(ch, AFF_FLOAT)) {
    send_to_char("You can't float thru a tunnel.\r\n", ch);
    return (0);
  }
  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_INDOORS) &&
      AFF_FLAGGED(ch, AFF_FLOAT)) {
    send_to_char("You can't float thru the door!\r\n", ch);
    return (0);
  }*/
  /* Mortals and low level gods cannot enter greater god rooms. */
  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM) &&
	GET_LEVEL(ch) < LVL_GRGOD) {
    send_to_char("You aren't godly enough to use that room!\r\n", ch);
    return (0);
  }


  if (AFF_FLAGGED(ch, AFF_MOUNTED)) { 
   if (GET_HORSEHAPPY(ch) <= 0) {
    REMOVE_BIT(AFF_FLAGS(ch), AFF_MOUNTED);
    GET_HORSEHAPPY(ch) = 50;
    GET_HORSEROOM(ch) = real_room(255); 
    send_to_char("\r\n&n&rYour horse has become too tired to go on, it goes back to the stable.&n\r\n\r\n", ch);
   }
   if (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM && GET_HORSEEQ(ch) < 1) {
    send_to_char("\r\n&n&RYour horse can't go there!&n\r\n", ch);
    return (0);
   }
   if (SECT(EXIT(ch, dir)->to_room) == SECT_FLYING && GET_HORSEEQ(ch) < 2)  {
    send_to_char("\r\n&n&RYour horse can't go there!&n\r\n", ch);
    return (0);
   }
 }

  if (AFF_FLAGGED(ch, AFF_MOUNTED))
    GET_HORSEHAPPY(ch) -= need_movement;
  
  /* Now we know we're allow to go into the room. */
  if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_MOUNTED) && !AFF_FLAGGED(ch, AFF_FLOAT))
    GET_MOVE(ch) -= need_movement;

  if (!AFF_FLAGGED(ch, AFF_SNEAK) && !ROOM_FLAGGED(ch->in_room, ROOM_PERFECTDARK)) 
  {
    sprintf(buf2, "&W$n leaves %s.&n", dirs[dir]);
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }

  /* see if an entry trigger disallows the move */
  if (!entry_mtrigger(ch))
    return 0;
  if (!enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
    return 0;

  was_in = ch->in_room;
  if (ROOM_FLAGGED(ch->in_room, ROOM_FISH) &&
      (PLR_FLAGGED2(ch, PLR2_FISHING) || PLR_FLAGGED2(ch, PLR2_FISH_ON) ||
      MOB_FLAGGED2(ch, PLR2_FISHING) || MOB_FLAGGED2(ch, PLR2_FISH_ON))) {
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISHING);
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISH_ON);
    send_to_char("\r\nYou pack up your fishing gear and move on.\r\n\r\n", ch);
  }

  if ((SECT(ch->in_room) == SECT_FOREST) && PLR_FLAGGED2(ch, PLR2_CUTTING)) {
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_CUTTING);
    send_to_char("\r\nYou decide to stop chopping for the day.\r\n\r\n", ch);
  }

  if ((SECT(ch->in_room) == SECT_MINE) && PLR_FLAGGED2(ch, PLR2_MINING)) {
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_MINING);
    send_to_char("\r\nYou decide mining is too tough to continue on.\r\n\r\n", ch);
  }

  if ((SECT(ch->in_room) == SECT_HUNTGROUNDS) &&
      (PLR_FLAGGED2(ch, PLR2_HUNTING) || PLR_FLAGGED2(ch, PLR2_KILLMOB))) {
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_KILLMOB);
    send_to_char("\r\nYou pack up your hunting gear and move on.\r\n\r\n", ch);
  }

  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);

  if (!AFF_FLAGGED(ch, AFF_SNEAK))
    act("&W$n has arrived.&n", TRUE, ch, 0, 0, TO_ROOM);

  if (ch->desc != NULL)
    look_at_room(ch, 0);

  if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
    log_death_trap(ch);
    death_cry(ch);
    extract_char(ch);
    return (0);
  }

  entry_memory_mtrigger(ch);
  if (!greet_mtrigger(ch, dir)) {
    char_from_room(ch);
    char_to_room(ch, was_in);
    look_at_room(ch, 0);
  } else greet_memory_mtrigger(ch);

  return (1);
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  room_rnum was_in;
  struct follow_type *k, *next;
  int buildwalk(struct char_data *ch, int dir);
  struct affected_type *af = NULL;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
    return (0);
  else if ((!EXIT(ch, dir) && !buildwalk(ch, dir)) || EXIT(ch, dir)->to_room == NOWHERE)
    send_to_char("&RAlas, you cannot go that way...&n\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_WEB)) 
  {
   af = GET_AFFECT(ch, SPELL_WEB);
   
  if (af != NULL && GET_STR(ch) + 15 - af->power > number(0,100))
   {
    send_to_char("You break out of the webs!\r\n", ch);
    REMOVE_BIT(AFF_FLAGS(ch), AFF_WEB);
    affect_from_char(ch, SPELL_WEB);
    WAIT_STATE(ch, PULSE_VIOLENCE / 2.);
   }   

   else 
   {
    send_to_char("&RYou struggle in gluey webs!&n\r\n", ch);
    act("&W$n struggles in gluey webs!&n", TRUE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE / 2.);
   }
  }

  else if (SECT(EXIT(ch, dir)->to_room) == SECT_FLYING && !IS_AFFECTED(ch, AFF_FLOAT) && GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char("You must be able to fly to enter!\r\n", ch);

  else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) {
    if (EXIT(ch, dir)->keyword) {
      sprintf(buf2, "&RThe %s seems to be closed.&n\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf2, ch);
    } else
      send_to_char("&RIt seems to be closed.&n\r\n", ch);

  } else {

    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return (0);

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING)) {
	act("&cYou follow $N.&n\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
	perform_move(k->follower, dir, 1);
      }
    }
    return (1);
  }
  return (0);
}


ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, subcmd - 1, 0);
}


int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname)
{
  int door;

  if (*dir) {			/* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
      send_to_char("That's not a direction.\r\n", ch);
      return (-1);
    }
    if (EXIT(ch, door)) {	/* Braces added according to indent. -gg */
      if (EXIT(ch, door)->keyword) {
	if (isname(type, EXIT(ch, door)->keyword))
	  return (door);
	else {
	  sprintf(buf2, "I see no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return (-1);
        }
      } else
	return (door);
    } else {
      sprintf(buf2, "I really don't see how you can %s anything there.\r\n", cmdname);
      send_to_char(buf2, ch);
      return (-1);
    }
  } else {			/* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
      send_to_char(buf2, ch);
      return (-1);
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return (door);

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return (-1);
  }
}


int has_key(struct char_data *ch, obj_vnum key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return (1);

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return (1);

  return (0);
}



#define NEED_OPEN	(1 << 0)
#define NEED_CLOSED	(1 << 1)
#define NEED_UNLOCKED	(1 << 2)
#define NEED_LOCKED	(1 << 3)

const char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;

  sprintf(buf, "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]) != NULL)
      if (back->to_room != ch->in_room)
	back = 0;

  switch (scmd) {
  case SCMD_OPEN:
  case SCMD_CLOSE:
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	  (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back) {
    sprintf(buf, "The %s is %s%s from the other side.",
	 (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
	    (scmd == SCMD_CLOSE) ? "d" : "ed");
    if (world[EXIT(ch, door)->to_room].people) {
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
    }
  }
}


int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int scmd)
{
  int percent;

  percent = number(1, 101);
  if (!GET_SKILL(ch, SKILL_PICK_LOCK)) percent = 101;
  
  if (scmd == SCMD_PICK) {
    if (keynum <= 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (pickproof)
      send_to_char("It resists your attempts to pick it.\r\n", ch);
    else if (percent > 60 + 2 * GET_SKILL(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else
      return (1);
    return (0);
  }
  return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			OBJVAL_FLAGGED(obj, CONT_CLOSEABLE)) :\
			(EXIT_FLAGGED(EXIT(ch, door), EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!OBJVAL_FLAGGED(obj, CONT_CLOSED)) :\
			(!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!OBJVAL_FLAGGED(obj, CONT_LOCKED)) :\
			(!EXIT_FLAGGED(EXIT(ch, door), EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(OBJVAL_FLAGGED(obj, CONT_PICKPROOF)) : \
			(EXIT_FLAGGED(EXIT(ch, door), EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int door = -1;
  obj_vnum keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char("But it's currently open!\r\n", ch);
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}



ACMD(do_enter)
{
  room_rnum was_in;
  int door;
  struct obj_data *obj = NULL;
  struct follow_type *k, *next;

  one_argument(argument, buf);

  if (*buf) {			/* an argument was supplied, search for door
				 * keyword */
    if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      if (CAN_SEE_OBJ(ch, obj)) {
        if (GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
          if (GET_OBJ_VAL(obj, 0) != NOWHERE && GET_OBJ_LEVEL(obj) <= GET_TOTAL_LEVEL(ch)) {
	    act("$n enters $p.", TRUE, ch, obj, 0, TO_ROOM);
            was_in = ch->in_room;     
            char_from_room(ch);
            char_to_room(ch, GET_OBJ_VAL(obj, 0));
   	    act("$n comes stepping out of a portal.\r\n", TRUE, ch, obj, 0, TO_ROOM);
            look_at_room(ch, 1);

          for (k = ch->followers;k && obj;k = next) {
	      next = k->next;
	      if ((k->follower->in_room == was_in) && GET_POS(k->follower) >= POS_STANDING) 
              {
                if (GET_OBJ_LEVEL(obj) >= GET_TOTAL_LEVEL(k->follower))
		{
   		  act("You cannot enter $p! Try again when you are older.", 
FALSE,k->follower,obj, ch, TO_CHAR);
		  continue;
		}
		act("You follow $N.", FALSE,k->follower,0, ch, TO_CHAR);
                act("$n enters $p.", TRUE, k->follower, obj, 0, TO_ROOM);
                char_from_room(k->follower);
                char_to_room(k->follower, GET_OBJ_VAL(obj, 0));
                act("$n comes stepping out of a portal.", TRUE, k->follower, 0, 0, TO_ROOM);
                look_at_room(k->follower, 1);
               }
           }
       }
       else {
         act("You cannot enter $p! Try again when you are older.", FALSE,ch,obj, ch, TO_CHAR);
       }
       return;

       }
     }  
   }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
    send_to_char("You are already indoors.\r\n", ch);
  else {
    /* try to locate an entrance */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
	      ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("You can't seem to find anything to enter.\r\n", ch);
  }
}


ACMD(do_leave)
{
  int door;

  if (OUTSIDE(ch))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
	    !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You are already standing.\r\n", ch);
    break;
  case POS_SITTING:
    send_to_char("You stand up.\r\n", ch);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    /* Will be sitting after a successful bash and may still be fighting. */
    GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
    break;
  case POS_RESTING:
    send_to_char("You stop resting, and stand up.\r\n", ch);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    do_wake(ch, "", 0, 0);
    break;
  case POS_FIGHTING:
    send_to_char("Do you not consider fighting as standing?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and put your feet on the ground.\r\n", ch);
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  if (IS_AFFECTED(ch, AFF_BERSERK))
  { 
    send_to_char("Rest is not an option you need BLOOD!\r\n", ch);
    return;
  }

  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You sit down.\r\n", ch);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    send_to_char("You stop resting, and sit up.\r\n", ch);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    send_to_char("You have to wake up first.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sit down while fighting? Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and sit down.\r\n", ch);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  if (IS_AFFECTED(ch, AFF_BERSERK))
  { 
    send_to_char("Rest is not an option you need BLOOD!\r\n", ch);
    return;
  }

  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You sit down and rest your tired bones.\r\n", ch);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    send_to_char("You rest your tired bones.\r\n", ch);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    send_to_char("You are already resting.\r\n", ch);
    break;
  case POS_SLEEPING:
    send_to_char("You have to wake up first.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Rest while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and stop to rest your tired bones.\r\n", ch);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  if (IS_AFFECTED(ch, AFF_BERSERK))
  { 
    send_to_char("Rest is not an option you need BLOOD!\r\n", ch);
    return;
  }

  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and lie down to sleep.\r\n", ch);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;
  ACMD(do_look);

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (AWAKE(vict))
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (AFF_FLAGGED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_RESTING;
    }
    if (!self)
      return;
  }
  if (AFF_FLAGGED(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken, stand up, and take a look around.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    do_look(ch, "", 15, 0);
  }
}


ACMD(do_follow)
{
  struct char_data *leader;

  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master)) {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {			/* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\r\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	send_to_char("Sorry, but following in loops is not allowed.\r\n", ch);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
      add_follower(ch, leader);
    }
  }
}

ACMD(do_dismount) {

  if (!PLR_FLAGGED(ch, PLR_HASHORSE) || !AFF_FLAGGED(ch, AFF_MOUNTED)) {
     send_to_char("You aren't riding a horse\r\n", ch);
     return;
  }

  if (AFF_FLAGGED(ch, AFF_MOUNTED)) {
     send_to_char("You gently dismount your steed\r\n", ch);
     REMOVE_BIT(AFF_FLAGS(ch), AFF_MOUNTED);
     GET_HORSEROOM(ch) = ch->in_room;
    }
}
