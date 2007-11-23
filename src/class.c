/*
************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */



#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"

extern int siteok_everyone;
void  char_to_room(struct char_data *ch, room_rnum room); 
extern void send_to_char(const char *messg, struct char_data *ch);

/* local functions */
int parse_class(char arg);
long find_class_bitvector(char arg);
byte saving_throws(int class_num, int type, int level);
int thaco(int class_num, int level);
void roll_real_abils(struct char_data * ch);
void do_start(struct char_data * ch);
double backstab_mult(struct char_data *ch, int level);
int invalid_class(struct char_data *ch, struct obj_data *obj);
int level_exp(int level);
int level_gold(int level);
int horse_exp(int horselevel);
const char *title_male(int chclass, int level);
const char *title_female(int chclass, int level);

/* Names first */

const char *class_abbrevs[] = {
  "Mu",
  "Cl",
  "Th",
  "Wa",
  "\n"
};


const char *pc_class_types[] = {
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"Select a class:\r\n"
"  [C]leric\r\n"
"  [T]hief\r\n"
"  [W]arrior\r\n"
"  [M]age\r\n";



/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'm': return CLASS_MAGIC_USER;
  case 'c': return CLASS_CLERIC;
  case 'w': return CLASS_WARRIOR;
  case 't': return CLASS_THIEF;
  default:  return CLASS_UNDEFINED;
  }
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'm': return (1 << CLASS_MAGIC_USER);
    case 'c': return (1 << CLASS_CLERIC);
    case 't': return (1 << CLASS_THIEF);
    case 'w': return (1 << CLASS_WARRIOR);
    default:  return 0;
  }
}


/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1
//CHANGE THIS!!!! if you change levels

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	*/

int prac_params[4][NUM_CLASSES] = {
  /* MAG	CLE	THE	WAR */
  {1,		1,	1,	1},		/* learned level */
  {1,		1,	1,	1},		/* max per prac */
  {1,		1,	1,	1,},		/* min per pac */
  {SPELL,	SPELL,	SKILL,	SKILL}		/* prac name */
};


/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 *
 * Don't forget to visit spec_assign.c if you create any new mobiles that
 * should be a guild master or guard so they can act appropriately. If you
 * "recycle" the existing mobs that are used in other guilds for your new
 * guild, then you don't have to change that file, only here.
 */ 
struct new_guild_info guilds[] = {
   {258, CLASS_WARRIOR},
   {260, CLASS_CLERIC},
   {276, CLASS_MAGIC_USER},
   {298, CLASS_THIEF},
   {0, CLASS_UNUSED},
   {-1, -1}
 };

int guild_info[][3] = {
/* Phobos */
  {CLASS_MAGIC_USER,	213,	SCMD_SOUTH},
  {CLASS_CLERIC,	262,	SCMD_WEST},
  {CLASS_THIEF,		281,	SCMD_NORTH},
  {CLASS_WARRIOR,	241,	SCMD_SOUTH},

/* Brass Dragon */
  {-999 /* all */ ,	5065,	SCMD_WEST},

/* this must go last -- add new guards above! */
  {-1, -1, -1}
};

