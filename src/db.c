/*
************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __DB_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "constants.h"
#include "assemblies.h"
#include "dg_scripts.h"
#include "diskio.h"
#include "pfdefaults.h"
#include "clan.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
**************************************************************************/

struct room_data *world = NULL;	/* array of rooms		 */
room_rnum top_of_world = 0;	/* ref to top element of world	 */

struct char_data *character_list = NULL; /* global linked list of chars */

struct index_data **trig_index; /* index table for triggers      */
int top_of_trigt = 0;           /* top of trigger index table    */
long max_id = MOBOBJ_ID_BASE;   /* for unique mob/obj id's       */
	
struct index_data *mob_index;	/* index table for mobile file	 */
struct char_data *mob_proto;	/* prototypes for mobs		 */
mob_rnum top_of_mobt = 0;	/* top of mobile index table	 */

struct obj_data *object_list = NULL;	/* global linked list of objs	 */
struct index_data *obj_index;	/* index table for object file	 */
struct obj_data *obj_proto;	/* prototypes for objs		 */
obj_rnum top_of_objt = 0;	/* top of object index table	 */

struct zone_data *zone_table;	/* zone table			 */
zone_rnum top_of_zone_table = 0;/* top element of zone tab	 */
struct message_list fight_messages[MAX_MESSAGES];	/* fighting messages	 */

struct player_index_element *player_table = NULL;	/* index to plr file	 */
FILE *player_fl = NULL;		/* file desc of player file	 */
int top_of_p_table = 0;		/* ref to top of table		 */
int top_of_p_file = 0;		/* ref of size of p file	 */
long top_idnum = 0;		/* highest idnum in use		 */

int no_mail = 0;		/* mail disabled?		 */
int mini_mud = 0;		/* mini-mud mode?		 */
int no_rent_check = 0;		/* skip rent check on boot?	 */
time_t boot_time = 0;		/* time of mud boot		 */
time_t LAST_ECON_RESET = 0;     /* last time economy was reset   */
int minutesplayed = 0;
int circle_restrict = 0;	/* level of game restriction	 */
room_rnum r_mortal_start_room;	/* rnum of mortal start room	 */
room_rnum r_immort_start_room;	/* rnum of immort start room	 */
room_rnum r_frozen_start_room;	/* rnum of frozen start room	 */

char *credits = NULL;		/* game credits			 */
char *news = NULL;		/* mud news			 */
char *buildercode = NULL;           /* new help files                */
char *motd = NULL;		/* message of the day - mortals */
char *imotd = NULL;		/* message of the day - immorts */
char *help = NULL;		/* help screen			 */
char *info = NULL;		/* info page			 */
char *wizlist = NULL;		/* list of higher gods		 */
char *immlist = NULL;		/* list of peon gods		 */
char *background = NULL;	/* background story		 */
char *handbook = NULL;		/* handbook for new immortals	 */
char *policies = NULL;		/* policies page		 */
char *smartmud = NULL;		/* AHAHAHAHAHA		 */
char *sparrank = NULL;          /* duuuuuuuuuuuuuuuur */
char *econrank = NULL;              /* Most economic players */
char *assassinrank = NULL;              /* Most vicious players */
char *areligion = NULL;              /* Most religious players*/
char *preligion = NULL;              /* " " */
char *dreligion = NULL;              /* " "*/
char *creligion = NULL;              /* " "*/
char *rreligion = NULL;              /* " "*/
char *happytimes = NULL;        /* happy hour times from file */
char *gms        = NULL;        /* string for best GMs */

struct help_index_element *help_table = 0;	/* the help table	 */
int top_of_helpt = 0;		/* top of help index table	 */
int top_of_questions = 0;
struct question_index *question_table = 0;

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;	/* the infomation about the weather */
struct player_special_data dummy_mob;	/* dummy spec area for mobs	*/
struct reset_q_type reset_q;	/* queue of zones to be reset	 */

const int LAST_EXP_RESET = 1055095692;

/* local functions */
int check_object_spell_number(struct obj_data *obj, int val);
int check_object_level(struct obj_data *obj, int val);
void setup_dir(FILE * fl, int room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode, char *filename);
int check_object(struct obj_data *);
void parse_trigger(FILE *fl, int virtual_nr);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void load_questions();
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
int is_empty(zone_rnum zone_nr);
void reset_zone(zone_rnum zone);
int file_to_string(const char *name, char *buf);
int file_to_string_alloc(const char *name, char **buf);
void reboot_wizlists(void);
ACMD(do_reboot);
void boot_world(void);
int count_alias_records(FILE *fl);
int count_hash_records(FILE * fl);
bitvector_t asciiflag_conv(char *flag);
void parse_simple_mob(FILE *mob_f, int i, int nr);
void interpret_espec(const char *keyword, const char *value, int i, int nr);
void parse_espec(char *buf, int i, int nr);
void parse_enhanced_mob(FILE *mob_f, int i, int nr);
void get_one_line(FILE *fl, char *buf);
void save_etext(struct char_data * ch);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(zone_rnum zone, int cmd_no, const char *message);
void reset_time(void);
void genolc_strip_cr(char *buffer);
long get_ptable_by_name(char *name);

/* external functions */
struct time_info_data *mud_time_passed(time_t t2, time_t t1);
void free_alias(struct alias_data *a);
void load_messages(void);
void weather_and_time(int mode);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void);	/* In objsave.c */
void sort_commands(void);
void sort_spells(void);
void load_banned(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
int find_name(char *name);
int build_cmd_tree(void);
void prune_crlf(char *txt);
void sprintbits(long vektor, char *outstring);
void tag_argument(char *argument, char *tag);
void clean_pfiles(void);
void save_char_vars(struct char_data *ch);
void free_object_strings(struct obj_data *obj);
void free_object_strings_proto(struct obj_data *obj);
void sprintbits(long vektor, char *outstring);
int xap_objs = 1;               /* Xap objs      */
void read_songs();
struct question_index *find_question(char *keyword);

/* external vars */
extern int no_specials;
extern int scheck;
extern struct specproc_info mob_procs[];
extern struct specproc_info obj_procs[];
extern struct specproc_info room_procs[];
extern room_vnum mortal_start_room;
extern room_vnum immort_start_room;
extern room_vnum frozen_start_room;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[];
extern const char *unused_spellname;
extern bool QUESTON;

/* external ascii pfile vars */
extern int pfile_backup_minlevel;
extern int backup_wiped_pfiles;
extern struct pclean_criteria_data pclean_criteria[];
extern int selfdelete_fastwipe;
extern int auto_pwipe;            

#define READ_SIZE 256

/* make these TRUE if you want aliases and poofs saved in the pfiles
   or FALSE to leave them out
*/
#define ASCII_SAVE_POOFS       TRUE
#define ASCII_SAVE_ALIASES     TRUE

/* this is necessary for description handling.
   It was taken from GenOLC (OLC 2.0) */
void genolc_strip_cr(char *buffer)
{
  int rpos, wpos;

  if (buffer == NULL)
    return;

  for (rpos = 0, wpos = 0; buffer[rpos]; rpos++) {
    buffer[wpos] = buffer[rpos];
    wpos += (buffer[rpos] != '\r');
  }
  buffer[wpos] = '\0';
}
   
/*************************************************************************
*  routines for booting the system                                       *
*************************************************************************/

/* this is necessary for the autowiz system */
void reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}


/*
 * Too bad it doesn't check the return values to let the user
 * know about -1 values.  This will result in an 'Okay.' to a
 * 'reload' command even when the string was not replaced.
 * To fix later, if desired. -gg 6/24/99
 */
ACMD(do_reboot)
{
  int i;

  one_argument(argument, arg);

  if (!str_cmp(arg, "all") || *arg == '*') {
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(BUILDERCODE_FILE, &buildercode);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(SMARTMUD_FILE, &smartmud);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
    file_to_string_alloc(SPAR_FILE, &sparrank);
    file_to_string_alloc(ECON_FILE, &econrank);
    file_to_string_alloc(ASSASSIN_FILE, &assassinrank);
    file_to_string_alloc(ARELIGION_FILE, &areligion);
    file_to_string_alloc(PRELIGION_FILE, &preligion);
    file_to_string_alloc(DRELIGION_FILE, &dreligion);
    file_to_string_alloc(RRELIGION_FILE, &rreligion);
    file_to_string_alloc(CRELIGION_FILE, &creligion);
    file_to_string_alloc(HAPPY_FILE, &happytimes);
    file_to_string_alloc(GM_FILE, &gms);
    load_messages();
  } else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd"))
    file_to_string_alloc(MOTD_FILE, &motd);
  else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "smartmud"))
    file_to_string_alloc(SMARTMUD_FILE, &smartmud);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "buildercode"))
    file_to_string_alloc(BUILDERCODE_FILE, &buildercode);
  else if (!str_cmp(arg, "sparrank"))
    file_to_string_alloc(SPAR_FILE, &sparrank);
  else if (!str_cmp(arg, "assassinrank"))
    file_to_string_alloc(ASSASSIN_FILE, &assassinrank);
  else if (!str_cmp(arg, "econrank"))
    file_to_string_alloc(ECON_FILE, &econrank);
  else if (!str_cmp(arg, "religion"))
  {
    file_to_string_alloc(ARELIGION_FILE, &areligion);
    file_to_string_alloc(PRELIGION_FILE, &preligion);
    file_to_string_alloc(DRELIGION_FILE, &dreligion);
    file_to_string_alloc(RRELIGION_FILE, &rreligion);
    file_to_string_alloc(CRELIGION_FILE, &creligion);
  }
  else if (!str_cmp(arg, "happytimes"))
    file_to_string_alloc(HAPPY_FILE, &happytimes);
  else if (!str_cmp(arg, "gms"))
    file_to_string_alloc(GM_FILE, &gms);
  else if (!str_cmp(arg, "messages"))
  {
    load_messages();
  }
  else if (!str_cmp(arg, "xhelp")) {
    if (help_table) {
      for (i = 0; i <= top_of_helpt; i++) {
        if (help_table[i].keywords)
	  free(help_table[i].keywords);
        if (help_table[i].entry)
	  free(help_table[i].entry);
      }
      free(help_table);
    }
    top_of_helpt = 0;
    if (question_table) {
      for (i = 0; i <= top_of_questions; i++) {
        if (question_table[i].keyword)
	  free(question_table[i].keyword);
        if (question_table[i].entry)
	  free(question_table[i].entry);
      }
      free(question_table);
    }
    top_of_questions = 0;
    index_boot(DB_BOOT_HLP);
  } else {
    send_to_char("Unknown reload option.\r\n", ch);
    return;
  }

  send_to_char(OK, ch);
}


