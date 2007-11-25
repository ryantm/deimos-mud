/*
************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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
#include "dg_scripts.h"
#include "assemblies.h"
/*   external vars  */
extern room_vnum blue_room;
extern room_vnum red_room;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;

extern struct spell_info_type spell_info[];
extern int guild_info[][3];
extern struct player_index_element *player_table;
extern int freeze_game_running;
extern int freeze_game_redpnt;
extern int freeze_game_bluepnt;
extern int file_to_string_alloc(const char *name, char **buf);
extern char *econrank;
extern char *assassinrank;
extern char *gms;
extern void aremoverelig(struct char_data *ch);
extern void premoverelig(struct char_data *ch);
extern void dremoverelig(struct char_data *ch);
extern void rremoverelig(struct char_data *ch);
extern void cremoverelig(struct char_data *ch);
extern struct clan_type *clan_info;
extern const char *pc_class_types[];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
void return_blue_flag();
void return_red_flag();
bool quotaok();

int level_exp(int level);
float level_gold(int level);
int horse_exp(int horselevel);
ACMD(do_drop);
ACMD(do_gen_door);
ACMD(do_say);
ACMD(do_look);
extern struct new_guild_info guilds[];
void update_pos(struct char_data * ch);
int needed_exp(byte char_class, byte gain_class, int level);
int needed_gold(byte char_class, byte gain_class, int level);
byte get_class_level(struct char_data *ch, byte class);

/* local functions */

void sort_spells(void);
int compare_spells(const void *x, const void *y);
void store_mail(long to, long from, sh_int vnum, char *message_pointer);
const char *how_good(int percent);
void list_skills(struct char_data * ch);
void list_skillst(struct char_data * ch); 
void list_skillsw(struct char_data * ch); 
void list_skillsm(struct char_data * ch); 
void list_skillsc(struct char_data * ch); 
void sort_econrank(struct char_data * ch);
void new_gm(struct char_data * ch);
void add_to_gm_list(struct char_data *ch);
SPECIAL(gm_zone);
SPECIAL(guild);
SPECIAL(dump);
SPECIAL(mayor);
void npc_steal(struct char_data * ch, struct char_data * victim);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(super_magic_user);
SPECIAL(guild_guard);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(cityguard);
SPECIAL(pet_shops);
SPECIAL(bank);
SPECIAL(recharger);
SPECIAL(pop_dispenser);
SPECIAL(wiseman);
SPECIAL(meta_physician);
SPECIAL(level);
SPECIAL(assassin_guild);
SPECIAL(embalmer);
SPECIAL(thief_class);
SPECIAL(warrior_class);
SPECIAL(thief_guild);
SPECIAL(warrior_guild);
SPECIAL(mage_guild);
SPECIAL(cleric_guild);
SPECIAL(butcher);
SPECIAL(bounty);
SPECIAL(stableshop);
SPECIAL(stable);
SPECIAL(gold_chipper);
SPECIAL(cussgiver);
SPECIAL(casino);
SPECIAL(rental);
SPECIAL(warehouse);
SPECIAL(cestadeath);
SPECIAL(firestation);
SPECIAL(calise_hunter);
SPECIAL(maiden);
SPECIAL(newspaper_stand);
SPECIAL(gm_plaque);
SPECIAL(block_north);
SPECIAL(block_east);
SPECIAL(block_south);
SPECIAL(block_west);
SPECIAL(gm_guild);
SPECIAL(cap_flag);
SPECIAL(religion);

/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */
void do_mob_bash(struct char_data *ch, struct char_data *vict)
  {

    int hit_roll = 0, to_hit = 0;
    
    hit_roll = number (1,100) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;

    if (hit_roll < to_hit) {
      GET_POS(ch) = POS_SITTING;
      damage(ch, vict, 0, SKILL_BASH);
    } else {
      GET_POS(vict) = POS_SITTING;
      damage(ch, vict, GET_LEVEL(ch), SKILL_BASH);
      WAIT_STATE(vict, PULSE_VIOLENCE * 2);
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
  }

  void do_mob_disarm(struct char_data *ch, struct char_data *vict)
  {

    int hit_roll = 0, to_hit = 0;
    struct obj_data *weap; 
   
    hit_roll = number (1,100) + GET_DEX(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;

    if (!(weap = GET_EQ(FIGHTING(ch), WEAR_WIELD))) {
      send_to_char("Nope, sorry\r\n", ch);
      return;      
    }

    if (hit_roll < to_hit) {
      if(GET_EQ(vict, WEAR_WIELD)){
      act("&3You disarm $p from $N's hand!&0", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
      act("&3$n disarms $p from your hand!&0", FALSE, ch, weap, FIGHTING(ch), TO_VICT); 
      act("&3$n disarms $p from $N's hand!&0", FALSE, ch, weap, FIGHTING(ch), TO_NOTVICT);
      obj_to_room(unequip_char(FIGHTING(ch), WEAR_WIELD), vict->in_room);
    } else {
      act("&3You fail to disarm $N&0", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
      act("&3$n fails to disarm you&0", FALSE, ch, weap, FIGHTING(ch), TO_VICT);
      act("&3$n fails to disarm $N&0", FALSE, ch, weap, FIGHTING(ch), TO_NOTVICT);
          }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE);

  }

  void do_mob_kick(struct char_data *ch, struct char_data *vict)
  {

    int hit_roll = 0, to_hit = 0;
    
    hit_roll = number (1,100) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;

    if (hit_roll < to_hit) {
      damage(ch, vict, 0, SKILL_KICK);
    } else {
      damage(ch, vict, GET_LEVEL(ch) / 2, SKILL_KICK);
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
  }

  void do_mob_trip(struct char_data *ch, struct char_data *vict)
  {

    int hit_roll = 0, to_hit = 0;
    
    hit_roll = number (1,100) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;

    if (hit_roll < to_hit) {
      damage(ch, vict, 0, SKILL_TRIP);
    } else {
      damage(ch, vict, GET_LEVEL(ch) / 2, SKILL_TRIP);
      WAIT_STATE(vict, PULSE_VIOLENCE * 2);
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
  }

void do_mob_backstab(struct char_data *ch, struct char_data *vict)
  {

    int hit_roll = 0, to_hit = 0;
    
    hit_roll = number (1,100) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;
    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) && !FIGHTING(ch))
    {
    if (FIGHTING(ch)){
      act("$N just tried to backstab you during combat!", FALSE, vict, 0, ch, TO_CHAR);
      act("$e notices you. You cannot backstab in combat!", FALSE, vict, 0, ch, TO_VICT);
      act("$N attempts to backstab $n during combat!", FALSE, vict, 0, ch, TO_NOTVICT);
    }
    if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
      act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
      act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
      act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
      hit(vict, ch, TYPE_UNDEFINED);
      return;
    }

    if (hit_roll < to_hit) {
      damage(ch, vict, 0, SKILL_BACKSTAB);
    } else {
      hit(ch, vict, SKILL_BACKSTAB);
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
    }
  }

int spell_sort_info[MAX_SKILLS + 1];

int compare_spells(const void *x, const void *y)
{
  int	a = *(const int *)x,
	b = *(const int *)y;

  return strcmp(spell_info[a].name, spell_info[b].name);
}

void sort_spells(void)
{
  int a;

  /* initialize array, avoiding reserved. */
  for (a = 1; a <= MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  qsort(&spell_sort_info[1], MAX_SKILLS, sizeof(int), compare_spells);
}

const char *how_good(int percent)
{     
  if (percent < 0)
    return " error)";
  if (percent == 0)
    return " (not learned)";
  if (percent <= 10)
    return " (awful)";
  if (percent <= 20)
    return " (bad)";
  if (percent <= 40)
    return " (poor)";
  if (percent <= 55)
    return " (average)";
  if (percent <= 70)
    return " (fair)";
  if (percent <= 80)
    return " (good)";
  if (percent <= 85)
    return " (very good)";

  return " (superb)";
}

const char *prac_types[] = {
  "spell",
  "skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[4][NUM_CLASSES];

#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

int LEARNED(int spell, struct char_data *ch, int realclass)
{
    int fakeclass = GET_CLASS(ch), maxpower = spell_info[spell].max_power[(int)GET_CLASS(ch)];

    if (fakeclass == realclass || maxpower == 1)
       return maxpower;

   if (maxpower == 10)
   {
    switch(fakeclass)
    {
      case CLASS_MAGIC_USER:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower; break;
           case CLASS_CLERIC: return maxpower - 2; break;
           case CLASS_THIEF: return maxpower -4; break;
           case CLASS_WARRIOR: return maxpower - 6; break;
         }
      break;
      case CLASS_CLERIC:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-2; break;
           case CLASS_CLERIC: return maxpower; break;
           case CLASS_THIEF: return maxpower -6; break;
           case CLASS_WARRIOR: return maxpower - 4; break;
         }
      break;
      case CLASS_THIEF:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-4; break;
           case CLASS_CLERIC: return maxpower - 6; break;
           case CLASS_THIEF: return maxpower; break;
           case CLASS_WARRIOR: return maxpower - 2; break;
         }
      break;
      case CLASS_WARRIOR:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-6; break;
           case CLASS_CLERIC: return maxpower - 4; break;
           case CLASS_THIEF: return maxpower -2; break;
           case CLASS_WARRIOR: return maxpower; break;
         }
      break;
    } 
   }

   if (maxpower == 5)
   {
    switch(fakeclass)
    {
      case CLASS_MAGIC_USER:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower; break;
           case CLASS_CLERIC: return maxpower - 1; break;
           case CLASS_THIEF: return maxpower -2; break;
           case CLASS_WARRIOR: return maxpower - 3; break;
         }
      break;
      case CLASS_CLERIC:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-1; break;
           case CLASS_CLERIC: return maxpower; break;
           case CLASS_THIEF: return maxpower -3; break;
           case CLASS_WARRIOR: return maxpower - 2; break;
         }
      break;
      case CLASS_THIEF:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-2; break;
           case CLASS_CLERIC: return maxpower - 3; break;
           case CLASS_THIEF: return maxpower; break;
           case CLASS_WARRIOR: return maxpower - 1; break;
         }
      break;
      case CLASS_WARRIOR:
         switch(realclass)
         {
           case CLASS_MAGIC_USER: return maxpower-3; break;
           case CLASS_CLERIC: return maxpower - 2; break;
           case CLASS_THIEF: return maxpower -1; break;
           case CLASS_WARRIOR: return maxpower; break;
         }
      break;
    } 
   }

  return 0;
}

void list_skillst(struct char_data * ch)
{
  int i, sortpos;
  const int temp = ch->player.chclass;
  GET_CLASS(ch) = CLASS_THIEF;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf + strlen(buf), "You know of the following thief skills:\r\n");

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (GET_THIEF_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      if ((temp != CLASS_THIEF && spell_info[i].multi != 1) 
          || (temp == CLASS_THIEF)) { 
      sprintf(buf, "%-20s %d/%d\r\n", spell_info[i].name, GET_SKILL(ch, i), LEARNED(i, ch, temp));
      strcat(buf2, buf);	/* The above, ^ should always be safe to do. */
      }
    }
  }
  GET_CLASS(ch) = temp;
  page_string(ch->desc, buf2, 1);
}

void list_skillsw(struct char_data * ch)
{
  int i, sortpos;
  const int temp = GET_CLASS(ch);
    GET_CLASS(ch) = CLASS_WARRIOR;
 
  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf + strlen(buf), "You know of the following warrior skills:\r\n");

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (GET_WARRIOR_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      if ((temp != CLASS_WARRIOR && spell_info[i].multi != 1)  
          || (temp == CLASS_WARRIOR)) { 
      sprintf(buf, "%-20s %d/%d\r\n", spell_info[i].name, GET_SKILL(ch, i), LEARNED(i, ch, temp));
      strcat(buf2, buf);	/* The above, ^ should always be safe to do. */
      }
    }
  }
  GET_CLASS(ch) = temp;
  page_string(ch->desc, buf2, 1);
}

void list_skillsm(struct char_data * ch)
{
  int i, sortpos;
  const int temp = GET_CLASS(ch);
  GET_CLASS(ch) = CLASS_MAGIC_USER;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf + strlen(buf), "You know of the following mage spells:\r\n");

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      GET_CLASS(ch) = temp;
      break;
    }
    if (GET_MAGE_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      if ((temp != CLASS_MAGIC_USER && spell_info[i].multi != 1)
          || (temp == CLASS_MAGIC_USER)) {        
      sprintf(buf, "%-20s %d/%d\r\n", spell_info[i].name, GET_SKILL(ch, i), LEARNED(i, ch, temp));
      strcat(buf2, buf);	/* The above, ^ should always be safe to do. */
      }
    }
  }
  GET_CLASS(ch) = temp;
  page_string(ch->desc, buf2, 1);
}


void list_skillsc(struct char_data * ch)
{
  int i, sortpos;
  const int temp = GET_CLASS(ch);
  GET_CLASS(ch) = CLASS_CLERIC; 

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf + strlen(buf), "You know of the following cleric spells:\r\n");

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (GET_CLERIC_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      if ((temp != CLASS_CLERIC && spell_info[i].multi != 1)
          || (temp == CLASS_CLERIC)) {    
      sprintf(buf, "%-20s %d/%d\r\n", spell_info[i].name, GET_SKILL(ch, i), LEARNED(i, ch, temp));
      strcat(buf2, buf);	/* The above, ^ should always be safe to do. */
      }
    }
  }
  GET_CLASS(ch) = temp;
  page_string(ch->desc, buf2, 1);
}


void list_skills(struct char_data * ch)
{
  int i, sortpos;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf + strlen(buf), "You know of the following %ss:\r\n", SPLSKL(ch));

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      sprintf(buf, "%-20s %d/%d\r\n", spell_info[i].name, GET_SKILL(ch, i), LEARNED(i, ch, GET_CLASS(ch)));
      strcat(buf2, buf);	/* The above, ^ should always be safe to do. */
    }
  }
  page_string(ch->desc, buf2, 1);
}

SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return (0);

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    send_to_char("You are awarded for outstanding performance.\r\n", ch);
    
act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return (1);
}


SPECIAL(mayor)
{
  const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path = NULL;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return (FALSE);

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
    do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return (FALSE);
}


void inc_class_stats(struct char_data *ch, byte class)
{
	int add_mana=0, add_move=0, add_hp = 0;

	add_hp   = con_app[GET_CON(ch)].hitp;
	add_mana = MAX(0, ((GET_INT(ch) - 16) /2));
	add_move = MAX(0, ((GET_DEX(ch) - 16) /2));

	switch (class) 
		{
		case CLASS_MAGE:
			//MAGE GUILD STATS
      switch (GET_CLASS(ch)) 
				{
				case CLASS_MAGIC_USER:
					add_hp += number(2, 5);
					add_mana += number(8, 17);
					add_move += number(1, 3);
					break;
				case CLASS_CLERIC:
					add_hp += number(2, 5);
					add_mana += number(6, 13);
					add_move += number(1, 2);
					break;
				case CLASS_THIEF:
					add_hp += number(2, 4);
					add_mana += number(2, 5);
					add_move += number(2, 4);
					break;
				case CLASS_WARRIOR:
					add_hp += number(3, 7);
					add_mana += number(2, 4);
					add_move += number(2, 4);
					break;
				}
			break;
		case CLASS_CLERIC:
			//CLERIC GUILD STATS
			switch (GET_CLASS(ch)) 
				{
				case CLASS_MAGIC_USER:
					add_hp += number(1, 3);
					add_mana += number(6, 13);
					add_move += number(0, 1);
					break;
				case CLASS_CLERIC:
					add_hp += number(4, 8);
					add_mana += number(6, 13);
					add_move += number(2, 4);
					break;
				case CLASS_THIEF:
					add_hp += number(2, 4);
					add_mana += number(2, 5);
					add_move += number(2, 4);
					break;
				case CLASS_WARRIOR:
					add_hp += number(3, 7);
					add_mana += number(2, 4);
					add_move += number(2, 4);
					break;
				}
			break;
		case CLASS_THIEF:
			//THIEF GUILD STATS
			switch (GET_CLASS(ch)) 
				{
				case CLASS_MAGE:
					add_hp += number(2, 5);
					add_mana = 0;
					add_move += number(0,1);
					break;
				case CLASS_CLERIC:
					add_hp += number(2, 5);
					add_mana = 0;
					add_move += number(0,6);
					break;
				case CLASS_THIEF:
					add_hp += number(7, 14);
					add_mana = 0;
					add_move += number(7, 14);
					break;
				case CLASS_WARRIOR:
					add_hp += number(8, 17);
					add_mana = 0;
					add_move += number(2, 5);
					break;
				}
			break;
		case CLASS_WARRIOR:
			//WARRIOR GUILD STATS
			switch(GET_CLASS(ch)) 
				{
				case CLASS_MAGIC_USER:
					add_hp += number(2, 5);
					add_mana = 0;
					add_move += number(1, 3);
					break;
				case CLASS_CLERIC:
					add_hp += number(6, 12);
					add_mana = 0;
					add_move += number(1, 2);
					break;
				case CLASS_THIEF:
					add_hp += number(7, 14);
					add_mana = 0;
					add_move += number(2, 5);
					break;
				case CLASS_WARRIOR:
					add_hp += number(8, 17);
					add_mana = 0;
					add_move += number(2, 4);
					break;
				}
			break;
		}	

	ch->points.max_hit  += MAX(0, add_hp);
	ch->points.max_move += MAX(0, add_move); 
	ch->points.max_mana += MAX(0, add_mana); 
	
	GET_PRACTICES(ch) += 1;
	send_to_char("You gain one practice session.\r\n", ch);

	sprintf(buf, "You gain (%d) hit points, (%d) mana, (%d) movement.\r\n", 
					add_hp,
					add_mana, 
					add_move);
	send_to_char(buf, ch);
}

void inc_class_level(struct char_data *ch, byte class) 
{
	struct descriptor_data *dd = NULL;
	struct obj_data *obj;
	int num;
	const int inc_by = 1;
	const int new_level = get_class_level(ch, class) + inc_by;

	switch (class) 
		{
		case CLASS_MAGE:    SET_MAGE_LEVEL(ch,    new_level); break;
		case CLASS_CLERIC:  SET_CLERIC_LEVEL(ch,  new_level); break;
		case CLASS_THIEF:   SET_THIEF_LEVEL(ch,   new_level); break;
		case CLASS_WARRIOR: SET_WARRIOR_LEVEL(ch, new_level); break;
		}

	// Set regular level to class level
	GET_LEVEL(ch) = get_class_level(ch, GET_CLASS(ch));

	inc_class_stats(ch, class);

	//Notify immortals and logs
	sprintf(buf, "%s gained one %s level to level %d.",
					GET_NAME(ch), 
					pc_class_types[(int)class], 
					new_level);
	mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

	// Notify everyone
	sprintf(buf, "&G%s gained one %s level.&n", 
					GET_NAME(ch), 
					pc_class_types[(int)class]);
	for (dd = descriptor_list; dd; dd = dd->next)
		if (STATE(dd) == CON_PLAYING && dd != ch->desc && PRF_FLAGGED2(dd->character,PRF2_HEARLEVEL))
			act(buf, 0, 0, 0, dd->character, TO_VICT | TO_SLEEP);

	//Notify self
	sprintf(buf, "You gain one %s level!\r\n",
					pc_class_types[(int)class]);
	send_to_char(buf, ch);
	
	if(GET_CLASS(ch) == class && new_level == 60)
		{
			sprintf(buf, "You have now mastered %s. Congratulations.\r\n",
							pc_class_types[(int)class]);
			send_to_char(buf, ch);
			send_to_char("Check your inventory for a Master Item.\r\n", ch);

			num = number(0, 14);
			obj = read_object(100 + num, VIRTUAL);
			obj_to_char(obj, ch);
		}
	if(GET_TOTAL_LEVEL(ch) == MAX_TOTAL_LEVEL)
		{
			new_gm(ch);
		}
}


void cant_multiclass_until(struct char_data *ch, int level, char *again) {
	sprintf(buf, "You cannot multiclass %suntil you reach level %d in %s.\r\n", again, level, pc_class_types[(int)GET_CLASS(ch)]);
	send_to_char(buf, ch);
}

int gain(struct char_data *ch, int gain_class) {
  int needs_gold, needs_exp, short_exp, short_gold;

  if (get_class_level(ch, gain_class) >= MAX_MASTER_LEVEL) 
		{
			sprintf(buf, "You've already mastered the %s class.\r\n", pc_class_types[gain_class]);
			send_to_char(buf, ch);
			return (TRUE);
		}
	
	// Checks for additional multiclassing.
	if (GET_CLASS(ch) != gain_class) 
		{
			if (GET_LEVEL(ch) < MULTICLASS_LEVEL_ONE) 
				{
					cant_multiclass_until(ch, MULTICLASS_LEVEL_ONE, "");
					return (TRUE);
				} 
			
			if ((get_class_level(ch, gain_class) == 0)	
					&& (PLR_FLAGGED(ch, PLR_MULTI)) && (GET_LEVEL(ch) < MULTICLASS_LEVEL_TWO)) 
				{  
					cant_multiclass_until(ch, MULTICLASS_LEVEL_TWO, "again ");
					return (TRUE);
				}
			
			if ((get_class_level(ch, gain_class) == 0) 
					&& (PLR_FLAGGED(ch, PLR_MULTII)) && (GET_LEVEL(ch) < MULTICLASS_LEVEL_THREE)) 
				{ 
					cant_multiclass_until(ch, MULTICLASS_LEVEL_THREE, "again ");
					return (TRUE);
				}
		}

	needs_exp  = needed_exp( GET_CLASS(ch), gain_class, get_class_level(ch, gain_class));
	needs_gold = needed_gold(GET_CLASS(ch), gain_class, get_class_level(ch, gain_class));
	

	//Actually level up.
	if (GET_EXP(ch) >= needs_exp	&& GET_GOLD(ch) >= needs_gold)
		{
			GET_EXP(ch)  -= needs_exp;
			GET_GOLD(ch) -= needs_gold;
			inc_class_level(ch, gain_class);
			save_char(ch, NOWHERE);   
		}
	else
		{
			short_exp  = MAX(0, needs_exp  - GET_EXP(ch));
			short_gold = MAX(0, needs_gold - GET_GOLD(ch));
			sprintf(buf, "You are short %d experience and %d gold.\r\n", short_exp, short_gold);
			send_to_char(buf, ch);
			return (TRUE);
		}

	//Muticlassing flags
	if ((GET_CLASS(ch) != gain_class) && PLR_FLAGGED(ch, PLR_MULTII) && (GET_LEVEL(ch) == MULTICLASS_LEVEL_THREE))
		SET_BIT(PLR_FLAGS(ch), PLR_MULTIII);
	if ((GET_CLASS(ch) != gain_class) && PLR_FLAGGED(ch, PLR_MULTI) && (GET_LEVEL(ch) >= MULTICLASS_LEVEL_TWO))
		SET_BIT(PLR_FLAGS(ch), PLR_MULTII);
	if (GET_CLASS(ch) != gain_class) {
		SET_BIT(PLR_FLAGS(ch), PLR_MULTI);
	}
	return (TRUE);
}

SPECIAL(thief_guild)
{
  char buf[128];
  int percent, skill_num;
  extern struct spell_info_type spell_info[];
  int temp = GET_CLASS(ch);


 if(CMD_IS("practice")) {
   skip_spaces(&argument);
	 
  GET_CLASS(ch) = 2;
  if (!*argument) {               
		GET_CLASS(ch) = temp;
		list_skillst(ch);
    return (TRUE);
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }
	
  skill_num = find_skill_num(argument);
  if (skill_num < 1 ||
			((GET_THIEF_LEVEL(ch) + GET_GM_LEVEL(ch)) < spell_info[skill_num].min_level[(int) GET_CLASS(ch)])
			|| (temp != CLASS_THIEF && spell_info[skill_num].multi == 1)) { 
		sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
		send_to_char(buf, ch);
		GET_CLASS(ch) = temp;
		return (TRUE);
	}
	
	if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }
	
  GET_CLASS(ch) = 2;
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;
	
  percent = GET_SKILL(ch, skill_num);
  percent++;
	
  SET_SKILL(ch, skill_num, percent);
	
  if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp))
    send_to_char("You are now learned in that area.\r\n", ch);
	
  GET_CLASS(ch) = temp;
	
  return (TRUE);                   
 }
 
 if (CMD_IS("gain")) {
	 return gain(ch, CLASS_THIEF);
 }
 return (FALSE);
}

SPECIAL(warrior_guild)
{
  char buf[128];
  int percent, skill_num;
  extern struct spell_info_type spell_info[];
  int temp = GET_CLASS(ch);
	
	if(CMD_IS("practice")) {
		skip_spaces(&argument);
		
		GET_CLASS(ch) = 3;
		if (!*argument) {               
			GET_CLASS(ch) = temp;
			list_skillsw(ch);
			return (TRUE);
		}
		if (GET_PRACTICES(ch) <= 0) {
			send_to_char("You do not seem to be able to practice now.\r\n", ch);
			GET_CLASS(ch) = temp;
			return (TRUE);
		}
		
		skill_num = find_skill_num(argument);
		
		if (skill_num < 1 ||
				((GET_WARRIOR_LEVEL(ch) + GET_GM_LEVEL(ch)) < spell_info[skill_num].min_level[(int)GET_CLASS(ch)])
				|| (temp != CLASS_WARRIOR && spell_info[skill_num].multi == 1)) 
			{
				sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
				send_to_char(buf, ch);
				GET_CLASS(ch) = temp;
				return (TRUE);
			}               
    if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp)) {
			send_to_char("You are already learned in that area.\r\n", ch);
			GET_CLASS(ch) = temp;
			return (TRUE);
		}
		
		GET_CLASS(ch) = 3; 
		send_to_char("You practice for a while...\r\n", ch);
		GET_PRACTICES(ch)--;
		
		percent = GET_SKILL(ch, skill_num);
		percent++;
		
		SET_SKILL(ch, skill_num, percent);
		
		if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp))
			send_to_char("You are now learned in that area.\r\n", ch);
		
		GET_CLASS(ch) = temp;
		
		return (TRUE);                   
  }
	
  if (CMD_IS("gain")) {
		return gain(ch, CLASS_WARRIOR);
  }

  return (FALSE);
}
  
SPECIAL(mage_guild)
{
  char buf[128];
  int percent, skill_num;
  extern struct spell_info_type spell_info[];
  int temp = GET_CLASS(ch);

 if(CMD_IS("practice")) {
   skip_spaces(&argument);

  GET_CLASS(ch) = 0;
  if (!*argument) {               
    GET_CLASS(ch) = temp;
     list_skillsm(ch);
    return (TRUE);
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }

  skill_num = find_skill_num(argument);
  
  if (skill_num < 1 ||
      ((GET_MAGE_LEVEL(ch) + GET_GM_LEVEL(ch)) 
         < spell_info[skill_num].min_level[(int) GET_CLASS(ch)])
     || (temp != CLASS_MAGIC_USER && spell_info[skill_num].multi == 1)) 
  { 
    sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
    send_to_char(buf, ch);
    GET_CLASS(ch) = temp;
    return (TRUE);               
  }               
    if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }

  GET_CLASS(ch) = 0;
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch),
  int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(skill_num, ch, temp), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp))
    send_to_char("You are now learned in that area.\r\n", ch);

   GET_CLASS(ch) = temp;

  return (TRUE);                   
  }

  if (CMD_IS("gain")) {
		return gain(ch, CLASS_MAGE);
  }
  return (FALSE);
}
  
