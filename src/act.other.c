/*
************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __ACT_OTHER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char *class_abbrevs[];
extern room_vnum mortal_start_room;
extern int free_rent;
extern int pt_allowed;
extern int max_filesize;
extern int nameserver_is_slow;
extern int auto_save;
extern int track_through_doors;
extern int xap_objs;
extern struct new_guild_info guilds[];
extern char *smartmud;
extern zone_rnum top_of_zone_table;
extern struct obj_data *object_list;
extern struct index_data *obj_index;
extern char *areligion;
extern char *preligion;
extern char *dreligion;
extern char *rreligion;
extern char *creligion;
extern int file_to_string_alloc(const char *name, char **buf);
extern struct pclean_criteria_data pclean_criteria[];
extern room_vnum frozen_start_room;
/* Romance Module Defines */
#define SAME_SEX_ALLOWED TRUE /* True/False */
#define OUT_OF_WEDLOCK TRUE /* True/False */
#define REWARD_ALLOWED TRUE /* T/F */

/* extern procedures */
void make_corpse(struct char_data * ch);
void symptoms(struct char_data * ch);
struct char_data *is_in_game(int idnum);
void list_skills(struct char_data * ch);
void list_skillst(struct char_data * ch); 
void list_skillsw(struct char_data * ch); 
void list_skillsm(struct char_data * ch); 
void list_skillsc(struct char_data * ch); 
void improve_skill(struct char_data *ch, int skill);
void appear(struct char_data * ch);
void write_aliases(struct char_data *ch);
void perform_immort_vis(struct char_data *ch);
SPECIAL(shop_keeper);
ACMD(do_gen_comm);
void die(struct char_data * ch, struct char_data * killer);
void Crash_rentsave(struct char_data * ch, int cost);
int needed_exp(byte char_class, byte gain_class, int level);
int needed_gold(byte char_class, byte gain_class, int level);
byte get_class_level(struct char_data *ch, byte class);
int rvnum = 0;
char *num_punct(int foo);

/* local functions */
ACMD(do_quit);
ACMD(do_save);
ACMD(do_not_here);
ACMD(do_sneak);
ACMD(do_hide);
ACMD(do_steal);
ACMD(do_practice);
ACMD(do_visible);
ACMD(do_title);
int perform_group(struct char_data *ch, struct char_data *vict);
void print_group(struct char_data *ch);
void autoall(struct char_data *ch);
ACMD(do_group);
ACMD(do_ungroup);
ACMD(do_report);
ACMD(do_split);
ACMD(do_use);
ACMD(do_wimpy);
ACMD(do_display);
ACMD(do_gen_write);
ACMD(do_gen_tog);
ACMD(do_gen_togx);
ACMD(do_mskillset);
ACMD(do_mteach);
ACMD(do_finger);
ACMD(do_level);
ACMD(do_die);
ACMD(do_autoall);
ACMD(do_bandage);
ACMD(do_ride);
C_FUNC(test_func);
C_FUNC(test_func_chain);
ACMD(do_smartmud);
ACMD(do_firefight);
ACMD(do_stop);
ACMD(do_gjob);
ACMD(do_gtitle);
ACMD(do_reimbobj);
void update_sacrifice(struct char_data *ch);
void sort_areligrank(struct char_data *ch);
void sort_preligrank(struct char_data *ch);
void sort_dreligrank(struct char_data *ch);
void sort_rreligrank(struct char_data *ch);
void sort_creligrank(struct char_data *ch);


ACMD(do_quit)
{
  struct descriptor_data *d, *next_d;
  room_rnum start = 200;
  int i;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char("You have to type quit--no less, to quit!\r\n", ch);
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if (IS_SET(ROOM_FLAGS(IN_ROOM(ch)), ROOM_NO_QUIT))
    send_to_char("You are not allowed to quit here!\r\n", ch);
  else if (GET_POS(ch) < POS_STUNNED) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch, NULL);
  } else {
    int loadroom = ch->in_room;
    rvnum = GET_ROOM_VNUM(IN_ROOM(ch));
      if (real_room(rvnum) != NOWHERE) {
        SET_BIT(PLR_FLAGS(ch), PLR_LOADROOM);
        GET_LOADROOM(ch) = rvnum;
        }
    GET_LOADROOM(ch) = world[ch->in_room].number;
    REMOVE_BIT(AFF_FLAGS(ch), AFF_MOUNTED);
   if (PLR_FLAGGED(ch, PLR_HASHORSE))
    GET_HORSEROOM(ch) = real_room(255);
    
   if (PLR_FLAGGED(ch, PLR_NEWBIE)) {
    REMOVE_BIT(PLR_FLAGS(ch), PLR_NEWBIE);
    GET_LOADROOM(ch) = start;
   }

   if (!GET_INVIS_LEV(ch))
      act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s has quit the game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);

   sprintf(buf, "&R%s has left the realm of DeimosMUD.&n", GET_NAME(ch));
   if (!PRF_FLAGGED(ch, PRF_WHOINVIS))
    showplayer(buf, BRF, MAX(0, GET_INVIS_LEV(ch)), TRUE);


    for(i = 0; i == 0 || (pclean_criteria[i].level > pclean_criteria[i - 1].level); i++) 
      if(GET_LEVEL(ch) <= pclean_criteria[i].level)
          break;

    sprintf(buf, "&GYou should log in within %d days to avoid being deleted.&n\r\n", pclean_criteria[i].days);
    send_to_char(buf, ch);

    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (d == ch->desc)
        continue;
      if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
        STATE(d) = CON_DISCONNECT;
    }


    if (free_rent)
      Crash_rentsave(ch, 0);

    extract_char(ch);		/* Char is saved in extract char */

    /* If someone is quitting in their house, let them load back here */
    /* if (ROOM_FLAGGED(loadroom, ROOM_HOUSE)) */
      loadroom = GET_LOADROOM(ch);
      save_char(ch, loadroom);
  }
}



ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  /* Only tell the char we're saving if they actually typed "save" */
  if (cmd) {
    /*
     * This prevents item duplication by two PC's using coordinated saves
     * (or one PC with a house) and system crashes. Note that houses are
     * still automatically saved without this enabled. This code assumes
     * that guest immortals aren't trustworthy. If you've disabled guest
     * immortal advances from mortality, you may want < instead of <=.
     */
    if (auto_save && GET_LEVEL(ch) <= LVL_IMMORT) {
      send_to_char("Saving aliases.\r\n", ch);
      write_aliases(ch);
      rvnum = GET_ROOM_VNUM(IN_ROOM(ch));
    if (real_room(rvnum) != NOWHERE) {
      SET_BIT(PLR_FLAGS(ch), PLR_LOADROOM);
      GET_LOADROOM(ch) = rvnum;
        }  
      return;
    }
    sprintf(buf, "Saving %s and aliases.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
    rvnum = GET_ROOM_VNUM(IN_ROOM(ch));
     if (real_room(rvnum) != NOWHERE) 
     {
        SET_BIT(PLR_FLAGS(ch), PLR_LOADROOM);
        GET_LOADROOM(ch) = rvnum;
     }
  }
    
  write_aliases(ch);
  save_char(ch, IN_ROOM(ch));
  Crash_crashsave(ch);
  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
    House_crashsave(GET_ROOM_VNUM(IN_ROOM(ch)));
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}


/* Function specifically designed for the Mount code *JS* haha, f' you bastards!, hehehe */
ACMD(do_ride)
{
  if (ch->in_room != GET_HORSEROOM(ch)) 
    send_to_char("Your horse is not presently in this room.\r\n", ch);
  else {
    send_to_char("You jump onto your horse, who is grazing on some nearby grass.\r\n", ch);
    SET_BIT(AFF_FLAGS(ch), AFF_MOUNTED);
    GET_HORSEROOM(ch) = real_room(99);
  }
}


ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;
  if (AFF_FLAGGED(ch, AFF_SNEAK))
  {
     send_to_char("You stop sneaking.\r\n", ch);
     REMOVE_BIT(AFF_FLAGS(ch), AFF_SNEAK);
     return;
  }
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SNEAK)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }
  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (AFF_FLAGGED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > 60 + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
  
}



ACMD(do_hide)
{
  byte percent;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_HIDE)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }

  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (AFF_FLAGGED(ch, AFF_HIDE))
    REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > 60 + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
  //improve_skill(ch, SKILL_HIDE);
}


ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_STEAL)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  two_arguments(argument, obj_name, vict_name);

  if (!(vict = get_char_vis(ch, vict_name, FIND_CHAR_ROOM))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  }

  if (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOHIT))
  {
    send_to_char("No.\r\n", ch);
    return;
  }

  if (!CAN_STEAL(ch, vict))
  {
    send_to_char("You are violating assassin rules!\r\n", ch);
    return;
  }
  /* 101% is a complete failure */
  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(vict) < POS_SLEEPING)
    percent = -1;		/* ALWAYS SUCCESS, unless heavy object. */

  if (!pt_allowed && !IS_NPC(vict))
    pcsteal = 1;

  if (!AWAKE(vict))	/* Easier to steal from sleeping people. */
    percent -= 50;

  if (GET_CLASS(ch) == CLASS_THIEF)  /* Thieves can steal easier */
    percent -= 10;

  if (GET_CLASS(ch) == CLASS_MAGIC_USER)  /* Mages can't steal worth shit */
    percent += 30;

  /* NO NO With Imp's and Shopkeepers, and if player thieving is not allowed */
  if (GET_LEVEL(vict) >= LVL_IMMORT || pcsteal ||
      GET_MOB_SPEC(vict) == shop_keeper)
    percent = 101;		/* Failure */


  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(vict, eq_pos) &&
	    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
	  obj = GET_EQ(vict, eq_pos);
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	return;
      } else {			/* It is equipment */
	if ((GET_POS(vict) > POS_STUNNED)) {
	  send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	  obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {			/* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */
      if (AWAKE(vict) && (percent > 60)) 
      {
 	ohoh = TRUE;
	send_to_char("Oops..\r\n", ch);
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
      } else {			/* Steal the item */
	if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
	  if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\r\n", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {			/* Steal some coins */
    if (AWAKE(vict) && (percent > 60)) {
      ohoh = TRUE;
      send_to_char("Oops..\r\n", ch);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
        if (gold > 1) {
	  sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	  send_to_char(buf, ch);
	} else {
	  send_to_char("You manage to swipe a solitary gold coin.\r\n", ch);
	}
      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}

ACMD(do_practice)
{
  if (IS_NPC(ch))
    return;

  if (GET_LEVEL(ch) >= 61) {
    list_skills(ch);
    return;
  }

  one_argument(argument, arg);

  if (*arg)
    send_to_char("You can only practice skills in your guild.\r\n", ch);
  else
   if (GET_THIEF_LEVEL(ch) > 0)
    list_skillst(ch);
   if (GET_WARRIOR_LEVEL(ch) > 0)
    list_skillsw(ch);
   if (GET_MAGE_LEVEL(ch) > 0)
    list_skillsm(ch); 
   if (GET_CLERIC_LEVEL(ch) > 0)
    list_skillsc(ch); 
}


ACMD(do_visible)
{
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    perform_immort_vis(ch);
    return;
  }

  if AFF_FLAGGED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);

    if(IS_AFFECTED(ch, AFF_SHROUD)) {
       affect_from_char(ch, SPELL_SHROUD);
       send_to_char("Your magic shroud dissapates.\r\n", ch);
    }
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	    MAX_TITLE_LENGTH);
    send_to_char(buf, ch);
  } else {
    set_title(ch, argument);
    sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
  }
}

