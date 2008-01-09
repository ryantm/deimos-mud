/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern int pk_allowed;
extern const char *shot_types[];
extern int *shot_damage[];

/* extern functions */
void raw_kill(struct char_data * ch, struct char_data * killer);
void check_killer(struct char_data * ch, struct char_data * vict);
int compute_armor_class(struct char_data *ch);
void improve_skill(struct char_data *ch, int skill);
bool CAN_FIGHT(struct char_data *ch, struct char_data *vict);
int damage_from(struct char_data * ch, int type);

/* local functions */
ACMD(do_assist);
ACMD(do_hit);
ACMD(do_kill);
ACMD(do_backstab);
ACMD(do_order);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_rescue);
ACMD(do_kick);
ACMD(do_disarm);
ACMD(do_push);
ACMD(do_stab);
ACMD(do_throw);
ACMD(do_trip);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else if (IS_NPC(helpee))
    send_to_char("You can only assist players!\r\n", ch);
  else {
    /*
     * Hit the same enemy the person you're helping is.
     */
    if (FIGHTING(helpee))
      opponent = FIGHTING(helpee);
    else
      for (opponent = world[ch->in_room].people;
	   opponent && (FIGHTING(opponent) != helpee);
	   opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if ((!CAN_ASSASSIN(ch, opponent)))
      send_to_char("You may not assist an assassin.\r\n", ch);
    else if (!pk_allowed && !IS_NPC(opponent))	/* prevent accidental pkill */
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (PLR_FLAGGED2(ch, PLR2_REVIEW))
    send_to_char("You cannot attack as a review character.\r\n", ch);
  else if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY)
    send_to_char("You cannot attack as an immortal.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else if (IS_NPC(vict) && MOB_FLAGGED(vict, MOB_NOHIT))
    send_to_char("You may not attack these mobs!\r\n", ch);  
  else {
		if (!IS_NPC(vict) && !IS_NPC(ch)) {
			if (subcmd != SCMD_MURDER) {
				send_to_char("Use 'murder' to hit another player.\r\n", ch);
				return;
			}
			if (CAN_ASSASSIN(ch, vict)) 
				{
					if (FIGHTING(ch)) return;
					hit(ch, vict, TYPE_UNDEFINED);
					return;
					}
			else {
				send_to_char("You cannot attack another player!\r\n", ch);
				return;
			}
		}
		if (AFF_FLAGGED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
			return;			/* you can't order a charmed pet to attack a
									 * player */
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}



ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}



ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob, apr;
 
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_BACKSTAB)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (!CAN_ASSASSIN(ch, vict)) {
    send_to_char("You can't backstab them!\r\n", ch);
    return;
  }

  if (GET_HIT(vict) != GET_MAX_HIT(vict)) {
    send_to_char("You cannot backstab an injured person!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  apr = 1;
	
	if (GET_THIEF_LEVEL(ch) >= 30)
	  apr++;
	if (GET_THIEF_LEVEL(ch) >= 45)
	  apr++;
		
	if (GET_CLASS(ch) == CLASS_MAGE    && apr > 1) apr = 1;
	if (GET_CLASS(ch) == CLASS_CLERIC  && apr > 1) apr = 1;
	if (GET_CLASS(ch) == CLASS_WARRIOR && apr > 2) apr = 2;
	if (GET_CLASS(ch) == CLASS_THIEF   && apr > 3) apr = 3;
	
	//If fast do another. Very Fast not included becaust that would be crazy.
	if (GET_EQ(ch, WEAR_WIELD) && IS_OBJ_STAT(GET_EQ(ch, WEAR_WIELD), ITEM_FASTHIT)) apr++;
		
	apr = MAX(0, MIN(apr, 6));

	if (GET_LEVEL(ch) == LVL_IMPL) apr = 20;

	prob = 64 + 3.5 * GET_SKILL(ch, SKILL_BACKSTAB);
		
	for (; apr >= 1; apr--)
		{
			percent = number(1,100);
			
			if (percent < prob)
				{
					//Success
					hit(ch, vict, SKILL_BACKSTAB);
				}
			else
				{
					//Failure
          if (ch->in_room == vict->in_room) {
						act("&G$N attempts to backstab you, but stabs $Mself instead.&n"  , FALSE, vict, 0, ch, TO_CHAR);
						act("&RYou miss a backstab and stab your finger instead!&n"     , FALSE, vict, 0, ch, TO_VICT);
						act("&Y$N attempts to backstab $n but stabs $Mself instead.&n" , FALSE, vict, 0, ch, TO_NOTVICT);
						damage(ch, ch, damage_from(ch, SKILL_BACKSTAB) * 0.5, TYPE_HIT);
          }
				}
		}
  WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
}

ACMD(do_stab)
{
  struct char_data *vict;
  int percent, prob;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_STAB)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }

  one_argument(argument, buf);

  if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
    send_to_char("Stab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("Yeah right...\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for stabbing.\r\n", ch);
    return;
  }
  if (AWAKE(vict) && (GET_POS(vict) != POS_FIGHTING)) {
    hit(vict, ch, SKILL_STAB);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = 60;

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_STAB);
//  improve_skill(ch, SKILL_STAB);
  }
  else
    hit(ch, vict, SKILL_STAB);
  WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
}


ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  bool found = FALSE;
  room_rnum org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_vis(ch, name, FIND_CHAR_ROOM)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (AFF_FLAGGED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !AFF_FLAGGED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(OK, ch);
	command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (AFF_FLAGGED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(OK, ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss;
  struct char_data *was_fighting;
  struct affected_type *af = NULL;

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char("You are in pretty bad shape, unable to flee!\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(ch, AFF_BERSERK))
  {
    send_to_char("You MUST continue fighting!!!! &rBLOOOD!?!?!&n\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(ch, AFF_WEB)) {
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
    send_to_char("You struggle in gluey webs!\r\n", ch);
    act("$n struggles in gluey webs!", TRUE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE / 2.);
   }
    return;
  }
  
  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH) &&
         
         (!(SECT(EXIT(ch, attempt)->to_room) == SECT_FLYING) || 
          AFF_FLAGGED(ch, AFF_FLOAT))
       ) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      was_fighting = FIGHTING(ch);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char("You flee head over heels.\r\n", ch);
	if (was_fighting && !IS_NPC(ch)) 
        {
	  loss = MAX(0, GET_TOTAL_LEVEL(ch) * 10000 / 240);
	  gain_exp(ch, -1 * MIN(MAX(1, loss), 10000));
	}
      } else 
      {
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;
  int skill, dam;

  skill = GET_SKILL(ch, SKILL_BASH);
  one_argument(argument, arg);

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_BASH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_NOBASH)) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("You're right to attack has been REVOKED.\r\n", ch);
    return;
  }
  if (!CAN_ASSASSIN(ch, vict)) {
    send_to_char("You can't bash an non-assassin!\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = 60 +3.7 * skill;


  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) 
  {
    damage(ch, vict, 0, SKILL_BASH);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);    
    return;
  } else {
    /*
     * If we bash a player and they wimp out, they will move to the previous
     * room before we set them sitting.  If we try to set the victim sitting
     * first to make sure they don't flee, then we can't bash them!  So now
     * we only set them sitting if they didn't flee. -gg 9/21/98
     */
    dam = damage_from(ch, SKILL_BASH);
    if (damage(ch, vict, (skill/5.) * dam, SKILL_BASH) > 0) 
    {	/* -1 = dead, 0 = miss */
      if (skill > 0 && skill < 5)
         WAIT_STATE(vict, PULSE_VIOLENCE * 1);
      else if (skill >= 6 && skill <= 10 && skill > number(0,100))
         WAIT_STATE(vict, PULSE_VIOLENCE * 2);
      else
         WAIT_STATE(vict, PULSE_VIOLENCE * 1);

      if (IN_ROOM(ch) == IN_ROOM(vict))
      { 
         SET_BIT(AFF_FLAGS(vict), AFF_STUNNED);
      }
    }
  }
  //improve_skill(ch, SKILL_BASH);
  WAIT_STATE(ch, PULSE_VIOLENCE * 4);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_RESCUE)) {
    send_to_char("You have no idea how to do that.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY) {
    send_to_char("You cannot assist mortals.\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't rescue a mob!\r\n", ch);
    return;
  }


  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  switch(GET_CLASS(ch))
  {
   case CLASS_MAGE: prob = 70; break;
   case CLASS_CLERIC: prob = 75; break;
   case CLASS_THIEF:  prob = 85; break;
   case CLASS_WARRIOR: prob = 95; break;
   default: prob = 0;break;
  }

  if (percent > prob) {
    send_to_char("You fail the rescue!\r\n", ch);
    return;
  }

  send_to_char("Banzai!  To the rescue...\r\n", ch);
  act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

  if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
  if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
  if (FIGHTING(ch))
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
}

ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;
  int dam = 0, skill = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_KICK)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  one_argument(argument, arg);
  skill = GET_SKILL(ch, SKILL_KICK);

  if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!CAN_ASSASSIN(ch, vict))
  {
    send_to_char("You can't kick a non-assassin!\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }

  /* 101% is a complete failure */
  percent = number(1, 101);
  prob = 60 + 3.5*skill;

  if (GET_CLASS(ch) == CLASS_WARRIOR)
  prob += 10;
 
  if (GET_CLASS(ch) == CLASS_THIEF)
  prob += 5;
 
  if (GET_CLASS(ch) == CLASS_MAGIC_USER)
  prob -= 4;
 
  if (GET_CLASS(ch) == CLASS_CLERIC)
  prob += 0;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK);
  } else
  {
    dam = damage_from(ch, SKILL_KICK);
    damage(ch, vict, dam , SKILL_KICK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}

ACMD(do_disarm)
{
  struct obj_data *obj;
  struct char_data *vict;
  int dam = 0;

  one_argument(argument, buf);

  if (!*buf) {
        send_to_char("Whom do you want to disarm?\r\n", ch);
  return;
  }
  else if (!(vict = get_char_room_vis(ch, buf))) {
        send_to_char(NOPERSON, ch);
  return;
  }
  else if (!GET_SKILL(ch, SKILL_DISARM)) {
        send_to_char("You have no idea how.\r\n", ch);
  return;
  }
  else if (vict == ch) {
        send_to_char("That would be funny to watch.\r\n", ch);
  return;
  }
  else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict)) {
        send_to_char("The thought of disarming your master makes you weep.\r\n", ch);
  return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
  return;
  }
  if(!CAN_ASSASSIN(ch, vict))
  {
    send_to_char("You can't disarm an non-assassin!\r\n", ch);
  return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }
  else if (!(obj = GET_EQ(vict, WEAR_WIELD))) {
        act("$N is unarmed!", FALSE, ch, 0, vict, TO_CHAR);
    }
  else if (IS_SET(GET_OBJ_EXTRA(obj), ITEM_NODISARM) ||
      MOB_FLAGGED(vict, MOB_NOBASH) ||
           (number(1, 101) > (!IS_NPC(ch) ? 
             60 +3.5*GET_SKILL(ch, SKILL_DISARM) : number(0, 100)))) {
        act("You failed to disarm $N and make $M rather angry!", FALSE, ch, 0, vict, TO_CHAR);
        dam = damage_from(vict, TYPE_HIT);
        damage(vict, ch, number(1, dam), TYPE_HIT);
    }
  else if (dice(2, GET_STR(ch)) < dice(2, GET_STR(vict))) {
        act("Your hand just misses $N's weapon, failing to disarm $M/", FALSE, ch, 0, vict, TO_CHAR);
        act("$N has tried and failed to disarm you!", FALSE, vict, 0, ch, TO_CHAR);
        dam = damage_from(vict, TYPE_HIT);
        damage(vict, ch, number(1, dam), TYPE_HIT);
  } else {
        obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
        act("You rip $N's weapon from $S hands!", FALSE, ch, 0, vict, TO_CHAR);
        act("$N smashes your weapon from your hands!", FALSE, vict, obj, ch, TO_CHAR);
        act("$n disarms $N!", FALSE, ch, obj, vict, TO_ROOM);
  }
  hit(vict , ch, TYPE_UNDEFINED);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void log_push_to_death( struct char_data * ch, struct char_data *attacker)
  {
    extern struct room_data *world;
    sprintf( buf, "%s pushed to Death Trap #%d (%s)", GET_NAME(ch),
             world[ch->in_room].number, world[ch->in_room].name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    sprintf(buf, "Pushed by %s", GET_NAME(attacker) );
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
  }

int perform_push(struct char_data *ch, int dir, int need_specials_check,
              struct char_data *attacker )
  {
    extern const char *dirs[];
    int was_in;
    int House_can_enter(struct char_data * ch, sh_int house);
    void death_cry(struct char_data * ch);
    int special(struct char_data *ch, int cmd, char *arg);
    /*int escape = GET_STR(ch) + 15;*/
    struct affected_type *af = NULL;

    if (need_specials_check && special(ch, dir + 1, ""))
      return 0;

    /* charmed? */
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {
      send_to_char("The thought of leaving your master makes you weep.\r\n.\r\n", ch);
      act("$n bursts into tears at the thought of leaving $s master.", FALSE, ch, 0, 0, TO_ROOM);
      return 0;
    }

    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_ATRIUM)) {
      if (!House_can_enter(ch, world[EXIT(ch, dir)->to_room].number)) {
        send_to_char("You are pushed, but you can't trespass!\r\n", ch);
        return 0;
      }
    }
    if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_NOPUSH)) 
    {
        send_to_char("You can't push people into that room.!\r\n", ch);
        return 0;
    }
    if (IS_SET(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
        world[EXIT(ch, dir)->to_room].people != NULL) {
      send_to_char("You are pushed, but there isn't enough room\r\n", ch);
      return 0;
    }
    if (IS_AFFECTED(ch, AFF_WEB))
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
    WAIT_STATE(attacker, PULSE_VIOLENCE / 2.);
    return (0);   
}
   }	
    
    sprintf(buf2, "$n is pushed to the %s by $N.",
            dirs[dir] );
    act(buf2, TRUE, ch, 0, attacker, TO_NOTVICT);
    was_in = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, world[was_in].dir_option[dir]->to_room);

    if (!IS_AFFECTED(ch, AFF_SNEAK))
      act("$n falls rolling on the ground!", TRUE, ch, 0, 0, TO_ROOM);

    if (ch->desc != NULL)
      look_at_room(ch, 0);

    if (IS_SET(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT)
    {
      log_push_to_death(ch, attacker);
      death_cry(ch);
      extract_char(ch);
      return 0;
    }
    return 1;
  }