void boot_world(void)
{
  log("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  log("Loading triggers and generating index.");
  index_boot(DB_BOOT_TRG);
  
  log("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  log("Renumbering rooms.");
  renum_world();

  log("Checking start rooms.");
  check_start_rooms();

  log("Loading mobs and generating index.");
  index_boot(DB_BOOT_MOB);

  log("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  log("Renumbering zone table.");
  renum_zone_table();

  if (!no_specials) {
    log("Loading shops.");
    index_boot(DB_BOOT_SHP);
  }
}

  

/* body of the booting system */
void boot_db(void)
{
  FBFILE *fl;
  zone_rnum i;

  log("Resetting the game time:");
  reset_time();

#ifdef CMD_TREE_SEARCH
  log("Building command search tree:");
  log("  %d commands. Done.", build_cmd_tree());
#endif 

  log("Reading news, buildercode, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(BUILDERCODE_FILE, &buildercode);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);
  file_to_string_alloc(SMARTMUD_FILE, &smartmud);
  file_to_string_alloc(SPAR_FILE, &sparrank);
  file_to_string_alloc(ECON_FILE, &econrank);
  file_to_string_alloc(ASSASSIN_FILE, &assassinrank);
  file_to_string_alloc(ARELIGION_FILE, &areligion);
  file_to_string_alloc(PRELIGION_FILE, &preligion);
  file_to_string_alloc(DRELIGION_FILE, &dreligion);
  file_to_string_alloc(RRELIGION_FILE, &rreligion);
  file_to_string_alloc(CRELIGION_FILE, &creligion);
  file_to_string_alloc(HAPPY_FILE, &happytimes);
  file_to_string_alloc(GM_FILE, &gms);

  log("Loading spell definitions.");
  mag_assign_spells();

  log("Reading Songs.");
  /*read_songs();*/

  boot_world();

  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  log("Generating player index.");
  build_player_index();

  if(auto_pwipe) {
    log("Cleaning out the pfiles.");
    clean_pfiles();
  }
      
  log("Loading fight messages.");
  load_messages();

  log("Loading social messages.");
  boot_social_messages();

  log("Loading clans.");
  load_clans();

  log("Assigning function pointers:");

  if (!no_specials) {
    log("   Mobiles.");
    assign_mobiles();
    log("   Shopkeepers.");
    assign_the_shopkeepers();
    log("   Objects.");
    assign_objects();
    log("   Rooms.");
    assign_rooms();
  }

  log("Booting assembled objects.");
  assemblyBootAssemblies();

  log("Assigning spell and skill levels.");
  init_spell_levels();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting mail system.");
  if (!scan_file()) {
    log("    Mail boot failed -- Mail system disabled");
    no_mail = 1;
  }
  log("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  /* Moved here so the object limit code works. -gg 6/24/98 */
  if (!mini_mud) {
    log("Booting houses.");
    House_boot();
  }

  for (i = 0; i <= top_of_zone_table; i++) 
		{
			//log("Resetting %s (rooms %d-%d).", zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0), zone_table[i].top);
			reset_zone(i);
		}
	
  reset_q.head = reset_q.tail = NULL;

  boot_time = time(0);

  log("Booting Econreset.");
  if (!(fl = fbopen("../lib/etc/econresetsave", FB_READ))) 
  {
      log("No Econ Reset Found");
      LAST_ECON_RESET = time(0);
  }
  else
  {
      log("Econ Reset Logged");
      fbgetline(fl, buf);
      LAST_ECON_RESET = atoi(buf);
      fbclose(fl);
  }
  log("Booting MinutesPlayed");
  if (!(fl = fbopen("../lib/etc/minutesplayed", FB_READ))) 
  {
      log("No minutes have been played.");
      minutesplayed= 0;
  }
  else
  {
      log("minutesplayed logged");
      fbgetline(fl, buf);
      LAST_ECON_RESET = atoi(buf);
      fbclose(fl);
  }
  
  log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
#if defined(CIRCLE_MACINTOSH)
  long beginning_of_time = -1561789232;
#else
  long beginning_of_time = 650336715;
#endif

  time_info = *mud_time_passed(time(0), beginning_of_time);

  if (time_info.hours <= 4)
    weather_info.sunlight = SUN_DARK;
  else if (time_info.hours == 5)
    weather_info.sunlight = SUN_RISE;
  else if (time_info.hours <= 20)
    weather_info.sunlight = SUN_LIGHT;
  else if (time_info.hours == 21)
    weather_info.sunlight = SUN_SET;
  else
    weather_info.sunlight = SUN_DARK;

  log("   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
	  time_info.day, time_info.month, time_info.year);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}

bitvector_t asciiflag_conv(char *flag)
{
  bitvector_t flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
      flags |= 1 << (26 + (*p - 'A'));

    if (!isdigit(*p))
      is_number = 0;
  }

  if (is_number)
    flags = atol(flag);

  return (flags);
}


/* new version to build the player index for the ascii pfiles */      

/* generate index table for the player file */
void build_player_index(void)
{
  int rec_count = 0, i;
  FBFILE *plr_index;
  char index_name[40], line[256], bits[64];
  char arg2[80];
	int last_time = 0;


  sprintf(index_name, "%s", PLR_INDEX_FILE);
  if(!(plr_index = fbopen(index_name, FB_READ))) {
    top_of_p_table = -1;
    log("No player index file!  First new char will be IMP!");
    return;                                                       
  }

   /* count the number of players in the index */
  while(fbgetline(plr_index, line))
    if(*line != '~')
      rec_count++;
  fbrewind(plr_index);

  if(rec_count == 0) {  
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  CREATE(player_table, struct player_index_element, rec_count);
  for(i = 0; i < rec_count; i++) {
    fbgetline(plr_index, line);
    sscanf(line, "%ld %s %d %d %d %d %d %s %d %d", 
					 &player_table[i].id, 
					 arg2, 
					 &player_table[i].level, 
					 &player_table[i].levelthief, 
					 &player_table[i].levelwarrior, 
					 &player_table[i].levelmage, 
					 &player_table[i].levelcleric,
					 bits, 
					 &last_time,
					 &player_table[i].autowiz);   

		player_table[i].last = (time_t)last_time;

    CREATE(player_table[i].name, char, strlen(arg2) + 1);
    strcpy(player_table[i].name, arg2);
    player_table[i].flags = asciiflag_conv(bits);
    top_idnum = MAX(top_idnum, player_table[i].id);
  }
  fbclose(plr_index);
  top_of_p_file = top_of_p_table = i - 1;
 }                                         

/*
 * Thanks to Andrey (andrey@alex-ua.com) for this bit of code, although I
 * did add the 'goto' and changed some "while()" into "do { } while()".
 *	-gg 6/24/98 (technically 6/25/98, but I care not.)
 */
int count_alias_records(FILE *fl)
{
  char key[READ_SIZE], next_key[READ_SIZE];
  char line[READ_SIZE], *scan;
  int total_keywords = 0;

  /* get the first keyword line */
  get_one_line(fl, key);

  while (*key != '$') {
    /* skip the text */
    do {
      get_one_line(fl, line);
      if (feof(fl))
	goto ackeof;
    } while (*line != '#');

    /* now count keywords */
    scan = key;
    do {
      scan = one_word(scan, next_key);
      if (*next_key)
        ++total_keywords;
    } while (*next_key);

    /* get next keyword line (or $) */
    get_one_line(fl, key);

    if (feof(fl))
      goto ackeof;
  }

  return (total_keywords);

  /* No, they are not evil. -gg 6/24/98 */
ackeof:	
  log("SYSERR: Unexpected end of help file.");
  exit(1);	/* Some day we hope to handle these things better... */
}

/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE * fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return (count);
}



void index_boot(int mode)
{
  const char *index_filename, *prefix = NULL;	/* NULL or egcs 1.1 complains */
  FILE *index, *db_file;
  int rec_count = 0, size[2];

  switch (mode) {
  case DB_BOOT_TRG:
    prefix = TRG_PREFIX;
    break;
  case DB_BOOT_WLD:
    prefix = WLD_PREFIX;
    break;
  case DB_BOOT_MOB:
    prefix = MOB_PREFIX;
    break;
  case DB_BOOT_OBJ:
    prefix = OBJ_PREFIX;
    break;
  case DB_BOOT_ZON:
    prefix = ZON_PREFIX;
    break;
  case DB_BOOT_SHP:
    prefix = SHP_PREFIX;
    break;
  case DB_BOOT_HLP:
    prefix = HLP_PREFIX;
    break;
  default:
    log("SYSERR: Unknown subcommand %d to index_boot!", mode);
    exit(1);
  }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r"))) {
    log("SYSERR: opening index file '%s': %s", buf2, strerror(errno));
    exit(1);
  }

  /* first, count the number of records in the file so we can malloc */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      log("SYSERR: File '%s' listed in '%s/%s': %s", buf2, prefix,
	  index_filename, strerror(errno));
      fscanf(index, "%s\n", buf1);
      continue;
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else if (mode == DB_BOOT_HLP)
	rec_count += count_alias_records(db_file);
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* Exit if 0 records, unless this is shops */
  if (!rec_count) {
    if (mode == DB_BOOT_SHP)
      return;
    log("SYSERR: boot error - 0 records counted in %s/%s.", prefix,
	index_filename);
    exit(1);
  }

  /* Any idea why you put this here Jeremy? */
  rec_count++;

  /*
   * NOTE: "bytes" does _not_ include strings or other later malloc'd things.
   */
  switch (mode) {
  case DB_BOOT_TRG:
    CREATE(trig_index, struct index_data *, rec_count);
    break;
  case DB_BOOT_WLD:
    CREATE(world, struct room_data, rec_count);
    size[0] = sizeof(struct room_data) * rec_count;
    log("   %d rooms, %d bytes.", rec_count, size[0]);
    break;
  case DB_BOOT_MOB:
    CREATE(mob_proto, struct char_data, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    size[0] = sizeof(struct index_data) * rec_count;
    size[1] = sizeof(struct char_data) * rec_count;
    log("   %d mobs, %d bytes in index, %d bytes in prototypes.", rec_count, size[0], size[1]);
    break;
  case DB_BOOT_OBJ:
    CREATE(obj_proto, struct obj_data, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    size[0] = sizeof(struct index_data) * rec_count;
    size[1] = sizeof(struct obj_data) * rec_count;
    log("   %d objs, %d bytes in index, %d bytes in prototypes.", rec_count, size[0], size[1]);
    break;
  case DB_BOOT_ZON:
    CREATE(zone_table, struct zone_data, rec_count);
    size[0] = sizeof(struct zone_data) * rec_count;
    log("   %d zones, %d bytes.", rec_count, size[0]);
    break;
  case DB_BOOT_HLP:
    CREATE(help_table, struct help_index_element, rec_count);
    size[0] = sizeof(struct help_index_element) * rec_count;
    log("   %d entries, %d bytes.", rec_count, size[0]);
    CREATE(question_table, struct question_index, rec_count);
    size[0] = sizeof(struct question_index) * rec_count;
    log("   %d entries, %d bytes.", rec_count, size[0]);
    break;
  }

  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      log("SYSERR: %s: %s", buf2, strerror(errno));
      exit(1);
    }
    switch (mode) {
    case DB_BOOT_TRG:
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
      discrete_load(db_file, mode, buf2);
      break;
    case DB_BOOT_ZON:
      load_zones(db_file, buf2);
      break;
    case DB_BOOT_HLP:
      /*
       * If you think about it, we have a race here.  Although, this is the
       * "point-the-gun-at-your-own-foot" type of race.
       */
      load_help(db_file);
      load_questions();
      break;
    case DB_BOOT_SHP:
      boot_the_shops(db_file, buf2, rec_count);
      break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }
  fclose(index);

}


void discrete_load(FILE * fl, int mode, char *filename)
{
  int nr = -1, last;
  char line[256];

  const char *modes[] = {"world", "mob", "obj", "ZON", "SHP", "HLP", "trg"};

  for (;;) {
    /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
    if (mode != DB_BOOT_OBJ || nr < 0)
      if (!get_line(fl, line)) {
	if (nr == -1) {
	  log("SYSERR: %s file %s is empty!", modes[mode], filename);
	} else {
	  log("SYSERR: Format error in %s after %s #%d\n"
	      "...expecting a new %s, but file ended!\n"
	      "(maybe the file is not terminated with '$'?)", filename,
	      modes[mode], nr, modes[mode]);
	}
	exit(1);
      }
    if (*line == '$')
      return;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1) {
	log("SYSERR: Format error after %s #%d", modes[mode], last);
	exit(1);
      }
      if (nr >= 99999)
	return;
      else
	switch (mode) {
        case DB_BOOT_TRG:
          parse_trigger(fl, nr);
          break;
	case DB_BOOT_WLD:
	  parse_room(fl, nr);
	  break;
	case DB_BOOT_MOB:
	  parse_mobile(fl, nr);
	  break;
	case DB_BOOT_OBJ:
	  strcpy(line, parse_object(fl, nr));
	  break;
	}
    } else {
      log("SYSERR: Format error in %s file %s near %s #%d", modes[mode],
	  filename, modes[mode], nr);
      log("SYSERR: ... offending line: '%s'", line);
      exit(1);
    }
  }
}

char fread_letter(FILE *fp)
{
  char c;
  do {
    c = getc(fp);  
  } while (isspace(c));
  return c;
}

/* load the rooms */
void parse_room(FILE * fl, int virtual_nr)
{
  static int room_nr = 0, zone = 0;
  int t[10], i;
  char line[256], flags[128], sp_buf[64];    
  struct extra_descr_data *new_descr;
  char letter;


  sprintf(buf2, "room #%d", virtual_nr);

  if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1)) {
    log("SYSERR: Room #%d is below zone %d.", virtual_nr, zone);
    exit(1);
  }
  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table) {
      log("SYSERR: Room %d is outside of any zone.", virtual_nr);
      exit(1);
    }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    log("SYSERR: Expecting roomflags/sector type of room #%d but file ended!",
	virtual_nr);
    exit(1);
  }

  if (sscanf(line, " %d %s %d ", t, flags, t + 2) != 3) {
    log("SYSERR: Format error in roomflags/sector type of room #%d",
	virtual_nr);
    exit(1);
  }
  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags = asciiflag_conv(flags);
  world[room_nr].sector_type = t[2];

  world[room_nr].func = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people = NULL;
  world[room_nr].light = 0;	/* Zero light sources */

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf,"SYSERR: Format error in room #%d (expecting D/E/S/T)",virtual_nr);

  for (;;) {
    if (!get_line(fl, line)) {
      log(buf);
      exit(1);
    }
    switch (*line) {
    case 'D':
      setup_dir(fl, room_nr, atoi(line + 1));
      break;
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(fl, buf2);
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
    case 'P':  /* Do room spec procs */
      if (!get_line(fl, line) || sscanf(line, " %s", sp_buf) != 1) {
        fprintf(stderr, "Format error in room #%d's section P\n", virtual_nr);
        exit(1);
      }
      t[0] = get_spec_proc(room_procs, sp_buf);
      if (t[0] == 0) {
        sprintf(buf,"SYSWARR: Attempt to assign non-existing room spec-proc (Room: %d)", virtual_nr);
        log(buf);
      } else
        world[room_nr].func = room_procs[t[0]].sp_pointer;
      break;

    case 'S':			/* end of room */
      /* DG triggers -- script is defined after the end of the room */
      letter = fread_letter(fl);
      ungetc(letter, fl);
      while (letter=='T') {
        dg_read_trigger(fl, &world[room_nr], WLD_TRIGGER);
        letter = fread_letter(fl);
        ungetc(letter, fl);
      }
      top_of_world = room_nr++;
      return;

    default:
      log(buf);
      exit(1);
    }
  }
}



/* read direction data */
void setup_dir(FILE * fl, int room, int dir)
{
  int t[5];
  char line[256];

  sprintf(buf2, "room #%d, direction D%d", GET_ROOM_VNUM(room), dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    log("SYSERR: Format error, %s", buf2);
    exit(1);
  }
  if (sscanf(line, " %d %d %d ", t, t + 1, t + 2) != 3) {
    log("SYSERR: Format error, %s", buf2);
    exit(1);
  }
  if (t[0] == 1)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if (t[0] == 2)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->key = t[1];
  world[room].dir_option[dir]->to_room = t[2];
}


/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
  if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
    log("SYSERR:  Mortal start room does not exist.  Change in config.c.");
    exit(1);
  }
  if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
    r_immort_start_room = r_mortal_start_room;
  }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
    r_frozen_start_room = r_mortal_start_room;
  }
}


/* resolve all vnums into rnums in the world */
void renum_world(void)
{
  register int room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
  int cmd_no, a, b, c, olda, oldb, oldc;
  zone_rnum zone;
  char buf[128];

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = c = 0;
      olda = ZCMD.arg1;
      oldb = ZCMD.arg2;
      oldc = ZCMD.arg3;
      switch (ZCMD.command) {
      case 'M':
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	c = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'O':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (ZCMD.arg3 != NOWHERE)
	  c = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'G':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'E':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'P':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	c = ZCMD.arg3 = real_object(ZCMD.arg3);
	break;
      case 'D':
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	break;
      case 'R': /* rem obj from room */
        a = ZCMD.arg1 = real_room(ZCMD.arg1);
	b = ZCMD.arg2 = real_object(ZCMD.arg2);
        break;
      case 'T': /* a trigger */
        /* designer's choice: convert this later */
        /* b = ZCMD.arg2 = real_trigger(ZCMD.arg2); */
        b = real_trigger(ZCMD.arg2); /* leave this in for validation */
        break;
      case 'V': /* trigger variable assignment */
        if (ZCMD.arg1 == WLD_TRIGGER)
          b = ZCMD.arg2 = real_room(ZCMD.arg2);
        break;
      }
      if (a < 0 || b < 0 || c < 0) {
	if (!mini_mud) {
	  sprintf(buf,  "Invalid vnum %d, cmd disabled",
			 (a < 0) ? olda : ((b < 0) ? oldb : oldc));
	  log_zone_error(zone, cmd_no, buf);
	}
	ZCMD.command = '*';
      }
    }
}