ACMD(do_pretitle)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	    MAX_TITLE_LENGTH);
    send_to_char(buf, ch);
  } else {
    set_pretitle(ch, argument);
    sprintf(buf, "Okay, you're now %s%s.\r\n", GET_PRETITLE(ch), GET_NAME(ch));
    send_to_char(buf, ch);
  }
}

int perform_group(struct char_data *ch, struct char_data *vict)
{
  if (AFF_FLAGGED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return (0);

  SET_BIT(AFF_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return (1);
}

void print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;

  if (!AFF_FLAGGED(ch, AFF_GROUP))
    send_to_char("&wBut you are not the member of a group!\r\n", ch);
  else {
    send_to_char("&wYour group consists of:\r\n", ch);

    k = (ch->master ? ch->master : ch);


   if (AFF_FLAGGED(k, AFF_GROUP)) {
      if(!PRF_FLAGGED(k, PRF_ANON))
       {
      sprintf(buf, "&w     [%4d/%4dH %4d/%4dM %4d/%4dV] [%2d %s] $N (Leader)",
              GET_HIT(k),   GET_MAX_HIT(k),
              GET_MANA(k),  GET_MAX_MANA(k),
              GET_MOVE(k),  GET_MAX_MOVE(k),
              GET_LEVEL(k), CLASS_ABBR(k));
       }
      else
       {
      sprintf(buf, "&w     [%4d/%4dH %4d/%4dM %4d/%4dV] [ ANON] $N (Leader)",
              GET_HIT(k), GET_MAX_HIT(k),
              GET_MANA(k), GET_MAX_MANA(k),
              GET_MOVE(k), GET_MAX_MOVE(k));
       }
    act(buf, FALSE, ch, 0, k, TO_CHAR | TO_SLEEP);
    }

    for (f = k->followers; f; f = f->next) {
      if (!AFF_FLAGGED(f->follower, AFF_GROUP))
        continue;
      if(!PRF_FLAGGED(f->follower, PRF_ANON))
       {
      sprintf(buf, "&w     [%4d/%4dH %4d/%4dM %4d/%4dV] [%2d %s] $N (%s) %s",
              GET_HIT(f->follower),   GET_MAX_HIT(f->follower),
              GET_MANA(f->follower),  GET_MAX_MANA(f->follower),
              GET_MOVE(f->follower),  GET_MAX_MOVE(f->follower),
              GET_LEVEL(f->follower), CLASS_ABBR(f->follower),
              GET_GJOB(f->follower) == NULL  ?  GET_GJOB(f->follower) = "" :  
		   GET_GJOB(f->follower),
              IN_ROOM(f->follower) == IN_ROOM(k) ? "" : "&R{AWAY}&n"); 

       }
      else
       {
      sprintf(buf, "&w     [%4d/%4dH %4d/%4dM %4d/%4dV] [ ANON] $N (%s) %s", 
              GET_HIT(f->follower),   GET_MAX_HIT(f->follower),
              GET_MANA(f->follower),  GET_MAX_MANA(f->follower),
              GET_MOVE(f->follower),  GET_MAX_MOVE(f->follower),
              GET_GJOB(f->follower) == NULL  ?  GET_GJOB(f->follower) = "" :  
		   GET_GJOB(f->follower),
              IN_ROOM(f->follower) == IN_ROOM(k) ? "" : "&R{AWAY}&n"); 

       }

      act(buf, FALSE, ch, 0, f->follower, TO_CHAR | TO_SLEEP);
  }
 }
}

ACMD(do_group)
{
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);
    return;
  }

  if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)))
    send_to_char(NOPERSON, ch);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!AFF_FLAGGED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
	act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT(AFF_FLAGS(vict), AFF_GROUP);
    }
  }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(AFF_FLAGGED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (AFF_FLAGGED(f->follower, AFF_GROUP)) {
	REMOVE_BIT(AFF_FLAGS(f->follower), AFF_GROUP);
	send_to_char(buf2, f->follower);
        if (!AFF_FLAGGED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    return;
  }
  if (!(tch = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }
  if (tch->master != ch) {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  if (!AFF_FLAGGED(tch, AFF_GROUP)) {
    send_to_char("That person isn't in your group.\r\n", ch);
    return;
  }

  REMOVE_BIT(AFF_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
 
  if (!AFF_FLAGGED(tch, AFF_CHARM))
    stop_follower(tch);
}


ACMD(do_report)
{
  sprintf(buf, "&W%s reports: (%d/%dH, %d/%dM, %d/%dV, Age: %d)&n\r\n",
	  GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch), GET_AGE(ch));
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
}


ACMD(do_split)
{
  int amount, num, share, rest;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) && GET_LEVEL(f->follower) < LVL_IMMORT)
	num++;

    if (num && AFF_FLAGGED(ch, AFF_GROUP)) {
      share = amount / num;
      rest = amount % num;
    } else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
            amount, share);
    if (rest) {
      sprintf(buf + strlen(buf), "%d coin%s %s not splittable, so %s "
              "keeps the money.\r\n", rest,
              (rest == 1) ? "" : "s",
              (rest == 1) ? "was" : "were",
              GET_NAME(ch));
    }
    if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k) && GET_LEVEL(k) < LVL_IMMORT) && k != ch) {
      GET_GOLD(k) += share;
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch && GET_LEVEL(f->follower) < LVL_IMMORT) {
	GET_GOLD(f->follower) += share;
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    if (rest) {
      sprintf(buf + strlen(buf), "%d coin%s %s not splitable, so you keep "
                                 "the money.\r\n", rest,
                                 (rest == 1) ? "" : "s",
                                 (rest == 1) ? "was" : "were");
      GET_GOLD(ch) += rest;
    }
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  struct obj_data *mag_item;
  room_rnum was_in;
  struct follow_type *k, *next;


  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_use.", subcmd);
      return;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.\r\n", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.\r\n", ch);
      return;
    }
    break;
  case SCMD_USE:
    if (GET_OBJ_TYPE(mag_item) == ITEM_PORTAL)
    { 
          if (GET_OBJ_VAL(mag_item, 0) != NOWHERE && GET_OBJ_LEVEL(mag_item) <= 
GET_TOTAL_LEVEL(ch)) 
          {
            act("$n enters $p.", TRUE, ch, mag_item, 0, TO_ROOM);
            was_in = ch->in_room;
            char_from_room(ch);
            char_to_room(ch, GET_OBJ_VAL(mag_item, 0));
            act("$n comes stepping out of a portal.\r\n", TRUE, ch, 0, 0, TO_ROOM);
            look_at_room(ch, 1);

            obj_to_room(unequip_char(ch,WEAR_HOLD), was_in);

            for (k = ch->followers;k && mag_item;k = next) 
            {
              next = k->next;
              if ((k->follower->in_room == was_in) && GET_POS(k->follower) >= POS_STANDING)
              {
                act("You cannot follow $N into $p, because it has fallen to the floor.", 
FALSE,k->follower,mag_item, ch, TO_CHAR);
            
              }   
            }
          }
          if (GET_OBJ_LEVEL(mag_item) >= GET_TOTAL_LEVEL(ch))
          {
            act("You cannot enter $p! Try again when you are older.",
FALSE,ch,mag_item, ch, TO_CHAR);
          }

          return;
    }
      

    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  /* 'wimp_level' is a player_special. -gg 2/25/98 */
  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	      GET_WIMP_LEV(ch));
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
      return;
    }
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg)) != 0) {
      if (wimp_lev < 0)
	send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
      else if (wimp_lev > GET_MAX_HIT(ch))
	send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      else if (wimp_lev > (GET_MAX_HIT(ch) / 2))
	send_to_char("You can't set your wimp level above half your hit points.\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
		wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);
}