SPECIAL(cleric_guild)
{
  char buf[128];
  int percent, skill_num;
  extern struct spell_info_type spell_info[];
  int temp = GET_CLASS(ch);

 if(CMD_IS("practice")) {
   skip_spaces(&argument);

  GET_CLASS(ch) = 1;
  if (!*argument) {               
    GET_CLASS(ch) = temp;
     list_skillsc(ch);
    return (TRUE);
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }

  skill_num = find_skill_num(argument);

  if (skill_num < 1 ||
     ((GET_CLERIC_LEVEL(ch) + GET_GM_LEVEL(ch)) < spell_info[skill_num].min_level[(int) GET_CLASS(ch)])
     || (temp != CLASS_CLERIC && spell_info[skill_num].multi == 1)) 
  {
    sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
    send_to_char(buf, ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }  
                                
    if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    GET_CLASS(ch) = temp;
    return (TRUE);
  }

  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch),
  int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(skill_num, ch, temp), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(skill_num, ch, temp))
    send_to_char("You are now learned in that area.\r\n", ch);

  GET_CLASS(ch) = temp;


  return (TRUE);                   
  }

  if (CMD_IS("gain")) {
		return gain(ch, CLASS_CLERIC);
  }
  return (FALSE);
}
  
  

/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


SPECIAL(snake)
{
  if (cmd)
    return (FALSE);

  if (GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return (FALSE);

  if (GET_POS(ch) != POS_STANDING)
    return (FALSE);

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4))) {
      npc_steal(ch, cons);
      return (TRUE);
    }
  return (FALSE);
}


SPECIAL(magic_user)
{
  struct char_data *vict;
  int spellnum;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  //Mobs get affected by trip.
  if (GET_MOB_WAIT(ch) > 0)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
    return (TRUE);

  /* Make sure mobs don't cast every round. */
 if (1 == number(1,3)) {

  /* Intelligent Mobs */

  /* Mob will dispel you if you are sanced */
  if ((GET_LEVEL(ch) >= 50)) 
  {
    if (AFF_FLAGGED(vict, AFF_SANCTUARY)) 
    {
     cast_spell(ch, vict, NULL, SPELL_DISPEL_MAGIC);
     return (TRUE);
    }
  }

  /* Mob will web you if level 66, and your HP is low...MUHAHAHAHAHAHA */
  if (GET_LEVEL(ch) == 66) {
    if ((GET_HIT(vict) < GET_MAX_HIT(vict) / 3 || GET_HIT(vict) <= (GET_WIMP_LEV(vict) + 50)) && !AFF_FLAGGED(vict, AFF_WEB)) {
     cast_spell(ch, vict, NULL, SPELL_WEB);
     return (TRUE);
     }
  }

  /* Mob will blind your arse if you arn't careful. */
  if (GET_LEVEL(ch) >= 60) {
    if (GET_HIT(vict) > GET_MAX_HIT(vict)  / 2 && !AFF_FLAGGED(vict, AFF_BLIND) &&  !AFF_FLAGGED(vict, AFF_POISON)) {
     if (number(0,1)) 
     	cast_spell(ch, vict, NULL, SPELL_BLINDNESS);
     else
        cast_spell(ch, vict, NULL, SPELL_POISON);
          
     return (TRUE);
    }
  }

  /* Mob will curse your arse for no apparent reason. */
  if (GET_LEVEL(ch) >= 40) {
    if (!AFF_FLAGGED(vict, AFF_CURSE)) {
     cast_spell(ch, vict, NULL, SPELL_CURSE);
     return (TRUE);
     }
  }


  spellnum = number(1,7);

  switch (spellnum) {
  case 1:
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
    break;
  case 2:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 3:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
    break;
  case 4:
    cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
    break;
  case 5:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  case 6:
    cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  }
 }

  return (TRUE);
}


SPECIAL(super_magic_user)
{
  struct char_data *vict;
  int spellnum;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  //Mobs get affected by trip.
  if (GET_MOB_WAIT(ch) > 0)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
    return (TRUE);

  /* Super Wizards cast every round. */
 if (1 == 1) {

  /* Intelligent Mobs */

  /* Mob will dispel you if you are sanced */
  if ((GET_LEVEL(ch) >= 50)) 
  {
    if (AFF_FLAGGED(vict, AFF_SANCTUARY) && number (0,1) == 1) 
    {
     cast_spell(ch, vict, NULL, SPELL_DISPEL_MAGIC);
     return (TRUE);
    }
  }

  /* Mob will web you if level 66, and your HP is low...MUHAHAHAHAHAHA */
  if (GET_LEVEL(ch) == 66) {
    if (!AFF_FLAGGED(vict, AFF_WEB)) 
    {
     cast_spell(ch, vict, NULL, SPELL_WEB);
     return (TRUE);
    }
  }

  /* Mob will blind your arse if you arn't careful. */
  if (GET_LEVEL(ch) >= 60) {
    if (!AFF_FLAGGED(vict, AFF_BLIND)) {
     if (number(0,1)) 
     	cast_spell(ch, vict, NULL, SPELL_BLINDNESS);
     else if (!AFF_FLAGGED(vict, AFF_POISON))
        cast_spell(ch, vict, NULL, SPELL_POISON);
          
     return (TRUE);
    }
  }

  spellnum = number(1,7);

  switch (spellnum) {
  case 1:
    cast_spell(ch, vict, NULL, SPELL_NOVASTORM);
    break;
  case 2:
    cast_spell(ch, ch, NULL, SPELL_HEAL);
    cast_spell(ch, ch, NULL, SPELL_HEAL);
    cast_spell(ch, ch, NULL, SPELL_HEAL);
    break;
  case 3:
    cast_spell(ch, vict, NULL, SPELL_ICEBOLT);
    break;
  case 4:
    cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE);
    break;
  case 5:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  case 6:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  case 7:
    cast_spell(ch, vict, NULL, SPELL_TELEPORT);
    break;
  }
 }

  return (TRUE);
}


SPECIAL(warrior_class)
  {
    struct char_data *vict;
    int att_type = 0, hit_roll = 0, to_hit = 0 ;
   
    if (cmd || !AWAKE(ch))
      return FALSE;

    /* if two people are fighting in a room */
    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

        vict = FIGHTING(ch);

        if (number(1, 5) == 5) {
         act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
        }
      if (GET_POS(ch) == POS_FIGHTING) {

        att_type = number(1,40);          

        hit_roll = number (1,100) + GET_STR(ch);
        to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
        if (GET_LEVEL(vict) >= LVL_IMMORT)  
          hit_roll = 0;

        switch(att_type) {  
        case 1: case 2: case 3: case 4:
                  do_mob_kick(ch, vict);
      break;
        case 5: case 6: case 7:
                  do_mob_bash(ch, vict);
          break;
            case 8: case 9: case 10:
                  do_mob_disarm(ch, vict);
          break; 
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            break;
        default:
          break;
        }
    
      }
    }
    return FALSE;  
  }

/*SPECIAL(cleric_class)
  {
    struct char_data *vict;
    int att_type = 0, hit_roll = 0, to_hit = 0 ;
   
    if (cmd || !AWAKE(ch))
      return FALSE;

    if (!FIGHTING(ch))
    {
     if (AFF_FLAGGED(ch, AFF_POISON)) { 
       cast_spell(ch, ch, NULL, SPELL_REMOVE_POISON);
       continue;
     }
     else if (AFF_FLAGGED(ch, AFF_CURSE)) {
       cast_spell(ch, ch, NULL, SPELL_REMOVE_CURSE);
       continue;
     }
     else if (AFF_FLAGGED(ch, AFF_BLIND)) {
       cast_spell(ch, ch, NULL, SPELL_CURE_BLIND); 
       continue;
     }
     else if (!affected_by_spell(ch, SPELL_ARMOR)) {
         cast_spell(ch, ch, NULL, SPELL_ARMOR); 
         continue;
     }
     else if (!affected_by_spell(ch, SPELL_STRENGTH)) {
         cast_spell(ch, ch, NULL, SPELL_STRENGTH); 
         continue;
     }
    return TRUE;
  }

    if (FIGHTING(ch)) 
    {
      if (number (1,3) == 1)
       cast_spell(ch, ch, NULL, SPELL_HEAL);
      return TRUE;
    }

  return FALSE;  
}*/

SPECIAL(thief_class)
  {
    struct char_data *cons;

    if (cmd)
      return (FALSE);

    if (GET_POS(ch) != POS_STANDING)
      return (FALSE);

    //Mobs get affected by trip.
    if (GET_MOB_WAIT(ch) > 0)
      return (FALSE);

    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
      if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0,3))) {
        switch (number(1,3))
        {
         case 1: 
          do_mob_backstab(ch, cons);
         break;
         case 2: 
          npc_steal(ch, cons);
         break;
         case 3:
          do_mob_trip(ch, cons);
         break;
        }
        return (TRUE);
      }
    return (FALSE);
  }

SPECIAL(troll)
  {
    struct char_data *vict;
    int i, attempt;
   
    if (cmd || !AWAKE(ch))
      return FALSE;

    /* if two people are fighting in a room */
    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) 
    {
        vict = FIGHTING(ch);
        if (GET_LEVEL(vict) >= LVL_IMMORT)  
           return FALSE;

        if (number(1, 3) == 3) 
        {
         act("&R$n roars extremely loudly!&n", 1, ch, 0, 0, TO_ROOM);
        }
      if (GET_POS(ch) == POS_FIGHTING) 
      {
        if (number(0,150) < MIN(GET_LEVEL(ch), 60))
        {
        if (vict->master && vict->master->in_room == vict->in_room) 
        {
          vict = vict->master; //Target Leader
        }
          act ("&R$n eyes $N with a dopey expression.&n",1,ch,0,vict,TO_ROOM);
          act ("&R$n picks up $N and throws $M out of the room violently.&n",1,ch,0,vict,TO_ROOM); 
          act ("&R$n picks You up and throws You out of the room violently.&n",1,ch,0,vict,TO_VICT); 
          if (AFF_FLAGGED(vict, AFF_WEB))
          {
            send_to_char("You break out of the webs!\r\n", vict);
            REMOVE_BIT(AFF_FLAGS(vict), AFF_WEB);
            affect_from_char(vict, SPELL_WEB);
            WAIT_STATE(vict, PULSE_VIOLENCE / 2.);
          }
            for (i = 0; i < 6; i++) {
              attempt = number(0, NUM_OF_DIRS - 1);       /* Select a random direction */
              if (CAN_GO(vict, attempt) &&
              !ROOM_FLAGGED(EXIT(vict, attempt)->to_room, ROOM_DEATH) &&
              (!(SECT(EXIT(vict, attempt)->to_room) == SECT_FLYING) ||
               AFF_FLAGGED(vict, AFF_FLOAT))) 
              {
                do_simple_move(vict, attempt, TRUE);
              }
            }
       }
      }
     return TRUE;
    }
    return FALSE;  
  }

SPECIAL(fear_maker)
  {
    struct char_data *vict;
   
    if (cmd || !AWAKE(ch))
      return FALSE;

    /* if two people are fighting in a room */
    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) 
    {
        vict = FIGHTING(ch);
        if (GET_LEVEL(vict) >= LVL_IMMORT)  
           return FALSE;

        if (number(1, 3) == 3) 
        {
         act("&R$n lets out a bone chilling scream that makes you think twice!&n", 1, ch, 0, 0, TO_ROOM);
        }
      if (GET_POS(ch) == POS_FIGHTING) 
      {
        if (number(0,150) < MIN(GET_LEVEL(ch), 60))
        {
          act ("&R$n looks chillingly at $N.&n",1,ch,0,vict,TO_ROOM);
          if (GET_INT(ch) + GET_CON(ch) < number (20, 50))
          {
            act ("&R$n freaks the heck out of $N who flees in terror!&n",1,ch,0,vict,TO_ROOM); 
            act ("&R$n freaks the heck out of You!&n",1,ch,0,vict,TO_VICT); 
            command_interpreter(vict, "flee");
            return TRUE;
          }
          else 
          {
            act ("&R$N holds up to $n's chilling stares!&n",1,ch,0,vict,TO_ROOM); 
            act ("&RYou hold up to $n's stares!&n",1,ch,0,vict,TO_VICT); 
            return TRUE;
          }
          
       }
      }
    }
    return FALSE;  
  }

/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(guild_guard)
{
  int i;
  struct char_data *guard = (struct char_data *) me;
  const char *buf = "The guard humiliates you, and blocks your way.\r\n";
  const char *buf2 = "The guard humiliates $n, and blocks $s way.";

  if (!IS_MOVE(cmd) || AFF_FLAGGED(guard, AFF_BLIND))
    return (FALSE);

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    return (FALSE);

  for (i = 0; guild_info[i][0] != -1; i++) {
    if ((IS_NPC(ch) || GET_CLASS(ch) != guild_info[i][0]) &&
	GET_ROOM_VNUM(IN_ROOM(ch)) == guild_info[i][1] &&
	cmd == guild_info[i][2]) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return (TRUE);
    }
  }

  return (FALSE);
}



SPECIAL(puff)
{
  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  default:
    return (0);
  }
}


#define NUM_BODY_PARTS 5

static char *headers[] =
{
  "the corpse of The ",
  "the corpse of the ",
  "the corpse of an ",
  "the corpse of An ",
  "the corpse of a ",
  "the corpse of A ",
  "the corpse of "
};

char *body_part[]=
{
  "an arm of",
  "a leg of",
  "a foot of",
  "a rib of",
  "a hand of",
  "\n"
};

char *body_word[]=
{
  "arm",
  "leg",
  "foot",
  "rib",
  "hand",
  "\n"
};

