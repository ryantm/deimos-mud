/*
************************************************************************
*   File: act.item.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
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
#include "constants.h"
#include "assemblies.h"
#include "dg_scripts.h"

/* extern variables */
extern room_rnum donation_room_1;
#if 0
extern room_rnum donation_room_2;  /* uncomment if needed! */
extern room_rnum donation_room_3;  /* uncomment if needed! */
#endif
extern struct obj_data *obj_proto;
extern struct room_data *world;
extern struct index_data *obj_index;

void update_sacrifice(struct char_data *ch);
/* local functions */
int can_take_obj(struct char_data * ch, struct obj_data * obj);
void get_check_money(struct char_data * ch, struct obj_data * obj);
int perform_get_from_room(struct char_data * ch, struct obj_data * obj);
void get_from_room(struct char_data * ch, char *arg, int amount);
void perform_give_gold(struct char_data * ch, struct char_data * vict, int amount);
void perform_give(struct char_data * ch, struct char_data * vict, struct obj_data * obj);
int perform_drop(struct char_data * ch, struct obj_data * obj, byte mode,const char *sname, room_rnum RDR, bool quiet);
#define NOT_QUIET FALSE
#define QUIET !NOT_QUIET
void perform_drop_gold(struct char_data * ch, int amount, byte mode, room_rnum RDR);
struct char_data *give_find_vict(struct char_data * ch, char *arg);
void weight_change_object(struct obj_data * obj, int weight);
void perform_put(struct char_data * ch, struct obj_data * obj, struct obj_data * cont);
void name_from_drinkcon(struct obj_data * obj);
void get_from_container(struct char_data * ch, struct obj_data * cont, char *arg, int mode, int amount);
void name_to_drinkcon(struct obj_data * obj, int type);
void wear_message(struct char_data * ch, struct obj_data * obj, int where);
void perform_wear(struct char_data * ch, struct obj_data * obj, int where);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
void perform_get_from_container(struct char_data * ch, struct obj_data * obj, struct obj_data * cont, int mode);
void perform_remove(struct char_data * ch, int pos);
bool can_wear(struct char_data *ch, struct obj_data *obj);
ACMD(do_assemble);
ACMD(do_remove);
ACMD(do_put);
ACMD(do_get);
ACMD(do_drop);
ACMD(do_give);
ACMD(do_drink);
ACMD(do_eat);
ACMD(do_pour);
ACMD(do_wear);
ACMD(do_wield);
ACMD(do_grab);
ACMD(do_sacrifice);
ACMD(do_castout);
ACMD(do_reelin);
ACMD(do_chopwood);
ACMD(do_mine);
ACMD(do_hunt);
ACMD(do_shoot);

void perform_put(struct char_data * ch, struct obj_data * obj,
		      struct obj_data * cont)
{
  if (!drop_otrigger(obj, ch))
    return;

  if (IS_OBJ_STAT(obj, ITEM_PERMACURSE) && (GET_LEVEL(ch) < LVL_IMMUNE)) {
    sprintf(buf, "You can't put $p away, it is uncontainable !");
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  
  if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
    act("$p cannot be put into $P.", FALSE, ch, obj, cont, TO_CHAR);
    return;
  } 
   
  if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0))
    act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
  else {
    obj_from_char(obj);
    obj_to_obj(obj, cont);

    act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);

    /* Yes, I realize this is strange until we have auto-equip on rent. -gg */
    if (IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_OBJ_STAT(cont, ITEM_NODROP)) {
      SET_BIT(GET_OBJ_EXTRA(cont), ITEM_NODROP);
      act("You get a strange feeling as you put $p in $P.", FALSE,
                ch, obj, cont, TO_CHAR);
    } else
      act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
  }
}


/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_assemble)
{
  long		lVnum = NOTHING;
  struct obj_data *pObject = NULL;

  skip_spaces(&argument);

  if (IN_ROOM_VNUM(ch) != 212) {
    send_to_char("Go to the forgery to do that!\r\n", ch);
    return;
    }
  if (*argument == '\0') {
    sprintf(buf, "%s what?\r\n", CMD_NAME);
    send_to_char(CAP(buf), ch);
    return;
  } else if ((lVnum = assemblyFindAssembly(argument)) < 0) {
    sprintf(buf, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
    send_to_char(buf, ch);
    return;
  } else if (assemblyGetType(lVnum) != subcmd) {
    sprintf(buf, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
    send_to_char(buf, ch);
    return;
  } else if (!assemblyCheckComponents(lVnum, ch)) {
    send_to_char("You haven't all the parts to make that.\r\n", ch);
    return;
  }

  /* Create the assembled object. */
  if ((pObject = read_object(lVnum, VIRTUAL)) == NULL ) {
    sprintf(buf, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
    send_to_char(buf, ch);
    return;
  }

  /* Now give the object to the character. */
  obj_to_char(pObject, ch);

  /* Tell the character they made something. */
  sprintf(buf, "You %s $p.", CMD_NAME);
  act(buf, FALSE, ch, pObject, NULL, TO_CHAR);

  /* Tell the room the character made something. */
  sprintf(buf, "$n %ss $p.", CMD_NAME);
  act(buf, FALSE, ch, pObject, NULL, TO_ROOM);
}

ACMD(do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj, *cont;
  struct char_data *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0, howmany = 1;
  char *theobj, *thecont;

  argument = two_arguments(argument, arg1, arg2);
  one_argument(argument, arg3);

  if (*arg3 && is_number(arg1)) {
    howmany = atoi(arg1);
    theobj = arg2;
    thecont = arg3;
  } else {
    theobj = arg1;
    thecont = arg2;
  }
  obj_dotmode = find_all_dots(theobj);
  cont_dotmode = find_all_dots(thecont);

  if (!*theobj)
    send_to_char("Put what in what?\r\n", ch);
  else if (cont_dotmode != FIND_INDIV)
    send_to_char("You can only put things into one container at a time.\r\n", ch);
  else if (!*thecont) {
    sprintf(buf, "What do you want to put %s in?\r\n",
	    ((obj_dotmode == FIND_INDIV) ? "it" : "them"));
    send_to_char(buf, ch);
  } else {
    generic_find(thecont, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
    if (!cont) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(thecont), thecont);
      send_to_char(buf, ch);
    } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
    else if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
      send_to_char("You'd better open it first!\r\n", ch);
    else {
      if (obj_dotmode == FIND_INDIV) {	/* put <obj> <container> */
	if (!(obj = get_obj_in_list_vis(ch, theobj, ch->carrying))) {
	  sprintf(buf, "You aren't carrying %s %s.\r\n", AN(theobj), theobj);
	  send_to_char(buf, ch);
	} else if (obj == cont)
	  send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
	else {
	  struct obj_data *next_obj;
	  while(obj && howmany--) {
	    next_obj = obj->next_content;
          if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
            send_to_char("You cannot place a container inside another container.\r\n", ch);
            return;
           }
	    perform_put(ch, obj, cont);
	    obj = get_obj_in_list_vis(ch, theobj, next_obj);
	  }
	}
      } else {
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
	      (obj_dotmode == FIND_ALL || isname(theobj, obj->name))) {
	    found = 1;
	    perform_put(ch, obj, cont);
	  }
	}
	if (!found) {
	  if (obj_dotmode == FIND_ALL)
	    send_to_char("You don't seem to have anything to put in it.\r\n", ch);
	  else {
	    sprintf(buf, "You don't seem to have any %ss.\r\n", theobj);
	    send_to_char(buf, ch);
	  }
	}
      }
    }
  }
}



