/*
************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "screen.h"
#include "constants.h"
#include "dg_scripts.h"

/* extern variables */
extern int minutesplayed;
extern int top_of_helpt;
extern struct help_index_element *help_table;
extern int top_of_questions;
extern struct question_index *question_table;
extern char *help;
extern struct time_info_data time_info;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct spell_info_type spell_info[];
extern time_t LAST_ECON_RESET;
extern time_t boot_time;


extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *gms;
ACMD(do_date);
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *buildercode;
extern char *smartmud;
extern char *sparrank;
extern char *econrank;
extern char *assassinrank;
extern char *preligion;
extern char *dreligion;
extern char *areligion;
extern char *creligion;
extern char *rreligion;
extern char *class_abbrevs[];
extern struct spell_info_type spell_info[];
extern int sim_combat(struct char_data *ch, struct char_data *vict);
int boot_high = 0;
int spell_sort_info[MAX_SKILLS + 1];
extern char *GREETINGS;
extern bool ISHAPPY, DOUBLEEXP, DOUBLEGOLD,GOLDCHIPPER, QUESTON, CANASSASSINATE;


/* extern functions */
ACMD(do_action);
void sort_spells(void);
int spell_sort_info[MAX_SKILLS + 1];
long find_class_bitvector(char arg);
int level_exp(int chclass, int level);
char *title_male(int chclass, int level);
char *title_female(int chclass, int level);
extern struct help_index_element *help_table;
struct time_info_data *real_time_passed(time_t t2, time_t t1);
int compute_armor_class(struct char_data *ch);
char *get_clan_name(struct char_data *ch);
char *get_clan_abbrev(struct char_data *ch);
extern bool shroud_success(struct char_data *ch, struct char_data *victim, bool quiet);
/* local functions */
void list_scanned_chars(struct char_data * list, struct char_data * ch,
int distance, int door);
void print_object_location(int num, struct obj_data * obj, struct char_data * ch, int recur);
void show_obj_to_char(struct obj_data * object, struct char_data * ch, int mode);
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
bool show);
int calc_ave_damage(struct char_data *ch, struct char_data *victim);
ACMD(do_look);
ACMD(do_examine);
ACMD(do_gold);
ACMD(do_score);
    char *num_punct(int foo);
ACMD(do_scan);
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_help);
ACMD(do_who);
ACMD(do_users);
ACMD(do_gen_ps);
void perform_mortal_where(struct char_data * ch, char *arg);
void perform_immort_where(struct char_data * ch, char *arg);
ACMD(do_where);
ACMD(do_levels);
ACMD(do_consider);
ACMD(do_diagnose);
ACMD(do_color);
ACMD(do_toggle);
ACMD(do_file);
ACMD(do_affects);
void sort_commands(void);
ACMD(do_commands);
void diag_char_to_char(struct char_data * i, struct char_data * ch);
void look_at_char(struct char_data * i, struct char_data * ch);
void list_all_char(struct char_data * i, struct char_data * ch, int num);
void list_char_to_char(struct char_data * list, struct char_data * ch);
void do_auto_exits(struct char_data * ch);
ACMD(do_exits);
void look_in_direction(struct char_data * ch, int dir);
void look_in_obj(struct char_data * ch, char *arg);
void look_at_horse(struct char_data * ch);
char *find_exdesc(char *word, struct extra_descr_data * list);
void look_at_target(struct char_data * ch, char *arg);
ACMD(do_skill); 
ACMD(do_gwho);
void cls_screen(struct descriptor_data * d);
void perform_animation(int seq);
ACMD(do_sparrank);
ACMD(do_econrank);
ACMD(do_assassinrank);
ACMD(do_atheist);
ACMD(do_phobos);
ACMD(do_deimos);
ACMD(do_calim);
ACMD(do_rulek);

char *GET_GM_TITLE(struct char_data *ch);

/*
 * This function screams bitvector... -gg 6/45/98
 */
void show_obj_to_char(struct obj_data * object, struct char_data * ch,
			int mode)
{
  *buf = '\0';
  if ((mode == 0) && object->description) {
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS))
     sprintf(buf,"%s&w[%d] ",buf,GET_OBJ_VNUM(object));
    strcat(buf, object->description);
  }
  else if (object->short_description && ((mode == 1) ||
				 (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buf, object->short_description);
  else if (mode == 5) {
      if (GET_OBJ_TYPE(object) != ITEM_NOTE) {
       if (object->action_description) {
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
        return;
       }   
      }
      if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
       if (object->action_description) {
	strcpy(buf, "&nThere is something written upon it:&n\r\n\r\n");
	strcat(buf, object->action_description);
	page_string(ch->desc, buf, 1);
      } else
	send_to_char("&nIt's blank.&n\r\n", ch);
      return;
    } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {
      strcpy(buf, "&nYou see nothing special about it..&n");
    } else			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcpy(buf, "&nIt looks like a drink container.&n");
  }
  if (mode != 3) {
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
      strcat(buf, "&n (invisible)&n");
    if (IS_OBJ_STAT(object, ITEM_BLESS) && AFF_FLAGGED(ch, AFF_DETECT_ALIGN))
      strcat(buf, "&C ..It glows blue!&n");
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
      strcat(buf, "&Y..It glows yellow!&n");
    if (IS_OBJ_STAT(object, ITEM_GLOW))
      strcat(buf, "&W ..It has a soft glowing aura!&n");
    if (IS_OBJ_STAT(object, ITEM_HUM))
      strcat(buf, "&n ..It emits a faint humming sound!&n");
  }
  strcat(buf, "&n\r\n");
   page_string(ch->desc, buf, TRUE);
}


void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
           bool show)
{
  struct obj_data *i, *j;
  char buf[10];
  bool found;
  int num;
  found = FALSE;
  for (i = list; i; i = i->next_content) {
      num = 0;
      for (j = list; j != i; j = j->next_content)
if (j->item_number==NOTHING) {
    if(strcmp(j->short_description,i->short_description)==0) break;
}
else if (j->item_number==i->item_number) break;
      if (j!=i) continue;
      for (j = i; j; j = j->next_content)
if (j->item_number==NOTHING) {
    if(strcmp(j->short_description,i->short_description)==0) num++;
  }
else if (j->item_number==i->item_number) num++;
     if (CAN_SEE_OBJ(ch, i) || (GET_OBJ_TYPE(i) == ITEM_LIGHT)) {

  if (num!=1) {
     sprintf(buf,"(%2i) ",num);
     send_to_char(buf,ch);
    }
  show_obj_to_char(i, ch, mode);
  found = TRUE;
      }
  }
  if (!found && show)
    send_to_char("&n Nothing.&n\r\n", ch);
}




  ACMD(do_scan)
  {
    struct char_data *i;
    int is_in, dir, dis, maxdis, found = 0;

    const char *distance[] = {
      "&nright here &n",
      "&ndirectly &n",
      "&nclose by &n",
      "&na ways &n",
      "&nfar &n",
      "&nvery far &n",
      "&nextremely far &n",
      "&nimpossibly far &n",
    };

    if (IS_AFFECTED(ch, AFF_BLIND)) {
      act("&nYou can't see anything, you're blind!&n", TRUE, ch, 0, 0, TO_CHAR);
      return;
    }
    if ((GET_MOVE(ch) < 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
      act("&nYou are too exhausted.&n", TRUE, ch, 0, 0, TO_CHAR);
      return;
    }

    maxdis = (GET_LEVEL(ch) / 10);
    if (GET_LEVEL(ch) >= LVL_IMMORT)
      maxdis = 7;

    act("&nYou quickly scan the area and see:&n", TRUE, ch, 0, 0, TO_CHAR);
    act("&n$n quickly scans the area.&n", FALSE, ch, 0, 0, TO_ROOM);
    if (GET_LEVEL(ch) < LVL_IMMORT)
      GET_MOVE(ch) -= 3;

    is_in = ch->in_room;
    for (dir = 0; dir < NUM_OF_DIRS; dir++) {
      ch->in_room = is_in;
      for (dis = 0; dis <= maxdis; dis++) {
        if (((dis == 0) && (dir == 0)) || (dis > 0)) {
          for (i = world[ch->in_room].people; i; i = i->next_in_room) {
            if ((!((ch == i) && (dis == 0))) && CAN_SEE(ch, i)) {
              sprintf(buf, "&n%33s: %s%s%s%s&n", GET_NAME(i), distance[dis],
                      ((dis > 0) && (dir < (NUM_OF_DIRS - 2))) ? "to the " : "",
                      (dis > 0) ? dirs[dir] : "",
                      ((dis > 0) && (dir > (NUM_OF_DIRS - 3))) ? "wards" : "");
              act(buf, TRUE, ch, 0, 0, TO_CHAR);
              found++;
            }
          }
        }
        if (!CAN_GO(ch, dir) || (world[ch->in_room].dir_option[dir]->to_room
  == is_in))
          break;
        else
          ch->in_room = world[ch->in_room].dir_option[dir]->to_room;
      }
    }
    if (found == 0)
      act("&nNobody anywhere near you.&n", TRUE, ch, 0, 0, TO_CHAR);
    ch->in_room = is_in;
  }



void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		/* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, "&W is in excellent condition.&n\r\n");
  else if (percent >= 90)
    strcat(buf, "&Y has a few scratches.&n\r\n");
  else if (percent >= 75)
    strcat(buf, "&y has some small wounds and bruises.&n\r\n");
  else if (percent >= 50)
    strcat(buf, "&C has quite a few wounds.&n\r\n");
  else if (percent >= 30)
    strcat(buf, "&C has some big nasty wounds and scratches.&n\r\n");
  else if (percent >= 15)
    strcat(buf, "&r looks pretty hurt.&n\r\n");
  else if (percent >= 0)
    strcat(buf, "&R is in awful condition.&n\r\n");
  else
    strcat(buf, "&R is bleeding awfully from big wounds.&n\r\n");

  send_to_char(buf, ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;
  int rank = (GET_SPARRANK(i)/100);
  int econrank = (GET_ECONRANK(i)/300);
  
  if (!ch->desc)
    return;

   if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("&nYou see nothing special about $m.&n", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;


  if(!IS_NPC(i)) {
     if (ROMANCE(i) == 2) {
     /* Char is engaged */
	sprintf(buf, "&n%s has an engagement ring from &n", GET_NAME(i));
	if (PARTNER(i) == GET_NAME(ch)) {
	   strcat(buf, "&nyou.&n\r\n");
	       }
	else {
	   strcat(buf, PARTNER(i));
	   strcat(buf, "&n\r\n");
	      }
	   send_to_char(buf, ch);
       }
	else if (ROMANCE(i) == 3) {
		/* Char is married */
		sprintf(buf, "&n%s has a wedding ring from &n", GET_NAME(i));
		if (PARTNER(i) == GET_NAME(ch)) {
			strcat(buf, "&nyou.&n\r\n");
		}
		else {
			strcat(buf, PARTNER(i));
			strcat(buf, "&n\r\n");
		}
		send_to_char(buf, ch);
	}

	else if (ROMANCE(i) == 1) {
		/* Char is dating */
		sprintf(buf, "&n%s is dating &n", GET_NAME(i));
		if (PARTNER(i) == GET_NAME(ch)) {
			strcat(buf, "&nyou.&n\r\n");
		}
		else {
			strcat(buf, PARTNER(i));
			strcat(buf, "&n\r\n");
		}
		send_to_char(buf, ch);
	}
	else if (ROMANCE(i) == 0) {
		/* Char is single */
		sprintf(buf, "&n%s is single.&n\r\n", GET_NAME(i));
		send_to_char(buf, ch);
	}
  }

 if (!IS_NPC(i)) {
  send_to_char("&n\r\n", ch);
  act("&n$n is using the following special equipment:&n", FALSE, i, 0, ch,TO_VICT);

  switch(rank) {
   case 0:
   send_to_char("<damage cap>         Nonexistant cap\r\n", ch);   
   break;
   case 1:
   send_to_char("<damage cap>         Scalp\r\n", ch);   
   break;
   case 2:
   send_to_char("<damage cap>         Hair\r\n", ch);   
   break;
   case 3:
   send_to_char("<damage cap>         A straw cap\r\n", ch);   
   break;
   case 4:
   send_to_char("<damage cap>         A stone cap\r\n", ch);   
   break;
   case 5:
   send_to_char("<damage cap>         A bronze cap\r\n", ch);   
   break;
   case 6:
   send_to_char("<damage cap>         A steel cap\r\n", ch);   
   break;
   case 7:
   send_to_char("<damage cap>         A silver cap\r\n", ch);   
   break;
   case 8:
   send_to_char("<damage cap>         A gold cap\r\n", ch);   
   break;
   case 9:
   send_to_char("<damage cap>         A titanium cap\r\n", ch);   
   break;
   case 10:
   send_to_char("<damage cap>         &RThe Death Cap&n\r\n", ch);   
   break;
   }

  switch(econrank) {
   case 0:
   send_to_char("<collar>             No Collar\r\n", ch);   
   break;
   case 1:
   send_to_char("<collar>             Paper Collar\r\n", ch);   
   break;
   case 2:
   send_to_char("<collar>             Blue Collar\r\n", ch);   
   break;
   case 3:
   send_to_char("<collar>             White Collar\r\n", ch);   
   break;
   case 4:
   send_to_char("<collar>             Financial Collar\r\n", ch);   
   break;
   case 5:
   send_to_char("<collar>             Fiscal Collar\r\n", ch);   
   break;
   case 6:
   send_to_char("<collar>             CEO Collar\r\n", ch);   
   break;
   case 7:
   send_to_char("<collar>             BOD Collar\r\n", ch);   
   break;
   default:
   send_to_char("<collar>             &YMoney Collar&n\r\n", ch);   
   break;
   }

 
  switch (GET_RELIGION(i))
  {
  case RELIG_ATHEIST:
    sprintf(buf, "&w%s is an avowed Atheist.&n\r\n", GET_NAME(i));
  break;
  case RELIG_PHOBOS:
    sprintf(buf, "&W%s is a peace-loving follower of Phobe.&n\r\n", GET_NAME(i));
  break;
  case RELIG_DEIMOS:
    sprintf(buf, "&R%s is a chaotic minion of Deimos.&n\r\n", GET_NAME(i));
  break;
  case RELIG_RULEK:
    sprintf(buf, "&C%s is a balanced disciple of Rulek.&n\r\n", GET_NAME(i));
  break;
  case RELIG_CALIM:
    sprintf(buf, "&Y%s is an energetic believer of Calim.&n\r\n", GET_NAME(i));
  break;
  }
  send_to_char(buf, ch);
 }

  if (found) {
    send_to_char("&n\r\n", ch);	/* act() does capitalization. */
    act("&n$n is using:&n", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(where[j], ch);
	show_obj_to_char(GET_EQ(i, j), ch, 1);
      }
  }

  if (ch != i && (GET_SKILL(ch, SKILL_SEE_INV) || GET_LEVEL(ch) >= LVL_IMMORT)) 
  {
    if (GET_THIEF_LEVEL(i) + (GET_CLASS(i) == CLASS_THIEF
? GET_GM_LEVEL(i) : 0) > GET_THIEF_LEVEL(ch) + (GET_CLASS(ch) ==
CLASS_THIEF ? GET_GM_LEVEL(ch) : 0) + GET_SKILL(ch, SKILL_SEE_INV))
    {
      sprintf(buf, "%s tries to look into your inventory and you catch them red handed.\r\n", GET_NAME(ch));
      send_to_char(buf, i);
      sprintf(buf, "%s catches your hand in their pocket.\r\n", GET_NAME(i));      
      send_to_char(buf, ch);
    }
    else
    {
      act("&n\r\nYou peek at $s inventory:&n", FALSE, i, 0, ch, TO_VICT);
      list_obj_to_char(i->carrying, ch, 1, TRUE);    
    }
  }
}


