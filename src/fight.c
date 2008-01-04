/*
************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define _GNU_SOURCE
#include <stdio.h>

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "dg_scripts.h"
#include "genmob.h"

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct time_info_data time_info;  
extern struct index_data *mob_index;
extern const struct weapon_prof_data wpn_prof[];
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int pk_allowed;		/* see config.c */
extern int max_exp_gain;	/* see config.c */
extern int max_exp_loss;	/* see config.c */
extern int max_npc_corpse_time, max_pc_corpse_time;
extern char *sparrank;
extern char *assassinrank;
extern int file_to_string_alloc(const char *name, char **buf);
extern bool DOUBLEEXP, DOUBLEGOLD, CANASSASSINATE;
/* External procedures */
char *fread_action(FILE * fl, int nr);
ACMD(do_flee);
ACMD(do_track);
double backstab_mult(struct char_data *ch, int level);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
int calc_ave_damage(struct char_data *ch, struct char_data *victim);
int fact(int num);
void update_sacrifice(struct char_data *ch);
void sort_assassinrank(struct char_data *ch);

/* local functions */
void dam_message(int dam, struct char_data * ch, struct char_data * victim, int w_type);
void appear(struct char_data * ch);
void load_messages(void);
void check_killer(struct char_data * ch, struct char_data * vict);
void make_corpse(struct char_data * ch);
void delete_eq(struct char_data *ch);
void change_alignment(struct char_data * ch, struct char_data * victim);
void death_cry(struct char_data * ch);
void raw_kill(struct char_data * ch, struct char_data * killer);
void die(struct char_data * ch, struct char_data * killer);
char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural);
void perform_violence(void);
int compute_armor_class(struct char_data *ch);
int get_weapon_prof(struct char_data *ch, struct obj_data *wield);
//void improve_skill(struct char_data *ch, int skill);
//void perform_group_gain(struct char_data * ch, int base, struct char_data * victim);
//void group_gain(struct char_data * ch, struct char_data * victim);
//void solo_gain(struct char_data * ch, struct char_data * victim);
// Gain Code
void exp_gain(struct char_data * ch, struct char_data * victim);
void perform_group_gain(struct char_data * ch, struct char_data * victim);
void perform_solo_gain(struct char_data * ch, struct char_data * victim);
int get_exp_gain(struct char_data * ch, struct char_data * victim);
void perform_exp_gain(struct char_data * ch, int exp, struct char_data * victim);
int get_group_exp(struct char_data * k, int exp, int members, int total_levels);
bool CAN_FIGHT(struct char_data * ch, struct char_data * vict);

int extra_attacks_from_level(struct char_data *ch);
int extra_attacks_from_skill(struct char_data *ch);

// Gain Prototype end.

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},		/* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	/* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	/* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

/* The Fight related routines */

void appear(struct char_data * ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE | AFF_SHADE);

  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}


int compute_armor_class(struct char_data *ch)
{
 // This function calculates percent of damange that remains after it goes through armor
  int armorclass;
  if (!IS_NPC(ch) || MOB_FLAGGED(ch, MOB_EDIT)) {
    //Max 1128 is 100
    if (GET_AC(ch) >= 0)
      armorclass = GET_AC(ch) * .25; /* Max Damage Reduction = 25% beforedex*/
    else
      armorclass = -GET_AC(ch) * .5; /* Doulbe Damage for Under 0 ac */
    
    if (AWAKE(ch) && !IS_NPC(ch))
      armorclass += (GET_DEX(ch) * .4); /* Can Boost max to 35% off */
    
    return (100 - armorclass);      /* -50 is lowest */
  }
  else
    return (100 - GET_LEVEL(ch) * .53);
}

void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    log("SYSERR: Error reading combat message file %s: %s", MESS_FILE, strerror(errno));
    exit(1);
  }
  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }


  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      log("SYSERR: Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}


void update_pos(struct char_data * victim)
{
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0)
    GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -4)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data * ch, struct char_data * vict)
{
  char buf[256];

  if (PLR_FLAGGED(vict, PLR_KILLER) || PLR_FLAGGED(vict, PLR_THIEF))
    return;
  if (PLR_FLAGGED(ch, PLR_KILLER) || IS_NPC(ch) || IS_NPC(vict) || ch == vict)
    return;

  SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
  sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
	    GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
  send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data * ch, struct char_data * vict)
{
  struct char_data *k;
  struct follow_type *f;

  if (ch == vict)
    return;

  if (FIGHTING(ch)) {
    core_dump();
    return;
  }

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (AFF_FLAGGED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;

	/* Preform AutoAssist */
  /* Right now I don't want NPC's to autoassist, I may change that or
       use spec_proc's for NPC groups and assisting.                  */

  if (IS_AFFECTED(ch, AFF_GROUP) && !IS_NPC(ch) ) {
    k = (ch->master ? ch->master : ch);   /* Find the master */

    /* Not an NPC, Not Yourself, Not Fighting, Above standing */
    if (!IS_NPC(ch) && k != ch && GET_POS(k) == POS_STANDING ) {
     if (k != vict) {
     if (ch->in_room == k->in_room) {

     if(PRF_FLAGGED(k, PRF_AUTOASSIST) && CAN_FIGHT(k, vict)) 
     { /* If he is flagged to assist */
       k->next_fighting = combat_list;    /* Master assists a follower */
       combat_list = k;
       FIGHTING(k) = vict;
       GET_POS(k) = POS_FIGHTING;
      }
     }
    }
   }

    for (f = k->followers; f; f = f->next) {
       if(!CAN_FIGHT(f->follower, vict)) 
           continue;                   /* Skip if any of these are true */
       if(PRF_FLAGGED(f->follower, PRF_AUTOASSIST)) {
        f->follower->next_fighting = combat_list;
        combat_list = f->follower;        /* Follower Assists Group */
        FIGHTING(f->follower) = vict;
        GET_POS(f->follower) = POS_FIGHTING;
      }
    }
  }
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);
}