int can_take_obj(struct char_data * ch, struct obj_data * obj)
{
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
    act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
  else if (GET_FREEZE_TEAM(ch) == TEAM_BLUE && IS_OBJ_STAT(obj, ITEM_BLUEFLAG))
  {
    act("$p: you can't take your own flag!", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
  else if (GET_FREEZE_TEAM(ch) == TEAM_RED && IS_OBJ_STAT(obj, ITEM_REDFLAG))
  {
    act("$p: you can't take your own flag!", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }

  return (1);
}


void get_check_money(struct char_data * ch, struct obj_data * obj)
{
  int value = GET_OBJ_VAL(obj, 0);

  if (GET_OBJ_TYPE(obj) != ITEM_MONEY || value <= 0)
    return;

  obj_from_char(obj);
  extract_obj(obj);

  GET_GOLD(ch) += value;

  if (value == 1)
    send_to_char("There was 1 coin.\r\n", ch);
  else {
    sprintf(buf, "There were %d coins.\r\n", value);
    send_to_char(buf, ch);
  }
}


void perform_get_from_container(struct char_data * ch, struct obj_data * obj,
				     struct obj_data * cont, int mode)
{
  if ((GET_OBJ_TYPE(cont) == ITEM_PCCORPSE) && (GET_OBJ_TYPE(cont) != ITEM_ASSCORPSE)) {
   sprintf(buf, "the corpse of %s", GET_NAME(ch));
   if (strcmp(buf, cont->short_description) != 0) {
    send_to_char("This is not your corpse!\r\n", ch);
    return;
   }
 }

  if (GET_OBJ_TYPE(cont) == ITEM_ASSCORPSE) {
   sprintf(buf, "the corpse of %s", GET_NAME(ch));
   if (!PLR_FLAGGED(ch, PLR_ASSASSIN) && (strcmp(buf, cont->short_description) != 0)) {
     send_to_char("You must be an assassin to loot an assassin corpse.\r\n",ch);
     return;
   }
 }

  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
    else if (get_otrigger(obj, ch)) {
      obj_from_obj(obj);
      obj_to_char(obj, ch);
      act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
      act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
      get_check_money(ch, obj);
      if(GET_OBJ_TYPE(cont) == ITEM_ASSCORPSE)
      {
	send_to_char("As you take the item from the corpse, its properties alter slightly.\r\n",ch);
        GET_OBJ_TYPE(cont) = ITEM_PCCORPSE;
      }
    }
  }
}


void get_from_container(struct char_data * ch, struct obj_data * cont,
			     char *arg, int mode, int howmany)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0;
  bool asstest = FALSE;

  obj_dotmode = find_all_dots(arg);

  if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
    act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
      sprintf(buf, "There doesn't seem to be %s %s in $p.", AN(arg), arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else {
      struct obj_data *obj_next;
      while(obj && howmany--) {
	asstest = FALSE;
        obj_next = obj->next_content;
        if (GET_OBJ_TYPE(cont) == ITEM_ASSCORPSE)
            asstest = TRUE;
        perform_get_from_container(ch, obj, cont, mode);
        if (asstest && GET_OBJ_TYPE(cont) == ITEM_PCCORPSE)
		break;
	
        obj = get_obj_in_list_vis(ch, arg, obj_next);
      }
    }
  } else {
    if (obj_dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = cont->contains; obj; obj = next_obj) {
      asstest = FALSE;
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;

        if (GET_OBJ_TYPE(cont) == ITEM_ASSCORPSE)
            asstest = TRUE;

	perform_get_from_container(ch, obj, cont, mode);

        if (asstest && GET_OBJ_TYPE(cont) == ITEM_PCCORPSE)
		break;

      }
    }
  /* Logging a get all if NOT autolooting - also omit npcs */
  if (found && !PLR_FLAGGED2(ch, PLR2_LOOTING) && !IS_NPC(ch)) {
    sprintf(buf, "%s got all from %s", GET_NAME(ch), cont->short_description);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
    }

    if (!found) {
      if (obj_dotmode == FIND_ALL)
	act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
      else {
	sprintf(buf, "You can't seem to find any %ss in $p.", arg);
	act(buf, FALSE, ch, cont, 0, TO_CHAR);
      }
    }
  }
}


int perform_get_from_room(struct char_data * ch, struct obj_data * obj)
{
  if (can_take_obj(ch, obj) && get_otrigger(obj, ch)) {
    obj_from_room(obj);
    obj_to_char(obj, ch);
    act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
    get_check_money(ch, obj);
    return (1);
  }
  return (0);
}


void get_from_room(struct char_data * ch, char *arg, int howmany)
{
  struct obj_data *obj, *next_obj;
  int dotmode, found = 0;

  dotmode = find_all_dots(arg);

  if (dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else {
      struct obj_data *obj_next;
      while(obj && howmany--) {
	obj_next = obj->next_content;
        perform_get_from_room(ch, obj);
        obj = get_obj_in_list_vis(ch, arg, obj_next);
      }
    }
  } else {
    if (dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_room(ch, obj);
      }
    }
    if (!found) {
      if (dotmode == FIND_ALL)
	send_to_char("There doesn't seem to be anything here.\r\n", ch);
      else {
	sprintf(buf, "You don't see any %ss here.\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  }
}



ACMD(do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  int cont_dotmode, found = 0, mode;
  struct obj_data *cont;
  struct char_data *tmp_char;

  argument = two_arguments(argument, arg1, arg2);
  one_argument(argument, arg3);

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else if (PLR_FLAGGED2(ch, PLR2_REVIEW))
    send_to_char("Reviewer's can't grab things!\r\n", ch);   
  else if (!*arg1)
    send_to_char("Get what?\r\n", ch);
  else if (!*arg2)
    get_from_room(ch, arg1, 1);
  else if (is_number(arg1) && !*arg3)
    get_from_room(ch, arg2, atoi(arg1));
  else {
    int amount = 1;
    if (is_number(arg1)) {
      amount = atoi(arg1);
      strcpy(arg1, arg2);
      strcpy(arg2, arg3);
    }
    cont_dotmode = find_all_dots(arg2);
    if (cont_dotmode == FIND_INDIV) {
      mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont) {
	sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
	send_to_char(buf, ch);
      } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER &&
        GET_OBJ_TYPE(cont) != ITEM_PCCORPSE &&
        GET_OBJ_TYPE(cont) != ITEM_ASSCORPSE)
	act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
      else
	get_from_container(ch, cont, arg1, mode, amount);
    } else {
      if (cont_dotmode == FIND_ALLDOT && !*arg2) {
	send_to_char("Get from all of what?\r\n", ch);
	return;
      }
      for (cont = ch->carrying; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    found = 1;
	    get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    found = 1;
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  }
	}
      for (cont = world[ch->in_room].contents; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
	    found = 1;
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    found = 1;
	  }
	}
      if (!found) {
	if (cont_dotmode == FIND_ALL)
	  send_to_char("You can't seem to find any containers.\r\n", ch);
	else {
	  sprintf(buf, "You can't seem to find any %ss here.\r\n", arg2);
	  send_to_char(buf, ch);
	}
      }
    }
  }
}