ACMD(do_display)
{
  size_t i;

  if (IS_NPC(ch)) {
    send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: prompt { { H | M | V } | all | none }\r\n", ch);
    return;
  }
  if (!str_cmp(argument, "on") || !str_cmp(argument, "all"))
    SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE);
  else if (!str_cmp(argument, "off") || !str_cmp(argument, "none"))
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE);
  else {
    REMOVE_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA | PRF_DISPMOVE);

    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h':
	SET_BIT(PRF_FLAGS(ch), PRF_DISPHP);
	break;
      case 'm':
	SET_BIT(PRF_FLAGS(ch), PRF_DISPMANA);
	break;
      case 'v':
	SET_BIT(PRF_FLAGS(ch), PRF_DISPMOVE);
	break;
      default:
	send_to_char("Usage: prompt { { H | M | V } | all | none }\r\n", ch);
	return;
      }
    }
  }

  send_to_char(OK, ch);
}



ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, buf[MAX_STRING_LENGTH];
  const char *filename;
  struct stat fbuf;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, CMP, LVL_IMMORT, FALSE);

  if (stat(filename, &fbuf) < 0) {
    perror("SYSERR: Can't stat() file");
    return;
  }
  if (fbuf.st_size >= max_filesize) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("SYSERR: do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	  GET_ROOM_VNUM(IN_ROOM(ch)), argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
  long result;

  const char *tog_messages[][2] = {
    {"You are now safe from summoning by other players.\r\n",
    "You may now be summoned by other players.\r\n"},
    {"Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
    "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
    "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n"},
    {"You can now hear auctions.\r\n",
    "You are now deaf to auctions.\r\n"},
    {"You can now hear shouts.\r\n",
    "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n"},
    {"You can now hear the congratulation messages.\r\n",
    "You are now deaf to the congratulation messages.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n"},
    {"You are no longer part of the Quest.\r\n",
    "Okay, you are part of the Quest!\r\n"},
    {"You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Autoexits disabled.\r\n",
    "Autoexits enabled.\r\n"},
    {"AutoSplit disabled.\r\n",
    "AutoSplit enabled.\r\n"},
    {"AutoLooting disabled.\r\n",
    "AutoLooting enabled.\r\n"},
    {"Will no longer track through doors.\r\n",
    "Will now track through doors.\r\n"},
    {"You will no longer Auto-Assist.\r\n",
    "You will now Auto-Assist.\r\n"},
    {"AFK flag is now off.\r\n",
    "AFK flag is now on.\r\n"},
    {"Autodiag disabled.\r\n",
    "Autodiag enabled.\r\n"},
    {"ASCII objects turned off.\r\n",
    "ASCII objects turned on.\r\n"},   
    {"AutoGold disabled.\r\n",
    "AutoGold enabled.\r\n"},
    {"Your level is now visible on the who list.\r\n",
    "You are now anonymous on the who list.\r\n"},
    {"You are now visible on the who list.\r\n",
    "You are now invisible from the who list.\r\n"},
    {"AutoSacrifice disabled.\r\n",
    "AutoSacrifice enabled.\r\n"},
    {"You can no longer hear all tells.\r\n",
     "You can hear all tells.\r\n"}
    };

  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_AUTOSPLIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOSPLIT);
    break;
  case SCMD_AUTOLOOT:
    result = PRF_TOG_CHK(ch, PRF_AUTOLOOT);
    break;
  case SCMD_AUTOASSIST:
    result = PRF_TOG_CHK(ch, PRF_AUTOASSIST);
    break;
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_SLOWNS:
    result = (nameserver_is_slow = !nameserver_is_slow);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    break;
  case SCMD_AFK:
    result = PRF_TOG_CHK(ch, PRF_AFK);
   if (PRF_FLAGGED(ch, PRF_AFK))
     act("$n has gone afk.", TRUE, ch, 0, 0, TO_ROOM);
   else
     act("$n has come back from AFK.", TRUE, ch, 0, 0, TO_ROOM);
   break;
  case SCMD_AUTODIAG:
     result = PRF_TOG_CHK(ch, PRF_AUTODIAG);
     break;
  case SCMD_TRACK:
     result = (track_through_doors = !track_through_doors);
     break;
  case SCMD_AUTOGOLD:
     result = PRF_TOG_CHK(ch, PRF_AUTOGOLD);
     break;
  case SCMD_XAP_OBJS:
     result = (xap_objs = !xap_objs);
     break;
  case SCMD_ANON:
     result = PRF_TOG_CHK(ch, PRF_ANON);
     break;
  case SCMD_AUTOSAC:
     result = PRF_TOG_CHK(ch, PRF_AUTOSAC);
     break;
  default:
    log("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
    return;
  }

  if (result)
    send_to_char(tog_messages[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd][TOG_OFF], ch);
  return;
}

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_togx)
{
  long result2;

  const char *tog_messages2[][2] = {
    {"AutoAffects disabled.\r\n",
    "AutoAffects enabled.\r\n"},
    {"AutoTick disabled.\r\n",
    "AutoTick enabled.\r\n"},
    {"Player Log disabled.\r\n",
    "Player Log enabled.\r\n"},
    {"BuildWalk disabled.\r\n",
    "BuildWalk enabled.\r\n"},
    {"You can no longer hear all tells.\r\n",
     "You can hear all tells.\r\n"},
    {"You will no longer hear the filthy rubbish of the scumbags.\r\n",
     "You let your virgin ears hear some things they probably shouldn't.\r\n"},
    {"lyric", "lyric"},
    {"You can no longer hear it when people gain levels.\r\n",
     "You can now hear it when people gain levels.\r\n"}, 
    {"You will no longer see full exit descriptins.\r\n",
     "You will now see full exit descriptions.\r\n"},
    {"You will now accept items from people.\r\n",
     "You will no longer accept items from people.\r\n"}
    };  

  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_AUTOAFFECTS:
    result2 = PRF_TOG_CHK2(ch, PRF2_AUTOSCORE);
    break;
  case SCMD_AUTOTICK:
    result2 = PRF_TOG_CHK2(ch, PRF2_AUTOTICK);
    break;
  case SCMD_AUTOLOGON:
    result2 = PRF_TOG_CHK2(ch, PRF2_AUTOLOGON);
    break;
  case SCMD_BUILDWALK:
    result2 = PRF_TOG_CHK2(ch, PRF2_BUILDWALK);
    break;
  case SCMD_HEARALLTELL:
    result2 = PRF_TOG_CHK2(ch, PRF2_HEARALLTELL);
    break;
  case SCMD_CUSS:
    result2 = PRF_TOG_CHK2(ch, PRF2_CUSS);
    break;
  case SCMD_LYRIC:
    result2 = PRF_TOG_CHK2(ch, PRF2_LYRIC);
    break;
  case SCMD_HEARLEVEL:
    result2 = PRF_TOG_CHK2(ch, PRF2_HEARLEVEL);
    break;
  case SCMD_FULLEXIT:
    result2 = PRF_TOG_CHK2(ch, PRF2_AUTOEXITFULL);
    break;
  case SCMD_NOGIVE:
    result2 = PRF_TOG_CHK2(ch, PRF2_NOGIVE);
    break;
  default:
    log("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
    return;
  }

  if (result2)
    send_to_char(tog_messages2[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages2[subcmd][TOG_OFF], ch);
  return;
}

ACMD(do_recall)
{
	if (IS_NPC(ch)) 
		{
			send_to_char("Monsters can't recall!!\r\n", ch);
			return;
		}
	
	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NO_RECALL | ROOM_NOESCAPE) && IN_ROOM(ch) != real_room(2004)) 
		{
			send_to_char("Your attempt to recall fails.\r\n", ch);
			return;
		}
	
	GET_MANA(ch) = (int)(GET_MANA(ch) * 0.25);
	GET_MOVE(ch) = (int)(GET_MOVE(ch) * 0.25);

	send_to_char("Recalling...\r\n", ch);
	send_to_char("&RYou feel tired&n\r\n", ch);
	act("$n concentrates and disappears.", TRUE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, real_room(mortal_start_room));
	act("$n appears suddenly.", TRUE, ch, 0, 0, TO_ROOM);
	look_at_room(ch, 0);
}


#define MOB_OR_IMPL(ch) \
  (IS_NPC(ch) && (!(ch)->desc || GET_LEVEL((ch)->desc->original)>=LVL_IMPL))

ACMD(do_mskillset)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH],
       skill_name[MAX_INPUT_LENGTH],
       arg3[MAX_INPUT_LENGTH];
  int skill_num, percent, curr, qend;

  argument = one_argument(argument, name);

  if (!*name) {
    script_log("Too few arguments to mskillset");
    return;
  }

  if (!MOB_OR_IMPL(ch)) {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, name)))
    return;

  skip_spaces(&argument);

  if (!*argument) {
    script_log("Too few arguments to mskillset");
    return;
  }
  if (*argument != '\'') {
    script_log("Skill name not enclosed in 's in mskillset");
    return;
  }

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    script_log("Skill name not enclosed in 's in mskillset");
    return;
  }

  strcpy(skill_name, (argument + 1));
  skill_name[qend -1] = '\0';

  /* The script should do the checking for skills, or else the player
     will be able to practice any skill here. But just in case its not,
     we'll log errors at least */

  if ((skill_num = find_skill_num(skill_name)) <= 0) {
    script_log("Invalid skill to mskillset");
    send_to_char("Invalid skill\r\n", ch);
    return;
  }

  argument += qend + 1;         /* skip to next parameter */
  argument = one_argument(argument, arg3);

  if (!*arg3) {
    script_log("Too few arguments to mskillset");
    return;
  }

  percent = atoi(arg3);

  if (percent < 0) {
    script_log("Negative number to mskillset.\r\n");
    return;
  }
  if (percent > 100) {
    script_log("Learned level exceeding 100 in mskillset.\r\n");
    return;
  }
  if (IS_NPC(vict)) {
    return;
  }

  if (subcmd == SCMD_SKILLSET) {
    sprintf(buf2, "%s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
            spell_info[skill_num].name, percent);
    mudlog(buf2, BRF, LVL_GRGOD, TRUE);

    SET_SKILL(vict, skill_num, percent);
    return;
  }

  else if (subcmd == SCMD_TEACH) {
    if (GET_SKILL(vict, skill_num) >= 80) {
      send_to_char("You are already learned in that area.\r\n", vict);
      return;
    }
    if (GET_PRACTICES(vict) <= 0) {
      send_to_char("You can't practice any more this level.\r\n", vict);
      return;
    }

    curr = GET_SKILL(vict, skill_num);
    curr += percent;
    if (curr >= 100)
      curr = 100;
    SET_SKILL(vict, skill_num, curr);
    GET_PRACTICES(vict)--;
  }
}

ACMD(do_finger)
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
    return;
  }                                                                         
  sprintf(buf, "%s's last appearance was %-20s", GET_NAME(vict), ctime(&vict->player.time.logon));
  send_to_char(buf, ch);
}

char * levels_exp_req_helper(struct char_data *ch, byte gain_class) 
{
	if (get_class_level(ch, gain_class) == MAX_MASTER_LEVEL)
		sprintf(buf1, "        Mastered        ");
  else {
    char* temp = num_punct(needed_exp(GET_CLASS(ch),  gain_class, get_class_level(ch, gain_class))); 
    sprintf(buf1, "&BExp  req : %13s&n", temp);
    free(temp);
  }
	return str_dup(buf1);
}

char * levels_exp_left_helper(struct char_data *ch, byte gain_class) 
{
	if (get_class_level(ch, gain_class) == MAX_MASTER_LEVEL)
		sprintf(buf1, "                        ");
  else {
    char* temp = num_punct(MAX(0,needed_exp(GET_CLASS(ch),  gain_class, get_class_level(ch, gain_class))-GET_EXP(ch)));
    sprintf(buf1, "&BExp  left: %13s&n", temp);
    free(temp);
  }
	return str_dup(buf1);
}

char * levels_gold_req_helper(struct char_data *ch, byte gain_class) 
{
	if (get_class_level(ch, gain_class) == MAX_MASTER_LEVEL)
		sprintf(buf1, "                        ");
  else {
    char* temp = num_punct(needed_gold(GET_CLASS(ch),  gain_class, get_class_level(ch, gain_class)));
    sprintf(buf1, "&YGold req : %13s&n", temp);
    free(temp);
  }
	return str_dup(buf1);
}

char * levels_gold_left_helper(struct char_data *ch, byte gain_class) 
{
	if (get_class_level(ch, gain_class) == MAX_MASTER_LEVEL)
		sprintf(buf1, "                        ");
  else {
    char* temp = num_punct(MAX(0,needed_gold(GET_CLASS(ch),  gain_class, get_class_level(ch, gain_class))-GET_GOLD(ch)));
    sprintf(buf1, "&YGold left: %13s&n", temp); 
    free(temp);
  }
	return str_dup(buf1);
}

char * can_level_class(struct char_data *ch, byte gain_class) {
	if (get_class_level(ch, gain_class) == 0)
		return "           ";
	if (MAX(0,needed_gold(GET_CLASS(ch),  gain_class, get_class_level(ch,gain_class))-GET_GOLD(ch)) == 0 &&
			MAX(0,needed_exp(GET_CLASS(ch),  gain_class, get_class_level(ch,gain_class))-GET_EXP(ch)) == 0)
		return "&G(CAN LEVEL)&n";
	else
		return "           ";
}