void list_all_char(struct char_data * i, struct char_data * ch, int num)
{
  const char *positions[] = {
    " is lying here, dead.",
    " is lying here, dead.",
    " is lying here, dead.",
    " is lying here, dead.",
    " is completely frozen!",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here."
  };

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    if (AFF_FLAGGED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else
      *buf = '\0';

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
     sprintf(buf,"%s&w[%d]&n ",buf,GET_MOB_VNUM(i));
    }

    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
      if (IS_EVIL(i))
	strcat(buf, "&R(Red Aura)&n ");
      else if (IS_GOOD(i))
	strcat(buf, "&B(Blue Aura)&n ");
    }
    strcat(buf, i->player.long_descr);
      if (num > 1)
         {
         while ((buf[strlen(buf)-1]=='\r') ||
         (buf[strlen(buf)-1]=='\n') ||
         (buf[strlen(buf)-1]==' ')) {
             buf[strlen(buf)-1] = '\0';
                }
           sprintf(buf2,"&n [%d]&n\n\r", num);
           strcat(buf, buf2);
      }
    send_to_char(buf, ch);

    if (AFF_FLAGGED(i, AFF_SANCTUARY))
      act("&W...$e glows with a bright light!&n", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_BLIND))
      act("&w...$e is groping around blindly!&n", FALSE, i, 0, ch, TO_VICT);

    return;
  }
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
     sprintf(buf,"&w[%d]&n ", GET_MOB_VNUM(i));
    } else strcpy(buf,"\0");

  if (IS_NPC(i)) {
    strcpy(buf, i->player.short_descr);
    CAP(buf);
  } else
      sprintf(buf, "&n%s%s %s %s&n", GET_PRETITLE(i), i->player.name,
        	GET_LNAME(i) != NULL ? GET_LNAME(i) : "",
                GET_TITLE(i));
     
  if (AFF_FLAGGED(i, AFF_INVISIBLE))
    strcat(buf, "&n (invisible)&n");
  if (AFF_FLAGGED(i, AFF_HIDE))
    strcat(buf, "&n (hidden)&n");
  if (AFF_FLAGGED(i, AFF_SHADE))
    strcat(buf, "&n (shaded)&n");

  if (!IS_NPC(i)) 
  {
    if (!i->desc)
      strcat(buf, "&n (linkless)&n");
    if (PLR_FLAGGED(i, PLR_WRITING))
      strcat(buf, "&n (writing)&n");
    if (PRF_FLAGGED2(i, PRF2_BUILDWALK))
      strcat(buf, "&n (buildwalk)&n");
  }

  if (GET_POS(i) != POS_FIGHTING)
    strcat(buf, positions[(int) GET_POS(i)]);
  else {
    if (FIGHTING(i)) {
      strcat(buf, "&n is here, fighting &n");
      if (FIGHTING(i) == ch)
	strcat(buf, "&RYOU!&n");
      else {
	if (i->in_room == FIGHTING(i)->in_room)
	  strcat(buf, PERS(FIGHTING(i), ch));
	else
	  strcat(buf, "&nsomeone who has already left&n");
   strcat(buf, "!");
      }
    } else			/* NIL fighting pointer */
      strcat(buf, "&n is here struggling with thin air.&n");
  }

  if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, "&R (Red Aura)&n");
    else if (IS_GOOD(i))
      strcat(buf, "&B (Blue Aura)&n");
  }
  strcat(buf, "&n\r\n");
  send_to_char(buf, ch);

  if (AFF_FLAGGED(i, AFF_SANCTUARY))
    act("&W...$e glows with a bright light!&n", FALSE, i, 0, ch, TO_VICT);
}


void list_char_to_char(struct char_data *list, struct char_data *ch)
{
    struct char_data *i, *plr_list[100];
    int num, counter, locate_list[100], found=FALSE;

    num = 0;

    for (i=list; i; i = i->next_in_room) {
      if(i != ch)  {
        if (CAN_SEE(ch, i))
        {
            if (num< 100)
            {
                found = FALSE;
                for (counter=0;(counter<num&& !found);counter++)
                {
                    if (i->nr == plr_list[counter]->nr &&

/* for odd reasons, you may want to seperate same nr mobs by factors
other than position, the aff flags, and the fighters. (perhaps you want
to list switched chars differently.. or polymorphed chars?) */

                        (GET_POS(i) == GET_POS(plr_list[counter])) &&
                        (AFF_FLAGS(i)==AFF_FLAGS(plr_list[counter])) &&
                        (FIGHTING(i) == FIGHTING(plr_list[counter])) &&
                        !strcmp(GET_NAME(i), GET_NAME(plr_list[counter])))
                    {
                        locate_list[counter] += 1;
                        found=TRUE;
                    }
                }
                if (!found) {
                    plr_list[num] = i;
                    locate_list[num] = 1;
                    num++;
                }
            } else {
                list_all_char(i,ch,0);
            }
        }
      }
    }
    if (num) {
        for (counter=0; counter<num; counter++) {
            if (locate_list[counter] > 1) {
                list_all_char(plr_list[counter],ch,locate_list[counter]);
            } else {
                list_all_char(plr_list[counter],ch,0);
            }
        }
    }
}


void do_auto_exits(struct char_data * ch)
{
  int door, slen = 0;

  *buf = '\0';
  if (!PRF_FLAGGED2(ch, PRF2_AUTOEXITFULL))
  {
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
      slen += sprintf(buf + slen, "%c ", LOWER(*dirs[door]));

  sprintf(buf2, "&n%s[ Exits: %s]%s&n\r\n", CCCYN(ch, C_NRM),
	  *buf ? buf : "&nNone! &n", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
  }
  else
  {
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_IMMORT)
	sprintf(buf2, "&c[%-5s - [%5d] %s]&n\r\n", dirs[door],
		GET_ROOM_VNUM(EXIT(ch, door)->to_room),
		world[EXIT(ch, door)->to_room].name);
      else {
	sprintf(buf2, "&c[%-5s - ", dirs[door]);
	if ((IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch)) || ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_PERFECTDARK))
	  strcat(buf2, "&cToo dark to tell]&n\r\n");
	else {
	  strcat(buf2, world[EXIT(ch, door)->to_room].name);
	  strcat(buf2, "]&n\r\n");
	}
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("&cExits:&n\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("&c None.&n]\r\n", ch);
  }
}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("&nYou can't see a damned thing, you're blind!&n\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_IMMORT)
	sprintf(buf2, "&n%-5s - [%5d] %s&n\r\n", dirs[door],
		GET_ROOM_VNUM(EXIT(ch, door)->to_room),
		world[EXIT(ch, door)->to_room].name);
      else {
	sprintf(buf2, "%-5s - ", dirs[door]);
	if ((IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch)) || ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_PERFECTDARK))
	  strcat(buf2, "&nToo dark to tell&n\r\n");
	else {
	  strcat(buf2, world[EXIT(ch, door)->to_room].name);
	  strcat(buf2, "&n\r\n");
	}
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("&nObvious exits:&n\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("&n None.&n\r\n", ch);
}



void look_at_room(struct char_data * ch, int ignore_brief)
{
  if (!ch->desc)
    return;

  if (GET_POS(ch) <= POS_DEAD)
     return;

  if ((IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) && ((time_info.hours < 22) &&
     (time_info.hours >=5) && !ROOM_FLAGGED(ch->in_room, ROOM_INDOORS)) &&
     (SECT(ch->in_room) != SECT_UNDERWATER))) {
    send_to_char("&nIt is pitch black...&n\r\n", ch);
    return;

  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("&nYou see nothing but infinite darkness...&n\r\n", ch);
    return;
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_PERFECTDARK) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("&nIt is completely dark...&n\r\n", ch);
    return;
  }
  send_to_char(CCCYN(ch, C_NRM), ch);
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
    sprintbit(ROOM_FLAGS(ch->in_room), room_bits, buf);
    sprintf(buf2, "&c[%5d]&n %s &n&c[ %s]&n", GET_ROOM_VNUM(IN_ROOM(ch)),
	    world[ch->in_room].name, buf);
    send_to_char(buf2, ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if ((!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_BRIEF)) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) {
    send_to_char(world[ch->in_room].description, ch);
  if ((ch->in_room == GET_HORSEROOM(ch)) && !AFF_FLAGGED(ch, AFF_MOUNTED) && PLR_FLAGGED(ch, PLR_HASHORSE))
    send_to_char("&RYour horse is grazing here&n\r\n", ch);
   }

  /* autoexits */
  if (!IS_NPC(ch) && (PRF_FLAGGED(ch, PRF_AUTOEXIT) || PRF_FLAGGED2(ch,
PRF2_AUTOEXITFULL)))
    do_auto_exits(ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}