ACMD(do_push)
  {
    char name[100], todir[256];
    int to;
    struct char_data *victim=NULL;
    extern const char *dirs[];

    half_chop(argument, name, todir);
    if (!*name || !*todir)
      send_to_char("Push whom where?\r\n", ch);
    else if (!(victim = get_char_room_vis(ch, name)) )
      send_to_char("No one by that name here.\r\n", ch);
    else if (ch == victim)
      send_to_char("But...can't you walk?\r\n", ch);
    else if (IS_AFFECTED(ch, AFF_CHARM))
      send_to_char("No, no....\r\n", ch);
    else if ((to = search_block(todir, dirs, FALSE)) < 0) {
      send_to_char( "That is not a direction.\r\n", ch );
      return;
    } else {
      strcpy( todir, dirs[to] );
      if (GET_POS(victim) <= POS_SITTING && GET_POS(victim) > POS_DEAD) {
      send_to_char( "You can't push anyone that is in a prone position.", ch );
        return;
      }
      if (GET_POS(victim) == POS_FIGHTING) {
        sprintf( buf, "No! you can't push %s while fighting!\r\n", HESH(ch) );
        send_to_char( buf, ch );
        return;
      }
      sprintf(buf, "$n is trying to push you to the %s!", todir);
      act( buf, FALSE, ch, 0, victim, TO_VICT );
      act( "$n is trying to push $N", FALSE, ch, 0, victim, TO_NOTVICT);
      if (!CAN_GO( victim, to)) {
        sprintf( buf, "Can't push %s there\r\n", HMHR(ch) );
        send_to_char( buf, ch );
      } else if ( GET_LEVEL(victim) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_IMMORT )
  {
        send_to_char( "Oh, no, no, no.\r\n", ch );
        sprintf( buf, "%s is trying to push you...what a mistake!\r\n", GET_NAME(ch) );
        send_to_char( buf, victim);
      } else if (IS_NPC(victim) && GET_LEVEL(victim) - GET_LEVEL(ch) > 5) {
        sprintf( buf, "You can't push %s.\r\n", HMHR(victim) );
        send_to_char( buf, ch );
        sprintf( buf, "%s can't push you.\r\n", GET_NAME(ch) );
        send_to_char( buf, victim );
      } else if ( MOB_FLAGGED(victim, MOB_NOPUSH)) {
        send_to_char( "Ouch! Too big for you!\r\n", ch );
      } else if ((dice(1,20)+3)-(GET_STR(ch)-GET_STR(victim)) < GET_STR(ch)) {
       /* You can balance the check above, this works fine for me */
        if (perform_push(victim, to, TRUE, ch)) {
          sprintf(buf, "You successfully push %s %s!\r\n", HMHR(victim),todir);
          send_to_char(buf, ch);
          sprintf(buf, "%s has pushed you!", GET_NAME(ch));
          send_to_char(buf, victim);
        }
      } else {
        send_to_char( "Oops...you fail.", ch );
        sprintf( buf, "%s fails.\r\n", HESH(ch) );
        *buf = UPPER(*buf);
        send_to_char( buf, victim );
      }
    }
  }