char saving_throws(int class_num, int type, int level)
{
  int top_level, high_val, low_val;

  switch(class_num)
  {
  case CLASS_MAGIC_USER:
    switch(type)
    {
    case SAVING_PARA:   high_val = 71; low_val = 29; break;
    case SAVING_ROD:    high_val = 54; low_val = 8; break;
    case SAVING_PETRI:  high_val = 65; low_val = 14; break;
    case SAVING_BREATH: high_val = 75; low_val = 24; break;
    case SAVING_SPELL:  high_val = 60; low_val = 9; break;
    default: log("SYSERR: Invalid saving throw type."); return 99;
    }
    break;
  case CLASS_CLERIC:
    switch(type)
    {
    case SAVING_PARA:   high_val = 58; low_val = 5; break;
    case SAVING_ROD:    high_val = 71; low_val = 27; break;
    case SAVING_PETRI:  high_val = 65; low_val = 22; break;
    case SAVING_BREATH: high_val = 80; low_val = 37; break;
    case SAVING_SPELL:  high_val = 75; low_val = 32; break;
    default: log("SYSERR: Invalid saving throw type."); return 99;
    }
    break;
  case CLASS_THIEF:
    switch(type)
    {
    case SAVING_PARA:   high_val = 65; low_val = 36; break;
    case SAVING_ROD:    high_val = 70; low_val = 13; break;
    case SAVING_PETRI:  high_val = 60; low_val = 31; break;
    case SAVING_BREATH: high_val = 80; low_val = 51; break;
    case SAVING_SPELL:  high_val = 75; low_val = 17; break;
    default: log("SYSERR: Invalid saving throw type."); return 99;
    }
    break;
  case CLASS_WARRIOR:
    switch(type)
    {
    case SAVING_PARA:   high_val = 70; low_val = 8; break;
    case SAVING_ROD:    high_val = 80; low_val = 16; break;
    case SAVING_PETRI:  high_val = 75; low_val = 13; break;
    case SAVING_BREATH: high_val = 85; low_val = 13; break;
    case SAVING_SPELL:  high_val = 85; low_val = 21; break;
    default: log("SYSERR: Invalid saving throw type."); return 99;
    }
    break;
  default:
    log("SYSERR: Invalid class passed to saving throw.");
    return 99;
  }
  top_level = 100;
  if(level > top_level)
    return 0;
  else
    return (int)((-(high_val - low_val) / ((float)(top_level-1))) * (level - 1) + high_val);
}

/* THAC0 for classes and levels.  (To Hit Armor Class 0) */
int thaco(int class_num, int level)
{
  int t;
  t = 100; /* Default THAC0 */
  
  if (level > 0) {
    switch (class_num) {
    case CLASS_MAGIC_USER: t = 20 - ((level - 1) / 3);       break;
    case CLASS_CLERIC:     t = 20 - (((level - 1) / 3) * 2); break;
    case CLASS_THIEF:      t = 20 - ((level - 1) / 2);       break;
    case CLASS_WARRIOR:    t = 20 - (level - 1);             break;
	default:
      log("SYSERR: Unknown class in thac0 chart.");
     }
    }
  if (t < 1) /* Nobody can have a THAC0 better than 1! */
    t = 1;
    
  return (t);
    }
  
/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
void roll_real_abils(struct char_data * ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = number(1, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 6; k++)
      if (table[k] < temp) {
	temp ^= table[k];
	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->real_abils.intel = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.str = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_CLERIC:
    ch->real_abils.wis = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.dex = table[3];
    ch->real_abils.con = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_THIEF:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_WARRIOR:
    ch->real_abils.str = table[0];
    ch->real_abils.dex = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.wis = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    break;
  }
  ch->aff_abils = ch->real_abils;
}


/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
 
 GET_LEVEL(ch) = 1;
 if ((GET_CLASS(ch)) == 0) {
	 SET_MAGE_LEVEL(ch,1);
  }
 if ((GET_CLASS(ch)) == 1) {
	 SET_CLERIC_LEVEL(ch,1); 
  }
 if ((GET_CLASS(ch)) == 2) {
	 SET_THIEF_LEVEL(ch,1);
  } 
 if ((GET_CLASS(ch)) == 3) {
	 SET_WARRIOR_LEVEL(ch,1); 
  }

  GET_EXP(ch) = 1;
  GET_PRETITLE(ch) = "";   

  set_title(ch, NULL);

  ch->points.max_hit = 10;

  //Starting Skills
  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    break;

  case CLASS_CLERIC:
	break;

  case CLASS_THIEF:
	break;

  case CLASS_WARRIOR:
	break;
  }

  advance_level(ch);
  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;
  GET_HORSEHAPPY(ch) = GET_MAXHAPPY(ch);

  ch->char_specials.position = POS_STANDING;
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  send_to_char("All preferences automatically turned on. To see which are on type toggle.\r\n", ch);
  command_interpreter(ch, "autoall");

  if (siteok_everyone)
    SET_BIT(PLR_FLAGS(ch), PLR_SITEOK);

}



