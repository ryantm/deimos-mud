/************************************************************************
 * OasisOLC - Mobiles / medit.c					v2.0	*
 * Copyright 1996 Harvey Gilpin						*
 * Copyright 1997-1999 George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "shop.h"
#include "genolc.h"
#include "genmob.h"
#include "genzon.h"
#include "genshp.h"
#include "oasis.h"
#include "handler.h"
#include "constants.h"
#include "improved-edit.h"
#include "dg_olc.h"

/* locals */
void mob_defaults(struct char_data *mob, int level);
void powerup_mobile(struct char_data *mob, int level);
void randomize_mobile(struct char_data *mob, int range);

/*-------------------------------------------------------------------*/

/*
 * External variable declarations.
 */
extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct char_data *character_list;
extern mob_rnum top_of_mobt;
extern struct zone_data *zone_table;
extern struct attack_hit_type attack_hit_text[];
extern struct shop_data *shop_index;
extern struct descriptor_data *descriptor_list;
extern struct specproc_info mob_procs[];
#if CONFIG_OASIS_MPROG
extern const char *mobprog_types[];
#endif

/*-------------------------------------------------------------------*/

/*
 * Handy internal macros.
 */
#if CONFIG_OASIS_MPROG
#define GET_MPROG(mob)		(mob_index[(mob)->nr].mobprogs)
#define GET_MPROG_TYPE(mob)	(mob_index[(mob)->nr].progtypes)
#endif

/* Externs */
void olc_disp_spec_proc_menu(struct descriptor_data * d,
  struct specproc_info table[]);

/*-------------------------------------------------------------------*/

/*
 * Function prototypes.
 */
#if CONFIG_OASIS_MPROG
void medit_disp_mprog(struct descriptor_data *d);
void medit_change_mprog(struct descriptor_data *d);
const char *medit_get_mprog_type(struct mob_prog_data *mprog);
#endif

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/

void medit_save_to_disk(zone_vnum foo)
{
  save_mobiles(real_zone(foo));
}