void parse_simple_mob(FILE *mob_f, int i, int nr)
{
  int j, t[10];
  char line[256];

  mob_proto[i].real_abils.str = 11;
  mob_proto[i].real_abils.intel = 11;
  mob_proto[i].real_abils.wis = 11;
  mob_proto[i].real_abils.dex = 11;
  mob_proto[i].real_abils.con = 11;
  mob_proto[i].real_abils.cha = 11;

  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error in mob #%d, file ended after S flag!", nr);
    exit(1);
  }

  if (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
	  t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9) {
    log("SYSERR: Format error in mob #%d, first line after S flag\n"
	"...expecting line of form '# # # #d#+# #d#+#'", nr);
    exit(1);
  }

  GET_LEVEL(mob_proto + i) = t[0];
  mob_proto[i].points.hitroll = 20 - t[1];
  mob_proto[i].points.armor = 10 * t[2];

  /* max hit = 0 is a flag that H, M, V is xdy+z */
  mob_proto[i].points.max_hit = 0;
  mob_proto[i].points.hit = t[3];
  mob_proto[i].points.mana = t[4];
  mob_proto[i].points.move = t[5];

  mob_proto[i].points.max_mana = 10;
  mob_proto[i].points.max_move = 50;

  mob_proto[i].mob_specials.damnodice = t[6];
  mob_proto[i].mob_specials.damsizedice = t[7];
  mob_proto[i].points.damroll = t[8];

  if (!get_line(mob_f, line)) {
      log("SYSERR: Format error in mob #%d, second line after S flag\n"
	  "...expecting line of form '# #', but file ended!", nr);
      exit(1);
    }

  if (sscanf(line, " %d %d ", t, t + 1) != 2) {
    log("SYSERR: Format error in mob #%d, second line after S flag\n"
	"...expecting line of form '# #'", nr);
    exit(1);
  }

  GET_GOLD(mob_proto + i) = t[0];
  GET_EXP(mob_proto + i) = t[1];

  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error in last line of mob #%d\n"
	"...expecting line of form '# # #', but file ended!", nr);
    exit(1);
  }

  if (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) != 3) {
    log("SYSERR: Format error in last line of mob #%d\n"
	"...expecting line of form '# # #'", nr);
    exit(1);
  }

  mob_proto[i].char_specials.position = t[0];
  mob_proto[i].mob_specials.default_pos = t[1];
  mob_proto[i].player.sex = t[2];

  mob_proto[i].player.chclass = 0;
  mob_proto[i].player.weight = 200;
  mob_proto[i].player.height = 198;

  /*
   * these are now save applies; base save numbers for MOBs are now from
   * the warrior save table.
   */
  for (j = 0; j < 5; j++)
    GET_SAVE(mob_proto + i, j) = 0;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void interpret_espec(const char *keyword, const char *value, int i, int nr)
{
  int num_arg, matched = 0;

  num_arg = atoi(value);

  CASE("BareHandAttack") {
    RANGE(0, 99);
    mob_proto[i].mob_specials.attack_type = num_arg;
  }

  CASE("Str") {
    RANGE(3, 25);
    mob_proto[i].real_abils.str = num_arg;
  }

  CASE("StrAdd") {
    RANGE(0, 100);
    mob_proto[i].real_abils.str_add = num_arg;    
  }

  CASE("Int") {
    RANGE(3, 25);
    mob_proto[i].real_abils.intel = num_arg;
  }

  CASE("Wis") {
    RANGE(3, 25);
    mob_proto[i].real_abils.wis = num_arg;
  }

  CASE("Dex") {
    RANGE(3, 25);
    mob_proto[i].real_abils.dex = num_arg;
  }

  CASE("Con") {
    RANGE(3, 25);
    mob_proto[i].real_abils.con = num_arg;
  }

  CASE("Cha") {
    RANGE(3, 25);
    mob_proto[i].real_abils.cha = num_arg;
  }
  CASE("SpecProc") {
    num_arg = get_spec_proc(mob_procs, value);
    if (num_arg == 0) {
      log("ESPEC ERROR: Attempt to assign non-existing spec-proc.");
    } else {
      mob_index[i].func = mob_procs[num_arg].sp_pointer;
    }
  }

  if (!matched) {
    log("SYSERR: Warning: unrecognized espec keyword %s in mob #%d",
	    keyword, nr);
  }    
}

#undef CASE
#undef RANGE

void parse_espec(char *buf, int i, int nr)
{
  char *ptr;

  if ((ptr = strchr(buf, ':')) != NULL) {
    *(ptr++) = '\0';
    while (isspace(*ptr))
      ptr++;
#if 0	/* Need to evaluate interpret_espec()'s NULL handling. */
  }
#else
  } else
    ptr = "";
#endif
  interpret_espec(buf, ptr, i, nr);
}


void parse_enhanced_mob(FILE *mob_f, int i, int nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line)) {
    if (!strcmp(line, "E"))	/* end of the enhanced section */
      return;
    else if (*line == '#') {	/* we've hit the next mob, maybe? */
      log("SYSERR: Unterminated E section in mob #%d", nr);
      exit(1);
    } else
      parse_espec(line, i, nr);
  }

  log("SYSERR: Unexpected end of file reached after mob #%d", nr);
  exit(1);
}


void parse_mobile(FILE * mob_f, int nr)
{
  static int i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128];

  mob_index[i].vnum = nr;
  mob_index[i].number = 0;
  mob_index[i].func = NULL;

  clear_char(mob_proto + i);

  /*
   * Mobiles should NEVER use anything in the 'player_specials' structure.
   * The only reason we have every mob in the game share this copy of the
   * structure is to save newbie coders from themselves. -gg 2/25/98
   */
  mob_proto[i].player_specials = &dummy_mob;
  sprintf(buf2, "mob vnum %d", nr);

  /***** String data *****/
  mob_proto[i].player.name = fread_string(mob_f, buf2);
  tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);
  mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
  mob_proto[i].player.description = fread_string(mob_f, buf2);
  mob_proto[i].player.title = NULL;

  /* *** Numeric data *** */
  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error after string section of mob #%d\n"
	"...expecting line of form '# # # {S | E}', but file ended!", nr);
    exit(1);
  }

#ifdef CIRCLE_ACORN	/* Ugh. */
  if (sscanf(line, "%s %s %d %s", f1, f2, t + 2, &letter) != 4) {
#else
  if (sscanf(line, "%s %s %d %c", f1, f2, t + 2, &letter) != 4) {
#endif
    log("SYSERR: Format error after string section of mob #%d\n"
	"...expecting line of form '# # # {S | E}'", nr);
    exit(1);
  }
  MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
  SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
  GET_ALIGNMENT(mob_proto + i) = t[2];

  switch (UPPER(letter)) {
  case 'S':	/* Simple monsters */
    parse_simple_mob(mob_f, i, nr);
    break;
  case 'E':	/* Circle3 Enhanced monsters */
    parse_enhanced_mob(mob_f, i, nr);
    break;
  /* add new mob types here.. */
  default:
    log("SYSERR: Unsupported mob type '%c' in mob #%d", letter, nr);
    exit(1);
  }

  /* DG triggers -- script info follows mob S/E section */
  letter = fread_letter(mob_f);
  ungetc(letter, mob_f);
  while (letter=='T') {
    dg_read_trigger(mob_f, &mob_proto[i], MOB_TRIGGER);
    letter = fread_letter(mob_f);
    ungetc(letter, mob_f);
  }
  
  mob_proto[i].aff_abils = mob_proto[i].real_abils;

  for (j = 0; j < NUM_WEARS; j++)
    mob_proto[i].equipment[j] = NULL;

  mob_proto[i].nr = i;
  mob_proto[i].desc = NULL;

  top_of_mobt = i++;
}




/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, int nr)
{
  static int i = 0;
  static char line[256], sp_buf[128];
  int t[10], j, retval;
  char *tmpptr;
  char f1[256], f2[256];
  struct extra_descr_data *new_descr;

  obj_index[i].vnum = nr;
  obj_index[i].number = 0;
  obj_index[i].func = NULL;

  clear_object(obj_proto + i);
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%d", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
    log("SYSERR: Null obj name or format error at or near %s", buf2);
    exit(1);
  }
  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    CAP(tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting first numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, " %d %s %s %d", t, f1, f2, t + 3)) != 4) {
    if (retval == 3)
      t[3] = 0;
    else {
      log("SYSERR: Format error in first numeric line (expecting 4 args, got %d), %s", retval, buf2);
      exit(1);
    }
  }
  obj_proto[i].obj_flags.type_flag = t[0];
  obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
  obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f2);
  obj_proto[i].obj_flags.bitvector = t[3];

  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting second numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
    log("SYSERR: Format error in second numeric line (expecting 4 args, got %d), %s", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];

  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting third numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
    if (retval == 3)
      t[3] = 0;
    else {
      log("SYSERR: Format error in third numeric line (expecting 4 args, got %d), %s", retval, buf2);
      exit(1);
    }
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost = t[1];
  obj_proto[i].obj_flags.cost_per_day = t[2];
  obj_proto[i].obj_flags.level = t[3];

  /* check to make sure that weight of containers exceeds curr. quantity */
  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
      obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN) {
    if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
      obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
  }

  /* *** extra descriptions and affect fields *** */

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    obj_proto[i].affected[j].location = APPLY_NONE;
    obj_proto[i].affected[j].modifier = 0;
  }

  strcat(buf2, ", after numeric constants\n"
	 "...expecting 'E', 'A', '$', or next object number");
  j = 0;

  for (;;) {
    if (!get_line(obj_f, line)) {
      log("SYSERR: Format error in %s", buf2);
      exit(1);
    }
    switch (*line) {
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(obj_f, buf2);
      new_descr->description = fread_string(obj_f, buf2);
      new_descr->next = obj_proto[i].ex_description;
      obj_proto[i].ex_description = new_descr;
      break;
    case 'A':
      if (j >= MAX_OBJ_AFFECT) {
	log("SYSERR: Too many A fields (%d max), %s", MAX_OBJ_AFFECT, buf2);
	exit(1);
      }
      if (!get_line(obj_f, line)) {
	log("SYSERR: Format error in 'A' field, %s\n"
	    "...expecting 2 numeric constants but file ended!", buf2);
	exit(1);
      }

      if ((retval = sscanf(line, " %d %d ", t, t + 1)) != 2) {
	log("SYSERR: Format error in 'A' field, %s\n"
	    "...expecting 2 numeric arguments, got %d\n"
	    "...offending line: '%s'", buf2, retval, line);
	exit(1);
      }
      obj_proto[i].affected[j].location = t[0];
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;
    case 'S':
      get_line(obj_f, line);
      sscanf(line, "%s ", sp_buf);
      t[0] = get_spec_proc(obj_procs, sp_buf);
      obj_index[i].func = obj_procs[t[0]].sp_pointer;
      if (t[0] == 0)
        log("SYSWARR: Attempt to assign nonexisting obj spec-proc");
      break;
    case 'T':  /* DG triggers */
      dg_obj_trigger(line, &obj_proto[i]);
      break;
    case '$':
    case '#':
      check_object(&obj_proto[i]);
      top_of_objt = i++;
      return (line);
    default:
      log("SYSERR: Format error in %s", buf2);
      exit(1);
    }
  }
}


#define Z	zone_table[zone]

/* load the zone table and command tables */
void load_zones(FILE * fl, char *zonename)
{
  static zone_rnum zone = 0;
  int cmd_no, num_of_cmds = 0, line_num = 0, tmp, error, arg_num;
  char *ptr, buf[256], zname[256];
  char t1[80], t2[80];

  strcpy(zname, zonename);

  while (get_line(fl, buf))
    num_of_cmds++;		/* this should be correct within 3 or so */
  rewind(fl);

  if (num_of_cmds == 0) {
    log("SYSERR: %s is empty!", zname);
    exit(1);
  } else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%hd", &Z.number) != 1) {
    log("SYSERR: Format error in %s, line %d", zname, line_num);
    exit(1);
  }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
    *ptr = '\0';
  Z.builders = str_dup(buf);

  line_num += get_line(fl, buf);  
  if (sscanf(buf, " %hd %d %d ", &Z.top, &Z.lifespan, &Z.reset_mode) != 3) {
    log("SYSERR: Format error in 3-constant line of %s", zname);
    exit(1);
  }
  cmd_no = 0;

  for (;;) {
    if ((tmp = get_line(fl, buf)) == 0) {
      log("SYSERR: Format error in %s - premature end of file", zname);
      exit(1);
    }
    line_num += tmp;
    ptr = buf;
    skip_spaces(&ptr);

    if ((ZCMD.command = *ptr) == '*')
      continue;

    ptr++;

    if (ZCMD.command == 'S' || ZCMD.command == '$') {
      ZCMD.command = 'S';
      break;
    }
    error = 0;

     if (strchr("D", ZCMD.command) != NULL) { /* ### */
        if (sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2,
                  &ZCMD.arg3) != 4)
         error = 1;
     }
     else if (strchr("R", ZCMD.command) != NULL) { /* ### */
       if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1,
            &ZCMD.arg2) != 3)
         error = 1;
     }
     else if (strchr("G", ZCMD.command) != NULL) { /* ### */
       if ((arg_num = sscanf(ptr, " %d %d %d %d %d", &tmp, &ZCMD.arg1,
            &ZCMD.arg2, &ZCMD.arg3, &ZCMD.arg4)) != 5) {
         if (arg_num != 4)
           error = 1;
         else {
           ZCMD.arg3 = 0;
          ZCMD.arg4 = 0;
         }
       }
     }
     else if (ZCMD.command=='V') { /* a string-arg command */
      if (sscanf(ptr, " %d %d %d %d %s %s", &tmp, &ZCMD.arg1, &ZCMD.arg2,
		 &ZCMD.arg3, t1, t2) != 6)
	error = 1;
      else {
        ZCMD.sarg1 = str_dup(t1);
        ZCMD.sarg2 = str_dup(t2);
      }
    } 
   else {
      if ((arg_num = sscanf(ptr, " %d %d %d %d %d ", &tmp, &ZCMD.arg1,
	   &ZCMD.arg2, &ZCMD.arg3, &ZCMD.arg4)) != 5){
        if (arg_num != 4)
          error = 1;
        else
          ZCMD.arg4 = 0;
        }
    }
    ZCMD.if_flag = tmp;

    if (error) {
      log("SYSERR: Format error in %s, line %d: '%s'", zname, line_num, buf);
      exit(1);
    }
    ZCMD.line = line_num;
    cmd_no++;
  }

  top_of_zone_table = zone++;
}

#undef Z


void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    log("SYSERR: error reading help file: not terminated with $?");
    exit(1);
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

void load_help(FILE *fl)
{
  char key[READ_SIZE+1], entry[32384];
  char line[READ_SIZE+1], *write;
  struct help_index_element el;

  /* get the keyword line */
  get_one_line(fl, key);
  while (*key != '$') {
    get_one_line(fl, line);
    *entry = '\0';
    while (*line != '#') 
    {
      if ((write = strchr(line, '|')) != NULL)
      {
        *write = ' ';
        break;
      }
      strcat(entry, strcat(line, "\r\n"));
      get_one_line(fl, line);
    }
    while (*line != '#')
       get_one_line(fl, line);
   
    el.min_level = 0;
    if ((*line == '#') && (*(line + 1) != 0))
      el.min_level = atoi((line + 1));

    el.min_level = MAX(0, MIN(el.min_level, LVL_IMPL));

    /* now, add the entry to the index with each keyword on the keyword line */
    el.entry = str_dup(entry);
    el.keywords = str_dup(key);

    help_table[top_of_helpt] = el;
    top_of_helpt++;

    /* get next keyword line (or $) */
    get_one_line(fl, key);
  }
}

void load_questions()
{
  char key[READ_SIZE+1], entry[32384];
  struct question_index el, *tel;
  char filename[32384];
  char *this_name;
  FBFILE *fi1, *fi2;


  if (!(fi1 = fbopen("../lib/text/help/NewHelp/index", FB_READ)))
  {
      log("Error: Cannot Open Question Help Files.");
      return;
  }
  else
  {

      while(fbgetline(fi1,buf))
      {
	  *entry = '\0';
          sprintf(key,"%s", buf);
          sprintf(filename, "../lib/text/help/NewHelp/%s", buf);
          if (!(fi2 = fbopen(filename, FB_READ)))
          {
            sprintf(entry, "File not Found");
          }
          else
          {
            while(fbgetline(fi2, buf))
               sprintf(entry, "%s\r\n%s",entry, buf);
            fbclose(fi2);
          }
          if ((tel = find_question(key))) continue;
          el.entry = str_dup(entry);
          el.keyword = str_dup(key);
          for (this_name = el.keyword; *this_name; this_name++)
             CAP(this_name);
          el.modified = FALSE;
          question_table[top_of_questions] = el;
          top_of_questions++;           
      }
  }
  fbclose(fi1);
}

