/*
************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "screen.h"
#include "improved-edit.h"
#include "dg_scripts.h"
#include "diskio.h"

/* extern variables */
extern int level_can_shout;
extern int holler_move_cost;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;

/* local functions */
void logghistory(char *buffer, struct char_data *ch);
void logthistory(char *buffer, struct char_data *ch);
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
int is_tell_ok(struct char_data *ch, struct char_data *vict);
ACMD(do_say);
ACMD(do_boom);
ACMD(do_gsay);
ACMD(do_tell);
ACMD(do_reply);
ACMD(do_spec_comm);
ACMD(do_write);
ACMD(do_page);
ACMD(do_gen_comm);
ACMD(do_qcomm);
ACMD(do_conspire);
ACMD(do_cuss);

ACMD(do_say)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  else {
    sprintf(buf, "&W$n says, '%s'&n", argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM|DG_NO_TRIG);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      delete_doubledollar(argument);
      sprintf(buf, "&WYou say, '%s'&n\r\n", argument);
      send_to_char(buf, ch);
    }
  }
  /* trigger check */
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}

ACMD(do_boom)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("&RYes, but WHAT message should ring from the heavens?&n\r\n", ch);
  else 
  {
    act("&MA voice booms from all directions filling your mind!&n", FALSE, ch, 0,0, TO_ROOM|DG_NO_TRIG);
    act("&MYour voice shouts out deafening all who hear it!&n", FALSE, ch, 0,0, TO_CHAR);
    
    sprintf(buf, "&M      %s&n", argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM|DG_NO_TRIG);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

  }
}


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;
  struct descriptor_data *i;

  skip_spaces(&argument);

  if (!AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    sprintf(buf, "&C$n tells the group, '&C%s&C'&n", argument);

    if (AFF_FLAGGED(k, AFF_GROUP) && (k != ch))
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
    for (f = k->followers; f; f = f->next)
      if (AFF_FLAGGED(f->follower, AFF_GROUP) && (f->follower != ch))
	act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);

   for (i = descriptor_list; i; i = i->next)
     if (STATE(i) == CON_PLAYING &&
       PRF_FLAGGED2(i->character, PRF2_HEARALLTELL) && 
       GET_LEVEL(i->character) == LVL_IMPL) {
        send_to_char(CCYEL(i->character, C_CMP), i->character);
        sprintf(buf, ">> %s tells the group, '&y%s&y'\r\n", GET_NAME(ch),argument);
        send_to_char(buf, i->character);
        send_to_char(CCNRM(i->character, C_CMP), i->character);
        }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "&CYou tell the group, '&C%s&C'&n\r\n", argument);
      send_to_char(buf, ch);
    }
  }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  struct descriptor_data *i;

  send_to_char(CCRED(vict, C_NRM), vict);
  sprintf(buf, "$n tells you, '%s'", arg);
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    sprintf(buf1, "%s tells you, '%s'", CAN_SEE(vict, ch) ? GET_NAME(ch) : "An Immortal", arg);
  else
    sprintf(buf1, "%s tells you, '%s'", CAN_SEE(vict, ch) ? GET_NAME(ch) : "Someone", arg);

  logthistory(buf1, vict);

  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(vict, C_NRM), vict);

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    send_to_char(CCRED(ch, C_CMP), ch);
    sprintf(buf, "You tell $N, '%s'", arg);
    if (GET_LEVEL(vict) < LVL_IMMORT)
      sprintf(buf1, "You tell %s, '%s'", CAN_SEE(ch, vict) ? GET_NAME(vict) : "Someone",arg);
    else
      sprintf(buf1, "You tell %s, '%s'", CAN_SEE(ch, vict) ? GET_NAME(vict) : "An Immortal",arg);

    logthistory(buf1, ch);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    send_to_char(CCNRM(ch, C_CMP), ch);
  }

   for (i = descriptor_list; i; i = i->next)
     if (STATE(i) == CON_PLAYING && i != ch->desc && i != vict->desc &&
       PRF_FLAGGED2(i->character, PRF2_HEARALLTELL) && 
       GET_LEVEL(i->character) == LVL_IMPL && !IS_NPC(ch) && !IS_NPC(vict)) 
       {
    	send_to_char(CCRED(i->character, C_CMP), i->character);
   	sprintf(buf, ">> %s tells %s, '%s'\r\n", GET_NAME(ch), GET_NAME(vict), arg);
	send_to_char(buf, i->character);
        send_to_char(CCNRM(i->character, C_CMP), i->character);
       }
  if (!IS_NPC(vict) && !IS_NPC(ch))
    GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