ACMD(do_throw)
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];

 struct obj_data *missile;
 struct char_data *vict;

 int num1 = 0;
 int num2 = 0;
 int roll = 0;
 int dmg = 0;

 two_arguments(argument, arg1, arg2);

 if (!*arg1 || !*arg2) {
      send_to_char("Usage: throw <item> <victim>",ch);
      return;
 }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
     send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
     return;
 }
 if (!GET_SKILL(ch, SKILL_SHOOT)) {
     send_to_char("You aren't trained in throwing crap.",ch);
     return;
 }

 missile = GET_EQ(ch, WEAR_HOLD);

 if (!missile) {
     sprintf(buf2, "You aren't holding %s!\r\n", arg1);
     send_to_char(buf2, ch);
     return;
 }

 if ((GET_OBJ_TYPE(missile) != ITEM_MISSILE)) {
     send_to_char("You shouldn't throw that!\r\n",ch);
     return;
  }

 if (!(vict = get_char_vis(ch, arg2, FIND_CHAR_ROOM))) {
  sprintf(buf, "There doesn't seem to be a %s here.",arg2);
  send_to_char(buf, ch);
  return;
  }

  if ((!*arg2) || (!vict)) {
   act("Who are you trying to throw $p at?", FALSE, ch, missile, 0, TO_CHAR);
   return;
  }
  if (!CAN_ASSASSIN(ch, vict)) {
    send_to_char("You can't disarm a non-assassin!\r\n", ch);
    return;
  }

  num1 = GET_DEX(ch) - GET_DEX(vict);
 if (num1 > 25) 
  num1 = 25;
 if (num1 < -25) 
  num1 = -25;

  num2 = 60;
  num2 = (int)(num2 * .75);

 if (num2 < 1) num2 = 1;
  roll = number(1, 101);

 if ((num1+num2) >= roll) {
  sprintf(buf, "You hit $N with %s!", arg1);
        act(buf, FALSE, ch, missile, vict, TO_CHAR);
        sprintf(buf, "$n hit you with %s!", arg1);
        act(buf, FALSE, ch, missile, vict, TO_VICT);
        sprintf(buf, "$n hit $N with %s!", arg1);
        act(buf, FALSE, ch, missile, vict, TO_NOTVICT);

  dmg = GET_OBJ_WEIGHT(missile) * (GET_LEVEL(ch)/3);
  damage(ch, vict, dmg, TYPE_UNDEFINED);
  extract_obj(missile);

 } else {
   sprintf(buf, "You throw %s at $N and miss!", arg1);
   act(buf, FALSE, ch, missile, vict, TO_CHAR);
        sprintf(buf, "$n throws %s at you and misses!", arg1);
   act(buf, FALSE, ch, missile, vict, TO_VICT);
   sprintf(buf, "$n throws %s at $N and misses!", arg1);
        act(buf, FALSE, ch, missile, vict, TO_NOTVICT);
  }
}

