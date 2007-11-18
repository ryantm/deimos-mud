/*
************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "dg_scripts.h"

extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern int max_exp_gain;
extern const char *dirs[];
extern int max_exp_loss;
extern int idle_rent_time;
extern int idle_max_level;
extern int idle_void;
extern int use_autowiz;
extern int min_wizlist_lev;
extern int free_rent;
extern int cmd;
extern room_vnum frozen_start_room;
extern room_rnum r_frozen_start_room;  
extern zone_rnum top_of_zone_table; 
/* Those two extern funtion */
void add_llog_entry(struct char_data *ch, int type);
void symptoms(struct char_data * ch);
void improve_skill(struct char_data *ch, int skill);
int find_first_step(room_rnum src, room_rnum target);
extern bool ISHAPPY, DOUBLEEXP, DOUBLEGOLD, GOLDCHIPPER;
/* local functions */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
void check_autowiz(struct char_data * ch);
void Crash_rentsave(struct char_data *ch, int cost);
int level_exp(int chclass, int level);
char *title_male(int chclass, int level);
char *title_female(int chclass, int level);
void update_char_objects(struct char_data * ch);	/* handler.c */
void reboot_wizlists(void);
ACMD(do_track);
ACMD(do_purge);
ACMD(do_goto);
ACMD(do_at);
ACMD(do_stop);

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = GET_INT(ch);

    /* Class calculations */
    switch (GET_CLASS(ch)) {
    case CLASS_THIEF:
      gain += 25;
      break;
    case CLASS_CLERIC:
      gain += 35;
      break;
    case CLASS_WARRIOR:
      gain += 15;
      break;
    case CLASS_MAGIC_USER:
      gain += 45;
      break;
    }

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += gain;
      break;
    case POS_RESTING:
      gain += (gain / 2);	/* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain / 4);	/* Divide by 4 */
      break;
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
  }

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_GOOD_REGEN))
    gain *= 3;

  if (AFF_FLAGGED(ch, AFF_MANASHIELD))
    gain = 0;

    gain += GET_SKILL(ch, SKILL_MP_REGEN) * 15;
  return (gain);
}


/* Hitpoint gain pr. game hour */
int hit_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch)*2;
  } else {
     gain = GET_CON(ch);

    /* Class/Level calculations */
    switch (GET_CLASS(ch)) {
    case CLASS_THIEF:
      gain += 40;
      break;
    case CLASS_CLERIC:
      gain += 30;
      break;
    case CLASS_WARRIOR:
      gain += 50;
      break;
    case CLASS_MAGIC_USER:
      gain += 20;
      break;
    }

    /* Skill/Spell calculations */

    /* Position calculations    */

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += gain;	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain / 4);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain / 8);	/* Divide by 8 */
      break;
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
  }

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_GOOD_REGEN))
    gain *= 3;

    gain += GET_SKILL(ch, SKILL_HP_REGEN) * 15;

  return (gain);
}



/* move gain pr. game hour */
int move_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = GET_DEX(ch);

    /* Class/Level calculations */
    switch (GET_CLASS(ch)) {
    case CLASS_THIEF:
      gain += 50;
      break;
    case CLASS_CLERIC:
      gain += 30;
      break;
    case CLASS_WARRIOR:
      gain += 40;
      break;
    case CLASS_MAGIC_USER:
      gain += 20;
      break;
    }

    /* Skill/Spell calculations */


    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain / 2);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain / 4);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain / 8);	/* Divide by 8 */
      break;
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
  }

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_GOOD_REGEN))
    gain *= 3;

    gain += GET_SKILL(ch, SKILL_MV_REGEN) * 15;

  return (gain);
}



void set_pretitle(struct char_data * ch, char *pretitle)
{
  if (pretitle == NULL)
     pretitle ='\0';    

  if (strlen(pretitle) > MAX_TITLE_LENGTH)
    pretitle[MAX_TITLE_LENGTH] = '\0';

  if (GET_PRETITLE(ch) != NULL)
    free(GET_PRETITLE(ch));

  GET_PRETITLE(ch) = str_dup(pretitle);
}

void set_title(struct char_data * ch, char *title)
{
  if (title == NULL) {
    if (GET_SEX(ch) == SEX_FEMALE)
      title = title_female(GET_CLASS(ch), GET_LEVEL(ch));
    else
      title = title_male(GET_CLASS(ch), GET_LEVEL(ch));
  }

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}