int is_tell_ok(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if ((!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_NOTELL)))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else { 
    if (PRF_FLAGGED(vict, PRF_AFK))
      act("$E's afk right now and might not get this message.",FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    return (TRUE);
  }
  return (FALSE);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict = NULL;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
    send_to_char(NOPERSON, ch);
  else if (is_tell_ok(ch, vict))
    perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */
				     
    /*
     * XXX: A descriptor list based search would be faster although
     *      we could not find link dead people.  Not that they can
     *      hear tells anyway. :) -gg 2/24/98
     */
    while (tch != NULL && (IS_NPC(tch) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else if (is_tell_ok(ch, tch))
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  const char *action_sing, *action_plur, *action_others;

  switch (subcmd) {
  case SCMD_WHISPER:
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
    break;

  case SCMD_ASK:
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
    break;

  default:
    action_sing = "oops";
    action_plur = "oopses";
    action_others = "$n is tongue-tied trying to speak with $N.";
    break;
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)))
    send_to_char(NOPERSON, ch);
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      send_to_char(buf, ch);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
    speech_mtrigger(ch, buf2);
    speech_wtrigger(ch, buf2);

  }

}



#define MAX_NOTE_LENGTH 1000	/* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper, *pen = NULL;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {		/* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {		/* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {		/* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
      pen = paper;
      paper = NULL;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
	      papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else {
    char *backstr = NULL;

    /* Something on it, display it as that's in input buffer. */
    if (paper->action_description) {
      backstr = str_dup(paper->action_description);
      send_to_char("There's something written on it already:\r\n", ch);
      send_to_char(paper->action_description, ch);
    }

    /* we can write - hooray! */
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    SET_BIT(GET_OBJ_EXTRA(paper),ITEM_UNIQUE_SAVE);
    send_editor_help(ch->desc);
    string_write(ch->desc, &paper->action_description, MAX_NOTE_LENGTH, 0, backstr);
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "\007\007*$n* %s", buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_GOD) {
	for (d = descriptor_list; d; d = d->next)
	  if (STATE(d) == CON_PLAYING && d->character)
	    act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
	send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if ((vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(OK, ch);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  struct descriptor_data *i;
  char color_on[24];
  byte gmote = FALSE; /* add this line */

  /* Array of flags which must _not_ be set in order for comm to be heard */
  int channels[] = {
    0,
    PRF_DEAF,
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    0,
    PRF_DEAF,
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */
  const char *com_msgs[][4] = {
    {"You cannot holler!!\r\n",
      "holler",
      "",
    KYEL},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KYEL},


    {"You cannot auction!!\r\n",
      "auction",
      "You aren't even on the channel!\r\n",
    KMAG},

    {"You cannot congratulate!\r\n",
      "congrat",
      "You aren't even on the channel!\r\n",
    KGRN},

    {"Gemote\r\n",
      "gemote",
      "gemote\r\n",
    KGRN},
   
    {"You cannot talk on the newbie channel!!\r\n",
      "newbie",
      "You aren't even on the channel!\r\n",
    DCYN}
  };

  ACMD(do_gmote);

  /* to keep pets, etc from being ordered to shout */
  if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM))
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }

  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < level_can_shout) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    level_can_shout, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd])) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);