void look_in_direction(struct char_data * ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("&nYou see nothing special.&n\r\n", ch);

    if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "&nThe %s is closed.&n\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    } else if (EXIT_FLAGGED(EXIT(ch, dir), EX_ISDOOR) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "&nThe %s is open.&n\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    }
  } else
    send_to_char("&nNothing special there...&n\r\n", ch);
}

void look_at_horse(struct char_data * ch)
{
 sprintf(buf, 
      "&n&R%s has these current statistics:&n \r\n"
      "&n&RCurrent energy %d / %d&n\r\n"
      "&n&RCurrent level  %d\r\n"
      "&n&RCurrent experience points  %d\r\n"
      "&n&RYour horse is carrying the following items: &n\r\n",
      GET_HORSENAME(ch),GET_HORSEHAPPY(ch), GET_MAXHAPPY(ch), GET_HORSELEVEL(ch), 
      GET_HORSEEXP(ch));
 send_to_char(buf, ch);
  if (GET_HORSEEQ(ch) >= 1)
     send_to_char("A pair of Kraken gills\r\n", ch);
  if (GET_HORSEEQ(ch) >= 2)
     send_to_char("A wings of a pegasus\r\n", ch);
  if (GET_HORSEEQ(ch) >= 3)
     send_to_char("A titanium war helmet\r\n", ch);
  if (GET_HORSEEQ(ch) >= 4)
     send_to_char("The horseshoes of war\r\n", ch);
  if (GET_HORSEEQ(ch) >= 5)
     send_to_char("Metallic skin\r\n", ch);
  if (GET_HORSEEQ(ch) == 6)
     send_to_char("The Grace of Deimos\r\n", ch); 
}

void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("&nLook in what?&n\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "&nThere doesn't seem to be %s %s here.&n\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER)&&
	     (GET_OBJ_TYPE(obj) != ITEM_PCCORPSE) &&
	     (GET_OBJ_TYPE(obj) != ITEM_ASSCORPSE)) {
         if ((GET_OBJ_TYPE(obj) == ITEM_FIREWEAPON) ||
                   (GET_OBJ_TYPE(obj) == ITEM_MISSILE)) {
           sprintf(buf, "&nIt contains %d units of ammunition.&n\r\n", GET_OBJ_VAL(obj,3));
           send_to_char(buf, ch);
     } else {
        send_to_char("&nThere's nothing inside that!&n\r\n", ch);
     }
  } else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER ||
       (GET_OBJ_TYPE(obj) == ITEM_PCCORPSE) ||
       (GET_OBJ_TYPE(obj) == ITEM_ASSCORPSE)) {
      if (OBJVAL_FLAGGED(obj, CONT_CLOSED))
	send_to_char("&nIt is closed.&n\r\n", ch);
      else {
	send_to_char(fname(obj->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char("&n (carried): &n\r\n", ch);
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char("&n (here): &n\r\n", ch);
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char("&n (used): &n\r\n", ch);
	  break;
	}

	list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {		/* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char("&nIt is empty.&n\r\n", ch);
      else {
	if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0)) {
	  sprintf(buf, "&nIts contents seem somewhat murky.&n\r\n"); /* BUG */
	} else {
	  amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
	  sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
	  sprintf(buf, "&nIt's %sfull of a %s liquid.&n\r\n", fullness[amt], buf2);
	}
	send_to_char(buf, ch);
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return (NULL);
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 *
 * Thanks to Angus Mezick <angus@EDGIL.CCMAIL.COMPUSERVE.COM> for the
 * suggested fix to this problem.
 */
void look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = FALSE, j, fnum, i = 0;
  struct char_data *found_char = NULL;
  struct obj_data *obj, *found_obj = NULL;
  char *desc;

  if (!ch->desc)
    return;

  if (!*arg) {
    send_to_char("&nLook at what?&n\r\n", ch);
    return;
  }

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    if (IS_AFFECTED(found_char, AFF_SHROUD))
    {
        int randval = number(-5, 20);
        if(GET_LEVEL(ch) < LVL_IMMORT && GET_TOTAL_LEVEL(ch) - GET_WARRIOR_LEVEL(ch) - GET_THIEF_LEVEL(ch) - 10 + randval
           < GET_TOTAL_LEVEL(found_char) - GET_WARRIOR_LEVEL(found_char) - GET_THIEF_LEVEL(found_char))
        {
           sprintf(buf, "Your magic slips off %s as if %s didn't exist.\r\n", GET_NAME(found_char),HESH(found_char));
           send_to_char(buf, ch); 
           sprintf(buf, "Your shroud protects you from %s's weak spells.\r\n", GET_NAME(ch));
           send_to_char(buf, found_char);
           return;
        }
           sprintf(buf, "%s's magical shroud fails to stop your powerful magic.\r\n", GET_NAME(found_char));
           send_to_char(buf, ch); 
           sprintf(buf, "Your shroud yields easily at the power of %s's spells.\r\n", GET_NAME(ch));
           send_to_char(buf, found_char);
    }

    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
	act("&n$n looks at you.&n", TRUE, ch, 0, found_char, TO_VICT);
      act("&n$n looks at $N.&n", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }

  /* Strip off "number." from 2.foo and friends. */
  if (!(fnum = get_number(&arg))) {
    send_to_char("&nLook at what?&n\r\n", ch);
    return;
  }

  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL && ++i == fnum) {
    page_string(ch->desc, desc, FALSE);
    return;
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }

  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }
  }

  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }

  /* If an object was found back in generic_find */
  if (bits) {
    if (!found)
      show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
  } else if (!found)
    send_to_char("&nYou do not see that here.&n\r\n", ch);
}


ACMD(do_look)
{
  char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("&nYou can't see anything but stars!&n\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_BLIND))
    send_to_char("&nYou can't see a damned thing, you're blind!&n\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("&nIt is pitch black...&n\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_PERFECTDARK) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("&nIt is pitch black...&n\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);   /* glowing red eyes */
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("&nRead what?&n\r\n", ch);
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)			/* "look" alone, without an argument at all */
      look_at_room(ch, 1);
   else if (str_cmp(arg, "horse") == 0  && IN_ROOM(ch) == GET_HORSEROOM(ch) && PLR_FLAGGED(ch, PLR_HASHORSE)) {
     look_at_horse(ch);
  }
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("&nExamine what?&n\r\n", ch);
    return;
  }
  look_at_target(ch, arg);

  generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("&nWhen you look inside, you see:&n\r\n", ch);
      look_in_obj(ch, argument);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("&nYou're broke!&n\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("&nYou have one miserable little gold coin.&n\r\n", ch);
  else {
    sprintf(buf, "&nYou have %d gold coins.&n\r\n", GET_GOLD(ch));
    send_to_char(buf, ch);
  }
}


ACMD(do_score)
{
  char *temp = NULL;
  struct time_info_data playing_time;
  extern const char *pc_class_types[];    

  if (IS_NPC(ch))
    return;
  
  sprintf(buf, "\r\n&W-==============Deimos Mud Score Sheet==============-&n\r\n");
  if (GET_GM_LEVEL(ch) == 0 && GET_LEVEL(ch) < LVL_IMMORT)
    sprintf(buf + strlen(buf), "&BPlayer: %s%s %s %s&n\r\n",
      GET_PRETITLE(ch), GET_NAME(ch),
      GET_LNAME(ch) != NULL ? GET_LNAME(ch) : "", GET_TITLE(ch));
  else
  {
    temp = GET_GM_TITLE(ch);
    sprintf(buf + strlen(buf), "&BPlayer: %s%s %s %s&n\r\n", temp, GET_NAME(ch), 
      GET_LNAME(ch) != NULL ? GET_LNAME(ch) : "", GET_TITLE(ch));
  }
  if (IS_LORD(ch) && GET_LNAME(ch) != NULL)
    sprintf(buf + strlen(buf), "&RYou are the Lord of House %s.&n\r\n",
           GET_LNAME(ch));
   
  strcpy(buf + strlen(buf), "&CMain Class : ");
  sprinttype(ch->player.chclass, pc_class_types, buf + strlen(buf));
  sprintf(buf + strlen(buf), "\r\n&BClan: %s&n", get_clan_name(ch));
  if (GET_LEVEL(ch) >= LVL_IMMORT || GET_TOTAL_LEVEL(ch) < 240)
   sprintf(buf + strlen(buf), "\r\n&MLevels: (Thief: %d) (Warrior: %d) (Mage : %d) (Cleric : %d)&n\r\n", GET_THIEF_LEVEL(ch), GET_WARRIOR_LEVEL(ch), GET_MAGE_LEVEL(ch), GET_CLERIC_LEVEL(ch));
  else
   sprintf(buf + strlen(buf), "\r\n&MYour Training has advanced %d levels past GrandMaster.&n\r\n"
          , GET_GM_LEVEL(ch)); 
  sprintf(buf + strlen(buf), "&GAge   : %d&n\r\n", GET_AGE(ch));
  if (age(ch)->month == 0 && age(ch)->day == 0)
    strcat(buf, "It's your birthday today.\r\n");
  else
    strcat(buf, "\r\n");
  
  sprintf(buf + strlen(buf), "&CHitP: %d / %d   Mana: %d / %d   Move: %d / %d&n\r\n",
GET_HIT(ch), GET_MAX_HIT(ch), GET_MANA(ch), GET_MAX_MANA(ch),
GET_MOVE(ch), GET_MAX_MOVE(ch)); 

  playing_time = *real_time_passed((time(0) - ch->player.time.logon) +
                                  ch->player.time.played, 0);

  sprintf(buf + strlen(buf), "&RArmor: %d   Alignment: %d Playtime: %d day%s / %d hour%s&n\r\n", GET_AC(ch), 
GET_ALIGNMENT(ch), playing_time.day, playing_time.day == 1 ? "" : "s",
playing_time.hours, playing_time.hours == 1 ? "" : "s");

 
  if (GET_LEVEL(ch) < LVL_IMMORT)
    sprintf(buf + strlen(buf), "&BExpUnused : %s (%d) NextLevel: %s&n\r\n", 
num_punct(GET_EXP(ch)), GET_EXP(ch), num_punct(level_exp(GET_CLASS(ch),
GET_LEVEL(ch))));

  sprintf(buf + strlen(buf), "&YGold: %s&n\r\n", num_punct(GET_GOLD(ch)));
  sprintf(buf + strlen(buf), "&WDamroll: %d        Hitroll: %d        Sacri: %d&n\r\n",
GET_DAMROLL(ch), GET_HITROLL(ch), GET_SACRIFICE(ch));
  sprintf(buf + strlen(buf), "&CKills: %d          Deaths: %d        Ratio: %f&n\r\n",
GET_KILLS(ch), GET_DEATHS(ch),
(float)GET_KILLS(ch)/(float)GET_DEATHS(ch));
  sprintf(buf + strlen(buf), "&GStr: %d, Wis: %d, Int: %d, Dex: %d, Con: %d, Cha: %d&n\r\n",
GET_STR(ch), GET_WIS(ch), GET_INT(ch), GET_DEX(ch), GET_CON(ch), GET_CHA(ch));
  sprintf(buf + strlen(buf), "&RYou are carrying&n &G%d&n &Rof max&n &B%d&n &Ritems and &M%d&n &Rof&n &m%d&n &Rweight.&n\r\n", IS_CARRYING_N(ch), CAN_CARRY_N(ch),
IS_CARRYING_W(ch), CAN_CARRY_W(ch));            
  sprintf(buf + strlen(buf), "&WStance:&n ");

  switch (GET_POS(ch)) {
  case POS_DEAD:
    strcat(buf, "&YYou are DEAD!&n\r\n");
    break;
  case POS_MORTALLYW:
    strcat(buf, "&YYou are mortally wounded!  You should seek help!&n\r\n");
    break;
  case POS_INCAP:
    strcat(buf, "&YYou are incapacitated, slowly fading away...&n\r\n");
    break;
  case POS_STUNNED:
    strcat(buf, "&YYou are stunned! You can't move!&n\r\n");
    break;
  case POS_SLEEPING:
    strcat(buf, "&YYou are sleeping.&n\r\n");
    break;
  case POS_RESTING:
    strcat(buf, "&YYou are resting.&n\r\n");
    break;
  case POS_SITTING:
    strcat(buf, "&YYou are sitting.&n\r\n");
    break;
  case POS_FIGHTING:
    if (FIGHTING(ch))
      sprintf(buf + strlen(buf), "&YYou are fighting %s&n.\r\n",
		PERS(FIGHTING(ch), ch));
    else
      strcat(buf, "&YYou are fighting thin air.&n\r\n");
    break;
  case POS_STANDING:
    strcat(buf, "&YYou are standing.&n\r\n");
    break;
  default:
    strcat(buf, "&YYou are floating.&n\r\n");
    break;
  }

  if (PLR_FLAGGED(ch, PLR_HASHORSE)) {
  sprintf(buf + strlen(buf), "&BHorse : %s&n\r\n", GET_HORSENAME(ch)); 
  }
  if (PLR_FLAGGED(ch, PLR_FEDHORSE)) {
  sprintf(buf + strlen(buf), "&CYour horse is currently being cared for.&n\r\n");
  }
  sprintf(buf + strlen(buf), "&WYour current sparring rank is: %d.&n\r\n", GET_SPARRANK(ch));
  sprintf(buf + strlen(buf), "&WYour current economy rank is: %d.&n\r\n", GET_ECONRANK(ch));
  if (PLR_FLAGGED(ch, PLR_ASSASSIN))
   sprintf(buf + strlen(buf), "&WYour current assassin rank is: %d.&n\r\n", GET_ASSASSINRANK(ch));
  
  switch (GET_RELIGION(ch))
  {
  case RELIG_ATHEIST:
    sprintf(buf + strlen(buf), "&wYou are an avowed Atheist.&n\r\n");
  break;
  case RELIG_PHOBOS:
    sprintf(buf + strlen(buf), "&WYou are a peace-loving follower of Phobe.&n\r\n");
  break;
  case RELIG_DEIMOS:
    sprintf(buf + strlen(buf), "&RYou are a chaotic minion of Deimos.&n\r\n");
  break;
  case RELIG_RULEK:
    sprintf(buf + strlen(buf), "&CYou are a balanced disciple of Rulek.&n\r\n");
  break;
  case RELIG_CALIM:
    sprintf(buf + strlen(buf), "&YYou are an energetic believer of Calim.&n\r\n");
  break;
  }

  if (ROMANCE(ch) == 0) 
  {
      strcat(buf, "You are single.\r\n");
  }  
  else if (ROMANCE(ch) == 1 && PARTNER(ch) != NULL) {
      sprintf(buf+ strlen(buf), "You are dating %s.\r\n", PARTNER(ch));
  }
  else if (ROMANCE(ch) == 2 && PARTNER(ch) != NULL) {
      sprintf(buf+ strlen(buf), "You are engaged to %s.\r\n", PARTNER(ch));
  }
  else if (ROMANCE(ch) == 3 && PARTNER(ch) != NULL) {
      sprintf(buf+ strlen(buf), "You are married to %s.\r\n", PARTNER(ch));
  }
  else if (ROMANCE(ch) == 4 && PARTNER(ch) != NULL) {
      sprintf(buf+ strlen(buf), "You are asking out %s.\r\n", PARTNER(ch));
  }
  else if (ROMANCE(ch) == 5 && PARTNER(ch) != NULL) {
      sprintf(buf+ strlen(buf), "You are proposing to %s.\r\n", PARTNER(ch));
  }
  else
      sprintf(buf+ strlen(buf), "Romance error: %d, %s.\r\n", ROMANCE(ch), PARTNER(ch));


  sprintf(buf + strlen(buf), 
"&W-===================================================-&n\r\n");
  send_to_char(buf, ch);
 if (PRF_FLAGGED2(ch, PRF2_AUTOSCORE))
  do_affects(ch, "", 0, 0);
}

char *num_punct(int foo)
{
    int index, index_new, rest;
    char buf[16];
    static char buf_new[16];

    sprintf(buf,"%d",foo);
    rest = strlen(buf)%3;

    for (index=index_new=0;index<strlen(buf);index++,index_new++)
    {
        if (index!=0 && (index-rest)%3==0 )
        {
            buf_new[index_new]=',';
            index_new++;
            buf_new[index_new]=buf[index];
        }
        else
            buf_new[index_new] = buf[index];
    }
    buf_new[index_new]='\0';
    return strdup(buf_new);
}




ACMD(do_inventory)
{
  send_to_char("&nYou are carrying:&n\r\n", ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("&nYou are using:&n\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
	send_to_char(where[i], ch);
	show_obj_to_char(GET_EQ(ch, i), ch, 1);
	found = TRUE;
      } else {
	send_to_char(where[i], ch);
	send_to_char("&nSomething.&n\r\n", ch);
	found = TRUE;
      }
    }
  }
  if (!found) {
    send_to_char("&n Nothing.&n\r\n", ch);
  }
}