void set_lname(struct char_data * ch, char *lname)
{
  if (strlen(lname) > MAX_TITLE_LENGTH)
    lname[MAX_TITLE_LENGTH] = '\0';

  if (GET_LNAME(ch) != NULL)
    free(GET_LNAME(ch));

  GET_LNAME(ch) = str_dup(lname);
}

void check_autowiz(struct char_data * ch)
{
#if defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS)
  if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
    char buf[128];

#if defined(CIRCLE_UNIX)
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
#elif defined(CIRCLE_WINDOWS)
    sprintf(buf, "autowiz %d %s %d %s", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE);
#endif /* CIRCLE_WINDOWS */

    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
    reboot_wizlists();
  }
#endif /* CIRCLE_UNIX || CIRCLE_WINDOWS */
}



void gain_exp(struct char_data * ch, int gain)
{
  int max_exp = 2000000000;
  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (IS_NPC(ch)) {
    return;
  }
  if (gain > 0) {
    gain = MIN(max_exp_gain, gain); 
    if(GET_EXP(ch) + gain > max_exp || GET_EXP(ch) + gain < 0)
    {
      send_to_char("You cannot gain anymore exp!\r\n", ch);
      GET_EXP(ch) = max_exp;
      return;
    }  
    GET_EXP(ch) += gain;
    }
  if (gain < 0) {
    gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
    GET_EXP(ch) += gain;
  }

    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }


void gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;
 
  GET_EXP(ch) += gain;
  if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;
 
  if (!IS_NPC(ch)) {
    while (GET_LEVEL(ch) < LVL_IMPL &&
        GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }
 
    if (is_altered) {
      sprintf(buf, "%s advanced %d level%s to level %d.",
                GET_NAME(ch), num_levels, num_levels == 1 ? "" : "s",     
                 GET_LEVEL(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
        sprintf(buf, "You rise %d levels!\r\n", num_levels);
        send_to_char(buf, ch);
	GET_EXP(ch) = 0;
      }
      set_title(ch, NULL);
      check_autowiz(ch);
    }
  }
}


void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (IS_NPC(ch) || GET_COND(ch, condition) == -1)	/* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;

  switch (condition) {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default:
    break;
  }

}


void check_idling(struct char_data * ch)
{
  if (++(ch->char_specials.timer) > idle_void) {
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    } else if (ch->char_specials.timer > idle_rent_time) {
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 3);
      add_llog_entry(ch,LAST_IDLEOUT);
      if (ch->desc) {
	STATE(ch->desc) = CON_DISCONNECT;
	/*
	 * For the 'if (d->character)' test in close_socket().
	 * -gg 3/1/98 (Happy anniversary.)
	 */
	ch->desc->character = NULL;
	ch->desc = NULL;
      }
    if (ch->in_room != r_frozen_start_room) {
      if (free_rent)
	Crash_rentsave(ch, 0);
      else
	Crash_idlesave(ch);
      sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
      mudlog(buf, CMP, LVL_GOD, TRUE);
      extract_char(ch);
      }
    }
  }
}