void perform_drop_gold(struct char_data * ch, int amount,
		            byte mode, room_rnum RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
  else if (GET_GOLD(ch) < amount)
    send_to_char("You don't have that many coins!\r\n", ch);
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE);	/* to prevent coin-bombing */
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char("You throw some gold into the air where it disappears in a puff of smoke!\r\n", ch);
	act("$n throws some gold into the air where it disappears in a puff of smoke!",
	    FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, RDR);
	act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
      } else {
        if (!drop_wtrigger(obj, ch)) {
          extract_obj(obj);
          return;
        }
	send_to_char("You drop some gold.\r\n", ch);
	sprintf(buf, "$n drops %s.", money_desc(amount));
	act(buf, TRUE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, ch->in_room);
      }
    } else {
      sprintf(buf, "$n drops %s which disappears in a puff of smoke!",
	      money_desc(amount));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You drop some gold which disappears in a puff of smoke!\r\n", ch);
    }
    GET_GOLD(ch) -= amount;
  }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int perform_drop(struct char_data * ch, struct obj_data * obj,
		     byte mode, const char *sname, room_rnum RDR, bool quiet)
{
  int value;

  if (!drop_otrigger(obj, ch))
    return 0;
  if ((mode == SCMD_DROP) && !drop_wtrigger(obj, ch))
    return 0;

  if (IS_OBJ_STAT(obj, ITEM_NODROP) && (GET_LEVEL(ch) < LVL_IMMUNE)) {
    sprintf(buf, "You can't %s $p, it must be CURSED!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }

  if (IS_OBJ_STAT(obj, ITEM_PERMACURSE) && (GET_LEVEL(ch) < LVL_IMMUNE)) {
    sprintf(buf, "You can't %s $p, it is undroppable!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }

  if ((mode == SCMD_DONATE) && IS_OBJ_STAT(obj, ITEM_NODONATE)) {
    send_to_char("You cannot donate this item!\r\n", ch);
    return (0); 
  }
  if ((mode == SCMD_JUNK) && GET_OBJ_TYPE(obj) == ITEM_PCCORPSE) {
    send_to_char("You cannot junk a players corpse, bucko!\r\n", ch);
    return (0);
  }
  if (quiet == NOT_QUIET)
  {
  sprintf(buf, "You %s $p.%s", sname, VANISH(mode));
  act(buf, FALSE, ch, obj, 0, TO_CHAR);
  sprintf(buf, "$n %ss $p.%s", sname, VANISH(mode));
  act(buf, TRUE, ch, obj, 0, TO_ROOM);
  }

  if (GET_OBJ_VNUM(obj) == 11007)
    {
      sprintf(buf, "echo \"`date` item junked by %s\" >> wear_log", GET_NAME(ch));
      system(buf);
    }


  obj_from_char(obj);



  switch (mode) {
  case SCMD_DROP:
    obj_to_room(obj, ch->in_room);
    return (1);
  case SCMD_DONATE:
    obj_to_room(obj, RDR);
    act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
    return (1);
  case SCMD_JUNK:
    value = MAX(1, MIN(200, GET_OBJ_COST(obj) / 16));
    extract_obj(obj);
    return (value);
  default:
    log("SYSERR: Incorrect argument %d passed to perform_drop.", mode);
    break;
  }

  return (0);
}



ACMD(do_drop)
{
  struct obj_data *obj, *next_obj;
  room_rnum RDR = 0; int count, temp;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0, multi;
  const char *sname; char *short_desc;

  switch (subcmd) {
  case SCMD_JUNK:
    sname = "junk";
    mode = SCMD_JUNK;
    break;
  case SCMD_DONATE:
    sname = "donate";
    mode = SCMD_DONATE;
    RDR = real_room(donation_room_1);
    break;
    /*if (RDR == NOWHERE) {
      send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
      return;
    }
    break;*/
  default:
    sname = "drop";
    break;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "What do you want to %s?\r\n", sname);
    send_to_char(buf, ch);
    return;
  } else if (is_number(arg)) {
    multi = atoi(arg);
    one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
      perform_drop_gold(ch, multi, mode, RDR);
    else if (multi <= 0)
      send_to_char("Yeah, that makes sense.\r\n", ch);
    else if (!*arg) {
      sprintf(buf, "What do you want to %s %d of?\r\n", sname, multi);
      send_to_char(buf, ch);
    } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
      send_to_char(buf, ch);
    } else {
      do {
        next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
        amount += perform_drop(ch, obj, mode, sname, RDR, NOT_QUIET);
        obj = next_obj;
      } while (obj && --multi);
    }
  } else {
    dotmode = find_all_dots(arg);

    /* Can't junk or donate all */
    if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
      if (subcmd == SCMD_JUNK)
        send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
      else
        send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
      return;
    }
    if (dotmode == FIND_ALL) {
      if (!ch->carrying)
        send_to_char("You don't seem to be carrying anything.\r\n", ch);
      else
        for (obj = ch->carrying; obj; obj = next_obj) {
          next_obj = obj->next_content;
          amount += perform_drop(ch, obj, mode, sname, RDR,NOT_QUIET);
        }
        if (!IS_NPC(ch)) {
          sprintf(buf, "%s dropped all inventory.", GET_NAME(ch));
          mudlog(buf, BRF, LVL_IMPL, TRUE);
        }
    } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
        sprintf(buf, "What do you want to %s all of?\r\n", sname);
        send_to_char(buf, ch);
        return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
        send_to_char(buf, ch);
        return;
      }
      count = 0, temp = 0;
      if (obj) short_desc = strdup(obj->short_description);
      while (obj) {
        temp = amount;
        next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
        amount += perform_drop(ch, obj, mode, sname, RDR, QUIET);
        if (temp != amount)
           count++;
        obj = next_obj;
      }
      if (count > 0) {
        sprintf(buf, "You %s %d %s.%s", sname, count, short_desc, VANISH(mode));
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
        sprintf(buf, "$n %ss %d %s.%s", sname,count,short_desc,VANISH(mode));
        act(buf, TRUE, ch, 0, 0, TO_ROOM);
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
      } else
        amount += perform_drop(ch, obj, mode, sname, RDR, NOT_QUIET);
    }
  }

  if (amount && (subcmd == SCMD_JUNK)) {
    send_to_char("You have been rewarded by the gods!\r\n", ch);
    act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
    GET_GOLD(ch) += amount;
  }
}