/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(struct char_data * ch)
{
  int add_hp, add_mana = 0, add_move = 0, i;
  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    add_hp += number(3, 8);
    add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    add_mana = MIN(add_mana, 10);
    add_move = number(0, 2);
    break;

  case CLASS_CLERIC:
    add_hp += number(5, 10);
    add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    add_mana = MIN(add_mana, 10);
    add_move = number(0, 2);
    break;

  case CLASS_THIEF:
    add_hp += number(7, 13);
    add_mana = 0;
    add_move = number(1, 3);
    break;

  case CLASS_WARRIOR:
    add_hp += number(10, 15);
    add_mana = 0;
    add_move = number(1, 3);
    break;
  }

  ch->points.max_hit += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;

    GET_PRACTICES(ch) += 1;
    send_to_char("You gain one more practice.\r\n", ch);

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }

  save_char(ch, NOWHERE);
}


/*
 * This simply calculates the backstab multiplier based on a character's
 * level.  This used to be an array, but was changed to be a function so
 * that it would be easier to add more levels to your MUD.  This doesn't
 * really create a big performance hit because it's not used very often.
 */
double backstab_mult(struct char_data *ch, int level)
{
  int i = 0;

  switch(GET_SKILL(ch, SKILL_BACKSTAB))
  {
  case 1: i = 1; break;
  case 2: i = 1; break;
  case 3: i = 1.25; break;
  case 4: i = 1.5; break;
  case 5: i = 1.75; break;
  case 6: i = 2; break;
  case 7: i = 2.25; break;
  case 8: i = 2.5; break;
  case 9: i = 3; break;
  case 10: i = 4; break;
  }
  
  return i;
}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(struct char_data *ch, struct obj_data *obj) {
  if (GET_LEVEL(ch) >= LVL_IMMUNE)
	 return 0;
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)))
	return 1;
  else
	return 0;
}




