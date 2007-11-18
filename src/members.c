/* ************************************************************************
*  file:  members.c                                   Part of CircleMud   *
*  Usage: list a members belonging to certain clan 			  *
*  (for use by /bin/circle)						  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../db.h"
#include "../clan.h"

struct member_t {
  char *name;
  char *title;
  int level;
  int class;
  int sex;
  struct member_t *next;
};

char *clanname = NULL;
int clan = -1;
char *ranks[CLAN_NUM_RANKS];
struct member_t *members[CLAN_NUM_RANKS];

const char *class_abbrevs[] = {
  "Mu",
  "Cl",
  "Th",
  "Wa",
  "Pa",
  "Ra",
  "Wl",
  "Cy",
  "Ne",
  "Dr",
  "Al",
  "Ba",
  "\n"
};

void add_alpha_sort(struct char_file_u *player, struct member_t **m) 
{
  struct member_t **ptr = m;
  struct member_t *n = NULL;
  
  n = (struct member_t *) malloc(sizeof(struct member_t));
  n->name = strdup(player->name);
  n->title = strdup(player->title);
  n->level = player->level;
  n->class = player->chclass;
  n->sex = player->sex;
  n->next = NULL;
  
  while (*ptr != NULL) {
    if (strcmp((*ptr)->name, n->name) > 0) {
      n->next = (*ptr)->next;
      *ptr = n;
      return;
    }
    ptr = &((*ptr)->next);
  }
  *ptr = n;   
  return;
}

void print_title(char *s, int width)
{
  int x = (width - strlen(s)) / 2;
  int i;
  printf("&W");
  for (i = 0; i < x; i++)
    printf("-");
  printf("&C%s&W", s);  
  for (i = i+strlen(s); i < width; i++)
    printf("-");
  printf("\r\n");
}

void show_rank(int rank) 
{
  struct member_t *m = members[rank - 1];
  char classname[10];
  char sexname;
  char buf[256];
  int first = 1;
  
  while (m != NULL) {
    if (first) {
      sprintf(buf, " Rank %s ", ranks[rank - 1]);
      print_title(buf, 60);  
      first = 0;
    }
    if (m->class >= 0 && m->class < NUM_CLASSES)
      strcpy(classname, class_abbrevs[m->class]);
    else
      strcpy(classname, "--");
    switch (m->sex) {
    case SEX_FEMALE:
      sexname = 'F';
      break;
    case SEX_MALE:
      sexname = 'M';
      break;
    case SEX_NEUTRAL:
      sexname = 'N';
      break;
    default:
      sexname = '-';
      break;
    }
    printf("&w[&m%s &c%3d &r%c&w] &g%s %s\r\n", classname, m->level, sexname, m->name, m->title);
    m = m->next;
  }
  if (!first) printf("&w\r\n");
}

void show(char *filename)
{
  FILE *fl;
  struct char_file_u player;  
  int i, recs;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    perror("error opening playerfile");
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile showplay.\n");
    fclose(fl);
    exit(1);
  }
  recs = size / sizeof(struct char_file_u);
  for (i = 0; i < recs ; i++) {
    fread(&player, sizeof(struct char_file_u), 1, fl);
    if (!(player.char_specials_saved.act[0] & PLR_DELETED)
      && player.player_specials_saved.clannum == clan
      && player.player_specials_saved.clanrank >= CLAN_APPLY
      && player.player_specials_saved.clanrank <= CLAN_RETIRED)
        add_alpha_sort(&player, members + player.player_specials_saved.clanrank - 1);
  }
  show_rank(CLAN_RETIRED);
  show_rank(CLAN_RULER);
  show_rank(CLAN_BUILDER);
  show_rank(CLAN_CAPTAIN);
  show_rank(CLAN_SARGEANT);
  show_rank(CLAN_SOLDIER);
  show_rank(CLAN_APPLY);
}

int load_clan_file(char *fname)
{
  FILE *f;
  char buf[256];
  int i;
  
  if ((f = fopen(fname, "rb")) == NULL) {
    printf("Something went wrong. Contact IMPLEMENTOR.\r\n");
    perror(fname);
    return 0;
  }
  if (fscanf(f, " %d\n", &clan) != 1) {
    printf("Something went wrong. Contact IMPLEMENTOR.\r\n");
    fclose(f);
    unlink(fname);
    return 0;
  }
  fgets(buf, 255, f);
  clanname = strdup(buf);
  for (i = 1; i < CLAN_NUM_RANKS; i++) {
    if (fscanf(f, " %s", buf) != 1) {
      printf("Something went wrong. Contact IMPLEMENTOR.\r\n");
      fclose(f);
      unlink(fname);
      return 0;
    }
    members[i-1] = NULL;
    ranks[i-1] = strdup(buf);
  }
  fclose(f);
  unlink(fname);
  return 1;
}

int main(int argc, char **argv)
{
  if (argc != 3)
    printf("Usage: %s clanfile-name playerfile-name\n", argv[0]);
  else {
    if (load_clan_file(argv[1]))
      show(argv[2]);
  }

  return 0;
}
