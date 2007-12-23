/************************************************************************
 * OasisOLC - General / oasis.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-1999 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genmob.h"
#include "genshp.h"
#include "genzon.h"
#include "genwld.h"
#include "genobj.h"
#include "oasis.h"
#include "screen.h"

const char *nrm, *grn, *cyn, *yel, *red;

/*
 * External data structures.
 */
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct room_data *world;
extern zone_rnum top_of_zone_table;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern void trigedit_setup_new(struct descriptor_data *d);
extern void trigedit_setup_existing(struct descriptor_data *d, int rtrg_num);
extern int real_trigger(int vnum);
extern void hedit_save_to_disk(void);
extern void free_help(struct help_index_element *help);
extern struct clan_type *clan_info;
int find_help_rnum(char *keyword);
extern void hedit_setup_new(struct descriptor_data *d, char *new_key);
extern void hedit_setup_existing(struct descriptor_data *d, int rnum);
int can_edit_zone(struct char_data *ch, int number);
int is_name(const char *str, const char *namelist);
int isname(const char *str, const char *namelist);

/*
 * Internal data structures.
 */
struct olc_scmd_info_t {
  const char *text;
  int con_type;
} olc_scmd_info[] = {
  { "room",	CON_REDIT },
  { "object",	CON_OEDIT },
  { "zone",	CON_ZEDIT },
  { "mobile",	CON_MEDIT },
  { "shop",	CON_SEDIT },
  { "trig", 	CON_TRIGEDIT },
  { "help",     CON_HEDIT },
  { "clan",     CON_CEDIT },
 // { "business", CON_BEDIT },
  { "\n",	-1	  }
};

/* -------------------------------------------------------------------------- */

/*
 * Only player characters should be using OLC anyway.
 */
void clear_screen(struct descriptor_data *d)
{
  if (PRF_FLAGGED(d->character, PRF_CLS))
    send_to_char("[H[J", d->character);
}

/* -------------------------------------------------------------------------- */

int can_edit_zone(struct char_data *ch, int number)   // number is a real_znum
{

  if (IS_NPC(ch))
       return FALSE;

  if (GET_LEVEL(ch) >= LVL_DEITY)
    return TRUE;
 
  if (GET_LEVEL(ch) < LVL_IMMORT)
    return FALSE;

  if (!zone_table[number].builders || !isname(GET_NAME(ch), zone_table[number].builders))
    return FALSE;

    return TRUE;
 }

/* -------------------------------------------------------------------------- */

/*
 * Exported ACMD do_oasis function.
 *
 * This function is the OLC interface.  It deals with all the 
 * generic OLC stuff, then passes control to the sub-olc sections.
 */
