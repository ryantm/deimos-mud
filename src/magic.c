/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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
#include "interpreter.h"
#include "dg_scripts.h"


#ifndef SINFO
#define SINFO spell_info[spellnum]
#endif

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;

extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;
extern char *spell_wear_off_msg[];

byte saving_throws(int class_num, int type, int level); /* class.c */
void clearMemory(struct char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
void ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim);
extern struct spell_info_type spell_info[];

/* local functions */
int mag_materials(struct char_data * ch, int item0, int item1, int item2, int extract, int verbose);
void perform_mag_groups(int level, struct char_data * ch, struct char_data * tch, int spellnum, int savetype);
int mag_savingthrow(struct char_data * ch, int type, int modifier);
void affect_update(void);
int baldam(struct char_data *ch, int primeclass, int maxpower);
bool resist(struct char_data *ch, struct char_data *victim, int primeclass, int spellnum);
/*
 * Saving throws are now in class.c as of bpl13.
 */


/*
 * Negative apply_saving_throw[] values make saving throws better!
 * Then, so do negative modifiers.  Though people may be used to
 * the reverse of that. It's due to the code modifying the target
 * saving throw instead of the random number of the character as
 * in some other systems.
 */
int mag_savingthrow(struct char_data * ch, int type, int modifier)
{
  /* NPCs use warrior tables according to some book */
  int class_sav = CLASS_WARRIOR;
  int save;

  if (!IS_NPC(ch))
    class_sav = GET_CLASS(ch);

  save = saving_throws(class_sav, type, GET_LEVEL(ch));
  save += GET_SAVE(ch, type);
  save += modifier;

  /* Throwing a 0 is always a failure. */
  if (MAX(1, save) < number(0, 99))
    return (TRUE);

  /* Oops, failed. Sorry. */
  return (FALSE);
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  struct affected_type *af, *next;
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)	/* No action */
	af->duration = -1;	/* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\r\n", i);
	    }
	affect_remove(i, af);
      }
    }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
	send_to_char("A wart sprouts on your nose.\r\n", ch);
	break;
      case 1:
	send_to_char("Your hair falls out in clumps.\r\n", ch);
	break;
      case 2:
	send_to_char("A huge corn develops on your big toe.\r\n", ch);
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}




/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 *
 * -1 = dead, otherwise the amount of damage done.
 */