/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void init_spell_levels(void)
{
  int mp = 10, hp = 5, np = 1;
  /* MAGES */
  //Attack
  spell_levelm(SPELL_LIGHTNING_BOLT, CLASS_MAGIC_USER, 1, 0, mp);
  spell_levelm(SPELL_FIREBALL, CLASS_MAGIC_USER, 15, 0, mp);
  spell_levelm(SPELL_ICEBOLT, CLASS_MAGIC_USER, 60, 0, mp);
  spell_levelm(SPELL_NOVASTORM, CLASS_MAGIC_USER, 60, 1, mp);
  spell_levelm(SPELL_EARTHQUAKE, CLASS_MAGIC_USER, 30, 1, mp);

  //Negative
  spell_levelm(SPELL_BLINDNESS, CLASS_MAGIC_USER, 1, 0,mp);
  spell_levelm(SPELL_POISON, CLASS_MAGIC_USER, 1, 0,mp);
  spell_levelm(SPELL_SLEEP, CLASS_MAGIC_USER, 15, 0,mp);
  spell_levelm(SPELL_SLOW, CLASS_MAGIC_USER, 15, 0,mp);
  spell_levelm(SPELL_WEB, CLASS_MAGIC_USER, 15, 0,mp);
  spell_levelm(SPELL_DISPEL_MAGIC, CLASS_MAGIC_USER, 30, 0,mp);

  
  //Positive
  spell_levelm(SPELL_DETECT_INVIS, CLASS_MAGIC_USER, 1, 0, np);
  spell_levelm(SPELL_SHROUD, CLASS_MAGIC_USER, 45, 1,mp);
  spell_levelm(SKILL_MP_REGEN, CLASS_MAGIC_USER, 1, 0, mp);
  spell_levelm(SKILL_SPELL_MASTERY, CLASS_MAGIC_USER, 45, 0, mp);
  spell_levelm(SKILL_HALF_MANA, CLASS_MAGIC_USER, 110, 1, np);


  //Special
  spell_levelm(SPELL_WORD_OF_RECALL, CLASS_MAGIC_USER, 1, 0,np);
  spell_levelm(SPELL_INVISIBLE, CLASS_MAGIC_USER, 15, 0,np);

  spell_levelm(SPELL_STRENGTH, CLASS_MAGIC_USER, 30, 0, hp);
  spell_levelm(SPELL_INFRAVISION, CLASS_MAGIC_USER, 30, 0, np);

  spell_levelm(SPELL_IDENTIFY, CLASS_MAGIC_USER, 15, 0,np);
  spell_levelm(SPELL_LOCATE_OBJECT, CLASS_MAGIC_USER, 30, 0,np);
  spell_levelm(SPELL_FLOAT, CLASS_MAGIC_USER, 30, 0,np);
  spell_levelm(SPELL_FIND_TARGET, CLASS_MAGIC_USER, 45, 0,1);
  spell_levelm(SPELL_CHARM, CLASS_MAGIC_USER, 45, 0,mp);
  spell_levelm(SPELL_ENCHANT_WEAPON, CLASS_MAGIC_USER, 45, 0,mp);  
  spell_levelm(SKILL_TETHER, CLASS_MAGIC_USER, 45, 1, mp);
  spell_levelm(SPELL_GATE, CLASS_MAGIC_USER, 45, 0, hp);
 

  /* CLERICS */
  //Negative
  spell_levelc(SPELL_WEB, CLASS_CLERIC, 15, 0,mp);
  spell_levelc(SPELL_DISPEL_MAGIC, CLASS_CLERIC, 30, 0,mp);
  

  //Positive
  spell_levelc(SPELL_HEAL, CLASS_CLERIC, 1, 0,mp);
  spell_levelc(SPELL_GROUP_HEAL, CLASS_CLERIC, 10, 0,mp);
  spell_levelc(SPELL_RESURRECTION, CLASS_CLERIC, 45, 0,hp);
  spell_levelc(SPELL_SENSE_LIFE, CLASS_CLERIC, 1, 0, np);
  spell_levelc(SPELL_MANASHIELD, CLASS_CLERIC, 60, 1,hp);
  spell_levelc(SPELL_BLESS, CLASS_CLERIC, 1, 0,mp);
  spell_levelc(SPELL_AUROROUS, CLASS_CLERIC, 1, 0,mp);
  spell_levelc(SPELL_ARMOR, CLASS_CLERIC, 1, 0,mp);
  spell_levelc(SPELL_PROT_FROM_EVIL, CLASS_CLERIC, 15, 0,hp);
  spell_levelc(SPELL_REFRESH, CLASS_CLERIC, 15, 0,hp);
  spell_levelc(SPELL_SANCTUARY, CLASS_CLERIC, 30, 0,mp);
  spell_levelc(SPELL_REMOVE_CURSE, CLASS_CLERIC, 30, 0, np);
  spell_levelc(SPELL_CURE_BLIND, CLASS_CLERIC, 30, 0, np);
//spell_levelc(SPELL_GROUP_ARMOR, CLASS_CLERIC, 45, 1,mp);
  spell_levelc(SPELL_REMOVE_POISON, CLASS_CLERIC, 1, 0, np);

  //Special
  spell_levelc(SPELL_WORD_OF_RECALL, CLASS_CLERIC, 1, 0,np);
  spell_levelc(SPELL_DETECT_INVIS, CLASS_CLERIC, 1, 0,np);  
  spell_levelc(SPELL_IDENTIFY, CLASS_CLERIC, 15, 0,np); 
  spell_levelc(SPELL_SHROUD, CLASS_CLERIC, 45, 1,hp);
  spell_levelc(SPELL_SUSTAINANCE, CLASS_CLERIC, 1, 0,mp);
  spell_levelc(SPELL_SUMMON, CLASS_CLERIC, 30, 0,mp);
  

  /* THIEVES */
  //Passive
  spell_levelt(SKILL_DOUBLE_STAB, CLASS_THIEF, 1, 0,mp);
  spell_levelt(SKILL_WEAPON_DAGGERS, CLASS_THIEF, 1, 0,mp);
  spell_levelt(SKILL_MV_REGEN, CLASS_THIEF, 1,0,mp);
  spell_levelt(SKILL_SEE_INV, CLASS_THIEF, 15,0,np);
  //Effect
  spell_levelt(SKILL_SNEAK, CLASS_THIEF, 1, 0, mp);
  spell_levelt(SKILL_BANDAGE, CLASS_THIEF, 30, 0,mp);
  //Active
  spell_levelt(SKILL_PICK_LOCK, CLASS_THIEF, 1, 0,hp);
  spell_levelt(SKILL_TRACK, CLASS_THIEF, 15, 0,mp);
  spell_levelt(SKILL_STEAL, CLASS_THIEF, 15, 0,mp);
  spell_levelt(SKILL_BACKSTAB, CLASS_THIEF, 15, 0,mp);
  spell_levelt(SKILL_TRIP, CLASS_THIEF, 30, 0,mp);
  spell_levelt(SKILL_DISARM, CLASS_THIEF, 30, 0,hp);
  
  //spell_levelt(SKILL_SHADE, CLASS_THIEF, 50, 1,1); 


  /* WARRIORS */
  //Passive
  spell_levelw(SKILL_MULTI_ATTACK, CLASS_WARRIOR,  30, 0, hp);
  spell_levelw(SKILL_WEAPON_SWORDS, CLASS_WARRIOR, 1, 0, mp);
  spell_levelt(SKILL_HP_REGEN, CLASS_WARRIOR, 1, 0, mp);
  //Effect
  spell_levelw(SKILL_RESCUE, CLASS_WARRIOR, 15, 0,np);
  spell_levelw(SPELL_BERSERK, CLASS_WARRIOR, 30, 1,hp);
  //Active
  spell_levelw(SKILL_KICK, CLASS_WARRIOR, 1, 0,mp);
  spell_levelw(SKILL_BASH, CLASS_WARRIOR, 15, 0,mp);
  spell_levelw(SKILL_WHIRLWIND, CLASS_WARRIOR, 45, 0,mp);
  
}