ACMD(do_oasis)
{
  int number = -1, save = 0, real_num;
  struct descriptor_data *d;
  struct clan_type *cptr = NULL;

  /*
   * No screwing around as a mobile.
   */
  if (IS_NPC(ch))
    return;
  
  /*
   * The command to see what needs to be saved, typically 'olc'.
   */
  if (subcmd == SCMD_OLC_SAVEINFO) {
    do_show_save_list(ch);
    return;
  }

  /*
   * Parse any arguments.
   */
  two_arguments(argument, buf1, buf2);
  if (!*buf1) {		/* No argument given. */
    switch (subcmd) {
    case SCMD_OASIS_ZEDIT:
    case SCMD_OASIS_REDIT:
      number = GET_ROOM_VNUM(IN_ROOM(ch));
      break;
    case SCMD_OASIS_OEDIT:
    case SCMD_OASIS_MEDIT:
    case SCMD_OASIS_SEDIT:
    case SCMD_OASIS_TRIGEDIT:
      sprintf(buf, "Specify a %s VNUM to edit.\r\n", olc_scmd_info[subcmd].text);
      send_to_char(buf, ch);
      return;
    case SCMD_OASIS_HEDIT:
       sprintf(buf, "Specify a %s entry to edit.\r\n", olc_scmd_info[subcmd].text);
        send_to_char(buf, ch);
        return;
    case SCMD_OASIS_CEDIT:
      send_to_char("Specify a clan to edit.\r\n", ch);
      return;
  //  case SCMD_OASIS_BEDIT:
  //    number = GET_BUSINESS_VNUM(ch);
  //    break;

    }
  } else if (!isdigit(*buf1)) {
    if (str_cmp("save", buf1) == 0) {
      save = TRUE;
      if (subcmd == SCMD_OASIS_HEDIT)
           number = 0;
      else
      if ((number = (*buf2 ? atoi(buf2) : (GET_OLC_ZONE(ch) ? GET_OLC_ZONE(ch) : -1)) * 100) < 0 ) {
	send_to_char("Save which zone?\r\n", ch);
	return;
      }
    }
    else if (subcmd == SCMD_OASIS_HEDIT)
       number = 0; 
    else if (subcmd == SCMD_OASIS_CEDIT)
       number = 0;
    else if (subcmd == SCMD_OASIS_ZEDIT && GET_LEVEL(ch) >= LVL_IMPL) {
      if (str_cmp("new", buf1) == 0 && *buf2 && (number = atoi(buf2)) >= 0)
	zedit_new_zone(ch, number);
      else
	send_to_char("Specify a new zone number.\r\n", ch);
      return;
    } else {
      send_to_char("Yikes!  Stop that, someone will get hurt!\r\n", ch);
      return;
    }
  }

  /*
   * If a numeric argument was given (like a room number), get it.
   */
  if (number == -1 && subcmd != SCMD_OASIS_HEDIT)
    number = atoi(buf1);

  /*
   * Check that whatever it is isn't already being edited.
   */
  for (d = descriptor_list; d; d = d->next)
    if (STATE(d) == olc_scmd_info[subcmd].con_type)
      if (d->olc && OLC_NUM(d) == number) {
	if (subcmd == SCMD_OASIS_HEDIT)
	  sprintf(buf, "Help files are already being editted by %s.\r\n",
		  (CAN_SEE(ch, d->character) ? GET_NAME(d->character) : "someone"));
	else
	  sprintf(buf, "That %s is currently being edited by %s.\r\n",
		  olc_scmd_info[subcmd].text, PERS(d->character, ch));
	send_to_char(buf, ch);
	return;
      }
  d = ch->desc;
 
  /*
   * Give descriptor an OLC structure.
   */
  if (d->olc) {
    mudlog("SYSERR: do_oasis: Player already had olc structure.", BRF, LVL_IMMORT, TRUE);
    free(d->olc);
  }
  CREATE(d->olc, struct oasis_olc_data, 1);

  /*
   * Find the zone.
   */
   if (subcmd == SCMD_OASIS_HEDIT && !save)
     OLC_ZNUM(d) = find_help_rnum(buf1);
   else
  if ((OLC_ZNUM(d) = real_zone_by_thing(number)) == -1) {
    send_to_char("Sorry, there is no zone for that number!\r\n", ch);
    free(d->olc);
    d->olc = NULL;
    return;
  }

  /*
   * Everyone but IMPLs can only edit zones they have been assigned.
   */
  if ((GET_LEVEL(ch) < LVL_DEITY && subcmd != SCMD_OASIS_HEDIT)) {
    if (subcmd == SCMD_OASIS_HEDIT) {
      send_to_char("You do not have permission to edit help entries.\r\n", ch);
      free(d->olc);
      d->olc = NULL;
      return;
     } else if (((!can_edit_zone(ch, OLC_ZNUM(d)))) && \
               (subcmd != SCMD_OASIS_HEDIT)) {
      send_to_char("You do not have permission to edit this zone.\r\n", ch);
      free(d->olc);
      d->olc = NULL;
      return;
    }
  }
  if (save) {
    const char *type = NULL;
 
    if (subcmd >= 0 && subcmd <= (int)(sizeof(olc_scmd_info) / sizeof(struct olc_scmd_info_t) - 1))
      type = olc_scmd_info[subcmd].text;
    else {
      send_to_char("Oops, I forgot what you wanted to save.\r\n", ch);
      return;
    }
     if (subcmd == SCMD_OASIS_HEDIT) {
       send_to_char("Saving all help entries.\r\n", ch);
       sprintf(buf, "OLC: %s saves help entries.", GET_NAME(ch));
       mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    } else if (subcmd == SCMD_OASIS_CEDIT) {
        send_to_char("Saving clans.\r\n",ch);
        sprintf(buf, "OLC: %s saves clans.", GET_NAME(ch));
        mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
     } else {
    sprintf(buf, "Saving all %ss in zone %d.\r\n", type, zone_table[OLC_ZNUM(d)].number);
    send_to_char(buf, ch);
    sprintf(buf, "OLC: %s saves %s info for zone %d.", GET_NAME(ch), type, zone_table[OLC_ZNUM(d)].number);
    mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    }
    switch (subcmd) {
      case SCMD_OASIS_REDIT: save_rooms(OLC_ZNUM(d)); break;
      case SCMD_OASIS_ZEDIT: save_zone(OLC_ZNUM(d)); break;
      case SCMD_OASIS_OEDIT: save_objects(OLC_ZNUM(d)); break;
      case SCMD_OASIS_MEDIT: save_mobiles(OLC_ZNUM(d)); break;
      case SCMD_OASIS_SEDIT: save_shops(OLC_ZNUM(d)); break;
      case SCMD_OASIS_HEDIT: hedit_save_to_disk(); break;
      case SCMD_OASIS_CEDIT: save_clans(); break;
//    case SCMD_OASIS_BEDIT: save_economy(); break;
    }
    free(d->olc);
    d->olc = NULL;
    return;
  }

  OLC_NUM(d) = number;

  /*
   * Steal player's descriptor and start up subcommands.
   */
  switch (subcmd) {
   case SCMD_OASIS_HEDIT:
     if (OLC_ZNUM(d) < 0)
       hedit_setup_new(d, buf1);
     else
       hedit_setup_existing(d, OLC_ZNUM(d));
     STATE(d) = CON_HEDIT;
     break;
  case SCMD_OASIS_REDIT:
    if ((real_num = real_room(number)) >= 0)
      redit_setup_existing(d, real_num);
    else
      redit_setup_new(d);
    STATE(d) = CON_REDIT;
    break;
  case SCMD_OASIS_ZEDIT:
    if ((real_num = real_room(number)) < 0) {
      send_to_char("That room does not exist.\r\n", ch);
      free(d->olc);
      d->olc = NULL;
      return;
    }
    zedit_setup(d, real_num);
    STATE(d) = CON_ZEDIT;
    break;
  case SCMD_OASIS_MEDIT:
    if ((real_num = real_mobile(number)) < 0)
      medit_setup_new(d);
    else
      medit_setup_existing(d, real_num);
    STATE(d) = CON_MEDIT;
    break;
  case SCMD_OASIS_OEDIT:
    if ((real_num = real_object(number)) >= 0)
      oedit_setup_existing(d, real_num);
    else
      oedit_setup_new(d);
    STATE(d) = CON_OEDIT;
    break;
  case SCMD_OASIS_SEDIT:
    if ((real_num = real_shop(number)) >= 0)
      sedit_setup_existing(d, real_num);
    else
      sedit_setup_new(d);
    STATE(d) = CON_SEDIT;
    break;
  case SCMD_OASIS_TRIGEDIT:
     if ((real_num = real_trigger(number)) >= 0)
       trigedit_setup_existing(d, real_num);
     else
       trigedit_setup_new(d);
     STATE(d) = CON_TRIGEDIT;
     break;

  case SCMD_OASIS_CEDIT:
    if (number == 0)
       cedit_setup_new(ch->desc);
    else {
       for (cptr = clan_info; cptr && cptr->number != number; cptr=cptr->next);
        if (cptr && (cptr->number == number)) {
          OLC_CLAN(d) = cptr;
          cedit_disp_menu(d);
        } else {
          send_to_char("Invalid clan number!\r\n", d->character);
          return;
          }
        }
       STATE(d) = CON_CEDIT;
       break;
  }

//  case SCMD_OASIS_BEDIT:
  //  if ((real_num = real_business(number)) >= 0)
 //     business_setup_existing(d, real_num);
 //   else
 //     business_setup_new(d);
 //   STATE(d) = CON_BEDIT;
 //   break;

  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT(PLR_FLAGS(ch), PLR_WRITING);
}