ACMD(do_time)
{
  const char *suf;
  int weekday, day;


  if (GET_LEVEL(ch) >= LVL_IMMORT)
  {
     sprintf(buf, "Time: %d\r\n", (int)time(0));
     send_to_char(buf, ch);
  }
  sprintf(buf, "&nIt is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "&nThe %d%s Day of the %s, Year %d.&n\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
  do_date(ch, "", 0, 0);
}


ACMD(do_weather)
{
  const char *sky_look[] = {
    "&ncloudless&n",
    "&ncloudy&n",
    "&nrainy&n",
    "&nlit by flashes of lightning&n"
  };

  if (OUTSIDE(ch)) {
    sprintf(buf, "&nThe sky is %s and %s.&n\r\n", sky_look[weather_info.sky],
	    (weather_info.change >= 0 ? "&nyou feel a warm wind from south&n" :
	     "&nyour foot tells you bad weather is due&n"));
    send_to_char(buf, ch);
  } else
    send_to_char("&nYou have no feeling about the weather at all.&n\r\n", ch);
}

struct help_index_element *find_help(char *keyword, struct char_data *ch)
{
  extern int top_of_helpt;
  int i;
  for (i = 0; i < top_of_helpt; i++)
    if (isname(keyword, help_table[i].keywords))
    {
         return (help_table + i);    
    }

  return NULL;
}

ACMD(do_help)
{
  extern char *help;
  struct help_index_element *this_help;
  char entry[MAX_STRING_LENGTH];

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char("&nNo help available.&n\r\n", ch);
    return;
  }

  if (!(this_help = find_help(argument, ch))) 
  {
        send_to_char("&nThere is no help on that word.&n\r\n", ch);
        sprintf(buf, "&nHELP: %s tried to get help on %s&n", GET_NAME(ch),argument);
        log(buf);
        return;
  }
  if (this_help->min_level > GET_LEVEL(ch)) {
    send_to_char("&nYou are not able to find help for that word.&n\r\n", ch);
    return;
  }
  sprintf(entry, "&W------------------------=Deimos Help=--------------------\r\n%s\r\n%s&n\r\n---------------------------=============---------------------",
this_help->keywords, this_help->entry);
  page_string(ch->desc, entry, 0);
}


struct question_index *find_question(char *keyword)
{
  extern int top_of_questions;
  int i;
  char *temp = NULL;

  for (temp = keyword; *temp; temp++)
        CAP(temp);
  for (i = 0; i < top_of_questions; i++)
    if (isname(keyword, question_table[i].keyword))
      return (question_table + i);

  return NULL;
}


ACMD(do_question)
{
  extern char *help;
  struct question_index *this_question;

  char entry[MAX_STRING_LENGTH];
  int i = 0;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) 
  {
    page_string(ch->desc, help, 0);
    return;
  }

  if (!question_table) 
  {
    send_to_char("&nNo questions available.&n\r\n", ch);
    return;
  }
  while (i < top_of_questions && !isname(argument,
         question_table[i].keyword))
  { 
    i++;
  }
  
  if (!(this_question = find_question(argument)))
  {
      send_to_char("&nThere is no help on that word.&n\r\n", ch);
      sprintf(buf, "&nQuestion: %s tried to get help on %s&n", GET_NAME(ch), argument);
      log(buf);
      return;
  }

  sprintf(entry,
"&W------------------------=Deimos Help=--------------------\r\n%s\r\n%s&n\r\n---------------------------=============---------------------",
this_question->keyword, this_question->entry);

  page_string(ch->desc, entry, 0);

}


/*********************************************************************
* New 'do_who' by Daniel Koepke [aka., "Argyle Macleod"] of The Keep *
* With Sorting                                                       *
******************************************************************* */

	/* Sorter By: Jorgen, Made Compatable by Ryan Mulligan*/
	/* do_who() and auxiliary functions */
	/* this function uses qsort */


	struct char_data *theWhoList[500];

   /* Comparison function for Total Mortal Levels	*/
	int compareChars(const void *l, const void *r) {

	    struct char_data **left;
	    struct char_data **right;
            int leftlevel, rightlevel;
            bool immsort = FALSE;

	    left = (struct char_data **)l;
	    right = (struct char_data **)r;
            if (GET_LEVEL(*left) >= LVL_IMMORT && GET_LEVEL(*right) >= LVL_IMMORT)
            { //Immortals only care about level
              leftlevel = GET_LEVEL(*left);
              rightlevel = GET_LEVEL(*right);
              immsort = TRUE;
            }
            else
            { // Everyone else cares about everything
              // Next 4 lines added by Mobius in attempt to fix html wholist output
              if (GET_LEVEL(*left) < LVL_IMMORT && GET_LEVEL(*right) >= LVL_IMMORT)
                return 1;
              if (GET_LEVEL(*left) >= LVL_IMMORT && GET_LEVEL(*right) < LVL_IMMORT)
                return -1;
              leftlevel = GET_TOTAL_LEVEL(*left) + GET_GM_LEVEL(*left);
              rightlevel = GET_TOTAL_LEVEL(*right) + GET_GM_LEVEL(*right);
            }
            if (immsort && leftlevel == rightlevel 
            && PLR_FLAGGED2(*right, PLR2_REALIMM)
            && !PLR_FLAGGED2(*left, PLR2_REALIMM))
            {
              return 1;
            }
            if (immsort && leftlevel == rightlevel 
            && !PLR_FLAGGED2(*right, PLR2_REALIMM)
            && PLR_FLAGGED2(*left, PLR2_REALIMM))
            {
              return -1;
            }

	    if(leftlevel < rightlevel)
	        return 1;
	    else if(leftlevel == rightlevel)
   	      return 0;
	    else
   	     return -1;
	}