ACMD(do_level)
{
	bool can_level= FALSE;

  char* curexp = num_punct(GET_EXP(ch));
  char* curgold = num_punct(GET_GOLD(ch));

  sprintf(buf,               "&BExp : %13s&n\r\n", curexp);
  sprintf(buf + strlen(buf), "&YGold: %13s&n\r\n", curgold);
  sprintf(buf + strlen(buf), "\r\n");

  free(curexp);
  free(curgold);


	if (GET_TOTAL_LEVEL(ch) == MAX_TOTAL_LEVEL) {
      int short_exp  = MAX(0, GMEXP  - GET_EXP (ch));
      int short_gold = MAX(0, GMGOLD - GET_GOLD(ch));
		  char* gmer = num_punct(GMEXP);  char* gmel = num_punct(short_exp);
			char* gmgr = num_punct(GMGOLD); char* gmgl = num_punct(short_gold);
			can_level = (short_exp == 0 && short_gold == 0);
	
  		sprintf(buf + strlen(buf), "GrandMaster  %s\r\n", can_level ? "&G(CAN LEVEL)&n" : "           ");
      sprintf(buf + strlen(buf), "Level %d\r\n", GET_GM_LEVEL(ch)); 
			sprintf(buf + strlen(buf), "------------------------\r\n");
      sprintf(buf + strlen(buf), "&BExp  req : %13s&n\r\n", gmer);
      sprintf(buf + strlen(buf), "&BExp  left: %13s&n\r\n", gmel);
      sprintf(buf + strlen(buf), "&YGold req : %13s&n\r\n", gmgr);
      sprintf(buf + strlen(buf), "&YGold left: %13s&n\r\n", gmgl);

			send_to_char(buf, ch);

      free(gmer); free(gmel);
      free(gmgr); free(gmgl);

	} else {
      char* ter = levels_exp_req_helper (ch,  CLASS_THIEF);   char* tel = levels_exp_left_helper (ch,  CLASS_THIEF);
      char* tgr = levels_gold_req_helper(ch,  CLASS_THIEF);   char* tgl = levels_gold_left_helper(ch,  CLASS_THIEF);
      char* wer = levels_exp_req_helper (ch,  CLASS_WARRIOR); char* wel = levels_exp_left_helper (ch,  CLASS_WARRIOR);
      char* wgr = levels_gold_req_helper(ch,  CLASS_WARRIOR); char* wgl = levels_gold_left_helper(ch,  CLASS_WARRIOR);
      char* mer = levels_exp_req_helper (ch,  CLASS_MAGE);    char* mel = levels_exp_left_helper (ch,  CLASS_MAGE);
      char* mgr = levels_gold_req_helper(ch,  CLASS_MAGE);    char* mgl = levels_gold_left_helper(ch,  CLASS_MAGE);
      char* cer = levels_exp_req_helper (ch,  CLASS_CLERIC);  char* cel = levels_exp_left_helper (ch,  CLASS_CLERIC);
      char* cgr = levels_gold_req_helper(ch,  CLASS_CLERIC);  char* cgl = levels_gold_left_helper(ch,  CLASS_CLERIC);
      
			sprintf(buf + strlen(buf), "Thief        %s    Warrior       %s\r\n", can_level_class(ch, CLASS_THIEF), can_level_class(ch, CLASS_WARRIOR));
      sprintf(buf + strlen(buf), "Level %-2d                    Level %-2d\r\n", GET_THIEF_LEVEL(ch), GET_WARRIOR_LEVEL(ch)); 
      sprintf(buf + strlen(buf), "------------------------    ------------------------\r\n");
      sprintf(buf + strlen(buf), "%s    %s\r\n", ter, wer);
      sprintf(buf + strlen(buf), "%s    %s\r\n", tel, wel);
      sprintf(buf + strlen(buf), "%s    %s\r\n", tgr, wgr);
      sprintf(buf + strlen(buf), "%s    %s\r\n", tgl, wgl);
      sprintf(buf + strlen(buf), "\r\n");

			sprintf(buf + strlen(buf), "Mage         %s    Cleric        %s\r\n", can_level_class(ch, CLASS_MAGE), can_level_class(ch, CLASS_CLERIC));
      sprintf(buf + strlen(buf), "Level %-2d                    Level %-2d\r\n", GET_MAGE_LEVEL(ch), GET_CLERIC_LEVEL(ch)); 
      sprintf(buf + strlen(buf), "------------------------    ------------------------\r\n");
      sprintf(buf + strlen(buf), "%s    %s\r\n", mer, cer);
      sprintf(buf + strlen(buf), "%s    %s\r\n", mel, cel);
      sprintf(buf + strlen(buf), "%s    %s\r\n", mgr, cgr);
      sprintf(buf + strlen(buf), "%s    %s\r\n", mgl, cgl);
      sprintf(buf + strlen(buf), "\r\n");
							
			send_to_char(buf, ch);

      free(ter); free(tel);
      free(tgr); free(tgl);
      free(wer); free(wel);
      free(wgr); free(wgl);
      free(mer); free(mel);
      free(mgr); free(mgl);
      free(cer); free(cel);
      free(cgr); free(cgl);
		}
}


ACMD(do_die) 
{
  if (GET_HIT(ch) >= 0) 
		{
			send_to_char("You are not dead!\r\n", ch);
			return;
		} 
	else
		if (!IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_SPARRING)) 
			{
				GET_HIT(ch) = 1;
				send_to_char("You have lost the sparring match!\r\n", ch);
				char_from_room(ch);
				char_to_room(ch, real_room(291));
				stop_fighting(ch);
				REMOVE_BIT(PLR_FLAGS(ch), PLR_DEAD);
				REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADI);
				REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADII);
				REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADIII);
				GET_POS(ch) = POS_RESTING;
				return;
			}  
 
  if (IS_NPC(GET_KILLER(ch))) 
		GET_EXP(ch) = 0;
	
  GET_DEATHS(ch) = GET_DEATHS(ch) + 1;

	REMOVE_BIT(PLR_FLAGS(ch), PLR_DEAD);
	REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADI);
	REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADII);
	REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADIII);
	GET_POS(ch) = POS_RESTING;
  make_corpse(ch);
  save_char(ch, NOWHERE);
  Crash_crashsave(ch);
  extract_char(ch);
}

ACMD(do_autoall) 
{
	send_to_char("All auto-commands have been intiated\r\n", ch);
	send_to_char("AUTO_LOOT is defaulted, turn on AUTOGOLD seperatly\r\n", ch);
	autoall(ch);
}

void autoall(struct char_data *ch)
{
	SET_BIT(PRF_FLAGS(ch), PRF_AUTOEXIT);
	SET_BIT(PRF_FLAGS(ch), PRF_AUTOSPLIT); 
	SET_BIT(PRF_FLAGS(ch), PRF_AUTODIAG);
	SET_BIT(PRF_FLAGS(ch), PRF_AUTOLOOT);
	REMOVE_BIT(PRF_FLAGS(ch), PRF_AUTOGOLD);
	SET_BIT(PRF_FLAGS(ch), PRF_AUTOASSIST);
	SET_BIT(PRF_FLAGS(ch), PRF_AUTOSAC);
	SET_BIT(PRF_FLAGS2(ch), PRF2_AUTOSCORE);
	SET_BIT(PRF_FLAGS2(ch), PRF2_AUTOTICK);
	SET_BIT(PRF_FLAGS2(ch), PRF2_AUTOLOGON);
	if (PLR_FLAGGED(ch, PLR_CUSSER)) SET_BIT(PRF_FLAGS2(ch), PRF2_CUSS);
	SET_BIT(PRF_FLAGS2(ch), PRF2_HEARLEVEL);
	
	if (GET_LEVEL(ch) >= LVL_IMMORT) {
		SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
		SET_BIT(PRF_FLAGS(ch), PRF_ROOMFLAGS);
  }

}




ACMD(do_bandage)
{
  int percent, prob, cost;
  struct obj_data *obj;
  obj = get_obj_in_list_vis(ch, "bandage", ch->carrying);
	
  if (IS_NPC(ch))
		return;

  cost = (GET_THIEF_LEVEL(ch) / 2) + (GET_DEX(ch) * 2);
	
  if (!GET_SKILL(ch, SKILL_BANDAGE)) {
		if (!obj) {
			send_to_char("You can't bandage without bandages.\r\n", ch);
			return; }}
	
  if (GET_SKILL(ch, SKILL_BANDAGE))
		if (!obj) {
			if (GET_MOVE(ch) < cost) {
				send_to_char("You don't have enough movement points to bandage.\r\n",ch);
				return; }}
	
  if (GET_HIT(ch) >= GET_MAX_HIT(ch)) {
		send_to_char("You are already healthy!\r\n", ch);
		return; }
	
  percent = number(1, 101);
  if (GET_SKILL(ch, SKILL_BANDAGE))
		prob = 60 + 3.5 * GET_SKILL(ch, SKILL_BANDAGE);
  else
		prob = GET_DEX(ch) * 4;
	
  if (percent > prob)
		{
			send_to_char("You fail to bandage.\r\n", ch);
			if (obj)
				extract_obj(obj);

		}
	else 
		{
			
			if (!obj)
				GET_MOVE(ch) -= cost;
			if (!obj)
				switch (GET_CLASS(ch))
					{
					case CLASS_THIEF:
						GET_HIT(ch) += ((GET_THIEF_LEVEL(ch) / 2) + (GET_DEX(ch) * 2)) * .9;
						break;
					case CLASS_MAGIC_USER:
					case CLASS_CLERIC:
						GET_HIT(ch) += ((GET_THIEF_LEVEL(ch) / 2) + (GET_DEX(ch) * 2)) * .5;
						break;
					case CLASS_WARRIOR:
						GET_HIT(ch) += ((GET_THIEF_LEVEL(ch) / 2) + (GET_DEX(ch) * 2)) * .8;
						break;
					}
			else
				GET_HIT(ch) += (GET_THIEF_LEVEL(ch) / 2);

			if (!obj) 
				{
					send_to_char("You bandage yourself.\r\n", ch);
					act("$n bandages $mself.", FALSE, ch, 0, 0, TO_ROOM);
				} 
			else 
				{
					act("You bandage yorself with $p.", FALSE, ch, obj, 0, TO_CHAR);
					act("$n bandages $mself with $p.", FALSE, ch, obj, 0, TO_ROOM);
					extract_obj(obj); 
				}
		
			if (GET_HIT(ch) >= GET_MAX_HIT(ch)) 
				{
					send_to_char("You are now at full health.\r\n", ch);
					GET_HIT(ch) = GET_MAX_HIT(ch); 
				}
		}
	WAIT_STATE(ch, PULSE_VIOLENCE * 1);	
}

C_FUNC(test_func) {
 sprintf(buf, "You typed '%s'!\r\n", arg);
 SEND_TO_Q(buf, d);
 // This is how you CHAIN functions together
 line_input(d, "Type something else: ", test_func_chain, str_dup(arg));

}

 C_FUNC(test_func_chain) {
 sprintf(buf, "You typed '%s' and then '%s'... Cool.\r\n", (char *)info, arg);
 SEND_TO_Q(buf, d);
  // We str_dup()'d this pointer, so we must FREE it
 if (info)
   free(info);
}

int crashcheck_alpha (struct char_data *ch, struct char_data *vict)
{
	/* Just some crash-preventing checks. */
	/* Returns 1 to indicate a crash will occur. */
	/* Return 0 if everything's ok */
	if(ch->in_room == -1)
	{
		return 1;
	}
	else if (IS_NPC(ch))
	{
		send_to_char("You have to do that to a player!\r\n", vict);
		return 1;
	}
	else if (IS_NPC(vict))
	{
		send_to_char("You can't do that!\r\n", vict);
		return 1;
	}
	else if(GET_POS(ch) < POS_RESTING)
	{
		send_to_char("You can't do that now!\r\n", vict);
		return 1;
	}
	else
	{
		return 0;
	}
}