void perform_give(struct char_data * ch, struct char_data * vict,
		       struct obj_data * obj)
{
  if (IS_OBJ_STAT(obj, ITEM_NODROP) && (GET_LEVEL(ch) < LVL_IMMUNE)) {
    act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_OBJ_STAT(obj, ITEM_PERMACURSE) && (GET_LEVEL(ch) < LVL_IMMUNE)) {
    sprintf(buf, "You can't give $p, it is ungiveable!");
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
    act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
    act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (!give_otrigger(obj, ch, vict) || !receive_mtrigger(vict, ch, obj))
    return;

  if (GET_POS(vict) <= POS_STUNNED)
  {
    act("$E can't receive objects while dead.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  
  obj_from_char(obj);
  obj_to_char(obj, vict);
  act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
  act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
  act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);
}

/* utility function for give */
struct char_data *give_find_vict(struct char_data * ch, char *arg)
{
  struct char_data *vict;

  if (!*arg) {
    send_to_char("To who?\r\n", ch);
    return (NULL);
  } else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
    send_to_char(NOPERSON, ch);
    return (NULL);
  } else
    return (vict);
}


void perform_give_gold(struct char_data * ch, struct char_data * vict,
		            int amount)
{
  if (amount <= 0) {
    send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
    return;
  }
  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_DEITY))) 
  {
    send_to_char("You don't have that many coins!\r\n", ch);
    return;
  }
  if (GET_POS(vict) <= POS_STUNNED)
  {
    act("$E can't receive objects while dead.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }

  send_to_char(OK, ch);
  sprintf(buf, "&Y$n gives you %d gold coin%s.&n", amount, amount == 1 ? "" : "s");
  act(buf, FALSE, ch, 0, vict, TO_VICT);
  sprintf(buf, "&Y$n gives %s to $N.&n", money_desc(amount));
  act(buf, TRUE, ch, 0, vict, TO_NOTVICT);
  if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))
    GET_GOLD(ch) -= amount;
  GET_GOLD(vict) += amount;

  bribe_mtrigger(vict, ch, amount);
}


ACMD(do_give)
{
  int amount, dotmode;
  struct char_data *vict = NULL;
  struct obj_data *obj, *next_obj;

  argument = one_argument(argument, arg);

  if (!*arg)
    send_to_char("Give what to who?\r\n", ch);
  else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg)) {
      one_argument(argument, arg);
      if ((vict = give_find_vict(ch, arg)) != NULL)
        perform_give_gold(ch, vict, amount);
      return;
    } else if (!*arg) {	/* Give multiple code. */
      sprintf(buf, "What do you want to give %d of?\r\n", amount);
      send_to_char(buf, ch);
    } else if (!(vict = give_find_vict(ch, argument))) {
      return;
    } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
      send_to_char(buf, ch);
    } else if (PRF_FLAGGED2(vict, PRF2_NOGIVE)) {
      act("&n$N currently is not accepting items.&n", TRUE, ch, 0, vict, TO_CHAR);
    } else {
      while (obj && amount--) {
        next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
        perform_give(ch, vict, obj);
        obj = next_obj;
      }
    }
  } else {
    one_argument(argument, buf1);
    if (!(vict = give_find_vict(ch, buf1)))
      return;
  
    if (PRF_FLAGGED2(vict, PRF2_NOGIVE)) {
      act("&n$N currently is not accepting items.&n", TRUE, ch, 0, vict, TO_CHAR);
      return;
    }

    dotmode = find_all_dots(arg);
    if (dotmode == FIND_INDIV) {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
        send_to_char(buf, ch);
      } else
        perform_give(ch, vict, obj);
    } else {
      if (dotmode == FIND_ALLDOT && !*arg) {
        send_to_char("All of what?\r\n", ch);
        return;
      }
      if (!ch->carrying)
        send_to_char("You don't seem to be holding anything.\r\n", ch);
      else
        for (obj = ch->carrying; obj; obj = next_obj) {
          next_obj = obj->next_content;
          if (CAN_SEE_OBJ(ch, obj) && ((dotmode == FIND_ALL || isname(arg, obj->name))))
            perform_give(ch, vict, obj);
        }
      if (!IS_NPC(ch)) {
        sprintf(buf, "%s gave all to %s.", GET_NAME(ch), GET_NAME(vict));
        mudlog(buf, BRF, LVL_IMPL, TRUE);
      }
    }
  }
}



void weight_change_object(struct obj_data * obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (obj->in_room != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("SYSERR: Unknown attempt to subtract weight from an object.");
  }
}



void name_from_drinkcon(struct obj_data * obj)
{
  int i;
  char *new_name;

  for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);

  if (*((obj->name) + i) == ' ') {
    new_name = str_dup((obj->name) + i + 1);
    if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
      free(obj->name);
    obj->name = new_name;
  }
}



void name_to_drinkcon(struct obj_data * obj, int type)
{
  char *new_name;

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", drinknames[type], obj->name);
  if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}



ACMD(do_drink)
{
  struct obj_data *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument(argument, arg);

  if (IS_NPC(ch))	/* Cannot use GET_COND() on mobs. */
    return;

  if (!*arg) {
    send_to_char("Drink from what?\r\n", ch);
    return;
  }
  if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      send_to_char("You can't find it!\r\n", ch);
      return;
    } else
      on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
    send_to_char("You can't drink from that!\r\n", ch);
    return;
  }
  if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
    send_to_char("You have to be holding that to drink from it.\r\n", ch);
    return;
  }
  if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
    /* The pig is drunk */
    send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
    act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {
    send_to_char("Your stomach can't contain anymore!\r\n", ch);
    return;
  }
  if (!GET_OBJ_VAL(temp, 1)) {
    send_to_char("It's empty.\r\n", ch);
    return;
  }
  if (subcmd == SCMD_DRINK) {
    sprintf(buf, "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM);

    sprintf(buf, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);

    if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
    else
      amount = number(3, 10);

  } else {
    act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
    sprintf(buf, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);
    amount = 1;
  }

  amount = MIN(amount, GET_OBJ_VAL(temp, 1));

  /* You can't subtract more than the object weighs */
  weight = MIN(amount, GET_OBJ_WEIGHT(temp));

  weight_change_object(temp, -weight);	/* Subtract amount */

  gain_condition(ch, DRUNK,
	 (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] * amount) / 4);

  gain_condition(ch, FULL,
	  (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][FULL] * amount) / 4);

  gain_condition(ch, THIRST,
	(int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount) / 4);

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char("You feel drunk.\r\n", ch);

  if (GET_COND(ch, THIRST) > 20)
    send_to_char("You don't feel thirsty any more.\r\n", ch);

  if (GET_COND(ch, FULL) > 20)
    send_to_char("You are full.\r\n", ch);

  if (GET_OBJ_VAL(temp, 3)) {	/* The shit was poisoned ! */
    send_to_char("Hm...it tasted rather strange...\r\n", ch);
    act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  /* empty the container, and no longer poison. */
  GET_OBJ_VAL(temp, 1) -= amount;
  if (!GET_OBJ_VAL(temp, 1)) {	/* The last bit */
    GET_OBJ_VAL(temp, 2) = 0;
    GET_OBJ_VAL(temp, 3) = 0;
    name_from_drinkcon(temp);
  }
  return;
}