SPECIAL(butcher)
{
  struct obj_data *obj;
  char loc_buf[MAX_STRING_LENGTH]="";
  char loc_buf2[MAX_STRING_LENGTH]="";
  int i=0, len=0, total_len=0, done=FALSE, part=0;

  if(CMD_IS("butcher"))
    {
      argument=one_argument(argument,buf);
      if(strcmp(buf,"corpse")==0)
        {
          obj=get_obj_in_list_vis(ch,"corpse",ch->carrying);
          if(!obj)
            {
              act("$N says to you 'Sorry. You don't seem to have"
                  " any corpses.'",FALSE,ch,0,me,TO_CHAR);
              return TRUE;
            }

          if(GET_GOLD(ch)<500)
            {
              act("$N says to you 'Sorry. You don't have"
                  " enough gold.'",FALSE,ch,0,me,TO_CHAR);
              return TRUE;
            }
          else
            {
              part=number(1,NUM_BODY_PARTS)-1;
              for (i = 0; (i < 7) || (!done); i++)
                {
                  len=strlen(headers[i]);
                  if(memcmp(obj->short_description,headers[i],len)==0)
                    {

                      total_len=strlen(obj->short_description);

                      strncpy(loc_buf,obj->short_description+len,
                              total_len-len);

                      free(obj->name);
                      sprintf(loc_buf2,"%s %s",body_word[part],loc_buf);
                      obj->name=str_dup(loc_buf2);

                      sprintf(loc_buf2,"%s %s lies here.",
                              body_part[part],loc_buf);
                      free(obj->description);
                      obj->description=str_dup(loc_buf2);

                      sprintf(loc_buf2,"%s %s",body_part[part],loc_buf);
                      free(obj->short_description);
                      obj->short_description=str_dup(loc_buf2);

                      GET_OBJ_TYPE(obj)=ITEM_FOOD;
                      GET_OBJ_WEAR(obj)=ITEM_WEAR_TAKE;
                      GET_OBJ_VAL(obj,0)=5; /* adjust this for fill */
                      GET_OBJ_VAL(obj,1)=0;
                      GET_OBJ_VAL(obj,2)=0;
                      GET_OBJ_VAL(obj,3)=0; /* adjust this for poison */
                      GET_OBJ_WEIGHT(obj)=5;  /* adjust this for weight */
                      GET_OBJ_COST(obj)=500;
                      GET_OBJ_RENT(obj)=100;  /* shouldn't be rentable..
                                                 unless you make a change
                                                 to your mud to do so,
                                                 because corpses have a
                                                 vnum&rnum of -1 */
                      GET_OBJ_TIMER(obj)=0;

                      GET_GOLD(ch)-=1000;
                      done=TRUE;

                      act("$N says to you 'Done! Enjoy your meal!'",
                          FALSE,ch,0,me,TO_CHAR);
                      return 1;
                    }
                }
              act("$N says to you 'Sorry, I can't make cuts with"
                  "this.'",FALSE,ch,0,me,TO_CHAR);
              return 1;
            }
        }
      return 0; /* if it wasn't corpse... it wasn't meant for me... */
    }
  return 0;
}

SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (IS_CORPSE(i)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}

SPECIAL(bounty)
{
  struct char_data *victim;
  struct descriptor_data *i;

  if (CMD_IS("bounty")) {

    one_argument(argument, arg);

    if (!*arg) {
      send_to_char("Who is it that you wish to place a bounty on?\r\n", ch);
      return TRUE;
    }
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected && i->character && i->character != ch) {
        victim = i->character;
        if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
          send_to_char(NOPERSON, ch);
          return TRUE;
        }
        if (victim == ch) {
          send_to_char("You want to place a bounty on yourself? Nah.\r\n", ch);
          return TRUE;
        }
        if (GET_LEVEL(victim) >= LVL_IMMORT) {
          send_to_char("Let's just call that person 'priviledged', okay?\r\n", ch);
          return TRUE;
        }
        if (GET_LEVEL(victim) < 100 && GET_CLASS(victim) < 9) {
          send_to_char("You can't place bounties on players under level "
                       "100 unless they are remorts.\r\n", ch);
          return TRUE;
        }
        if (GET_GOLD(ch) < 550000) {
          send_to_char("You do not have the funds to place a bounty upon them.\r\n"
                       "The cost to place a bounty is 550,000 gold.\r\n", ch);
          return TRUE;
        }
        if (PLR_FLAGGED(victim, PLR_BOUNTY)) {
          send_to_char("That person already has a bounty on them.\r\n", ch);
          return TRUE;
        } else
          SET_BIT(PLR_FLAGS(victim), PLR_BOUNTY);
          GET_GOLD(ch) = GET_GOLD(ch) - 550000;
          send_to_char("You pay the 550,000 gold to place the bounty.\r\n", ch);
          send_to_char("A bounty has been placed upon your head - watch your "
                       "back!\r\n", victim);
          sprintf (buf, "\r\n/cRBOUNTY:: A bounty has been placed upon %s's "
                        "life!/c0\r\n", GET_NAME(victim));
          send_to_all(buf);
          return TRUE;
       }
     }
   }
   return FALSE;
}


SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return (TRUE);
  }

  return (FALSE);
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return (FALSE);

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && PLR_FLAGGED(tch, PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && PLR_FLAGGED(tch, PLR_THIEF)){
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}


#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)

SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  room_rnum pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    two_arguments(argument, buf, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= PET_PRICE(pet);

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);
    load_mtrigger(pet);


    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return (1);
  }
  /* All commands except list and buy */
  return (0);
}

SPECIAL(wiseman)
{
  struct obj_data *object = NULL;
  char buf[MAX_STRING_LENGTH];
  
  if (CMD_IS("whatis")) {
    argument = one_argument(argument, buf);

    if (GET_POS(ch) == POS_FIGHTING) return TRUE; 
    if (*buf) {
      object = get_obj_car(ch, buf);
      if (object) {
          act("$n does several strange gestures above that object.", FALSE, (struct char_data
*) me, 0, ch, TO_VICT);
          call_magic(ch, NULL, object, SPELL_IDENTIFY, 30, 0);
          act("$n looked at some object.", FALSE, (struct char_data *) me, 0, ch,
TO_NOTVICT);
          return TRUE;
        }
      else
        {
          act("$n shouts: 'Show me that thing or go away.'", FALSE, (struct char_data *) me,
0, ch, TO_VICT);
          return TRUE;
        }
      }
    else
      {
        act("$n asks you what do you want to know.", FALSE, (struct char_data *) me, 0, ch,
TO_VICT);
        return TRUE;
      }
  }
  return FALSE;
}


SPECIAL(meta_physician)
{

 char meta_type[256]= {"     "}; /* Array for meta type  name */
 long meta_gold = 0;
 int meta_maxh = 0;
 int meta_maxm = 0;
 int meta_maxv = 0;

 /* Parse command parm and set meta cost accordingly */

  if (CMD_IS("buy")) {
     argument = one_argument(argument, meta_type);
 
      if (strcmp(meta_type,"health")==0) {
        meta_gold = 40000;
       if (GET_CLASS(ch) == 0)
	meta_maxh = 400;
       if (GET_CLASS(ch) == 1)
	meta_maxh = 700; 
       if (GET_CLASS(ch) == 2)
	meta_maxh = 1200;
       if (GET_CLASS(ch) == 3)
	meta_maxh = 1500;
      }

      if (strcmp(meta_type, "mana")==0) {
        meta_gold = 40000;
      if (GET_CLASS(ch) == 0)
	meta_maxm = 1250;
      if (GET_CLASS(ch) == 1)
	meta_maxm = 850; 
      if (GET_CLASS(ch) == 2)
	meta_maxm = 400;
      if (GET_CLASS(ch) == 3)
	meta_maxm = 350;
      }

      if (strcmp(meta_type, "move")==0) {
        meta_gold = 30000;
      if (GET_CLASS(ch) == 0)
	meta_maxv = 300;
      if (GET_CLASS(ch) == 1)
	meta_maxv = 300; 
      if (GET_CLASS(ch) == 2)
	meta_maxv = 600;
      if (GET_CLASS(ch) == 3)
	meta_maxv = 300;
      }

      /* Gold and Exp validity check */
      if (meta_gold >= 30000) {
       if (GET_LEVEL(ch) < 60) {
          send_to_char("&RStan tells you, 'Come Back when you are more experienced, adventurer. (Level 60)'\r\n", ch);
          return (TRUE);
          }      
      if ((GET_GOLD(ch) < meta_gold)) {
          send_to_char("You don't have enough gold!\r\n", ch);
          return (TRUE);}

      /* Extract Cash and experience */
   
       GET_GOLD(ch) -= meta_gold;
       send_to_char("The MetaPhysician accepts your payment and begins the procedure... \r\n",ch);
    
      /* Boost Stats */       
   
       if (strcmp(meta_type,"health")==0) {
	if (GET_MAX_HIT(ch) <= meta_maxh) {
      GET_MAX_HIT(ch) += number(0, 5);
      send_to_char("Your vitality increases!\r\n",ch);
      return 1;} else {
	send_to_char("You have enough hit points already!\r\n", ch);}
            }

      if (strcmp(meta_type,"mana")==0) {
	if (GET_MAX_MANA(ch) <= meta_maxm) {
      GET_MAX_MANA(ch) += number(0, 5); 
      send_to_char("You feel a surge in magical power!\r\n",ch);
      return 1;} else {
	send_to_char("You have enough mana already!\r\n", ch);}
            }

     if (strcmp(meta_type,"move")==0) {
	if (GET_MAX_MOVE(ch) <= meta_maxv) {
      GET_MAX_MOVE(ch) += number(0, 5);
      send_to_char("You feel more strenght in your legs!\r\n",ch);
      return 1;} else {
	send_to_char("You have enough movement points already!\r\n", ch);}
            }
    }

     /* If it gets this far, show them the menu */
      send_to_char("\r\n", ch);
      send_to_char("Select an operation from the following...\r\n",ch);
      send_to_char("_____________________________ \r\n",ch);
      send_to_char("Operations        Gold \r\n",ch);
      send_to_char("_____________________________ \r\n",ch);
      send_to_char("Health            40k \r\n",ch);
      send_to_char("Mana              40k \r\n",ch);
      send_to_char("Move              30k \r\n",ch);   
      send_to_char("Banish            10mil\r\n",ch);
      send_to_char("------ \r\n",ch);
   
      return 1;}
    if(CMD_IS("banish"))
    {
      if(GET_GOLD(ch) < 10000000)
      { 
       send_to_char("You do not have enough gold to banish hunger and thirst.\r\n", ch);
       return TRUE;     
      }
      GET_COND(ch, FULL) = (char) -1;
      GET_COND(ch, THIRST) = (char) -1;
      GET_GOLD(ch)  -= 10000000;

      send_to_char("Your hunger and thirst are banished(Gold Taken).\r\n", ch);
      return TRUE;
    }

 return 0;
}

SPECIAL(assassin_guild)
{

  if (IS_NPC(ch)) {
    send_to_char("&WThe hell with you!&n\r\n", ch);
    return TRUE;
  }

  if (CMD_IS("whatis")) {
    send_to_char("&WYou are required to pay a fee of 1.5 million to join the assassin's guild!&n\r\n", ch);
  } else if (CMD_IS("join")) {

    if (PLR_FLAGGED(ch, PLR_ASSASSIN)) {
       send_to_char("&WYou are already a member!&n\r\n", ch);
       return (TRUE);
     }
     if (GET_GOLD(ch) < 1500000) {
       send_to_char("&WYou don't have enough coins, can't do this for free!&n\r\n", ch);
       return (TRUE);
     }
    if (GET_LEVEL(ch) >= LVL_IMMORT) { 
       send_to_char("&WImmortals can't join guilds!\r\n", ch);
       return (TRUE);
     }
    if (GET_LEVEL(ch) <= 50) { 
       send_to_char("&WYou should be at least level 50 before becoming an assassin!\r\n", ch);
       return (TRUE);
     }
     send_to_char("&wBeing an assassin is a very hard task, other assassins will most surely try to kill you, and if they do, they can take all the equipment that you have  worked so hard to get. If you are not ready to accept the loss of all of your equipment, you should not becomeone. If you still wish to become an assassin you should type DEATH.\r\n",ch);
    return TRUE;
}    
 else if (CMD_IS("death")) 
{

    if (PLR_FLAGGED(ch, PLR_ASSASSIN)) {
       send_to_char("&WYou are already a member!&n\r\n", ch);
       return (TRUE);
     }
     if (GET_GOLD(ch) < 1500000) {
       send_to_char("&WYou don't have enough coins, can't do this for free!&n\r\n", ch);
       return (TRUE);
     }
    if (GET_LEVEL(ch) >= LVL_IMMORT) { 
       send_to_char("&WImmortals can't join guilds!\r\n", ch);
       return (TRUE);
     }
    if (GET_LEVEL(ch) <= 50) { 
       send_to_char("&WYou should be at least level 50 before becoming an assassin!\r\n", ch);
       return (TRUE);
     }

     GET_GOLD(ch) -= 1500000;
     SET_BIT(PLR_FLAGS(ch), PLR_ASSASSIN);
     send_to_char("&WEnjoy your life as an assassin.&n\r\n", ch);
     return TRUE;
   }
   return 0;
 }