void namesave(struct char_data *ch, struct char_data *vict) {
	save_char(ch, NOWHERE);
	Crash_crashsave(ch);
    if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
	House_crashsave(GET_ROOM_VNUM(IN_ROOM(ch)));

		save_char(vict, NOWHERE);
		Crash_crashsave(vict);
		if (ROOM_FLAGGED(vict->in_room, ROOM_HOUSE_CRASH))
			House_crashsave(GET_ROOM_VNUM(IN_ROOM(vict)));
		// Saved, now reorient Partner via a load.
		if(load_char(GET_NAME(vict), vict) < 0) {
			send_to_char("Error: Cannot Load VICT\r\n", vict);
			return;
		}
		if(load_char(GET_NAME(ch), ch) < 0) {
			send_to_char("Error: Cannot Load CH\r\n", ch);
			return;
		}

}

void changesave(struct char_data *ch, struct char_data *vict) {
	/* Saves changes after rejection */
	save_char(ch, NOWHERE);
			Crash_crashsave(ch);
			if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
				House_crashsave(GET_ROOM_VNUM(IN_ROOM(ch)));
			save_char(vict, NOWHERE);
			Crash_crashsave(vict);
			if (ROOM_FLAGGED(vict->in_room, ROOM_HOUSE_CRASH))
				House_crashsave(GET_ROOM_VNUM(IN_ROOM(vict)));
}