void check_careers(void) {

  struct char_data *i, *next_char;
  int bite;
  obj_vnum fire_num = 2580;
  room_rnum rnum;
  int chance, wood_num, wood, stone_num, stone; /* replacement for bite */
  struct obj_data *block, *rock, *fire;

  for (i = character_list; i; i = next_char) {
    next_char = i->next;    

    if (PLR_FLAGGED2(i, PLR2_FISHING) &&
       !ROOM_FLAGGED(i->in_room, ROOM_FISH))
      REMOVE_BIT(PLR_FLAGS2(i), PLR2_FISHING);

    if (PLR_FLAGGED2(i, PLR2_FISHING)) {
      bite = number(1, 10);

      if (bite >= 7 && bite <= 8) {
        send_to_char("Time goes by... not even a nibble.\r\n", i);
      } else if (bite >= 6) {
       send_to_char("You feel a slight jiggle on your line.\r\n", i);
      } else if (bite >= 4) {
       send_to_char("You feel a very solid pull on your line!\r\n", i);
       SET_BIT(PLR_FLAGS2(i), PLR2_FISH_ON);

      } else if (bite >= 2) {
       send_to_char("Your line suddenly jumps to life, FISH ON!!!\r\n",
                     i);
       SET_BIT(PLR_FLAGS2(i), PLR2_FISH_ON);
      }
    }

   if (PLR_FLAGGED2(i, PLR2_CUTTING) &&
       SECT(i->in_room) != SECT_FOREST)
      REMOVE_BIT(PLR_FLAGS2(i), PLR2_CUTTING);

   if (PLR_FLAGGED2(i, PLR2_CUTTING)) {
      chance = number(1, 10);

      if (chance >= 4 && chance <= 10) {
        send_to_char("You continue on your cutting.\r\n", i);
      } else if (chance >= 3) {
       send_to_char("You successfully cut down the tree!\r\n", i);
       REMOVE_BIT(PLR_FLAGS2(i), PLR2_CUTTING);
     if (SECT(i->in_room) ==  SECT_FOREST) {
       wood = 2559;
       wood_num = real_object(wood);
       block = read_object(wood_num, REAL);
       sprintf(buf, "You receive %s! Good job!\r\n", block->short_description);
       send_to_char(buf, i);
       obj_to_char(block, i);
       return;
     } else
       send_to_char("You should never see this message, please report it.\r\n", i);
       return;
      }
    }

   if (PLR_FLAGGED2(i, PLR2_MINING) &&
       SECT(i->in_room) != SECT_MINE)
      REMOVE_BIT(PLR_FLAGS2(i), PLR2_MINING);

   if (PLR_FLAGGED2(i, PLR2_MINING)) {
      chance = number(1, 15);

      if (chance >= 3 && chance <= 15) {
        send_to_char("You continue on mining.\r\n", i);
      } else if (chance <= 2) {
       send_to_char("You successfully mine the ore!\r\n", i);
       REMOVE_BIT(PLR_FLAGS2(i), PLR2_MINING);
     if (SECT(i->in_room) == SECT_MINE) {
       stone = 2560;
       stone_num = real_object(stone);
       rock = read_object(stone_num, REAL);
       sprintf(buf, "You receive %s! Good job!\r\n", rock->short_description);
       send_to_char(buf, i);
       obj_to_char(rock, i);
       return;
     } else
       send_to_char("You should never see this message, please report it.\r\n", i);
       return;
      }
    }
    if (PLR_FLAGGED2(i, PLR2_HUNTING) &&
       SECT(i->in_room) !=  SECT_HUNTGROUNDS)
      REMOVE_BIT(PLR_FLAGS2(i), PLR2_HUNTING);

    if (PLR_FLAGGED2(i, PLR2_HUNTING)) {
      bite = number(1, 15);

      if (bite >= 11 && bite <= 15) {
        send_to_char("You can't seem to find any prey.\r\n", i);
      } else if (bite >= 7 && bite < 11) {
       send_to_char("You see a small rustling in the bushes.\r\n", i);
      } else if (bite >= 4 && bite < 7) {
       send_to_char("You can see a small outline of an animal in the distace !\r\n", i);
       SET_BIT(PLR_FLAGS2(i), PLR2_KILLMOB);

      } else if (bite >= 1) {
       send_to_char("An animal is directly in your sights!!!!\r\n",
                     i);
       SET_BIT(PLR_FLAGS2(i), PLR2_KILLMOB);
      }
    }
  if (PLR_FLAGGED2(i, PLR2_FFIGHTING)) {
    if (PLR_FLAGGED2(i, PLR2_FIREOUT)) {
       rnum = number(114, 213);
       fire = read_object(fire_num, VIRTUAL);
       obj_to_room(fire, rnum);
       sprintf(buf, "Your next mission is at %s, hurry!\r\n", world[rnum].name);
       send_to_char(buf, i);
       REMOVE_BIT(PLR_FLAGS2(i), PLR2_FIREOUT);
       SET_BIT(PLR_FLAGS2(i), TIMER_ONE);
     }
   }

    if (PLR_FLAGGED2(i, TIMER_ONE)) {
       SET_BIT(PLR_FLAGS2(i), TIMER_TWO);
       REMOVE_BIT(PLR_FLAGS2(i), TIMER_ONE);
       continue;
    }

    if (PLR_FLAGGED2(i, TIMER_TWO)) {
       REMOVE_BIT(PLR_FLAGS2(i), TIMER_TWO);
       send_to_char("You have failed your mission, FAILURE!\r\n", i);
       do_stop(i, "", 0, 0);
       continue;
    }

  }
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;
  ACMD(do_die);

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
	
	/* MatingMod - Extra Hunger/Thirst */
    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
   
    if (PLR_FLAGGED(i, PLR_DEAD)) {
      SET_BIT(PLR_FLAGS(i), PLR_DEADI);
      REMOVE_BIT(PLR_FLAGS(i), PLR_DEAD);
      send_to_char("Your soul begins ascending to heaven.\r\n", i);
      if (GET_LEVEL(i) < 30)
        send_to_char("If you type die, you will corpse instantly.\r\n", i);
      continue;
    }
    if (PLR_FLAGGED(i, PLR_DEADI)) {
      SET_BIT(PLR_FLAGS(i), PLR_DEADII);
      REMOVE_BIT(PLR_FLAGS(i), PLR_DEADI);
      send_to_char("A bright light shines in the distance.\r\n", i);
      if (GET_LEVEL(i) < 30)
        send_to_char("If you type die, you will corpse instantly.\r\n", i);
      continue;
    }
   if (PLR_FLAGGED(i, PLR_DEADII)) {
      SET_BIT(PLR_FLAGS(i), PLR_DEADIII);
      REMOVE_BIT(PLR_FLAGS(i), PLR_DEADII);
      send_to_char("You begin to see the gates of heaven\r\n", i);
      if (GET_LEVEL(i) < 30)
        send_to_char("If you type die, you will corpse instantly.\r\n", i);
      continue;
    }
    if (PLR_FLAGGED(i, PLR_DEADIII)) {
      REMOVE_BIT(PLR_FLAGS(i), PLR_DEADIII);
      send_to_char("Well, see ya.\r\n", i);
      do_die(i, "die", 0, 0);
      continue;
    }

   if (PRF_FLAGGED2(i, PRF2_AUTOTICK))
    send_to_char("&RTick&n\r\n", i);
	
    if (GET_POS(i) >= POS_STUNNED) {
      if (!PLR_FLAGGED2(i, PLR2_HAFF))
      {
        GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      }
      if (AFF_FLAGGED(i, AFF_POISON))
	if (damage(i, i, 2, SPELL_POISON) == -1)
	  continue;	/* Oops, they died. -gg 6/24/98 */
      if (GET_POS(i) <= POS_STUNNED)
	update_pos(i);
    } else if (GET_POS(i) == POS_INCAP) {
      if (damage(i, i, 1, TYPE_SUFFERING) == -1)
	continue;
    } else if (GET_POS(i) == POS_MORTALLYW) {
      if (damage(i, i, 2, TYPE_SUFFERING) == -1)
	continue;
    }
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) < idle_max_level && GET_ROOM_VNUM(i->in_room) != frozen_start_room)
	check_idling(i);
    }
   /* Romance Module -- Timeout Functions */
	/* If someone's been propositioned, and not yet accepted, then time it out. */
	if(ROMANCE(i) > 3) { /* Higher than 3 = been propositioned */
		if (ROMANCE(i) == 4) { /* They've been asked out.. */
			ROMANCE(i) = 0; /* Back to "Single" */
			send_to_char("The dating offer has expired..\r\n", i);
		}
		else if (ROMANCE(i) == 5) { /* They've been proposed to.. */
			ROMANCE(i) = 1; /* Back to "Dating" */
			send_to_char("The engagement offer has expired.. \r\n", i);
		}
		else if(ROMANCE(i) == 6)  { // They've asked someone out..
			ROMANCE(i) = 0;
			send_to_char("Your dating offer has expired.\r\n", i);
		}
		else if(ROMANCE(i) == 7) { // They've proposed.
			ROMANCE(i) = 1;
			send_to_char("Your proposal has expired.\r\n", i);
		}
		else { /* We should never get here. Something is wrong. */
			send_to_char("ERROR IN ROMANCE MODULE. Romance Value > 7\r\n",i);
		}
	}
	/* End Romance Module entry */
	/* MatingMod Entry -- Timeout Function */

  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;	/* Next in object list */
     
    /* If this is a portal */
    if (GET_OBJ_TYPE(j) == ITEM_PORTAL) {
      if (GET_OBJ_TIMER(j) > 0)
          GET_OBJ_TIMER(j)--;
      if (!GET_OBJ_TIMER(j)) {
       if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
        act("A glowing portal fades from existence.",
             TRUE, world[j->in_room].people, j, 0, TO_ROOM);
        act("A glowing portal fades from existence.",
             TRUE, world[j->in_room].people, j, 0, TO_CHAR);
        }
        extract_obj(j);
      }
    }

    /* If this is a corpse */
    if (IS_CORPSE(j)) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
	GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {

	if (j->carried_by)
	  act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	  act("A quivering horde of maggots consumes $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	}

	for (jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content;	/* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj, j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj, j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj, j->in_room);
	  else
	    core_dump();
	}
	extract_obj(j);
      }
    }
    /* If the timer is set, count it down and at 0, try the trigger */
    /* note to .rej hand-patchers: make this last in your point-update() */
    else if (GET_OBJ_TIMER(j)>0) {
      GET_OBJ_TIMER(j)--; 
      if (!GET_OBJ_TIMER(j))
        timer_otrigger(j);
    }
  }
}

void harvest_ticker(void) {

  struct char_data *i, *next_char;
 
  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;

  if (PLR_FLAGGED(i, PLR_HASHORSE) && !PLR_FLAGGED(i, PLR_FEDHORSE) && GET_HORSEHAPPY(i) > 50) {
    GET_HORSEHAPPY(i) = (GET_HORSEHAPPY(i) - 1);
    }
 
  if (PLR_FLAGGED(i, PLR_FEDHORSE)) {
    REMOVE_BIT(PLR_FLAGS(i), PLR_FEDHORSE);
    }
  }
}

