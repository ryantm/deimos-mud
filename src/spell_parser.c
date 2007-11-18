/*
************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
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
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "dg_scripts.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#ifndef SINFO
#define SINFO spell_info[spellnum]
#endif

extern struct room_data *world;

/* local functions */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch, struct obj_data * tobj);
void spello(int spl, const char *name, int max_mana, int min_mana, int mana_change, int minpos, int targets, int violent, int routines);
int mag_manacost(struct char_data * ch, int spellnum);
ACMD(do_cast);
void unused_spell(int spl);
void mag_assign_spells(void);

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, 200 should provide
 * ample slots for skills.
 */

struct syllable {
  const char *org;
  const char *news;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};

const char *unused_spellname = "!UNUSED!"; /* So we can get &unused_spellname */

int mag_manacost(struct char_data * ch, int spellnum)
{
int cost = 0;
	
if (GET_LEVEL(ch) < LVL_IMMUNE) //2% per level of mastery off
{
 cost =MIN(SINFO.mana_min + (SINFO.mana_change * GET_SKILL(ch,spellnum)), SINFO.mana_max);
 cost *= (100. - GET_SKILL(ch, SKILL_SPELL_MASTERY)*2.)/100.;
}
if(IS_NPC(ch))
{
 cost =(int)(1-GET_SKILL(ch, SKILL_SPELL_MASTERY)/50) * MIN(SINFO.mana_min + (SINFO.mana_change * GET_SKILL(ch,spellnum)), SINFO.mana_max);
 cost *= (int) (100. - GET_SKILL(ch, SKILL_SPELL_MASTERY)*2.)/100.;
}
/* else immortal casting cost is 0 */
if (GET_SKILL(ch, SKILL_HALF_MANA)) cost = (int)(cost / 2.);

return cost;
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
	            struct obj_data * tobj)
{
  char lbuf[256];
  char buf4[MAX_STRING_LENGTH];
  const char *format;
  const char *toself;
  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, skill_name(spellnum));

  while (lbuf[ofs]) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].news);
	ofs += strlen(syls[j].org);
        break;
      }
    }
    /* i.e., we didn't find a match in syls[] */
    if (!*syls[j].org) {
      log("No entry in syllable table for substring of '%s'", lbuf);
      ofs++;
    }
  }

  if (tch != NULL && IN_ROOM(tch) == IN_ROOM(ch)) {
    if (tch == ch)
    {
      format = "&Y$n&Y closes $s eyes and utters the words, '%s'.&n";
      toself = "&YYou close your eyes and utter the words, '%s'.&n";
    }
    else
    {
      format = "&Y$n&Y stares at $N&Y and utters the words, '%s'.&n";
      toself = "&YYou stare at $N&Y and utter the words, '%s'.&n";
    }
  } else if (tobj != NULL &&
	     ((IN_ROOM(tobj) == IN_ROOM(ch)) || (tobj->carried_by == ch)))
    {
     format = "&Y$n stares at $p&Y and utters the words, '%s'.&n";
     toself = "&YYou stare at $p&Y and utter the words, '%s'.&n";
    }
  else
  {
    format = "&Y$n&Y utters the words, '%s'.&n";
    toself = "&YYou utter the words, '%s'.&n";
  }

  sprintf(buf1, format, skill_name(spellnum));
  sprintf(buf4, toself, skill_name(spellnum));
  sprintf(buf2, format, buf);

  if (!PRF_FLAGGED(ch, PRF_NOREPEAT))
    act(buf4,0, ch, tobj, tch, TO_CHAR);

  for (i = world[IN_ROOM(ch)].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_SKILL(i, spellnum))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && IN_ROOM(tch) == IN_ROOM(ch)) {
    sprintf(buf1, "&Y$n&Y stares at you and utters the words, '%s'.&n",
	    GET_CLASS(ch) == GET_CLASS(tch) ? skill_name(spellnum) : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

/*
 * This function should be used anytime you are not 100% sure that you have
 * a valid spell/skill number.  A typical for() loop would not need to use
 * this because you can guarantee > 0 and <= TOP_SPELL_DEFINE.
 */
const char *skill_name(int num)
{
  if (num > 0 && num <= TOP_SPELL_DEFINE)
    return (spell_info[num].name);
  else if (num == -1)
    return ("UNUSED");
  else
    return ("UNDEFINED");
}

	 
int find_skill_num(char *name)
{
  int index, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  for (index = 1; index <= TOP_SPELL_DEFINE; index++) {
    if (is_abbrev(name, spell_info[index].name))
      return (index);

    ok = TRUE;
    /* It won't be changed, but other uses of this function elsewhere may. */
    temp = any_one_arg((char *)spell_info[index].name, first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = FALSE;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return (index);
  }

  return (-1);
}



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return (0);
  if (caster->nr != real_mobile(DG_CASTER_PROXY)) {
    if (!IS_NPC(caster)) {
  if ((ROOM_FLAGGED(IN_ROOM(caster), ROOM_NOMAGIC) ||
     ROOM_FLAGGED(IN_ROOM(caster), ROOM_NOMAGIC))
     && (GET_LEVEL(caster) < LVL_IMMUNE)) {
      send_to_char("&MYour magic fizzles out and dies.&n\r\n", caster);
      act("&M$n's magic fizzles out and dies.&n", FALSE, caster, 0, 0, TO_ROOM);
      return (0);
      }
    }
    if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_PEACEFUL) &&
        (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
      send_to_char("&WA flash of white light fills the room, dispelling your "
		   "violent magic!&n\r\n", caster);
      act("&WWhite light from no particular source suddenly fills the room, "
	  "then vanishes.&n", FALSE, caster, 0, 0, TO_ROOM);
      return (0);
    }
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }


  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    if (mag_damage(level, caster, cvict, spellnum, savetype) == -1)
      return (-1);	/* Successful and target died, don't cast again. */

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
    case SPELL_CREATE_WATER:	MANUAL_SPELL(spell_create_water); break;
    case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_MAGIC_ERASER:    MANUAL_SPELL(spell_magic_eraser); break;
    case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
    case SPELL_SUMMON:		MANUAL_SPELL(spell_summon); break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
    case SPELL_ARENBURG_RECALL:	MANUAL_SPELL(spell_arenburg_recall); break;
    case SPELL_TELEPORT:	MANUAL_SPELL(spell_teleport); break;
    case SPELL_RESURRECTION:    MANUAL_SPELL(spell_resurrection); break;
    case SPELL_REFRESH:         MANUAL_SPELL(spell_refresh);break;
    case SPELL_DISPEL_MAGIC:    MANUAL_SPELL(spell_dispel_magic);break;
    case SPELL_FIND_TARGET:     MANUAL_SPELL(spell_find_target);break;
    case SPELL_GATE:            MANUAL_SPELL(spell_gate);break;
//    caseSPELL_DIMPHASE:       MANUAL_SPELL(spell_dimension_phase);break;
    case SPELL_SUSTAINANCE:     MANUAL_SPELL(spell_sustainance); break;
    case SPELL_TURKEY:    	MANUAL_SPELL(spell_golden_turkey); break;

    }

  return (1);
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		          char *argument)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("&GYou tap $p three times on the ground.&n", FALSE, ch, obj, 0, TO_CHAR);
    act("&G$n taps $p three times on the ground.&n", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char("&RIt seems powerless.&n\r\n", ch);
      act("&RNothing seems to happen.&n", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      /* Level to cast spell at. */
      k = GET_OBJ_VAL(obj, 0) ? GET_OBJ_VAL(obj, 0) : DEFAULT_STAFF_LVL;

      /*
       * Problem : Area/mass spells on staves can cause crashes.
       * Solution: Remove the special nature of area/mass spells on staves.
       * Problem : People like that behavior.
       * Solution: We special case the area/mass spells here.
       */

      if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, 3), MAG_MASSES | MAG_AREAS)) {
        for (i = 0, tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
	  i++;
	while (i-- > 0)
	  call_magic(ch, NULL, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
      } else {
	for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
	  next_tch = tch->next_in_room;
	  if (ch != tch)
	    call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
	}
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("&GYou point $p at yourself.&n", FALSE, ch, obj, 0, TO_CHAR);
	act("&G$n points $p at $mself.&n", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("&GYou point $p at $N.&n", FALSE, ch, obj, tch, TO_CHAR);
	act("&G$n points $p at $N.&n", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("&GYou point $p at $P.&n", FALSE, ch, obj, tobj, TO_CHAR);
      act("&G$n points $p at $P.&n", TRUE, ch, obj, tobj, TO_ROOM);
    } else if (IS_SET(spell_info[GET_OBJ_VAL(obj, 3)].routines, MAG_AREAS | MAG_MASSES)) {
      /* Wands with area spells don't need to be pointed. */
      act("&GYou point $p outward.&n", FALSE, ch, obj, NULL, TO_CHAR);
      act("&G$n points $p outward.&n", TRUE, ch, obj, NULL, TO_ROOM);
    } else {
      act("&RAt what should $p be pointed?&n", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char("&RIt seems powerless.&n\r\n", ch);
      act("&RNothing seems to happen.&n", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("&RThere is nothing to here to affect with $p.&n", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("&GYou recite $p which dissolves.&n", TRUE, ch, obj, 0, TO_CHAR);
    act("&G$n recites $p.&n", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    act("&GYou quaff $p.&n", FALSE, ch, obj, NULL, TO_CHAR);
    act("&G$n quaffs $p.&n", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type %d in mag_objectmagic.",
	GET_OBJ_TYPE(obj));
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
	           struct obj_data * tobj, int spellnum)
{
  int skill = 0, mana = 0;
  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    log("SYSERR: cast_spell trying to call spellnum %d/%d.\n", spellnum,
	TOP_SPELL_DEFINE);
    return (0);
  }
    
  if (GET_POS(ch) < SINFO.min_position && (!IS_NPC(ch))) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("&RYou dream about great magical powers.&n\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("&RYou cannot concentrate while resting.&n\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("&RYou can't do this sitting!&n\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("&RImpossible!  You can't concentrate enough!&n\r\n", ch);
      break;
    default:
      send_to_char("&RYou can't do much of anything like this!&n\r\n", ch);
      break;
    }
    return (0);
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("&RYou are afraid you might hurt your master!&n\r\n", ch);
    return (0);
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char("&RYou can only cast this spell upon yourself!&n\r\n", ch);
    return (0);
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("&RYou cannot cast this spell upon yourself!&n\r\n", ch);
    return (0);
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char("&RYou can't cast this spell if you're not in a group!&n\r\n",ch);
    return (0);
  }
  if (tch && !IS_NPC(tch) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(tch) >= LVL_IMMORT) 
     {tch = ch;}

  ///Mobs use mana too!////////////
  if (IS_NPC(ch))
  {
   mana = mag_manacost(ch, spellnum);

   if (MIN(70 + GET_LEVEL(ch) / 20., 97) < number(0,100))
   {
      send_to_char("&RYou lost your concentration!&n\r\n", ch);
      act("&G$n lost $s concentration!&n", FALSE,ch, 0, 0, TO_ROOM);
      return (0);
   }
   if ((mana > 0) && (GET_MANA(ch) < mana))
   {
    send_to_char("&RYou haven't the energy to cast that spell!&n\r\n", ch);
    act("&G$n trys to cast a spell but doesn't have enough energy.&n", FALSE,ch, 0, 0, TO_ROOM);
    send_to_char(buf, ch);
    return (0);
   }
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
  }
  ///////////////////////////////// 

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);

  say_spell(ch, spellnum, tch, tobj);

  // Caclculate skill of casting
  if (!IS_NPC(ch))
    skill = GET_SKILL(ch, spellnum);
  else
  {// Calculate a mob's casting power
    skill = MIN(GET_LEVEL(ch) / 15., spell_info[spellnum].max_power[(int)GET_CLASS(ch)]);
  }
  return (call_magic(ch, tch, tobj, spellnum, GET_SKILL(ch, spellnum), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0, perc = 0, curperc = 0;
  int skill, mp;

  if (IS_NPC(ch))
    return;

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }

  if (PLR_FLAGGED2(ch, PLR2_REVIEW)) {
    send_to_char("You cannot cast spells as a reviewer.\r\n", ch);
    return;
  }

  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if (t != NULL )
    ch->player_specials->tmp_text = strdup(t);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char("You are unfamiliar with that spell.\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BERSERK))
  {
    send_to_char("Blood clouds your mind! You cannot think to cast any spell. \r\n", ch);
    return;
  }

  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
     strip_hyphan(t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_vis(ch, t, FIND_CHAR_ROOM)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t, FIND_CHAR_WORLD)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[IN_ROOM(ch)].contents)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)) != NULL)
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD | TAR_OBJ_EQUIP) ? "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  if (tch && !IS_NPC(tch) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(tch) >= LVL_IMMORT) 
  {
     sprintf(buf, "Your attempt to cast a spell on %s, mirrors back at you.\r\n", GET_NAME(tch));
     send_to_char(buf, ch);
     sprintf(buf, "%s attempts to cast a spell on you, PFFBT- what a stupid mortal.\r\n", GET_NAME(ch));
     send_to_char(buf, tch);
     tch = ch;
  }

  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_IMMORT))) {
    send_to_char("&RYou haven't the energy to cast that spell!&n\r\n", ch);
    sprintf(buf, "&R(You need %d mana points.)&n\r\n", mana);
    send_to_char(buf, ch);
    return;
  }

  /* You throws the dice and you takes your chances.. 101% is total failure */
  perc = number(0,1001);
  if (GET_CLASS(ch) == CLASS_MAGIC_USER)  curperc = 700;
  if (GET_CLASS(ch) == CLASS_CLERIC)      curperc = 650;
  if (GET_CLASS(ch) == CLASS_THIEF)       curperc = 550;
  if (GET_CLASS(ch) == CLASS_WARRIOR)     curperc = 500;
  skill = GET_SKILL(ch, spellnum);  
  mp = (int)SINFO.max_power;

  curperc += GET_SKILL(ch, SKILL_SPELL_MASTERY)*10;
  curperc += 3*(60 + 35 * (skill / mp));
  curperc += 2*(GET_INT(ch) + GET_WIS(ch));

  if (perc > curperc && GET_LEVEL(ch) < LVL_IMMUNE) 
  {

    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);

    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana / 2)));
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
     if (AFF_FLAGGED(ch, AFF_SHADE)) 
       REMOVE_BIT(AFF_FLAGS(ch), AFF_SHADE);
     if (GET_LEVEL(ch) < LVL_IMMUNE) 
        WAIT_STATE(ch, PULSE_VIOLENCE);

      if (mana > 0)
	GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));

      if (spellnum == SPELL_SLOW)
      {    
      if (!CAN_ASSASSIN(ch, tch))  
      {
        send_to_char("You != Assassin.\r\n", ch);
        return;
      }
       if (!IS_NPC(tch))
       {
         WAIT_STATE(tch, PULSE_VIOLENCE * (int)(GET_SKILL(ch,spellnum)/2));
       }
       else
       {
         GET_MOB_WAIT(tch) = GET_MOB_WAIT(tch) + PULSE_VIOLENCE *(int)(GET_SKILL(ch, spellnum)/2);
       }

      }
    }
  }
}