int mag_damage(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int dam = 0;
  int skill = 0;

  if (victim == NULL || ch == NULL)
    return (0);

  if (GET_SKILL(ch, spellnum) != level)
      skill = level; //Object
  else
      skill = GET_SKILL(ch, spellnum); //Player


   if (!CAN_ASSASSIN(ch, victim))
   {
    send_to_char("You != Assassin.\r\n", ch);
    return (0);
  }

  if (MOB_FLAGGED(victim, MOB_NOHIT)) {
    send_to_char("You cannot hurt these type of mobs.\r\n", ch);
    return (0);
  }

  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("You're right to attack has been REVOKED.\r\n", ch);
    return (0);
  }
  
  switch (spellnum) {
    /* Mostly mages */
  case SPELL_MAGIC_MISSILE:
  case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
    if (IS_MAGIC_USER(ch))
      dam = number(15, 20);
    else
      dam = number(8, 15);
    break;
  case SPELL_BURNING_HANDS:
    if (IS_MAGIC_USER(ch))
      dam = number(30, 40);
    else
      dam = number(15, 25);
    break;
  case SPELL_SHOCKING_GRASP:
    if (IS_MAGIC_USER(ch))
      dam = number(50, 60);
    else
      dam = number(25, 30);
    break;
  case SPELL_LIGHTNING_BOLT:
    dam = number(skill*15, skill*25);
    break;
  case SPELL_COLOR_SPRAY:
    if (IS_MAGIC_USER(ch))
      dam = number(90, 120);
    else
      dam = number(50, 75);
    break;
  case SPELL_FIREBALL:
    dam = 100 + number(skill*25, skill*35);
    break;

  /* Bring in the big boys */
  case SPELL_DISINTEGRATE:
    if (IS_MAGIC_USER(ch))
      dam = number(250, 300);
    else
      dam = number(125, 150);
    break;

  case SPELL_ICEBOLT:
    dam = baldam(ch, CLASS_MAGE, 200 + number(skill*45, skill*55));
    break;


    /* Mostly clerics */
  case SPELL_DISPEL_EVIL:
    dam = dice(6, 8) + 6;
    if (IS_EVIL(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_GOOD(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;
  case SPELL_DISPEL_GOOD:
    dam = dice(6, 8) + 6;
    if (IS_GOOD(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_EVIL(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;


  case SPELL_CALL_LIGHTNING:
    if (IS_MAGIC_USER(ch))
      dam = dice(6, 8) + 4;
    else
      dam = dice(6, 6) + 4;
    break;

  case SPELL_HARM:
    dam = dice(8, 5) + 8;
    break;

  case SPELL_ENERGY_DRAIN:
    if (GET_LEVEL(victim) <= 2)
      dam = 100;
    else
      dam = dice(1, 10);
    break;

    /* Area spells */
  case SPELL_EARTHQUAKE:
      dam = number(skill*15, skill*25);
      if (GET_HIT(victim) - dam < 1 && number(1,5) != 5)
        dam = GET_HIT(victim) - 1;
    break;

  case SPELL_NOVASTORM:
      dam = 100 + number(skill*50, skill*55);
      if (GET_HIT(victim) - dam < 1 && number(1,5) != 5)
        dam = GET_HIT(victim) - 1;
    break;

  } /* switch(spellnum) */


  /* divide damage by two if victim makes his saving throw */
  //if (mag_savingthrow(victim, savetype, 0))
  //  dam /= 2;

  /* and finally, inflict the damage */
  return (damage(ch, victim, dam, spellnum));
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/
#define MAX_SPELL_AFFECTS 5	/* change if more needed */

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		      int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  bool accum_affect = FALSE, accum_duration = FALSE;
  const char *to_vict = NULL, *to_room = NULL;
  int i; int skill = 0;


  if (victim == NULL || ch == NULL)
    return;

  if (GET_SKILL(ch, spellnum) != level)
      skill = level; //Object
  else
      skill = GET_SKILL(ch, spellnum); //Player

  if (MOB_FLAGGED(victim, MOB_NOHIT)) {
    send_to_char("You cannot hurt these type of mobs.\r\n", ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type = spellnum;
    af[i].bitvector = 0;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }


  switch (spellnum) {

  case SPELL_CHILL_TOUCH:
   if (!IS_NPC(victim)) {
    return;
    }

    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype, 0))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    af[0].modifier = -1;
    accum_duration = FALSE;
    to_vict = "You feel your strength wither!";
    break;

  case SPELL_ARMOR:
    af[0].location = APPLY_AC;
    af[0].modifier = skill * 3;

    af[0].duration = 30;
    af[0].power = skill;
    accum_duration = FALSE;
    to_vict = "You feel someone protecting you.";
    break;

  case SPELL_BLESS:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = skill / 2;
    af[0].duration = 30;
    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -1;
    af[1].duration = 30;
    af[0].power = skill;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel righteous.";
    break;

  case SPELL_AUROROUS:
    af[0].location = APPLY_DAMROLL;
    af[0].modifier = skill;
    af[0].duration = skill/2;
    af[0].power = skill;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You are enveloped by a powerful aura.";
    break;

  case SPELL_BLINDNESS:
    if (MOB_FLAGGED(victim,MOB_NOBLIND)) 
    {// || mag_savingthrow(victim, savetype, 0)) {
      send_to_char("You fail.\r\n", ch);
      return;
    }

    if (!CAN_ASSASSIN(ch, victim))
    {
      send_to_char("You can't blind a non-assassin\r\n", ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4;
    af[0].duration = 2;
    af[0].bitvector = AFF_BLIND;
    af[0].power = skill;

    af[1].location = APPLY_AC;
    af[1].modifier = -40;
    af[1].duration = 2;
    af[1].bitvector = AFF_BLIND;
    af[1].power = skill;

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_CURSE:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    if (!CAN_ASSASSIN(ch, victim))
    {
      send_to_char("You can't curse a non-assassin\r\n", ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].duration = 1 + (GET_LEVEL(ch) / 2);
    af[0].modifier = -1;
    af[0].bitvector = AFF_CURSE;
    af[0].power = skill;

    af[1].location = APPLY_DAMROLL;
    af[1].duration = 1 + (GET_LEVEL(ch) / 2);
    af[1].modifier = -1;
    af[1].bitvector = AFF_CURSE;
    af[1].power = skill;

    accum_duration = FALSE;
    accum_affect = TRUE;
    to_room = "$n briefly glows red!";
    to_vict = "You feel very uncomfortable.";
    break;

  case SPELL_DETECT_ALIGN:
    af[0].duration = 30 + level;
    af[0].bitvector = AFF_DETECT_ALIGN;
    af[0].power = skill;
    accum_duration = FALSE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_INVIS:
    af[0].duration = 30 + level;
    af[0].bitvector = AFF_DETECT_INVIS;
    af[0].power = skill;

    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_MAGIC:
    af[0].duration = 30 + level;
    af[0].bitvector = AFF_DETECT_MAGIC;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_FLOAT:
    af[0].duration = 5;
    af[0].bitvector = AFF_FLOAT;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "Your feet float off the ground.";
    to_room = "$n's feet float off the ground.";
    break;

  case SPELL_INFRAVISION:
    af[0].duration = 30 + level;
    af[0].bitvector = AFF_INFRAVISION;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    af[0].duration = 12 + (GET_LEVEL(ch) / 4);
    af[0].modifier = 20;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    af[0].power = skill;
    accum_duration = TRUE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_MANASHIELD:
    af[0].duration = 1;
    af[0].power = skill;
    af[0].bitvector = AFF_MANASHIELD;

    accum_duration = FALSE;
    to_vict = "A blue forceshield surrounds your body...";
    to_room = "A forceshield surrounds $n's body.";
    break;

  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    if (!CAN_ASSASSIN(ch, victim))
    {
       send_to_char("You have violated Assassin rules!\r\n", ch);
       return;
     }

    af[0].location = APPLY_STR;
    af[0].duration = GET_LEVEL(ch);
    af[0].modifier = -2;
    af[0].bitvector = AFF_POISON;
    af[0].power = skill;
    
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_PROT_FROM_EVIL:
    af[0].duration = 30;
    af[0].bitvector = AFF_PROTECT_EVIL;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "You feel invulnerable!";
    break;

  case SPELL_SANCTUARY:
    af[0].duration = MAX(1,skill/2-1);
    accum_duration = FALSE;
    af[0].power = skill;

    if (GET_LEVEL(ch) >= LVL_DEITY || PLR_FLAGGED2(ch, PLR2_REALIMM)) 
    {
      af[0].duration = 99;
      accum_duration = FALSE;
      af[0].power = 99;
    }

   if (GET_SKILL(ch, spellnum) != level && skill > 10)
	af[0].duration = skill;
 
    af[0].bitvector = AFF_SANCTUARY;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_SLEEP:
  if (GET_POS(victim) <= POS_MORTALLYW) 
  {
    send_to_char("You can't sleep dead players!\r\n", ch);
    return;
   }

    if (!CAN_ASSASSIN(ch, victim))
    {
      send_to_char("You have violated Assassin rules!\r\n", ch);
      return;
    }

    if (GET_LEVEL(victim) > LVL_IMMUNE)
       return;
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;

    if (4. * skill < number(0, 80))
    {
      send_to_char("Your spell fails to work",ch);
      return;
    }

    if (resist(ch,victim, CLASS_MAGE, spellnum))
        return;

    af[0].duration = skill;
    af[0].bitvector = AFF_SLEEP;
    af[0].power = skill;

    if (GET_POS(victim) > POS_SLEEPING) 
    {
      send_to_char("You feel very sleepy...  Zzzz......\r\n", victim);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    af[0].location = APPLY_STR;
    af[0].duration = skill*8;
    af[0].modifier = skill;
    af[0].power = skill;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel stronger!";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "Your feel your awareness improve.";
    af[0].duration = GET_TOTAL_LEVEL(ch);
    af[0].bitvector = AFF_SENSE_LIFE;
    af[0].power = skill;
    accum_duration = FALSE;
    break;

  case SPELL_WATERWALK:
    af[0].duration = 30;
    af[0].bitvector = AFF_WATERWALK;
    af[0].power = skill;
    accum_duration = FALSE;
    to_vict = "You feel webbing between your toes.";
    break;

  case SPELL_WEB:
  
    if(!CAN_ASSASSIN(ch, victim))
    {
      send_to_char("You have violated the Assassin rules.\r\n", ch);
      return;
    }

    af[0].duration = skill;
    af[0].bitvector = AFF_WEB;
    af[0].power = skill;
    accum_duration = FALSE;
    to_vict = "You are stuck in gluey webs!";
    to_room = "$n's stuck in gluey webs!";
    break;

  case SPELL_SLOW:

    if(!CAN_ASSASSIN(ch, victim))
    {
      send_to_char("You can't slow a non-assassin\r\n", ch);
      return;
    }
    SET_BIT(AFF_FLAGS(victim), AFF_SLOW);
    damage(ch, victim, 1, SPELL_SLOW);
    to_vict = "You are slowed down by $N";
    to_room = "$N slows down $n.";
    send_to_char("You slow them down.\r\n", ch);
    break;

  case SPELL_GASEOUS:

    SET_BIT(AFF_FLAGS(victim), AFF_GASEOUS);

    af[0].duration = skill;
    af[0].bitvector = AFF_GASEOUS;

    accum_duration = FALSE;

    to_vict = "Your shimmers slightly and remains hazey from your words.";
    to_room = "$N's words cause $S body to shimmer slightly and remain in a hazey state.";
    break;

  case SPELL_BERSERK:
    if(GET_CLASS(ch) != CLASS_WARRIOR)
    {
      send_to_char("Only warriors may enter an enraged state.\r\n", ch);
      return;
    }
    af[0].duration = 10;
    af[0].bitvector = AFF_BERSERK;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "You feel the blood cloud your eyes!";
    to_room = "$N is engraged in bloodlust!";
    break;

  case SPELL_SHROUD:
    if(GET_CLASS(ch) != CLASS_CLERIC && GET_CLASS(ch) != CLASS_MAGIC_USER)
    {
      send_to_char("Only those of impecable magical training can shroud themselves from spells of an inquisitive nature.\r\n", ch);
      return;
    }
    af[0].duration = GET_SKILL(ch, SPELL_SHROUD);
    af[0].bitvector = AFF_SHROUD;
    af[0].power = skill;

    accum_duration = FALSE;
    to_vict = "Your body loses touch with the magical plane as you shroud yourself in ethereal darkness!";
    to_room = "$N has vanished from magical sight!";
    break;


  }
    
  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
  if (IS_NPC(victim) && !affected_by_spell(victim, spellnum))
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      if (AFF_FLAGGED(victim, af[i].bitvector)) {
	send_to_char(NOEFFECT, ch);
	return;
      }

  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void perform_mag_groups(int level, struct char_data * ch,
			struct char_data * tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    mag_points(MAX(1, level/2), ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(MAX(1, level/2), ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL);
    break;
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!AFF_FLAGGED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!AFF_FLAGGED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && AFF_FLAGGED(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, savetype);
  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  const char *to_char = NULL, *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "You gesture and the earth begins to shake all around you!";
    to_room ="$n gracefully gestures and the earth begins to shake violently!";
    break;
  case SPELL_NOVASTORM:
    to_char = "You gesture and blinding blasts of energy erupt all around you!";
    to_room ="$n gracefully gestures and blinding blasts of energy erupt everywhere!";
    break;
   break;

  }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);
  
  int hitcount = 0;
  int canhit = GET_SKILL(ch, spellnum) / 2 + 3;

  for (tch = world[ch->in_room].people; tch && hitcount < canhit; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     */

    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (!IS_NPC(ch) && !IS_NPC(tch))
      continue;
    if (IS_NPC(tch) && MOB_FLAGGED(tch, MOB_NOHIT)) 
        continue;


    if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
      continue;
    if (IS_NPC(ch) && IS_NPC(tch)) continue;

    /* Doesn't matter if they die here so we don't check. -gg 6/24/98 */
    hitcount++;
    mag_damage(level, ch, tch, spellnum, 1);
  }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in Circle 3.0; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 *
 * 10/15/97 (gg) - Implemented Animate Dead and Clone.
 */

/*
 * These use act(), don't put the \r\n.
 */
const char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!",
  "$n animates a corpse!",
  "$N appears from a cloud of thick blue smoke!",
  "$N appears from a cloud of thick green smoke!",
  "$N appears from a cloud of thick red smoke!",
  "$N disappears in a thick black cloud!"
  "As $n makes a strange magical gesture, you feel a strong breeze.",
  "As $n makes a strange magical gesture, you feel a searing heat.",
  "As $n makes a strange magical gesture, you feel a sudden chill.",
  "As $n makes a strange magical gesture, you feel the dust swirl.",
  "$n magically divides!",
  "$n animates a corpse!"
};

/*
 * Keep the \r\n because these use send_to_char.
 */
const char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh crap!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};

/* These mobiles do not exist. */
#define MOB_MONSUM_I		130
#define MOB_MONSUM_II		140
#define MOB_MONSUM_III		150
#define MOB_GATE_I		160
#define MOB_GATE_II		170
#define MOB_GATE_III		180

/* Defined mobiles. */
#define MOB_ELEMENTAL_BASE	20	/* Only one for now. */
#define MOB_CLONE		10
#define MOB_ZOMBIE		11
#define MOB_AERIALSERVANT	19


void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		      int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  int pfail = 0, msg = 0, fmsg = 0, num = 1, handle_corpse = FALSE, i;
  mob_vnum mob_num;

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_CLONE:
    msg = 10;
    fmsg = number(2, 6);	/* Random fail message. */
    mob_num = MOB_CLONE;
    pfail = 50;	/* 50% failure, should be based on something later. */
    break;

  case SPELL_ANIMATE_DEAD:
    if (obj == NULL || !IS_CORPSE(obj)) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    handle_corpse = TRUE;
    msg = 11;
    fmsg = number(2, 6);	/* Random fail message. */
    mob_num = MOB_ZOMBIE;
    pfail = 10;	/* 10% failure, should vary in the future. */
    break;

  default:
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }
  if (number(0, 101) < pfail) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    return;
  }
  for (i = 0; i < num; i++) {
    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
      send_to_char("You don't quite remember how to make that creature.\r\n", ch);
      return;
    }
    char_to_room(mob, ch->in_room);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
    if (spellnum == SPELL_CLONE) {	/* Don't mess up the proto with strcpy. */
      mob->player.name = str_dup(GET_NAME(ch));
      mob->player.short_descr = str_dup(GET_NAME(ch));
    }
    act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
    load_mtrigger(mob);
    add_follower(mob, ch);
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, int savetype)
{
  int hit = 0, move = 0;
  double hptomana = 0;
  int skill = 0;

  if (victim == NULL)
    return;

  if (GET_SKILL(ch, spellnum) != level)
      skill = level; //Object
  else
      skill = GET_SKILL(ch, spellnum); //Player

  if (GET_POS(victim) < POS_MORTALLYW) {
    send_to_char("You can't heal dead players (resurrection)\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(victim, MOB_NOHIT)) {
    send_to_char("You cannot heal this type of mob.\r\n", ch);
    return;
  }

  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    hit = dice(1, 8) + 1 + (level / 4);
    send_to_char("You feel better.\r\n", victim);
    break;
  case SPELL_CURE_CRITIC:
    hit = dice(3, 8) + 3 + (level / 4);
    send_to_char("You feel a lot better!\r\n", victim);
    break;
  case SPELL_HEAL:
   hptomana = 1.2;
   hit =  (hptomana + (.03 * skill)) * MIN(SINFO.mana_max, (SINFO.mana_min + SINFO.mana_change* skill));
   hit += dice(3,8);
   hit = baldam(ch, CLASS_CLERIC, hit);

   send_to_char("A warm feeling floods your body.\r\n", victim);
   break;

  case SPELL_MANA:
    GET_MANA(victim) = MIN(GET_MAX_MANA(victim), GET_MANA(victim) + skill*10);
    send_to_char("Your mind feels envigorated.\r\n", victim);
    break;

  case SPELL_GREATERHEAL:
   hit = 75;
   if (GET_CLASS(ch) == CLASS_CLERIC) {
    hit = 125 + dice(3, 8);
   }
   if (GET_CLASS(ch) == CLASS_MAGIC_USER) {
    hit = 100 + dice(3, 8);
   }
    send_to_char("An onrush of relief floods your body.\r\n", victim);
    break;
  }
  GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
  update_pos(victim);
  set_killer(victim, NULL, hit);
}

void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		        int spellnum, int type)
{
  int spell = 0;
  const char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_BLIND:
  case SPELL_HEAL:
  case SPELL_GREATERHEAL:
    spell = SPELL_BLINDNESS;
    to_vict = "Your vision returns!";
    to_room = "There's a momentary gleam in $n's eyes.";
    break;
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "You don't feel so unlucky.";
    break;
  default:
    log("SYSERR: unknown spellnum %d passed to mag_unaffects.", spellnum);
    return;
  }

  if (!affected_by_spell(victim, spell)) {
    if (spellnum != SPELL_HEAL && spellnum != SPELL_GREATERHEAL) {
      send_to_char(NOEFFECT, ch);
    }
    if (spellnum != SPELL_GREATERHEAL && spellnum != SPELL_HEAL) {
      send_to_char(NOEFFECT, ch);
    }
    return;
  }

  affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
		         int spellnum, int savetype)
{
  const char *to_char = NULL, *to_room = NULL;

  if (obj == NULL)
    return;



  switch (spellnum) {
    case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
	  (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
	to_char = "$p glows briefly.";
      }
      break;
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
	SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, struct char_data * ch, int spellnum)
{
  struct obj_data *tobj;
  obj_vnum z;

  if (ch == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1); - Hm, not used. */

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 10;
    break;
  default:
    send_to_char("Spell unimplemented, it would seem.\r\n", ch);
    return;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    log("SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
  load_otrigger(tobj);
}

bool resist(struct char_data *ch, struct char_data *victim, int primeclass, int spellnum)
// Decides whether the Victim Resists the magical spell
// True means it was resisted
{
  // Negative ldiff means Ch is a higher level; 
  int ldiff = GET_TOTAL_LEVEL(victim) - GET_TOTAL_LEVEL(ch);
  int spell_level = 0;

  if (IS_NPC(victim)) return FALSE;

  switch (primeclass)
  {
    case CLASS_MAGE: ldiff += 2 * (GET_MAGE_LEVEL(victim) - GET_MAGE_LEVEL(ch)); break;
    case CLASS_CLERIC: ldiff += 2 * (GET_CLERIC_LEVEL(victim) - GET_CLERIC_LEVEL(ch)); break;
    case CLASS_THIEF: ldiff += 2 * (GET_THIEF_LEVEL(victim) - GET_THIEF_LEVEL(ch)); break;
    case CLASS_WARRIOR: ldiff += 2 * (GET_WARRIOR_LEVEL(victim) - GET_WARRIOR_LEVEL(ch)); break;
  }
  spell_level = GET_SKILL(ch, spellnum);

  if (ldiff - spell_level + number(-30, 30) >= 0)
  {
    return TRUE;
  }

  return FALSE;
}


int baldam(struct char_data *ch, int primeclass, int maxpower)
// This balances the damage done by all spells/skills Including heal
//
{
  int realclass = (int)GET_CLASS(ch);

  if (!ch) return maxpower;

  switch (primeclass)
  {
    case CLASS_MAGE:
      switch(realclass)
      {
         case CLASS_MAGE: return maxpower        ; break;
         case CLASS_CLERIC: return maxpower  * .8; break;
         case CLASS_THIEF: return maxpower   * .7; break;
         case CLASS_WARRIOR: return maxpower * .6; break;
      }
      break;
    case CLASS_CLERIC:
      switch(realclass)
      { 
         case CLASS_MAGE: return maxpower    * .8; break;
         case CLASS_CLERIC: return maxpower      ; break;
         case CLASS_THIEF: return maxpower   * .6; break;
         case CLASS_WARRIOR: return maxpower * .7; break;
      }
      break;
    case CLASS_THIEF:
      switch(realclass)
      {
         case CLASS_MAGE: return maxpower    * .7; break;
         case CLASS_CLERIC: return maxpower  * .6; break;
         case CLASS_THIEF: return maxpower       ; break;
         case CLASS_WARRIOR: return maxpower * .8; break;
      }
      break;
    case CLASS_WARRIOR:
      switch(realclass)
      {
         case CLASS_MAGE: return maxpower   * .6; break;
         case CLASS_CLERIC: return maxpower * .7; break;
         case CLASS_THIEF: return maxpower  * .8; break;
         case CLASS_WARRIOR: return maxpower    ; break;
      }
      break;
    }
  return maxpower;
}