char *WHO_USAGE =
  "Usage: who [minlev[-maxlev]] [-n name] [-c classes] [-rzqimo]\r\n"
  "\r\n"
  "Classes: (M)age, (C)leric, (T)hief, (W)arrior\r\n"
  "\r\n"
  " Switches: \r\n"
  "_.,-'^'-,._\r\n"
  "\r\n"
  "  -r = who is in the current room\r\n"
  "  -z = who is in the current zone\r\n"
  "\r\n"
  "  -q = only show questers\r\n"
  "  -i = only show immortals\r\n"
  "  -m = only show mortals\r\n"
  "  -o = only show outlaws\r\n"
  "  -a = Show title without clan affilations\r\n"
  "\r\n";

  const char *BuildLevels[LVL_IMPL -(LVL_IMMORT-1)] = 
  {
    "   Architect   ",
    "    Creator    ",
    "    Designer   ",
    "     Maker     ",
    "     Coder     ",
    "* Implementor *"
  };
  
  const char *ImmLevels[LVL_IMPL -(LVL_IMMORT-1)] = 
  {
    "    Immortal   ",
    "Higher Immortal",
    "     Deity     ",
    "    OverSeer   ",
    "    Steward    ",
    "* Implementor *"
  };


ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *wch;
  char Imm_buf[MAX_STRING_LENGTH];
  char Mort_buf[MAX_STRING_LENGTH];
  char name_search[MAX_NAME_LENGTH+1];
  char mode;
  time_t mytime;
  int day, hour, min;
  int low = 0, high = LVL_IMPL, showclass = 0;
  bool who_room = FALSE, who_zone = FALSE, who_quest = 0;
  bool outlaws = FALSE, noimm = FALSE, nomort = FALSE, showclan = TRUE;
    int noElements = 0;
    int curEl;

  int Wizards = 0, Mortals = 0;
  size_t i;



  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  /* the below is from stock CircleMUD -- found no reason to rewrite it */
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);       /* just in case; we destroy arg in the switch */
  switch (mode) {
   case 'o':
	outlaws = TRUE;
	strcpy(buf, buf1);
	break;
      case 'z':
	who_zone = TRUE;
	strcpy(buf, buf1);
	break;
      case 'q':
	who_quest = TRUE;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = TRUE;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
   case 'i':
     nomort = TRUE;
     strcpy(buf, buf1);
   break;
   case 'm':
     noimm = TRUE;
     strcpy(buf, buf1);
   break;
   case 'a':
     showclan = FALSE;
     strcpy(buf, buf1);
   break;
   default:
	send_to_char(WHO_USAGE, ch);
	return;
   }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_USAGE, ch);
      return;
    }
  }				/* end while (parser) */

  /* Begin Mortal Who-List Utils */
  strcpy(Mort_buf,"&wMortals currently online&n\r\n&R------------------------&n\r\n   Th Wa Ma Cl\r\n");
  /* Load Array for Mortals */
  for(noElements = 0, d = descriptor_list; d; d = d->next) {
     /* Conditions for Listing */
     if (STATE(d) != CON_PLAYING)
      continue;
     if (d->original)
        wch = d->original;
     else if (!(wch = d->character))
        continue;

     if (GET_LEVEL(wch) >= LVL_IMMORT)
     	 continue;

     if (!CAN_SEE(ch, wch))
        continue;
     if (GET_LEVEL(wch) < low || GET_LEVEL(wch) > high)
        continue;
     if (PRF_FLAGGED(wch, PRF_WHOINVIS) && GET_LEVEL(ch) < LVL_IMMORT)
        continue;
     if ((nomort && GET_LEVEL(wch) < LVL_IMMORT))
        continue;
     if (*name_search && str_cmp(GET_NAME(wch), name_search) && !strstr(GET_TITLE(wch), name_search))
        continue;
     if (outlaws && !PLR_FLAGGED(wch, PLR_KILLER) && !PLR_FLAGGED(wch, PLR_THIEF))
        continue;
     if (who_quest && !PRF_FLAGGED(wch, PRF_QUEST))
        continue;
     if (who_zone && world[ch->in_room].zone != world[wch->in_room].zone)
        continue;
     if (who_room && (wch->in_room != ch->in_room))
       continue;
     if (showclass && !(showclass & (1 << GET_CLASS(wch))))
       continue;

     /* End Conditions */

     theWhoList[noElements++] = wch;
    }

    /* Sort it using the built in libc-quicksort routine */
  qsort(theWhoList, noElements, sizeof(struct char_data  *), compareChars);

    /* Mortal Buff Printer */
  for(curEl = 0; curEl < noElements; curEl++) {
        wch = theWhoList[curEl];
        /*Default Output */
        if (!PRF_FLAGGED(wch, PRF_ANON) && !PRF_FLAGGED(wch, PRF_WHOINVIS)) {
          if (GET_TOTAL_LEVEL(wch) < 240)  
          sprintf(Mort_buf, "%s&n[ {%2d %2d %2d %2d} ] %s %s",Mort_buf,
        	GET_THIEF_LEVEL(wch), GET_WARRIOR_LEVEL(wch), GET_MAGE_LEVEL(wch), GET_CLERIC_LEVEL(wch),
             	GET_NAME(wch), GET_TITLE(wch));
          else
          sprintf(Mort_buf, "%s&n[&c GM:  %2d   :GM &w] %s %s",Mort_buf,
        	GET_GM_LEVEL(wch),
             	GET_NAME(wch), GET_TITLE(wch));
 
         Mortals++;
        }
        if (GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(wch) < LVL_IMMORT && PRF_FLAGGED(wch, PRF_ANON) && !PRF_FLAGGED(wch, PRF_WHOINVIS) && !PLR_FLAGGED2(wch, PLR2_REVIEW)) {
        	sprintf(Mort_buf, "%s&n[&w - Anonymous - &n] %s %s", Mort_buf,
         GET_NAME(wch), GET_TITLE(wch));
         Mortals++;
        }
        if ((GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(wch) < LVL_IMMORT &&
PRF_FLAGGED(wch, PRF_ANON) && !PRF_FLAGGED(wch, PRF_WHOINVIS) && !PLR_FLAGGED2(wch, PLR2_REVIEW)) || (PRF_FLAGGED(wch, PRF_WHOINVIS))) {
          if (GET_TOTAL_LEVEL(wch) < 240)  
            sprintf(Mort_buf, "%s&n[ {%2d %2d %2d %2d} ] %s %s", Mort_buf,
            GET_THIEF_LEVEL(wch), GET_WARRIOR_LEVEL(wch),
            GET_MAGE_LEVEL(wch), GET_CLERIC_LEVEL(wch),
            GET_NAME(wch), GET_TITLE(wch));
          else
          sprintf(Mort_buf, "%s&n[&c GM:  %2d   :GM &w] %s %s",Mort_buf,
        	GET_GM_LEVEL(wch),
             	GET_NAME(wch), GET_TITLE(wch));

          Mortals++;
        }
      /* Who String Additions */
     *buf = '\0';

     if (GET_INVIS_LEV(wch) && GET_LEVEL(wch) >= LVL_IMMORT)
       sprintf(buf, "%s &w(i%d)&n", buf, GET_INVIS_LEV(wch));
     else if (IS_AFFECTED(wch, AFF_INVISIBLE))
       strcat(buf, " &w(invis)&n");

     if (PLR_FLAGGED(wch, PLR_MAILING))
       strcat(buf, " &w(mailing)&n");
     else if (PLR_FLAGGED(wch, PLR_WRITING))
       strcat(buf, " &w(writing)&n");

     if (showclan && (GET_CLAN(wch) != CLAN_NONE || GET_CLAN(wch) != CLAN_UNDEFINED)) {
        if (showclan) {
           sprintf(buf1, " %s", get_clan_abbrev(wch));
           strcat(buf, buf1);
        } 
     }

     if (PRF_FLAGGED(wch, PRF_DEAF))
       strcat(buf, " &w(deaf)&n");
     if (PRF_FLAGGED(wch, PRF_NOTELL))
       strcat(buf, " &w(notell)&n");
     if (PRF_FLAGGED(wch, PRF_QUEST))
       strcat(buf, " &w(Quest)&n");
     if (PRF_FLAGGED2(wch, PRF2_BUILDWALK))
      strcat(buf, " &w(Buildwalk)&n");
     if (PLR_FLAGGED(wch, PLR_THIEF))
       strcat(buf, " &w(THIEF)&n");
     if (PLR_FLAGGED(wch, PLR_KILLER))
       strcat(buf, " &w(KILLER)&n");
     if (GET_LEVEL(wch) >= LVL_IMMORT)
       strcat(buf, CCNRM(ch, C_SPR));
     if (PRF_FLAGGED(wch, PRF_AFK))
       strcat(buf, " &w(AFK)&n");
     if (PRF_FLAGGED(wch, PRF_ANON) && GET_LEVEL(ch) >= LVL_IMMORT)
       strcat(buf, " &w(Anon)&n");
     if (PRF_FLAGGED(wch, PRF_WHOINVIS) && GET_LEVEL(ch) >= LVL_IMMORT)
       strcat(buf, " &w(WhoInvis)&n");
     strcat(buf, "\r\n");

     /* Who String Additions */

       strcat(Mort_buf, buf);
    }
    /* End Mortal Who List Utils */

    /* Begin Immortal Who List Utils */
     strcpy(Imm_buf, "&wImmortals&n\r\n&R---------&n\r\n");

    /* Load Array for Immortals */
    for(noElements = 0, d = descriptor_list; d; d = d->next) {
     /* Conditions for Listing */
     if (STATE(d) != CON_PLAYING)
      continue;
     if (d->original)
        wch = d->original;
     else if (!(wch = d->character))
        continue;
     if (GET_LEVEL(wch) < LVL_IMMORT)
     	 continue;

     if (!CAN_SEE(ch, wch))
        continue;
     if (GET_LEVEL(wch) < low || GET_LEVEL(wch) > high)
        continue;
     if (PRF_FLAGGED(wch, PRF_WHOINVIS) && GET_LEVEL(ch) < LVL_IMMORT)
        continue;
     if ((noimm && GET_LEVEL(wch) >= LVL_IMMORT))
        continue;
     if (*name_search && str_cmp(GET_NAME(wch), name_search) && !strstr(GET_TITLE(wch), name_search))
        continue;
     if (outlaws && !PLR_FLAGGED(wch, PLR_KILLER) && !PLR_FLAGGED(wch, PLR_THIEF))
        continue;
     if (who_quest && !PRF_FLAGGED(wch, PRF_QUEST))
        continue;
     if (who_zone && world[ch->in_room].zone != world[wch->in_room].zone)
        continue;
     if (who_room && (wch->in_room != ch->in_room))
       continue;
     if (showclass && !(showclass & (1 << GET_CLASS(wch))))
       continue;


     /* End Conditions */

     theWhoList[noElements++] = wch;
    }

   /* Sort it using the built in libc-quicksort routine */
  qsort(theWhoList, noElements, sizeof(struct char_data  *), compareChars);

  /* Immortal Buff Printer */
  for(curEl = 0; curEl < noElements; curEl++) {
     wch = theWhoList[curEl];
     /*Default Output */
     	  sprintf(Imm_buf, "%s&n%s%s %s", Imm_buf,
        GET_PRETITLE(wch), GET_NAME(wch), GET_TITLE(wch));
        Wizards++;

           /* Who String Additions */
     *buf = '\0';

     if (GET_INVIS_LEV(wch) && GET_LEVEL(wch) >= LVL_IMMORT)
       sprintf(buf, "%s &w(i%d)&n", buf, GET_INVIS_LEV(wch));
     else if (IS_AFFECTED(wch, AFF_INVISIBLE))
       strcat(buf, " &w(invis)&n");

     if (PLR_FLAGGED(wch, PLR_MAILING))
       strcat(buf, " &w(mailing)&n");
     else if (PLR_FLAGGED(wch, PLR_WRITING))
       strcat(buf, " &w(writing)&n");

     if (showclan && (GET_CLAN(wch) != CLAN_NONE || GET_CLAN(wch) != CLAN_UNDEFINED)) {
        if (showclan) {
           sprintf(buf1, " %s", get_clan_abbrev(wch));
           strcat(buf, buf1);
        } 
     }

     if (PRF_FLAGGED(wch, PRF_DEAF))
       strcat(buf, " &w(deaf)&n");
     if (PRF_FLAGGED(wch, PRF_NOTELL))
       strcat(buf, " &w(notell)&n");
     if (PRF_FLAGGED(wch, PRF_QUEST))
       strcat(buf, " &w(Quest)&n");
     if (PRF_FLAGGED2(wch, PRF2_BUILDWALK))
      strcat(buf, " &w(Buildwalk)&n");
     if (PLR_FLAGGED(wch, PLR_THIEF))
       strcat(buf, " &w(THIEF)&n");
     if (PLR_FLAGGED(wch, PLR_KILLER))
       strcat(buf, " &w(KILLER)&n");
     if (GET_LEVEL(wch) >= LVL_IMMORT)
       strcat(buf, CCNRM(ch, C_SPR));
     if (PRF_FLAGGED(wch, PRF_AFK))
       strcat(buf, " &w(AFK)&n");
     if (PRF_FLAGGED(wch, PRF_ANON) && GET_LEVEL(ch) >= LVL_IMMORT)
       strcat(buf, " &w(Anon)&n");
     if (PRF_FLAGGED(wch, PRF_WHOINVIS) && GET_LEVEL(ch) >= LVL_IMMORT)
       strcat(buf, " &w(WhoInvis)&n");
     strcat(buf, "\r\n");

     /* Who String Additions */

     strcat(Imm_buf, buf);

  }
  /* End Immortal Who List Utils */

   if ((Wizards + Mortals) == 0)
     strcpy(buf, "&WNo immortals or mortals are currently visible to you...&n\r\n");
   if (Wizards)
     sprintf(buf, "&WThere %s %d visible immortal%s%s&n", (Wizards == 1 ? "is" : "are"),
     Wizards, (Wizards == 1 ? "" : "s"), (Mortals ? " and there " : "."));
   if (Mortals)
     sprintf(buf, "%s&W%s %d visible mortal%s.&n", (Wizards ? buf : "&WThere&n "),
     (Mortals == 1 ? "is" : "are"), Mortals, (Mortals == 1 ? "" : "s"));

   strcat(buf, "\r\n");

   if ((Wizards + Mortals) > boot_high)
     boot_high = Wizards+Mortals;
   sprintf(buf, "%s&YThere is a boot time high of %d player%s.&n\r\n",
   buf, boot_high, (boot_high == 1 ? "" : "s"));
    
    mytime = time(0) - boot_time;
    day = mytime / 86400;
    hour = (mytime / 3600) % 24;
    min = (mytime / 60) % 60;