ACMD(do_tether)
{
 //struct char_data *vict;
  int perc = number(1, 100);
  int manacost = 100, skill = GET_SKILL(ch, SKILL_TETHER);
  int maxlocations = MAX(0, skill / 3 - 1);
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

    
    
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_TETHER)) 
  {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if(skill < 3)  
    manacost = 100;
  else if(skill < 9)
    manacost = 150;
  else if(skill < 10)
    manacost = 200;
  
  if (GET_LEVEL(ch) > LVL_IMMUNE)  manacost = 0;

  if(GET_MANA(ch) < manacost)
  {
    send_to_char("You lack the mana to tether something.\r\n", ch);
    return;
  }

  if (perc > 3.5*GET_SKILL(ch, SKILL_TETHER) + 60)
  {
    send_to_char("Your incantation failed!\r\n", ch);
    GET_MANA(ch) -= manacost;
    return;
  }
  
  two_arguments(argument, arg1, arg2);
  if(!*arg1)
  {  
    send_to_char("No tether arguement supplied.\r\n", ch);
    return;
  }

  if(strcmp("cast", arg1) == 0 && *arg2)
  {
    if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOMAGIC) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOESCAPE))
    {
      send_to_char("Your tether fails to stick to the room.\r\n", ch); 
      return;
    }
    if (atoi(arg2) > maxlocations || atoi(arg2) < 0)
    {
     send_to_char("Invalid Memorization Slot.\r\n", ch);
     return;
    }

    send_to_char("You sit on the floor chanting, tethering the room to yourself.\r\n", ch);
    act("$n tethers $mself to the room!", FALSE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "Room Slotted to %d memorized location.\r\n", atoi(arg2));
    send_to_char(buf, ch);
    
    GET_TETHER(ch, atoi(arg2)) = &world[IN_ROOM(ch)].number;

    GET_MANA(ch) -= manacost;
    return;
  }

  if (strcmp("find", arg1) == 0 && *arg2)
  {
    if (atoi(arg2) > maxlocations || atoi(arg2) < 0)
    {
     send_to_char("Invalid memorization slot.\r\n", ch);
     return;
    }

    if(!GET_TETHER(ch, atoi(arg2)))
    {
     send_to_char("You have never tethered yourself to anything!\r\n", ch);
     return;
    } 
     sprintf(buf, "You are tethered to %s.\r\n", world[real_room(*GET_TETHER(ch, atoi(arg2)))].name);
     send_to_char(buf, ch);
     GET_MANA(ch) -= manacost;
     return;   
  }

  if (strcmp("return", arg1) == 0 && *arg2)
  {
    if (atoi(arg2) > maxlocations || atoi(arg2) < 0)
    {
     send_to_char("Invalid Memorization Slot.\r\n", ch);
     return;
    }

   if(!GET_TETHER(ch, atoi(arg2))) 
   {
     send_to_char("You have never memorized anything in that slot!\r\n", ch);
     return;
   }

   if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOMAGIC) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOESCAPE))
   {
     send_to_char("You cannot return from here!\r\n", ch);
     return;
   }

   send_to_char("You are pulled back to your tether point. (Tether Reset)\r\n", ch);
   act("$n flies out of the room trailing a tether", FALSE, ch, 0, 0, TO_ROOM);

   char_from_room(ch);
   char_to_room(ch, real_room(*GET_TETHER(ch, atoi(arg2))));
   act("A tether retracts into the room as $n flys in behind.", FALSE, ch, 0, 0, TO_ROOM);
   command_interpreter(ch, "look");
   GET_TETHER(ch, atoi(arg2)) = NULL;
   GET_MANA(ch) -= manacost;
   return;
  }
  send_to_char("Invalid tether command.\r\n", ch);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_whirlwind)
{
  struct char_data *tch = NULL, *next_tch;
  int dam = 0; int skill;

  int percent, prob;
  percent = number(1, 100);

  skill = GET_SKILL(ch, SKILL_WHIRLWIND);
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_WHIRLWIND)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  prob = 60+ 3.5 * skill;

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }
  if (
     (5 + prob < percent && GET_CLASS(ch) == CLASS_WARRIOR) ||
     (2 + prob < percent && GET_CLASS(ch) == CLASS_THIEF) ||
     (0 + prob  < percent && GET_CLASS(ch) == CLASS_CLERIC) ||
     (prob - 2 < percent && GET_CLASS(ch) == CLASS_MAGIC_USER)
     )
     {
       send_to_char("Your attempt to whirlwind failed.\r\n", ch);
       return;
     }