/*
 * This is the exp given to implementors -- it must always be greater
 * than the exp required for immortality, plus at least 20,000 or so.
 */
#define EXP_MAX  999999999999

// 0 Mage, 1 Cleric, 2 Thief, 3 Warrior
byte first_multi(byte class) {
	switch (class) 
		{
		case CLASS_MAGE:    return CLASS_CLERIC;
		case CLASS_CLERIC:  return CLASS_MAGE;
		case CLASS_THIEF:   return CLASS_WARRIOR;
		case CLASS_WARRIOR: return CLASS_THIEF;
		}	
	return -1;
}

byte second_multi(byte class) {
	switch (class) 
		{
		case CLASS_MAGE:    return CLASS_THIEF;
		case CLASS_CLERIC:  return CLASS_WARRIOR;
		case CLASS_THIEF:   return CLASS_MAGE;
		case CLASS_WARRIOR: return CLASS_CLERIC;
		}	
	return -1;
}

byte third_multi(byte class) {
	switch (class) 
		{
		case CLASS_MAGE:    return CLASS_WARRIOR;
		case CLASS_CLERIC:  return CLASS_THIEF;
		case CLASS_THIEF:   return CLASS_CLERIC;
		case CLASS_WARRIOR: return CLASS_MAGE;
		}	
	return -1;
}