/*------------------------------------------------------------*\
 Exported utilities 
\*------------------------------------------------------------*/

/*
 * Set the colour string pointers for that which this char will
 * see at color level NRM.  Changing the entries here will change 
 * the colour scheme throughout the OLC.
 */
void get_char_colors(struct char_data *ch)
{
  nrm = CCNRM(ch, C_NRM);
  grn = CCGRN(ch, C_NRM);
  cyn = CCCYN(ch, C_NRM);
  yel = CCYEL(ch, C_NRM);
  red = CCRED(ch, C_NRM);
}

/*
 * This procedure frees up the strings and/or the structures
 * attatched to a descriptor, sets all flags back to how they
 * should be.
 */
void cleanup_olc(struct descriptor_data *d, byte cleanup_type)
{
  /*
   * Clean up WHAT?
   */
  if (d->olc == NULL)
    return;


     /*
      * Check for help.
      */
     if (OLC_HELP(d)) {
       switch (cleanup_type) {
       case CLEANUP_ALL:      free_help(OLC_HELP(d));	break;
       case CLEANUP_STRUCTS:  free(OLC_HELP(d));	break;
       default: /* The caller has screwed up. */	break;
       }
     }
  /*
   * Check for a room. free_room doesn't perform
   * sanity checks, we must be careful here.
   */
  if (OLC_ROOM(d)) {
    switch (cleanup_type) {
    case CLEANUP_ALL:
      free_room(OLC_ROOM(d));
      break;
    case CLEANUP_STRUCTS:
      free(OLC_ROOM(d));
      break;
    default: /* The caller has screwed up. */
      log("SYSERR: cleanup_olc: Unknown type!");
      break;
    }
  }

  /*
   * Check for an existing object in the OLC.  The strings
   * aren't part of the prototype any longer.  They get added
   * with str_dup().
   */
  if (OLC_OBJ(d)) {
    free_object_strings(OLC_OBJ(d));
    free(OLC_OBJ(d));
  }

  /*
   * Check for a mob.  free_mobile() makes sure strings are not in
   * the prototype.
   */
  if (OLC_MOB(d))
    free_mobile(OLC_MOB(d));

  /*
   * Check for a zone.  cleanup_type is irrelevant here, free() everything.
   */
  if (OLC_ZONE(d)) {
    free(OLC_ZONE(d)->name);
    free(OLC_ZONE(d)->cmd);
    free(OLC_ZONE(d));
  }

  /*
   * Check for a shop.  free_shop doesn't perform sanity checks, we must
   * be careful here.
   */
  if (OLC_SHOP(d)) {
    switch (cleanup_type) {
    case CLEANUP_ALL:
      free_shop(OLC_SHOP(d));
      break;
    case CLEANUP_STRUCTS:
      free(OLC_SHOP(d));
      break;
    default:
      /* The caller has screwed up but we already griped above. */
      break;
    }
  }

   if (OLC_CLAN(d)) {
     switch(cleanup_type) {
     case CLEANUP_ALL:
       cedit_free_clan(OLC_CLAN(d));
       break;
     case CLEANUP_STRUCTS:
       break;
     default:
       /* Caller has screwed up */
       break;
     }
     OLC_CLAN(d) = NULL;
   }

  /*
   * Restore descriptor playing status.
   */
  if (d->character) {
    REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
    STATE(d) = CON_PLAYING;
    act("$n stops using OLC.", TRUE, d->character, NULL, NULL, TO_ROOM);
  }

  free(d->olc);
  d->olc = NULL;
}