void make_corpse(struct char_data * ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  
  int i;
  
  corpse = create_obj();

  corpse->item_number = NOTHING;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");

  sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = str_dup(buf2);

  sprintf(buf2, "the corpse of %s", GET_NAME(ch));
  corpse->short_description = str_dup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
  //GET_OBJ_EXTRA(corpse) = ITEM_NODONATE;

  if (!IS_NPC(ch)) {
  GET_OBJ_TYPE(corpse) = ITEM_PCCORPSE;
  }
  if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_ASSASSIN)) {
  GET_OBJ_TYPE(corpse) = ITEM_ASSCORPSE;
  }
  GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch); 
  //something is wrong with this line ^
  //GET_OBJ_RENT(corpse) = 100000;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
  else
    GET_OBJ_TIMER(corpse) = max_pc_corpse_time;
  SET_BIT(GET_OBJ_EXTRA(corpse), ITEM_UNIQUE_SAVE);

  /* transfer character's inventory to the corpse */

  /*for (o = ch->carrying; o != NULL; o = o->next_content)
  {  
    if (!IS_OBJ_STAT(o, ITEM_QUEST)) {
      obj_from_char(o);
      obj_to_obj(o, corpse);
    }
  }*/

  corpse->contains = ch->carrying;

  for (o = corpse->contains; o != NULL; o = o->next_content) 
  {
    o->in_obj = corpse;
  }
    object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
  {
    if (GET_EQ(ch, i))
    {
  //   if (!IS_OBJ_STAT(GET_EQ(ch, i), ITEM_QUEST)) {
      remove_otrigger(GET_EQ(ch, i), ch);
      obj_to_obj(unequip_char(ch, i), corpse);
    //  }
    }
  }
  /* transfer gold */

  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) { 
       if (DOUBLEGOLD && IS_NPC(ch))
       {
        if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_EDIT))
           money = create_money(MAX(0, .1*2*GET_MOB_GOLD(ch)));
        else 
           money = create_money(MAX(0, 2*GET_GOLD(ch)));
       }      
       else
       {
        if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_EDIT))
           money = create_money(MAX(0, .1*GET_MOB_GOLD(ch)));
        else 
           money = create_money(MAX(0, GET_GOLD(ch)));
       }

      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  


    ch->carrying = NULL;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;

  obj_to_room(corpse, ch->in_room);
}



void delete_eq(struct char_data * ch)
{
  struct obj_data *o, *p;
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
    {
      remove_otrigger(GET_EQ(ch, i), ch);
    }
  //for (o = corpse->contains; o != NULL; o = o->next_content)

  if (!ch->carrying) return;

  o = ch->carrying->contains;
  while (o != NULL)
  {
   p = o;
   o = o->next_content;
   free_obj(p);
  }

  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;
  send_to_char("If you see this your eq has been deleted hahahahah!\r\n", ch);

  return;
}


/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 16;
}



void death_cry(struct char_data * ch)
{
  int door;

  if (!PLR_FLAGGED(ch, PLR_DEAD))
     act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (CAN_GO(ch, door))
     if (!PLR_FLAGGED(ch, PLR_DEAD))
      send_to_room("Your blood freezes as you hear someone's death cry.\r\n",
		world[ch->in_room].dir_option[door]->to_room);
}

/* Alright...let's sort em out... */
void sort_sparrank(struct char_data *ch) {
  FILE *sparfile = fopen(SPAR_FILE, "r");
  FILE *sparfile2 = fopen(SPAR_FILE2, "w");
  char buf[80];
  char name[32];

  /* We're adding the names that just fought */
  while (fgets(buf, 80, sparfile))
    {
      sscanf(buf, "<< Name: %s   Sparrank: %*d >>", name);
      if (strcmp(name, GET_NAME(ch)) != 0)
	fprintf(sparfile2, "%s", buf);
    }
  fclose(sparfile);
  fprintf(sparfile2, "<< Name: %s   Sparrank: %d >>\n", GET_NAME(ch), GET_SPARRANK(ch));
  fclose(sparfile2);
 
  system("sort -rn -k 4 ../lib/etc/sparranktwo | head > ../lib/etc/sparrank");
  system("rm ../lib/etc/sparranktwo");

  return;
}


void set_rank(struct char_data *killer, struct char_data *ch) {
 int rank = (50-((GET_SPARRANK(killer)-GET_SPARRANK(ch))/2));
 int ranki = (50+((GET_SPARRANK(ch)-GET_SPARRANK(killer))/2));

 /* If victim is more than 50 rankings below you */
 if (GET_SPARRANK(ch) < (GET_SPARRANK(killer) - 50)) {
  send_to_char("Your rank has not been changed\r\n", killer);
  } 

 /* Victim is below you but not 50 below you. */
 if ((GET_SPARRANK(ch) > (GET_SPARRANK(killer) - 50)) && (GET_SPARRANK(ch) < GET_SPARRANK(killer))) {
  GET_SPARRANK(killer) += rank;
  GET_SPARRANK(ch)     -= rank;
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_SPARRANK(ch) == GET_SPARRANK(killer)) {
  GET_SPARRANK(killer) += 50;
  GET_SPARRANK(ch) -= 50;
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_SPARRANK(ch) > GET_SPARRANK(killer)) {
  GET_SPARRANK(killer) += ranki;
  GET_SPARRANK(ch) -= ranki;
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_SPARRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_SPARRANK(killer) >= 1000) {
  GET_SPARRANK(killer) = 1000;
  send_to_char("Your already the top rank!\r\n", killer);
  }

 if (GET_SPARRANK(ch) <= 1) {
  GET_SPARRANK(ch) = 1;
  send_to_char("You are the lowest of low!\r\n", ch);
  }
  if(!(GET_LEVEL(ch) > 60))
    sort_sparrank(ch);
  if(!(GET_LEVEL(killer) > 60))
    sort_sparrank(killer);
  file_to_string_alloc(SPAR_FILE, &sparrank);
}  

void set_arank(struct char_data *killer, struct char_data *ch) {
 int rank = (50-((GET_ASSASSINRANK(killer)-GET_ASSASSINRANK(ch))/2));
 int ranki = (50+((GET_ASSASSINRANK(ch)-GET_ASSASSINRANK(killer))/2));

 /* If victim is more than 50 rankings below you */
 if (GET_ASSASSINRANK(ch) < (GET_ASSASSINRANK(killer) - 50)) {
  send_to_char("Your rank has not been changed\r\n", killer);
  } 

 /* Victim is below you but not 50 below you. */
 if ((GET_ASSASSINRANK(ch) > (GET_ASSASSINRANK(killer) - 50)) && (GET_ASSASSINRANK(ch) < GET_ASSASSINRANK(killer))) {
  GET_ASSASSINRANK(killer) += rank;
  GET_ASSASSINRANK(ch)     -= rank;
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_ASSASSINRANK(ch) == GET_ASSASSINRANK(killer)) {
  GET_ASSASSINRANK(killer) += 50;
  GET_ASSASSINRANK(ch) -= 50;
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_ASSASSINRANK(ch) > GET_ASSASSINRANK(killer)) {
  GET_ASSASSINRANK(killer) += ranki;
  GET_ASSASSINRANK(ch) -= ranki;
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(killer));
  send_to_char(buf, killer);
  sprintf(buf, "Your rank is now %d\r\n", GET_ASSASSINRANK(ch));
  send_to_char(buf, ch);
  }

 if (GET_ASSASSINRANK(killer) >= 1000) {
  GET_ASSASSINRANK(killer) = 1000;
  send_to_char("Your already the top rank!\r\n", killer);
  }

 if (GET_ASSASSINRANK(ch) <= 1) {
  GET_ASSASSINRANK(ch) = 1;
  send_to_char("You are the lowest of low!\r\n", ch);
  }
  if(!(GET_LEVEL(ch) > 60))
    sort_assassinrank(ch);
  if(!(GET_LEVEL(killer) > 60))
    sort_assassinrank(killer);
  file_to_string_alloc(ASSASSIN_FILE, &assassinrank);
}  


