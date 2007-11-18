/* ************************************************************************
*  file:  mailindex.c                                 Part of CircleMud   *
*  Usage: list all pieces of mail in a player mail file                   *
*  Written by Jeremy Elson                                                *
*  All Rights Reserved                                                    *
*  Copyright (C) 1993 The Trustees of The Johns Hopkins University        *
************************************************************************* */

/*
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
*/

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"
#include "../db.h"
#include "../mail.h"

#undef log(x)
#define log(x) puts(x)


/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

#ifdef ENOENT
#undef ENOENT
#endif

#define ENOENT 1

struct player_index_element *player_table = NULL;	/* index to plr file	 */
FILE *player_fl = NULL;		/* file desc of player file	 */
int top_of_p_table = 0;		/* ref to top of table		 */
int top_of_p_file = 0;		/* ref of size of p file	 */
long top_idnum = 0;		/* highest idnum in use		 */
char *MAILFILE;
char buf[MAX_STRING_LENGTH];
mail_index_type		*mail_index = 0; /* list of recs in the mail file  */
position_list_type 	*free_list = 0;  /* list of free positions in file */
long	file_end_pos = 0; /* length of file */

int MAX(int a, int b)
{
  return a > b ? a : b;
} 

/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  fprintf(stderr, "Player file: %s\n", "../lib/etc/players");
  if (!(player_fl = fopen("../lib/etc/players", "r+b"))) {
    if ((errno = ENOENT)) {
      perror("fatal error opening playerfile");
      exit(1);
    } else {
      if (!(player_fl = fopen("../lib/etc/players", "r+b"))) {
	perror("fatal error opening playerfile");
	exit(1);
      }
    }
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    fprintf(stderr, "   %ld players in database.", recs);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {	/* new record */
		nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
		player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
  }

  top_of_p_file = top_of_p_table = nr;
}

void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}

long get_id_by_name(char *name)
{
  int i;

  skip_spaces(&name);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp((player_table + i)->name, name))
      return ((player_table + i)->id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}


void	push_free_list(long pos)
{
   position_list_type * new_pos;

   CREATE(new_pos, position_list_type, 1);
   new_pos->position = pos;
   new_pos->next = free_list;
   free_list = new_pos;
}



long	pop_free_list(void)
{
   position_list_type * old_pos;
   long	return_value;

   if ((old_pos = free_list) != 0) {
      return_value = free_list->position;
      free_list = old_pos->next;
      free(old_pos);
      return return_value;
   } else
      return file_end_pos;
}




mail_index_type *find_char_in_index(long searchee)
{
	mail_index_type * tmp;

	if (searchee < 0) {
		log("Mail system -- non fatal error #1");
		return 0;
	}
  for (tmp = mail_index; (tmp && tmp->recipient != searchee); tmp = tmp->next);

	return tmp;
}





void	read_from_file(void *buf, int size, long filepos)
{
   FILE * mail_file;

   if (!(mail_file = fopen("../lib/etc/plrmail", "r"))) {
      perror("Error opening mail file for read");
      exit(1);
   }


   if (filepos % BLOCK_SIZE) {
      log("Mail system -- fatal error #2!!!");
      return;
   }

   fseek(mail_file, filepos, SEEK_SET);
   fread(buf, size, 1, mail_file);
   fclose(mail_file);
   return;
}




void	index_mail(long id_to_index, long pos)
{
   mail_index_type     * new_index;
   position_list_type * new_position;

   if (id_to_index < 0) {
      log("Mail system -- non-fatal error 4");
      return;
   }

   if (!(new_index = find_char_in_index(id_to_index))) {
      /* name not already in index".. add it */
      CREATE(new_index, mail_index_type, 1);
      new_index->recipient = id_to_index;
      new_index->list_start = NULL;

      /* add to front of list */
      new_index->next = mail_index;
      mail_index = new_index;
   }

   /* now, add this position to front of position list */
   CREATE(new_position, position_list_type, 1);
   new_position->position = pos;
   new_position->next = new_index->list_start;
   new_index->list_start = new_position;
}


/* SCAN_FILE */
/* scan_file is called once during boot-up.  It scans through the mail file
   and indexes all entries currently in the mail file. */
int	scan_file(void)
{
   FILE 		   * mail_file;
   header_block_type  next_block;
   int	total_messages = 0, block_num = 0;
   char	buf[100];

   if (!(mail_file = fopen("../lib/etc/plrmail", "r"))) {
      perror("Error opening mail file for read");
      exit(0);
   }

   while (fread(&next_block, sizeof(header_block_type), 1, mail_file)) {
      if (next_block.block_type == HEADER_BLOCK) {
	 index_mail(next_block.header_data.to, block_num * BLOCK_SIZE);
	 total_messages++;
      } else if (next_block.block_type == DELETED_BLOCK)
	 push_free_list(block_num * BLOCK_SIZE);
      block_num++;
   }

   file_end_pos = ftell(mail_file);
   fclose(mail_file);
   sprintf(buf, "   %ld bytes read.", file_end_pos);
   log(buf);
   if (file_end_pos % BLOCK_SIZE) {
      log("Error booting mail system -- Mail file corrupt!");
      log("Mail disabled!");
      return 0;
   }
   sprintf(buf, "   Mail file read -- %d messages.", total_messages);
   log(buf);
   return 1;
} /* end of scan_file */



int	main(int argc, char *argv[])
{
//   long	searchee;
//   char *ptr;
   mail_index_type * i1;
   position_list_type * i2;
   header_block_type header;

/*   if (argc != 2) {
      fprintf(stderr, "%s <mailfile>\n", argv[0]);
      exit(1);
   }

   build_player_index();
   "../lib/etc/plrmail" = strdup(argv[1]);*/
   build_player_index();
   fprintf(stderr, "\nPost Office: %s\n", "../lib/etc/plrmail");
   scan_file();

   for (i1 = mail_index; i1; i1 = i1->next) {
		printf("To: %-20s", get_name_by_id(i1->recipient));
		for (i2 = i1->list_start; i2; i2 = i2->next) {
	 read_from_file(&header, BLOCK_SIZE, i2->position);
         if (header.block_type != HEADER_BLOCK)
	   printf("\tFrom: Uncertain\r\n");
	 else
	   printf("\tFrom: %s\r\n", get_name_by_id(header.header_data.from));
      }
   }

   exit(0);
}