float exp_multiplier(byte char_class, byte gain_class)
{
	const float MULTIPLIER_ONE         = 1.0;
	const float MULTIPLIER_TWO         = 2.0;
	const float MULTIPLIER_THREE       = 3.0;
	const float MULTIPLIER_FOUR        = 4.0;

	if (char_class == gain_class) 
		return MULTIPLIER_ONE;
	if (first_multi(char_class) == gain_class) 
		return MULTIPLIER_TWO;
	if (second_multi(char_class) == gain_class) 
		return MULTIPLIER_THREE;
	if (third_multi(char_class) == gain_class) 
		return MULTIPLIER_FOUR;

	return MULTIPLIER_FOUR;
}

float gold_multiplier(byte char_class, byte gain_class)
{
	const float MULTIPLIER_ONE         = 0.0;
	const float MULTIPLIER_TWO         = 2.0;
	const float MULTIPLIER_THREE       = 3.0;
	const float MULTIPLIER_FOUR        = 4.0;


	if (char_class == gain_class) 
		return MULTIPLIER_ONE;
	if (first_multi(char_class) == gain_class) 
		return MULTIPLIER_TWO;
	if (second_multi(char_class) == gain_class) 
		return MULTIPLIER_THREE;
	if (third_multi(char_class) == gain_class) 
		return MULTIPLIER_FOUR;
	
	return MULTIPLIER_FOUR;
}

int needed_exp(byte char_class, byte gain_class, int level)
{
	return level_exp(level) * exp_multiplier(char_class, gain_class);
}

int needed_gold(byte char_class, byte gain_class, int level)
{
	return level_gold(level) * gold_multiplier(char_class, gain_class);
}


/* Function to return the exp required for each class/level */
int level_exp(int level)
{
  /* Exp required for normal mortals is below */
   if (level < 20 && level >= 0) {
     return level*level*level * 50;
   }
   if (level < 40 && level >= 20) {
     return level*level*level * 75;
   }
   if (level < 59 && level >= 40) {
     return level*level*level * 100;
   }
   if (level == 59) {
     return 30000000;
   }

  return 2000000000;
}

int level_gold(int level) {
  if (level < LVL_IMMORT && level >= 0) {
    return level_exp(level) * 0.2;
  }
 
  return 2000000000;
}

int horse_exp(int horselevel)
{
  if (horselevel < 0) {
    log("SYSERR: Requesting exp for invalid level %d!", horselevel);
    return 0;
  }


  /* Exp required for horses below */

    switch (horselevel) {
      case  0: return 0;
      case  1: return 50;
      case  2: return 55;
      case  3: return 60;
      case  4: return 65;
      case  5: return 70;
      case  6: return 75;
      case  7: return 80;
      case  8: return 85;
      case  9: return 90;
      case  10:return 100;
  }
 return 1234;
}

/* 
 * Default titles of male characters.
 */
const char *title_male(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Man";
  if (level == LVL_IMPL)
    return "Implementor";

  switch (chclass) {

    case CLASS_MAGIC_USER:

      return "the Mage";
      
    break;

    case CLASS_CLERIC:
      return "the Cleric";
    break;
    case CLASS_THIEF:
      return "the Thief";
    break;

    case CLASS_WARRIOR:
      return "the Warrior";
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}


/* 
 * Default titles of female characters.
 */
const char *title_female(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Woman";
  if (level == LVL_IMPL)
    return "the Implementress";

  switch (chclass) {

    case CLASS_MAGIC_USER:
      return "the Witch";
    break;

    case CLASS_CLERIC:
       return "the Cleric";
    break;

    case CLASS_THIEF:
      return "the Thief";
    break;

    case CLASS_WARRIOR:
      return "the Warrior";
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}