void raw_kill(struct char_data * ch, struct char_data * killer)
{
  GET_KILLER(ch) = killer;
  if (FIGHTING(ch)) 
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  if (killer) {
    if (death_mtrigger(ch, killer))
      death_cry(ch);
    GET_KILLS(killer) += 1;
  } else
      death_cry(ch);

 if (IS_NPC(ch)) {
   make_corpse(ch);
   extract_char(ch);
 }

  if (killer && (killer->in_room == real_room(1527) || 
                killer->in_room == real_room(1528) ||
                killer->in_room == real_room(1529) ||
                killer->in_room == real_room(1530) ||
                killer->in_room == real_room(1531)))

  {
    stop_fighting(killer);
    char_from_room(killer);
    char_to_room(killer, real_room(291));
    GET_POS(killer) = POS_RESTING;
    set_rank(killer, ch);
  }

}



void die(struct char_data * ch, struct char_data * killer)
{
  GET_LOADROOM(ch) = NOWHERE;
  if (!IS_NPC(ch))
    REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
    stop_fighting(ch);

  if (!(PLR_FLAGGED(ch, PLR_DEAD | PLR_DEADI | PLR_DEADII | PLR_DEADIII))
      && !ROOM_FLAGGED(ch->in_room, ROOM_SPARRING))
  {  
		GET_EXP(ch) = 0;
  }

  raw_kill(ch, killer);
}

// Group Gain Code
void exp_gain(struct char_data * ch, struct char_data * victim)
{
	// Do nothing Checks
	// Place other specific Checks within perform_solo_gain and perform_group_gain
	if (!IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_SPARRING))
		return; // Sparring Room Case = Do Nothing
	
	if (AFF_FLAGGED(ch, AFF_GROUP))
		{
			//Group Code
			perform_group_gain(ch, victim);
		} else {
		//Solo Code
		perform_solo_gain(ch, victim);
	}
}

void perform_group_gain(struct char_data * ch, struct char_data * victim)
{
  //Declarations
  int total_members;
  int total_levels;
  int oldexp = 1;
  int total_gain = 1;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))  //NO CLUE WHAT THIS DOES, something to find group leader.
    k = ch;

  if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room)) //Base Member Case
  {
    total_members = 1;
    total_levels = GET_TOTAL_LEVEL(k);
  }else {
    total_members = 0;
    total_levels = 0;
  }

  for (f = k->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room 
				&& !IS_NPC(f->follower) && GET_LEVEL(f->follower) < LVL_IMMORT)
    {
      total_members++;
      total_levels += GET_TOTAL_LEVEL(f->follower);  //Followers Levels
    }

  if (total_members == 1) 
   {
      perform_solo_gain(ch, victim);
      return;
   } 
	else if (total_members > 1) 
   {
      total_gain = get_exp_gain(k, victim);
   }
    oldexp = total_gain;

  if (AFF_FLAGGED(k, AFF_GROUP) && k->in_room == ch->in_room)
  {
    total_gain = get_group_exp(k, oldexp, total_members, total_levels);
    total_gain = MIN(total_gain, oldexp*2);
    perform_exp_gain(k, total_gain, victim);
  }
	
  for (f = k->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room 
				&& GET_LEVEL(f->follower) < LVL_IMMORT) 
			{
				total_gain = get_group_exp(f->follower ,oldexp, total_members, total_levels);
				total_gain = MIN(total_gain, oldexp*2);
				perform_exp_gain(f->follower, total_gain, victim);
			}
}



void perform_solo_gain(struct char_data * ch, struct char_data * victim)
{
	//Solo Code
	perform_exp_gain(ch, get_exp_gain(ch, victim), victim);
}


int get_exp_gain(struct char_data * ch, struct char_data * victim)
{
	int exp;
	
	if (IS_NPC(victim) && !MOB_FLAGGED(victim, MOB_EDIT))
		exp = MIN(max_exp_gain, (.1*GET_MOB_EXP(victim))); // Standard Mob
	else
	  exp = MIN(max_exp_gain, GET_EXP(victim));     // Everything Else
	
	if (PLR_FLAGGED(victim, PLR_ASSASSIN)) // Assassin Case
	  exp = MIN(100000000,GET_EXP(victim) 
							* (float)(GET_TOTAL_LEVEL(victim) / (float)240));
	
	/* Calculate level-difference bonus */
	if (IS_NPC(ch))
	  exp = 0;   // Gainer is a Mob, Set Exp to 0 Mobs can't gain exp.
	else
	  exp += MAX(0, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
	
	/* prevent illegal xp creation when killing players */
	if (!IS_NPC(victim))
		exp = MIN(max_exp_loss, exp);
	
	return MAX(exp, 1);
}

void perform_exp_gain(struct char_data * ch, int exp, struct char_data * victim)
{
	//Messages
	if (exp > 1) { //More than 1 exp
		if (DOUBLEEXP)
			{
				send_to_char("&MYou gain double the experience!&n\r\n",ch);
				exp *= 2;
			}
		sprintf(buf2, "You receive %d experience points.\r\n", exp);
		send_to_char(buf2, ch);
	} else //Less than 1 exp
		send_to_char("You receive one lousy experience point.\r\n", ch);
	
  if (GET_KILLER(victim) != NULL && !IS_NPC(GET_KILLER(victim)))
		gain_exp(victim, -exp);
	
	stop_fighting(ch); // End the fight [This was moved from within the message, Might be a problem]
	gain_exp(ch, exp); // Do the gaining
	change_alignment(ch, victim);
}

int get_group_exp(struct char_data * k, int exp, int members, int total_levels)
{
	float percent;
	float temp = (float)members;
	//Create New Exp Pool Based off How many members are there
	if (members > 1)
		exp = ((float)exp * (1.0 + (temp - 1) / 10.)); 
	// Currently 60% of the number of Members is Multipled in.
	
	percent = (float)GET_TOTAL_LEVEL(k) / (float)total_levels;
	
	//   sprintf(buf, "DEBUG: %s, %d/%d, Earning: %d of a %d total, with %d
	//group members.\r\n", 
	//          GET_NAME(k), GET_TOTAL_LEVEL(k), total_levels,(int)(exp *
	//percent), exp, members);
	//   send_to_char(buf, k); 
	return exp * percent; // G_T_L(ch)/t_l = Perecent of total levels.
}
// End Group Gain Code


char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural)
{
  static char buf[256];
  char *cp = buf;
	
  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
				for (; *weapon_plural; *(cp++) = *(weapon_plural++));
				break;
      case 'w':
				for (; *weapon_singular; *(cp++) = *(weapon_singular++));
				break;
      default:
				*(cp++) = '#';
				break;
      }
    } else
      *(cp++) = *str;
		
    *cp = 0;
  }				/* For */
	
  return (buf);
}