ACMD(do_eat)
{
  struct obj_data *food;
  struct affected_type af;
  int amount;

  one_argument(argument, arg);

  if (IS_NPC(ch))	/* Cannot use GET_COND() on mobs. */
    return;

  if (!*arg) {
    send_to_char("Eat what?\r\n", ch);
    return;
  }
  if (!(food = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return;
  }
  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
			       (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }
  if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_GOD)) {
    send_to_char("You can't eat THAT!\r\n", ch);
    return;
  }
  if (GET_COND(ch, FULL) > 20) {/* Stomach full */
    send_to_char("You are too full to eat more!\r\n", ch);
    return;
  }
  if (subcmd == SCMD_EAT) {
    act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);

  gain_condition(ch, FULL, amount);

  if (GET_COND(ch, FULL) > 20)
    send_to_char("You are full.\r\n", ch);

  if (GET_OBJ_VAL(food, 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    /* The shit was poisoned ! */
    send_to_char("Oops, that tasted rather strange!\r\n", ch);
    act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  if (subcmd == SCMD_EAT)
    extract_obj(food);
  else {
    if (!(--GET_OBJ_VAL(food, 0))) {
      send_to_char("There's nothing left now.\r\n", ch);
      extract_obj(food);
    }
  }
}


ACMD(do_pour)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj = NULL, *to_obj = NULL;
  int amount;

  two_arguments(argument, arg1, arg2);

  if (subcmd == SCMD_POUR) {
    if (!*arg1) {		/* No arguments */
      send_to_char("From what do you want to pour?\r\n", ch);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      send_to_char("You can't find it!\r\n", ch);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
      send_to_char("You can't pour from that!\r\n", ch);
      return;
    }
  }
  if (subcmd == SCMD_FILL) {
    if (!*arg1) {		/* no arguments */
      send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      send_to_char("You can't find it!\r\n", ch);
      return;
    }
    if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
      act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!*arg2) {		/* no 2nd argument */
      act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
      sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
      act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
  }
  if (GET_OBJ_VAL(from_obj, 1) == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {	/* pour */
    if (!*arg2) {
      send_to_char("Where do you want it?  Out or in what?\r\n", ch);
      return;
    }
    if (!str_cmp(arg2, "out")) {
      act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
      act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

      weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); /* Empty */

      GET_OBJ_VAL(from_obj, 1) = 0;
      GET_OBJ_VAL(from_obj, 2) = 0;
      GET_OBJ_VAL(from_obj, 3) = 0;
      name_from_drinkcon(from_obj);

      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
      send_to_char("You can't find it!\r\n", ch);
      return;
    }
    if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
      send_to_char("You can't pour anything into that.\r\n", ch);
      return;
    }
  }
  if (to_obj == from_obj) {
    send_to_char("A most unproductive effort.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(to_obj, 1) != 0) &&
      (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
    send_to_char("There is already another liquid in it!\r\n", ch);
    return;
  }
  if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
    send_to_char("There is no room for more.\r\n", ch);
    return;
  }
  if (subcmd == SCMD_POUR) {
    sprintf(buf, "You pour the %s into the %s.",
	    drinks[GET_OBJ_VAL(from_obj, 2)], arg2);
    send_to_char(buf, ch);
  }
  if (subcmd == SCMD_FILL) {
    act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
    act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
  }
  /* New alias */
  if (GET_OBJ_VAL(to_obj, 1) == 0)
    name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

  /* First same type liq. */
  GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

  /* Then how much to pour */
  GET_OBJ_VAL(from_obj, 1) -= (amount =
			 (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

  GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

  if (GET_OBJ_VAL(from_obj, 1) < 0) {	/* There was too little */
    GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
    amount += GET_OBJ_VAL(from_obj, 1);
    GET_OBJ_VAL(from_obj, 1) = 0;
    GET_OBJ_VAL(from_obj, 2) = 0;
    GET_OBJ_VAL(from_obj, 3) = 0;
    name_from_drinkcon(from_obj);
  }
  /* Then the poison boogie */
  GET_OBJ_VAL(to_obj, 3) =
    (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

  /* And the weight boogie */
  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount);	/* Add weight */
}



void wear_message(struct char_data * ch, struct obj_data * obj, int where)
{
  const char *wear_messages[][2] = {
    {"&w$n lights $p&w and holds it.&n",
    "&wYou light $p&w and hold it.&n"},

    {"&w$n slides $p&w on to $s right ring finger.&n",
    "&wYou slide $p&w on to your right ring finger.&n"},

    {"&w$n slides $p&w on to $s left ring finger.&n",
    "&wYou slide $p&w on to your left ring finger.&n"},

    {"&w$n wears $p&w around $s neck.&n",
    "&wYou wear $p&w around your neck.&n"},

    {"&w$n wears $p&w around $s neck.&n",
    "&wYou wear $p&w around your neck.&n"},

    {"&w$n wears $p&w on $s body.&n",
    "&wYou wear $p&w on your body.&n"},

    {"&w$n wears $p&w on $s head.&n",
    "&wYou wear $p&w on your head.&n"},

    {"&w$n puts $p&w on $s legs.&n",
    "&wYou put $p&w on your legs.&n"},

    {"&w$n wears $p&w on $s feet.&n",
    "&wYou wear $p&w on your feet.&n"},

    {"&w$n puts $p&w on $s hands.&n",
    "&wYou put $p&w on your hands.&n"},

    {"&w$n wears $p&w on $s arms.&n",
    "&wYou wear $p&w on your arms.&n"},

    {"&w$n straps $p&w around $s arm as a shield.&n",
    "&wYou start to use $p&w as a shield.&n"},

    {"&w$n wears $p&w about $s body.&n",
    "&wYou wear $p&w around your body.&n"},

    {"&w$n wears $p&w around $s waist.&n",
    "&wYou wear $p&w around your waist.&n"},

    {"&w$n puts $p&w on around $s right wrist.&n",
    "&wYou put $p&w on around your right wrist.&n"},

    {"&w$n puts $p&w on around $s left wrist.&n",
    "&wYou put $p&w on around your left wrist.&n"},

    {"&w$n wields $p&w.&n",
    "&wYou wield $p&w.&n"},

    {"&w$n grabs $p&w.&n",
    "&wYou grab $p&w.&n"}
  };

  act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}


void perform_wear(struct char_data * ch, struct obj_data * obj, int where)
{
  /*
   * ITEM_WEAR_TAKE is used for objects that do not require special bits
   * to be put into that position (e.g. you can hold any object, not just
   * an object with a HOLD bit.)
   */

  int wear_bitvectors[] = {
    ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
    ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
    ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
    ITEM_WEAR_WIELD, ITEM_WEAR_TAKE
  };

  const char *already_wearing[] = {
    "You're already using a light.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your legs.\r\n",
    "You're already wearing something on your feet.\r\n",
    "You're already wearing something on your hands.\r\n",
    "You're already wearing something on your arms.\r\n",
    "You're already using a shield.\r\n",
    "You're already wearing something about your body.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding a weapon.\r\n",
    "You're already holding something.\r\n"
  };

  if (GET_OBJ_VNUM(obj) == 11007)
    {
      sprintf(buf, "echo \"`date` item worn by %s\" >> wear_log", GET_NAME(ch));
      system(buf);
    }

  /* first, make sure that the wear position is valid. */
  if (!CAN_WEAR(obj, wear_bitvectors[where])) {
    act("&RYou can't wear $p there.&n", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }

  if (!can_wear(ch, obj))
  {
     return;
  }

  /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
  if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
    if (GET_EQ(ch, where))
      where++;

  if (GET_EQ(ch, where)) {
    send_to_char("&R", ch);
    send_to_char(already_wearing[where], ch);
    send_to_char("&n", ch);
    return;
  }

  if (!wear_otrigger(obj, ch, where))
    return;
  wear_message(ch, obj, where);
  obj_from_char(obj);
  equip_char(ch, obj, where);
}



int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg)
{
  int where = -1;

  /* \r to prevent explicit wearing. Don't use \n, it's end-of-array marker. */
  const char *keywords[] = {
    "\r!RESERVED!",
    "finger",
    "\r!RESERVED!",
    "neck",
    "\r!RESERVED!",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "\r!RESERVED!",
    "\r!RESERVED!",
    "\r!RESERVED!",
    "\n"
  };

  if (!arg || !*arg) {
    if (CAN_WEAR(obj, ITEM_WEAR_FINGER))      where = WEAR_FINGER_R;
    if (CAN_WEAR(obj, ITEM_WEAR_NECK))        where = WEAR_NECK_1;
    if (CAN_WEAR(obj, ITEM_WEAR_BODY))        where = WEAR_BODY;
    if (CAN_WEAR(obj, ITEM_WEAR_HEAD))        where = WEAR_HEAD;
    if (CAN_WEAR(obj, ITEM_WEAR_LEGS))        where = WEAR_LEGS;
    if (CAN_WEAR(obj, ITEM_WEAR_FEET))        where = WEAR_FEET;
    if (CAN_WEAR(obj, ITEM_WEAR_HANDS))       where = WEAR_HANDS;
    if (CAN_WEAR(obj, ITEM_WEAR_ARMS))        where = WEAR_ARMS;
    if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))      where = WEAR_SHIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))       where = WEAR_ABOUT;
    if (CAN_WEAR(obj, ITEM_WEAR_WAIST))       where = WEAR_WAIST;
    if (CAN_WEAR(obj, ITEM_WEAR_WRIST))       where = WEAR_WRIST_R;
  } else {
    if (((where = search_block(arg, keywords, FALSE)) < 0) ||
         (*arg=='!')) {
      sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
      send_to_char(buf, ch);
      return -1;
    }
  }

  return (where);
}