/* add these four lines */
  if(subcmd == SCMD_GMOTE || (subcmd == SCMD_GOSSIP && *argument == '@')) {
    subcmd = SCMD_GOSSIP;
    gmote = TRUE;
  }

  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
	    com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

/* add these seven lines */
  if (gmote) {
    if (*argument == '@')
      do_gmote(ch, argument + 1, 0, 1);
    else
      do_gmote(ch, argument, 0, 1);
    return;
  }

  if (subcmd == SCMD_NEWBIE && GET_LEVEL(ch) > 20 && GET_LEVEL(ch) < LVL_IMMORT) 
  {
   if (GET_CLAN(ch) != 10)
   {
    send_to_char("You can't use the newbie channel!\r\n", ch);
    return;
   }
  }

  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }
  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP)
      sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1],
	      argument, KNRM);
    else
      sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
  }

  sprintf(buf, "$n %ss, '%s'", com_msgs[subcmd][1], argument);
  sprintf(buf1, "&YYou %s, '%s'&n", com_msgs[subcmd][1], argument);

  if (subcmd == SCMD_GOSSIP)
       logghistory(buf1, ch);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) == CON_PLAYING && i != ch->desc && i->character &&
				!PRF_FLAGGED(i->character, channels[subcmd]) &&
				!PLR_FLAGGED(i->character, PLR_WRITING)) {
			
      if (subcmd == SCMD_SHOUT &&
	  ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
	   !AWAKE(i->character)))
				continue;
			
      if (subcmd == SCMD_NEWBIE 
					&& GET_LEVEL(i->character) > 20 && GET_LEVEL(i->character) < LVL_IMMORT 
					&& GET_CLAN(i->character) != 10) 
        continue;
			
      if (COLOR_LEV(i->character) >= C_NRM)
				send_to_char(color_on, i->character);
      
			act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
        if (GET_LEVEL(ch) >= LVL_IMMORT)
          sprintf(buf1, "&Y%s %ss, '%s'&n", CAN_SEE(i->character,ch) ? GET_NAME(ch) : "An Immortal", com_msgs[subcmd][1], argument);
        else
          sprintf(buf1, "&Y%s %ss, '%s'&n", CAN_SEE(i->character,ch) ? GET_NAME(ch) : "Someone", com_msgs[subcmd][1], argument);
				
				if (subcmd == SCMD_GOSSIP)
         logghistory(buf1, i->character);
				
				if (COLOR_LEV(i->character) >= C_NRM)
					send_to_char(KNRM, i->character);
    }
  }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
	    CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_QSAY)
	sprintf(buf, "You quest-say, '%s'", argument);
      else
	strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "$n quest-says, '%s'", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i != ch->desc &&
	  PRF_FLAGGED(i->character, PRF_QUEST))
	act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}

ACMD(do_conspire) {
  struct descriptor_data *i;
 
  skip_spaces(&argument);

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) 
  {
    send_to_char("Your right to conspire has been revoked!\r\n", ch);
    return;
  } 

  if (!PLR_FLAGGED(ch, PLR_ASSASSIN) && GET_LEVEL(ch) < LVL_IMMORT) {
    sprintf(buf, "&WYou cannot conspire, you are not an assassin.&n\r\n");
    send_to_char(buf, ch);
     } else {
  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
              CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_CONSPIRE)
        sprintf(buf, "&WYou conspire, '%s'&n\r\n", argument);
      else
        strcpy(buf, argument);
        send_to_char(buf, ch);
    }
    if (subcmd == SCMD_CONSPIRE)
       skip_spaces(&argument);
    if (subcmd == SCMD_CONSPIRE)
      {
      sprintf(buf, "&W$n conspires, '%s'&n\r\n", argument);
      sprintf(buf2, "&W$n conspires to the assassins, '%s'&n\r\n", argument);
      }
 
    for (i = descriptor_list; i; i = i->next)
    {
      if (STATE(i) == CON_PLAYING && i != ch->desc && PLR_FLAGGED(i->character, PLR_ASSASSIN)){
        act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
      } else{
      if (STATE(i) == CON_PLAYING && i != ch->desc && GET_LEVEL(i->character) >= LVL_IMMORT)
        act(buf2, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
     }
    }
  }
}  