void improve_skill(struct char_data *ch, int skill)
{
  int percent = GET_SKILL(ch, skill);
  int newpercent;
  char skillbuf[MAX_STRING_LENGTH];
	
  if (number(1, 200) > GET_WIS(ch) + GET_INT(ch))
		return;
  if (percent >= 97 || percent <= 0)
		return;
  newpercent = number(1, 3);
  percent += newpercent;
  SET_SKILL(ch, skill, percent);
  if (newpercent >= 4) {
		sprintf(skillbuf, "You feel your abilities improving.");
		send_to_char(skillbuf, ch);
  }
}

/* message for doing damage with a weapon */
void dam_message(int dam, struct char_data * ch, struct char_data * victim,
								 int w_type)
{
  char *buf;
  int msgnum;
	
  static struct dam_weapon_type {
    const char *to_room;
    const char *to_char;
    const char *to_victim;
  } dam_weapons[] = {
		
    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */
		
    {
      "&G$n&G tries to #w $N&G, but misses.&n",	/* 0: 0     */
      "&RYou try to #w $N&R, but miss.&n",
      "&G$n&G tries to #w you, but misses.&n"
    },

    {
      "&Y$n&Y injures $N&Y with $s #w.&n",	/* 1: 1..2  */
      "&YYou injure $N&Y with your #w.&n",
      "&R$n&R injures you with $s #w.&n"
    },

    {
      "&Y$n&Y wounds $N&Y with $s #w.&n",		/* 2: 3..4  */
      "&YYou wound $N&Y with your #w.&n",
      "&R$n&R wounds you with $s #w.&n"
    },

    {
      "&Y$n&Y destroys $N&Y with $s #w.&n",		/* 3: 5..6*/
      "&YYou destroy $N&Y with your #w&n",
      "&R$n&R destroys you with $s #w.&n"
    },

    {
      "&Y$n&Y shreds $N&Y into little pieces with $s #w.&n", /* 4: 7..10*/
      "&YYou shred $N&Y into little pieces with your #w.&n",
      "&R$n&R shreds you into little pieces with $s #w.&n"
    },

    {
      "&Y$n&Y MANGLES $N&Y with $s #w.&n",		/* 5: 11..14  */
      "&YYou MANGLE $N&Y with your #w.&n",
      "&R$n&R MANGLES you with $s #w.&n"
    },

    {
      "&Y$n&Y CRIPPLES $N&Y with $s #w.&n",	/* 6: 15..19  */
      "&YYou CRIPPLE $N&Y with your #w.&n",
      "&R$n&R CRIPPLES you with $s #w.&n"
    },

    {
      "&Y$n&Y LACERATES $N&Y with $s #w.&n",	/* 7: 19..23 */
      "&YYou LACERATE $N&Y with your #w.&n",
      "&R$n&R LACERATES you with $s #w.&n"
    },

    {
      "&Y$n&Y DISFIGURES $N&Y with $s mighty #w!!&n",	/* 8: > 23   */
      "&YYou DISFIGURE $N&Y with your mighty #w!!&n",
      "&R$n&R DISFIGURES you with $s mighty #w!!&n"
    },

    {
      "&Y$n&Y DISMEMBERS $N&Y with $s destructive #w!!&n", /* 9: > 23*/
      "&YYou DISMEMBER $N&Y with your destructive #w!!&n",
      "&R$n&R DISMEMBERS you with $s destructive #w!!&n"
    },

    {
      "&Y$n&Y MASSACRES $N&Y with $s amazing #w!!&n",	/* 10: > 23   */
      "&YYou MASSACRE $N&Y with your amazing #w!!&n",
      "&R$n&R MASSACRES you with $s amazing #w!!&n"
    },

    {
      "&Y$n&Y FLAYS $N&Y with $s deadly #w!!&n",	/* 11: > 23   */
      "&YYou FLAY $N&Y with your deadly #w!!&n",
      "&R$n&R FLAYS you with $s deadly #w!!&n"
    },

    {
      "&Y$n&Y ~~DEMOLISHES~~ $N&Y with $s fatal #w!!&n", /* 12: > 23   */
      "&YYou ~~DEMOLISH~~ $N&Y with your fatal #w!!&n",
      "&R$n&R ~~DEMOLISHES~~ you with $s fatal #w!!&n"
    },

    {
      "&Y$n&Y ~~OBLITERATES~~ $N&Y with $s pulverizing  #w!!&n", /* 13: > 23 */
      "&YYou ~~OBLITERATE~~ $N&Y with your pulverizing #w!!&n",
      "&R$n&R ~~OBLITERATES~~ you with $s pulverizing #w!!&n"
    },

    {
      "&Y$n&Y  *=* A T O M I Z E S *=* $N&Y with $s critical #w!!&n",	/*14: > 23   */
      "&YYou *=* A T O M I Z E *=* $N&Y with your critical #w!!&n",
      "&R$n&R *=* A T O M I Z E S *=* you with $s critical #w!!&n"
    },

    {
      "&Y$n&Y **** SMITES **** $N&Y with $s debilitating #w!!&n", /* 15: > 23 */
      "&YYou **** SMITE **** $N&Y with your debilitating #w!!&n",
      "&R$n&R **** SMITES **** you with $s debilitating #w!!&n"
    }   
  };

  w_type -= TYPE_HIT;		/* Change to base of table with text */

  if (dam == 0)		msgnum = 0;
  else if (dam <= 2)    msgnum = 1;
  else if (dam <= 4)    msgnum = 2;
  else if (dam <= 7)    msgnum = 3;
  else if (dam <= 10)   msgnum = 4;
  else if (dam <= 13)   msgnum = 5;
  else if (dam <= 15)   msgnum = 6;
  else if (dam <= 20)   msgnum = 7; 
  else if (dam <= 30)   msgnum = 8;
  else if (dam <= 50)   msgnum = 9;
  else if (dam <= 80)   msgnum = 10;
  else if (dam <= 100)  msgnum = 11;
  else if (dam <= 120)  msgnum = 12;
  else if (dam <= 150)  msgnum = 13;
  else if (dam <= 180)  msgnum = 14;
  else			msgnum = 15;


  /* damage message to onlookers */
  buf = replace_string(dam_weapons[msgnum].to_room,
											 attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
	
  /* damage message to damager */
  send_to_char(CCYEL(ch, C_CMP), ch);
  buf = replace_string(dam_weapons[msgnum].to_char,
											 attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  send_to_char(CCNRM(ch, C_CMP), ch);
	
  /* damage message to damagee */
  send_to_char(CCRED(victim, C_CMP), victim);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, struct char_data * ch, struct char_data * vict,
		      int attacktype)
{
  int i, j, nr;
  struct message_type *msg;

  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
  if (AFF_FLAGGED(vict, AFF_GASEOUS) && (attacktype < 1 || attacktype > MAX_SPELLS))
  {
	act("&YYour attack falls through $N's body doing no damage.&n", FALSE, ch, weap, vict, TO_CHAR);
	act("&Y$n's attack falls right through your body doing no damage, and making $m look quite silly.&n", FALSE, ch, weap, vict, TO_VICT);
	act("&Y$n's attack falls through $N's body yielding no harm.&n", FALSE, ch, weap, vict, TO_NOTVICT);
        return (1);
  }

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	/* Dam == 0 */
	send_to_char(CCYEL(ch, C_CMP), ch);
	act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	send_to_char(CCNRM(ch, C_CMP), ch);

	send_to_char(CCRED(vict, C_CMP), vict);
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(CCNRM(vict, C_CMP), vict);

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return (1);
    }
  }
  return (0);
}