void tracker(void) {

  struct char_data *ch, *next_char;
  struct char_data *vict;
  int dir;

  for (ch = character_list; ch; ch = next_char) {
    next_char = ch->next;
 
 if (PLR_FLAGGED(ch, PLR_TRACK)) {
  /* The person can't see the victim. */

  if (!(vict = get_char_vis(ch, TRACKING(ch), FIND_CHAR_WORLD))) {
    REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
    TRACKING(ch) = NULL;
    return;
  }

 if (IN_ROOM(ch) == IN_ROOM(vict) && PLR_FLAGGED(ch, PLR_TRACK)) {
   REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
   TRACKING(ch) = NULL;
   return;
  }

 if (IN_ROOM(ch) == IN_ROOM(vict) && !PLR_FLAGGED(ch, PLR_TRACK)) {
   send_to_char("You're already in the same room!!\r\n", ch);
   REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
   TRACKING(ch) = NULL;
   return;
  }

  /* 101 is a complete failure, no matter what the proficiency. */
  if (number(0, 101) >= GET_SKILL(ch, SKILL_TRACK)) {
    int tries = 10;
    /* Find a random direction. :) */
    do {
      dir = number(0, NUM_OF_DIRS - 1);
    } while (!CAN_GO(ch, dir) && --tries);
    sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
    send_to_char(buf, ch);
    return;
  }

  /* They passed the skill check. */
  dir = find_first_step(ch->in_room, vict->in_room);

  switch (dir) {
  case BFS_ERROR:
    send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
   REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
   TRACKING(ch) = NULL;
    break;
  case BFS_ALREADY_THERE:
    send_to_char("You're already in the same room!!\r\n", ch);
   REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
   TRACKING(ch) = NULL;
    break;
  case BFS_NO_PATH:
    sprintf(buf, "You can't sense a trail to %s from here.\r\n", HMHR(vict));
    send_to_char(buf, ch);
   REMOVE_BIT(PLR_FLAGS(ch), PLR_TRACK);
   TRACKING(ch) = NULL;
    break;
  default:	/* Success! */
    SET_BIT(PLR_FLAGS(ch), PLR_TRACK);
    TRACKING(ch) = str_dup(arg);
    sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
    send_to_char(buf, ch);
    perform_move(ch, dir, 1);
    improve_skill(ch, SKILL_TRACK);
    break;
   }
     }
   }
}