ACMD(do_cuss) {
  struct descriptor_data *i;
 
  skip_spaces(&argument);

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) 
  {
    send_to_char("&RYour right to cuss has been revoked!&n\r\n", ch);
    return;
  } 

  if (!PLR_FLAGGED(ch, PLR_CUSSER) && GET_LEVEL(ch) < LVL_IMMORT) {
    sprintf(buf, "&RYou must first gain the right to cuss.&n\r\n");
    send_to_char(buf, ch);
     } else {
  if (!*argument) {
    sprintf(buf, "&RWhat the hell are you trying cuss!&n\r\n");
      CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_CUSS)
        sprintf(buf, "&mYou cuss, '%s'&n\r\n", argument);
      else
        strcpy(buf, argument);
        send_to_char(buf, ch);
    }
    if (subcmd == SCMD_CUSS)
       skip_spaces(&argument);
    if (subcmd == SCMD_CUSS)
      {
      sprintf(buf, "&m$n&m cusses, '%s'&n\r\n", argument);
      sprintf(buf2, "&m$n&m cusses to the scumbags, '%s'&n\r\n", argument);
      }
 
    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i != ch->desc && !PRF_FLAGGED2(i->character, PRF2_CUSS))
      {
      if (PLR_FLAGGED(i->character, PLR_CUSSER) && PRF_FLAGGED2(ch,PRF2_CUSS))
        act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
      else if (GET_LEVEL(i->character) >= LVL_IMMORT)
        act(buf2, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
    }
  }
}      

void do_message_world(char *argument, int type)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  switch (type) {
  case 0:
   sprintf(buf2, "&n[&DDarkness Falling&n]&n:&n &r%s&n\r\n", argument);
   break;
  case 1:
   sprintf(buf2, "&m[&MFreeze Tag&m]&n: &r%s&n\r\n", argument);
   break;
  }
  if (type == MSG_FREEZE) {
    for (pt = descriptor_list; pt; pt = pt->next) {
      if (STATE(pt) == CON_PLAYING && pt->character && GET_FREEZE_TEAM(pt->character)) {
          send_to_char(buf2, pt->character);
      }
    }
    return;
  }

  for (pt = descriptor_list; pt; pt = pt->next) {
    if (STATE(pt) == CON_PLAYING && pt->character) {
        send_to_char(buf2, pt->character);
    }
  }
}


int MAXSONGS = -1;
struct song jukebox[100];
struct jukeboxrequest *jukefront = NULL;

void sing(char *lyric)
{
    struct descriptor_data *i;
    
    sprintf(buf, "&M###\"%s\"###&n\r\n", lyric);

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && PRF_FLAGGED2(i->character, PRF2_LYRIC))
        act(buf, 0, 0 , 0, i->character, TO_VICT | TO_SLEEP);
}

bool request_song(int request)
{
  struct jukeboxrequest *temp = jukefront;
  struct jukeboxrequest *newrequest;
  CREATE(newrequest, struct jukeboxrequest, 1);

  newrequest->songnum = request;
  newrequest->next = NULL;

  if (request > MAXSONGS)
     return FALSE;

  while (temp && temp->next != NULL)
    temp = temp->next;

  if (temp)
    temp->next = newrequest; /* Enqueue! */
  else
    temp = newrequest;

  return TRUE;
}

void read_songs()
{
  int i = 0, tempnum = 0, templines = 0;
  FBFILE *lyricfile = NULL;

  if (!(lyricfile = fbopen("../lib/text/lyric", FB_READ)))
  {
      log("Error Reading Songs");
      return;
  }

  while (fbgetline(lyricfile, buf))
  {
    if (scanf(buf,"#%d", tempnum) != EOF)
    { 
      MAXSONGS++;
      jukebox[i].num = tempnum;
      templines = 0;
    }
    else if (!strcmp("^", buf))
    {
       jukebox[i].lines = templines;
       i++;
    }    
    else
    {
       strcpy(jukebox[i].lyrics[templines], buf);
       templines++;
    }
 } 

 fbclose(lyricfile);

}

