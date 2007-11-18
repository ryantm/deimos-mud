/* ************************************************************************
*  file:  wizlist.c                                     Part of CircleMUD *
*  Usage: self-updating wizlists                                          *
*  Written by Vladimir Marko (Yilard/VisionMUD)                           *
*  All Rights Reserved                                                    *
*  Copyright (C) 1998 Electronic Phantasies                               *
************************************************************************* */


#include "conf.h"
#include "sysdep.h"

#include <signal.h>

#include "structs.h"
#include "utils.h"
#include "db.h"

#define IMM_LMARG "   "
#define IMM_NSIZE  16
#define LINE_LEN   64
#define MIN_LEVEL LVL_IMMORT
#define MAX_LIST_LEVELS		10

/* max level that should be in columns instead of centered */
#define COL_LEVEL LVL_IMMORT

char *WIZLIST_HEADER=
"&b****************************************************************************\n"
"&g* The following people have reached immortality on VisionMUD.  They are to *\n"
"&G* treated with respect and awe.  Occasional prayers to them are advisable. *\n"
"&G* Annoying them is not recommended.  Stealing from them is punishable by   *\n"
"&g* immediate death.                                                         *\n"
"&b****************************************************************************&w\n\n";


struct name_rec {
  char name[25];
  struct name_rec *next;
};

struct control_rec {
  int level;
  char *level_name;
};

struct level_rec {
  struct control_rec *params;
  struct level_rec *next;
  struct name_rec *names;
};

typedef struct {
  char *data;
  char *last_line;
  int  ls_count;
} LINES;

typedef struct plr_rec {
  char *name;
  int  level;
  struct plr_rec *next;
} PLR_REC;

struct plr_rec *plr_list;
struct plr_rec *last_rec;

typedef struct {
  char * cc;
  char * txt;
} LEV_PAR;

LEV_PAR level_params[] =
{
    {"&b","Immortals"},
    {"&B","Gurus"},
    {"&y","Half Gods"},
    {"&r","Minor Gods"},
    {"&g","Gods"},
    {"&c","Greater Gods"},
    {"&C","Builders"},
    {"&M","Chief Builders"},
    {"&G","Coders"},
    {"&Y","CoImplementors"},
    {"&W","Implementor"}
};


void initialize(void)
{
  plr_list = NULL;
  last_rec = NULL;  
}


void read_file(int min_level)
{
  void add_name(byte level, char *name);

  struct char_file_u player;
  FILE *fl;

  if (!(fl = fopen(PLAYER_FILE, "rb"))) {
    perror("Error opening playerfile");
    exit(1);
  }
  while (!feof(fl)) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl) && player.level >= min_level &&
	!(IS_SET(player.char_specials_saved.act[0], PLR_FROZEN)) &&
	!(IS_SET(player.char_specials_saved.act[0], PLR_NOWIZLIST)) &&
	!(IS_SET(player.char_specials_saved.act[0], PLR_DELETED)))
      add_name(player.level, player.name);
  }

  fclose(fl);
}


void add_name(byte level, char *name)
{
  struct plr_rec *tmp;
  char *ptr;

  if (!*name)
    return;

  for (ptr = name; *ptr; ptr++)
    if (!isalpha(*ptr))
      return;

  tmp = (struct plr_rec *) malloc(sizeof(struct plr_rec));
  tmp->name = strdup(name);
  tmp->level = level;
  tmp->next = NULL;

  if (!last_rec) {
    plr_list = tmp;
    last_rec = tmp;
  } else {
    last_rec->next = tmp;
    last_rec = tmp;
  }
}

char *center_text(char *txt, int width)
{
  static char buf1[MAX_INPUT_LENGTH];
  int i;
  
  if (!txt) log("Serious problem.");
  if (width <= strlen(txt)) {
    strcpy(buf1, txt);
    return buf1;
  }
  for (i = 0; (i < (width-strlen(txt)) / 2) && i < MAX_INPUT_LENGTH-1; i++) buf1[i] = ' ';
  buf1[(width-strlen(txt)) / 2] = '\0';
  strcat(buf1, txt);
  for (i = strlen(buf1); i < width && i < MAX_INPUT_LENGTH-1; i++) buf1[i] = ' ';
  buf1[width] = '\0'; 
  return buf1;
}

void write_wizlist(char *fn, int min_level, int max_level, char *header)
{
  FILE *fw;
  int i;
  char buf[8192];
  int write_cnt;
  struct plr_rec *tmp;
  

  fw = fopen(fn, "w+");
  
  fprintf(fw, header);
  
  write_cnt = 0;
  buf[0] = '\0';
  for (i = max_level; i >= min_level; i--) {
    fprintf(fw, "\r\n%s%s&w\r\n\r\n", level_params[i-LVL_IMMORT].cc, 
      center_text(level_params[i-LVL_IMMORT].txt, 79));
    tmp = plr_list;
    write_cnt = 0;
    buf[0] = '\0';
    while (tmp) {
      if (tmp->level == i) {
        if (write_cnt % 4 != 3) strcat(buf, center_text(tmp->name, 20));
	else {
	  strcat(buf, center_text(tmp->name, 20));
	  fprintf(fw,"%s\r\n", center_text(buf, 79));
	  buf[0]='\0';
	}        
        write_cnt++;
      }
      tmp = tmp->next;    
    }
    if (write_cnt == 0) fprintf(fw,"%s\r\n", center_text("-", 79));
    if (write_cnt % 4 != 0) fprintf(fw,"%s\r\n", center_text(buf, 79));
  }
  
  fclose(fw);
}

void dealloc(void)
{
  struct plr_rec * tmp;
  struct plr_rec * tmp1;
  
  tmp = plr_list;
  while (tmp) {
    tmp1 = tmp->next;
    free(tmp->name);
    free(tmp);
    tmp = tmp1;
  }
}

void relist() 
{
  int wizlevel, immlevel;

  wizlevel = LVL_GOD;
  immlevel = LVL_IMMORT;
  initialize();
  read_file(LVL_IMMORT);
  write_wizlist(WIZLIST_FILE, LVL_GOD, LVL_IMPL, WIZLIST_HEADER);
  write_wizlist(IMMLIST_FILE, LVL_IMMORT, LVL_MINOR_GOD, WIZLIST_HEADER);
  dealloc();
  return;
}