void spell_levelt(int spell, int chclass, int levelthief, int multi, int power)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (levelthief < 1 || levelthief > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		levelthief, LVL_IMPL);
    bad = 1;
  }

  if (!bad) {
    spell_info[spell].min_level[chclass] = levelthief;
    spell_info[spell].multi = multi;
    spell_info[spell].max_power[chclass] = power;
  }
}

void spell_levelw(int spell, int chclass, int levelwarrior, int multi, int
power)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (levelwarrior < 1 || levelwarrior > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		levelwarrior, LVL_IMPL);
    bad = 1;
  }

  if (!bad) {   
    spell_info[spell].min_level[chclass] = levelwarrior;
    spell_info[spell].multi = multi;
    spell_info[spell].max_power[chclass] = power;
  }
}

void spell_levelm(int spell, int chclass, int levelmage, int multi, int power)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (levelmage < 1 || levelmage > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		levelmage, LVL_IMPL);
    bad = 1;
  }

  if (!bad) {
    spell_info[spell].min_level[chclass] = levelmage;
    spell_info[spell].max_power[chclass] = power;
    spell_info[spell].multi = multi;
  }
}

void spell_levelc(int spell, int chclass, int levelcleric, int multi, int power)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

   if (levelcleric < 1 || levelcleric > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		levelcleric, LVL_IMPL);
    bad = 1;
  }

  if (!bad) {
    spell_info[spell].min_level[chclass] = levelcleric;
    spell_info[spell].max_power[chclass] = power;
    spell_info[spell].multi = multi;
  }
}

