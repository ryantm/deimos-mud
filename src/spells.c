/*
************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "interpreter.h"
#include "dg_scripts.h"

extern room_vnum mortal_start_room;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern room_rnum find_target_room(struct char_data * ch, char *rawroomstr);

extern int mini_mud;
extern int pk_allowed;

void clearMemory(struct char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type, int modifier);
void name_to_drinkcon(struct obj_data * obj, int type);
void name_from_drinkcon(struct obj_data * obj);
int compute_armor_class(struct char_data *ch);
bool shroud_success(struct char_data *ch, struct char_data *victim, bool 
quiet);

/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  if (ch == NULL || obj == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1);	 - not used */

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
	if (GET_OBJ_VAL(obj, 1) >= 0)
	  name_from_drinkcon(obj);
	GET_OBJ_VAL(obj, 2) = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) = water;
	name_to_drinkcon(obj, LIQ_WATER);
	weight_change_object(obj, water);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}


ASPELL(spell_recall)
{
  if (victim == NULL || IS_NPC(victim))
    return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
      return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_NO_RECALL | ROOM_NOESCAPE)) {
    send_to_char("A magical shroud prevents recall!\r\n", victim);
    return;
  }

  if (PLR_FLAGGED2(victim, PLR2_FFIGHTING)) {
    send_to_char("You cannot recall while on a mission!\r\n", victim);
    return;
  }

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, real_room(mortal_start_room));
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_arenburg_recall)
{
  if (victim == NULL || IS_NPC(victim))
    return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
      return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_NO_RECALL | ROOM_NOESCAPE)) {
    send_to_char("A magical shroud prevents recall!\r\n", victim);
    return;
  }

  if (PLR_FLAGGED2(victim, PLR2_FFIGHTING)) {
    send_to_char("You cannot recall while on a mission!\r\n", victim);
    return;
  }

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, real_room(8500));
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}


ASPELL(spell_teleport)
{
  room_rnum to_room;

  if (victim == NULL || IS_NPC(victim) || GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
      return;

  if (!IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC | ROOM_NOESCAPE))
    return;

  if (ch != victim) 
  {
    send_to_char("You can only teleport yourself!\r\n", ch); 
    return;
  }

  do {
    to_room = number(0, top_of_world);
  } while (!ROOM_FLAGGED(to_room, ROOM_TELEPORT));

  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  if (ch == NULL || victim == NULL)
    return;

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_SPARRING) && GET_POS(victim) <= POS_MORTALLYW)
      return;

  if (!IS_NPC(ch) &&GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, GET_LEVEL(ch) + 3 + GET_SKILL(ch, SPELL_SUMMON))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (IS_NPC(victim) && GET_POS(victim) == POS_DEAD) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (!IS_NPC(ch) && !IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) 
  {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (ROOM_FLAGGED(victim->in_room, ROOM_NOMAGIC | ROOM_NOESCAPE | ROOM_PEACEFUL)) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC | ROOM_NOESCAPE | ROOM_SPARRING | ROOM_PEACEFUL)) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if(!IS_NPC(ch) &&shroud_success(ch, victim, TRUE)) //Look for function in spells.c
  {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (GET_LEVEL(victim) >= (GET_LEVEL(ch) - 11)) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
      act("As the words escape your lips and $N travels\r\n"
	  "through time and space towards you, you realize that $E is\r\n"
	  "aggressive and might harm you, so you wisely send $M back.",
	  FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
    if (!IS_NPC(ch) && !IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) &&
	!PLR_FLAGGED(victim, PLR_KILLER)) {
      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
	      "%s failed because you have summon protection on.\r\n"
	      "Type NOSUMMON to allow other players to summon you.\r\n",
	      GET_NAME(ch), world[ch->in_room].name,
	      (ch->player.sex == SEX_MALE) ? "He" : "She");
      send_to_char(buf, victim);

      sprintf(buf, "You failed because %s has summon protection on.\r\n",
	      GET_NAME(victim));
      send_to_char(buf, ch);

      sprintf(buf, "%s failed summoning %s to %s.",
	      GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      return;
    }

  if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
      (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL, 0))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}


ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char *name = NULL;
  int j;
  if (!(ch && ch->player_specials && ch->player_specials->tmp_text))
  {
    send_to_char("You must supply a keyword for this spell.\r\n", ch);
    return;
  }
  name = strdup((ch)->player_specials->tmp_text);
  skip_spaces(&name);  
 
  level = GET_LEVEL(ch);
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->name))
    {
      continue;
    }
    if (IS_SET(GET_OBJ_EXTRA(i), ITEM_NOLOCATE))
      continue;
 
    if ((i->carried_by && !IS_NPC(i->carried_by)) || i->in_obj || (i->worn_by && !IS_NPC(i->worn_by)))
      continue;
    else if (i->carried_by || i->worn_by)
      sprintf(buf, "%s is in the possession of a mobile.\n\r",
              i->short_description);