ACMD(do_zhelp)
{
 int i, k, fnd = 0, found = FALSE, rnum, zone, zn;
/*  int i, found = FALSE, rnum, zone, zn; */
  char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if(!*arg) {
    send_to_char("Usage: zhelp mobs|objects|rooms\r\n", ch);
    return;
  }

  
  zone = (world[ch->in_room].number / 100);
/*  zn = real_zone(world[ch->in_room].number); */
  zn = real_zone(zone);

/* Used to bug test the code. Since oasis olc changed the format of real_zone.
  sprintf(buf, "zone = %d \r\nzn = %d\r\n top = %d\r\n", zone, zn, zone_table[zn].top);
  send_to_char(buf, ch);
  send_to_char("\r\nCommand Disabled for the time being\r\n", ch);
  return;
*/

  if(is_abbrev(arg, "mobs")) {
    sprintf(buf, "Mobs in zone #%d:\r\n\r\n", zone);
    sprintf(buf, "%s VNUM   Name                            lvl      Exp     Gold    Hits\r\n", buf);
    for(i = zone * 100; i <= zone_table[zn].top; i++) {
      if((rnum = real_mobile(i)) >=0) {
        sprintf(buf, "%s%5d - %-30.30s  %3d  %7d  %7d\r\n", buf, i,
          mob_proto[rnum].player.short_descr ?
          mob_proto[rnum].player.short_descr : "Unnamed",
          mob_proto[rnum].player.level,
          mob_proto[rnum].points.exp,
          mob_proto[rnum].points.gold);
        found = TRUE;
      }
    }
  } else if(is_abbrev(arg, "objects")) {
    sprintf(buf, "Objects in zone #%d:\r\n\r\n", zone);
    for(i = zone * 100; i <= zone_table[zn].top; i++) {
      if((rnum = real_object(i)) >=0) {
        sprintf(buf, "%s%5d - %-30.30s  Aff:", buf, i,
          obj_proto[rnum].short_description ?
          obj_proto[rnum].short_description : "Unnamed");
        found = TRUE;
    fnd = 0;   
    for (k = 0; k < MAX_OBJ_AFFECT; k++)
      if (obj_proto[rnum].affected[k].modifier) {
        sprinttype(obj_proto[rnum].affected[k].location, apply_types, buf2);
        sprintf(buf, "%s%s %+d to %s", buf, fnd++ ? "," : "",
                obj_proto[rnum].affected[k].modifier, buf2);
      }
    if (!fnd)
      sprintf(buf, "%s None", buf);
    sprintf(buf, "%s \r\n", buf);
      }
    }
  } else if(is_abbrev(arg, "rooms")) {
    sprintf(buf, "Rooms in zone #%d:\r\n\r\n", zone);
    for(i = zone * 100; i <= zone_table[zn].top; i++) {
      if((rnum = real_room(i)) >=0) {
	sprintf(buf, "%s%5d - %s\r\n", buf, i,
	  world[rnum].name ?
	  world[rnum].name : "Unnamed");
	found = TRUE;
      }
    }
  } else {
    send_to_char("Invalid argument.\r\n", ch);
    send_to_char("Usage: zlist mobs|objects|rooms\r\n", ch);
    return;
  }

  if(!found) sprintf(buf, "%sNone!\r\n", buf);

  page_string(ch->desc, buf, TRUE); 
}