void goldchip(void) 
{
  struct descriptor_data *pt;
  struct obj_data *chip, *next_thing,*new_chip;
  obj_rnum thing = 11;
  room_vnum new = 0;

  for (chip = object_list; chip; chip = next_thing)
    { 
      next_thing = chip->next; // since loop changes object_list we need to do it this way.
      if (GET_OBJ_TYPE(chip) == ITEM_GOLDDISK)
	{
	  extract_obj(chip);
	}
    }
  
  new_chip = read_object(thing, VIRTUAL);  
  new = number(0, top_of_world);
  obj_to_room(new_chip, new);

  if (GOLDCHIPPER)  
  {
    sprintf(buf, "&YGold Chip: %s&n\r\n", world[new].name);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character)
	send_to_char(buf, pt->character);  
  }
}

void econupdate(void)
{
  return;
/*
  FILE *econfile = fopen(ECON_FILE, "r");
  struct descriptor_data *pt;
  int i;
  char buf[80];
  char name[32];
  for (i = 0; i < 5; i++)
  {
    if (fgets(buf, 80, econfile))
    {
      sscanf(buf, "<< Name: %s   Econrank: %*d >>", name);

      for (pt = descriptor_list; pt; pt = pt->next)
       if (STATE(pt) == CON_PLAYING && pt->character)
          if (strcmp(name, GET_NAME(pt->character)) == 0)
          {
              sprintf(buf, "&REconomic Earnings: +%d&n\r\n", 50 - i*10);
              send_to_char(buf, pt->character);
              if (GET_GOLD(pt->character) < 2000000000)
                GET_GOLD(pt->character) += 50 - i*10;
              break;
          }    
    }
    else break;
  }
  fclose(econfile);
*/
}