/*
 * Alert: As of bpl14, this function returns the following codes:
 *	< 0	Victim died.
 *	= 0	No damage.
 *	> 0	How much damage done.
 */
int damage(struct char_data * ch, struct char_data * victim, int dam, int attacktype)
{
	ACMD(do_sacrifice);
	ACMD(do_get);
	ACMD(do_split);
	long old_gold = 0;
	long local_gold = 0;
	char local_buf[256];
	float percent;
	
	
  if (GET_POS(victim) <= POS_DEAD) {
    log("SYSERR: Attempt to damage corpse '%s' in room #%d by '%s'.",
				GET_NAME(victim), GET_ROOM_VNUM(IN_ROOM(victim)), GET_NAME(ch));
    die(victim, ch);
    return (0);	
  }
	
  /* peaceful rooms */
  if (ch->nr != real_mobile(DG_CASTER_PROXY) &&
      ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
		send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return (0);
  }
	
  /* shopkeeper protection */
  if (!ok_damage_shopkeeper(ch, victim))
    return (0);
	
  /* You can't damage an immortal or non-assassin! */
	if (!CAN_ASSASSIN(ch, victim))
   {
       dam = 0;
       stop_fighting(ch);
       return (0);
   }
	
  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
    dam = 0;  

  if (MOB_FLAGGED(victim, MOB_NOHIT))
		{
			send_to_char("&RYou can't attack this type of mobile.&n\r\n", ch);
			return (0);
		}

  if (AFF_FLAGGED(victim, AFF_GASEOUS) && (attacktype < 1 || attacktype > MAX_SPELLS))
    dam = 0;
  
  if (victim != ch) {
    /* Start the attacker fighting the victim */
    if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL))
      set_fighting(ch, victim);
		
    /* Start the victim fighting the attacker */
    if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == NULL)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch))
				remember(victim, ch);
    }
  }
	
  /* If you attack a pet, it hates your guts */
  if (victim->master == ch)
    stop_follower(victim);
	
  /* If the attacker is invisible, he becomes visible */
  if (AFF_FLAGGED(ch, AFF_INVISIBLE | AFF_HIDE | AFF_SHADE))
    appear(ch);

  if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_EDIT))
     dam = MAX(MIN(dam, 1400), 0);
	
  /* Cut damage in half if victim has sanct, to a minimum 1 */
  if (AFF_FLAGGED(victim, AFF_SANCTUARY) && dam >= 2)
    dam /= 2;
	
  if (AFF_FLAGGED(victim, AFF_PROTECT_EVIL)  && GET_ALIGNMENT(ch) < -100 && dam >=10 )
    dam *= .85;
	
	if (AFF_FLAGGED(victim, AFF_MANASHIELD)) 
		{
			if (GET_MANA(victim) > (dam * .1 * GET_SKILL(ch, SPELL_MANASHIELD) / 5.)) 
				{    
					GET_MANA(victim) -=(int) (dam * .1);
					dam -= (int)(dam * .2 * GET_SKILL(ch, SPELL_MANASHIELD) / 5.);
				}
		}
	
  /* Heres Where Armor takes affect, after the attack gets through all */
	percent = (1 - ((float)compute_armor_class(victim) / 100));
	percent = MAX(MIN(percent, .35), -.25); // Safeguard
	dam -= dam * percent;
	
  if (GET_LEVEL(ch) >= LVL_IMPL || attacktype == SPELL_TURKEY)
		dam = MAX(0, dam);
  else if (attacktype == SKILL_BACKSTAB) {
		dam = MAX(MIN(dam, 1000), 0);
  }else if (!IS_NPC(ch)) {
		dam = MAX(MIN(dam, 1000), 0); // Sparrank taken out // Damage cap
  } else
		dam = MAX(MIN(dam, 1000), 0);
	
  if (attacktype == TYPE_AREA_EFFECT) {
    if (GET_HIT(victim) - dam < 1 && number(1,5) != 5) {
			dam = GET_HIT(victim) - 1;
    } 
    struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
		
    /* Find the weapon type (for display purposes only, i think) */
    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
      attacktype = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
    else {
      if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
        attacktype = ch->mob_specials.attack_type + TYPE_HIT;
      else
        attacktype = TYPE_HIT;
    }
  }
	
	GET_HIT(victim) -= dam;
	
  /* Set the maximum damage per round and subtract the hit points */

  update_pos(victim);

  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   * 
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */
	if (ch != victim)
		{
		if (!IS_WEAPON(attacktype))
			skill_message(dam, ch, victim, attacktype);
		else 
			{
				if (GET_POS(victim) == POS_DEAD || dam == 0) 
					{
						if (!skill_message(dam, ch, victim, attacktype))
							dam_message(dam, ch, victim, attacktype);
					} 
				else 
					{
						dam_message(dam, ch, victim, attacktype);
					}
			}
		}
	
  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
	
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("&r$n is mortally wounded, and will die soon, if not aided.&n", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("&rYou are mortally wounded, and will die soon, if not aided.&n\r\n", victim);
    break;
  case POS_INCAP:
    act("&r$n is incapacitated and will slowly die, if not aided.&n", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("&rYou are incapacitated an will slowly die, if not aided.&n\r\n", victim);
    break;
  case POS_STUNNED:
    act("&r$n is stunned, but will probably regain consciousness again.&n", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("&rYou're stunned, but will probably regain consciousness again.&n\r\n", victim);
    break;
  case POS_DEAD:
    act("&r$n&r is dead!  R.I.P.&n", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("&RYou are dead!  Sorry...&n\r\n", victim);
    break;
  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) / 4))
      send_to_char("&RThat really did HURT!&n\r\n", victim);

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (ch != victim && MOB_FLAGGED(victim, MOB_WIMPY) && !AFF_FLAGGED(ch, AFF_WEB))
				do_flee(victim, NULL, 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) && GET_HIT(victim) < 
				GET_WIMP_LEV(victim) && GET_HIT(victim) > 0 && !AFF_FLAGGED(victim, AFF_WEB)) {
      send_to_char("&WYou wimp out, and attempt to flee!&n\r\n", victim);
      do_flee(victim, NULL, 0, 0);
    }
    break;
  }
	
  /* Help out poor linkless people who are attacked */
  if (!IS_NPC(victim) && !(victim->desc) && GET_POS(victim) > POS_STUNNED
      && !PLR_FLAGGED(ch, PLR_ASSASSIN)) {
		act("&M$n is rescued by divine forces.&n", FALSE, victim, 0, 0, TO_ROOM);
		GET_WAS_IN(victim) = victim->in_room;
		char_from_room(victim);
		char_to_room(victim, 0);
  }
	
  /* stop someone from fighting if they're stunned or worse */
  while ((GET_POS(victim) < POS_INCAP) && (FIGHTING(victim) != NULL)) {
    stop_fighting(victim);
    stop_fighting(ch);
    SET_BIT(PLR_FLAGS(victim), PLR_DEAD);
  }
	
	
	/* Uh oh.  Victim died. */
  if (GET_POS(victim) < POS_INCAP) {
    if ((ch != victim) && (IS_NPC(victim) || victim->desc || PLR_FLAGGED(victim, PLR_ASSASSIN))) {
			exp_gain(ch, victim);
    }
		
    if (!IS_NPC(victim)) {
      sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
							world[victim->in_room].name);
      mudlog(buf2, BRF, LVL_IMMORT, TRUE);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
				forget(ch, victim);
    }
     
		if (IS_NPC(victim)) {
			if (!MOB_FLAGGED(victim, MOB_EDIT))
				{
          local_gold = MAX(0, .1*GET_MOB_GOLD(victim));
				}
			else 
				local_gold = MAX(0, GET_GOLD(victim));
			
      if (DOUBLEGOLD)
				{
					send_to_char("&MYou gain double the gold!&n\r\n",ch);
					local_gold *= 2;
				}      
			sprintf(local_buf,"%ld", (long)local_gold);
		}
    die(victim, ch);
		
		if (PLR_FLAGGED(victim, PLR_BOUNTY)) {
			REMOVE_BIT(PLR_FLAGS(victim), PLR_BOUNTY);
			GET_GOLD(ch) = GET_GOLD(ch) + 1000000;
			sprintf(buf, "\r\n&RBOUNTY:: The bounty on %s has been collected"
							"by %s!&n\r\n", GET_NAME(victim), GET_NAME(ch));
			send_to_all(buf);
		}    
		
		
		old_gold = GET_GOLD(ch); //Ockham- To fix auto_split bug
		
    /* If Autogold enabled, get gold corpse */
		if (IS_NPC(victim) && !IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOGOLD)) {
			do_get(ch,"gold corpse",0,0);
		}
		
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {
			SET_BIT(PLR_FLAGS2(ch), PLR2_LOOTING);
			do_get(ch,"all corpse",0,0);
			REMOVE_BIT(PLR_FLAGS2(ch), PLR2_LOOTING);
		}
		
    /* If Autosac enabled, get all corpse */
    if (IS_NPC(victim) && !IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOSAC)) {
      do_sacrifice(ch,"corpse",0,0);
    }
		
		if (IS_AFFECTED(ch, AFF_GROUP)  && (local_gold > 0) 
				&& ((GET_GOLD(ch) + 100) >= (old_gold + local_gold)) &&
				PRF_FLAGGED(ch, PRF_AUTOSPLIT) && (PRF_FLAGGED(ch,PRF_AUTOLOOT) || PRF_FLAGGED(ch, 
																																											 PRF_AUTOGOLD))) {
			do_split(ch,local_buf,0,0);
		}
    return (-1);
  }
  return(dam); 
}