/*    sprintf(buf, "%s&CUp for %d day%s and %d:%02d&n\r\n", buf, day,
            ((day == 1) ? "" : "s"), hour, min);*/

   if (ISHAPPY)
     sprintf(buf, "%s&MIt is currently HAPPY HOUR!&n\r\n", buf);
   if (!DOUBLEEXP && !DOUBLEGOLD)
   {}
   else if (DOUBLEEXP && !DOUBLEGOLD)
     sprintf(buf, "%s&MDOUBLE EXP is currently turned ON!&n\r\n",
      buf);
   else if (!DOUBLEEXP && DOUBLEGOLD)
     sprintf(buf, "%s&MDOUBLE GOLD is currently turned ON!&n\r\n",
      buf);
   else if (DOUBLEEXP && DOUBLEGOLD)
     sprintf(buf, "%s&MDOUBLE EXP and GOLD are currently turned ON!&n\r\n",
      buf);

   if (GOLDCHIPPER)
     sprintf(buf, "%s&MThe Gold Chipper is currently turned ON!&n\r\n", buf);
   if (QUESTON)
     sprintf(buf, "%s&MA Quest is Currently in Progress!&n\r\n", buf);
   if (!CANASSASSINATE)
     sprintf(buf, "%s&MAssassinations are currently restricted!&n\r\n", buf);

   if (!Mortals)
     sprintf(Imm_buf, "%s\r\n%s", Imm_buf, buf);
   else
     sprintf(Mort_buf, "%s\r\n%s", Mort_buf, buf);

   if (Wizards) 
   {
     page_string(ch->desc, Imm_buf, 0);
     if (Mortals)  
       send_to_char("\r\n", ch);
   }
   if (Mortals) 
   {
     page_string(ch->desc, Mort_buf, 0);
   }

}

 /*********************************************************************
 * End Do_Who                                                         *
 *********************************************************************/

#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

#define MAX_IPMASKS 5

const char *ipmask_table[] = 
{
 "[a34-243-54.ch.medusa.com]\r\n",
 "[m02-d5-344.adl.global.net]\r\n",
 "[s23-10-943.tai.lannet.net]\r\n",
 "[342-34-232.el.signnet.com]\r\n",
 "[85-341-34.de.fnalisp.com]\r\n"
};

ACMD(do_users)
{
  const char *format = "%3d %-7s %-12s %-14s %-3s %-8s ";
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  size_t i;
  int low = 0, high = LVL_IMPL, num_can_see = 0, ipmask = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(USERS_FORMAT, ch);
	return;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */
  strcpy(line,
	 "&GNum Class   Name         State          Idl Login@   Site&n\r\n");
  strcat(line,
	 "&G--- ------- ------------ -------------- --- -------- ------------------------&n\r\n");
  send_to_char(line, ch);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && playing)
      continue;
    if (STATE(d) == CON_PLAYING && deadweight)
      continue;
    if (STATE(d) == CON_PLAYING) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	continue;
      if (GET_INVIS_LEV(tch) > GET_LEVEL(ch))
	continue;

      if (d->original)
	sprintf(classname, "[%2d %s]", GET_LEVEL(d->original),
		CLASS_ABBR(d->original));
      else
	sprintf(classname, "[%2d %s]", GET_LEVEL(d->character),
		CLASS_ABBR(d->character));
    } else
      strcpy(classname, "&G   -   &G");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (STATE(d) == CON_PLAYING && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[STATE(d)]);

    if (d->character && STATE(d) == CON_PLAYING && GET_LEVEL(d->character) < LVL_GOD)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "");

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(line, format, d->desc_num, classname,
		d->original->player.name, state, idletime, timeptr);
      else
	sprintf(line, format, d->desc_num, classname,
		d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
	      state, idletime, timeptr);

    if (d->host && *d->host && !PLR_FLAGGED2(d->character, PLR2_IPMASK))
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else if (PLR_FLAGGED2(d->character, PLR2_IPMASK)) {
      ipmask = number(1, MAX_IPMASKS) -1;
      strcat(line, ipmask_table[ipmask]);
      }
    else
      strcat(line, "[Hostname unknown]&n\r\n");


    if (STATE(d) != CON_PLAYING) {
      sprintf(line2, "&G%s&n", line);
      strcpy(line, line2);
    }
    else
    {
      sprintf(line2, "&W%s&n", line);
      strcpy(line, line2);
    }
    if (STATE(d) != CON_PLAYING ||
		(STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character))) {
      send_to_char(line, ch);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.&n\r\n", num_can_see);
  send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    SEND_TO_Q(motd, ch->desc);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_BUILDERCODE:
    page_string(ch->desc, buildercode, 0);
    break;
  case SCMD_GMS:
    page_string(ch->desc, gms, 0);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    break;
  case SCMD_VERSION:
    send_to_char(strcat(strcpy(buf, circlemud_version), "\r\n"), ch);
    send_to_char(strcat(strcpy(buf, DG_SCRIPT_VERSION), "\r\n"), ch);
    break;
  case SCMD_WHOAMI:
    send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
    break;
  default:
    log("SYSERR: Unhandled case in do_gen_ps. (%d)", subcmd);
    return;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct descriptor_data *d;

  if (!*arg) {
    send_to_char("&nPlayers in your Zone\r\n--------------------&n\r\n", ch);
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || d->character == ch)
	continue;
      if ((i = (d->original ? d->original : d->character)) == NULL)
	continue;
      if (i->in_room == NOWHERE || !CAN_SEE(ch, i))
	continue;
      if (world[ch->in_room].zone != world[i->in_room].zone)
	continue;
      sprintf(buf, "&n%-20s - %s&n\r\n", GET_NAME(i), world[i->in_room].name);
      send_to_char(buf, ch);
    }
  } else {			/* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next) {
      if (i->in_room == NOWHERE || i == ch)
	continue;
      if (!CAN_SEE(ch, i) || world[i->in_room].zone != world[ch->in_room].zone)
	continue;
      if (!isname(arg, i->player.name))
	continue;
      sprintf(buf, "&n%-25s - %s&n\r\n", GET_NAME(i), world[i->in_room].name);
      send_to_char(buf, ch);
      return;
    }
    send_to_char("&nNo-one around by that name.&n\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
			        int recur)
{
  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%5d] %s\r\n",
	    GET_ROOM_VNUM(IN_ROOM(obj)), world[obj->in_room].name);
    send_to_char(buf, ch);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\r\n",
	    PERS(obj->carried_by, ch));
    send_to_char(buf, ch);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\r\n",
	    PERS(obj->worn_by, ch));
    send_to_char(buf, ch);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\r\n",
	    obj->in_obj->short_description, (recur ? ", which is" : " "));
    send_to_char(buf, ch);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\r\n");
    send_to_char(buf, ch);
  }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char("&nPlayers\r\n-------&n\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (STATE(d) == CON_PLAYING) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	  if (d->original)
	    sprintf(buf, "&n%-20s - [%5d] %s (in %s)&n\r\n",
		    GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(d->character)),
		 world[d->character->in_room].name, GET_NAME(d->character));
	  else
	    sprintf(buf, "&n%-20s - [%5d] %s&n\r\n", GET_NAME(i),
		    GET_ROOM_VNUM(IN_ROOM(i)), world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	sprintf(buf, "&nM%3d. %-25s - [%5d] %s\r\n&n", ++num, GET_NAME(i),
		GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
	send_to_char(buf, ch);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (k->name)
       if (ch && k && CAN_SEE_OBJ(ch, k))
         if(isname(arg, k->name)) 
         {
 	   found = 1;
       	   print_object_location(++num, k, ch, TRUE);
         }

    if (!found)
      send_to_char("&nCouldn't find any such thing.&n\r\n", ch);
  }
}



ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("&nYou ain't nothin' but a hound-dog.&n\r\n", ch);
    return;
  }
  *buf = '\0';

  for (i = 1; i < LVL_IMMORT; i++) {
    sprintf(buf + strlen(buf), "&n[%2d] %8d-%-8d : &n", i,
	    level_exp(GET_CLASS(ch), i), level_exp(GET_CLASS(ch), i+1) - 1);
    switch (GET_SEX(ch)) {
    case SEX_MALE:
    case SEX_NEUTRAL:
      strcat(buf, title_male(GET_CLASS(ch), i));
      break;
    case SEX_FEMALE:
      strcat(buf, title_female(GET_CLASS(ch), i));
      break;
    default:
      send_to_char("&nOh dear.  You seem to be sexless.&n\r\n", ch);
      break;
    }
    strcat(buf, "\r\n");
  }
  page_string(ch->desc, buf, 1);
}

ACMD(do_consider)
{
  struct char_data *victim;
  int diff = 0, chardam = 0, victdam = 0, charrounds = 0, victrounds = 0;

  one_argument(argument, buf);

  if (!(victim = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
    send_to_char("&nConsider killing who?&n\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("&nEasy!  Very easy indeed!&n\r\n", ch);
    return;
  }
  /* delete this if statement if you have a pk mud and want the
   * improved consider to work on other players  -rtb
   */
  if (!CAN_ASSASSIN(ch, victim)) {
    send_to_char("&nWould you like to borrow a cross and a shovel?&n\r\n", ch);
    return;
  }
 //Normal Code
  chardam = calc_ave_damage(ch, victim);
  victdam = calc_ave_damage(victim, ch);

  charrounds = GET_HIT(victim) / chardam;
  victrounds = GET_HIT(ch) / victdam;

  sprintf(buf, "&CYou could kill %s, in %d hits.&n\r\n", GET_NAME(victim),
  charrounds);
  send_to_char(buf, ch);
  sprintf(buf, "&R%s could kill You, in %d hits.&n\r\n", GET_NAME(victim),
  victrounds);
  send_to_char(buf, ch);

//  diff = victrounds - charrounds;
  diff = charrounds - victrounds;

  //diff += ; // Convert player levels to mob levels
  if (diff <= -100)
    send_to_char("&nNow where did that chicken go?&n\r\n", ch);
  else if (diff <= -30)
    send_to_char("&nYou could do it with a needle!&n\r\n", ch);
  else if (diff <= -20)
    send_to_char("&nEasy.&n\r\n", ch);
  else if (diff <= -10)
    send_to_char("&nFairly easy.&n\r\n", ch);
  else if (diff == 0)
    send_to_char("&nThe perfect match!&n\r\n", ch);
  else if (diff <= 2)
    send_to_char("&nYou would need some luck!&n\r\n", ch);
  else if (diff <= 5)
    send_to_char("&nYou would need a lot of luck!&n\r\n", ch);
  else if (diff <= 10)
    send_to_char("&nYou would need a lot of luck and great equipment!&n\r\n", ch);
  else if (diff <= 20)
    send_to_char("&nDo you feel lucky, punk?&n\r\n", ch);
  else if (diff <= 50)
    send_to_char("&nAre you mad!?&n\r\n", ch);
  else
    /* removed the last if so no upper limit to diff - rtb */
    send_to_char("&nDeath becomes you!&n\r\n", ch);
}


ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)))
      send_to_char(NOPERSON, ch);
    else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("&nDiagnose who?&n\r\n", ch);
  }
}