ACMD(do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  int where, dotmode, items_worn = 0;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("Wear what?\r\n", ch);
    return;
  }

  if (PLR_FLAGGED2(ch, PLR2_REVIEW)) {
    send_to_char("Reviewer's can't wear things!\r\n", ch);   
    return;
  }

  dotmode = find_all_dots(arg1);

  if (*arg2 && (dotmode != FIND_INDIV)) {
    send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
    return;
  }
  if (dotmode == FIND_ALL) {
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
	items_worn++;
	perform_wear(ch, obj, where);
      }
    }

    if (!items_worn)
      send_to_char("You don't seem to have anything wearable.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg1) {
      send_to_char("Wear all of what?\r\n", ch);
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
      send_to_char(buf, ch);
    } else
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
	if ((where = find_eq_pos(ch, obj, 0)) >= 0  && (can_wear(ch, obj)))
	  perform_wear(ch, obj, where);
	else
	  act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	obj = next_obj;
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
      send_to_char(buf, ch);
    } 
    else if (!can_wear(ch,obj))
    {}
    else {
     
      if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
	perform_wear(ch, obj, where);
      else if (!*arg2)
	act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}



ACMD(do_wield)
{
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Wield what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else {
    if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
      send_to_char("You can't wield that.\r\n", ch);
    if (IS_OBJ_STAT(obj, ITEM_TWO_HANDED) && GET_EQ(ch, WEAR_HOLD))
      send_to_char("Your other hand is already holding something.\r\n", ch);
    if (!can_wear(ch, obj))
      send_to_char("You are not experienced enough to wield this weapon!\r\n", ch);
    else if (GET_OBJ_WEIGHT(obj) > str_app[MIN(GET_STR(ch) + 5, 25)].wield_w && (GET_LEVEL(ch) < LVL_IMMUNE))
    // str_app wield weight lookup - capped at 25 str ( = 40 wt)
      send_to_char("It's too heavy for you to use.\r\n", ch);
    else
      perform_wear(ch, obj, WEAR_WIELD);
  }
}



ACMD(do_grab)
{
  struct obj_data *obj;
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hold what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      perform_wear(ch, obj, WEAR_LIGHT);
	else if ((GET_EQ(ch, WEAR_WIELD) && IS_OBJ_STAT(wielded, ITEM_TWO_HANDED))) {
    send_to_char("You cannot hold something as you are wielding a two handed weapon.\r\n", ch);
	return;
	}
    else {
      if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
      GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
	  GET_OBJ_TYPE(obj) != ITEM_POTION)
	send_to_char("You can't hold that.\r\n", ch);
      else
	perform_wear(ch, obj, WEAR_HOLD);
    }
  }
}



void perform_remove(struct char_data * ch, int pos)
{
  struct obj_data *obj;

  if (!(obj = GET_EQ(ch, pos)))
    log("SYSERR: perform_remove: bad pos %d passed.", pos);
   else if (IS_OBJ_STAT(obj, ITEM_NODROP) && (GET_LEVEL(ch) < LVL_IMMUNE))
    act("&RYou can't remove $p&R, it must be CURSED!&n", FALSE, ch, obj, 0, TO_CHAR);
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    act("&R$p&R: you can't carry that many items!&n", FALSE, ch, obj, 0, TO_CHAR);
  else {
    if (!remove_otrigger(obj, ch))
      return;
    obj_to_char(unequip_char(ch, pos), ch);
    act("&wYou stop using $p.&n", FALSE, ch, obj, 0, TO_CHAR);
    act("&w$n stops using $p.&n", TRUE, ch, obj, 0, TO_ROOM);
  }
}



ACMD(do_remove)
{
  int i, dotmode, found;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("&RRemove what?&n\r\n", ch);
    return;
  }
  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found = 0;
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
	perform_remove(ch, i);
	found = 1;
      }
    if (!found)
      send_to_char("&RYou're not using anything.&n\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg)
      send_to_char("&RRemove all of what?&n\r\n", ch);
    else {
      found = 0;
      for (i = 0; i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
	    isname(arg, GET_EQ(ch, i)->name)) {
	  perform_remove(ch, i);
	  found = 1;
	}
      if (!found) {
	sprintf(buf, "&RYou don't seem to be using any %ss.&n\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  } else {
    /* Returns object pointer but we don't need it, just true/false. */
    if (!get_object_in_equip_vis(ch, arg, ch->equipment, &i)) {
      sprintf(buf, "&RYou don't seem to be using %s %s.&n\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else
      perform_remove(ch, i);
  }
}
ACMD(do_compare)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    struct obj_data *obj1;
    struct obj_data *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying)))
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content)
        {
            if ( !((obj2->obj_flags.type_flag == ITEM_WEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_FIREWEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_ARMOR))
            &&   CAN_SEE_OBJ(ch, obj2)
            &&   obj1->obj_flags.type_flag == obj2->obj_flags.type_flag
            && CAN_GET_OBJ(ch, obj2) )
                break;
        }

        if (!obj2)
        {
            send_to_char( "You aren't wearing anything comparable.\n\r",
ch );
            return;
        }
    }
    else
    {
        if (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)))
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }
    }

    msg         = NULL;
    value1      = 0;
    value2      = 0;

    if (obj1 == obj2)
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->obj_flags.type_flag != obj2->obj_flags.type_flag )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch (obj1->obj_flags.type_flag)
        {
        default:
            msg = "You can't compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->obj_flags.value[0];
            value2 = obj2->obj_flags.value[0];
            break;

        case ITEM_WEAPON:
            value1 = obj1->obj_flags.value[1] + obj1->obj_flags.value[2];
            value2 = obj2->obj_flags.value[1] + obj2->obj_flags.value[2];
            break;
        }
    }

    if (!msg)
    {
             if (value1 == value2) msg = "$p and $P look about the same.";
        else if (value1  > value2) msg = "$p looks better than $P.";
        else                         msg = "$p looks worse than $P.";
    }

    act(msg, FALSE, ch, obj1, obj2, TO_CHAR);
    return;
}