void medit_setup_new(struct descriptor_data *d)
{
  struct char_data *mob;

  /*
   * Allocate a scratch mobile structure.  
   */
  CREATE(mob, struct char_data, 1);

  init_mobile(mob);

  GET_MOB_RNUM(mob) = -1;
  /*
   * Set up some default strings.
   */
  GET_ALIAS(mob) = str_dup("mob unfinished");
  GET_SDESC(mob) = str_dup("the unfinished mob");
  GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
  GET_DDESC(mob) = str_dup("It looks unfinished.\r\n");
#if CONFIG_OASIS_MPROG
  OLC_MPROGL(d) = NULL;
  OLC_MPROG(d) = NULL;
#endif

  OLC_MOB(d) = mob;
  /* Has changed flag. (It hasn't so far, we just made it.) */
  OLC_VAL(d) = FALSE;
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;
  OLC_SPEC(d) = 0;

  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

void medit_setup_existing(struct descriptor_data *d, int rmob_num)
{
  struct char_data *mob;

  /*
   * Allocate a scratch mobile structure. 
   */
  CREATE(mob, struct char_data, 1);

  copy_mobile(mob, mob_proto + rmob_num);

#if CONFIG_OASIS_MPROG
  {
    MPROG_DATA *temp;
    MPROG_DATA *head;

    if (GET_MPROG(mob))
      CREATE(OLC_MPROGL(d), MPROG_DATA, 1);
    head = OLC_MPROGL(d);
    for (temp = GET_MPROG(mob); temp; temp = temp->next) {
      OLC_MPROGL(d)->type = temp->type;
      OLC_MPROGL(d)->arglist = str_dup(temp->arglist);
      OLC_MPROGL(d)->comlist = str_dup(temp->comlist);
      if (temp->next) {
        CREATE(OLC_MPROGL(d)->next, MPROG_DATA, 1);
        OLC_MPROGL(d) = OLC_MPROGL(d)->next;
      }
    }
    OLC_MPROGL(d) = head;
    OLC_MPROG(d) = OLC_MPROGL(d);
  }
#endif

  OLC_MOB(d) = mob;
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;
  dg_olc_script_copy(d);
  OLC_SPEC(d) = get_spec_name(mob_procs, mob_index[rmob_num].func);
  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

/*
 * Ideally, this function should be in db.c, but I'll put it here for
 * portability.
 */
void init_mobile(struct char_data *mob)
{
  clear_char(mob);

  GET_HIT(mob) = GET_MANA(mob) = 1;
  GET_MAX_MANA(mob) = GET_MAX_MOVE(mob) = 100;
  GET_NDD(mob) = GET_SDD(mob) = 1;
  GET_WEIGHT(mob) = 200;
  GET_HEIGHT(mob) = 198;

  mob->real_abils.str = mob->real_abils.intel = mob->real_abils.wis = 11;
  mob->real_abils.dex = mob->real_abils.con = mob->real_abils.cha = 11;
  mob->aff_abils = mob->real_abils;

  SET_BIT(MOB_FLAGS(mob), MOB_ISNPC);
  mob->player_specials = &dummy_mob;
}

/*-------------------------------------------------------------------*/

/*
 * Save new/edited mob to memory.
 */
void medit_save_internally(struct descriptor_data *d)
{
  int i;
  mob_rnum new_rnum;
  struct descriptor_data *dsc;

  /* put the script into proper position */

  new_rnum = real_mobile(OLC_NUM(d));
  i = (real_mobile(OLC_NUM(d)) == NOBODY);

  if ((new_rnum = add_mobile(OLC_MOB(d), OLC_NUM(d))) < 0) {
    log("medit_save_internally: add_object failed.");
    return;
  }

  if (!i)	/* Only renumber on new mobiles. */
    return;

  /*. Update spec procs .*/
    mob_index[new_rnum].func = mob_procs[OLC_SPEC(d)].sp_pointer;

  /*
   * Update keepers in shops being edited and other mobs being edited.
   */
  for (dsc = descriptor_list; dsc; dsc = dsc->next) {
    if (STATE(dsc) == CON_SEDIT)
      S_KEEPER(OLC_SHOP(dsc)) += (S_KEEPER(OLC_SHOP(dsc)) >= new_rnum);
    else if (STATE(dsc) == CON_MEDIT)
      GET_MOB_RNUM(OLC_MOB(dsc)) += (GET_MOB_RNUM(OLC_MOB(dsc)) >= new_rnum);
  }

  /*
   * Update other people in zedit too. From: C.Raehl 4/27/99
   */
  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    if (STATE(dsc) == CON_ZEDIT)
      for (i = 0; OLC_ZONE(dsc)->cmd[i].command != 'S'; i++)
        if (OLC_ZONE(dsc)->cmd[i].command == 'M')
          if (OLC_ZONE(dsc)->cmd[i].arg1 >= new_rnum)
            OLC_ZONE(dsc)->cmd[i].arg1++;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * Display positions. (sitting, standing, etc)
 */
void medit_disp_positions(struct descriptor_data *d)
{
  int i;

  get_char_colors(d->character);
  clear_screen(d);

  for (i = 0; *position_types[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("Enter position number : ", d);
}

/*-------------------------------------------------------------------*/

#if CONFIG_OASIS_MPROG
/*
 * Get the type of MobProg.
 */
const char *medit_get_mprog_type(struct mob_prog_data *mprog)
{
  switch (mprog->type) {
  case IN_FILE_PROG:	return ">in_file_prog";
  case ACT_PROG:	return ">act_prog";
  case SPEECH_PROG:	return ">speech_prog";
  case RAND_PROG:	return ">rand_prog";
  case FIGHT_PROG:	return ">fight_prog";
  case HITPRCNT_PROG:	return ">hitprcnt_prog";
  case DEATH_PROG:	return ">death_prog";
  case ENTRY_PROG:	return ">entry_prog";
  case GREET_PROG:	return ">greet_prog";
  case ALL_GREET_PROG:	return ">all_greet_prog";
  case GIVE_PROG:	return ">give_prog";
  case BRIBE_PROG:	return ">bribe_prog";
  }
  return ">ERROR_PROG";
}

/*-------------------------------------------------------------------*/

/*
 * Display the MobProgs.
 */
void medit_disp_mprog(struct descriptor_data *d)
{
  struct mob_prog_data *mprog = OLC_MPROGL(d);

  OLC_MTOTAL(d) = 1;

  clear_screen(d);
  while (mprog) {
    sprintf(buf, "%d) %s %s\r\n", OLC_MTOTAL(d), medit_get_mprog_type(mprog),
		(mprog->arglist ? mprog->arglist : "NONE"));
    SEND_TO_Q(buf, d);
    OLC_MTOTAL(d)++;
    mprog = mprog->next;
  }
  sprintf(buf,  "%d) Create New Mob Prog\r\n"
		"%d) Purge Mob Prog\r\n"
		"Enter number to edit [0 to exit]:  ",
		OLC_MTOTAL(d), OLC_MTOTAL(d) + 1);
  SEND_TO_Q(buf, d);
  OLC_MODE(d) = MEDIT_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProgs.
 */
void medit_change_mprog(struct descriptor_data *d)
{
  clear_screen(d);
  sprintf(buf,  "1) Type: %s\r\n"
		"2) Args: %s\r\n"
		"3) Commands:\r\n%s\r\n\r\n"
		"Enter number to edit [0 to exit]: ",
	medit_get_mprog_type(OLC_MPROG(d)),
	(OLC_MPROG(d)->arglist ? OLC_MPROG(d)->arglist: "NONE"),
	(OLC_MPROG(d)->comlist ? OLC_MPROG(d)->comlist : "NONE"));

  SEND_TO_Q(buf, d);
  OLC_MODE(d) = MEDIT_CHANGE_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProg type.
 */
void medit_disp_mprog_types(struct descriptor_data *d)
{
  int i;

  get_char_colors(d->character);
  clear_screen(d);

  for (i = 0; i < NUM_PROGS-1; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, mobprog_types[i]);
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("Enter mob prog type : ", d);
  OLC_MODE(d) = MEDIT_MPROG_TYPE;
}
#endif

/*-------------------------------------------------------------------*/

/*
 * Display the gender of the mobile.
 */
void medit_disp_sex(struct descriptor_data *d)
{
  int i;

  get_char_colors(d->character);
  clear_screen(d);

  for (i = 0; i < NUM_GENDERS; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("Enter gender number : ", d);
}

/*-------------------------------------------------------------------*/

/*
 * Display attack types menu.
 */
void medit_disp_attack_types(struct descriptor_data *d)
{
  int i;

  get_char_colors(d->character);
  clear_screen(d);

  for (i = 0; i < NUM_ATTACK_TYPES; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, attack_hit_text[i].singular);
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("Enter attack type : ", d);
}

/*-------------------------------------------------------------------*/

/*
 * Display mob-flags menu.
 */
void medit_disp_mob_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);
  for (i = 0; i < NUM_MOB_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, action_bits[i],
		!(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintbit(MOB_FLAGS(OLC_MOB(d)), action_bits, buf1);
  sprintf(buf, "\r\nCurrent flags : %s%s%s\r\nEnter mob flags (0 to quit) : ",
		  cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*-------------------------------------------------------------------*/

/*
 * Display affection flags menu.
 */
void medit_disp_aff_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);
  for (i = 0; i < NUM_AFF_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, affected_bits[i],
			!(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintbit(AFF_FLAGS(OLC_MOB(d)), affected_bits, buf1);
  sprintf(buf, "\r\nCurrent flags   : %s%s%s\r\nEnter aff flags (0 to quit) : ",
			  cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*-------------------------------------------------------------------*/

/*
 * Display main menu.
 */
void medit_disp_menu(struct descriptor_data *d)
{
  struct char_data *mob;

  mob = OLC_MOB(d);
  get_char_colors(d->character);
  clear_screen(d);

  sprintf(buf,
	  "-- Mob Number:  [%s%d%s]\r\n"
	  "%s1%s) Sex: %s%-7.7s%s	         %s2%s) Keywords: %s%s\r\n"
	  "%s3%s) S-Desc: %s%s\r\n"
	  "%s4%s) L-Desc:-\r\n%s%s"
	  "%s5%s) D-Desc:-\r\n%s%s",
	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, genders[(int)GET_SEX(mob)], nrm,
	  grn, nrm, yel, GET_ALIAS(mob),
	  grn, nrm, yel, GET_SDESC(mob),
	  grn, nrm, yel, GET_LDESC(mob),
	  grn, nrm, yel, GET_DDESC(mob)
	  );
  SEND_TO_Q(buf, d);

  if (GET_LEVEL(d->character) < LVL_CREATOR) 
  {
  sprintf(buf,
          "&WThe below options may not be edited by lesser Builders&n\r\n");
  SEND_TO_Q(buf, d);
  }

  sprintf(buf,
     "%s6%s) Level:       [%s%4d%s],  %s7%s) Alignment:    [%s%4d%s]\r\n"
     "%s8%s) Hitroll:     [%s%4d%s],  %s9%s) Damroll:      [%s%4d%s]\r\n"
     "%sA%s) NumDamDice:  [%s%4d%s],  %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
     "%sC%s) Num HP Dice: [%s%4d%s],  %sD%s) Size HP Dice: [%s%4d%s],  %sE%s) HP Bonus: [%s%5d%s]\r\n"
     "%sF%s) Armor Class: [%s%4d%s],  %sG%s) Exp:     [%s%9d%s],  %sH%s) Gold:  [%s%8d%s]\r\n",
	  grn, nrm, cyn, GET_LEVEL(mob), nrm,
	  grn, nrm, cyn, GET_ALIGNMENT(mob), nrm,
	  grn, nrm, cyn, GET_HITROLL(mob), nrm,
	  grn, nrm, cyn, GET_DAMROLL(mob), nrm,
	  grn, nrm, cyn, GET_NDD(mob), nrm,
	  grn, nrm, cyn, GET_SDD(mob), nrm,
	  grn, nrm, cyn, GET_HIT(mob), nrm,
	  grn, nrm, cyn, GET_MANA(mob), nrm,
	  grn, nrm, cyn, GET_MOVE(mob), nrm,
	  grn, nrm, cyn, GET_AC(mob), nrm,
	  grn, nrm, cyn, GET_EXP(mob), nrm,
	  grn, nrm, cyn, GET_GOLD(mob), nrm
	  );
  SEND_TO_Q(buf, d);
  

  if (GET_LEVEL(d->character) < LVL_CREATOR) 
  {
  sprintf(buf,
          "&WEnd Uneditable Portion.&n\r\n");
  SEND_TO_Q(buf, d);
  }

    
  sprintbit(MOB_FLAGS(mob), action_bits, buf1);
  sprintbit(AFF_FLAGS(mob), affected_bits, buf2);
  sprintf(buf,
	  "%sI%s) Position  : %s%s\r\n"
	  "%sJ%s) Default   : %s%s\r\n"
	  "%sK%s) Attack    : %s%s\r\n"
	  "%sL%s) NPC Flags : %s%s\r\n"
	  "%sM%s) AFF Flags : %s%s\r\n"
#if CONFIG_OASIS_MPROG
	  "%sP%s) Mob Progs : %s%s\r\n"
#endif
          "%sS%s) Scripts   : %s%s\r\n"
          "%sZ%s) Set Modifier Level\r\n"
          "%sO%s) Spec proc: %s%s\r\n"
	  "%sQ%s) Quit\r\n"
	  "Enter choice : ",

	  grn, nrm, yel, position_types[(int)GET_POS(mob)],
	  grn, nrm, yel, position_types[(int)GET_DEFAULT_POS(mob)],
	  grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, buf2,
#if CONFIG_OASIS_MPROG
	  grn, nrm, cyn, (OLC_MPROGL(d) ? "Set." : "Not Set."),
#endif
          grn, nrm, cyn, mob->proto_script?"Set.":"Not Set.",
          grn, nrm,
          grn, nrm, cyn, mob_procs[OLC_SPEC(d)].name,
	  grn, nrm
	  );
  SEND_TO_Q(buf, d);

  OLC_MODE(d) = MEDIT_MAIN_MENU;
}

/************************************************************************
 *			The GARGANTAUN event handler			*
 ************************************************************************/

void medit_parse(struct descriptor_data *d, char *arg)
{
  int i = -1, new_mob_num;
  char *oldtext = NULL;

       /* Mana and Move don't corrispond to the stats inMUD */

  new_mob_num = real_mobile(OLC_NUM(d));

  if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE) {
    i = atoi(arg);
    if (!*arg || (!isdigit(arg[0]) && ((*arg == '-') && !isdigit(arg[1])))) {
      SEND_TO_Q("Field must be numerical, try again : ", d);
      return;
    }
  } else {	/* String response. */
    if (!genolc_checkstring(d, arg))
      return;
  }
  switch (OLC_MODE(d)) {
/*-------------------------------------------------------------------*/
  case MEDIT_CONFIRM_SAVESTRING:
    /*
     * Ensure mob has MOB_ISNPC set or things will go pear shaped.
     */
    SET_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
    switch (*arg) {
    case 'y':
    case 'Y':
      /*
       * Save the mob in memory and to disk.
       */
      SEND_TO_Q("Saving mobile to memory.\r\n", d);
    /*. Update spec procs .*/
    mob_index[new_mob_num].func = mob_procs[OLC_SPEC(d)].sp_pointer;
      medit_save_internally(d);
      sprintf(buf, "OLC: %s edits mob %d", GET_NAME(d->character), OLC_NUM(d));
      mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      /* FALL THROUGH */
    case 'n':
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      return;
    default:
      SEND_TO_Q("Invalid choice!\r\n", d);
      SEND_TO_Q("Do you wish to save the mobile? : ", d);
      return;
    }
    break;

/*-------------------------------------------------------------------*/
  case MEDIT_MAIN_MENU:
    i = 0;
    switch (*arg) {
    case 'q':
    case 'Q':
      if (OLC_VAL(d)) {	/* Anything been changed? */
	SEND_TO_Q("Do you wish to save the changes to the mobile? (y/n) : ", d);
	OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
      } else
	cleanup_olc(d, CLEANUP_ALL);
      return;
    case 'z':
    case 'Z':
      OLC_MODE(d) = MEDIT_SETTER;
      i++;
      SEND_TO_Q("\r\nEnter modifier level : ", d);
      return;
    case '1':
      OLC_MODE(d) = MEDIT_SEX;
      medit_disp_sex(d);
      return;
    case '2':
      OLC_MODE(d) = MEDIT_ALIAS;
      i--;
      break;
    case '3':
      OLC_MODE(d) = MEDIT_S_DESC;
      i--;
      break;
    case '4':
      OLC_MODE(d) = MEDIT_L_DESC;
      i--;
      break;
    case '5':
      OLC_MODE(d) = MEDIT_D_DESC;
      send_editor_help(d);
      SEND_TO_Q("Enter mob description:\r\n\r\n", d);
      if (OLC_MOB(d)->player.description) {
	SEND_TO_Q(OLC_MOB(d)->player.description, d);
	oldtext = str_dup(OLC_MOB(d)->player.description);
      }
      string_write(d, &OLC_MOB(d)->player.description, MAX_MOB_DESC, 0, oldtext);
      OLC_VAL(d) = 1;
      return;
    case '6':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_LEVEL;
      i++;
    }
      break;
    case '7':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_ALIGNMENT;
      i++;
    }
      break;
    case '8':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_HITROLL;
      i++;
    }
      break;
    case '9':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_DAMROLL;
      i++;
    }
      break;
    case 'a':
    case 'A':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_NDD;
      i++;
    }
      break;
    case 'b':
    case 'B':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_SDD;
      i++;
    }
      break;
    case 'c':
    case 'C':
    if (GET_LEVEL(d->character) >= LVL_CREATOR){
      OLC_MODE(d) = MEDIT_NUM_HP_DICE;
      i++;
    }
      break;
    case 'd':
    case 'D':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
      i++;
    }
      break;
    case 'e':
    case 'E':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_ADD_HP;
      i++;
    }
      break;
    case 'f':
    case 'F':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_AC;
      i++;
    }
      break;
    case 'g':
    case 'G':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_EXP;
      i++;
    }
      break;
    case 'h':
    case 'H':
    if (GET_LEVEL(d->character) >= LVL_CREATOR) {
      OLC_MODE(d) = MEDIT_GOLD;
      i++;
    }
      break;
    case 'i':
    case 'I':
      OLC_MODE(d) = MEDIT_POS;
      medit_disp_positions(d);
      return;
    case 'j':
    case 'J':
      OLC_MODE(d) = MEDIT_DEFAULT_POS;
      medit_disp_positions(d);
      return;
    case 'k':
    case 'K':
      OLC_MODE(d) = MEDIT_ATTACK;
      medit_disp_attack_types(d);
      return;
    case 'l':
    case 'L':
      OLC_MODE(d) = MEDIT_NPC_FLAGS;
      medit_disp_mob_flags(d);
      return;
    case 'm':
    case 'M':
      OLC_MODE(d) = MEDIT_AFF_FLAGS;
      medit_disp_aff_flags(d);
      return;
#if CONFIG_OASIS_MPROG
    case 'p':
    case 'P':
      OLC_MODE(d) = MEDIT_MPROG;
      medit_disp_mprog(d);
      return;
#endif
    case 's':
    case 'S':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      return;
    case 'o':
    case 'O':
      OLC_MODE(d) = MEDIT_SPEC_PROC;
      olc_disp_spec_proc_menu(d, mob_procs);
      return;
    default:
      medit_disp_menu(d);
      return;
    }
    if (i == 0)
      break;
    else if (i == 1)
      SEND_TO_Q("\r\nEnter new value : ", d);
    else if (i == -1)
      SEND_TO_Q("\r\nEnter new text :\r\n] ", d);
    else
      SEND_TO_Q("Oops...\r\n", d);
    return;
/*-------------------------------------------------------------------*/
  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_ALIAS:
    if (GET_ALIAS(OLC_MOB(d)))
      free(GET_ALIAS(OLC_MOB(d)));
    GET_ALIAS(OLC_MOB(d)) = str_udup(arg);
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_S_DESC:
    if (GET_SDESC(OLC_MOB(d)))
      free(GET_SDESC(OLC_MOB(d)));
    GET_SDESC(OLC_MOB(d)) = str_udup(arg);
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_L_DESC:
    if (GET_LDESC(OLC_MOB(d)))
      free(GET_LDESC(OLC_MOB(d)));
    if (arg && *arg) {
      strcpy(buf, arg);
      strcat(buf, "\r\n");
      GET_LDESC(OLC_MOB(d)) = str_dup(buf);
    } else
      GET_LDESC(OLC_MOB(d)) = str_dup("undefined");

    break;
/*-------------------------------------------------------------------*/
  case MEDIT_D_DESC:
    /*
     * We should never get here.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!", BRF, LVL_BUILDER, TRUE);
    SEND_TO_Q("Oops...\r\n", d);
    break;
/*-------------------------------------------------------------------*/
#if CONFIG_OASIS_MPROG
  case MEDIT_MPROG_COMLIST:
    /*
     * We should never get here, but if we do, bail out.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached MPROG_COMLIST case!", BRF, LVL_BUILDER, TRUE);
    break;
#endif
/*-------------------------------------------------------------------*/
  case MEDIT_NPC_FLAGS:
    if ((i = atoi(arg)) <= 0)
      break;
    else if (i <= NUM_MOB_FLAGS)
      TOGGLE_BIT(MOB_FLAGS(OLC_MOB(d)), 1 << (i - 1));
    medit_disp_mob_flags(d);
    return;
/*-------------------------------------------------------------------*/
  case MEDIT_AFF_FLAGS:
    if ((i = atoi(arg)) <= 0)
      break;
    else if (i <= NUM_AFF_FLAGS)
      TOGGLE_BIT(AFF_FLAGS(OLC_MOB(d)), 1 << (i - 1));
    medit_disp_aff_flags(d);
    return;
/*-------------------------------------------------------------------*/

#if CONFIG_OASIS_MPROG
  case MEDIT_MPROG:
    if ((i = atoi(arg)) == 0)
      medit_disp_menu(d);
    else if (i == OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      CREATE(temp, struct mob_prog_data, 1);
      temp->next = OLC_MPROGL(d);
      temp->type = -1;
      temp->arglist = NULL;
      temp->comlist = NULL;
      OLC_MPROG(d) = temp;
      OLC_MPROGL(d) = temp;
      OLC_MODE(d) = MEDIT_CHANGE_MPROG;
      medit_change_mprog (d);
    } else if (i < OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      int x = 1;
      for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
        x++;
      OLC_MPROG(d) = temp;
      OLC_MODE(d) = MEDIT_CHANGE_MPROG;
      medit_change_mprog (d);
    } else if (i == (OLC_MTOTAL(d) + 1)) {
      SEND_TO_Q("Which mob prog do you want to purge? ", d);
      OLC_MODE(d) = MEDIT_PURGE_MPROG;
    } else
      medit_disp_menu(d);
    return;

  case MEDIT_PURGE_MPROG:
    if ((i = atoi(arg)) > 0 && i < OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      int x = 1;

      for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
	x++;
      OLC_MPROG(d) = temp;
      REMOVE_FROM_LIST(OLC_MPROG(d), OLC_MPROGL(d), next);
      free(OLC_MPROG(d)->arglist);
      free(OLC_MPROG(d)->comlist);
      free(OLC_MPROG(d));
      OLC_MPROG(d) = NULL;
      OLC_VAL(d) = 1;
    }
    medit_disp_mprog(d);
    return;

  case MEDIT_CHANGE_MPROG:
    if ((i = atoi(arg)) == 1)
      medit_disp_mprog_types(d);
    else if (i == 2) {
      SEND_TO_Q("Enter new arg list: ", d);
      OLC_MODE(d) = MEDIT_MPROG_ARGS;
    } else if (i == 3) {
      SEND_TO_Q("Enter new mob prog commands:\r\n", d);
      /*
       * Pass control to modify.c for typing.
       */
      OLC_MODE(d) = MEDIT_MPROG_COMLIST;
      if (OLC_MPROG(d)->comlist) {
        SEND_TO_Q(OLC_MPROG(d)->comlist, d);
        oldtext = str_dup(OLC_MPROG(d)->comlist);
      }
      string_write(d, &OLC_MPROG(d)->comlist, MAX_STRING_LENGTH, 0, oldtext);
      OLC_VAL(d) = 1;
    } else
      medit_disp_mprog(d);
    return;
#endif

/*-------------------------------------------------------------------*/

/*
 * Numerical responses.
 */

#if CONFIG_OASIS_MPROG
  case MEDIT_MPROG_TYPE:
    /*
     * This calculation may be off by one too many powers of 2?
     * Someone who actually uses MobProgs will have to check.
     */
    OLC_MPROG(d)->type = (1 << LIMIT(atoi(arg), 0, NUM_PROGS - 1));
    OLC_VAL(d) = 1;
    medit_change_mprog(d);
    return;

  case MEDIT_MPROG_ARGS:
    OLC_MPROG(d)->arglist = str_dup(arg);
    OLC_VAL(d) = 1;
    medit_change_mprog(d);
    return;
#endif

  case MEDIT_SETTER:
		mob_defaults(OLC_MOB(d), i);
		break;

  case MEDIT_SEX:
    GET_SEX(OLC_MOB(d)) = LIMIT(i, 0, NUM_GENDERS - 1);
    break;

  case MEDIT_HITROLL:
    GET_HITROLL(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_DAMROLL:
    GET_DAMROLL(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_NDD:
    GET_NDD(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_SDD:
    GET_SDD(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_NUM_HP_DICE:
    GET_HIT(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_SIZE_HP_DICE:
    GET_MANA(OLC_MOB(d)) = LIMIT(i, 0, 1000);
    break;

  case MEDIT_ADD_HP:
    GET_MOVE(OLC_MOB(d)) = LIMIT(i, 0, 2000000000);
    break;

  case MEDIT_AC:
    GET_AC(OLC_MOB(d)) = LIMIT(i, -50, 100);
    break;

  case MEDIT_EXP:
   if (GET_LEVEL(d->character) >= LVL_CREATOR)
    GET_EXP(OLC_MOB(d)) = MAX(i, 0);
    break;

  case MEDIT_GOLD:
   if (GET_LEVEL(d->character) >= LVL_CREATOR)
    GET_GOLD(OLC_MOB(d)) = MAX(i, 0);
    break;

  case MEDIT_POS:
    GET_POS(OLC_MOB(d)) = LIMIT(i, 0, NUM_POSITIONS - 1);
    break;

  case MEDIT_DEFAULT_POS:
    GET_DEFAULT_POS(OLC_MOB(d)) = LIMIT(i, 0, NUM_POSITIONS - 1);
    break;

  case MEDIT_ATTACK:
    GET_ATTACK(OLC_MOB(d)) = LIMIT(i, 0, NUM_ATTACK_TYPES - 1);
    break;

  case MEDIT_LEVEL:
    GET_LEVEL(OLC_MOB(d)) = LIMIT(i, 1, 100);
    break;

  case MEDIT_ALIGNMENT:
    GET_ALIGNMENT(OLC_MOB(d)) = LIMIT(i, -1000, 1000);
    break;

  case MEDIT_SPEC_PROC:
    if (*arg == '\0') break;
      OLC_SPEC(d) = get_spec_proc_index(mob_procs, GET_LEVEL(d->character), atoi(arg));
    if (OLC_SPEC(d)) SET_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_SPEC);
      else REMOVE_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_SPEC);
    break;

/*-------------------------------------------------------------------*/
  default:
    /*
     * We should never get here.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached default case!", BRF, LVL_BUILDER, TRUE);
    SEND_TO_Q("Oops...\r\n", d);
    break;
  }
/*-------------------------------------------------------------------*/

/*
 * END OF CASE 
 * If we get here, we have probably changed something, and now want to
 * return to main menu.  Use OLC_VAL as a 'has changed' flag  
 */

  OLC_VAL(d) = TRUE;
  medit_disp_menu(d);
}

void medit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {

#if CONFIG_OASIS_MPROG
  case MEDIT_MPROG_COMLIST:
    medit_change_mprog(d);
    break;
#endif

  case MEDIT_D_DESC:
  default:
     medit_disp_menu(d);
     break;
  }
}

void randomize_mobile(struct char_data *mob, int range) 
{
	if (MOB_FLAGGED(mob, MOB_EDIT))
		return;

	mob_defaults(mob, number(GET_LEVEL(mob) - range, GET_LEVEL(mob) + range));
}

void powerup_mobile(struct char_data *mob, int level)
{
	if (MOB_FLAGGED(mob, MOB_EDIT))
		return;
	
	mob_defaults(mob, GET_LEVEL(mob) + level);
}

void mob_defaults(struct char_data *mob, int level)
{
	if (MOB_FLAGGED(mob, MOB_EDIT))
		return;

	/* MEDIT_SETTER */
	int max_level = 1000, 
		max_exp = 10000000,
		max_gold = 5000000,
		max_ac =  100,
		max_droll = 100,
		max_hroll = 100,
		max_ndd = 99,
		max_sdd = 30,
		max_move = 2000000000;
	int j = 0;

   GET_LEVEL(mob) = LIMIT(level, 1, max_level);
	 level = GET_LEVEL(mob);

   GET_EXP(mob) = LIMIT(GET_MOB_EXP(mob), 1, max_exp);
   GET_GOLD(mob) = LIMIT(GET_MOB_GOLD(mob), 0, max_gold);
   
   GET_AC(mob) = LIMIT(level*2 - 100,-100, max_ac);
   GET_DAMROLL(mob) = LIMIT(level/5, 0,max_droll);
   GET_HITROLL(mob) = LIMIT(level/5, 0, max_hroll);

   /* NDD * SDD = base damage dealt */
   if(level <= 100)
		 {
     GET_NDD(mob) = LIMIT(MIN(level/7, max_ndd), 1, max_ndd);
     GET_SDD(mob) = LIMIT(MIN(level/7, max_sdd), 1, max_sdd);
   }
   else
   {
     // NDD * SDD = Total Damage
     // 40 * 30 = 1200 damage
     GET_NDD(mob) = LIMIT(
           MIN((float)level/70 * 10, max_ndd), 1, max_ndd);
     GET_SDD(mob) = LIMIT(
           MIN((float)level/90 * 8, max_sdd), 1, max_sdd);
   }

	 j = (float) 2* (float) level * (float)level + 100;

	 GET_MAX_HIT(mob)  = dice(level / 2, level / 2) + LIMIT(j,1, max_move);
	 GET_MAX_MANA(mob) = level * 15;
	 GET_MAX_MOVE(mob) = LIMIT(j, 1, max_move);	 
	 

	 GET_HIT(mob)  = GET_MAX_HIT(mob);
	 GET_MOVE(mob) = GET_MAX_MOVE(mob);
	 GET_MANA(mob) = GET_MAX_MANA(mob);
	 
	 GET_DEFAULT_POS(mob) = POS_STANDING;
	 GET_POS(mob) = POS_STANDING;
}

void give_chunks_to_mobile(struct char_data *mob)
{
  int chance_of_thousand = 0;
  int percent = number(1,100);
  struct obj_data *chunk = NULL;

  if (IS_NPC(mob))
  {
    chance_of_thousand = (int)((GET_LEVEL(mob) * GET_LEVEL(mob))/ 100.)-5;
    if (number(1,1000) < chance_of_thousand)
    {
       if (GET_LEVEL(mob) > 60 && percent < 70)
         percent += 15;

       //We get a chunk
       if (percent < 35)
       {
           //Tin
           chunk = read_object(1510, VIRTUAL);
       }
       else if (percent < 35+25)
       {
           //Copper
           chunk = read_object(1511, VIRTUAL);
       }
       else if (percent < 35+25+20)
       {
           //Iron
           chunk = read_object(1512, VIRTUAL);
       }
       else if (percent < 35+25+20+15)
       {
           //Gold
           chunk = read_object(1513, VIRTUAL);
       }
       else
       {
           //Titanium
           chunk = read_object(1514, VIRTUAL);
       }
      obj_to_char(chunk,mob);

			powerup_mobile(mob, 2);
    }
  }
  return;
}