int weapon_type(struct char_data * ch)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);

  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    return GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0)) 
		return ch->mob_specials.attack_type + TYPE_HIT;

	return TYPE_HIT;
}

int damage_from(struct char_data * ch, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int dam = 0;
  int w_type, wpnprof;
  
  /* Find the weapon type (for display purposes only) */
  w_type = weapon_type(ch);
  
  /* Weapon proficiencies */
  if (!IS_NPC(ch) && wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    wpnprof = get_weapon_prof(ch, wielded);
  else
    wpnprof = 0;
  
  
  /* Start with the damage bonuses: the damroll and strength apply */
  if (!IS_NPC(ch)) 
    dam += LIMIT(GET_STR(ch) - 10, 1, 15);
  
  dam += GET_DAMROLL(ch);
  
  
  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON) 
    {
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      dam += wpnprof;
    }
  
  /* If no weapon, add bare hand damage instead */
  if (IS_NPC(ch)) 
    {
      if (!MOB_FLAGGED(ch, MOB_EDIT))
	{
	  if(GET_LEVEL(ch) <= 66)
	    {
	      dam = dice(
			 MIN(GET_LEVEL(ch)/4, 99),
			 MIN(GET_LEVEL(ch)/5, 30));
	      
	    }
	  else if (GET_LEVEL(ch) <= 100)
	    {
	      dam = dice(
			 MIN(GET_LEVEL(ch)/7, 99),
			 MIN(GET_LEVEL(ch)/9, 30));
	    }
	  else
	    {
	      dam = dice(
			 MIN((float)GET_LEVEL(ch)/7, 99),
			 MIN((float)GET_LEVEL(ch)* 8/90, 30));
	    }
	}
      else
	{
	  dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice); 
	}
    } 
  
  if (type == SKILL_BACKSTAB) 
    dam *= backstab_mult(ch, GET_LEVEL(ch));
  
  dam = MAX(1, dam);
  
  return dam;
} 