ACMD(do_sacrifice) {
   struct obj_data *obj;
   one_argument(argument, arg);
   // note, I like to take care of no arg and wrong args up front, not
   // at the end of a function, lets get the wrongness out of the way :)
   if (!*arg)
   {
     send_to_char("&RWhat do you wish to sacrifice?&n\n\r",ch);
     return;
   }
   // if it's not in the room, we ain't gonna sac it
   if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))  {
     send_to_char("&RYou don't see that here.&n\n\r",ch);  return;  }
   // nifty, got the object in the room, now check its flags
   if (!CAN_WEAR(obj, ITEM_WEAR_TAKE) || GET_OBJ_TYPE(obj) ==
     ITEM_PCCORPSE || GET_OBJ_TYPE(obj) == ITEM_ASSCORPSE) {
     send_to_char("&RYou can't sacrifice THAT!&n\n\r",ch);
     return;
   }
   two_arguments(obj->short_description, buf, buf1);
   if (!strcmp(buf, "corpse") && !strcmp(buf1, "of"))
   { 
    send_to_char("&gYou sacrificed a corpse.&n\r\n", ch);
    update_sacrifice(ch); 
   }
   // seems as if everything checks out eh? ok now do it
   act("&g$n sacrifices $p.&n", FALSE, ch, obj, 0, TO_ROOM);
   act("&gYou sacrifice $p to your deity.&n\r\nThe gods give you one gold coin for your humility.&n", FALSE, ch, obj, 0, TO_CHAR);
   GET_GOLD(ch) += 1; 
/* simple addition that gives one coin to the char's inventory -spl */
   extract_obj(obj);
}