//    else if (i->in_room != NOWHERE && 
//      (GET_OBJ_TYPE(i) != ITEM_PCCORPSE) && (GET_OBJ_TYPE(i) !=
//ITEM_ASSCORPSE))
//      sprintf(buf, "%s is somewhere.\n\r", i->short_description);
    else if (i->in_room != NOWHERE)
      sprintf(buf, "%s is in %s.\n\r", i->short_description, world[i->in_room].name);       
    else
      sprintf(buf, "%s's location is uncertain.\n\r",
              i->short_description);

    CAP(buf);
    send_to_char("&n", ch);
    send_to_char(buf, ch);
    j--;
  }
  send_to_char("&n", ch);

  if (j == level >> 1)
    send_to_char("&RCannot find the target of your spell!&n\r\n", ch);
}


ASPELL(spell_find_target)
{
  struct char_data *i;
  char *name;
  int j;
  
  if (!(ch && ch->player_specials && ch->player_specials->tmp_text))
  {
    send_to_char("You must supply a keyword for this spell.\r\n", ch);
    return;
  }

  name = strdup((ch)->player_specials->tmp_text);
  skip_spaces(&name);  

   //strcpy(name, fname(victim->player.name));
   level = GET_LEVEL(ch);
  j = level >> 1;

  for (i = character_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->player.name))
      continue;

   if (i->in_room != NOWHERE && GET_LEVEL(i) < LVL_IMMORT) {

  if (IS_NPC(i) && MOB_FLAGGED(i, MOB_NOLOCATE))
    continue;

  if(shroud_success(ch, i, TRUE)) //Look for function in spells.c
    continue;

  if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT)
  {
     send_to_char("&RSomeone peers into your mind for a second.&n\r\n", i);
  }
  else if (!IS_NPC(i) && GET_LEVEL(i) >= LVL_IMMORT)
  {
    sprintf(buf, "&R%s peers into your mind for a second.&n\r\n", GET_NAME(ch));
    send_to_char(buf, i);
  }

      sprintf(buf, "&n%s is in %s.&n\n\r", IS_NPC(i) ? i->player.short_descr : i->player.name, world[i->in_room].name);
   } else
      sprintf(buf, "&n%s's location is shrouded in mystery.&n\n\r",
              IS_NPC(i) ? i->player.short_descr : i->player.name);

    CAP(buf);
    send_to_char(buf, ch);
    j--;
  }

  if (j == level >> 1)
    send_to_char("&RCannot find the target of your spell!&n\r\n", ch);
}