int hitcount = 0;
int canhit = skill / 2 + 3;

  send_to_char("You whirl about the room in a flurry of attacks.\r\n", ch);
  for (tch = world[ch->in_room].people; tch && hitcount < canhit; tch = next_tch) {
    next_tch = tch->next_in_room;
    if (tch == ch) continue;
    if (GET_LEVEL(tch) >= LVL_IMMORT) continue;
    if (!CAN_ASSASSIN(ch, tch)) continue;
    if (IS_NPC(tch) && MOB_FLAGGED(tch, MOB_NOBASH)) continue;

    hitcount++;    
  percent = number(1,90);
  if (
     (5 + prob < percent && GET_CLASS(ch) == CLASS_WARRIOR) ||
     (2 + prob < percent && GET_CLASS(ch) == CLASS_THIEF) ||
     (0 + prob  < percent && GET_CLASS(ch) == CLASS_CLERIC) ||
     (prob - 2 < percent && GET_CLASS(ch) == CLASS_MAGIC_USER)
     )
     {
       sprintf(buf, "You whirl right past %s with a big WHIFF.\r\n", GET_NAME(tch));
       send_to_char(buf, ch);
       continue;
     }


    sprintf(buf, "Your flurry of attacks whirls into %s.\r\n", GET_NAME(tch));
    send_to_char(buf, ch);
    act("You are caught up in a whirlwind of attacks from $n !", 0, ch, 0, tch, TO_VICT);
    act("$N is caught up in a flurry of attacks from $n.", FALSE, ch, 0, tch, TO_ROOM);
    dam = damage_from(ch, TYPE_HIT);

    if (FIGHTING(tch) == ch)
      damage(ch,tch, 2.*dam, TYPE_AREA_EFFECT);
    else
      hit(ch, tch, TYPE_AREA_EFFECT);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_trip)
{
  struct char_data *vict;
  int percent, prob;
  int skill;
  one_argument(argument, arg);

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_TRIP)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  skill = GET_SKILL(ch, SKILL_TRIP);

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Trip who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (MOB_FLAGGED(vict, MOB_NOBASH)) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_DEITY &&
    !PLR_FLAGGED2(ch, PLR2_REALIMM)) {
    send_to_char("Your right to attack has been REVOKED.\r\n", ch);
    return;
  }
  if (!CAN_ASSASSIN(ch, vict))
  {
    send_to_char("You can't trip a non-assassin!\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = 60 +3.5*skill;

  if (GET_CLASS(ch) == CLASS_THIEF) /* Warriors are good bashers */
  prob += 5;

  if (GET_CLASS(ch) == CLASS_WARRIOR) /* Thieves are good bashers */
  prob += 0;

  if (GET_CLASS(ch) == CLASS_MAGIC_USER) /* mage have sticks */
  prob -= 20;

  if (GET_CLASS(ch) == CLASS_CLERIC) /* clerics be wimpy men */
  prob -= 10;

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) 
  {
    damage(ch, vict, 0, SKILL_TRIP);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);    
    return;
  } else {
      damage(ch, vict, 3. * skill, SKILL_TRIP);

      if (skill == 10 && number(0, 100) > 85)
         WAIT_STATE(vict, PULSE_VIOLENCE * 2);      
      else
         WAIT_STATE(vict, PULSE_VIOLENCE * 1);      
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