/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*************************************************************************/



int vnum_mobile(char *searchname, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_mobt; nr++) {
    if (isname(searchname, mob_proto[nr].player.name)) {
      sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
	      mob_index[nr].vnum,
	      mob_proto[nr].player.short_descr);
      send_to_char(buf, ch);
    }
  }

  return (found);
}


int vnum_weapon(int attacktype, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (obj_proto[nr].obj_flags.type_flag == ITEM_WEAPON && obj_proto[nr].obj_flags.value[3] == attacktype) {
      sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
	      obj_index[nr].vnum,
	      obj_proto[nr].short_description);
      send_to_char(buf, ch);
    }
  }
  return (found);
}

int vnum_object(char *searchname, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (isname(searchname, obj_proto[nr].name)) {
      sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
	      obj_index[nr].vnum,
	      obj_proto[nr].short_description);
      send_to_char(buf, ch);
    }
  }
  return (found);
}


/* create a character, and add it to the char list */
struct char_data *create_char(void)
{
  struct char_data *ch;

  CREATE(ch, struct char_data, 1);
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;
  GET_ID(ch) = max_id++;

  return (ch);
}


/* create a new mobile from a prototype */
struct char_data *read_mobile(mob_vnum nr, int type) /* and mob_rnum */
{
  mob_rnum i;
  struct char_data *mob;

  if (type == VIRTUAL) {
    if ((i = real_mobile(nr)) < 0) {
      log("WARNING: Mobile vnum %d does not exist in database.", nr);
      core_dump();
      return (NULL);
    }
  } else
    i = nr;

  CREATE(mob, struct char_data, 1);
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  if (!mob->points.max_hit) {
    mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
      mob->points.move;
  } else
    mob->points.max_hit = number(mob->points.hit, mob->points.mana);
  //Set Mana based on level

  mob->points.max_mana = GET_LEVEL(mob) * 100;
  
  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  mob_index[i].number++;
  GET_ID(mob) = max_id++;
  assign_triggers(mob, MOB_TRIGGER);

  return (mob);
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
  struct obj_data *obj;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);

  return (obj);
}


/* create a new object from a prototype */
struct obj_data *read_object(obj_vnum nr, int type) /* and obj_rnum */
{
  struct obj_data *obj;
  obj_rnum i;

  if (nr < 0) {
    log("SYSERR: Trying to create obj with negative (%d) num!", nr);
    return (NULL);
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      log("Object (V) %d does not exist in database.", nr);
      i = real_object(10); 
//      return (NULL);
    }
  } else
    i = nr;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  *obj = obj_proto[i];
  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);
   
  return (obj);
}


#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp;
  static int timer = 0;
//  char buf[128];

  /* jelson 10/22/92 */
  if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60 && !QUESTON) {

    /* one minute has passed */
    /*
     * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
     * factor of 60
     */

    timer = 0;

    /* since one minute has passed, increment zone ages */
    for (i = 0; i <= top_of_zone_table; i++) {
      if (zone_table[i].age < zone_table[i].lifespan &&
	  zone_table[i].reset_mode)
	(zone_table[i].age)++;

      if (zone_table[i].age >= zone_table[i].lifespan &&
	  zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
	/* enqueue zone */

	CREATE(update_u, struct reset_q_element, 1);

	update_u->zone_to_reset = i;
	update_u->next = 0;

	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}

	zone_table[i].age = ZO_DEAD;
      }
    }
  }	/* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next)
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      /*sprintf(buf, "Auto zone reset: %s",
	      zone_table[update_u->zone_to_reset].name);
      mudlog(buf, CMP, LVL_GOD, FALSE);*/
      /* dequeue */
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u;
	     temp = temp->next);

	if (!update_u->next)
	  reset_q.tail = temp;

	temp->next = update_u->next;
      }

      free(update_u);
      break;
    }
}

void log_zone_error(zone_rnum zone, int cmd_no, const char *message)
{
  char buf[256];

  sprintf(buf, "SYSERR: zone file: %s", message);
  mudlog(buf, NRM, LVL_GOD, TRUE);

  sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
  mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/* execute the reset command table of a given zone */
void reset_zone(zone_rnum zone)
{
  int cmd_no, last_cmd = 0;
  struct char_data *mob = NULL;
  struct obj_data *obj, *obj_to;
  int room_vnum, room_rnum;
  struct char_data *tmob=NULL; /* for trigger assignment */
  struct obj_data *tobj=NULL;  /* for trigger assignment */
  int mob_load = FALSE; /* ### */
  int obj_load = FALSE; /* ### */

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {

    if (ZCMD.if_flag && !last_cmd && !mob_load && !obj_load)
      continue;

     if (!ZCMD.if_flag) { /* ### */
       mob_load = FALSE;
       obj_load = FALSE;
     }

    switch (ZCMD.command) {
    case '*':			/* ignore command */
      last_cmd = 0;
      break;

    case 'M':			/* read a mobile */
      if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) &&
           (number(1, 100) >= ZCMD.arg4)) {
	mob = read_mobile(ZCMD.arg1, REAL);
	char_to_room(mob, ZCMD.arg3);
	last_cmd = 1;
        mob_load = TRUE;
        load_mtrigger(mob);
        tmob = mob;
      } else
	last_cmd = 0;
      break;

    case 'O':			/* read an object */
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
					(number(1, 100) >= ZCMD.arg4)) {
				if (ZCMD.arg3 >= 0) {
					obj = read_object(ZCMD.arg1, REAL);
	  obj_to_room(obj, ZCMD.arg3);
		obj_load = TRUE;
		load_otrigger(obj);
		tobj = obj;
				} else {
					obj = read_object(ZCMD.arg1, REAL);
					obj->in_room = NOWHERE;
					last_cmd = 1;
          obj_load = TRUE;
				}
      } else
				last_cmd = 0;
      break;
			
    case 'P':			/* object to object */
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
					obj_load && (number(1, 100) >= ZCMD.arg4)) {
				obj = read_object(ZCMD.arg1, REAL);
				if (!(obj_to = get_obj_num(ZCMD.arg3))) {
					ZONE_ERROR("target obj not found, command disabled");
					ZCMD.command = '*';
					break;
				}
				obj_to_obj(obj, obj_to);
				load_otrigger(obj);
				tobj = obj;
				last_cmd = 1;
      } else
				tmob = NULL;
			last_cmd = 0;
      break;
			
    case 'G':			/* obj_to_char */
      if (!mob) {
				ZONE_ERROR("attempt to give obj to non-existant mob, command disabled");
				ZCMD.command = '*';
				break;
      }
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          mob_load && (number(1, 100) >= ZCMD.arg4)) {
				obj = read_object(ZCMD.arg1, REAL);
				obj_to_char(obj, mob);
        tobj = obj;
        load_otrigger(obj);
				last_cmd = 1;
      } else
				last_cmd = 0;
      tmob = NULL;
      break;
			
    case 'E':			/* object to equipment list */
      if (!mob) {
				ZONE_ERROR("trying to equip non-existant mob, command disabled");
				ZCMD.command = '*';
				break;
      }
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          mob_load && (number(1, 100) >= ZCMD.arg4)) {
				if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
					ZONE_ERROR("invalid equipment pos number");
				} else {
					obj = read_object(ZCMD.arg1, REAL);
					/*  IN_ROOM(obj) = IN_ROOM(mob); */
					IN_ROOM(obj) = NOWHERE;
          load_otrigger(obj);
          if (wear_otrigger(obj, mob, ZCMD.arg3))
						equip_char(mob, obj, ZCMD.arg3);
          else
            obj_to_char(obj, mob);
          tobj = obj;
					last_cmd = 1;
				}
      } else
				last_cmd = 0;
      tmob = NULL;
      break;
			
    case 'R': /* rem obj from room */
      if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != NULL) {
        obj_from_room(obj);
        extract_obj(obj);
      }
      last_cmd = 1;
      tmob = NULL;
      tobj = NULL;
      break;
			
			
    case 'D':			/* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
					(world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
				ZONE_ERROR("door does not exist, command disabled");
				ZCMD.command = '*';
      } else
				switch (ZCMD.arg3) {
				case 0:
					REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
										 EX_LOCKED);
					REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
					break;
				case 1:
					SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
									EX_CLOSED);
					REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
										 EX_LOCKED);
					break;
				case 2:
					SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
									EX_LOCKED);
					SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
									EX_CLOSED);
					break;
	}
      last_cmd = 1;
      tmob = NULL;
      tobj = NULL;
      break;
			
    case 'T': /* trigger command; details to be filled in later */
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob))
          CREATE(SCRIPT(tmob), struct script_data, 1);
        add_trigger(SCRIPT(tmob), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj))
          CREATE(SCRIPT(tobj), struct script_data, 1);
        add_trigger(SCRIPT(tobj), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      }
      break;

    case 'V':
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob)) {
          ZONE_ERROR("Attempt to give variable to scriptless mobile");
        } else
          add_var(&(SCRIPT(tmob)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj)) {
          ZONE_ERROR("Attempt to give variable to scriptless object");
        } else
          add_var(&(SCRIPT(tobj)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==WLD_TRIGGER) {
        if (ZCMD.arg2<0 || ZCMD.arg2>top_of_world) {
          ZONE_ERROR("Invalid room number in variable assignment");
        } else {
          if (!(world[ZCMD.arg2].script)) {
            ZONE_ERROR("Attempt to give variable to scriptless object");
          } else
            add_var(&(world[ZCMD.arg2].script->global_vars),
                    ZCMD.sarg1, ZCMD.sarg2, ZCMD.arg3);
          last_cmd = 1;
        }
      }
      break;

    default:
      ZONE_ERROR("unknown cmd in reset table; cmd disabled");
      ZCMD.command = '*';
      break;
    }
  }

  zone_table[zone].age = 0;

  /* handle reset_wtrigger's */
  room_vnum = zone_table[zone].number * 100;
  while (room_vnum <= zone_table[zone].top) {
    room_rnum = real_room(room_vnum);
    if (room_rnum != NOWHERE) reset_wtrigger(&world[room_rnum]);
    room_vnum++;
  }
}

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(zone_rnum zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING)
      continue;
    if (IN_ROOM(i->character) == NOWHERE)
      continue;
    if (GET_LEVEL(i->character) >= LVL_IMMORT)
      continue;
    if (world[i->character->in_room].zone != zone_nr)
      continue;

    return (0);
  }

  return (1);
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*************************************************************************/


long get_ptable_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp(player_table[i].name, arg))
      return (i);

  return (-1);
}


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp(player_table[i].name, arg))
      return (player_table[i].id);

  return (-1);
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if (player_table[i].id == id)
      return (player_table[i].name);

  return (NULL);
}

#define NUM_OF_SAVE_THROWS     5