ACMD(do_chopwood)
{
  struct obj_data *axe;
  int fail, lose;

  if (PLR_FLAGGED2(ch, PLR2_CUTTING)) {
    send_to_char("You have already begun cutting down a tree!\r\n", ch);
    return;
  }

  if (!(axe = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(axe) != ITEM_AXE)) {
    send_to_char("You need to be holding an axe first.\r\n", ch);
    return;
  }

  if (SECT(ch->in_room) != SECT_FOREST) {
    send_to_char("This is not a forest!\r\n", ch);
    return;
  }

 /* Always have a check, 10% chance of losing your weapon */
  lose = number(1,10);
  if (lose == 5) {
   extract_obj(axe);
   send_to_char("You find a tree and begin chopping, but the axe breaks!\r\n", ch);
   return;
  }

  fail = number(1, 10);

 /* 30% chance of not finding a tree, just to slow people down */
  if (fail <= 3) {
    send_to_char("You begin chopping, but realize the wood is rotten.\r\nTry again.\r\n", ch);
    act("$n realizes that the tree is rotten!\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* Ok, now they've gone through the checks, now set them cutting */
  SET_BIT(PLR_FLAGS2(ch), PLR2_CUTTING);
  send_to_char("You find a nice big tree, ready to be killed.\r\n", ch);
  act("$n begins chopping away on a tree.\r\n", FALSE, ch, 0, 0, TO_ROOM);
  return;

}

ACMD(do_castout)
{
  struct obj_data *pole;
  int fail;

  if (PLR_FLAGGED2(ch, PLR2_FISHING) || MOB_FLAGGED2(ch, PLR2_FISHING)) {
    send_to_char("You are already fishing!\r\n", ch);
    return;

  }
  if (!(pole = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(pole) != ITEM_POLE)) {
    send_to_char("You need to be holding a fishing pole first.\r\n", ch);
    return;
  }

  if (!ROOM_FLAGGED(ch->in_room, ROOM_FISH)) {
    send_to_char("This is not a good place to fish, you'll want to find a"
                 " better spot.\r\n", ch);
    return;
  }


  fail = number(1, 10);

 /* 30% of failing */
  if (fail <= 3) {
    send_to_char("You pull your arm back and try to cast out your line, but "
                 "it gets all tangled up.\r\nTry again.\r\n", ch);

    act("$n pulls $s arm back, trying to cast $s fishing line out into the "
        "water,\r\nbut ends up just a bit tangled.\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* Ok, now they've gone through the checks, now set them fishing */
  SET_BIT(PLR_FLAGS2(ch), PLR2_FISHING);

  send_to_char("You cast your line out into the water, hoping for a bite.\r\n", ch);
  act("$n casts $s line out into the water, hoping to catch some food.\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  return;

}

ACMD(do_reelin)
{
  int success, f_num, fish_num, lose;
  struct obj_data *fish, *pole;

  if (!(PLR_FLAGGED2(ch, PLR2_FISHING) || MOB_FLAGGED2(ch, PLR2_FISHING))) {
    send_to_char("You aren't even fishing!\r\n", ch);
    return;
  }

  if (!(pole = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(pole) != ITEM_POLE)) {
    send_to_char("You need to be holding a fishing pole first.\r\n", ch);
    return;
  }

  lose = number(1,100);
  if (lose == 1) {
   extract_obj(pole);
   send_to_char("A fish pulls the fishing pole out of your hands!\r\n", ch);
    act("A fish pulls $n's fishing pole right out of $s hands!\r\n", FALSE, ch, 0, 0, TO_ROOM);
   REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISHING);
   REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISH_ON);
   return;
  }

  if (!(PLR_FLAGGED2(ch, PLR2_FISH_ON) || MOB_FLAGGED2(ch, PLR2_FISH_ON))) {
    send_to_char("You reel in your line, but alas... nothing on the end.\r\n"
                 "Better luck next time.\r\n", ch);
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISHING);
    act("$n reels $s line in, but with nothing on the end.\r\n",
        FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* Ok, they are fishing and have a fish on */
  success = number(1, 8);


  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISHING);
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISH_ON);


  if (success <= 6) {
    send_to_char("You reel in your line, putting up a good fight, but you "
                 "lose him!\r\nTry again?\r\n", ch);
    act("$n reels $s line in, fighting with whatever is on the end, but loses "
        "the catch.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* We used object vnums 10030-10050 for our fish that people could
   * catch. The below numbers reflect that use. If you wish to change

   * the vnums of the fish, just change the numbers below. You can
   * see that we seperated the type of fish by freshwater and salt
   * water.
   */
  if (ROOM_FLAGGED(ch->in_room, ROOM_FISH)) {
    fish_num = 2599;
    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    sprintf(buf, "You reel in %s! Nice catch!\r\n", fish->short_description);
    send_to_char(buf, ch);
    act("$n reels in a big one! What a nice catch!\r\n", FALSE, ch, 0, 0, TO_ROOM);
    obj_to_char(fish, ch);
    return;
  } else
  send_to_char("You should never see this message, please report it.\r\n", ch);

  return;
}



ACMD(do_mine)
{
  struct obj_data *hammer;
  int fail, lose;

  if (PLR_FLAGGED2(ch, PLR2_MINING)) {
    send_to_char("You have already begun mining!\r\n", ch);
    return;

  }
  if (!(hammer = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(hammer) != ITEM_HAMMER)) {
    send_to_char("You need to be holding an hammer first.\r\n", ch);
    return;
  }
  if (SECT(ch->in_room) != SECT_MINE) {
    send_to_char("This is not a mine!\r\n", ch);
    return;
  }

  lose = number(1,10);
  if (lose == 5) {
   extract_obj(hammer);
   send_to_char("You begin mining, but your hammer breaks!\r\n", ch);
   return;
  }

  fail = number(1, 10);
  if (fail <= 2) {

    send_to_char("You cannot find anything to mine.\r\nTry again.\r\n", ch);

    act("$n cannot find anything to mine!\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  /* Ok, now they've gone through the checks, now set them mining */
  SET_BIT(PLR_FLAGS2(ch), PLR2_MINING);

  send_to_char("You begin mining on a piece of rock.\r\n", ch);
  act("$n begins mining on a piece of rock.\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  return;

}


ACMD(do_hunt)
{
  struct obj_data *bow;
  int fail;

  if (PLR_FLAGGED2(ch, PLR2_HUNTING)) {
    send_to_char("You have already begun hunting!\r\n", ch);
    return;

  }
  if (!(bow = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(bow) != ITEM_BOW)) {
    send_to_char("You need to be holding a bow first.\r\n", ch);
    return;
  }
  if (SECT(ch->in_room) != SECT_HUNTGROUNDS) {
    send_to_char("This is not a hunting ground!\r\n", ch);
    return;
  }

  fail = number(1, 10);
  if (fail <= 2) {

    send_to_char("You cannot find anything to hunt.\r\nTry again.\r\n", ch);

    act("$n cannot find anything to hunt!\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  /* Ok, now they've gone through the checks, now set them hunting */
  SET_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);

  send_to_char("You find a target and begin hunting it.\r\n", ch);
  act("$n begins hunting.\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  return;

}

ACMD(do_shoot)
{
  int success, m_num, mob_num, lose;
  struct obj_data *prey, *bow;

  if (!PLR_FLAGGED2(ch, PLR2_HUNTING)) {
    send_to_char("You aren't even hunting!\r\n", ch);
    return;
  }

  if (!(bow = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(bow) != ITEM_BOW)) {
    send_to_char("You need to be holding a bow first.\r\n", ch);
    return;
  }

  if (!PLR_FLAGGED2(ch, PLR2_KILLMOB)) {
    send_to_char("You shoot at nothing, and hit nothing...\r\n", ch);
    REMOVE_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);
    act("$n shoots $s bow, but hit just air.\r\n",
        FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  lose = number(1,10);
  if (lose == 5) {
   extract_obj(bow);
   send_to_char("The bow suddenly snaps in two!\r\n", ch);
   REMOVE_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);
   REMOVE_BIT(PLR_FLAGS2(ch), PLR2_KILLMOB);
   return;
  }


  /* Ok, they are fishing and have a fish on */
  success = number(1, 10);


  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_KILLMOB);


  if (success <= 7) {
    send_to_char("You fire at your target, but alas, you miss!\r\nTry again?\r\n", ch);
    act("$n shoots $s bow, but misses entirely.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* We used object vnums 10030-10050 for our fish that people could
   * catch. The below numbers reflect that use. If you wish to change

   * the vnums of the fish, just change the numbers below. You can
   * see that we seperated the type of fish by freshwater and salt
   * water.
   */
  if (SECT(ch->in_room) == SECT_HUNTGROUNDS) {
    mob_num = number(2561, 2566);
    m_num = real_object(mob_num);
    prey = read_object(m_num, REAL);
    sprintf(buf, "You shoot, hit, and collect %s! Nice job!\r\n", prey->short_description);
    send_to_char(buf, ch);
    obj_to_char(prey, ch);
    return;
  } else
  send_to_char("You should never see this message, please report it.\r\n", ch);

  return;
}


bool can_wear(struct char_data *ch, struct obj_data *obj)
{
 if (IS_NPC(ch)) return TRUE;

 if (GET_OBJ_LEVEL(obj) <= 60 && GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
 {
    send_to_char("You are not experienced enough to use this object\r\n", ch);
    return FALSE;
 }
    
 if (GET_TOTAL_LEVEL(ch) < GET_OBJ_LEVEL(obj))
 {
    send_to_char("You are not experienced enough to use this object\r\n", ch);
    return FALSE;
 }
 if(OBJ_FLAGGED(obj, ITEM_IMMONLY) && GET_LEVEL(ch) < LVL_IMMORT)
 {
    send_to_char("Only a god use an object such as this!\r\n",ch);
    return FALSE;
 }

 if(OBJ_FLAGGED(obj, ITEM_ANTI_GOOD) && GET_ALIGNMENT(ch) > 100) 
 {
    send_to_char("You are too good to use this.\r\n",ch);
    return FALSE;
 }
 if(OBJ_FLAGGED(obj, ITEM_ANTI_EVIL) && GET_ALIGNMENT(ch) < -100) 
 {
    send_to_char("You are too evil to use this.\r\n",ch);
    return FALSE;
 }
 if(OBJ_FLAGGED(obj, ITEM_ANTI_NEUTRAL) 
    && GET_ALIGNMENT(ch) < 100 && GET_ALIGNMENT(ch) > -100 ) 
 {
    send_to_char("Your neutrality makes this hard to use.\r\n",ch);
    return FALSE;
 }
 if(IS_OBJ_STAT(obj, ITEM_TWO_HANDED) && GET_EQ(ch, WEAR_HOLD))
 {
    send_to_char("Your hands seem full, and this weapon awefully  big.\r\n",ch);
    return FALSE;
 }

 if(
   (OBJ_FLAGGED(obj, ITEM_ANTI_WARRIOR) && GET_CLASS(ch) == CLASS_WARRIOR)
 ||(OBJ_FLAGGED(obj, ITEM_ANTI_THIEF) && GET_CLASS(ch) == CLASS_THIEF)
 ||(OBJ_FLAGGED(obj, ITEM_ANTI_CLERIC) && GET_CLASS(ch) == CLASS_CLERIC)
 ||(OBJ_FLAGGED(obj, ITEM_ANTI_MAGIC_USER) && GET_CLASS(ch) == CLASS_MAGIC_USER)
 ||(OBJ_FLAGGED(obj, ITEM_ASSASSIN_ONLY) && !PLR_FLAGGED(ch, PLR_ASSASSIN))
 )
 {
    send_to_char("You class cannot use this!\r\n",ch);
    return FALSE;
 }

 return TRUE;
}