bool hit(struct char_data * ch, struct char_data * victim, int type)
{
  int w_type, dam = 0, diceroll;
  int level = GET_TOTAL_LEVEL(ch);

	w_type = weapon_type(ch);

  /* check if the character has a fight trigger */
  fight_mtrigger(ch);
 
  /* Do some sanity checking, in case someone flees, etc. */
  if (ch->in_room != victim->in_room) 
		{
			if (FIGHTING(ch) && FIGHTING(ch) == victim)
				stop_fighting(ch);
			return FALSE;
		}
	
  /* roll the die and take your chances... */
  diceroll = number(1, 1000);
	
  //Main Evasion Calculation
  if (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_EDIT))
		diceroll += 60 + GET_LEVEL(ch)*2; 
  else
		diceroll += GET_DEX(ch)*5 + GET_HITROLL(ch)*5 + level/2;
	
  // End Evasion Code
	
  if (AFF_FLAGGED(ch, AFF_BLIND))
		diceroll -= 300; // Percent Chance of Missing
	
  if (AWAKE(victim) && GET_POS(victim) > POS_SLEEPING && diceroll < 350)
		{
			// Miss!
			if (type == SKILL_BACKSTAB)
				damage(ch, victim, 0, SKILL_BACKSTAB);
			else
				damage(ch, victim, 0, w_type);
			return FALSE;
		}
	

	
	/* okay, we know the guy has been hit.  now calculate damage. */

	dam = damage_from(ch, type);
	
	/*
	 * Include a damage multiplier if victim isn't ready to fight:
	 *
	 * Position sitting  1.33 x normal
	 * Position resting  1.66 x normal
	 * Position sleeping 2.00 x normal
	 * Position stunned  2.33 x normal
	 * Position incap    2.66 x normal
	 * Position mortally 3.00 x normal
	 *
	 * Note, this is a hack because it depends on the particular
	 * values of the POSITION_XXX constants.
	 */
	if (GET_POS(victim) < POS_FIGHTING)
		dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
	
	if (type == TYPE_AREA_EFFECT && GET_HIT(victim) - dam < 1 && number(1,5) != 5)
		dam = GET_HIT(victim) - 1;
	
	if (type == SKILL_BACKSTAB) 
		damage(ch, victim, dam, SKILL_BACKSTAB);
	else if (type == SKILL_DOUBLE_STAB)
		damage(ch, victim, dam, SKILL_DOUBLE_STAB);
	else
		damage(ch, victim, dam, w_type);
	
	
  /* check if the victim has a hitprcnt trigger */
	hitprcnt_mtrigger(victim);
	return TRUE;
}

int extra_attacks_from_level(struct char_data *ch)
{
	int attacks = 0;

	if (IS_NPC(ch)) 
		{
			if (GET_LEVEL(ch) >= 1)
				attacks = 0;
			if (GET_LEVEL(ch) >= 20)
				attacks = 1;
			if (GET_LEVEL(ch) >= 30)
				attacks = 2;
			if (GET_LEVEL(ch) >= 50)
				attacks = 3;
		}
	else
		{
			switch (GET_CLASS(ch))
				{
				case CLASS_MAGE:
					if (GET_MAGE_LEVEL(ch) > 40)
						attacks++;
					break;
				case CLASS_CLERIC:
					if (GET_CLERIC_LEVEL(ch) > 35)
						attacks++;
					break;
				case CLASS_THIEF:
					if (GET_THIEF_LEVEL(ch) > 30)
						attacks++;
					break;
				case CLASS_WARRIOR:
					if (GET_WARRIOR_LEVEL(ch) > 25)
						attacks++;
					break;
				}
		}

	return attacks;
}

int extra_attacks_from_skill(struct char_data *ch)
{
	int percent = 0;
	int attacks = 0;
	int loop_times = 0;
	int i;

	if (GET_SKILL(ch, SKILL_MULTI_ATTACK) == 0)
		return 0;

	switch (GET_CLASS(ch))
		{
		case CLASS_MAGE:    percent = 50; loop_times = 1; break;
		case CLASS_CLERIC:  percent = 55; loop_times = 1; break;
		case CLASS_THIEF:		percent = 60;	loop_times = 2; break;
		case CLASS_WARRIOR:	percent = 70;	loop_times = 2; break;
		}

	percent += GET_SKILL(ch, SKILL_MULTI_ATTACK) * 4;

	for (i = 0; i < loop_times; i++)
		{
			if (percent > number(0,100))
				attacks++;
		}
	
	return attacks;
}

int attack_cap(struct char_data *ch, int attacks) 
{
	const int MAX_ATTACK_MAGE    = 3;
	const int MAX_ATTACK_CLERIC  = 3;
	const int MAX_ATTACK_THIEF   = 3;
	const int MAX_ATTACK_WARRIOR = 4;

	switch (GET_CLASS(ch))
		{
		case CLASS_MAGE:    return MAX(attacks, MAX_ATTACK_MAGE);	   break;
		case CLASS_CLERIC:  return MAX(attacks, MAX_ATTACK_CLERIC);	 break;
		case CLASS_THIEF:		return MAX(attacks, MAX_ATTACK_THIEF);	 break;
		case CLASS_WARRIOR:	return MAX(attacks, MAX_ATTACK_WARRIOR); break;
		}
	return 0;
}

int attacks_per_round(struct char_data *ch) 
{
	int attacks = 1;
	
	attacks += extra_attacks_from_level(ch);
	attacks += extra_attacks_from_skill(ch);


	// Fast Weapons
	if (GET_EQ(ch, WEAR_WIELD) && IS_OBJ_STAT(GET_EQ(ch, WEAR_WIELD), ITEM_FASTHIT)) 
		attacks++;
	if (GET_EQ(ch, WEAR_WIELD) && IS_OBJ_STAT(GET_EQ(ch, WEAR_WIELD), ITEM_VERYFAST)) 
		attacks++;
	
	if (IS_AFFECTED(ch, AFF_BERSERK))
		attacks++;

	if(AFF_FLAGGED(ch, AFF_SLOW))
		{
			send_to_char("&WYou feel lethargic, missing out on an attack!&n\r\n", ch);
			act("&W$n is slow and misses an attack!&n", TRUE, ch, 0, 0,TO_ROOM);
			attacks--;
		}
				
	//attacks =  attack_cap(ch,attacks);	

	if (!IS_NPC(ch) && GET_LEVEL(ch) == LVL_IMPL)
		attacks = 20;

	return attacks;
}