/* new load_char that reads an ascii pfile */  
/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_data *ch) 
{
  int id, num = 0, num2 = 0, num3 = 0, num4 = 0, num5 = 0, num6 = 0, i;
  FBFILE *fl;
  char filename[40];
  char buf[128], line[MAX_INPUT_LENGTH+1], tag[6];
  struct affected_type tmp_aff[MAX_AFFECT];   

  if((id = find_name(name)) < 0) { 
    return (-1);
  } else {
    sprintf(filename, "%s/%c/%s%s", PLR_PREFIX, *player_table[id].name,
      player_table[id].name, PLR_SUFFIX);
    if(!(fl = fbopen(filename, FB_READ))) {
      sprintf(buf, "SYSERR: Couldn't open player file %s", filename);
      mudlog(buf, NRM, LVL_GOD, TRUE);
      return (-1);
    }

    /* initializations necessary to keep some things straight */
    if (ch->player_specials == NULL)
      CREATE(ch->player_specials, struct player_special_data, 1);
    ch->in_room = NOWHERE;
    ch->followers = NULL;
    ch->master = NULL;
    ch->carrying = NULL;
    ch->next = NULL;
    ch->next_fighting = NULL;
    ch->next_in_room = NULL;
    FIGHTING(ch) = NULL;
    ch->char_specials.position = POS_STANDING;
    ch->mob_specials.default_pos = POS_STANDING;     
    ch->char_specials.carry_weight = 0;
    ch->char_specials.carry_items = 0;
    ch->affected = NULL;
    ch->temp_master = NULL;
    // history counts.
    ch->char_specials.ghcount = 0;
    ch->char_specials.thcount = 0;

    for(i = 1; i <= MAX_SKILLS; i++)
      GET_SKILL(ch, i) = 0;
    for(i=0; i < 5; i++)
      GET_TETHER(ch, i) = NULL;
    GET_SEX(ch) = PFDEF_SEX;
    GET_CLASS(ch) = PFDEF_CLASS;
    GET_LEVEL(ch) = PFDEF_LEVEL;
    SET_THIEF_LEVEL(ch, PFDEF_THIEF_LEVEL); 
    SET_WARRIOR_LEVEL(ch, PFDEF_WARRIOR_LEVEL); 
    SET_MAGE_LEVEL(ch, PFDEF_MAGE_LEVEL); 
    SET_CLERIC_LEVEL(ch, PFDEF_CLERIC_LEVEL);
    GET_HOME(ch) = PFDEF_HOMETOWN;
    GET_HEIGHT(ch) = PFDEF_HEIGHT;
    GET_WEIGHT(ch) = PFDEF_WEIGHT;
    GET_ALIGNMENT(ch) = PFDEF_ALIGNMENT;
    PLR_FLAGS(ch) = PFDEF_PLRFLAGS;
    PLR_FLAGS2(ch) = PFDEF_PLRFLAGS2;
    AFF_FLAGS(ch) = PFDEF_AFFFLAGS;
    for(i = 0; i < NUM_OF_SAVE_THROWS; i++)
      GET_SAVE(ch, i) = PFDEF_SAVETHROW;
    GET_LOADROOM(ch) = PFDEF_LOADROOM;
    GET_INVIS_LEV(ch) = PFDEF_INVISLEV;
    GET_FREEZE_LEV(ch) = PFDEF_FREEZELEV;
    GET_WIMP_LEV(ch) = PFDEF_WIMPLEV;
    GET_COND(ch, FULL) = PFDEF_HUNGER;                    
    GET_COND(ch, THIRST) = PFDEF_THIRST;
    GET_COND(ch, DRUNK) = PFDEF_DRUNK;
    GET_BAD_PWS(ch) = PFDEF_BADPWS;
    PRF_FLAGS(ch) = PFDEF_PREFFLAGS;
    PRF_FLAGS2(ch) = PFDEF_PREFFLAGS2;
    GET_PRACTICES(ch) = PFDEF_PRACTICES;
    GET_GOLD(ch) = PFDEF_GOLD;
    GET_BANK_GOLD(ch) = PFDEF_BANK;
    GET_EXP(ch) = PFDEF_EXP;
    GET_HITROLL(ch) = PFDEF_HITROLL;
    GET_DAMROLL(ch) = PFDEF_DAMROLL;
    GET_AC(ch) = PFDEF_AC;
    GET_STR(ch) = PFDEF_STR;
    GET_DEX(ch) = PFDEF_DEX;
    GET_INT(ch) = PFDEF_INT;
    GET_WIS(ch) = PFDEF_WIS;
    GET_CON(ch) = PFDEF_CON;
    GET_CHA(ch) = PFDEF_CHA;
    GET_HIT(ch) = PFDEF_HIT;
    GET_MAX_HIT(ch) = PFDEF_MAXHIT;
    GET_MANA(ch) = PFDEF_MANA;                
    GET_MAX_MANA(ch) = PFDEF_MAXMANA;
    GET_MOVE(ch) = PFDEF_MOVE;
    GET_MAX_MOVE(ch) = PFDEF_MAXMOVE;
    ROMANCE(ch) = PFDEF_ROMANCE;
    PARTNER(ch) = PFDEF_PARTNER;
    GET_RELIGION(ch) = PFDEF_RELIGION;
    GET_CLAN(ch) = PFDEF_CLAN;
    GET_CLAN_RANK(ch) = PFDEF_CLANRANK;
    GET_HORSENAME(ch) = PFDEF_HORSENAME;
    GET_HORSEHAPPY(ch) = PFDEF_HORSEHAPPY;
    GET_HORSELEVEL(ch) = PFDEF_HORSELEVEL;
    GET_HORSEROOM(ch) = PFDEF_HORSEROOM;
    GET_HORSEEXP(ch) = PFDEF_HORSEEXP;
    GET_HORSEEQ(ch) = PFDEF_HORSEEQ;
    TRACKING(ch) = PFDEF_TRACKING;
    GET_MAXHAPPY(ch) = PFDEF_MAXHAPPY;
    GET_SPARRANK(ch) = PFDEF_SPARRANK;
    GET_ECONRANK(ch) = PFDEF_ECONRANK;
    GET_ASSASSINRANK(ch) = PFDEF_ASSASSINRANK;
    GET_CAREERRANK(ch) = PFDEF_CAREERRANK;
    GET_KILLS(ch) = PFDEF_KILLS;
    GET_DEATHS(ch) = PFDEF_DEATHS;
    GET_SACRIFICE(ch) = PFDEF_SACRIFICES;
    GET_GM_LEVEL(ch) = PFDEF_GM_LEVEL;
    IS_LORD(ch) = PFDEF_IS_LORD;
    GET_LNAME(ch) = PFDEF_LNAME;

    for (i = 0; i < MAX_AFFECT; i++) {
      tmp_aff[i].type = 0;
      tmp_aff[i].duration = 0;
      tmp_aff[i].bitvector = 0;
      tmp_aff[i].modifier = 0;
      tmp_aff[i].power = 0;
      tmp_aff[i].location = APPLY_NONE;
    }

     while(fbgetline(fl, line)) {
      tag_argument(line, tag);
      num = atoi(line);

      switch (*tag) {
      case 'A':
       if(!strcmp(tag, "Ac  "))
         GET_AC(ch) = PFDEF_AC;
       else if(!strcmp(tag, "Act "))
         PLR_FLAGS(ch) = num;
       else if(!strcmp(tag, "Act2"))
         PLR_FLAGS2(ch) = num;
       else if(!strcmp(tag, "Aff "))
         AFF_FLAGS(ch) = asciiflag_conv(line);
       else if(!strcmp(tag, "Affs")) {
         i = 0;
         do {
           fbgetline(fl, line);
           sscanf(line, "%d %d %d %d %d %d", &num, &num2, &num3, &num4, &num5, &num6);
           tmp_aff[i].type = num;
           tmp_aff[i].duration = num2;
           tmp_aff[i].modifier = num3;
           tmp_aff[i].location = num4;
           tmp_aff[i].bitvector = num5;
           tmp_aff[i].power = num6;
           i++;
         } while (num != 0);
       } else if(!strcmp(tag, "Alin"))
         GET_ALIGNMENT(ch) = num;
       else if(!strcmp(tag, "ARnk"))
            GET_ASSASSINRANK(ch) = num;

       break;

      case 'B':
       if(!strcmp(tag, "Badp"))
         GET_BAD_PWS(ch) = num;                
       else if(!strcmp(tag, "Bank"))
           if (ch->player.time.logon > 1055097146)
           {
            GET_BANK_GOLD(ch) = num;
           }
           else
           {
            GET_BANK_GOLD(ch) = .1 * num;
           }
       else if(!strcmp(tag, "Brth"))
         ch->player.time.birth = num;
       break;

      case 'C':
       if(!strcmp(tag, "Cha "))
         GET_CHA(ch) = num;
       else if(!strcmp(tag, "Clas"))
         GET_CLASS(ch) = num;
       else if(!strcmp(tag, "Clan"))
         GET_CLAN(ch) = num;
       else if(!strcmp(tag, "Con "))
         GET_CON(ch) = num;
       else if(!strcmp(tag, "Crr "))
         GET_CAREERRANK(ch) = num; 
       break;

      case 'D':
       if(!strcmp(tag, "Desc")) {
         ch->player.description = fbgetstring(fl);
       } else if(!strcmp(tag, "Dex "))
         GET_DEX(ch) = num;
       else if(!strcmp(tag, "Drnk"))             
         GET_COND(ch, DRUNK) = num;
       else if(!strcmp(tag, "Drol"))
         GET_DAMROLL(ch) = PFDEF_DAMROLL;
       else if(!strcmp(tag, "Deth"))
         GET_DEATHS(ch) = num;
       break;

      case 'E':
       if(!strcmp(tag, "Exp "))
           if (ch->player.time.logon > LAST_EXP_RESET)
           {
            GET_EXP(ch) = num;
           }
           else
           {
            GET_EXP(ch) = .1 * num;
           }
       else if(!strcmp(tag, "Econ"))
           if (ch->player.time.logon > LAST_ECON_RESET)
           {
            GET_ECONRANK(ch) = num;
           }
       break;

      case 'F':
       if(!strcmp(tag, "Frez"))
         GET_FREEZE_LEV(ch) = num;
       break;

      case 'G':
       if(!strcmp(tag, "Gold"))
           if (ch->player.time.logon > LAST_EXP_RESET)
           {
            GET_GOLD(ch) = num;
           }
           else
           {
            GET_GOLD(ch) = .1 * num;
           }
       else if(!strcmp(tag, "Gmst"))
         GET_GM_LEVEL(ch) = num;
       break;
       
      case 'H':                            
       if(!strcmp(tag, "Hit ")) {
         sscanf(line, "%d/%d", &num, &num2);
         GET_HIT(ch) = num;
         GET_MAX_HIT(ch) = num2;
       } else if(!strcmp(tag, "Hite"))
         GET_HEIGHT(ch) = num;
       else if(!strcmp(tag, "Home"))
         GET_HOME(ch) = num;
       else if(!strcmp(tag, "Hore"))
         GET_HORSEEXP(ch) = num;
       else if(!strcmp(tag, "Horh")) {
         sscanf(line, "%d/%d", &num, &num2);
         GET_HORSEHAPPY(ch) = num;
         GET_MAXHAPPY(ch) = num2;
       }
       if(!strcmp(tag, "Horl"))
         GET_HORSELEVEL(ch) = num;
       else if(!strcmp(tag, "Horn"))
         GET_HORSENAME(ch) = str_dup(line);
       else if(!strcmp(tag, "Horq"))
         GET_HORSEEQ(ch) = num;
       if(!strcmp(tag, "Hrom"))
         GET_HORSEROOM(ch) = num;
       else if(!strcmp(tag, "Host"))
         ch->player_specials->host = str_dup(line);
       else if(!strcmp(tag, "Hrol"))
         GET_HITROLL(ch) = PFDEF_HITROLL;
       else if(!strcmp(tag, "Hung"))
        GET_COND(ch, FULL) = -1;
       break;                

      case 'I':
       if(!strcmp(tag, "Id  "))
         // Always override based on player_table in case of mixup
         GET_IDNUM(ch) = player_table[id].id;
       else if(!strcmp(tag, "Int "))
         GET_INT(ch) = num;
       else if(!strcmp(tag, "Invs"))
         GET_INVIS_LEV(ch) = num;
       break;  

      case 'K':
        if(!strcmp(tag, "Kill"))
          GET_KILLS(ch) = num;
      break;

      case 'L':
       if(!strcmp(tag, "Last"))
         ch->player.time.logon = num;
       else if(!strcmp(tag, "Lern"))
           if (ch->player.time.logon > 1057104245)
           {
            GET_PRACTICES(ch) = num;
           }
           else
           {
            GET_PRACTICES(ch) = 999;
           }
       else if(!strcmp(tag, "Levl"))
         GET_LEVEL(ch) = num;
       else if(!strcmp(tag, "LThf")) {
         SET_THIEF_LEVEL(ch, num);
			 } else if(!strcmp(tag, "LWar")) {
				 SET_WARRIOR_LEVEL(ch, num);
			 } else if(!strcmp(tag, "LMge")) {
         SET_MAGE_LEVEL(ch, num);
			 } else if(!strcmp(tag, "LClr")) {
         SET_CLERIC_LEVEL(ch, num);
			 } else if(!strcmp(tag, "LNam")) {
         GET_LNAME(ch) = str_dup(line);
       } else if(!strcmp(tag, "Lord"))
         IS_LORD(ch) = num;

      break;

      case 'M':
       if(!strcmp(tag, "Mana")) {
         sscanf(line, "%d/%d", &num, &num2);
         GET_MANA(ch) = num;
         GET_MAX_MANA(ch) = num2;
       } else if(!strcmp(tag, "Move")) {
         sscanf(line, "%d/%d", &num, &num2);
         GET_MOVE(ch) = num;
         GET_MAX_MOVE(ch) = num2;
       }
       break; 

      case 'N':
				if(!strcmp(tag, "Name")) {
					SET_NAME(ch, str_dup(line));
				}
       break;      

      case 'P':
       if(!strcmp(tag, "Pass"))
         strcpy(GET_PASSWD(ch), line);
       else if(!strcmp(tag, "Part"))
       {
         ch->player.partner = str_dup(line);
       }
       else if(!strcmp(tag, "Plyd"))
         ch->player.time.played = num;
       else if(!strcmp(tag, "PfIn"))
         POOFIN(ch) = str_dup(line);
       else if(!strcmp(tag, "PfOt"))
         POOFOUT(ch) = str_dup(line);
       else if(!strcmp(tag, "Pref"))
         PRF_FLAGS(ch) = asciiflag_conv(line);
       else if(!strcmp(tag, "Prf2"))
         PRF_FLAGS2(ch) = asciiflag_conv(line);
       else if(!strcmp(tag, "PreT"))
         GET_PRETITLE(ch) = str_dup(line);
       else if(!strcmp(tag, "Pos "))
         GET_POS(ch) = (byte)num;
       break;

      case 'R':
       if(!strcmp(tag, "Room"))
         GET_LOADROOM(ch) = num;
       else if (!strcmp(tag, "Rmac"))
         ROMANCE(ch) = num;
       else if (!strcmp(tag, "Rank"))
         GET_CLAN_RANK(ch) = num;
       else if (!strcmp(tag, "Relg"))
         GET_RELIGION(ch) = num;
       break;

      case 'S':
       if(!strcmp(tag, "Sex "))
         GET_SEX(ch) = num;
       else if(!strcmp(tag, "Skil")) {
         do {
           fbgetline(fl, line);
           sscanf(line, "%d %d", &num, &num2);
           if(num != 0)
           {
            if (ch->player.time.logon > 1057104245)
            {
              GET_SKILL(ch, num) = num2;
            }
            else
            {
              GET_SKILL(ch, num) = 0;
            }
           }
          } while (num != 0);
       } 
      else if(!strcmp(tag, "Spar"))
         GET_SPARRANK(ch) = num;
      else if(!strcmp(tag, "Str ")) {
         sscanf(line, "%d/%d", &num, &num2);
         GET_STR(ch) = num;
       }
      else if(!strcmp(tag, "Sac "))
       {
         GET_SACRIFICE(ch) = num;
       }
       break;                                          

      case 'T':
       if(!strcmp(tag, "Thir"))
         GET_COND(ch, THIRST) = -1;
       else if(!strcmp(tag, "Thr1"))
         GET_SAVE(ch, 0) = num;
       else if(!strcmp(tag, "Thr2"))
         GET_SAVE(ch, 1) = num;
       else if(!strcmp(tag, "Thr3"))
         GET_SAVE(ch, 2) = num;
       else if(!strcmp(tag, "Thr4"))
         GET_SAVE(ch, 3) = num;
       else if(!strcmp(tag, "Thr5"))
         GET_SAVE(ch, 4) = num;
       else if(!strcmp(tag, "Titl"))
         GET_TITLE(ch) = str_dup(line);
       else if(!strcmp(tag, "Trak"))
         TRACKING(ch) = str_dup(line);
       break;

      case 'W':
       if(!strcmp(tag, "Wate"))
         GET_WEIGHT(ch) = num;
       else if(!strcmp(tag, "Wimp"))
         GET_WIMP_LEV(ch) = num;           
       else if(!strcmp(tag, "Wis "))
         GET_WIS(ch) = num;
       break;

      default:
       sprintf(buf, "SYSERR: Unknown tag %s in pfile %s", tag, name);
      }
    }             
  }

  /* set real_abils = aff_abils */
  ch->real_abils = ch->aff_abils;

  for (i = 0; i < MAX_AFFECT; i++) {
    if (tmp_aff[i].type)
      affect_to_char(ch, &tmp_aff[i]);
  }

  /* initialization for imms */
  if(GET_LEVEL(ch) >= LVL_IMMORT) {
    for(i = 1; i <= MAX_SKILLS; i++)
      GET_SKILL(ch, i) = 10;
    GET_COND(ch, FULL) = -1;
    GET_COND(ch, THIRST) = -1;
    GET_COND(ch, DRUNK) = -1;   
  }

  if (GET_PRACTICES(ch) == 999 || ch->player.time.logon < 1057104245)
     GET_PRACTICES(ch) = GET_TOTAL_LEVEL(ch) + GET_GM_LEVEL(ch)*2;

  fbclose(fl);


  return(id);
}