SPECIAL(embalmer)
{
  struct obj_data *obj, *temp;
  char loc_buf[MAX_STRING_LENGTH]="";
  char loc_buf2[MAX_STRING_LENGTH]="";
  int i=0, len=0, total_len=0, done=FALSE;

  if(CMD_IS("embalm"))
    {
      argument = one_argument(argument,buf);
      if(strcmp(buf,"corpse")==0)
        {
          temp=get_obj_in_list_vis(ch,"corpse",ch->carrying);
          obj = read_object(8099, VIRTUAL);

          if(!temp)
            {
              act("$N says to you 'Sorry. You don't seem to have"
                  " any corpses.'",FALSE,ch,0,me,TO_CHAR);
              return TRUE;
            }
          if(GET_GOLD(ch)<20000000)
            {
              act("$N says to you 'Sorry. You don't have"
                  " enough gold.'",FALSE,ch,0,me,TO_CHAR);
              return TRUE;
            }
          else
            {  
      for (i = 0; (i < 7) || (!done); i++)
           {
             len=strlen(headers[i]);
             if(memcmp(temp->short_description,headers[i],len)==0)
                {
                total_len=strlen(temp->short_description);
                strncpy(loc_buf,temp->short_description+len,
                       total_len-len);
                free(obj->name);
                sprintf(loc_buf2,"bag skin %s",loc_buf);
                obj->name=str_dup(loc_buf2);
                sprintf(loc_buf2,"A bag of %s skin lies"
                                 " here",loc_buf);
                free(obj->description);
                obj->description=str_dup(loc_buf2);
                sprintf(loc_buf2,"a bag made from %s skin",loc_buf);
                free(obj->short_description);
                obj->short_description=str_dup(loc_buf2);
                GET_OBJ_TYPE(obj)=ITEM_CONTAINER;
                GET_OBJ_WEAR(obj)=ITEM_WEAR_TAKE;
		SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE);
                GET_OBJ_VAL(obj,0)=100; /* adjust this for capacity */
                GET_OBJ_VAL(obj,3)=0; /* adjust this for capacity */
                GET_OBJ_WEIGHT(obj)=5;  /* adjust this for weight */
                GET_OBJ_COST(obj)=1000;
                GET_OBJ_TIMER(obj)=0;
                GET_GOLD(ch)-=100;
                done=TRUE;
                obj_to_char(obj, ch);
                extract_obj(temp);
                act("$N says to you 'Done! Enjoy your new bag.'",
                    FALSE,ch,0,me,TO_CHAR);
                return 1;
                }
            }
          act("$N says to you 'Sorry, I can't make a bag from"
              "this.'",FALSE,ch,0,me,TO_CHAR);
          return 1;
          }
       }
      return 0;
    }
  return 0;
 }

SPECIAL(gold_chipper)
{
 char train_type[256]= {"         "};
 struct obj_data *chip;
 obj_rnum r_chip = 11;

  if (CMD_IS("train")) {
     argument = one_argument(argument, train_type);
    
      /* Boost Stats */       
   if (!*train_type) {
     send_to_char("Try \"train <statname>\"\r\n", ch);
     return 1;
     }

   chip = read_object(r_chip, VIRTUAL);

   for (chip = ch->carrying; chip; chip = chip->next_content) {
   if (GET_OBJ_TYPE(chip) == ITEM_GOLDDISK) {
   
   if (isname(train_type,"strength")) {
     if (ch->real_abils.str < 18) {
      ch->real_abils.str += 1;
      send_to_char("The Wise Man takes your golden chip. \r\n",ch);
      send_to_char("Your strength increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 strength already!\r\n", ch);
      }
   } else if (isname(train_type,"wisdom")) {
     if (ch->real_abils.wis < 18) {
      ch->real_abils.wis += 1;
      send_to_char("The Wise Man takes your golden chip. \r\n",ch);
      send_to_char("Your wisdom increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 wisdom already!\r\n", ch);
      }
   } else if (isname(train_type,"intelligence")) {
     if (ch->real_abils.intel < 18) {
      ch->real_abils.intel += 1;
      send_to_char("The Wise Man takes your golden chip. \r\n",ch);
      send_to_char("Your intelligence increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 intelligence already!\r\n", ch);
      }
   } else if (isname(train_type,"dexterity")) {
     if (ch->real_abils.dex < 18) {
      ch->real_abils.dex += 1;
      send_to_char("The Wise Man takes your golden chip. \r\n",ch);
      send_to_char("Your dexterity increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 dexterity already!\r\n", ch);
      }
   } else if (isname(train_type,"constitution")) {
     if (ch->real_abils.con < 18) {
      ch->real_abils.con += 1;
      send_to_char("The Wise Man takes your golden chip. \r\n",ch);
      send_to_char("Your constitution increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 constitution already!\r\n", ch);
      }
   } else if (isname(train_type,"charisma")) {
     if (ch->real_abils.cha < 18) {
      ch->real_abils.cha += 1;
      send_to_char("Your charisma increases!\r\n",ch);
      extract_obj(chip);
      save_char(ch, NOWHERE);
      return 1;
    } else {
      send_to_char("You have 18 charisma already!\r\n", ch);
          }
        } 
     else {
     send_to_char("You have not specified the correct type of training.\r\n", ch);
     return 1;
    }
      }
    }
  }

  if (CMD_IS("list")) {
     /* If it gets this far, show them the menu */
      send_to_char("\r\n", ch);
      send_to_char("You may train yourself in one area\r\n",ch);
      send_to_char("___________________________________ \r\n",ch);
      send_to_char("Stats\r\n",ch);
      send_to_char("___________________________________ \r\n",ch);
      send_to_char("Strength\r\n",ch);
      send_to_char("Wisdom\r\n",ch);
      send_to_char("Intelligence\r\n",ch);
      send_to_char("Dexterity\r\n",ch);
      send_to_char("Constitution\r\n",ch);
      send_to_char("Charisma\r\n",ch);
      send_to_char("------ \r\n",ch); 
      return 1;
       }
   
    return 0; 
}

SPECIAL(calise_hunter) {

 int i = 0;

  //Mobs get affected by trip.
  if (GET_MOB_WAIT(ch) > 0)
    return (FALSE);

 if (FIGHTING(ch)) {
   i = number(1,5);
   if (i == 3) {
   send_to_char("You have been targeted my friend\r\n", FIGHTING(ch));
   i = number(1,3);
  switch(i) {
   case 1:
    send_to_char("The hunter has aimed at your feet and fires!\r\n", FIGHTING(ch));
    GET_HIT(FIGHTING(ch)) = GET_HIT(ch)/2;
    GET_MOVE(FIGHTING(ch)) = 0;
    break;
   case 2:
    send_to_char("The hunter has aimed at your heart and fires!\r\n", FIGHTING(ch));
    GET_HIT(FIGHTING(ch)) = GET_HIT(FIGHTING(ch))/3;
    break;
   case 3:
    send_to_char("The hunter has aimed between your eyes and fires!\r\n", FIGHTING(ch));
    GET_HIT(FIGHTING(ch)) = 1;
    break;
   default:
    send_to_char("You shouldn't see this, please report!\r\n", FIGHTING(ch));
    break;
      }
    return (TRUE);

    }
  }
  return (FALSE);
}

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */

#define XFER_TAX              	1000
#define MAX_XFER		999999999
#define MIN_XFER		1000