const char *ctypes[] = {
  "off", "sparse", "normal", "complete", "\n"
};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "&nYour current color level is %s.&n\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("&nUsage: color { Off | &ySparse&n |&WNormal&n | &GC&Yo&Bm&Mp&Dl&Re&Ct&ne }&n\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  sprintf(buf, "&nYour %scolor%s is now %s.&n\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "&nOFF&n");
  else
    sprintf(buf2, "&n%-3d&n", GET_WIMP_LEV(ch));

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    sprintf(buf,
	  "&n      No Hassle: %-3s    &n"
	  "&n      Holylight: %-3s    &n"
	  "&n     Room Flags: %-3s\r\n&n",
	ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)),
	ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)),
	ONOFF(PRF_FLAGGED(ch, PRF_ROOMFLAGS))
    );
    send_to_char(buf, ch);
  }

  sprintf(buf,
	  "&nHit Pnt Display: %-3s    &n"
	  "&n     Brief Mode: %-3s    &n"
	  "&n Summon Protect: %-3s\r\n&n"

	  "&n   Move Display: %-3s    &n"
	  "&n   Compact Mode: %-3s    &n"
	  "&n       On Quest: %-3s\r\n&n"

	  "&n   Mana Display: %-3s    &n"
	  "&n         NoTell: %-3s    &n"
	  "&n   Repeat Comm.: %-3s\r\n&n"

	  "&n Auto Show Exit: %-3s    &n"
          "&n   Auto Looting: %-3s    &n"
          "&n Auto Splitting: %-3s&n\r\n"

	  "&n           Deaf: %-3s    &n"
	  "&n     Wimp Level: %-3s    &n"
          "&n   Auto Affects: %-3s&n\r\n"

	  "&n Gossip Channel: %-3s    &n"
	  "&nAuction Channel: %-3s    &n"
	  "&n  Grats Channel: %-3s&n\r\n"

	  "&n       AutoTick: %-3s    &n"
	  "&n    Hear Levels: %-3s    &n"
          "&n      AutoLogOn: %-3s&n\r\n"
           
	  "&n       AutoGold: %-3s    &n"
	  "&n Auto Sacrifice: %-3s    &n"
	  "&n    Color Level: %s&n\r\n",

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
  	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),
	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  buf2,
	  ONOFF(PRF_FLAGGED2(ch, PRF2_AUTOSCORE)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),

	  ONOFF(PRF_FLAGGED2(ch, PRF2_AUTOTICK)),
	  ONOFF(PRF_FLAGGED2(ch, PRF2_HEARLEVEL)),
	  ONOFF(PRF_FLAGGED2(ch, PRF2_AUTOLOGON)),

          ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSAC)),
          ctypes[COLOR_LEV(ch)]);

  send_to_char(buf, ch);
}


struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void sort_commands(void)
{
  int a, b, tmp;

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)) || IS_NPC(vict)) {
      send_to_char("&nWho is that?&n\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("&nYou can't see the commands of people above your level.&n\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "&nThe following %s%s are available to %s:&n\r\n",
	  wizhelp ? "&nprivileged &n" : "",
	  socials ? "&nsocials&n" : "&ncommands&n",
	  vict == ch ? "&nyou&n" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if (cmd_info[i].minimum_level >= 0 &&
	GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	(cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      sprintf(buf + strlen(buf), "&n%-11s&n", cmd_info[i].command);
      if (!(no % 7))
	strcat(buf, "\r\n");
      no++;
    }
  }

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}
 void list_scanned_chars(struct char_data * list, struct char_data * ch, int
distance, int door)
{
  const char *how_far[] = {
    "&nclose by&n",
    "&na ways off&n",
    "&nfar off to the&n"
  };

  struct char_data *i;
  int count = 0;
  *buf = '\0';

/* this loop is a quick, easy way to help make a grammatical sentence
   (i.e., "You see x, x, y, and z." with commas, "and", etc.) */

  for (i = list; i; i = i->next_in_room)

/* put any other conditions for scanning someone in this if statement -
   i.e., if (CAN_SEE(ch, i) && condition2 && condition3) or whatever */

    if (CAN_SEE(ch, i))
     count++;

  if (!count)
    return;

  for (i = list; i; i = i->next_in_room) {

/* make sure to add changes to the if statement above to this one also, using
   or's to join them.. i.e.,
   if (!CAN_SEE(ch, i) || !condition2 || !condition3) */

    if (!CAN_SEE(ch, i))
      continue;
    if (!*buf)
      sprintf(buf, "&nYou see %s&n", GET_NAME(i));
    else
      sprintf(buf, "&n%s%s&n", buf, GET_NAME(i));
    if (--count > 1)
      strcat(buf, "&n, &n");
    else if (count == 1)
      strcat(buf, "&n and &n");
    else {
      sprintf(buf2, "&n %s %s.&n\r\n", how_far[distance], dirs[door]);
      strcat(buf, buf2);
    }

  }
  send_to_char(buf, ch);
}

ACMD(do_file)
{
  FILE *req_file;
  int cur_line = 0,
      num_lines = 0,
      req_lines = 0,
      i,
      j;
  int l;
  char field[MAX_INPUT_LENGTH],
       value[MAX_INPUT_LENGTH];

  struct file_struct {
    char *cmd;
    char level;
    char *file;
  } fields[] = {
    { "none",           LVL_IMPL,    "Does Nothing" },
    { "bug",            LVL_IMPL,    "../lib/misc/bugs"},
    { "typo",           LVL_IMMORT, "../lib/misc/typos"},
    { "ideas",          LVL_IMPL,    "../lib/misc/ideas"},
    { "xnames",         LVL_IMPL,    "../lib/misc/xnames"},
    { "levels",         LVL_IMMORT,   "../log/levels" },
    { "rip",            LVL_IMMORT,   "../log/rip" },
    { "players",        LVL_IMMORT,   "../log/newplayers" },
    { "rentgone",       LVL_IMMORT,   "../log/rentgone" },
    { "godcmds",        LVL_IMPL,    "../log/godcmds" },
    { "syslog",         LVL_IMPL,    "../syslog" },
    { "crash",          LVL_IMPL,    "../syslog.CRASH" },
    { "smartmud",       LVL_IMPL,    "../lib/misc/smartmud" },
    { "rents",       LVL_IMPL,    "../lib/misc/rents" },
    { "\n", 0, "\n" }
  };

  skip_spaces(&argument);

  if (!*argument)
  {
    strcpy(buf, "USAGE: file <option> <num lines>\r\n\r\nFile options:&n \r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))

sprintf(buf, "%s%-15s%s&n\r\n", buf, fields[i].cmd, fields[i].file);
    send_to_char(buf, ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if(*(fields[l].cmd) == '\n')
  {
    send_to_char("&nThat is not a valid option!&n\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < fields[l].level)
  {
    send_to_char("&nYou are not godly enough to view that file!&n\r\n", ch);
    return;
  }

  if(!*value)
     req_lines = 15; /* default is the last 15 lines */
  else
     req_lines = atoi(value);

  /* open the requested file */
  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "&nSYSERR: Error opening file %s using 'file' command.&n",
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }

  /* count lines in requested file */
  get_line(req_file,buf);
  while (!feof(req_file))
  {
     num_lines++;
     get_line(req_file,buf);
  }
  fclose(req_file);


  /* Limit # of lines printed to # requested or # of lines in file or
     150 lines */
  if(req_lines > num_lines) req_lines = num_lines;
  if(req_lines > 150) req_lines = 150;


  /* close and re-open */
  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "&nSYSERR: Error opening file %s using 'file' command.&n",
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }

  buf2[0] = '\0';

  /* and print the requested lines */
  get_line(req_file,buf);
  while (!feof(req_file))
  {
     cur_line++;
     if(cur_line > (num_lines - req_lines))
     {
        sprintf(buf2,"%s%s\r\n",buf2, buf);
     }
     get_line(req_file,buf);
   }
   page_string(ch->desc, buf2, 1);


   fclose(req_file);
}


ACMD(do_affects)
 {
  struct affected_type* af;
  long temp_aff[40];
  bitvector_t bitvector = AFF_FLAGS(ch);
  // struct obj_data *obj;
  int i = 0, j, nr;
  bool ignore;
  char sname[256];
  char *temp1 = NULL,*temp2 = NULL;
  //char added_aff[MAX_STRING_LENGTH];
  if (GET_LEVEL(ch) < 30)
  {
     send_to_char("&RYou must be level 30 to sense enchantments.&n\r\n", ch);
     return; 
  }
  send_to_char("&nYou carry these enchantments: &n\r\n", ch);

 for (i = 0,af = ch->affected; af; af = af->next) 
 {  
   *sname = '\0';
     strcpy(sname, skill_name(af->type));
   if (GET_LEVEL(ch) >= 30) 
   {  // first state is here
     sprintf(buf, "&n%-22s affects you for %d hours&n\r\n",
            (sname ? sname : "&n"), af->duration+1);
     send_to_char(buf, ch);
   }
   if (af->bitvector && 
      (!af->next || af->next->bitvector != af->bitvector))
   {
     // third state is here
     temp_aff[i++] = af->bitvector;
     //sprintbit(af->bitvector, affected_bits, added_aff);
     //sprintf(buf1, "&n%35sadds %s level %d%s&n\r\n", CCCYN(ch, C_NRM), added_aff, af->power, CCNRM(ch, C_NRM));
     //send_to_char(buf1, ch);
   }
  
 }
     send_to_char("&nYour objects give you the following enchantments: &n\r\n", ch);

  *buf = '\0';
  for (nr = 0; bitvector; bitvector >>= 1) 
  {
    ignore = FALSE;
    for (j = 0; j < i; j++)
    {
        *buf1 = '\0';
        *buf2 = '\0';
        temp1 = NULL; temp2 = NULL;
        sprintbit(temp_aff[j], affected_bits, buf1);
        sprintbit(bitvector, affected_bits, buf2);
        temp1 = strdup(buf1);
        temp2 = strdup(buf2);
        if (*temp1) skip_spaces(&temp1);
        if (*temp2) skip_spaces(&temp2);
        if (strcmp(buf1, buf2) == 0)
           ignore = TRUE;
    }    

    if (IS_SET(bitvector, 1) && !ignore) 
    {
      if (*affected_bits[nr] != '\n') {
       
       // sprintf(buf1, "%d: ", (int)(bitvector)); 
       // strcat(buf, buf1);

        strcat(buf, affected_bits[nr]);
        strcat(buf, "  ");
      } else
        strcat(buf, "Something ");
    }
    if (*affected_bits[nr] != '\n')
      nr++;
  }

  if (!*buf)
    strcpy(buf, "None ");

      send_to_char(buf, ch);
}

int thistempclass = 0;//Used to sort skills, ugh Global var.

int compareSkills(const void *l, const void *r) 
{

            int **left;
            int **right;
            int leftlevel, rightlevel;

            left = (int **)l;
            right = (int **)r;

            leftlevel = spell_info[**left].min_level[thistempclass];
            rightlevel = spell_info[**right].min_level[thistempclass];

            if(leftlevel > rightlevel)
                return 1;
            else if(leftlevel == rightlevel)
              return 0;
            else
             return -1;
}

ACMD(do_skill)
{
  int *theSkillList[500];
  int noElements = 0;
  int curEl = 0;
  int *i, sortpos;

  if (IS_NPC(ch))
    return;
  if (GET_LEVEL(ch) >= LVL_IMMORT) 
    send_to_char("&nThis only lists skills below level 60&n\r\n", ch);

 if (subcmd == SCMD_THIEF)
 {
  int temp = GET_CLASS(ch);

  GET_CLASS(ch) = CLASS_THIEF;
  thistempclass = (int)CLASS_THIEF;

  sprintf(buf, "&nThe following Thief skills are available eventually.&n\r\n");
  sprintf(buf + strlen(buf), "&nLvl: Skill&n\r\n");
  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) 
  {
     i = &spell_sort_info[sortpos];

     if (60 >= spell_info[*i].min_level[(int) GET_CLASS(ch)]) 
     {
       theSkillList[noElements++] = i;
     }
  }
  qsort(theSkillList, noElements, sizeof(int *), compareSkills);   

  for (curEl = 0; curEl < noElements; curEl++)
  {
   i = theSkillList[curEl];
   sprintf(buf, "&n%-3d: %-20s &n\r\n",
   spell_info[*i].min_level[(int) GET_CLASS(ch)], spell_info[*i].name);
   strcat(buf2, buf);

   if (strlen(buf2) >= MAX_STRING_LENGTH - 32) 
   {
      strcat(buf2, "&n**OVERFLOW**&n\r\n");
      break;
   }
  }
 GET_CLASS(ch) = temp;
 page_string(ch->desc, buf2, 1);
 }

 if (subcmd == SCMD_CLERIC)
 {
  int temp = GET_CLASS(ch);

  GET_CLASS(ch) = CLASS_CLERIC;
  thistempclass = (int)CLASS_CLERIC;

  sprintf(buf, "&nThe following Cleric skills are available eventually.&n\r\n");
  sprintf(buf + strlen(buf), "&nLvl: Skill&n\r\n");
  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) 
  {
     i = &spell_sort_info[sortpos];

     if (60 >= spell_info[*i].min_level[(int) GET_CLASS(ch)]) 
     {
       theSkillList[noElements++] = i;
     }
  }
  qsort(theSkillList, noElements, sizeof(int *), compareSkills);   

  for (curEl = 0; curEl < noElements; curEl++)
  {
   i = theSkillList[curEl];
   sprintf(buf, "&n%-3d: %-20s &n\r\n",
   spell_info[*i].min_level[(int) GET_CLASS(ch)], spell_info[*i].name);
   strcat(buf2, buf);

   if (strlen(buf2) >= MAX_STRING_LENGTH - 32) 
   {
      strcat(buf2, "&n**OVERFLOW**&n\r\n");
      break;
   }
  }
 GET_CLASS(ch) = temp;
 page_string(ch->desc, buf2, 1);
 }

 if (subcmd == SCMD_WARRIOR)
 {
  int temp = GET_CLASS(ch);

  GET_CLASS(ch) = CLASS_WARRIOR;
  thistempclass = (int)CLASS_WARRIOR;

  sprintf(buf, "&nThe following warrior skills are available eventually.&n\r\n");
  sprintf(buf + strlen(buf), "&nLvl: Skill&n\r\n");
  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) 
  {
     i = &spell_sort_info[sortpos];

     if (60 >= spell_info[*i].min_level[(int) GET_CLASS(ch)]) 
     {
       theSkillList[noElements++] = i;
     }
  }
  qsort(theSkillList, noElements, sizeof(int *), compareSkills);   

  for (curEl = 0; curEl < noElements; curEl++)
  {
   i = theSkillList[curEl];
   sprintf(buf, "&n%-3d: %-20s &n\r\n",
   spell_info[*i].min_level[(int) GET_CLASS(ch)], spell_info[*i].name);
   strcat(buf2, buf);

   if (strlen(buf2) >= MAX_STRING_LENGTH - 32) 
   {
      strcat(buf2, "&n**OVERFLOW**&n\r\n");
      break;
   }
  }
 GET_CLASS(ch) = temp;
 page_string(ch->desc, buf2, 1);
 }

 if (subcmd == SCMD_MAGE)
 {
  int temp = GET_CLASS(ch);

  GET_CLASS(ch) = CLASS_MAGE;
  thistempclass = (int)CLASS_MAGE;

  sprintf(buf, "&nThe following Mage skills are available eventually.&n\r\n");
  sprintf(buf + strlen(buf), "&nLvl: Skill&n\r\n");
  strcpy(buf2, buf);

  for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++) 
  {
     i = &spell_sort_info[sortpos];

     if (60 >= spell_info[*i].min_level[(int) GET_CLASS(ch)]) 
     {
       theSkillList[noElements++] = i;
     }
  }
  qsort(theSkillList, noElements, sizeof(int *), compareSkills);   

  for (curEl = 0; curEl < noElements; curEl++)
  {
   i = theSkillList[curEl];
   sprintf(buf, "&n%-3d: %-20s &n\r\n",
   spell_info[*i].min_level[(int) GET_CLASS(ch)], spell_info[*i].name);
   strcat(buf2, buf);

   if (strlen(buf2) >= MAX_STRING_LENGTH - 32) 
   {
      strcat(buf2, "&n**OVERFLOW**&n\r\n");
      break;
   }
  }
 GET_CLASS(ch) = temp;
 page_string(ch->desc, buf2, 1);
 }

}