int check_samesex(struct char_data *ch, struct char_data *victim)
{
	/*Checks if it's a same-sex proposition/marriage */
	/* Then checks if SAME_SEX_ALLOWED is TRUE. */
	/* If the proposition is same-sex, and SAME_SEX_ALLOWED */
	/* is FALSE, then it returns 1, which will halt the procedure. */
	/* Otherwise, if SAME_SEX_ALLOWED is TRUE or the proposition is */
	/* NOT same-sex, it returns 0, and things continue as normal */
	if((GET_SEX(ch) == GET_SEX(victim)) && (SAME_SEX_ALLOWED == FALSE))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/* Romance Module -- Ask Someone Out */
ACMD(do_askout)
{
	struct char_data *victim;

	one_argument(argument, arg);
	if (!*arg) { /* What, ask no one out? */
		send_to_char("Uh, whom do you wish to ask out?\r\n", ch);
        return;
	}
	else if (!(victim = get_char_room_vis(ch, arg))) {
		/* Is that person here? Nope! */
		send_to_char(NOPERSON, ch);
		return;
	}
	if (ROMANCE(ch) != 0) { /* Are you already involved? */
		send_to_char("Sorry, you're already romantically involved!\r\n", ch);
		return;
	}
	else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
	else if (ROMANCE(victim) != 0) { /* Is the person you're propositioning to involved? */
		send_to_char("Try propositioning someone who ISN'T already romantically involved!\r\n", ch);
		return;
	}
	else if (victim == ch) {
		/* Ask yourself out?!? */
		send_to_char("Sorry, but you cannot ask yourself out!\r\n", ch);
		return;
	}
	else if (check_samesex(ch, victim) == 1) {
		/* Check if it's same-sex, and if same-sex is allowed. */
		send_to_char("Sorry, same-sex relations are not allowed here.\r\n", ch);
	    return;
	}
	else if(ROMANCE(ch) == -3) {
		send_to_char("Sorry, you have to turn romance on before asking someone out.\r\n", ch);
		return;
	}
	else if(ROMANCE(victim) == -3) {
		send_to_char("Sorry, they've got romance turned off for the moment..\r\n", ch);
		return;
	}
	else { /* Okay, now we do that actual asking out.. */

	act("You ask $N out...\r\n", TRUE, ch, 0, victim, TO_CHAR);
	act("$n is asking you out!\r\n", TRUE, ch, 0, victim, TO_VICT);
	act("$n asks $N out on a date!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
	ROMANCE(victim) = ASKED_OUT; /* Now they're being asked out. */
	ROMANCE(ch) = RASKING; /* Temporarily declare the person as being asked out.. */
	  /* NOTE: PARTNER(ch) must be reset in DO_REJECT */
	/* At this point, the Victim can either ACCEPT or REJECT the
proposition. */
	}
}

/* Romance Module -- Accept a Proposition */

ACMD(do_accept)
{
	struct char_data *victim;
	if(ROMANCE(ch) < ASKED_OUT) { /* You're not being asked out.. */
		send_to_char("You're not being romantically propositioned..\r\n", ch);
		return;
	}
	else { /* Okay, you've been asked out or proposed to.. */
		one_argument(argument, arg);
		if(!*arg) { /* Accept no one? */
			send_to_char("Who do you want to accept?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Accept yourself? */
			send_to_char("You can't accept yourself as a partner!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if(ROMANCE(victim) < RASKING) /* Are they propositioning as well? */ {
			send_to_char("But they're not asking you out!\r\n", ch);
			return;
		}
		else if(check_samesex(ch, victim) == 1) {
			/* Check for same-sex relations.. */
			send_to_char("Sorry, same-sex relationships are not allowed here.\r\n", ch);
			/* Okay, they've been corrected. The tick-timer will expire the
			ROMANCE factor on it's own. */
			return;
		}
		else { /* Okay, all the tests pass.. */
			if((ROMANCE(ch) == ASKED_OUT) && (ROMANCE(victim) == RASKING)) {
				/* For dating.. */
				act("You agree to date $N!\r\n", TRUE, ch, 0, victim, TO_CHAR);
				act("$n has agreed to date you!\r\n", TRUE, ch, 0, victim, TO_VICT);
				act("$n agrees to date $N!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
				/* We've notified them, now change the variables. */
                                free(PARTNER(ch)); free(PARTNER(victim));
                        	PARTNER(ch) = strdup(GET_NAME(victim));
                        	PARTNER(victim) = strdup(GET_NAME(ch));
				ROMANCE(ch) = 1;
				ROMANCE(victim) = 1;
				/*namesave(ch,victim);*/
                                /* Now they're set as partners, and they've been set at Dating. */
                                save_char(ch, NOWHERE);
				save_char(victim, NOWHERE);
				return;
			}
			else if((ROMANCE(ch) == PROPOSED_TO) && (ROMANCE(victim) == PASKING)) 
{
				/* For engagement.. */
				act("You agree to marry $N!\r\n", TRUE, ch, 0, victim, TO_CHAR);
				act("$n has agreed to marry you!\r\n", TRUE, ch, 0, victim, TO_VICT);
				act("$n agrees to marry $N!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
	              		/* We've done the notification, now change the variables. */
                                free(PARTNER(ch)); free(PARTNER(victim));
				PARTNER(victim) = strdup(GET_NAME(ch));
				PARTNER(ch) = strdup(GET_NAME(victim));
				ROMANCE(ch) = 2;
				ROMANCE(victim) = 2;
				/* Now they're engaged and partnered.. */
				/* namesave(ch, victim); */ 
				save_char(ch, NOWHERE);
				save_char(victim, NOWHERE);
				return; /* Get the hell outta this command. */
			}
			else { /* They're not propositioning you after all. */
				send_to_char("That person isn't propositioning you..\r\n", ch);
				return;
			}
		}
	}
}

/* Romance Module -- Reject A Proposition */
ACMD(do_reject)
{
	struct char_data *victim;

 	if(ROMANCE(ch) < ASKED_OUT) { /* You're not being asked out.. */
		send_to_char("You're not being romantically propositioned..\r\n", ch);
		return;
	}
	else { /* Okay, you've been asked out or proposed to.. */
		one_argument(argument, arg);
		if(!*arg) { /* Reject no one? */
			send_to_char("Whose heart do you wish to break?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Reject yourself? */
			send_to_char("You can't reject yourself!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if(ROMANCE(victim) < RASKING) /* Are they propositioning as well? */ {
			send_to_char("But they're not asking you out!\r\n", ch);
			return;
		}
		else if(check_samesex(ch, victim) == 1) {
			/* Uh, they couldn't have asked you out. */
			send_to_char("Sorry, but that person isn't propositioning you.\r\n", ch);
			return;
		}
		else { /* Okay, all the tests pass.. */
		if ((ROMANCE(ch) == ASKED_OUT) && (ROMANCE(victim) == RASKING)) {
			/* For dating.. */
		act("You tell $N that you'd rather just be friends.\r\n", TRUE, ch, 0, victim, TO_CHAR);
		sprintf(buf, "%s would rather just be 'friends', but you feel your heart shatter into a thousand pieces.\r\n", GET_NAME(ch));
		send_to_char(buf, ch);
		act("$n rejects $N! How cold!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
		free(PARTNER(victim)); free(PARTNER(ch));
                PARTNER(victim) = NULL;
		PARTNER(ch) = NULL;
		ROMANCE(victim) = 0;
		ROMANCE(ch) = 0;
                /*namesave(ch, victim);*/
		changesave(ch, victim);
		return;
			}
			else if ((ROMANCE(ch) == PROPOSED_TO) && (ROMANCE(victim) == PASKING)) {
				/* For marriage proposals... */
			send_to_char("You decide you'd rather not get married just yet.\r\n", ch);
			act("$n doesn't want to get married just yet..\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n hands $N's ring back to $S.\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
                        free(PARTNER(ch)); free(PARTNER(victim));
 	        	PARTNER(victim) = strdup(GET_NAME(ch));
        		PARTNER(ch) = strdup(GET_NAME(victim));
			/* Just in case. */
			ROMANCE(victim) = 1;
			ROMANCE(ch) = 1; /* Back to "Dating" status. */
			/*namesave(ch, victim);*/ 
			save_char(ch, NOWHERE);
			save_char(victim, NOWHERE);
			return;
			}
			else { /* Oops! They're not asking you after all! */
				send_to_char("But they're not propositioning you!\r\n", ch);
				return;
			}
		}
	}
}

/* Romance Module -- Propose Marriage */
ACMD(do_propose)
{
	/* To propose to someone. */
	struct char_data *victim;
	if(ROMANCE(ch) < 1) { /* Are you already dating? */
		send_to_char("But you're not dating anyone!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) == 2) { /* Are you already engaged? */
		send_to_char("But you're already engaged!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) == 3) { /* Are you already married? */
		sprintf(buf, "But you're married already! %s wouldn't approve!\r\n", PARTNER(ch));
		send_to_char(buf, ch);
		return;
	}
	else if (ROMANCE(ch) == 4) { /* Are you being asking out by someone? */
		send_to_char("But you're being asked out! That would be rude!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) == 5) { /* Are you already proposing? */
		send_to_char("But you're already proposing!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) == RASKING) { /*Asking someone out? */
		send_to_char("But you're asking someone else!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) > 6) { /* Any errors in the module? */
		send_to_char("ERROR IN ROMANCE MODULE: Romance Factor > 6!\r\n", ch);
		return;
	}
	else { /* Okay, YOU pass... */
	one_argument(argument, arg);
		if(!*arg) { /* Propose to no one? */
			send_to_char("Whom do you want to propose to?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Propose to yourself? */
			send_to_char("You can't propose to yourself!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if(ROMANCE(victim) < 1) { /* Are they already dating? */
		send_to_char("But they're not dating anyone!\r\n", ch);
		return;
		}
		else if (ROMANCE(victim) == 2) { /* Are they already engaged? */
		sprintf(buf, "But they're already engaged to %s!\r\n", PARTNER(victim));
		send_to_char(buf, ch);
		return;
		}
		else if (ROMANCE(victim) == 3) { /* Are they already married? */
		sprintf(buf, "But they're married already! %s wouldn't approve!\r\n", PARTNER(victim));
		send_to_char(buf, ch);
		return;
		}
		else if (ROMANCE(victim) == 4) { /* Are they being asked out? */
		send_to_char("But they're being asked out! That would be rude!\r\n", ch);
		return;
		}
		else if (ROMANCE(victim) == 5) { /* Are they already proposing? */
		send_to_char("But they're already proposing!\r\n", ch);
		return;
		}
		else if (ROMANCE(victim) == 6) { /*Asking someone? */
			send_to_char("But they're already asking someone else!\r\n", ch);
			return;
		}
		else if (ROMANCE(victim) > 6) { /* Any errors in the module? */
		send_to_char("ERROR IN ROMANCE MODULE: Romance Factor > 5!\r\n", ch);
		return;
		}
		/* Okay, we've established you're both dating someone.. */
		else if (strcmp(PARTNER(ch), GET_NAME(victim))) {
			/* But are you dating them? */
			sprintf(buf, "But you're not dating %s!\r\n", GET_NAME(victim));
			send_to_char(buf, ch);
			return;
		}
		else if (strcmp(PARTNER(victim), GET_NAME(ch))) {
			/* Are they dating you? We shouldn't need this, though.. */
			sprintf(buf, "But %s isn't dating you!\r\n", GET_NAME(victim));
			send_to_char(buf, ch);
			return;
		}
		/* Okay, you're dating each other, now we can get on with it! */
		else {
			act("You kneel in front of $N and ask for $S hand in marriage!\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n kneels before you and asks for your hand in marriage!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n kneels before $N, asking for $S hand in marriage!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			/* We've informed the world, now change the vars.. */
			ROMANCE(ch) = PASKING;
			ROMANCE(victim) = PROPOSED_TO;
			/* Now all the partner has to do is accept or reject. */
			return;
		}
	}
}

/* Romance Module: Break up */

ACMD(do_breakup)
{
	struct char_data *victim;
 /* First, standard checks: Are you in a relationship? */
	if (ROMANCE(ch) == 0) {
		send_to_char("But you're not romantically involved!\r\n", ch);
		return;
	}
	else if (ROMANCE(ch) == 3) {
		sprintf(buf, "But you're already married! You have to DIVORCE %s!\r\n", PARTNER(ch));
		send_to_char(buf, ch);
		return;
	}
	else {
		/* Okay, you're involved.. */
		one_argument(argument, arg);
		if(!*arg) { /* Break up with noone? */
			send_to_char("Whom do you want to break up with?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Break up with yourself? */
			send_to_char("You can't break up with yourself!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if (ROMANCE(victim) == 0) {
		send_to_char("But they're not romantically involved!\r\n", ch);
		return;
		}
		else if ((ROMANCE(victim) == 1) 
                 && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're dating %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if ((ROMANCE(victim) == 2) 
                 && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're engaged to %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if ((ROMANCE(victim) == 3) 
                   && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're married to %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if ((ROMANCE(victim) == 3) 
                  && (!strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "They're already married to you! You have to DIVORCE %s!\r\n", GET_NAME(victim));
			send_to_char(buf, ch);
			return;
		}
		else if (strcmp(PARTNER(victim), GET_NAME(ch))) {
			sprintf(buf, "But %s isn't involved with you!\r\n", GET_NAME(victim));
			send_to_char(buf, ch);
			return;
		}
		else {
			/* Okay, they're involved and with you... */
			/* Now we break them up! How FUN! */
			if ((ROMANCE(ch) == 1) && (ROMANCE(victim) == 1)) {
				/* For dating */
			act("You inform $N that you will no longer date $S!\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n dumps you, tearing your heart out in the process!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n sends $s relationship with $N to Splitsville!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			/* Now set the variables to 0 */
			free(PARTNER(ch));free(PARTNER(victim));
                        PARTNER(ch) = NULL;
			PARTNER(victim) = NULL;
			ROMANCE(ch) = 0;
			ROMANCE(victim) = 0;
			/*namesave(ch, victim);*/ 
			save_char(ch, NOWHERE);
			save_char(victim, NOWHERE);
			/* Done. You've dumped them. */
			return;
			}
			else if ((ROMANCE(ch) == 2) && (ROMANCE(victim) == 2)) {
				/* For engagements */
			act("You call off your wedding with $N.\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n doesn't want to marry you anymore! The wedding's off!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n nullifies $s engagement with $N! The wedding's off!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			/* Now set the variables to 0 */
			free(PARTNER(ch)); free(PARTNER(victim));
                        PARTNER(ch) = NULL;
			PARTNER(victim) = NULL;
			ROMANCE(ch) = 0;
			ROMANCE(victim) = 0;
			/*namesave(ch, victim);*/ 
			save_char(ch, NOWHERE);
			save_char(victim, NOWHERE);
			/* Done. You've dumped them. */
			return;
			}
			else if ((ROMANCE(ch) == 4) && (ROMANCE(victim) == 4)) {
				/* For dating/askouts */
			act("You inform $N that you no longer wish to date $S!\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n doesn't feel like dating you anymore!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n decides not to date $N.\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			/* Now set the variables to 0 */
			free(PARTNER(ch)); free(PARTNER(victim));
                        PARTNER(ch) = NULL;
			PARTNER(victim) = NULL;
			ROMANCE(ch) = 0;
			ROMANCE(victim) = 0;
			/*namesave(ch, victim);*/  
			save_char(ch, NOWHERE);
			save_char(victim, NOWHERE);
			/* Done. You've dumped them. */
			return;
			}
			else if ((ROMANCE(ch) == 5) && (ROMANCE(victim) == 5)) {
				/* For engagements/proposals */
			act("You cancel your wedding proposal to $N.\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n doesn't want to marry you anymore!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n nullifies $s engagement proposal with $N!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			/* Now set the variables to back to Dating */
			/* This is the only exception. You can change it to cancel
				Dating status as well. */
			ROMANCE(ch) = 1;
			ROMANCE(victim) = 1;
			/* Done. You've cancelled your proposal them. */
			return;
			}
			else { /* Guess you're not involved after all. */
				sprintf(buf, "But you're not involved with %s!", GET_NAME(victim));
				send_to_char(buf, ch);
				return;
			}
		}
	}
}

/* Function for the actual marriage */
void marry_them (struct char_data *ch, struct char_data *victim, struct char_data *imm)
{
 /* Do standard checks.. */
	 if (crashcheck_alpha(victim, imm) == 1)
		{
			return;
		}
	 else if (crashcheck_alpha(ch, imm) == 1)
		{
			return;
		}
	 if (ROMANCE(ch) != 2) {
		/* Groom isn't engaged */
		sprintf(buf, "But %s isn't engaged!\r\n", GET_NAME(ch));
		send_to_char(buf, imm);
		return;
	}
	else if (ROMANCE(victim) != 2) {
		/* Bride isn't engaged */
		sprintf(buf, "But %s isn't engaged!\r\n", GET_NAME(victim));
		send_to_char(buf, imm);
		return;
	}
	else if (strcmp(PARTNER(ch), GET_NAME(victim))) {
		/* Not engaged to each other */
		send_to_char("But they're not engaged to each other!\r\n", imm);
		return;
	}
	else if (strcmp(PARTNER(victim), GET_NAME(ch))) {
		/* Not engaged to each other */
		send_to_char("But they're not engaged to each other!\r\n", imm);
		return;
	}
	else if (check_samesex(ch, victim) == 1) {
		/* Same Sex Marriages? */
		send_to_char("Same-sex marriages are not allowed.\r\n", imm);
		return;
	}
	else {
		if (GET_SEX(ch) != GET_SEX(victim)) {
			/* Regular Marriage */
		/* They're engaged to each other, now perform the marriage. */
		sprintf(buf, "%s declares you married to %s!\r\n", GET_NAME(imm), PARTNER(ch));
		send_to_char(buf, ch);
		sprintf(buf, "%s declares you married to %s!\r\n", GET_NAME(imm), PARTNER(victim));
		send_to_char(buf, victim);
		sprintf(buf, "You declare %s and %s man and wife!\r\n", GET_NAME(ch), GET_NAME(victim));
		send_to_char(buf, imm);
		sprintf(buf, "%s declares %s and %s man and wife!\r\n", GET_NAME(imm), GET_NAME(ch), GET_NAME(victim));
		act(buf, TRUE, 0, 0, 0, TO_ROOM);
		}
		else { /* Same-sex Marriage */
		sprintf(buf, "%s declares you married to %s!\r\n", GET_NAME(imm), PARTNER(ch));
		send_to_char(buf, ch);
		sprintf(buf, "%s declares you married to %s!\r\n", GET_NAME(imm), PARTNER(victim));
		send_to_char(buf, victim);
		sprintf(buf, "You declare %s and %s married!\r\n", GET_NAME(ch), GET_NAME(victim));
		send_to_char(buf, imm);
		sprintf(buf, "%s declares %s and %s married!\r\n", GET_NAME(imm), GET_NAME(ch), GET_NAME(victim));
		act(buf, TRUE, 0, 0, 0, TO_NOTVICT);
		}
                free(PARTNER(ch)); free(PARTNER(victim));
		PARTNER(ch) = strdup(GET_NAME(victim));
		PARTNER(victim) = strdup(GET_NAME(ch));
		/* Just in case! */
		ROMANCE(ch) = 3;
		ROMANCE(victim) = 3;
		/* Now we're set! Save it! */
		/*namesave(ch, victim);*/ 
		save_char(ch, NOWHERE);
		save_char(victim, NOWHERE);
		return;
	}
}



/* Romance Module -- Marry Two People */
/* THIS SHOULD BE AN IMM-LEVEL COMMAND! */

ACMD(do_marry)
{
	struct char_data *groom;
	struct char_data *bride;
	char groom_name [MAX_INPUT_LENGTH];
	char bride_name [MAX_INPUT_LENGTH];
  argument = one_argument(argument, groom_name);
  one_argument(argument, bride_name);
 if(!(groom = get_char_room_vis(ch, groom_name))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
 else if(!(bride = get_char_room_vis(ch, bride_name))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
	}
 if (groom_name == bride_name) {
	 /* Groom is the Bride? */
	 send_to_char("You can't marry someone to themself!\r\n", ch);
	 return;
 }
 else if (groom == ch)
 {
	 /* Can't perform a ceremony on yourself. */
	 send_to_char("Sorry, you have to get someone ELSE to perform the ceremony!\r\n", ch);
	 return;
 }
  else if (bride == ch)
 {
	 /* Can't perform a ceremony on yourself. */
	 send_to_char("Sorry, you have to get someone ELSE to perform the ceremony!\r\n", ch);
	 return;
 }
 else if ((!groom) || (!bride)) {
                send_to_char("Which couple do you wish to marry?\r\n", ch);
                return;
        }
 else { /* Let the marry function check the rest. */
	 marry_them(groom, bride, ch);
 }
}

/* Romance Module - Divorce */

ACMD(do_divorce)
{
struct char_data *victim;
 /* First, standard checks: Are you in a relationship? */
	if (ROMANCE(ch) != 3) {
		send_to_char("But you're not married!\r\n", ch);
		return;
	}
	else {
		/* Okay, you're involved.. */
		one_argument(argument, arg);
		if(!*arg) { /* Break up with noone? */
			send_to_char("Whom do you want to divorce?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Break up with yourself? */
			send_to_char("You can't divorce yourself!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if (ROMANCE(victim) == 0)
		{
		send_to_char("But they're not romantically involved!\r\n", ch);
		return;
		}
		else if ((ROMANCE(victim) == 1) 
                  && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're dating %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if ((ROMANCE(victim) == 2) 
                   && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're engaged to %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if ((ROMANCE(victim) == 3) 
                  && (strcmp(PARTNER(victim), GET_NAME(ch))))
		{
			sprintf(buf, "But they're married to %s, not you!\r\n", PARTNER(victim));
			send_to_char(buf, ch);
			return;
		}
		else if (strcmp(PARTNER(victim), GET_NAME(ch))) {
			sprintf(buf, "But %s isn't involved with you!\r\n", GET_NAME(victim));
			send_to_char(buf, ch);
			return;
		}
		else {
			/* Okay, they're involved and with you... */
			/* Now we break them up! How FUN! */
			act("You yank $N's wedding ring off and throw it on the floor!\r\n", TRUE, ch, 0, victim, TO_CHAR);
			act("$n yanks $s wedding ring off and throws it on the floor!\r\n", TRUE, ch, 0, victim, TO_VICT);
			act("$n yanks $s wedding ring from $N off and throws it on the floor! DIVORCE!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
			free(PARTNER(ch)); free(PARTNER(victim)); 
                        PARTNER(ch) = NULL;
			PARTNER(victim) = NULL;
			ROMANCE(ch) = 0;
			ROMANCE(victim) = 0;
                        /*namesave(ch,victim);*/
			/* It's all reset, save changes. */
			changesave(ch, victim);
		}
	}
}


ACMD (mate_toggle) {
	/* Romance/Mating Toggle On/Off
	 * Precondition: Person must be single + not pregnant
	 * Postcondition: If the person wants to not be bothered with
	 * M/R-mod thingies, then this toggle will prevent that.
	 * If they did not and now want to be involved in the
	 * social scene, then now they can.
	 */
	if(ROMANCE(ch) > 0) {
		send_to_char("Sorry, you have to be single to turn mating off.\n", ch);
	}
	/* Both Conditions pass.. Check + Set! */
	if(ROMANCE(ch) == 0) { ROMANCE(ch) = -1;  } // OFF
	else if(ROMANCE(ch) == -1) { ROMANCE(ch) = 0; } // Single
}

ACMD(do_smartmud) {}

ACMD(do_firefight) {
 struct obj_data *fire;
// obj_vnum fire_num = 2580;
 int money = 100;
// int extra = (GET_CAREERRANK(ch) * 25000);

 if (!PLR_FLAGGED2(ch, PLR2_FFIGHTING)) {
  send_to_char("You aren't a firefighter!\r\n", ch);
  return;
 }

 if (!(fire = get_obj_in_list_vis(ch,"Fire", world[ch->in_room].contents))) {
  send_to_char("The fire isn't here!!!\r\n", ch);
  return;
 }

 WAIT_STATE(ch, PULSE_VIOLENCE);
 extract_obj(fire);
 send_to_char("You have extinguished the fire! Onwards!\r\n", ch);
 SET_BIT(PLR_FLAGS2(ch), PLR2_FIREOUT);
 sprintf(buf, "You have received %d coins\r\n", (money));
 GET_GOLD(ch) += (money);
 send_to_char(buf, ch);

 if (PLR_FLAGGED2(ch, TIMER_ONE)) {
  REMOVE_BIT(PLR_FLAGS2(ch), TIMER_ONE);
 } 
 if (PLR_FLAGGED2(ch, TIMER_TWO)) {
  REMOVE_BIT(PLR_FLAGS2(ch), TIMER_TWO);
 } 
 if (PLR_FLAGGED2(ch, TIMER_THREE)) {
  REMOVE_BIT(PLR_FLAGS2(ch), TIMER_THREE);
 } 
 if (PLR_FLAGGED2(ch, TIMER_FOUR)) {
  REMOVE_BIT(PLR_FLAGS2(ch), TIMER_FOUR);
 } 
 GET_CAREERRANK(ch)++;

}

ACMD(do_stop) {
 zone_rnum i;
 int num = 0;
 obj_vnum yo = 2580;
 struct obj_data *fire;

 if (PLR_FLAGGED2(ch, PLR2_FISHING)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISHING);
 if (PLR_FLAGGED2(ch, PLR2_FISH_ON)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FISH_ON);
  }
  send_to_char("You decide to stop fishing\r\n", ch);
  return;
 }

 if (PLR_FLAGGED2(ch, PLR2_CUTTING)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_CUTTING);
  send_to_char("You decide to stop chopping wood.\r\n", ch);
  return;
 }

 if (PLR_FLAGGED2(ch, PLR2_MINING)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_MINING);
  send_to_char("You decide to stop mining.\r\n", ch);
  return;
 }

 if (PLR_FLAGGED2(ch, PLR2_HUNTING)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_HUNTING);
 if (PLR_FLAGGED2(ch, PLR2_KILLMOB)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_KILLMOB);
  }
  send_to_char("You decide to stop hunting\r\n", ch);
  return;
 }

 if (PLR_FLAGGED2(ch, PLR2_FFIGHTING)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FFIGHTING);
 if (PLR_FLAGGED2(ch, PLR2_FIREOUT)) {
  REMOVE_BIT(PLR_FLAGS2(ch), PLR2_FIREOUT);
  }
  send_to_char("You decide to stop firefighting\r\n", ch);
  GET_CAREERRANK(ch) = 0;
  for (i = 0; i <= top_of_zone_table; i++) {
    for (num = 0, fire = object_list; fire; fire = fire->next)
   {
   if (world[fire->in_room].zone == i && GET_OBJ_VNUM(fire) == yo)
       extract_obj(fire);
     }
   }
  if (PLR_FLAGGED2(ch, TIMER_ONE))
    REMOVE_BIT(PLR_FLAGS2(ch), TIMER_ONE);
  if (PLR_FLAGGED2(ch, TIMER_TWO)) 
    REMOVE_BIT(PLR_FLAGS2(ch), TIMER_TWO);
  if (PLR_FLAGGED2(ch, TIMER_THREE)) 
    REMOVE_BIT(PLR_FLAGS2(ch), TIMER_THREE); 
  if (PLR_FLAGGED2(ch, TIMER_FOUR)) 
    REMOVE_BIT(PLR_FLAGS2(ch), TIMER_FOUR); 
  return;
 }

 send_to_char("You are not currently in a career.\r\n", ch);
}


ACMD(do_gjob) {
 int maxlen = 10;  


  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!(*argument)) {
     send_to_char("You need to define a job.\r\n", ch);
     return;
     }

  if (ch->master != NULL && !AFF_FLAGGED(ch, AFF_GROUP)) {
     send_to_char("You need to be following someone first.\r\n", ch);
     return;
     } else {

   if (strlen(argument) > maxlen)
     argument[maxlen] = '\0';

     send_to_char("You have changed your job!\r\n",ch);
     GET_GJOB(ch) = str_dup(argument);
     return;
     }
  }

ACMD(do_gtitle) {

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!(*argument)) {
     send_to_char("You need to define a title.\r\n", ch);
     return;
     }
  if (ch->followers != NULL && !AFF_FLAGGED(ch, AFF_GROUP)) {
     send_to_char("You need to be grouped first.\r\n", ch);
     return;

     } else {
   if (GET_GTITLE(ch) != NULL)
     free(GET_GTITLE(ch));

     send_to_char("You have changed your group's title!\r\n",ch);
     GET_GTITLE(ch) = str_dup(argument);
     return;
     }
  }

ACMD(do_challenge)
{
	struct char_data *victim;

	one_argument(argument, arg);
	if (!*arg) { /* What, ask no one out? */
		send_to_char("Uh, whom do you wish to duel?\r\n", ch);
                 return;
	}
        else if (ch->in_room == real_room(frozen_start_room))
        {
           send_to_char("&RBad Person! Trying to escape!&n\r\n", ch);
           return;
        }
	else if (!(victim = get_char_room_vis(ch, arg))) {
		/* Is that person here? Nope! */
		send_to_char(NOPERSON, ch);
		return;
	}
        else if (num_pc_in_room(&world[real_room(1527)]) != 0 &&
                 num_pc_in_room(&world[real_room(1528)]) != 0 &&
                 num_pc_in_room(&world[real_room(1529)]) != 0 &&
                 num_pc_in_room(&world[real_room(1530)]) != 0 &&
                 num_pc_in_room(&world[real_room(1531)]) != 0)

        {
            send_to_char("&RSomeone else is sparring right now!&n\r\n", ch);
            return;
        }
	else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
	else if (victim == ch) {
		send_to_char("Sorry, You can't duel yourself.\r\n", ch);
		return;
	}
	else { 

	act("You challenge $N to a sparring match...\r\n", TRUE, ch, 0, victim, TO_CHAR);
	act("$n is challenging you to a sparring match!\r\n", TRUE, ch, 0, victim, TO_VICT);
	act("$n challenges $N to a sparring match!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);
	DUEL(victim) = 1; 
	DUEL(ch) = 5; 
	  /* NOTE: PARTNER(ch) must be reset in DO_REJECT */
	/* At this point, the Victim can either ACCEPT or REJECT the proposition. */
	}
}


ACMD(do_acceptchallenge)
{
	struct char_data *victim;
	if(DUEL(ch) != 1) { 
		send_to_char("You're not being challenged\r\n", ch);
		return;
	}
	else { 
		one_argument(argument, arg);
		if(!*arg) { /* Accept no one? */
			send_to_char("Who do you want to accept?\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Accept yourself? */
			send_to_char("You can't accept yourself as an opponent!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
                        send_to_char("Error\r\n", ch);
			return;
		}
		else if(DUEL(victim) != 5) {
			send_to_char("But they haven't challenged you\r\n", ch);
			return;
		}
		else { /* Okay, all the tests pass.. */
			if((DUEL(ch) == 1) && (DUEL(victim) == 5)) {
			
				act("You take the challenge!", TRUE, ch, 0, victim, TO_CHAR);
				act("$n has agreed to fight you!", TRUE, ch, 0, victim, TO_VICT);
				act("$n has agreed to spar $N!", TRUE, ch, 0, victim, TO_NOTVICT);
                                // Reset everyone
				DPARTNER(victim) = GET_NAME(ch);
				DUEL(ch) = 0;
				DUEL(victim) = 0;
				/* Now they're set to fight. */
                                // move them

                                char_from_room(victim);
                                char_to_room(victim, real_room(number(1527,1531)));
                                char_from_room(ch);
                                char_to_room(ch, real_room(number(1527,1531)));

                                command_interpreter(ch, "look");                                
                                command_interpreter(victim, "look");
                                
                                save_char(ch, NOWHERE);
                                save_char(victim, NOWHERE);
				return;
			}
                        send_to_char("Challenge Incorrectly Set Up", ch);
		}
	}
}

ACMD(do_rejectchallenge)
{
	struct char_data *victim;
 	if(DUEL(ch) != 1) { /* You're not being asked out.. */
		send_to_char("You're not being challenged\r\n", ch);
		return;
	}
	else { 
		one_argument(argument, arg);
		if(!*arg) { /* Reject no one? */
			send_to_char("Theres noone here to fight\r\n", ch);
			return;
		}
		else if(!(victim = get_char_room_vis(ch, arg))) {
			/* Are they here? No! */
			send_to_char(NOPERSON, ch);
			return;
		}
		else if(victim == ch) { /* Reject yourself? */
			send_to_char("You can't reject yourself!\r\n", ch);
			return;
		}
		else if (crashcheck_alpha(victim, ch) == 1)
		{
			return;
		}
		else if(DUEL(victim) != 5)
                {
			send_to_char("They arn't challenging you!\r\n", ch);
			return;
		} 
		if ((DUEL(ch) == 1) && (DUEL(victim) == 5)) {
		
		act("You decline $N's invitation to duel.\r\n", TRUE, ch, 0, victim, TO_CHAR);
	        act("$n has decided not to fight you!\r\n", TRUE, ch, 0, victim, TO_VICT);
		act("$n declines to fight $N! What a wussy!\r\n", TRUE, ch, 0, victim, TO_NOTVICT);

	        DUEL(victim) = 0;
		DUEL(ch) = 0;
		changesave(ch, victim);
		return;	        
		}
                send_to_char("Challenge Incorrectly Set Up", ch);

	}
}

ACMD(do_shade)
{
  byte percent;
  if(GET_CLASS(ch) != CLASS_THIEF) return;
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SHADE)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(ch, AFF_SHADE))
  {
    send_to_char("You uncloak your body into visage.\r\n", ch);
    REMOVE_BIT(AFF_FLAGS(ch), AFF_SHADE);
    return;
  }
  send_to_char("You attempt to shade yourself.\r\n", ch);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > 60)
    return;

  SET_BIT(AFF_FLAGS(ch), AFF_SHADE);
  //improve_skill(ch, SKILL_SHADE);
}




//Religion Code

void sort_areligrank(struct char_data *ch) 
{
  FILE *religfile = fopen(ARELIGION_FILE, "r");
  FILE *religfile2 = fopen(ARELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }
  fclose(religfile);
  fprintf(religfile2, "<< Name: %s   Sacrifices: %d >>\n", GET_NAME(ch), GET_SACRIFICE(ch));
  fclose(religfile2);

  system("sort -rn -k 4 ../lib/etc/areligiontwo | head > ../lib/etc/areligion");
  system("rm ../lib/etc/areligiontwo");

  file_to_string_alloc(ARELIGION_FILE, &areligion);
  return;
}

void sort_preligrank(struct char_data *ch) {

  FILE *religfile = fopen(PRELIGION_FILE, "r");
  FILE *religfile2 = fopen(PRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }
  fclose(religfile);
  fprintf(religfile2, "<< Name: %s   Sacrifices: %d >>\n", GET_NAME(ch), GET_SACRIFICE(ch));
  fclose(religfile2);

  system("sort -rn -k 4 ../lib/etc/preligiontwo | head > ../lib/etc/preligion");
  system("rm ../lib/etc/preligiontwo");

  file_to_string_alloc(PRELIGION_FILE, &preligion);
  return;
}

void sort_dreligrank(struct char_data *ch) {

  FILE *religfile = fopen(DRELIGION_FILE, "r");
  FILE *religfile2 = fopen(DRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }
  fclose(religfile);
  fprintf(religfile2, "<< Name: %s   Sacrifices: %d >>\n", GET_NAME(ch), GET_SACRIFICE(ch));
  fclose(religfile2);

  system("sort -rn -k 4 ../lib/etc/dreligiontwo | head > ../lib/etc/dreligion");
  system("rm ../lib/etc/dreligiontwo");

  file_to_string_alloc(DRELIGION_FILE, &dreligion);
  return;
}

void sort_creligrank(struct char_data *ch) {

  FILE *religfile = fopen(CRELIGION_FILE, "r");
  FILE *religfile2 = fopen(CRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }
  fclose(religfile);
  fprintf(religfile2, "<< Name: %s   Sacrifices: %d >>\n", GET_NAME(ch), GET_SACRIFICE(ch));
  fclose(religfile2);

  system("sort -rn -k 4 ../lib/etc/creligiontwo | head > ../lib/etc/creligion");
  system("rm ../lib/etc/creligiontwo");

  file_to_string_alloc(CRELIGION_FILE, &creligion);
  return;
}

void sort_rreligrank(struct char_data *ch) {

  FILE *religfile = fopen(RRELIGION_FILE, "r");
  FILE *religfile2 = fopen(RRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }
  fclose(religfile);
  fprintf(religfile2, "<< Name: %s   Sacrifices: %d >>\n", GET_NAME(ch), GET_SACRIFICE(ch));
  fclose(religfile2);

  system("sort -rn -k 4 ../lib/etc/rreligiontwo | head > ../lib/etc/rreligion");
  system("rm ../lib/etc/rreligiontwo");

  file_to_string_alloc(RRELIGION_FILE, &rreligion);
  return;
}

void update_sacrifice(struct char_data *ch)
{
   GET_SACRIFICE(ch) = GET_SACRIFICE(ch) + 1;
   if(GET_LEVEL(ch) >= LVL_IMMORT)
      return;

   switch (GET_RELIGION(ch))
   {
    case RELIG_ATHEIST:
        sort_areligrank(ch);
    break;
    case RELIG_PHOBOS:
        sort_preligrank(ch);
    break;
    case RELIG_DEIMOS:
        sort_dreligrank(ch);
    break;
    case RELIG_CALIM:
        sort_creligrank(ch);
    break;
    case RELIG_RULEK:
        sort_rreligrank(ch);
    break;
   }
}


void aremoverelig(struct char_data *ch) 
{
  FILE *religfile = fopen(ARELIGION_FILE, "r");
  FILE *religfile2 = fopen(ARELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
      else ;
    }

  fclose(religfile);
  fclose(religfile2);

  system("cp ../lib/etc/areligontwo ../lib/etc/areligon");
  system("rm ../lib/etc/areligiontwo");

  file_to_string_alloc(ARELIGION_FILE, &areligion);
  return;
}
void premoverelig(struct char_data *ch) 
{
  FILE *religfile = fopen(PRELIGION_FILE, "r");
  FILE *religfile2 = fopen(PRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }

  fclose(religfile);
  fclose(religfile2);

  system("cp ../lib/etc/preligontwo ../lib/etc/preligon");
  system("rm ../lib/etc/preligiontwo");

  file_to_string_alloc(PRELIGION_FILE, &preligion);
  return;
}
void dremoverelig(struct char_data *ch) 
{
  FILE *religfile = fopen(DRELIGION_FILE, "r");
  FILE *religfile2 = fopen(DRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }

  fclose(religfile);
  fclose(religfile2);

  system("cp ../lib/etc/dreligontwo ../lib/etc/dreligon");
  system("rm ../lib/etc/dreligiontwo");

  file_to_string_alloc(DRELIGION_FILE, &dreligion);
  return;
}
void cremoverelig(struct char_data *ch) 
{
  FILE *religfile = fopen(CRELIGION_FILE, "r");
  FILE *religfile2 = fopen(CRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }

  fclose(religfile);
  fclose(religfile2);

  system("cp ../lib/etc/creligontwo ../lib/etc/creligon");
  system("rm ../lib/etc/creligiontwo");

  file_to_string_alloc(CRELIGION_FILE, &creligion);
  return;
}
void rremoverelig(struct char_data *ch) 
{
  FILE *religfile = fopen(RRELIGION_FILE, "r");
  FILE *religfile2 = fopen(RRELIGION_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just sacrificed*/
  while (fgets(buf, 80, religfile))
    {
      sscanf(buf, "<< Name: %s   Sacrifices: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(religfile2, "%s", buf);
    }

  fclose(religfile);
  fclose(religfile2);

  system("cp ../lib/etc/rreligontwo ../lib/etc/rreligon");
  system("rm ../lib/etc/rreligiontwo");

  file_to_string_alloc(RRELIGION_FILE, &rreligion);
  return;
}

ACMD(do_skillreset)
{
    int i;
    struct char_data *victim = NULL;
    one_argument(argument, buf);
    if (!*buf) return;
    if (!(victim = get_char_vis(ch, buf, FIND_CHAR_WORLD))) return;

    send_to_char("Your skills have been reset.\r\n", victim);
    for(i = 1; i <= MAX_SKILLS; i++)
      GET_SKILL(victim, i) = 0;

    GET_PRACTICES(victim) = GET_TOTAL_LEVEL(victim) + GET_GM_LEVEL(victim)*2;
}

ACMD(do_tick)
{
   int ticknum = 1;
   argument = one_argument(argument, buf);
   ticknum = atoi(buf);
   if ( ticknum <= 0 || ticknum > 10)
   {
      send_to_char("&RInvalid Number of Ticks!&n\r\n", ch);
      return;
   }
   sprintf(buf, "&W$n is waiting for %d ticks!&n", ticknum);
   act (buf, TRUE,ch,0,0,TO_ROOM);
   sprintf(buf, "&WYou announce that you are waiting for %d tick%s!&n",
ticknum, (ticknum == 1) ? "" : "s");
   send_to_char(buf, ch);
}
ACMD(do_land)
{
  if (affected_by_spell(ch, SPELL_FLOAT) || AFF_FLAGGED(ch, AFF_FLOAT))
  {
    act("&GYou float to the ground.&n",FALSE,ch,0,0,TO_CHAR);
    act("&G$n floats to the ground.&n",FALSE,ch,0,0,TO_ROOM);
    if (affected_by_spell(ch, SPELL_FLOAT))
      affect_from_char(ch, SPELL_FLOAT);
    if (AFF_FLAGGED(ch, AFF_FLOAT))
     REMOVE_BIT(AFF_FLAGS(ch), AFF_FLOAT);
  }
  else
  {
    send_to_char("&RYou arn't floating!&n\r\n", ch);
  }
}


ACMD(do_mark)
{
   struct obj_data *tobj = NULL;
   char item_name[MAX_STRING_LENGTH];
   bool target = FALSE;
   
   if (!PLR_FLAGGED(ch, PLR_ASSASSIN))
   {
     send_to_char("&nYou can't use this... yet.&n\r\n",ch);
     return;
   }

   one_argument(argument, item_name);

   if (!*item_name)
   {
     send_to_char("&RUsage: mark <item name>&n\r\n",ch);
     return;
   }

   if ((tobj = get_obj_in_list_vis(ch, item_name, ch->carrying)) != NULL)
     target = TRUE;

   if (!target)
   {
     send_to_char("&RPlease give a valid item.&n\r\n",ch);
     return;
   }    
   if (OBJ_FLAGGED(tobj, ITEM_ASSASSIN_ONLY))
   {
     send_to_char("&RThis item already has the mark of an assassin.&n\r\n", ch);
     return;
   }

   SET_BIT(GET_OBJ_EXTRA(tobj), ITEM_ASSASSIN_ONLY);
   SET_BIT(GET_OBJ_EXTRA(tobj), ITEM_UNIQUE_SAVE);

   act("&YYou mark $p with the &rblood&Y of the slain.&n", FALSE, ch, tobj, 0, TO_CHAR);
   act("&Y$n marks $p with the &rblood&Y of $s slain.&n", FALSE, ch, tobj, 0, TO_ROOM);
   return;
}