/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  
  int gotperc = 0, value;
  int attacks;
  struct char_data *ch;
  struct obj_data *wielded;
  bool canstab = FALSE;
	bool hit_succeded = FALSE;
	
  for (ch = combat_list; ch; ch = next_combat_list) 
		{
			next_combat_list = ch->next_fighting;
			if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room || FIGHTING(ch) == ch) 
				{
					stop_fighting(ch);
					continue;
				}
			
			if (IS_NPC(ch)) 
				{
					if (GET_MOB_WAIT(ch) > 0) 
						{
							GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
							if (AFF_FLAGGED(ch, AFF_STUNNED))      
								{
									act("&W$n is stunned and cannot fight!&n", TRUE, ch, 0, 0,TO_ROOM);
									continue;
								}
						}
					if (GET_MOB_WAIT(ch) < 0) 
						GET_MOB_WAIT(ch) = 0;
					
					if (GET_POS(ch) < POS_FIGHTING && GET_POS(ch) > POS_STUNNED) 
						{
							GET_POS(ch) = POS_FIGHTING;
							act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
						}
					if (AFF_FLAGGED(ch, AFF_STUNNED) && GET_MOB_WAIT(ch) == 0)
						{
							act("&W$n is no longer stunned!&n", TRUE, ch, 0, 0, TO_ROOM);
							REMOVE_BIT(AFF_FLAGS(ch), AFF_STUNNED);
						}
					if (AFF_FLAGGED(ch, AFF_SLOW) && GET_MOB_WAIT(ch) == 0)
						{
							act("&W$n is no longer slowed!&n", TRUE, ch, 0, 0, TO_ROOM);
							REMOVE_BIT(AFF_FLAGS(ch), AFF_SLOW);
						}
			
					while ((FIGHTING(ch) && (GET_HIT(FIGHTING(ch)) <= 0))) 
						{
							stop_fighting(FIGHTING(ch));
							stop_fighting(ch);
						}
				}
		
			if (GET_POS(ch) < POS_FIGHTING) 
				{
					if (GET_POS(ch) == POS_SLEEPING) 
						{
							GET_POS(ch) = POS_FIGHTING;
							affect_from_char(ch, SPELL_SLEEP);
							act("$n is awoken by the brutal onslaught.", TRUE, ch, 0, 0,TO_ROOM);
							send_to_char("You are awoken by the brutal onslaught.", ch);
						}
					else  
						send_to_char("You can't fight while sitting!!\r\n", ch);
      
					continue;
				}

			if(AFF_FLAGGED(ch, AFF_STUNNED))
				{
					send_to_char("&WYou can't fight while stunned!&n\r\n", ch);
					act("&W$n is stunned and cannot fight!&n", TRUE, ch, 0, 0,TO_ROOM);
					continue;
				}		

			/* reset to 1 attack */
			attacks = attacks_per_round(ch);
	
			gotperc = GET_SKILL(ch, SKILL_DOUBLE_STAB)*.5;
			wielded = GET_EQ(ch, WEAR_WIELD);
			
			if (wielded)
				{
					value = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
					if (value == TYPE_STAB  || value == TYPE_PIERCE)
						{
							canstab = TRUE;
						}
				}
			
			for (; attacks > 0 && FIGHTING(ch); attacks--)
				{
					hit_succeded = hit(ch, FIGHTING(ch), TYPE_UNDEFINED); 
					//if an attack succeeds you can possibly get a double stab.
					if (hit_succeded && canstab && gotperc > number(0,100) && FIGHTING(ch))
							hit(ch, FIGHTING(ch), SKILL_DOUBLE_STAB);
				}
			
			if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
				(mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
		}
}

/* Weapon Proficiecies -- (C) 1999 by Fabrizio Baldi */
int get_weapon_prof(struct char_data *ch, struct obj_data *wield)
{
  int value = 0, bonus = 0, type = -1;

  /* No mobs here */
  if (IS_NPC(ch))
    return (bonus);

  value = GET_OBJ_VAL(wield, 3) + TYPE_HIT;
  switch (value) {
  case TYPE_SLASH:
    type = SKILL_WEAPON_SWORDS;
    break;

  case TYPE_STING:
  case TYPE_PIERCE:
  case TYPE_STAB:
    type = SKILL_WEAPON_DAGGERS;
    break;

  case TYPE_THRASH:
  case TYPE_WHIP:
    type = SKILL_WEAPON_WHIPS;
    break;

  case TYPE_CLAW:
    type = SKILL_WEAPON_TALONOUS_ARMS;
    break;

  case TYPE_BLUDGEON:
  case TYPE_MAUL:
  case TYPE_POUND:
  case TYPE_CRUSH:
    type = SKILL_WEAPON_BLUDGEONS;
    break;

  case TYPE_HIT:
  case TYPE_PUNCH:
  case TYPE_BITE:
  case TYPE_BLAST:
    type = SKILL_WEAPON_EXOTICS;
    break;

  default:
    type = -1;
    break;
  }

  if (type != -1) {
    return (2.5*GET_SKILL(ch, type));
  }

  return 0;
}



/* calculate factorial of a number */
int fact(int num)
{
  int i, sum;

  sum = 0;
  if (num == 0)
    return 0;
  else {
    for(i = 1; (i <= num); i++)
      sum += i;
    return sum;
  }
}

int calc_ave_damage(struct char_data *ch, struct char_data *vict)
{
  int damage = 0, try = 0;
  
  while (damage == 0 && try  < 10)
  {
    damage = damage_from(ch, TYPE_UNDEFINED);
    try++;
  }
  return(damage);
}

bool CAN_FIGHT(struct char_data *ch, struct char_data *vict)
{
  if (!ch || !vict)                 return FALSE;
  if (!CAN_SEE(ch, vict))           return FALSE;
  if (ch->in_room != vict->in_room) return FALSE;
  if (FIGHTING(ch))                 return FALSE;
  if (GET_POS(ch) < POS_STANDING)   return FALSE;
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
     return FALSE;

  if (!CAN_ASSASSIN(ch, vict)) return FALSE;
  return TRUE;
}
  
bool CAN_ASSASSIN(struct char_data *ch, struct char_data *victim)
{
	if (ch == victim) return TRUE;

	if (GET_POS(victim) == POS_DEAD) return FALSE;
	
	if (IS_NPC(victim) || IS_NPC(ch)) return TRUE;
	
	if (GET_LEVEL(ch) >= LVL_DEITY) return TRUE;
	
	if (ROOM_FLAGGED(ch->in_room, ROOM_SPARRING)) return TRUE;
	
	if (!PLR_FLAGGED(victim, PLR_ASSASSIN) || !PLR_FLAGGED(ch, PLR_ASSASSIN))
		return FALSE;
	if (!CANASSASSINATE)
		return FALSE;
	
  return TRUE;
}

bool CAN_STEAL(struct char_data *ch, struct char_data *victim)
{
   if (IS_NPC(victim) || IS_NPC(ch)) return TRUE;

   if (GET_LEVEL(ch) >= LVL_DEITY) return TRUE;

   if (ROOM_FLAGGED(ch->in_room, ROOM_SPARRING)) return FALSE;

   if (!PLR_FLAGGED(victim, PLR_ASSASSIN) || !PLR_FLAGGED(ch, PLR_ASSASSIN))
      return FALSE;
   if (!CANASSASSINATE)
      return FALSE;
 
  return TRUE;
}