ACMD(do_gwho)
{
  struct descriptor_data *d;
  struct char_data *wch;
  struct follow_type *fol;

  char group_buf[MAX_STRING_LENGTH];
  strcpy(group_buf, "&wCurrently Existing Groups:&n\r\n&R-------------------------------&n\r\n");

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING)
      continue;
    if (d->original)
      wch = d->original;
    else if (!(wch = d->character))
      continue;
    if (wch->master)
      continue;
    if (IS_NPC(wch))
      continue;
    if (!AFF_FLAGGED(wch, AFF_GROUP))
      continue;
    if (!wch->followers)
      continue;

  sprintf(buf, "&wLeader: [%10s %2s %2d] is Leading %s\r\n", GET_NAME(wch), 
         CLASS_ABBR(wch), GET_LEVEL(wch), 
         GET_GTITLE(wch) == NULL ? GET_GTITLE(wch) = "by Phobe's Will" : GET_GTITLE(wch));
  if (PRF_FLAGGED(wch, PRF_ANON))
  sprintf(buf, "&wLeader: [%10s [ANON] is Leading %s\r\n", GET_NAME(wch), 
         GET_GTITLE(wch) == NULL ? GET_GTITLE(wch) = "by Phobe's Will" : GET_GTITLE(wch));

  strcat(group_buf, buf);

  for (fol = wch->followers; fol; fol = fol->next) {
    if (IS_NPC(fol->follower)) continue;
    if (AFF_FLAGGED(fol->follower, AFF_GROUP)) {
    if (!PRF_FLAGGED(fol->follower, PRF_ANON)) {
    sprintf(buf, "&w        [%10s %2s %2d] Gjob:(%s)&n\r\n", PERS(fol->follower, wch), 
           CLASS_ABBR(fol->follower), GET_LEVEL(fol->follower), 
           GET_GJOB(fol->follower) == NULL  ? GET_GJOB(fol->follower) = "" : GET_GJOB(fol->follower));
    }
    else
    sprintf(buf, "&w        [%10s [ANON] Gjob:(%s)&n\r\n", PERS(fol->follower, wch), 
           GET_GJOB(fol->follower) == NULL  ? GET_GJOB(fol->follower) = "" : GET_GJOB(fol->follower));

    strcat(group_buf, buf);
      }

   }
  }
  send_to_char(group_buf, ch);
}


void cls_screen(struct descriptor_data *d) {
    SEND_TO_Q("\033[H\033[J", d);
    return;
}


ACMD(do_sparrank) {
 page_string(ch->desc, sparrank, 0);
}

ACMD(do_econrank) {
  sprintf(buf, "The last economy reset was:\r\n       %s\r\n", ctime(&LAST_ECON_RESET));
  send_to_char(buf, ch);
 page_string(ch->desc, econrank, 0);
}
ACMD(do_assassinrank) {
 page_string(ch->desc, assassinrank, 0);
}

ACMD(do_atheist) {
 page_string(ch->desc, areligion, 0);
}
ACMD(do_phobos) {
 page_string(ch->desc, preligion, 0);
}
ACMD(do_deimos) {
 page_string(ch->desc, dreligion, 0);
}
ACMD(do_calim) {
 page_string(ch->desc, creligion, 0);
}
ACMD(do_rulek) {
 page_string(ch->desc, rreligion, 0);
}

char *GET_GM_TITLE(struct char_data *ch)
{
   switch(GET_CLASS(ch))
    {
      case CLASS_WARRIOR:
        return "The Warlord, ";
      break;
      case CLASS_THIEF:
        return "The Rogue, ";
      break;
      case CLASS_MAGIC_USER:
        return "The Archmage, ";
      break;
      case CLASS_CLERIC:
        if (GET_SEX(ch) == SEX_FEMALE)
          return "The Priestess, ";
        else
          return "The Priest, ";
      break;
    }
   return " ";
}
void make_who2html(void)
{
   int port = 6666;
   void proc_color(char *inbuf, int color);
   char buf[MAX_STRING_LENGTH];
   extern struct descriptor_data *descriptor_list;
   FILE *opf;
   struct descriptor_data *d;
   struct char_data *ch;
   char mvcmd[256];
   bool immortals = TRUE;
   struct char_data *theWhoList[500];
   int noElements = 0, i;

   if ((opf = fopen("who.tmp", "w")) == 0) {
     perror("Failed to open who.tmp for writing.");
     return;
   }

   fprintf(opf, "<HTML>\n<HEAD>\n<TITLE>Who's on?</TITLE>\n"
                "<META HTTP-EQUIV=Refresh CONTENT='60;"
                "URL=who%d.html'>\n", port);

   fprintf(opf, "</HEAD>\n<BODY bgcolor=#333333>"
                "<pre><font color=#C0C0C0 face='courier new, courier'>\n");

   for (d = descriptor_list; d; d = d->next)
    { 
       if (STATE(d) != CON_PLAYING)
         continue;
       if (d->original)
         ch = d->original;
       else if (!(ch = d->character))
         continue;
       if (GET_INVIS_LEV(ch))
         continue;
       if (PRF_FLAGGED(ch, PRF_WHOINVIS))
         continue;
      theWhoList[noElements++] = ch;
     }
    /* Sort it using the built in libc-quicksort routine */
     qsort(theWhoList, noElements, sizeof(struct char_data  *),compareChars);

  for (i = 0; i < noElements; i++)
  {
    ch = theWhoList[i];
    if (i == 0 && GET_LEVEL(ch) >= LVL_IMMORT)
    {
      sprintf(buf, "<font color=#FF0000>Immortals<br>---------</font>\n");
      fprintf(opf, buf);
    }
    if (immortals && GET_LEVEL(ch) < LVL_IMMORT)
    {
      sprintf(buf, "\n<font color=#FF0000>Mortals\n---------</font>\n   Th Wa Ma Cl\n");
      fprintf(opf, buf);
      immortals = FALSE;
    }

      if (GET_LEVEL(ch) < LVL_IMMORT)
      {
        if (!PRF_FLAGGED(ch, PRF_ANON) && !PRF_FLAGGED(ch, PRF_WHOINVIS))
        {
          if (GET_TOTAL_LEVEL(ch) < 240)  
            sprintf(buf,
                      "[ {%2d %2d %2d %2d} ] %s %s\n",
                      GET_WARRIOR_LEVEL(ch),  GET_THIEF_LEVEL(ch),
                      GET_CLERIC_LEVEL(ch),  GET_MAGE_LEVEL(ch),
                      GET_NAME(ch), GET_TITLE(ch));
          else
            sprintf(buf,
                "[ GM:  %2d   :GM ] %s %s\n",
        	GET_GM_LEVEL(ch),
             	GET_NAME(ch), GET_TITLE(ch));
         }
         else
           sprintf(buf,
                "[ - Anonymous - ] %s %s\n",
                GET_NAME(ch), GET_TITLE(ch));
       }
       else {
         sprintf(buf, "%s%s %s\n",
                      GET_PRETITLE(ch) ? GET_PRETITLE(ch) : "", GET_NAME(ch), GET_TITLE(ch));
       }
       proc_color(buf, 0);
       fprintf(opf, buf);

  }

  if (noElements == 0)
       fprintf(opf,"Nobody!\n");

   fprintf(opf, "</pre></font>\n</BODY>\n</HTML>\n");
   fclose(opf);
   sprintf(mvcmd, "mv who.tmp ~/public_html/who%d.html &", port);
   system(mvcmd);
}
#define MAX_PLAYER_GRAPH_SLOTS 10000

int playergraph[MAX_PLAYER_GRAPH_SLOTS];
int pgraph_index = 0;


ACMD(do_playergraph)
{
  int i = 0;
  for (i = 0; i < pgraph_index;i++)
  {
   

  }

}

void update_playergraph(int sockets_playing)
{
  playergraph[pgraph_index++] = sockets_playing;
}