int load_family(char *name, struct descriptor_data *d)
{
     int num = 0, count = 0, i;
     FBFILE *fl;
     char filename[40];
     char buf[128], line[MAX_INPUT_LENGTH+1], tag[6];

     d->family.numchars = 0;
    
       /* change to lowercase */
       for (i = 0; name[i]; i++)
        name[i] = LOWER(name[i]);

	 sprintf(filename, "%s/%c/%s", "ffiles", name[0], name);

    if(!(fl = fbopen(filename, FB_READ))) 
    {
      sprintf(buf, "SYSERR: Couldn't open Family file %s", filename);
      mudlog(buf, NRM, LVL_GOD, TRUE);
      return (-1);
    }

    while(fbgetline(fl, line)) 
	{
      tag_argument(line, tag);
      num = atoi(line);

	 if(!strcmp(tag, "Name"))
		d->family.name = str_dup(line);
	 if(!strcmp(tag, "Pass"))
                strcpy(d->family.pass, line);
         if(!strcmp(tag, "NmCh"))
		d->family.numchars = num;

     if(!strcmp(tag, "Char"))
	 {
                CREATE(d->family.chars[count], struct char_data, 1);
                clear_char(d->family.chars[count]);
		if (load_char(str_dup(line), d->family.chars[count]) > -1)
		{
                    count++;
		}
                else 
                   free_char(d->family.chars[count]);
	 }
	}

	if (d->family.numchars > count+1)
         {
	  sprintf(buf, "SYSERR: Number of Chars in Family doesn't match in %s", filename);
      mudlog(buf, NRM, LVL_GOD, TRUE);
      return (-1);
	}

        fbclose(fl);
	return 1;
}

void save_family(struct descriptor_data *d)
{
  FBFILE *fl;
  char outname[40], buf[MAX_STRING_LENGTH];
  int i;

  sprintf(outname, "%s/%c/%s", "ffiles", d->family.name[0], d->family.name);
  if (!(fl = fbopen(outname, FB_WRITE))) 
  {
    sprintf(buf, "SYSERR: Couldn't open player file %s for write",
               outname);
    mudlog(buf, NRM, LVL_GOD, TRUE);
    return;
  }
  fbprintf(fl, "Name: %s\n", d->family.name);
  fbprintf(fl, "Pass: %s\n", d->family.pass);
  fbprintf(fl, "NmCh: %d\n", d->family.numchars);
  for (i = 0; i < d->family.numchars; i++)
  {
    fbprintf(fl, "Char: %s\n", GET_NAME(d->family.chars[i]));
  }
  fbclose(fl);

  return;
}

int add_char_family(char *name, struct descriptor_data *d)
{
  FBFILE *fl;
  char outname[40];
  struct char_data *tch = NULL;

  sprintf(outname, "%s/%c/%s", "ffiles", d->family.name[0], d->family.name);
  if (!(fl = fbopen(outname, FB_APPEND))) 
  {
    sprintf(buf, "SYSERR: Couldn't open family file %s for write",
               outname);
    mudlog(buf, NRM, LVL_GOD, TRUE);
    return -1;
  }
  if (load_char(name, tch) > -1)
  {
      d->family.numchars++;
      d->family.chars[d->family.numchars- 1] = tch;
      save_family(d);
  }
  else 
    return -1;

  fbclose(fl);
  return 1;
}


/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
/* new save_char that writes an ascii pfile */
void save_char(struct char_data * ch, sh_int load_room)     
{
  FBFILE *fl;
  char outname[40], bits[127], buf[MAX_STRING_LENGTH];
  int i, j, id, save_index = FALSE;
  struct affected_type *aff, tmp_aff[MAX_AFFECT];
  struct obj_data *char_eq[NUM_WEARS];   
  /* Econ Time */

  if (IS_NPC(ch) || GET_PFILEPOS(ch) < 0)
    return;

  /* If ch->desc doesn't exist, it's because we're doing a set file.
     Last host will disappear in that case.  No biggie, eh?
  */

  if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
    if (load_room == NOWHERE)
      GET_LOADROOM(ch) = NOWHERE;      
    else
      GET_LOADROOM(ch) = world[load_room].number;   
  }

  /*strcpy(player.pwd, GET_PASSWD(ch));*/  
  {
    for (i = 0;
      (*(bits + i) = LOWER(*(GET_NAME(ch) + i))); i++);
   sprintf(outname, "%s/%c/%s%s", PLR_PREFIX, *bits, bits, PLR_SUFFIX);  
    if (!(fl = fbopen(outname, FB_WRITE))) {
      sprintf(buf, "SYSERR: Couldn't open player file %s for write",
               outname);
      mudlog(buf, NRM, LVL_GOD, TRUE);
      return;
    }

    /* remove affects from eq and spells (from char_to_store) */   

    /* Unaffect everything a character can be affected by */

    for (aff = ch->affected, i = 0; i < MAX_AFFECT; i++) {
      if (aff) {
        tmp_aff[i] = *aff;
        tmp_aff[i].next = 0;
        aff = aff->next;
      } else {
        tmp_aff[i].type = 0;   /* Zero signifies not used */
        tmp_aff[i].duration = 0;
        tmp_aff[i].modifier = 0;
        tmp_aff[i].location = 0;
        tmp_aff[i].bitvector = 0;
        tmp_aff[i].power = 0;
        tmp_aff[i].next = 0;
      }
    }

    for (i = 0; i < NUM_WEARS; i++) 
    {
      if (GET_EQ(ch, i))
        char_eq[i] = unequip_char(ch, i);
      else
        char_eq[i] = NULL;
   }               
    /*
     * remove the affections so that the raw values are stored; otherwise the
     * effects are doubled when the char logs back in.
     */                                               

    while (ch->affected)
      affect_remove(ch, ch->affected); 

    if ((i >= MAX_AFFECT) && aff && aff->next)
      log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");  

    ch->aff_abils = ch->real_abils; 

    /* end char_to_store code */

    if(GET_NAME(ch))
      fbprintf(fl, "Name: %s\n", GET_NAME(ch));
    if(GET_PASSWD(ch))
      fbprintf(fl, "Pass: %s\n", GET_PASSWD(ch));
    if(GET_TITLE(ch))
      fbprintf(fl, "Titl: %s\n", GET_TITLE(ch));
    if(GET_PRETITLE(ch))
      fbprintf(fl, "PreT: %s\n", GET_PRETITLE(ch));
    if(GET_POS(ch) > POS_STUNNED)
      fbprintf(fl, "Pos : %d\n", GET_POS(ch));

    if(ch->player.description && *ch->player.description) 
    {
      strcpy(buf, ch->player.description);
      genolc_strip_cr(buf);
      fbprintf(fl, "Desc:\n%s~\n", buf);
    }
    if(POOFIN(ch))
      fbprintf(fl, "PfIn: %s\n", POOFIN(ch));
    if(POOFOUT(ch))
      fbprintf(fl, "PfOt: %s\n", POOFOUT(ch));
    if(GET_SEX(ch) != PFDEF_SEX)
      fbprintf(fl, "Sex : %d\n", GET_SEX(ch));
    if(GET_CLASS(ch) != PFDEF_CLASS)                   
      fbprintf(fl, "Clas: %d\n", GET_CLASS(ch));
    if(GET_LEVEL(ch) != PFDEF_LEVEL)
      fbprintf(fl, "Levl: %d\n", GET_LEVEL(ch));
    if(GET_THIEF_LEVEL(ch) != PFDEF_THIEF_LEVEL)
      fbprintf(fl, "LThf: %d\n", GET_THIEF_LEVEL(ch));
    if(GET_WARRIOR_LEVEL(ch) != PFDEF_WARRIOR_LEVEL)
      fbprintf(fl, "LWar: %d\n", GET_WARRIOR_LEVEL(ch));
    if(GET_MAGE_LEVEL(ch) != PFDEF_MAGE_LEVEL)
      fbprintf(fl, "LMge: %d\n", GET_MAGE_LEVEL(ch));
    if(GET_CLERIC_LEVEL(ch) != PFDEF_CLERIC_LEVEL)
      fbprintf(fl, "LClr: %d\n", GET_CLERIC_LEVEL(ch));  
    if(GET_HOME(ch) != PFDEF_HOMETOWN)
      fbprintf(fl, "Home: %d\n", GET_HOME(ch));

      fbprintf(fl, "Brth: %d\n", ch->player.time.birth);
      fbprintf(fl, "Plyd: %d\n", ch->player.time.played);
      fbprintf(fl, "Last: %d\n", ch->player.time.logon);
    if(ch->player_specials->host)
      fbprintf(fl, "Host: %s\n", ch->player_specials->host);
    if(GET_HEIGHT(ch) != PFDEF_HEIGHT)
      fbprintf(fl, "Hite: %d\n", GET_HEIGHT(ch));      
    if(GET_WEIGHT(ch) != PFDEF_HEIGHT)
      fbprintf(fl, "Wate: %d\n", GET_WEIGHT(ch));

    if(GET_ALIGNMENT(ch) != PFDEF_ALIGNMENT)
      fbprintf(fl, "Alin: %d\n", GET_ALIGNMENT(ch));
      fbprintf(fl, "Id  : %d\n", GET_IDNUM(ch));
    if(PLR_FLAGS(ch) != PFDEF_PLRFLAGS)
      fbprintf(fl, "Act : %d\n", PLR_FLAGS(ch));
    if(PLR_FLAGS2(ch) != PFDEF_PLRFLAGS2)
      fbprintf(fl, "Act2: %d\n", PLR_FLAGS2(ch));
    if(AFF_FLAGS(ch) != PFDEF_AFFFLAGS) {
      sprintbits(AFF_FLAGS(ch), bits); 
      fbprintf(fl, "Aff : %s\n", bits);    
     }
    if(GET_SAVE(ch, 0) != PFDEF_SAVETHROW)
      fbprintf(fl, "Thr1: %d\n", GET_SAVE(ch, 0));
    if(GET_SAVE(ch, 1) != PFDEF_SAVETHROW)
      fbprintf(fl, "Thr2: %d\n", GET_SAVE(ch, 1));
    if(GET_SAVE(ch, 2) != PFDEF_SAVETHROW)
      fbprintf(fl, "Thr3: %d\n", GET_SAVE(ch, 2));
    if(GET_SAVE(ch, 3) != PFDEF_SAVETHROW)
      fbprintf(fl, "Thr4: %d\n", GET_SAVE(ch, 3));
    if(GET_SAVE(ch, 4) != PFDEF_SAVETHROW)
      fbprintf(fl, "Thr5: %d\n", GET_SAVE(ch, 4));

    if(GET_LEVEL(ch) < LVL_IMMORT) {
      fbprintf(fl, "Skil:\n");
      for(i = 1; i <= MAX_SKILLS; i++) {
       if(GET_SKILL(ch, i))
         fbprintf(fl, "%d %d\n", i, GET_SKILL(ch, i));
      }
      fbprintf(fl, "0 0\n");
    }
    if(GET_WIMP_LEV(ch) != PFDEF_WIMPLEV)
      fbprintf(fl, "Wimp: %d\n", GET_WIMP_LEV(ch));
    if(GET_FREEZE_LEV(ch) != PFDEF_FREEZELEV)
      fbprintf(fl, "Frez: %d\n", GET_FREEZE_LEV(ch));      
    if(GET_INVIS_LEV(ch) != PFDEF_INVISLEV)
      fbprintf(fl, "Invs: %d\n", GET_INVIS_LEV(ch));
    if(GET_LOADROOM(ch) != PFDEF_LOADROOM)
      fbprintf(fl, "Room: %d\n", GET_LOADROOM(ch));
    if(GET_HORSELEVEL(ch) != PFDEF_HORSELEVEL)
      fbprintf(fl, "Horl: %d\n", GET_HORSELEVEL(ch));
    if(GET_HORSEROOM(ch) != PFDEF_HORSEROOM)
      fbprintf(fl, "Hrom: %d\n", GET_HORSEROOM(ch));
    if(PRF_FLAGS(ch) != PFDEF_PREFFLAGS) {
      sprintbits(PRF_FLAGS(ch), bits);
      fbprintf(fl, "Pref: %s\n", bits);
    }
    if(PRF_FLAGS2(ch) != PFDEF_PREFFLAGS2) {
      sprintbits(PRF_FLAGS2(ch), bits);
      fbprintf(fl, "Prf2: %s\n", bits);
    }
    if(GET_BAD_PWS(ch) != PFDEF_BADPWS)
      fbprintf(fl, "Badp: %d\n", GET_BAD_PWS(ch));
    if(GET_COND(ch, FULL) != PFDEF_HUNGER && GET_LEVEL(ch) < LVL_IMMORT)
      fbprintf(fl, "Hung: %d\n", GET_COND(ch, FULL));
    if(GET_COND(ch, THIRST) != PFDEF_THIRST && GET_LEVEL(ch) < LVL_IMMORT)
      fbprintf(fl, "Thir: %d\n", GET_COND(ch, THIRST));
    if(GET_COND(ch, DRUNK) != PFDEF_DRUNK && GET_LEVEL(ch) < LVL_IMMORT)
      fbprintf(fl, "Drnk: %d\n", GET_COND(ch, DRUNK));
    if(GET_PRACTICES(ch) != PFDEF_PRACTICES)
      fbprintf(fl, "Lern: %d\n", GET_PRACTICES(ch));
    if(GET_STR(ch) != PFDEF_STR)
      fbprintf(fl, "Str : %d/\n", GET_STR(ch));
    if(GET_SACRIFICE(ch) != PFDEF_SACRIFICES)
      fbprintf(fl, "Sac : %d\n", GET_SACRIFICE(ch));
    if(GET_INT(ch) != PFDEF_INT)
      fbprintf(fl, "Int : %d\n", GET_INT(ch));
    if(GET_WIS(ch) != PFDEF_WIS)
      fbprintf(fl, "Wis : %d\n", GET_WIS(ch));
    if(GET_DEX(ch) != PFDEF_DEX)
      fbprintf(fl, "Dex : %d\n", GET_DEX(ch));
    if(GET_CON(ch) != PFDEF_CON)
      fbprintf(fl, "Con : %d\n", GET_CON(ch));
    if(GET_CHA(ch) != PFDEF_CHA)
      fbprintf(fl, "Cha : %d\n", GET_CHA(ch));
    if(GET_HIT(ch) != PFDEF_HIT || GET_MAX_HIT(ch) != PFDEF_MAXHIT)
      fbprintf(fl, "Hit : %d/%d\n", GET_HIT(ch), GET_MAX_HIT(ch));
    if(GET_MANA(ch) != PFDEF_MANA || GET_MAX_MANA(ch) != PFDEF_MAXMANA)
      fbprintf(fl, "Mana: %d/%d\n", GET_MANA(ch), GET_MAX_MANA(ch));
    if(GET_MOVE(ch) != PFDEF_MOVE || GET_MAX_MOVE(ch) != PFDEF_MAXMOVE)
      fbprintf(fl, "Move: %d/%d\n", GET_MOVE(ch), GET_MAX_MOVE(ch));
    if(GET_GOLD(ch) != PFDEF_GOLD)
      fbprintf(fl, "Gold: %d\n", GET_GOLD(ch));
    if(GET_BANK_GOLD(ch) != PFDEF_BANK)
      fbprintf(fl, "Bank: %d\n", GET_BANK_GOLD(ch));                     
    if(GET_EXP(ch) != PFDEF_EXP)
      fbprintf(fl, "Exp : %d\n", GET_EXP(ch));
    if(GET_ECONRANK(ch) != PFDEF_ECONRANK)
      fbprintf(fl, "Econ: %d\n", GET_ECONRANK(ch)); 
    if(GET_ASSASSINRANK(ch) != PFDEF_ASSASSINRANK)
      fbprintf(fl, "ARnk: %d\n", GET_ASSASSINRANK(ch)); 
    if(ROMANCE(ch) != PFDEF_ROMANCE)
      fbprintf(fl, "Rmac: %d\n", ROMANCE(ch));
    if(GET_RELIGION(ch) != PFDEF_RELIGION)
      fbprintf(fl, "Relg: %d\n", GET_RELIGION(ch));
    if(PARTNER(ch) != PFDEF_PARTNER)
      fbprintf(fl, "Part: %s\n", PARTNER(ch));
    if(GET_CLAN(ch) != PFDEF_CLAN)
      fbprintf(fl, "Clan: %d\n", GET_CLAN(ch));
    if(GET_CLAN_RANK(ch) != PFDEF_CLANRANK)
      fbprintf(fl, "Rank: %d\n", GET_CLAN_RANK(ch));
    if(GET_HORSEEXP(ch) != PFDEF_HORSEEXP)
      fbprintf(fl, "Hore: %d\n", GET_HORSEEXP(ch));
    if(GET_HORSENAME(ch) != PFDEF_HORSENAME)
      fbprintf(fl, "Horn: %s\n", GET_HORSENAME(ch));
    if(GET_HORSEHAPPY(ch) != PFDEF_HORSEHAPPY || GET_MAXHAPPY(ch) != PFDEF_MAXHAPPY)
      fbprintf(fl, "Horh: %d/%d\n", GET_HORSEHAPPY(ch), GET_MAXHAPPY(ch));
    if(GET_HORSEEQ(ch) != PFDEF_HORSEEQ)
      fbprintf(fl, "Horq: %d\n", GET_HORSEEQ(ch));
    if(GET_DEATHS(ch) != PFDEF_DEATHS)
      fbprintf(fl, "Deth: %d\n", GET_DEATHS(ch));
    if(GET_KILLS(ch) != PFDEF_KILLS)
      fbprintf(fl, "Kill: %d\n", GET_KILLS(ch));
    if(GET_GM_LEVEL(ch) != PFDEF_GM_LEVEL)
      fbprintf(fl, "Gmst: %d\n", GET_GM_LEVEL(ch));
    if(IS_LORD(ch) != PFDEF_IS_LORD)  
      fbprintf(fl, "Lord: %d\n", IS_LORD(ch));
    if(GET_LNAME(ch) != PFDEF_LNAME)
      fbprintf(fl, "LNam: %s\n", GET_LNAME(ch));

    if(GET_SPARRANK(ch) != PFDEF_SPARRANK)
      fbprintf(fl, "Spar: %d\n", GET_SPARRANK(ch));
   if(GET_CAREERRANK(ch) != PFDEF_CAREERRANK)
      fbprintf(fl, "Crr : %d\n", GET_CAREERRANK(ch));
    if(TRACKING(ch) != PFDEF_TRACKING)
      fbprintf(fl, "Trak: %s\n", TRACKING(ch));

    /* affected_type */
    fbprintf(fl, "Affs:\n");
    for(i = 0; i < MAX_AFFECT; i++) {
      aff = &tmp_aff[i];
      if(aff->type)
       fbprintf(fl, "%d %d %d %d %d %d\n", aff->type, aff->duration,
         aff->modifier, aff->location, (int)aff->bitvector, aff->power);
    }
    fbprintf(fl, "0 0 0 0 0\n");                               
    fbclose(fl);  

    if((id = find_name(GET_NAME(ch))) < 0)
      return;
    /* update the player in the player index */
    if(player_table[id].level != GET_LEVEL(ch)) {
      save_index = TRUE;
      player_table[id].level = GET_LEVEL(ch);
    }
    if(player_table[id].levelthief != GET_THIEF_LEVEL(ch)) {
      save_index = TRUE;
      player_table[id].levelthief = GET_THIEF_LEVEL(ch);
    }
    if(player_table[id].levelmage != GET_MAGE_LEVEL(ch)) {
      save_index = TRUE;
      player_table[id].levelmage = GET_MAGE_LEVEL(ch);
    }
    if(player_table[id].levelcleric != GET_CLERIC_LEVEL(ch)) {
      save_index = TRUE;
      player_table[id].levelcleric = GET_CLERIC_LEVEL(ch);
    }
    if(player_table[id].levelwarrior != GET_WARRIOR_LEVEL(ch)) {
      save_index = TRUE;
      player_table[id].levelwarrior = GET_WARRIOR_LEVEL(ch);
    }
    if(player_table[id].last != ch->player.time.logon) {
      save_index = TRUE;
      player_table[id].last = ch->player.time.logon;
    }
    i = player_table[id].flags;
    if(PLR_FLAGGED(ch, PLR_DELETED))
      SET_BIT(player_table[id].flags, PINDEX_DELETED);
    else
      REMOVE_BIT(player_table[id].flags, PINDEX_DELETED);
    if(PLR_FLAGGED(ch, PLR_NODELETE) || PLR_FLAGGED(ch, PLR_CRYO))
      SET_BIT(player_table[id].flags, PINDEX_NODELETE);
    else
      REMOVE_BIT(player_table[id].flags, PINDEX_NODELETE);
    if (PLR_FLAGGED(ch, PLR_FROZEN) || PLR_FLAGGED(ch, PLR_NOWIZLIST) ||
        PLR_FLAGGED(ch, PLR_DELETED))
      j = TRUE;
    else
      j = FALSE;
 
    if (player_table[id].autowiz != j) {
      save_index = TRUE;
      player_table[id].autowiz = j;
    }                         
      save_player_index();
    }