/* Assign the spells on boot up */
void spello(int spl, const char *name, int max_mana, int min_mana,
	int mana_change, int minpos, int targets, int violent, int routines)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMMORT;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].name = name;
}


void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
  {
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
    spell_info[spl].max_power[i] = 0;
  }
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
  spell_info[spl].name = unused_spellname;
}

#define skillo(skill, name) spello(skill, name, 0, 0, 0, 0, 0, 0, 0);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * spellname: The name of the spell.
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below. */
  for (i = 0; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above. */

  spello(SPELL_ANIMATE_DEAD, "animate dead", 35, 10, 3, POS_STANDING,
	TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

  spello(SPELL_ARMOR, "armor", 60, 10, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLESS, "bless", 60, 10, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_AUROROUS, "aurorous", 100, 100, 0, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLINDNESS, "blindness", 35, 5, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_BURNING_HANDS, "burning hands", 50, 35, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, "call lightning", 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CHARM, "charm person", 75, 25, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CHILL_TOUCH, "chill touch", 50, 35, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CLONE, "clone", 80, 65, 5, POS_STANDING,
	TAR_SELF_ONLY, FALSE, MAG_SUMMONS);

  spello(SPELL_COLOR_SPRAY, "color spray", 90, 75, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CONTROL_WEATHER, "control weather", 75, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_SUSTAINANCE, "sustainance", 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_CREATE_FOOD, "create food", 30, 5, 4, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, "create water", 30, 5, 4, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_BLIND, "cure blind", 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_CURE_CRITIC, "cure critic", 30, 30, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_LIGHT, "cure light", 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURSE, "curse", 80, 50, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DISINTEGRATE, "disintegrate", 125, 100, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DETECT_ALIGN, "detect alignment", 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_INVIS, "detect invisibility", 20, 20, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, "detect magic", 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, "detect poison", 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DISPEL_EVIL, "dispel evil", 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DISPEL_GOOD, "dispel good", 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, "earthquake", 40, 25, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ENCHANT_WEAPON, "enchant weapon", 1000,100,100,POS_STANDING,
	TAR_OBJ_INV, FALSE, MAG_MANUAL);

  spello(SPELL_MAGIC_ERASER, "magic eraser", 100,100,0,POS_STANDING,
	TAR_OBJ_INV, FALSE, MAG_MANUAL);

  spello(SPELL_ENERGY_DRAIN, "energy drain", 40, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(SPELL_FLOAT, "float", 40, 20, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_LIGHTNING_BOLT, "lightning bolt", 40, 20, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FIREBALL, "fireball", 100, 40, 6, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ICEBOLT, "icebolt", 200, 100, 10, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GATE, "gate", 100, 50, 10, POS_STANDING,
         TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_HEAL, "heal", 100, 50, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS);

  spello(SPELL_MANA, "mana", 100, 20, 8, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS);

  spello(SPELL_GREATERHEAL, "greater heal", 110, 100, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS);

  spello(SPELL_GROUP_ARMOR, "group armor", 50, 30, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_HEAL, "group heal", 120, 50, 7, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_HARM, "harm", 75, 50, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HELLFIRE, "hellfire", 80, 70, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_INFRAVISION, "infravision", 25, 10, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INTERPOSING_HANDS, "interposing hand", 50, 20, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_INVISIBLE, "invisibility", 35, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_LOCATE_OBJECT, "locate object", 25, 20, 1, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_FIND_TARGET, "find target", 25, 20, 1, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_MANUAL);   

  spello(SPELL_MAGIC_MISSILE, "magic missile", 40, 20, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MANASHIELD, "manashield", 50, 40, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_NOVASTORM, "novastorm", 200, 50, 15, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_POISON, "poison", 50, 20, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE,
	MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_PROT_FROM_EVIL, "protection from evil", 40, 10, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_REMOVE_CURSE, "remove curse", 45, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE,
	MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_REMOVE_POISON, "remove poison", 40, 8, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SANCTUARY, "sanctuary", 100, 100, 0, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SENSE_LIFE, "sense life", 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SHOCKING_GRASP, "shocking grasp", 60, 45, 1,POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SLEEP, "sleep", 40, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_SLOW, "slow", 50, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_GASEOUS, "gaseous form", 1000,1000, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_STRENGTH, "strength", 40, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SUMMON, "summon", 75, 50, 3, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_TELEPORT, "teleport", 75, 50, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_WATERWALK, "waterwalk", 40, 20, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_WEB, "web", 50, 40, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_BERSERK, "berserk", 100, 90, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SHROUD, "magic shroud", 50, 25, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
 
  spello(SPELL_DIMPHASE, "dimension phase", 50, 25, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_WORD_OF_RECALL, "word of recall", 20, 20, 0, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_ARENBURG_RECALL, "Arenburgian recall", 20, 20, 0, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);


  /* NON-castable spells should appear below here. */

  spello(SPELL_IDENTIFY, "identify", 0, 0, 0, 0,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_RESURRECTION, "resurrection", 200, 150, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_REFRESH, "refresh", 15, 13, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);
  
  spello(SPELL_DISPEL_MAGIC, "dispel magic", 30, 20, 3, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  /*
   * These spells are currently not used, not implemented, and not castable.
   * Values for the 'breath' spells are filled in assuming a dragon's breath.
   */

  spello(SPELL_TURKEY, "thanksgiving spirit", 0, 0, 0, POS_STANDING,
	TAR_IGNORE, TRUE, MAG_MANUAL);

  spello(SPELL_FIRE_BREATH, "fire breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0);

  spello(SPELL_GAS_BREATH, "gas breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0);

  spello(SPELL_FROST_BREATH, "frost breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0);

  spello(SPELL_ACID_BREATH, "acid breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0);

  spello(SPELL_LIGHTNING_BREATH, "lightning breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0);

  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_BACKSTAB, "backstab");
  skillo(SKILL_BANDAGE, "bandage");
  skillo(SKILL_BASH, "bash");
  skillo(SKILL_WEAPON_BLUDGEONS,     "bludgeons");
  skillo(SKILL_WEAPON_DAGGERS,       "daggers");
  skillo(SKILL_DISARM, "disarm");
  skillo(SKILL_WEAPON_EXOTICS,       "exotics");
  skillo(SKILL_HIDE, "hide");
  skillo(SKILL_KICK, "kick");
  skillo(SKILL_PICK_LOCK, "pick lock");
  skillo(SKILL_PUNCH, "punch");
  skillo(SKILL_RESCUE, "rescue");
  skillo(SKILL_SHOOT, "shooting");
  skillo(SKILL_SNEAK, "sneak");
  skillo(SKILL_STAB,  "stab");
  skillo(SKILL_STEAL, "steal");
  skillo(SKILL_WEAPON_SWORDS,        "swords");
  skillo(SKILL_WEAPON_TALONOUS_ARMS, "talonous arms");
  skillo(SKILL_MULTI_ATTACK, "multiple attacks");
  skillo(SKILL_TRACK, "track");
  skillo(SKILL_WEAPON_WHIPS,         "whips");
  skillo(SKILL_WHIRLWIND, "whirlwind");
  skillo(SKILL_TETHER, "tether");
  skillo(SKILL_TRIP, "trip");
  skillo(SKILL_SHADE, "shade");
  skillo(SKILL_HP_REGEN, "HP regen");
  skillo(SKILL_MP_REGEN, "MP regen");
  skillo(SKILL_MV_REGEN, "MV regen");
  skillo(SKILL_DOUBLE_STAB, "double stab");
  skillo(SKILL_SPELL_MASTERY, "spell mastery");
  skillo(SKILL_HALF_MANA, "half mana");
  skillo(SKILL_SEE_INV, "see inventory");

}

