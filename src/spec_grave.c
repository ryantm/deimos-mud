/*
***********************************************************************
*  FILE: spec_grave.c                                                 * 
*  USAGE: Specials module for areas.                                  *
*                                                                     *
*  All rights reserved, see license.doc for more information.         *
*                                                                     *
*  This file is based on Diku-MUD Copyright (C) 1990, 1991.           *
***********************************************************************
*/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/*   external vars  */

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern HOMETOWN hometowns[];

/********************************************************************/

#define PULSE_EVENT 	PULSE_MOBILE

ACMD(do_get);
ACMD(do_bury);
ACMD(do_mpoload);

SPECIAL(grave_undertaker)
{
  if (cmd) return FALSE;
  if (!AWAKE(ch))
    return(FALSE);
  
  if (GET_STR(ch) < 25)
    GET_STR(ch) = 25;
  do_get(ch,"corpse", 0, 0);
  do_bury(ch,"corpse", 0, 0);
  return FALSE;
}


SPECIAL(grave_demilich)
{
  ACMD(do_look);
  struct char_data *victim;

  if (!AWAKE(ch) || !FIGHTING(ch))
    return FALSE;

  for(victim = world[ch->in_room].people; victim; victim = victim->next_in_room)
    if ((!IS_NPC(victim))&&(FIGHTING(victim) == ch) && GET_LEVEL(victim) < LVL_IMMORT)
		  /* person is a player who is fighting demlich */
      if (GET_LEVEL(victim) >= number(0,75)) { 
		  /* demilich wants high level victims */
        if (GET_LEVEL(victim) >= number(0,75))
	          /* high level more likely to resist */
	{
	  act("$n tried to capture your soul.",FALSE,ch,0,victim,TO_VICT);
	  act("$n gives $N a icy cold stare.",FALSE,ch,0,victim,TO_NOTVICT);
	  return FALSE;
	}
	else
	{
	  act("$n sucks you into one of his gems.",FALSE,ch,0,victim,TO_VICT);
	  act("$N disappears into one of $n's eyes.",FALSE,ch,0,victim,TO_NOTVICT);
	  act("You trap $N in one of your gems.",FALSE,ch,0,victim,TO_CHAR);
          send_to_char("Your soul is trapped within the demilich.\r\n",victim);
	  send_to_char("Slowly you feel your life-force drain away...\r\n",victim);
	  GET_HIT(victim) = 1;
	  GET_MOVE(victim) = 0;
	  GET_MANA(victim) = 0;
	  char_from_room(victim);
	  char_to_room(victim, real_room(hometowns[GET_HOME(victim)].magic_room));
	  do_look(victim,"",15, 0);
	  return TRUE;
	}
      }
  return FALSE;
}


SPECIAL(grave_ghoul)
{
  if (!FIGHTING(ch)) return (FALSE);

  if (!number(0,5)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), NULL, 0, SPELL_PARALYZE, GET_LEVEL(ch),CAST_SPELL);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(grave_priest)
{
  ACMD(do_say);
  struct char_data *vict;
  char buf[MAX_STRING_LENGTH];
  long num_in_room = 0, vict_num;

  if (number(0,4) || !AWAKE(ch)) return (FALSE);

  if (FIGHTING(ch)) {
    do_say(ch, "You are commiting blasphemy!", 0, 0);
    return (FALSE);
  }

  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (!IS_NPC(vict) && CAN_SEE(ch, vict)) num_in_room++;

  if (!num_in_room) return (FALSE);

  vict_num = number(1,num_in_room);

  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
    if (!IS_NPC(vict) && CAN_SEE(ch, vict)) vict_num--;
    if (!vict_num && !IS_NPC(vict)) break;
  }
  if (!vict) return (FALSE);

  if (IS_GOOD(ch) && IS_GOOD(vict)) {
    sprintf(buf, "You, %s, are blessed.", GET_NAME(vict));
    do_say(ch, buf, 0, 0);
    call_magic(ch, FIGHTING(ch), NULL, 0, SPELL_BLESS, GET_LEVEL(ch),CAST_SPELL);
  } else if (IS_EVIL(ch) && IS_EVIL(vict)) {
    act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, ch,
      0, vict, TO_NOTVICT);
    act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, ch,
      0, vict, TO_VICT);
  } else if (IS_NEUTRAL(ch) && IS_NEUTRAL(vict)) {
    sprintf(buf, "You, %s, follow the True Path.", GET_NAME(vict));
    do_say(ch, buf, 0, 0);
  } else if (IS_NEUTRAL(vict) || IS_NEUTRAL(ch)) {
    sprintf(buf, "%s, it is not too late for you to mend your ways.",
      GET_NAME(vict));
    do_say(ch, buf, 0, 0);
  } else {
    sprintf(buf, "Blasphemy!  %s, your presence will stain this temple no "
      "more!", GET_NAME(vict));
    do_say(ch, buf, 0, 0);
  }
  return (FALSE);
}
/********************************************************************/