/* PFILE PROBLEM */
  for (i = 0; i < NUM_WEARS; i++) 
  {
    if (char_eq[i]) 
    {
        equip_char(ch, char_eq[i], i);
    }
  }
   if (ch->affected) 
        while (ch->affected)
          affect_remove(ch, ch->affected);


    /* more char_to_store code to restore affects */
    for (i = 0; i < MAX_AFFECT; i++) {
      if (tmp_aff[i].type)
        affect_to_char(ch, &tmp_aff[i]);
    }


    /* end char_to_store code */


  
/*   affect_total(ch); unnecessary, I think !?! */
}				/* Char to store */



void save_etext(struct char_data * ch)
{
/* this will be really cool soon */

}


/*
 * Create a new entry in the in-memory index table for the player file.
 * If the name already exists, by overwriting a deleted character, then
 * we re-use the old position.
 */
int create_entry(char *name)
{
  int i, pos;

  if (top_of_p_table == -1) {	/* no table */
    CREATE(player_table, struct player_index_element, 1);
    pos = top_of_p_table = 0;
  } else if ((pos = get_ptable_by_name(name)) == -1) {	/* new name */
    i = ++top_of_p_table + 1;

    RECREATE(player_table, struct player_index_element, i);
    pos = top_of_p_table;
  }

  CREATE(player_table[pos].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (player_table[pos].name[i] = LOWER(name[i])); i++)
	/* Nothing */;

  return (pos);
}



/************************************************************************
*  funcs of a (more or less) general utility nature			*
************************************************************************/


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength;

  *buf = '\0';

  do {
    if (!fgets(tmp, 512, fl)) {
      log("SYSERR: fread_string: format error at or near %s", error);
      exit(1);
    }
    /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }

    templength = strlen(tmp);

    if (length + templength >= MAX_STRING_LENGTH) {
      log("SYSERR: fread_string: string too large (db.c)");
      log(error);
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    CREATE(rslt, char, length + 1);
    strcpy(rslt, buf);
  } else
    rslt = NULL;

  return (rslt);
}


/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias_data *a;

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {

    while ((a = GET_ALIASES(ch)) != NULL) {
      GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
      free_alias(a);
    }
    if (ch->player_specials->poofin)
      free(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      free(ch->player_specials->poofout);
    if(ch->player_specials->host)
      free(ch->player_specials->host);
    free(ch->player_specials);
    if (IS_NPC(ch))
      log("SYSERR: Mob %s (#%d) had player_specials allocated!", GET_NAME(ch), GET_MOB_VNUM(ch));
  }
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch))
      free(GET_NAME(ch));
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
  } else if ((i = GET_MOB_RNUM(ch)) >= 0) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      free(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      free(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      free(ch->player.description);
  }
  while (ch->affected)
    affect_remove(ch, ch->affected);

  if (ch->desc)
    ch->desc->character = NULL;

  free(ch);
}




/* release memory allocated for an obj struct */
void free_obj(struct obj_data * obj)
{
  if (GET_OBJ_RNUM(obj) == NOWHERE)
    free_object_strings(obj);
  else
    free_object_strings_proto(obj);

  free(obj);
}


/*
 * Steps:
 *   1: Make sure no one is using the pointer in paging.
 *   2: Read contents of a text file.
 *   3: Allocate space.
 *   4: Point 'buf' to it.
 *
 * We don't want to free() the string that someone may be
 * viewing in the pager.  page_string() keeps the internal
 * str_dup()'d copy on ->showstr_head and it won't care
 * if we delete the original.  Otherwise, strings are kept
 * on ->showstr_vector but we'll only match if the pointer
 * is to the string we're interested in and not a copy.
 */
int file_to_string_alloc(const char *name, char **buf)
{
  char temp[MAX_STRING_LENGTH];
  struct descriptor_data *in_use;

  for (in_use = descriptor_list; in_use; in_use = in_use->next)
    if (in_use->showstr_vector && *in_use->showstr_vector == *buf)
      return (-1);

  /* Lets not free() what used to be there unless we succeeded. */
  if (file_to_string(name, temp) < 0)
    return (-1);

  if (*buf)
    free(*buf);

  *buf = str_dup(temp);
  return (0);
}


/* read contents of a text file, and place in buf */
int file_to_string(const char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    log("SYSERR: reading %s: %s", name, strerror(errno));
    return (-1);
  }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
        log("SYSERR: %s: string too big (%d max)", name,
		MAX_STRING_LENGTH);
	*buf = '\0';
	return (-1);
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}



/* clear some of the the working variables of a char */
void reset_char(struct char_data * ch)
{
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    GET_EQ(ch, i) = NULL;

  ch->followers = NULL;
  ch->master = NULL;
  ch->in_room = NOWHERE;
  ch->carrying = NULL;
  ch->next = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room = NULL;
  FIGHTING(ch) = NULL;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;
  if (GET_HORSEHAPPY(ch) <= 0)
    GET_HORSEHAPPY(ch) = 50;

  GET_LAST_TELL(ch) = NOBODY;
 
  GET_ID(ch) = max_id++;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void clear_char(struct char_data * ch)
{
  memset((char *) ch, 0, sizeof(struct char_data));

  ch->in_room = NOWHERE;
  GET_PFILEPOS(ch) = -1;
  GET_MOB_RNUM(ch) = NOBODY;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->temp_master = NULL;
  ch->char_specials.thcount = 0;
  ch->char_specials.ghcount = 0;

  GET_AC(ch) = 0;		/* Basic Armor */
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;
}

void clear_family(struct family_data * fam)
{
  memset((char *) fam, 0, sizeof(struct family_data));

    fam->numchars = 0; 
}


void clear_object(struct obj_data * obj)
{
  memset((char *) obj, 0, sizeof(struct obj_data));

  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
  obj->worn_on = NOWHERE;
}




/* initialize a new character only if class is set */
void init_char(struct char_data * ch)
{
  int i;

  /* create a player_special structure */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  /* *** if this is our first player --- he be God *** */
  if (top_of_p_table == 0)
    GET_LEVEL(ch) = LVL_IMPL;

  set_title(ch, NULL);
  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.description = NULL;

  ch->player.hometown = 1;

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TONGUE; i++)
    GET_TALK(ch, i) = 0;

  /* make favors for sex */
  if (ch->player.sex == SEX_MALE) {
    ch->player.weight = number(120, 180);
    ch->player.height = number(160, 200);
  } else {
    ch->player.weight = number(100, 160);
    ch->player.height = number(150, 180);
  }

  ch->points.max_mana = 100;
  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.max_move = 82;
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 0;
  ch->points.horsehappy = GET_MAXHAPPY(ch);

  if ((i = get_ptable_by_name(GET_NAME(ch))) != -1)
    player_table[i].id = GET_IDNUM(ch) = ++top_idnum;
  else
    log("SYSERR: init_char: Character '%s' not found in player table.", GET_NAME(ch));

  for (i = 1; i <= MAX_SKILLS; i++) {
    if (GET_LEVEL(ch) < LVL_IMPL)
      SET_SKILL(ch, i, 0);
    else
      SET_SKILL(ch, i, 100);
  }

  ch->char_specials.saved.affected_by = 0;
  ch->char_specials.thcount = 0;
  ch->char_specials.ghcount = 0;


  for (i = 0; i < 5; i++)
    GET_SAVE(ch, i) = 0;

  /*
  ch->real_abils.intel = 25;
  ch->real_abils.wis = 25;
  ch->real_abils.dex = 25;
  ch->real_abils.str = 25;
  ch->real_abils.str_add = 100;
  ch->real_abils.con = 25;
  ch->real_abils.cha = 25;
  */

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 24);

  GET_LOADROOM(ch) = NOWHERE;
}



/* returns the real number of the room with given virtual number */
room_rnum real_room(room_vnum vnum)
{
  room_rnum bot, top, mid;

  bot = 0;
  top = top_of_world;

  /* perform binary search on world-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((world + mid)->number == vnum)
      return (mid);
    if (bot >= top)
      return (NOWHERE);
    if ((world + mid)->number > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

void sprintbits(long vektor,char *outstring)
{
  int i;
  char flags[53]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  strcpy(outstring,"");
  for (i=0;i<32;i++)
  {
    if (vektor & 1) {
      *outstring=flags[i];
      outstring++;
    };
    vektor>>=1;
  };
  *outstring=0;
};

/* returns the real number of the monster with given virtual number */
mob_rnum real_mobile(mob_vnum vnum)
{
  mob_rnum bot, top, mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->vnum == vnum)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->vnum > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}


void tag_argument(char *argument, char *tag)
{
  char *tmp = argument, *ttag = tag, *wrt = argument;
  int i;

  for(i = 0; i < 4; i++)
    *(ttag++) = *(tmp++);
  *ttag = '\0';
  
  while(*tmp == ':' || *tmp == ' ')
    tmp++;

  while(*tmp)
    *(wrt++) = *(tmp++);
  *wrt = '\0';
}


void save_player_index(void)
{
  int i;
  char bits[64];
  FBFILE *index_file;

  if(!(index_file = fbopen(PLR_INDEX_FILE, FB_WRITE))) {
    log("SYSERR:  Could not write player index file");
    return;
  }

  for(i = 0; i <= top_of_p_table; i++)
    if(*player_table[i].name) {
      sprintbits(player_table[i].flags, bits);
      fbprintf(index_file, "%ld %s %d %d %d %d %d %s %d %d\n",
player_table[i].id,
	player_table[i].name, player_table[i].level, player_table[i].levelthief, player_table[i].levelwarrior, player_table[i].levelmage, player_table[i].levelcleric, *bits ? bits
: "0", player_table[i].last, player_table[i].autowiz);
    }
  fbprintf(index_file, "~\n");

  fbclose(index_file);
}