void update_lyrics()
{
  static int currentsong = 0, online = 0;
  if (currentsong > MAXSONGS)
    return;

  if (online < jukebox[currentsong].lines)
  {
    sing(jukebox[currentsong].lyrics[online]);
    online++;
  }
  else /* The song is done */
  {     
   if (!jukefront)
   {
      if (MAXSONGS >=0)
        currentsong = number(0, MAXSONGS);
   }
   else 
   {
     struct jukeboxrequest *temp = jukefront; 
     currentsong = jukefront->songnum;
     jukefront = jukefront->next;  /* Dequeue! */
     free(temp);
    /* memset((char *) temp, 0, sizeof(struct jukeboxrequest));*/

   }       
  }
}



void logghistory(char *buffer, struct char_data *ch)
{
  int counter;

  if (ch->char_specials.ghcount == 30) {
    for (counter = 1; counter <= 29;counter++)
    {
      ch->char_specials.ghistory[counter] = ch->char_specials.ghistory[counter + 1];
    }  
    ch->char_specials.ghistory[ch->char_specials.ghcount] = strdup(buffer);
  }

  if (ch->char_specials.ghcount < 30) { 
    ch->char_specials.ghistory[++ch->char_specials.ghcount] = strdup(buffer);
  }

  if (ch->char_specials.ghcount > 30) 
    ch->char_specials.ghcount = 30;
}

void logthistory(char *buffer, struct char_data *ch)
{
  int counter;

  if (ch->char_specials.thcount == 10) {
    for (counter = 1; counter <= 9;counter++)
    {
      ch->char_specials.thistory[counter] = ch->char_specials.thistory[counter + 1];
    }  
    ch->char_specials.thistory[ch->char_specials.thcount] = strdup(buffer);
  }

  if (ch->char_specials.thcount < 10) { 
    ch->char_specials.thistory[++ch->char_specials.thcount] = strdup(buffer);
  }

  if (ch->char_specials.thcount > 10) 
    ch->char_specials.thcount = 10;
}

ACMD(do_ghistory)
{
 int count = 1;
 send_to_char("&YGossip History Log&n\r\n", ch);
 for (count = 1; count <= ch->char_specials.ghcount; count++)
 {
    sprintf(buf, "&Y%-2d: %s&n\r\n", count, ch->char_specials.ghistory[count]);
    send_to_char(buf, ch);
 }
}

ACMD(do_thistory)
{
 int count = 1;
 send_to_char("&rTell History Log&n\r\n", ch);
 for (count = 1; count <= ch->char_specials.thcount; count++)
 {
    sprintf(buf, "&r%-2d: %s&n\r\n", count, ch->char_specials.thistory[count]);
    send_to_char(buf, ch);
 }
}

ACMD(do_petition) 
{
  struct descriptor_data *i;
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  
  send_to_char("&rWarning: abuse of this command will result in deletion.&n\r\n", ch);

  if (!*argument) 
  {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
              CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } 
  else 
  {
    half_chop(argument, arg, arg2);
    if (is_abbrev(arg, "all"))
    {
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
      else
      { 
          sprintf(buf, "&WYou petition to the Staff, '%s'&n\r\n", arg2);
          send_to_char(buf,ch);
      }
      sprintf(buf, "\007\007&W$n petitioned, '%s'", arg2);

      for (i = descriptor_list; i; i = i->next)
      {
        if (GET_LEVEL(i->character) < LVL_IMMORT)
             continue;
        if (STATE(i) == CON_PLAYING && i != ch->desc && GET_LEVEL(i->character) >= LVL_IMMORT)
          act(buf2, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
    }
    else
    {
       send_to_char("Specific staff petitions are currently not implemented, please use petition all message.\r\n",ch);
    }
  }
}  