SPECIAL(bank)
{
  char arg[256]= {"    "}; /* Array for meta type  name */
  int amount;
  int temp;
  struct char_data *victim = NULL;
  int player_i = 0;
  struct char_data * vict;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "&WYour current balance is&n &Y%d&n &Wcoins.&n\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "&WYou currently have no money deposited.&n\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    
    argument = one_argument(argument, arg);
    /*send_to_char(argument, ch);
    sprintf(buf,"%d",str_cmp(argument,"all")==0);
    send_to_char(buf,ch);*/
    if (str_cmp(arg,"all")==0)
    {
        amount = GET_GOLD(ch);
    }
    else if ((amount = atoi(arg)) <= 0) 
    {
      send_to_char("&WHow much do you want to deposit?&n\r\n", ch);
      return 1;
    }

    if (GET_GOLD(ch) < amount) 
    {
      send_to_char("&WYou don't have that many coins!&n\r\n", ch);
      return 1;
    }

    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "&WYou deposit&n &Y%d&n &Wcoins.&n\r\n", amount);
    send_to_char(buf, ch);
    act("&W$n makes a bank transaction.&n", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;

  } else if (CMD_IS("withdraw")) {
    argument = one_argument(argument, arg);
    if (str_cmp(arg,"all")==0)
    {
	amount = GET_BANK_GOLD(ch);
    }

    else if ((amount = atoi(arg)) <= 0) {
      send_to_char("&WHow much do you want to withdraw?&n\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("&WYou don't have that many coins deposited!&n\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "&WYou withdraw&n &Y%d&n &Wcoins.&n\r\n", amount);
    send_to_char(buf, ch);
    act("&W$n makes a bank transaction.&n", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("bankxfer")) {
    two_arguments(argument, buf1, buf2);
    if ((!buf1) || (!*buf1) || (amount = atoi(buf1)) <= 0) {
      send_to_char("&WHow much do you want to transfer?&n\r\n", ch);
      return 1;
    }
    if ((!buf2) || (!*buf2)) {
      send_to_char("&WUm, and who is the recipient?&n\r\n", ch);
      return 1;
    }
    if (amount > MAX_XFER) {
      send_to_char("&WBank cannot transfer such huge amounts of gold!&n\r\n", ch);
      return 1;
    }
    
    if (amount < MIN_XFER) {
      send_to_char("&WBank will not transfer little amounts of money!&n\r\n", ch);
      return 1;
    }
    /* I must to do it so strange because of overflowing */
    temp = GET_BANK_GOLD(ch);
    temp -= amount;
    //temp -=XFER_TAX;
    if (temp < 0) {
      send_to_char("&WYou don't have that many coins deposited!&n\r\n", ch);
      return 1;
    }

    /*get victim */
   
    if ((vict = get_char_vis(ch, buf2, FIND_CHAR_WORLD)) && !IS_NPC(vict)) 
    {
      GET_BANK_GOLD(ch) = temp;
      GET_BANK_GOLD(vict) += amount;
      strcpy(buf2, vict->player.name);
      sprintf(buf, "&r-&R=&M#&R=&r-&n &WFirst National Bank of DeimosMUD &r-&R=&M#&R=&r-&n\r\n\r\n");   
      sprintf(buf, "%s&WTransfered&n &Y%d&n &Wcoins to&n &c%s's&n &Waccount.&n\r\n", buf, amount, CAP(buf2));
      send_to_char(buf, ch);
      strcpy(buf2, ch->player.name);
      sprintf(buf, "&r-&R=&M#&R=&r-&n &WFirst National Bank of DeimosMUD &r-&R=&M#&R=&r-&n\r\n\r\n");
      sprintf(buf, "%s&W%s transfered&n &Y%d&n &Wcoins to your account.&n\r\n\r\n", buf, CAP(buf2), amount);
      send_to_char(buf, vict);
      act("&W$n makes a bank transaction.&n", TRUE, ch, 0, FALSE, TO_ROOM);
    } else {
          CREATE(victim, struct char_data, 1);
          clear_char(victim);
          if ((player_i = load_char(buf2, victim)) > -1)
          {
             vict = victim;
          }
          else
          {
            free(victim);
            send_to_char("There is no such player.\r\n", ch);
            return 1;
          }
      GET_BANK_GOLD(ch) = temp;
      GET_BANK_GOLD(vict) += amount;
      strcpy(buf2, vict->player.name);
      sprintf(buf, "&r-&R=&M#&R=&r-&n &WFirst National Bank of DeimosMUD &r-&R=&M#&R=&r-&n\r\n\r\n");   
      sprintf(buf, "%s&WTransfered&n &Y%d&n &Wcoins to&n &c%s's&n &Waccount.&n\r\n", buf, amount, CAP(buf2));
      send_to_char(buf, ch);

      strcpy(buf2, ch->player.name);
      sprintf(buf, "&r-&R=&M#&R=&r-&n &WFirst National Bank of DeimosMUD &r-&R=&M#&R=&r-&n\r\n\r\n");
      sprintf(buf, "%s&W%s transfered&n &Y%d&n &Wcoins to your account.&n\r\n\r\n", buf, CAP(buf2), amount);

      store_mail(GET_IDNUM(vict), GET_IDNUM(ch), NOTHING, strdup(buf));


      act("&W$n makes a bank transaction.&n", TRUE, ch, 0, FALSE, TO_ROOM);
      GET_PFILEPOS(victim) = player_i;
      save_char(victim, GET_LOADROOM(victim));
      free_char(victim);
    }
    return 1;
  } else
    return 0;
  return 0;
}

                                                                               
SPECIAL(recharger)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int maxcharge = 0, mincharge = 0, chargeval = 0;

  if (CMD_IS("list"))
  {
    send_to_char("You may use the SWEET machine to recharge a staff or wand.\r\
n", ch);
    send_to_char("It costs 10000 coins to recharge a staff or wand.\r\n", ch);
    send_to_char("To recharge and item type: 'recharge <staff or wand>'.\r\n",
ch);
    return (TRUE);
  } else
    if (CMD_IS("recharge")) {
    argument = one_argument(argument, buf);

    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
      send_to_char("You don't have that!\r\n", ch);
      return (TRUE);
    }
    if (GET_OBJ_TYPE(obj) != ITEM_STAFF &&
        GET_OBJ_TYPE(obj) != ITEM_WAND)
    {
      send_to_char("Are you daft!  You can't recharge that!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < 1000) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    maxcharge = GET_OBJ_VAL(obj, 1);
    mincharge = GET_OBJ_VAL(obj, 2);

    if (mincharge < maxcharge)
    {
     chargeval = maxcharge - mincharge;
     GET_OBJ_VAL(obj, 2) += chargeval;
     GET_GOLD(ch) -= 1000;
     send_to_char("Grrrr... Hmmmmm... Belch... BING!\r\n",ch);
     sprintf(buf, "The item now has %d charges remaining.\r\n", maxcharge);
     send_to_char(buf, ch);
     act("The machine hums and churns as it recharges the item.",
          FALSE, ch, obj, 0, TO_ROOM);
     }
   else
     {
     send_to_char("The item does not need recharging.\r\n", ch);
     act("The machine hums, churns, and then goes quiet.",
         FALSE, ch, obj, 0, TO_ROOM);
     }
    return 1;
  }
  return 0;
}

SPECIAL(pop_dispenser)
{
 struct obj_data *obj = me, *drink;
 int give_coke = 8002; /* Vnum of the can of coke */

 if (CMD_IS("list"))
     {
     send_to_char("To buy a pop, type 'buy pop'.\r\n", ch);
     return (TRUE);
     }
 else if (CMD_IS("buy")) {
          if (GET_GOLD(ch) < 2) {
          send_to_char("You don't have enough gold!\r\n", ch);
          return (TRUE);
          } else {
            drink = read_object(give_coke, VIRTUAL);
            obj_to_char(drink, ch);
            send_to_char("You insert your money into the machine\r\n",ch);
            GET_GOLD(ch) -= 2; /* coke costs 25 gold */
            act("$n gets a pop can from $p.", FALSE, ch, obj, 0, TO_ROOM);
            send_to_char("You get a pop can from the machine.\r\n",ch);
          }
          return 1;
 }
 return 0;
}

SPECIAL(newspaper_stand) {

 if(CMD_IS("look"))
  {
   argument = one_argument(argument, buf);  
   if(((strcmp(buf,"newspaper") == 0) ||
     (strcmp(buf,"newspape") == 0) ||
     (strcmp(buf,"newspap") == 0) ||
     (strcmp(buf,"newspa") == 0) ||
     (strcmp(buf,"newsp") == 0) ||
     (strcmp(buf,"news") == 0) ||
     (strcmp(buf,"new") == 0) ||
     (strcmp(buf,"ne") == 0) ||
     (strcmp(buf,"n") == 0)) &&
     !PLR_FLAGGED2(ch, PLR2_WRITER))
   {
//   newspaper = read_object(vnum, VIRTUAL);
   send_to_char("Hey, no free peeks on the new edition!\r\n", ch);
   return TRUE;
    }
  }
 return FALSE;
}

SPECIAL(gm_plaque) {
  struct obj_data *obj = me;
  if(CMD_IS("look"))
  {
    argument = one_argument(argument, buf);  
    if(isname(buf, obj->name)) {
      page_string(ch->desc, gms, 0);
      return TRUE;
    }
  }
 return FALSE;
}

/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

/* Horse Buying and Codes */

void set_horsename(struct char_data *ch, char *horsename) {

  if (strlen(horsename) > 256)
    horsename[256] = '\0';
 
  if (GET_HORSENAME(ch) != NULL)
    free(GET_HORSENAME(ch));
 
  GET_HORSENAME(ch) = str_dup(horsename);
 }

SPECIAL(stableshop) {
  char horsename[256];

  skip_spaces(&argument);
  delete_doubledollar(argument);

 if (CMD_IS("list"))
     {
     send_to_char("To buy a horse, type 'buy <name of horse>'.\r\n", ch);
     send_to_char("A Baby Horse                     100000000.\r\n", ch);
     send_to_char("Special items avaliable to you at this time:\r\n", ch);
  if (GET_HORSEEQ(ch) >= 0) 
     send_to_char("1) A pair of Kraken gills         50000000.\r\n", ch);     
  if (GET_HORSEEQ(ch) >= 1) 
     send_to_char("2) A wings of a pegasus           75000000.\r\n", ch);
  if (GET_HORSEEQ(ch) >= 2) 
     send_to_char("3) A titanium war helmet         100000000.\r\n", ch);
  if (GET_HORSEEQ(ch) >= 3) 
     send_to_char("4) The horseshoes of war         125000000.\r\n", ch);
  if (GET_HORSEEQ(ch) >= 4) 
     send_to_char("5) Metallic skin                 200000000.\r\n", ch);
  if (GET_HORSEEQ(ch) >= 5) 
     send_to_char("6) The Grace of Deimos           500000000.\r\n", ch);
     return (TRUE);
     }

   else if (CMD_IS("upgrade")) {
     send_to_char("The stableman takes in your horse for new equipment.\r\n", ch);

  if (GET_HORSEEQ(ch) == 0) {
   if (GET_GOLD(ch) >= 5000000) { 
     GET_HORSEEQ(ch) = 1;
     GET_GOLD(ch) -= 5000000;
     send_to_char("You have given your horse kraken gills!\r\n", ch);
     return(TRUE);
  } else {
  send_to_char("You don't have enough money!\r\n", ch);
  return (FALSE);
   }
 }
  if (GET_HORSEEQ(ch) == 1) { 
   if (GET_GOLD(ch) >= 7500000) { 
     GET_HORSEEQ(ch) = 2;
     GET_GOLD(ch) -= 7500000;
     send_to_char("You have given your horse pegasus wings!\r\n", ch);
     return(TRUE);
  } else {
     send_to_char("You don't have enough money!\r\n", ch);
     return(FALSE);
   }
 }

  if (GET_HORSEEQ(ch) == 2) { 
   if (GET_GOLD(ch) >= 10000000) { 
     GET_HORSEEQ(ch) = 3;
     GET_GOLD(ch) -= 10000000;
     send_to_char("You have given your horse a war helmet!\r\n", ch);
     return(TRUE);
  } else {
  send_to_char("You don't have enough money!\r\n", ch);
  return(TRUE);
   }
 }

  if (GET_HORSEEQ(ch) == 3) {
   if (GET_GOLD(ch) >= 12500000) { 
     GET_HORSEEQ(ch) = 4;
     GET_GOLD(ch) -= 12500000;
     send_to_char("You have given your horse horseshoes of war!\r\n", ch);
     return(TRUE);
  } else {
  send_to_char("You don't have enough money!\r\n", ch);
  return(TRUE);
   }
 }

  if (GET_HORSEEQ(ch) == 4) {
   if (GET_GOLD(ch) >= 20000000) { 
     GET_HORSEEQ(ch) = 5;
     GET_GOLD(ch) -= 20000000;
     send_to_char("You have given your horse metallic skin!\r\n", ch);
     return(TRUE);
  } else {
  send_to_char("You don't have enough money!\r\n", ch);
  return(TRUE);
   }
 }

  if (GET_HORSEEQ(ch) == 5) { 
   if (GET_GOLD(ch) >= 30000000) { 
     GET_HORSEEQ(ch) = 6;
     GET_GOLD(ch) -= 30000000;
     send_to_char("You have given your horse the Grace of Deimos!\r\n", ch);
     return(TRUE);
  } else {
  send_to_char("You don't have enough money!\r\n", ch);
  return(TRUE);
    }
  } 
}  

   else if (CMD_IS("change"))
     {
    if (!(*argument)) {
       send_to_char("You need to give your horse a name.\r\n", ch);
       return (TRUE);
       } 
    if (!PLR_FLAGGED(ch, PLR_HASHORSE)) {
       send_to_char("You need a horse first.\r\n", ch);
       return (TRUE);
      } else {
         act("$n changes his horse's name!", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char("You have changed the name of your horse!\r\n",ch);
	 set_horsename(ch, argument);     
         return (TRUE);
       }
     }

  else if (CMD_IS("buy")) {
     one_argument(argument, horsename);
      if (!(*argument)) {
       send_to_char("You need to give your horse a name.\r\n", ch);
       return (TRUE);
       }
      if (PLR_FLAGGED(ch, PLR_HASHORSE)) {
       send_to_char("You are not allowed to have another horse.\r\n", ch);
       return (TRUE);
       }
      if (GET_LEVEL(ch) < 60) {
       send_to_char("You are not allowed to have a horse at this time.\r\n", ch);
       return (TRUE);
       }

      if (GET_GOLD(ch) < 10000000) {
       send_to_char("You don't have enough gold!\r\n", ch);
       return (TRUE);
       } else {
         send_to_char("You give 100 million coins to the stable keeper\r\n",ch);
         GET_GOLD(ch) -= 1000000;
         act("$n buys a new horse!", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char("You now are a proud owner of a new horse!\r\n",ch);
         SET_BIT(PLR_FLAGS(ch), PLR_HASHORSE);
	 set_horsename(ch, argument);     
         GET_MAXHAPPY(ch) = 50;
         GET_HORSEROOM(ch) = real_room(255);
	 GET_HORSELEVEL(ch) = 1;
	 GET_HORSEEXP(ch) = 0;
         return (TRUE);
       }
         return 1;
    }   
 return 0;
}

SPECIAL(stable) {
char care_type[256]= {"     "}; 
int gold_stuff = 0;
int temp = 0;

 if (CMD_IS("list"))
     {
     send_to_char("Welcome to the Phobos Horse Trainer.\r\n", ch);
     send_to_char("Services being offered\r\n", ch);
     send_to_char("*Deluxe* Care            100000.\r\n", ch);
     send_to_char("*Standard* Care           50000.\r\n", ch);
     send_to_char("*Minimum* Care            25000.\r\n", ch);
     return (TRUE);
     }

 else if (CMD_IS("gain"))
  {
   if (!PLR_FLAGGED(ch, PLR_HASHORSE)) 
     {
     send_to_char("You do not have a horse you moron.\r\n", ch);
     return (TRUE);
     }
   if (GET_HORSELEVEL(ch) >= 10) 
     {
     send_to_char("Your horse has already reached it's max level. \r\n", ch);
     return (TRUE);
     }

   if (GET_HORSELEVEL(ch) <= 9) 
     {
    if (GET_HORSEEXP(ch) >= (horse_exp(GET_HORSELEVEL(ch))))
      {
     GET_HORSEEXP(ch) -= horse_exp(GET_HORSELEVEL(ch));
     GET_HORSELEVEL(ch)++;
     temp = number(10, 15);
     GET_MAXHAPPY(ch) += temp;
     send_to_char("Your horse's level has gone up one!\r\n", ch);
     sprintf(buf, "Your horse has gained %d energy!\r\n", temp);
     send_to_char(buf, ch);
     return 1;
      } 
     else 
     {
     send_to_char("You need some more exp in order to level.\r\n", ch);
     return 1;
     }
   }
 }        


  else if (CMD_IS("buy")) 
     {

     argument = one_argument(argument, care_type);

      if (!(*care_type)) {
       send_to_char("Please choose something...\r\n", ch);
       return (TRUE);
       }

      if (!PLR_FLAGGED(ch, PLR_HASHORSE)) {
       send_to_char("You do not have a horse you moron.\r\n", ch);
       return (TRUE);
       }

      if (PLR_FLAGGED(ch, PLR_FEDHORSE)) {
       send_to_char("You have already cared for your horse today.\r\n", ch);
       return (TRUE);
      }

      if (strcmp(care_type,"deluxe")==0) {
        gold_stuff = 10000;
      }

      if (strcmp(care_type,"standard")==0) {
        gold_stuff = 5000;
      }

      if (strcmp(care_type,"minimum")==0) {
        gold_stuff = 2500;
      }

       if (GET_GOLD(ch) < gold_stuff) {
       send_to_char("You do not have enough gold!\r\n", ch);
       return (TRUE);
       } else {
         GET_GOLD(ch) -= gold_stuff;
         send_to_char("You buy the care package.\r\n",ch);
       }

      if (strcmp(care_type,"deluxe")==0) {
      GET_MAXHAPPY(ch) += 3;
      send_to_char("The stableman feeds, brushes, and lets it go outside.\r\n",ch);
      SET_BIT(PLR_FLAGS(ch), PLR_FEDHORSE);
      return 1;
      } 

      if (strcmp(care_type,"standard")==0) {
      GET_MAXHAPPY(ch) += 2;
      send_to_char("The stableman feed the horse and brushes it.\r\n",ch);
      SET_BIT(PLR_FLAGS(ch), PLR_FEDHORSE);
      return 1; 
         }

     if (strcmp(care_type,"minimum")==0) {
      GET_MAXHAPPY(ch) += 1;
      send_to_char("The stableman throws some food at the horse.\r\n", ch);
      SET_BIT(PLR_FLAGS(ch), PLR_FEDHORSE);
      return 1;
       }
      return 1;
    } 
 else if (CMD_IS("ride"))
     { 
  if (!PLR_FLAGGED(ch, PLR_HASHORSE)) {
    send_to_char("You do not have a horse you moron.\r\n", ch);
    return (TRUE);
       }
  if (GET_HORSEROOM(ch) != real_room(255)) {
    send_to_char("Your horse is not presently in this room.\r\n", ch);
    return (TRUE);
       }
    GET_HORSEHAPPY(ch) = GET_MAXHAPPY(ch);    
    send_to_char("You jump on your horse...booyah.\r\n", ch);
    SET_BIT(AFF_FLAGS(ch), AFF_MOUNTED);
    GET_HORSEROOM(ch) = real_room(2);
    return 1;
   }
 return 0;
 }


SPECIAL(gmaster) {
 if (CMD_IS("east") && GET_THIEF_LEVEL(ch) >= 60 && GET_CLERIC_LEVEL(ch) >= 60 && GET_MAGE_LEVEL(ch) >= 60 && GET_WARRIOR_LEVEL(ch) >= 60 )
 {
  char_from_room(ch);
  char_to_room(ch, real_room(1512));
  send_to_char("&GYou walk through the wall into the grandmaster's hangout.&n\r\n", ch);
  do_look(ch, "", 15, 0);
  return 1;
 }
 return 0;
}


SPECIAL(cussgiver) {

  if (CMD_IS("apply")) {
    if (GET_GOLD(ch) < 500000) {
        send_to_char("You Don't have enough money to get the cuss channel, you bastard.\r\n", ch);
        return 1;
      } else if (GET_LEVEL(ch) >= LVL_IMMORT) {
        send_to_char("Immortals don't need to apply to cuss....moron.\r\n", ch);
        return 1;
      } else { 
        send_to_char("Here bitch, you can cuss now.\r\n", ch);
        GET_GOLD(ch) -= 500000;
        SET_BIT(PLR_FLAGS(ch), PLR_CUSSER);
        return 1;
       }  
    }
  return 0;
}


SPECIAL(casino) 
 {
 	if (CMD_IS("pull"))
         {
         int A; int B; int C;	 
	 if (GET_GOLD(ch) < 100)	
	  {
          send_to_char("You don't have enough money to play the slot machine.\r\n", ch);
          return 1;
          }
	 else
	  {
          GET_GOLD(ch) -= 100;
	  A = number(1, 7);
          B = number(1, 7);
	  C = number(1, 7);
          sprintf(buf, "| %d | %d | %d | (- 100)\r\n", A, B, C);
          send_to_char(buf, ch);
	  if (A == B && B == C) 
	   {
           send_to_char("JackPot! (+ 1000) \r\n", ch);
           GET_GOLD(ch) += 1000;
	   }
	  else if (A == B || A == C) 
	   {
           send_to_char("Double! (+ 500) \r\n", ch);
           GET_GOLD(ch) += 500;
	   }
	  else if (B == A || B == C) 
	   {
           send_to_char("Double! (+ 250) \r\n", ch);
           GET_GOLD(ch) += 250;
	   }
	  else if (A == B -1 && A == C -2) 
	   {
           send_to_char("Front Straight! (+ 500) \r\n", ch);
           GET_GOLD(ch) += 500;
	   }
	  else if (C == A -2 && C == B-1 ) 
	   {
           send_to_char("Back Straight! (+ 500) \r\n", ch);
           GET_GOLD(ch) += 500;
	   }
          else
           {       
           send_to_char("Nothing!! (+ 0) \r\n", ch);
           }
          }
          return 1;
	 }
    return 0;
}

SPECIAL(rental) {
 char filename[MAX_INPUT_LENGTH];
 char backname[MAX_INPUT_LENGTH];
 char buf[MAX_INPUT_LENGTH];
 int numitems = IS_CARRYING_N(ch);
 int max_obj_save = 30;
 int pfilepos = GET_PFILEPOS(ch);

 if (CMD_IS("rent")) 
 {

   if (PLR_FLAGGED2(ch, PLR2_NORENT))
   {
     send_to_char("&RYou arn't allowed to rent!&n\r\n", ch);
     return 1;
   }

 if (IS_NPC(ch))
   { return 1; } 

 if (!*player_table[pfilepos].name)
   return 1;

  if (numitems > max_obj_save) 
  {
    sprintf(buf, "He tells you, 'Sorry, but I cannot store more than %d items.'\r\n",
            max_obj_save);
    send_to_char(buf, ch);
    return 1;
  }

  if (GET_LEVEL(ch) < 15) 
  {
   send_to_char("You must be at least level 15 in order to rent!\r\n", ch);
   return 1;
  }

  Crash_crashsave(ch);
  get_filename(GET_NAME(ch), filename, NEW_OBJ_FILES);
  get_filename(GET_NAME(ch), backname, BACKUP_FILES);
 
  sprintf(buf, "cp %s %s", filename, backname); 
  system(buf);
  sprintf(buf, "cp %s/%c/%s%s %s/%c/%s.back", PLR_PREFIX, *player_table[pfilepos].name, 
    player_table[pfilepos].name, PLR_SUFFIX, PLR_PREFIX, *player_table[pfilepos].name, 
    player_table[pfilepos].name);
  system(buf);
  send_to_char("You have been rented!\r\n", ch);
  return 1;
  }
  return 0;
}

SPECIAL(warehouse) {
  struct obj_data *obj, *object;
  obj_vnum one = 2558;
  obj_vnum two = 2569;
  char buy_type[256]= {"     "};
  //char *buy_type = NULL;
  int buy_amount = 0, buy_num = 0, buytype = 0;

  static int stone = 0, fish = 0, wood = 0, animal = 0;

 if (CMD_IS("drop")) {
   
   one_argument(argument, arg);

   if (!*arg) {
    send_to_char("Drop Something?", ch);
    return 1;
   }

   if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return 1;
   }

   if ((GET_OBJ_VNUM(obj) >= one && GET_OBJ_VNUM(obj) <= two) 
        || GET_OBJ_VNUM(obj) == 2599) {
    switch (GET_OBJ_VNUM(obj))
    {
     case 2599: fish++; break;
     case 2559: wood++; break;
     case 2560: stone++; break;
     case 2561: animal++; break;
     case 2562: animal += 2; break;
     case 2563: animal += 3; break;
     case 2564: animal += 4; break;
     case 2565: animal += 5; break;
     case 2566: animal += 6; break;
    }

    sprintf(buf, "You have gained %d gold.\r\n", GET_OBJ_COST(obj));
    send_to_char(buf, ch);
    GET_GOLD(ch) +=  GET_OBJ_COST(obj) + GET_ECONRANK(ch)/100;
    GET_ECONRANK(ch) += MAX(GET_OBJ_COST(obj)/1000, 0);
    if (GET_LEVEL(ch) < LVL_IMMORT)
       sort_econrank(ch);
    extract_obj(obj);
    return 1;
  } else {
    send_to_char("This is not an allowable item to be housed in this warehouse\r\n", ch);
    return 1;
    }
  }
  if (CMD_IS("stock"))
  {
    send_to_char("Current Contents of the Warehouse:\r\n", ch);
    sprintf(buf, " Wood: %d \n Fish: %d \n Stone: %d \n Animal: %d \r\n",
            wood, fish, stone, animal);
    send_to_char(buf, ch);
    return 1;
  }
  if (CMD_IS("buy")) {
     argument = one_argument(argument, buy_type);

      if (strcmp(buy_type, "fish") == 0) 
      {
        buy_num = 2570;
        buytype = 0;
        buy_amount = 800;
      }
      else if (strcmp(buy_type, "stone") == 0) 
      {
        buy_num = 2571;
        buytype = 1;
        buy_amount = 550;
      }
      else if (strcmp(buy_type, "wood") == 0) 
      {
        buy_num = 2572;
        buytype = 2;
        buy_amount = 300;
      }
      else if (strcmp(buy_type, "animal") == 0) 
      {
        buy_num = 2573;
        buytype = 3;
        buy_amount = 400;
      }
      if (GET_GOLD(ch) < buy_amount)
      {
        send_to_char("Not enough money!\r\n", ch);
        return 1; 
      }
      if (buy_num == 0)
        return 1;

      switch (buytype)
      {
      case 0:
        if (fish < 1)
        {
          send_to_char("Not enough raw materials!\r\n", ch);
          return 1;
        }
        fish--;
        object = read_object(buy_num, VIRTUAL);
        obj_to_char(object, ch);
      break;
      case 1:
        if (stone < 1)
        {
          send_to_char("Not enough raw materials!\r\n", ch);
          return 1;
        }
        stone--;
        object = read_object(buy_num, VIRTUAL);
        obj_to_char(object, ch);
      break;
      case 2:
        if (wood < 1)
        {
          send_to_char("Not enough raw materials!\r\n", ch);
          return 1;
        }
        wood--;
        object = read_object(buy_num, VIRTUAL);
        obj_to_char(object, ch);
      break;
      case 3:
        if (animal < 1)
        {
          send_to_char("Not enough raw materials!\r\n", ch);
          return 1;
        }
        animal--;
        object = read_object(buy_num, VIRTUAL);
        obj_to_char(object, ch);
      break;
     }
     send_to_char("You buy the materials.\r\n", ch);

     GET_GOLD(ch) -= buy_amount;
     return 1;
  }
  return 0;
}

SPECIAL(cestadeath) {
 if (CMD_IS("down")) 
 {
  char_from_room(ch);
  char_to_room(ch, real_room(200));
  send_to_char("&RYou fall down into Phobos spearing yourself on the center of the fountain.&n\r", ch);
  act("$n falls from the sky and is speared by the fountain of Phobos!\r\n", FALSE, ch, 0, ch, TO_NOTVICT);
  GET_HIT(ch) = -100;	
  if (GET_LEVEL(ch) < LVL_IMMORT) 
  {  
     GET_LOADROOM(ch) = NOWHERE;
     update_pos(ch);
  }
  return 1;
 }
 return 0;
}

SPECIAL(firestation) {

 room_rnum rnum;
 obj_vnum fire_num = 2580;
 struct obj_data *fire;

  if (CMD_IS("apply")) {
    if (PLR_FLAGGED2(ch, PLR2_FFIGHTING)) {
        send_to_char("You're working already.\r\n", ch); 
        return 1;
      } else {
        send_to_char("The Station Head says, 'Alright, we'll give you a try'\r\n", ch);
        SET_BIT(PLR_FLAGS2(ch), PLR2_FFIGHTING);
        SET_BIT(PLR_FLAGS2(ch), TIMER_ONE);
        rnum = real_room(number(200, 299));
        fire = read_object(fire_num, VIRTUAL);
        obj_to_room(fire, rnum);
        sprintf(buf, "Your next mission is at %s, hurry!\r\n", world[rnum].name);
        send_to_char(buf, ch);
        return 1;
       }
    }
  return 0;
}


SPECIAL(maiden) {
  struct char_data *vict;

 if (cmd) {
  return (FALSE);}


 while (time_info.hours >= 0) {
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
  
  /* Hm...didn't pick anyone...I'll wait. */
  if (vict == NULL)
    return(TRUE);

  /* Uhh....no... */
  if (vict == ch)
    return(TRUE);


  /* Make sure mobs don't cast every so often. */
 if (1 == number(1,3)) {

  /* Intelligent Mobs */

  if (GET_POS(vict) < POS_INCAP) { 
      cast_spell(ch, vict, NULL, SPELL_RESURRECTION);
      continue;
  }
  if (AFF_FLAGGED(vict, AFF_POISON)) { 
       cast_spell(ch, vict, NULL, SPELL_REMOVE_POISON);
       continue;
  }
  if (AFF_FLAGGED(vict, AFF_CURSE)) {
       cast_spell(ch, vict, NULL, SPELL_REMOVE_CURSE);
       continue;
  }
  if (AFF_FLAGGED(vict, AFF_BLIND)) {
       cast_spell(ch, vict, NULL, SPELL_CURE_BLIND); 
       continue;
  }
  if (GET_TOTAL_LEVEL(vict) <= 40 && !affected_by_spell(vict, SPELL_ARMOR)) {
       cast_spell(ch, vict, NULL, SPELL_ARMOR); 
       continue;
  }
  if (GET_TOTAL_LEVEL(vict) <= 30 && !affected_by_spell(vict, SPELL_STRENGTH)) {
       cast_spell(ch, vict, NULL, SPELL_STRENGTH); 
       continue;
  }

  /* Heal Crap */
  if (GET_TOTAL_LEVEL(vict) <= 20) {
    if (GET_HIT(vict) < GET_MAX_HIT(vict) - 50) 
       cast_spell(ch, vict, NULL, SPELL_HEAL);
  } else if (GET_TOTAL_LEVEL(vict) <= 40) {
    if (GET_HIT(vict) < GET_MAX_HIT(vict) - 200) 
       cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC);
  } else {
    if (GET_HIT(vict) < GET_MAX_HIT(vict) - 500) 
       cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT);
       }
    return (TRUE);
       }
    }
   return (FALSE);
  }
   return (FALSE);
} 