void remove_player(int pfilepos)
{
  char backup_name[128], pfile_name[128]/*, rent_name[128]*/;
  char filename[MAX_INPUT_LENGTH];
  FBFILE *backup_file;

  if(!*player_table[pfilepos].name)
    return;

  if(player_table[pfilepos].level >= pfile_backup_minlevel
	&& backup_wiped_pfiles && (!selfdelete_fastwipe ||
	!IS_SET(player_table[pfilepos].flags, PINDEX_SELFDELETE))) {
    sprintf(backup_name, "%s/%s.%d", BACKUP_PREFIX,
	player_table[pfilepos].name, (int)time(0));
    if(!(backup_file = fbopen(backup_name, FB_WRITE))) {
      sprintf(buf, "PCLEAN: Unable to open backup file %s.",
	backup_name);
      log(buf);
      return;
    }

    /* pfile */
    fbprintf(backup_file, "**\n** PFILE: %s\n**\n",
	player_table[pfilepos].name);
    sprintf(pfile_name, "%s/%c/%s%s", PLR_PREFIX,
	*player_table[pfilepos].name, player_table[pfilepos].name,
	PLR_SUFFIX);
    if(!fbcat(pfile_name, backup_file))
      fbprintf(backup_file, "** (NO FILE)\n");

    /* rent file */
/*    fbprintf(backup_file, "**\n** RENT FILE: %s\n**\n",
	player_table[pfilepos].name);
    sprintf(rent_name, "%s/%c/%s", RENT_PREFIX,
	*player_table[pfilepos].name,
	player_table[pfilepos].name);
    if(!fbcat(rent_name, backup_file))
      fbprintf(backup_file, "** (NO FILE)\n");*/

    /* mail file */
/*    fbprintf(backup_file, "**\n** MAIL FILE: %s\n**\n",
	player_table[pfilepos].name);
    sprintf(mail_name, "%s/%c/%s", MAIL_PREFIX,
	*player_table[pfilepos].name,
	player_table[pfilepos].name);
    if(!fbcat(mail_name, backup_file))
      fbprintf(backup_file, "** (NO FILE)\n");*/

    fbclose(backup_file);
  }
  
  unlink(pfile_name);
  
  //// Delete Files////
  //Objects//
   get_filename(player_table[pfilepos].name, filename, NEW_OBJ_FILES);
   sprintf(buf, "rm %s", filename);
   system(buf); 
  //Pfile//
  sprintf(buf, "rm %s/%c/%s%s", PLR_PREFIX,
   *player_table[pfilepos].name,
    player_table[pfilepos].name, PLR_SUFFIX);
  system(buf);

  /////End Deletion/////

  sprintf(buf, "PCLEAN: %s Lev: %d Last: %s",
	player_table[pfilepos].name, player_table[pfilepos].level,
	asctime(localtime(&player_table[pfilepos].last)));
  log(buf);
  player_table[pfilepos].name[0] = '\0';
  save_player_index();
}


void clean_pfiles(void)
{
  int i, ci, timeout;

  for(i = 0; i <= top_of_p_table; i++) {
    if(IS_SET(player_table[i].flags, PINDEX_NODELETE))
      continue;
    timeout = -1;
    for(ci = 0; ci == 0 || (pclean_criteria[ci].level > 
	pclean_criteria[ci - 1].level); ci++) {
      if((pclean_criteria[ci].level == -1 && IS_SET(player_table[i].flags,
		PINDEX_DELETED)) || player_table[i].level <=
		pclean_criteria[ci].level) {
	timeout = pclean_criteria[ci].days;
	break;
      }
    }
    if(timeout >= 0) {
      timeout *= SECS_PER_REAL_DAY;
      if((time(0) - player_table[i].last) > timeout)
	remove_player(i);
    }
  }  
}



/* returns the real number of the object with given virtual number */
obj_rnum real_object(obj_vnum vnum)
{
  obj_rnum bot, top, mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->vnum == vnum)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->vnum > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/*
 * Extend later to include more checks.
 *
 * TODO: Add checks for unknown bitvectors.
 */
int check_object(struct obj_data *obj)
{
  int error = FALSE;

  if (GET_OBJ_WEIGHT(obj) < 0 && (error = TRUE))
    log("SYSERR: Object #%d (%s) has negative weight (%d).",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_WEIGHT(obj));

  if (GET_OBJ_RENT(obj) < 0 && (error = TRUE))
    log("SYSERR: Object #%d (%s) has negative cost/day (%d).",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_RENT(obj));

  sprintbit(GET_OBJ_WEAR(obj), wear_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown wear flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown extra flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown affection flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_DRINKCON:
  {
    char onealias[MAX_INPUT_LENGTH], *space = strchr(obj->name, ' ');
    int offset = space ? space - obj->name : strlen(obj->name);

    strncpy(onealias, obj->name, offset);
    onealias[offset] = '\0';
  }
  /* Fall through. */
  case ITEM_FOUNTAIN:
    if (GET_OBJ_VAL(obj, 1) > GET_OBJ_VAL(obj, 0) && (error = TRUE))
      log("SYSERR: Object #%d (%s) contains (%d) more than maximum (%d).",
		GET_OBJ_VNUM(obj), obj->short_description,
		GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 0));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    error |= check_object_level(obj, 0);
    error |= check_object_spell_number(obj, 1);
    error |= check_object_spell_number(obj, 2);
    error |= check_object_spell_number(obj, 3);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    error |= check_object_level(obj, 0);
    error |= check_object_spell_number(obj, 3);
    if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1) && (error = TRUE))
      log("SYSERR: Object #%d (%s) has more charges (%d) than maximum (%d).",
		GET_OBJ_VNUM(obj), obj->short_description,
		GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1));
    break;
 }

  return (error);
}


int my_obj_save_to_disk(FILE *fp, struct obj_data *obj, int locate)
{
  int counter2;
  struct extra_descr_data *ex_desc;
  char buf1[MAX_STRING_LENGTH +1];

    if (obj->action_description) {
        strcpy(buf1, obj->action_description);
        strip_string(buf1);
      } else
        *buf1 = 0;

        fprintf(fp,
              "#%d\n"
             "%d %d %d %d %d %d\n",
              GET_OBJ_VNUM(obj),
              locate,
              GET_OBJ_VAL(obj, 0),
              GET_OBJ_VAL(obj, 1),
              GET_OBJ_VAL(obj, 2),
              GET_OBJ_VAL(obj, 3),
              GET_OBJ_EXTRA(obj));

  if(!(IS_OBJ_STAT(obj,ITEM_UNIQUE_SAVE))) {
    return 1;
  }
       fprintf(fp,
             "XAP\n"
              "%s~\n"
              "%s~\n"
              "%s~\n"
              "%s~\n"
              "%d %d %d %d %d %d\n",
              obj->name ? obj->name : "undefined",
              obj->short_description ? obj->short_description : "undefined",
             obj->description ? obj->description : "undefined",
              buf1,
              GET_OBJ_TYPE(obj),
              GET_OBJ_WEAR(obj),
              GET_OBJ_WEIGHT(obj),
              GET_OBJ_COST(obj),
              GET_OBJ_RENT(obj),
              GET_OBJ_LEVEL(obj));
      /* Do we have affects? */
      for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++)
        if (obj->affected[counter2].modifier)
          fprintf(fp, "A\n"
                  "%d %d\n",
                  obj->affected[counter2].location,
                  obj->affected[counter2].modifier
            );

      /* Do we have extra descriptions? */
      if (obj->ex_description) {        /*. Yep, save them too . */
        for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) {
          /*. Sanity check to prevent nasty protection faults . */
          if (!*ex_desc->keyword || !*ex_desc->description) {
            continue;
          }
          strcpy(buf1, ex_desc->description);
          strip_string(buf1);
          fprintf(fp, "E\n"
                  "%s~\n"
                  "%s~\n",
                  ex_desc->keyword,
                  buf1
            );
        }
      }
  return 1;
}

/* This procedure removes the '\r\n' from a string so that it may be
  saved to a file.  Use it only on buffers, not on the orginal
  strings. */

void strip_string(char *buffer)
{
  register char *ptr, *str;

  ptr = buffer;
  str = ptr;

 while ((*str = *ptr)) {
    str++;
    ptr++;
   if (*ptr == '\r')
      ptr++;
  }
}

int read_xap_objects(FILE *fl,struct char_data *ch) {
  char line[MAX_STRING_LENGTH];
  int t[10],nr,danger,j,k,zwei;
  struct obj_data *temp=NULL;
  struct extra_descr_data *new_descr;
  int num_of_objs=0;

  if(!feof(fl))
    get_line(fl, line);
  while (!feof(fl) && line != NULL) {
       /* first, we get the number. Not too hard. */
    if(*line == '#') {
      num_of_objs++;
      if (sscanf(line, "#%d", &nr) != 1) {
      continue;
      }
      /* we have the number, check it, load obj. */
      if (nr == -1) {  /* then its unique */
        temp = create_obj();
        temp->item_number=NOTHING;
        SET_BIT(GET_OBJ_EXTRA(temp), ITEM_UNIQUE_SAVE);
      } else if ( nr < 0) {
         continue;
      } else  {
        if(nr >= 99999) {
          continue;
        }
        temp=read_object(nr,VIRTUAL);
        if (!temp) {
          continue;
        }
      }
      /* now we read locate. - this is for autoeq, will be 0 elsewise */
      /* only the rest of the vals, and extra flags will be read */
      get_line(fl,line);
      sscanf(line,"%d %d %d %d %d %d",t, t + 1, t+2, t + 3, t + 4,t+5);
      GET_OBJ_VAL(temp,0) = t[1];
      GET_OBJ_VAL(temp,1) = t[2];
      GET_OBJ_VAL(temp,2) = t[3];
      GET_OBJ_VAL(temp,3) = t[4];
      GET_OBJ_EXTRA(temp) = t[5];

      get_line(fl,line);
       /* read line check for xap. */
      if(!strcasecmp("XAP",line)) {  /* then this is a Xap Obj, requires
                                       special care */
        if ((temp->name = fread_string(fl, buf2)) == NULL) {
          temp->name = str_dup("undefined");
        }

        if ((temp->short_description = fread_string(fl, buf2)) == NULL) {
          temp->short_description = str_dup("undefined");
        }

        if ((temp->description = fread_string(fl, buf2)) == NULL) {
          temp->description = str_dup("undefined");
        }

        if ((temp->action_description = fread_string(fl, buf2)) == NULL) {
          temp->action_description=0;
       }
        if (!get_line(fl, line) ||
           (sscanf(line, "%d %d %d %d %d %d", t,t+1,t+2,t+3,t+4,t+5) != 6)) {
          fprintf(stderr, "Format error in first numeric line (expecting _x_ args)");
          return 0;
        }
        temp->obj_flags.type_flag = t[0];
        temp->obj_flags.wear_flags = t[1];
        temp->obj_flags.weight = t[2];
        temp->obj_flags.cost = t[3];
        temp->obj_flags.cost_per_day = t[4];
       /* Delete */
        temp->obj_flags.level = t[5];

       /* buf2 is error codes pretty much */
        strcat(buf2, ", after numeric constants (expecting E/#xxx)");

       /* we're clearing these for good luck */

       for (j = 0; j < MAX_OBJ_AFFECT; j++) {
          temp->affected[j].location = APPLY_NONE;
          temp->affected[j].modifier = 0;
        }

       /* You have to null out the extradescs when you're parsing a xap_obj.
           This is done right before the extradescs are read. */

        if (temp->ex_description) {
          temp->ex_description = NULL;
        }

        for (k=j=zwei=0;!zwei && !feof(fl);) {
          switch (*line) {
            case 'E':
              CREATE(new_descr, struct extra_descr_data, 1);
              new_descr->keyword = fread_string(fl, buf2);
              new_descr->description = fread_string(fl, buf2);
              new_descr->next = temp->ex_description;
              temp->ex_description = new_descr;
             get_line(fl,line);
              break;
           case 'A':
              if (j >= MAX_OBJ_AFFECT) {
                log("Too many object affectations in loading rent file");
                danger=1;
             }
             get_line(fl, line);
              sscanf(line, "%d %d", t, t + 1);

              temp->affected[j].location = t[0];
              temp->affected[j].modifier = t[1];  
              j++;
              get_line(fl,line);
              break;

            case '$':
            case '#':
              zwei=1;
              break;
            default:
              zwei=1;
              break;
          }
        }      /* exit our for loop */
        get_line(fl,line);
      }   /* exit our xap loop */
      IN_ROOM(ch)=1;
      if(temp != NULL) {
        obj_to_char(temp,ch);
      }
    }
  }  /* exit our while loop */
  return num_of_objs;
}

int check_object_spell_number(struct obj_data *obj, int val)
{
  int error = FALSE;
  const char *spellname;

  if (GET_OBJ_VAL(obj, val) == -1)	/* i.e.: no spell */
    return (error);

  /*
   * Check for negative spells, spells beyond the top define, and any
   * spell which is actually a skill.
   */
  if (GET_OBJ_VAL(obj, val) < 0)
    error = TRUE;
  if (GET_OBJ_VAL(obj, val) > TOP_SPELL_DEFINE)
    error = TRUE;
  if (GET_OBJ_VAL(obj, val) > MAX_SPELLS && GET_OBJ_VAL(obj, val) <= MAX_SKILLS)
    error = TRUE;
  if (error)
    log("SYSERR: Object #%d (%s) has out of range spell #%d.",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

  /*
   * This bug has been fixed, but if you don't like the special behavior...
   */
#if 0
  if (GET_OBJ_TYPE(obj) == ITEM_STAFF &&
	HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS | MAG_MASSES))
    log("... '%s' (#%d) uses %s spell '%s'.",
	obj->short_description,	GET_OBJ_VNUM(obj),
	HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS) ? "area" : "mass",
	skill_name(GET_OBJ_VAL(obj, val)));
#endif

  if (scheck)		/* Spell names don't exist in syntax check mode. */
    return (error);

  /* Now check for unnamed spells. */
  spellname = skill_name(GET_OBJ_VAL(obj, val));

  if ((spellname == unused_spellname || !str_cmp("UNDEFINED",
spellname)) && (error = TRUE));

  return (error);
}

int check_object_level(struct obj_data *obj, int val)
{
  int error = FALSE;

  if ((GET_OBJ_VAL(obj, val) < 0 || GET_OBJ_VAL(obj, val) > MAX_GM_LEVEL) && (error = TRUE))
    log("SYSERR: Object #%d (%s) has out of range level #%d.",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

  return (error);
}


int get_new_pref() {
  FILE *fp;
  char filename[1024];
  int i;

  sprintf(filename,PREF_FILE);
  if(!(fp=fopen(filename,"r"))) {
    touch(filename);
    if(!(fp=fopen(filename,"w"))) {
      return (number(1,128000));
    }
    fprintf(fp,"1\r\n");
    fclose(fp);
    return 1;
  }
  fscanf(fp,"%d",&i);
  fclose(fp);
  unlink(filename);
  if(!(fp=fopen(filename,"w"))) {
    log("Unable to open p-ref file for writing");
    return i;
  }
  fprintf(fp,"%d\n",i+1);
  fclose(fp);
  return i;

}