ASPELL(spell_charm)
{
  int numfollowers = 0, cancharm = 0;
  struct follow_type *k, *next = NULL;
  struct affected_type af;
  int skill = 0;
  
  if (victim == NULL || ch == NULL)
    return;

  skill = GET_SKILL(ch, SPELL_CHARM);

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (!IS_NPC(victim))
    send_to_char("You can't charm a player!\r\n", ch);
  else if ((GET_LEVEL(victim) > GET_MAGE_LEVEL(ch)/2 +5) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("Your victim resists!\r\n", ch);
  else if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE))
    send_to_char("You fail because SUMMON protection is on!\r\n", ch);
  else if (AFF_FLAGGED(victim, AFF_SANCTUARY) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("Your victim resists!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOHIT) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("Your victim seems immune to your spell!\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if ((AFF_FLAGGED(victim, AFF_CHARM) || GET_MAGE_LEVEL(ch) < GET_LEVEL(victim)) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("You fail.\r\n", ch);
  /* player charming another player - no legal reason for this */
  else if (!IS_NPC(victim))
    send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_PARA, 0) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("Your victim resists!\r\n", ch);
  else {
    if (victim->master)
      stop_follower(victim);
  for (k = ch->followers; k; k = next) 
  {
      next = k->next;
      if (IS_NPC(k->follower))
        numfollowers++;
  }
  
  cancharm = 0; 
/*cancharm = skill /2;
  cancharm +=  GET_CHA(ch) - 18;
  cancharm = MAX(1, cancharm); 
*/  
  cancharm = MAX(1, (int)(skill/10.*(GET_CHA(ch)-15.)));

  
  if (numfollowers >= cancharm)
  {
    sprintf(buf, "You will and good looks can only command %d creatures!\r\n", cancharm);
    send_to_char(buf,ch);
    return;
  }

  add_follower(victim, ch);

    af.type = SPELL_CHARM;

    af.duration = 15+skill;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}


ASPELL(spell_identify)
{
  int i;
  int found;

  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d, Rent: %d, Min Level: %d\r\n",
	    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj), GET_OBJ_LEVEL(obj));
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      sprintf(buf, "This %s casts level %d: ", item_types[(int) GET_OBJ_TYPE(obj)], GET_OBJ_VAL(obj, 0));

      if (GET_OBJ_VAL(obj, 1) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 1)));
      if (GET_OBJ_VAL(obj, 2) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 2)));
      if (GET_OBJ_VAL(obj, 3) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 3)));
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts level %d: ", item_types[(int) GET_OBJ_TYPE(obj)], GET_OBJ_VAL(obj,0));
      sprintf(buf + strlen(buf), " %s\r\n", skill_name(GET_OBJ_VAL(obj, 3)));
      sprintf(buf + strlen(buf), "It has %d maximum charge%s and %d remaining.\r\n",
	      GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
	      GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", GET_OBJ_VAL(obj, 1),
	      GET_OBJ_VAL(obj, 2));
      sprintf(buf + strlen(buf), " for an average per-round damage of %.1f.\r\n",
	      (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\r\n", ch);
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
	send_to_char(buf, ch);
      }
    }
  } else if (victim) {		/* victim */

   if(shroud_success(ch, victim, FALSE))
     return;

    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim)->year, age(victim)->month,
	      age(victim)->day, age(victim)->hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
	    GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf + strlen(buf), "Level: %d, Hits: %d, Mana: %d\r\n",
	    GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim));
    sprintf(buf + strlen(buf), "AC: %d, Hitroll: %d, Damroll: %d\r\n",
	    GET_AC(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
    sprintf(buf + strlen(buf), "Str: %d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
	GET_STR(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
    send_to_char(buf, ch);

  }
}



/*
 * Cannot use this spell on an equipped object or it will mess up the
 * wielding character's hit/dam totals.
 */
ASPELL(spell_enchant_weapon)
{
  int i;
  int skill = 0;
  int canbreak = FALSE;
  int breakpercent = 0;
  int powerlevel = 0;

  if (ch == NULL || obj == NULL)
    return;

  if (GET_SKILL(ch, SPELL_ENCHANT_WEAPON) != level)
  { //This is a scroll
      skill = level;
      canbreak = FALSE;
  }
  else
      skill = GET_SKILL(ch, SPELL_ENCHANT_WEAPON);

  
  /* Either already enchanted or not a weapon. */
  if (GET_OBJ_TYPE(obj) != ITEM_WEAPON)
    return;

  if (OBJ_FLAGGED(obj, ITEM_MAGIC))
  {
    send_to_char("&RThis weapon already has magical properties.&n\r\n", ch);
    return;
  }

  /* Make sure no other affections. */
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (obj->affected[i].location != APPLY_NONE)
    {
      send_to_char("&RThis weapon already has magical properties.&n\r\n", ch);
      return;
    }

  SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
  SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE);

  powerlevel = obj->affected[0].modifier;
  breakpercent = powerlevel * 2 - skill/3;
  breakpercent = MIN(100, breakpercent);

  if (canbreak && breakpercent > number(0,100))
  {
    act("&M$p shatters under intense magical pressure.&n", FALSE, ch, obj, 0, TO_CHAR);
    extract_obj(obj);
    return;
  }

  obj->affected[0].location = APPLY_HITROLL;
  obj->affected[0].modifier = MIN(MAX(1, skill/2.5), 6);
  obj->affected[1].location = APPLY_DAMROLL;
  obj->affected[1].modifier = MIN(MAX(1, skill/2.5), 6);

  act("&Y$p glows yellow.&n", FALSE, ch, obj, 0, TO_CHAR);
}


ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (AFF_FLAGGED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\r\n", ch);
      else
        send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (AFF_FLAGGED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}

#define MOB_INTERPOSER   16
ASPELL(spell_interposing_hand)
{
  struct char_data *mob;

  mob = read_mobile(MOB_INTERPOSER, VIRTUAL);
  GET_MAX_HIT(mob)  = GET_HIT(ch);
  GET_HIT(mob)      = GET_MAX_HIT(mob);
  GET_LEVEL(mob)    = GET_LEVEL(ch);
  GET_AC(mob)       = 0;
  GET_MAX_MANA(mob) = 3 * GET_LEVEL(ch); /* max is 35th level to increase */
  if (GET_MAX_MANA(mob) > 105)
    GET_MAX_MANA(mob) = 105;
  GET_MANA(mob) = GET_MAX_MANA(mob);

  char_to_room(mob, victim->in_room);
  IS_CARRYING_W(mob) = 1000;
  IS_CARRYING_N(mob) = 100;
  remember(victim, mob);
  if (FIGHTING(victim))
   stop_fighting(victim);
  hit(mob, victim, TYPE_UNDEFINED);
}

ASPELL(spell_resurrection) {

  int diceroll = number(1, 100);
  int skill = GET_SKILL(ch, SPELL_RESURRECTION);
 
  if (victim == NULL || ch == NULL)
    return;

  if (GET_LEVEL(ch) >= 66)
    diceroll = 101;


  if (FIGHTING(ch))
  {
   send_to_char("Complex Spells while fighting!?\r\n", ch);
   return;
  }

  if (victim == ch)  
    send_to_char("Lie still you!\r\n", ch);
  if (GET_HIT(victim) > -6)
    send_to_char("He ain't dead boy!\r\n", ch);
  else {
   if (
      (diceroll < (30 - skill * 2) && GET_CLASS(ch) == CLASS_CLERIC)
   || (diceroll < (50 - skill * 2) && GET_CLASS(ch) == CLASS_MAGIC_USER)
   || (diceroll < (60 - skill * 2) && GET_CLASS(ch) == CLASS_THIEF)
   || (diceroll < (80 - skill * 2) && GET_CLASS(ch) == CLASS_WARRIOR)
   )
   {
    send_to_char("You fumble with the spell words...", ch);
    if(GET_CON(victim) + GET_WIS(ch) + GET_INT(ch) < 45)
    {
      //Failure
      send_to_char("You place your hands onto the dying corpse, and try to bring it back to life!\r\n", ch);
      sprintf(buf, "%s has been killed in an attempt to resurrect!\r\n", GET_NAME(victim));
      send_to_room(buf, ch->in_room);
      sprintf(buf, "%s utters a long magical spell, failing miserably, killing you!\r\n", GET_NAME(ch));
      send_to_char(buf, victim);

      command_interpreter(victim, "die");
      return;
    }
    send_to_char("But you regain your place in the spell due to your wisdom.\r\n", ch);
   }  
   if (GET_HIT(victim) <= -6 && !IS_NPC(victim)) {
    send_to_char("You place your hands onto the dying corpse, and bring it back to life!\r\n", ch);
    sprintf(buf, "%s has been brought back to life!\r\n", GET_NAME(victim));
    send_to_room(buf, ch->in_room);
    sprintf(buf, "%s utters a long magical spell, and brings you back to life!\r\n", GET_NAME(ch));
    send_to_char(buf, victim);
    GET_HIT(victim) = 1;
    REMOVE_BIT(PLR_FLAGS(victim), PLR_DEAD);
    REMOVE_BIT(PLR_FLAGS(victim), PLR_DEADI);
    REMOVE_BIT(PLR_FLAGS(victim), PLR_DEADII);
    REMOVE_BIT(PLR_FLAGS(victim), PLR_DEADIII);
    GET_POS(victim) = POS_STANDING;
     }
   }
 }

ASPELL(spell_refresh)
 {

  int gain = 50 + GET_SKILL(ch, SPELL_REFRESH);

  if (victim == NULL || ch == NULL)
    return; 
 
  if ((GET_MOVE(victim) == GET_MAX_MOVE(victim)))
    send_to_char("Movement is already full!\r\n", ch);
  else {
   if (!IS_NPC(victim)) {
    send_to_char("You quickly restrengthen your legs!\r\n", ch); 
    sprintf(buf, "%s fills your legs with more strength!\r\n", GET_NAME(ch));
    send_to_char(buf, victim);
    GET_MOVE(victim) += gain;
     }
   }
 } 

bool dispel_affect(struct char_data *vict, int skill, struct affected_type *af)
{
  if (!af) return FALSE;
  if (af->power < skill + number (-4,4))
  {
         if ((af->type > 0) && (af->type <= MAX_SPELLS))
         {
            if (*spell_wear_off_msg[af->type]) {
              send_to_char(spell_wear_off_msg[af->type], vict);
              send_to_char("\r\n", vict);
            }
         }

         affect_remove(vict, af);
         return TRUE;
  }
  return FALSE;
}

ASPELL(spell_dispel_magic)
{
   int skill = GET_SKILL(ch, SPELL_DISPEL_MAGIC);     
   struct affected_type *af = 0, *next;
   int counter = 0;
   bool removedspell = FALSE;

   assert(ch && (victim));
        
   if (victim && IS_NPC(victim) && MOB_FLAGGED(victim, MOB_NOHIT))
   {
      send_to_char("I don't think so.\r\n", ch);
      return;
   }
    for (af = victim->affected; af; af = next)
    {
      next = af->next;
      if (!dispel_affect(victim, skill, af))
      {
         sprintf(buf, "An effect jumps from %s body.\r\n", HSHR(victim));
         send_to_char(buf, ch);
         removedspell = TRUE;
      }
    }
    for (counter = 0; counter < 32; counter++)
    {
      if (IS_AFFECTED(victim, (1 << counter)))
      {
         if (IS_NPC(victim) 
         && (skill > number(0, 5)))
         {
            if(affects_wear_off[counter] != -1)
            {
              REMOVE_BIT(AFF_FLAGS(victim), (1 << counter));
              removedspell = TRUE;
              sprintf(buf, "An effect jumps from %s body.\r\n", HSHR(victim));
              send_to_char(buf, ch);
          
              if (*spell_wear_off_msg[affects_wear_off[counter]]) 
              {
                send_to_char(spell_wear_off_msg[affects_wear_off[counter]],victim);
                send_to_char("\r\n", victim);
              }
            }

         }
      }
    }


    if (!removedspell)
      send_to_char("Your dispell did nothing!\r\n", ch);
    /* this is aggressive to mobs
    if the mob is capable of fighting and not,
    start it fighting, taerin 4/30/98 */
        
    if (IS_NPC(victim) &&
      (GET_POS(victim) > POS_STUNNED) &&
      (FIGHTING(victim) == NULL) )
        set_fighting(victim, ch);
        
}

#define PORTAL_OBJ  20   /* the vnum of the portal object */

ASPELL(spell_gate)
{
  /* create a magic portal */
  struct obj_data *tmp_obj, *tmp_obj2;
  struct extra_descr_data *ed;
  struct room_data *rp, *nrp;
  struct char_data *tmp_ch = (struct char_data *) victim;
  char buf[512];
  int skill = 0;

  assert(ch);
  assert((level >= 0) && (level <= LVL_IMPL));

  if (GET_SKILL(ch, SPELL_GATE) != level)
      skill = level;
  else
      skill = GET_SKILL(ch, SPELL_GATE);


  /*
    check target room for legality.
   */
  rp = &world[ch->in_room];
  tmp_obj = read_object(PORTAL_OBJ, VIRTUAL);
  if (!rp || !tmp_obj) {
    send_to_char("The magic fails\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }
  if (IS_SET(rp->room_flags, ROOM_TUNNEL)) {
    send_to_char("There is no room in here to summon!\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }

  if (!(nrp = &world[tmp_ch->in_room])) {
    char str[180];
    sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
    log(str);
    send_to_char("The magic cannot locate the target\n", ch);
    extract_obj(tmp_obj);
    return;
  }

  if (!IS_NPC(ch) && ROOM_FLAGGED(tmp_ch->in_room, ROOM_NOMAGIC | ROOM_HOUSE | ROOM_ATRIUM | ROOM_NOESCAPE) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("Your target is protected against your magic.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }
   
  if(shroud_success(ch, victim, TRUE) && GET_LEVEL(ch) < LVL_IMMORT)
       return;

  if (!IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC | ROOM_NOESCAPE)) {
    send_to_char("Your may not escape this room.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }

  if ((GET_LEVEL(tmp_ch) >= LVL_IMMORT || IS_NPC(tmp_ch)) 
       && GET_LEVEL(ch) < LVL_IMMORT) 
  {
    send_to_char("Your target is protected against your magic.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }

  if (GET_POS(tmp_ch) == POS_DEAD && GET_LEVEL(ch) < LVL_IMMORT)
  {
    send_to_char("Your magic cannot lock onto the mind of the dead.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }
  
  sprintf(buf, "Through the mists of the portal, you can faintly see %s",nrp->name);

  CREATE(ed , struct extra_descr_data, 1);
  ed->next = tmp_obj->ex_description;
  tmp_obj->ex_description = ed;
  CREATE(ed->keyword, char, strlen(tmp_obj->name) + 1);
  strcpy(ed->keyword, tmp_obj->name);
  ed->description = str_dup(buf);

  tmp_obj->obj_flags.value[0] = tmp_ch->in_room;
  GET_OBJ_TIMER(tmp_obj) = GET_SKILL(ch, SPELL_GATE);
  obj_to_room(tmp_obj,ch->in_room);

  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

	/* Portal at other side */
   rp = &world[ch->in_room];
   tmp_obj2 = read_object(PORTAL_OBJ, VIRTUAL);
   if (!rp || !tmp_obj2) {
     send_to_char("The magic fails\n\r", ch);
     extract_obj(tmp_obj2);
     return;
   }
  sprintf(buf,"Through the mists of the portal, you can faintly see %s", rp->name);

  CREATE(ed, struct extra_descr_data, 1);
  ed->next = tmp_obj2->ex_description;
  tmp_obj2->ex_description = ed;
  CREATE(ed->keyword, char, strlen(tmp_obj2->name) + 1);
  strcpy(ed->keyword, tmp_obj2->name);
  ed->description = str_dup(buf);
  tmp_obj2->obj_flags.value[0] = ch->in_room;
  GET_OBJ_TIMER(tmp_obj) = GET_SKILL(ch, SPELL_GATE);
  obj_to_room(tmp_obj2,tmp_ch->in_room);
  act("$p suddenly appears.", TRUE, tmp_ch, tmp_obj2, 0, TO_ROOM);
  act("$p suddenly appears.", TRUE, tmp_ch, tmp_obj2, 0, TO_CHAR);
}

bool shroud_success(struct char_data *ch, struct char_data *victim, bool
quiet)
//PRE: char is caster of spell, Victim is checked for magical shroud
//POST: True = Shroud Suceeded False = It failed or wasn't there at all
{
    int cmagiclevels = GET_TOTAL_LEVEL(ch)  - GET_WARRIOR_LEVEL(ch) -
GET_THIEF_LEVEL(ch);
    int vmagiclevels = GET_TOTAL_LEVEL(victim) - GET_WARRIOR_LEVEL(victim) - 
GET_THIEF_LEVEL(victim);

    if (IS_AFFECTED(victim, AFF_SHROUD))
    {
        int randval = number(-20, 40);
        if( GET_LEVEL(ch) < LVL_IMMORT && 
            GET_SKILL(ch, SPELL_SHROUD) <= GET_SKILL(victim, SPELL_SHROUD) &&
            cmagiclevels + randval < vmagiclevels)
        {
           if (!quiet)
           {
           sprintf(buf, "Your magic slips off %s as if %s didn't exist.\r\n", GET_NAME(victim),HESH(victim));
           send_to_char(buf, ch); 
           }
           sprintf(buf, "Your shroud protects you from %s's weak spells.\r\n", GET_NAME(ch));
           send_to_char(buf, victim);
           return TRUE;
        }
           sprintf(buf, "%s's magical shroud fails to stop your powerful magic.\r\n", GET_NAME(victim));
           send_to_char(buf, ch); 
           sprintf(buf, "Your shroud yields easily at the power of %s's spells.\r\n", GET_NAME(ch));
           send_to_char(buf, victim);
    }
   return FALSE;
}



ASPELL(dimension_phase)
{
    room_rnum location = ch->in_room;
    bool found = FALSE;
    room_vnum rnum = GET_ROOM_VNUM(ch->in_room);

    if (FIGHTING(ch)) 
    {
      send_to_char("Complicated spells in battle?\r\n", ch);
      return;
    }

    while (!found)
    {
      found = TRUE;
      rnum = GET_ROOM_VNUM(ch->in_room);    
      rnum += number(0,90) % 10 * 10 + number(0, 9);
      if ((location = real_room(rnum)) < 0) 
      {
         found = FALSE;
      }
    }
    char_from_room(ch);
    char_to_room(ch, location);
}

ASPELL(slow)
{
    room_rnum location = ch->in_room;
    bool found = FALSE;
    room_vnum rnum = GET_ROOM_VNUM(ch->in_room);

    if (FIGHTING(ch)) 
    {
      send_to_char("Complicated spells in battle?\r\n", ch);
      return;
    }

    while (!found)
    {
      found = TRUE;
      rnum = GET_ROOM_VNUM(ch->in_room);    
      rnum += number(0,90) % 10 * 10 + number(0, 9);
      if ((location = real_room(rnum)) < 0) 
      {
         found = FALSE;
      }
    }
    char_from_room(ch);
    char_to_room(ch, location);
}

ASPELL(spell_magic_eraser)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
  {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
  SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE);
  send_to_char("The object loses it's glow.\r\n", ch);
}


ASPELL(spell_sustainance)
{
  int skill = GET_SKILL(ch, SPELL_SUSTAINANCE)*4;
  if (GET_COND(ch, FULL) == skill && GET_COND(ch, THIRST) == skill)
  {
    send_to_char("&RYou feel the same.\r\n", ch);
    return;
  }
  if (GET_COND(ch, FULL) >= 0 )
    GET_COND(ch, FULL) = skill;
  if (GET_COND(ch, THIRST) >= 0 )
     GET_COND(ch, THIRST) = skill;
  sprintf(buf, "&YYou feel full and your thirst is quenched for %d ticks.&n\r\n", skill);
  send_to_char(buf, ch);
}


ASPELL(spell_golden_turkey)
{

  if (ch == NULL || victim == NULL)
    return;
  
  if (IS_NPC(victim) && GET_MOB_VNUM(victim) > 1584 && GET_MOB_VNUM(victim) < 1599)
  {
    act("&YA bright light expands from the Golden Turkey, spreading the spirit of thanksgiving to all.&n", TRUE,ch,0,0,TO_ROOM);
    act("&YYou hold out the Golden Turkey, which exudes a bright light that begins spreading the spirit of thanksgiving to all.&n", TRUE,ch,0,0,TO_ROOM);
    damage(ch, victim, 10000000, SPELL_TURKEY);
  }   
  else
  {
   act("&RYou must aim at a Turkey Snatcher, or those who hate Thanksgiving.", TRUE,ch,0,0,TO_CHAR);
  }
}