SPECIAL(block_north) {
  struct char_data *vict = (struct char_data *) me;
  if (IS_NPC(ch)) return FALSE;
  if (AFF_FLAGGED(vict, AFF_SLEEP)) return FALSE;
  if (GET_POS(vict) < POS_SLEEPING) return FALSE;
  if (AFF_FLAGGED(vict, AFF_BLIND)) return FALSE;
  if (GET_LEVEL(ch) >= LVL_IMMUNE) return FALSE;
  if (CMD_IS("north")) {
        send_to_char("You can't go that way... Yet.", ch);
        return TRUE;
  }
  return FALSE;
}
SPECIAL(block_east) {
  struct char_data *vict = (struct char_data *) me;
  if (IS_NPC(ch)) return FALSE;
  if (GET_POS(vict) < POS_SLEEPING) return FALSE;
  if (AFF_FLAGGED(vict, AFF_SLEEP)) return FALSE;
  if (AFF_FLAGGED(vict, AFF_BLIND)) return FALSE;
  if (GET_LEVEL(ch) >= LVL_IMMUNE) return FALSE;
  if (CMD_IS("east")) {
        send_to_char("You can't go that way... Yet.", ch);
        return TRUE;
  }
  return FALSE;
}
SPECIAL(block_south) {
  struct char_data *vict = (struct char_data *) me;
  if (IS_NPC(ch)) return FALSE;
  if (GET_POS(vict) < POS_SLEEPING) return FALSE;
  if (AFF_FLAGGED(vict, AFF_SLEEP)) return FALSE;
  if (AFF_FLAGGED(vict, AFF_BLIND)) return FALSE;
  if (GET_LEVEL(ch) >= LVL_IMMUNE) return FALSE;
  if (CMD_IS("south")) {
        send_to_char("You can't go that way... Yet.", ch);
        return TRUE;
  }
  return FALSE;
}
SPECIAL(block_west) {
  struct char_data *vict = (struct char_data *) me;
  if (IS_NPC(ch)) return FALSE;
  if (AFF_FLAGGED(vict, AFF_SLEEP)) return FALSE;
  if (GET_POS(vict) < POS_SLEEPING) return FALSE;
  if (AFF_FLAGGED(vict, AFF_BLIND)) return FALSE;
  if (GET_LEVEL(ch) >= LVL_IMMUNE) return FALSE;
  if (CMD_IS("west")) {
        send_to_char("You can't go that way... Yet.", ch);
        return TRUE;
  }
  return FALSE;
}