void olc_disp_spec_proc_menu(struct descriptor_data * d,
  struct specproc_info table[])
{
  show_spec_proc_table(table, GET_LEVEL(d->character), buf);
  send_to_char(buf, d->character);
  send_to_char("\r\n&wSelect spec proc: ", d->character);
}

ACMD(do_odelete)
{
  int vnum,rnum;
  /*
   * No screwing around as a mobile.
   */
  if (IS_NPC(ch))
    return;
  
  /*
   * Parse any arguments.
   */
  two_arguments(argument, buf1, buf2);
  if (!*buf1) 
    {		/* No argument given. */
      send_to_char("Specify a vnum to delete.\r\n",ch);
      return;
    }

  vnum = atoi(buf1);
  rnum = real_object(vnum);
  if (vnum < 0)
    {
      sprintf(buf, "Object %d doesn't exist.\r\n", vnum);
      send_to_char(buf, ch);
      return;
    }
  if (!can_edit_zone(ch, real_zone_by_thing(vnum)))
    {
      send_to_char("&RYou do not have permission to delete things in this zone.&n\r\n", ch);
      return;
    }
  
  delete_object(rnum);
  send_to_char("Deleted object\r\n", ch);
}

ACMD(do_rdelete)
{
  int vnum,rnum;
  /*
   * No screwing around as a mobile.
   */
  if (IS_NPC(ch))
    return;
  
  /*
   * Parse any arguments.
   */
  two_arguments(argument, buf1, buf2);
  if (!*buf1) 
    {		/* No argument given. */
      send_to_char("Specify a vnum to delete.\r\n",ch);
      return;
    }

  vnum = atoi(buf1);
  rnum = real_room(vnum);
  if (vnum < 0)
    {
      sprintf(buf, "Room %d doesn't exist.\r\n", vnum);
      send_to_char(buf, ch);
      return;
    }
  if (!can_edit_zone(ch, real_zone_by_thing(vnum)))
    {
      send_to_char("&RYou do not have permission to delete things in this zone.&n\r\n", ch);
      return;
    }
  
  delete_room(rnum);
  send_to_char("Deleted room\r\n", ch);
}

ACMD(do_mdelete)
{
  int vnum,rnum;
  /*
   * No screwing around as a mobile.
   */
  if (IS_NPC(ch))
    return;
  
  /*
   * Parse any arguments.
   */
  two_arguments(argument, buf1, buf2);
  if (!*buf1) 
    {		/* No argument given. */
      send_to_char("Specify a vnum to delete.\r\n",ch);
      return;
    }

  vnum = atoi(buf1);
  rnum = real_mobile(vnum);
  if (vnum < 0)
    {
      sprintf(buf, "Mobile %d doesn't exist.\r\n", vnum);
      send_to_char(buf, ch);
      return;
    }
  if (!can_edit_zone(ch, real_zone_by_thing(vnum)))
    {
      send_to_char("&RYou do not have permission to delete things in this zone.&n\r\n", ch);
      return;
    }
  
  delete_mobile(rnum);
  send_to_char("Deleted mobile\r\n", ch);
}