SPECIAL(gm_guild)
{
  char arg[256]= {"    "}; /* Array for meta type  name */
  int is_altered = FALSE;
  /*char buf[128];*/
  int add_hp =   MAX(0, ((GET_CON(ch) - 16) /2)), 
      add_mana = MAX(0,((GET_INT(ch) - 16) /2)),
      add_move = MAX(0, ((GET_DEX(ch) - 16) /2));
  //GM Requirements defined here

  struct descriptor_data *dd; 

  argument = one_argument(argument, arg);  

  if (CMD_IS("gain")) 
  {
   if (GET_TOTAL_LEVEL(ch) < MAX_TOTAL_LEVEL) 
     return FALSE;
     
   if (GET_EXP(ch) >= GMEXP  && GET_GOLD(ch) >= GMGOLD)
   {
     GET_EXP(ch) -= (GMEXP);
     GET_GOLD(ch) -= (GMGOLD);
     GET_GM_LEVEL(ch) = GET_GM_LEVEL(ch) + 1;
   }
   else 
   {
     send_to_char("You do not have enough exp or gold to level.\r\n", ch);
     return TRUE;
   }

     switch(GET_CLASS(ch))
     {
      case CLASS_MAGIC_USER:
       add_hp += 20;
       add_mana += 40;
       add_move += 10;
       break;
      case CLASS_CLERIC:
       add_hp += 30;
       add_mana += 30;
       add_move += 10;
       break;
      case CLASS_THIEF:
       add_hp += 30;
       add_mana += 20;
       add_move += 20;
       break;
      case CLASS_WARRIOR:
       add_hp += 60;
       add_mana += 0;
       add_move += 10;
       break;
    }

  ch->points.max_hit  += MAX(0, add_hp);
  ch->points.max_move += MAX(0, add_move);
  ch->points.max_mana += MAX(0, add_mana);
 
  save_char(ch, NOWHERE);  
  is_altered = TRUE;
 }

 if (is_altered == TRUE) 
 {
      sprintf(buf, "%s advanced a GM level to level  %d.",
                GET_NAME(ch), GET_GM_LEVEL(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        // All imms good now..
        sprintf(buf, "&G%s gained a GM level.&n", GET_NAME(ch));
        for (dd = descriptor_list; dd; dd = dd->next)
          if (STATE(dd) == CON_PLAYING && dd != ch->desc && GET_LEVEL(dd->character) < LVL_IMMORT && PRF_FLAGGED2(dd->character, PRF2_HEARLEVEL))
             act(buf, 0, 0, 0, dd->character, TO_VICT | TO_SLEEP);
      
        send_to_char("You raise one GM level!\r\n", ch);
        sprintf(buf, "You gain (%d) hp, (%d) mana, (%d) mv.\r\n", 
                add_hp, add_mana, add_move);
        GET_PRACTICES(ch) += 2;
        send_to_char("You gain two practices.\r\n", ch);

        send_to_char(buf, ch);
        return (TRUE);
 }

if(CMD_IS("exchange"))
 {
  if (!*arg)
  {
    send_to_char("You must tell me what you wish to exchange.(gold or exp)\r\n", ch);
    return TRUE;
  }
  if(str_cmp(arg, "gold") == 0)
  {
    if (GET_GOLD(ch) < 1000000)
    {
      send_to_char("You can only exchange in amounts of 1 million.\r\n", ch);
      return TRUE;
    }
    send_to_char("You exchange 1 million gold for 6 million exp.\r\n", ch);
    GET_GOLD(ch) -= 1000000;
    GET_EXP(ch) += 6000000;
    return TRUE;
  }

  if(str_cmp(arg, "exp") == 0)
  {
    if (GET_EXP(ch) < 5000000)
    {
      send_to_char("You can only exchange in amounts of 5 million.\r\n", ch);
      return TRUE;
    }
    send_to_char("You exchange 5 million exp for 500k gold.\r\n", ch);
    GET_EXP(ch) -= 5000000;
    GET_GOLD(ch) += 500000;
    return TRUE;
  }
  send_to_char("You can't exchange that!\r\n", ch);
  return TRUE;
 }
  return (FALSE);
}


SPECIAL(cap_flag) {
  struct obj_data *obj;
  obj_vnum one = 5001;
  obj_vnum two = 5000;


 if (CMD_IS("drop")) {

   one_argument(argument, arg);

   if (!*arg) {
    send_to_char("Do you have the flag or not?!?", ch);
    return 1;
   }

   if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return 1;
   }
 
   if (freeze_game_running) {
   if (GET_OBJ_VNUM(obj) == one && GET_FREEZE_TEAM(ch) == TEAM_RED && GET_ROOM_VNUM(IN_ROOM(ch)) == red_room) {
    if (get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)) 
    {
    sprintf(buf, "You have captured blue flag!\r\n");
    send_to_char(buf, ch);
    freeze_game_redpnt = (freeze_game_redpnt + 10);
    }
    else
    {
    sprintf(buf, "Your flag was not here, so Flag Reset!\r\n");
    send_to_char(buf, ch);
    }
    do_drop(ch, "flag", cmd, 0);
    return_blue_flag();
    return 1;
   } 
   if (GET_OBJ_VNUM(obj) == two && GET_FREEZE_TEAM(ch) == TEAM_BLUE && GET_ROOM_VNUM(IN_ROOM(ch)) == blue_room) {
    if (get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)) 
    {
    sprintf(buf, "You have captured red flag!\r\n");
    send_to_char(buf, ch);
    freeze_game_bluepnt = (freeze_game_bluepnt + 10);
    }
    else
    {
    sprintf(buf, "Your flag was not here, so Flag Reset!\r\n");
    send_to_char(buf, ch);
    }
    do_drop(ch, "flag", cmd, 0);
    return_red_flag();
    return 1;
    } 
  }
   else {
    do_drop(ch, arg, cmd, 0);
    return 1;
    }
  }
  return 0;
}


SPECIAL(reincarnation)
{
 if (CMD_IS("use ankh"))
 {
   if (GET_POS(ch) <= POS_DEAD)
   {
        send_to_char("The ankh glows.\r\n",ch);
        GET_HIT(ch) = 100;
        GET_MANA(ch) = 100;
        GET_MOVE(ch) = 100;
        REMOVE_BIT(PLR_FLAGS(ch), PLR_DEAD);
        REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADI);
        REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADII);
        REMOVE_BIT(PLR_FLAGS(ch), PLR_DEADIII);
        update_pos(ch);
        extract_obj(me);
   }
   else
   {
        send_to_char("The ankh glows, and nothing happens.\r\n",ch);
        extract_obj(me);
   }
  return (TRUE);
 }
return (FALSE);
}

void new_gm(struct char_data *ch) {
  struct obj_data *obj;
  add_to_gm_list(ch);
  send_to_char("You are now a GrandMaster!\r\n", ch);
  send_to_char("Check your inventory for a GM item.\r\n", ch);
  obj = read_object(5117, VIRTUAL);
  obj_to_char(obj, ch);
}
void add_to_gm_list(struct char_data *ch) {
  FILE *gm_file = fopen(GM_FILE, "a");
  fprintf(gm_file, "%s\n", GET_NAME(ch));
  fclose(gm_file);
  file_to_string_alloc(GM_FILE, &gms);
}

/* Alright...let's sort em out... */
void sort_econrank(struct char_data *ch) {
  FILE *econfile = fopen(ECON_FILE, "r");
  FILE *econfile2 = fopen(ECON_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just fought */
  while (fgets(buf, 80, econfile))
    {
      sscanf(buf, "<< Name: %s   Econrank: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(econfile2, "%s", buf);
    }
  fclose(econfile);
  fprintf(econfile2, "<< Name: %s   Econrank: %d >>\n", GET_NAME(ch), GET_ECONRANK(ch));
  fclose(econfile2);

  system("sort -rn -k 4 ../lib/etc/econranktwo | head > ../lib/etc/econrank");
  system("rm ../lib/etc/econranktwo");

  file_to_string_alloc(ECON_FILE, &econrank);
  return;
}

/* Alright...let's sort em out... */
void sort_assassinrank(struct char_data *ch) {
  FILE *assassinfile = fopen(ASSASSIN_FILE, "r");
  FILE *assassinfile2 = fopen(ASSASSIN_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just fought */
  while (fgets(buf, 80, assassinfile))
    {
      sscanf(buf, "<< Name: %s   Rank: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
        fprintf(assassinfile2, "%s", buf);
    }
  fclose(assassinfile);
  fprintf(assassinfile2, "<< Name: %s   Rank: %d >>\n", GET_NAME(ch), GET_ASSASSINRANK(ch));
  fclose(assassinfile2);

  system("sort -rn -k 4 ../lib/etc/assassinranktwo | head > ../lib/etc/assassinrank");
  system("rm ../lib/etc/assassinranktwo");

  file_to_string_alloc(ASSASSIN_FILE, &assassinrank);
  return;
}


SPECIAL(religion)
{
  int temp = GET_RELIGION(ch);

  if (CMD_IS("apply"))
  {
   switch (GET_ROOM_VNUM(IN_ROOM(ch)))
   {
     case 280:
       if (GET_RELIGION(ch) != RELIG_PHOBOS)
       {
       send_to_char("You switch your religious affilations to that of the goddess of peace and love, Phobe.\r\n", ch);
       GET_RELIGION(ch) = RELIG_PHOBOS;
       GET_SACRIFICE(ch) = 0;
       }
     break;
     case 1540:
       if (GET_RELIGION(ch) != RELIG_ATHEIST)
       {
       send_to_char("You rescind all religious beliefs you might have had.\r\n", ch);
       GET_RELIGION(ch) = RELIG_ATHEIST;
       GET_SACRIFICE(ch) = 0;
       }
     break;
     case 1591:
       if (GET_RELIGION(ch) != RELIG_DEIMOS)
       {
       send_to_char("You devote yourself to chaos and the work of Deimos.\r\n", ch);
       GET_RELIGION(ch) = RELIG_DEIMOS;
       GET_SACRIFICE(ch) = 0;
       }
     break;
     case 24168:
       if (GET_RELIGION(ch) != RELIG_RULEK)
       {
       send_to_char("You accept the tenants of stability and balance, and proceed to follow Rulek.\r\n", ch);
       GET_RELIGION(ch) = RELIG_RULEK;
       GET_SACRIFICE(ch) = 0;
       }
     break;
     case 1592:
       if (GET_RELIGION(ch) != RELIG_CALIM)
       {
       send_to_char("You start to view life and creation with utmost reverance, like your god, Calim.\r\n", ch);
       GET_RELIGION(ch) = RELIG_CALIM;
       GET_SACRIFICE(ch) = 0;
       }
     break;
   }

   if (temp != GET_RELIGION(ch))
   {
     switch (temp)
     {
     case RELIG_ATHEIST:
         aremoverelig(ch);
     break;	
     case RELIG_PHOBOS:
         premoverelig(ch);
     break;	
     case RELIG_DEIMOS:
         dremoverelig(ch);
     break;	
     case RELIG_CALIM:
         cremoverelig(ch);
     break;	
     case RELIG_RULEK:
         rremoverelig(ch);
     break;	
    }
   }
 
   return TRUE;

  //Phobe: 280
  //Atheist: 1540
  //Calim: 2810
  //Rulek: 24168
  //Deimos 1591

  }
 return FALSE;
}

SPECIAL(clan_guard)
{
  int clan = CLAN_NONE, room = 0;
   struct clan_type *cptr = NULL;

  if(GET_ROOM_VNUM(ch->in_room) == 289) 
  {clan = 1; room = 1541;} //WHY
  if(GET_ROOM_VNUM(ch->in_room) == 246) 
  {clan = 2; room = 1542;} //GoD
  if(GET_ROOM_VNUM(ch->in_room) == 292)
  {clan = 3; room = 1544;} //OoC  

  for (cptr = clan_info; cptr && cptr->number != clan; cptr = cptr->next);

  if(CMD_IS("password"))
  {
   skip_spaces(&argument);
   if (!*argument)
   {
     send_to_char("&RYou must supply a password!&n\r\n", ch);
     return TRUE;
   }
   if (strcmp(argument, cptr->clan_password) == 0)
   {
     act("&WThe Guard moves aside and you quickly walk down a darkened hallway.&n", 1,ch,0,0,TO_CHAR);
     act("&WThe Guard moves aside and $n disappears down a hallway.&n", 1,ch,0,0,TO_ROOM);
     char_from_room(ch);
     char_to_room(ch, real_room(room));
     return TRUE;
   }
   else
   {
     send_to_char("&RThat password is incorrect.&n\r\n", ch);
     return TRUE;
   }

  }
 return FALSE;
}


SPECIAL(assemble)
{
  long          lVnum = NOTHING;
  struct obj_data *pObject = NULL;

  skip_spaces(&argument);

  if(!CMD_IS("assemble") && ! CMD_IS("forge"))
  {
    return FALSE;
  }

  if (*argument == '\0') {
    sprintf(buf, "%s what?\r\n", CMD_NAME);
    send_to_char(CAP(buf), ch);
    return TRUE;
  } else if ((lVnum = assemblyFindAssembly(argument)) < 0) {
    sprintf(buf, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
    send_to_char(buf, ch);
    return TRUE;
  } else if (!assemblyCheckComponents(lVnum, ch)) {
    send_to_char("You haven't all the parts to make that.\r\n", ch);
    return TRUE;
  }

  /* Create the assembled object. */
  if ((pObject = read_object(lVnum, VIRTUAL)) == NULL ) {
    sprintf(buf, "You can't %s %s %s.\r\n", CMD_NAME, AN(argument), argument);
    send_to_char(buf, ch);
    return TRUE;
  }

  /* Now give the object to the character. */
  obj_to_char(pObject, ch);

  /* Tell the character they made something. */
  sprintf(buf, "You %s $p.", CMD_NAME);
  act(buf, FALSE, ch, pObject, NULL, TO_CHAR);

  /* Tell the room the character made something. */
  sprintf(buf, "$n %ss $p.", CMD_NAME);
  act(buf, FALSE, ch, pObject, NULL, TO_ROOM);

  return TRUE;
}

SPECIAL(gm_zone)
{
  skip_spaces(&argument);

  if(!CMD_IS("say"))
  {
    return FALSE;
  }

  if (*argument == '\0') 
  {
    return FALSE;
  }
  if (GET_TOTAL_LEVEL(ch) < 240) return FALSE;

  if (strcmp(argument, "oztersmf sefokspef hwrpesjf") == 0)
  {
    
    /* Tell the character they made something. */
    sprintf(buf, "Garumeth holds his hands over you, and you feel a tingling feeling.");
    act(buf, FALSE, ch, NULL, NULL, TO_CHAR);

    /* Tell the room the character made something. */
    sprintf(buf, "Garumeth puts his hands over $n, and $e disappears.");
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    char_from_room(ch);
    char_to_room(ch, real_room(2066));
    
    do_look(ch, "", 15, 0);
    return TRUE;
  }
  return FALSE;
}
SPECIAL(sholo)
{
  return FALSE;
}
