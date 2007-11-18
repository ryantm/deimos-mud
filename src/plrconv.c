/* ************************************************************************
*  file: plrconv.c                                      Part of CircleMUD * 
*  Usage:                                                                 *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 Yilard/VisionMUD a.k.a. Cyberlord/EPH         *                   *
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct old_char_special_data_saved {
   int	alignment;		/* +-1000 for alignments                */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   long	act;			/* act flag for NPC's; player flag for PC's */

   long	affected_by;		/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};

/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct old_player_special_data_saved {
   byte skills[MAX_SKILLS+1];	/* array of skills plus skill 0		*/
   byte PADDING0;		/* used to be spells_to_learn		*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   byte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   room_vnum load_room;		/* Which room to place char in		*/
   long	pref;			/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[3];         /* Drunk, full, thirsty			*/

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ubyte spare0;
   ubyte spare1;
   ubyte spare2;
   ubyte spare3;
   ubyte spare4;
   ubyte spare5;
   int spells_to_learn;		/* How many can you learn yet this level*/
   int olc_zone;
   int spare8;
   int clannum;			/* Index number of clan you belong to (FIDO) */
   int spare10;	
   int clanrank;                /* Rank within clan */
   int rip_cnt;		
   int kill_cnt;
   int dt_cnt;
   int spare15;
   int spare16;
   long	spare17;
   long	spare18;
   long	spare19;
   long	spare20;
   long	spare21;
};

/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct old_affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long	bitvector;       /* Tells which bits to set (AFF_XXX)       */
   struct affected_type *next;
};


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct old_char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH];
   char	title[MAX_TITLE_LENGTH+1];
   char prompt[MAX_INPUT_LENGTH+1];
   byte sex;
   byte class;
   byte level;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int	played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct old_char_special_data_saved char_specials_saved;
   struct old_player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct old_affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
};

typedef struct char_file_u NewData;
typedef struct old_char_file_u OldData;

NewData NewBuf;
OldData OldBuf;

void ConvertData(OldData *oldd, NewData *newd)
{
  int i;
  
  
//  printf("1 ");
  /* First move all between name and pwd (inclusive) */
  memmove((void  *) newd, (const void *) oldd, 
    (void *) &(oldd->char_specials_saved)- (void *) &(oldd->name));
  
//  printf("2 ");
  newd->char_specials_saved.alignment = oldd->char_specials_saved.alignment;
  newd->char_specials_saved.idnum = oldd->char_specials_saved.idnum;
  newd->char_specials_saved.act[0] = oldd->char_specials_saved.act;
  newd->char_specials_saved.act[1] = 0;
  newd->char_specials_saved.act[2] = 0;
  newd->char_specials_saved.act[3] = 0;
  newd->char_specials_saved.affected_by[0] = oldd->char_specials_saved.affected_by;
  newd->char_specials_saved.affected_by[1] = 0;
  newd->char_specials_saved.affected_by[2] = 0;
  newd->char_specials_saved.affected_by[3] = 0;
  newd->char_specials_saved.apply_saving_throw[0] = oldd->char_specials_saved.apply_saving_throw[0];
  newd->char_specials_saved.apply_saving_throw[1] = oldd->char_specials_saved.apply_saving_throw[1];
  newd->char_specials_saved.apply_saving_throw[2] = oldd->char_specials_saved.apply_saving_throw[2];
  newd->char_specials_saved.apply_saving_throw[3] = oldd->char_specials_saved.apply_saving_throw[3];
  newd->char_specials_saved.apply_saving_throw[4] = oldd->char_specials_saved.apply_saving_throw[4];
    
//  printf("3 ");  
  /* Then copy player_specials_saved.skills to player_specials_saved.pref */
  memmove((void *) &(newd->player_specials_saved.skills), 
          (const void *) &(oldd->player_specials_saved.skills), 
    (void *) &(oldd->player_specials_saved.pref) - (void *) &(oldd->player_specials_saved.skills));
  
//  printf("4 ");  
  /* Copy pref[] */
  newd->player_specials_saved.pref[0] = oldd->player_specials_saved.pref;
  newd->player_specials_saved.pref[1] = 0;
  newd->player_specials_saved.pref[2] = 0;
  newd->player_specials_saved.pref[3] = 0;

  /* Copy rest of player_spelsias_saved, abilities and points */
  memmove((void *) &(newd->player_specials_saved.bad_pws), 
          (const void *) &(oldd->player_specials_saved.bad_pws), 
    (void *) &(oldd->affected) - (void *) &(oldd->player_specials_saved.bad_pws));
    
  /* Copy affected */
  for (i=0; i<MAX_AFFECT; i++) {
  newd->affected[i].type = oldd->affected[i].type;
  newd->affected[i].modifier = oldd->affected[i].modifier;
  newd->affected[i].duration = oldd->affected[i].duration;
  newd->affected[i].location = oldd->affected[i].location;
  newd->affected[i].bitvector[0] = oldd->affected[i].bitvector;
  newd->affected[i].bitvector[1] = 0;
  newd->affected[i].bitvector[2] = 0;
  newd->affected[i].bitvector[3] = 0;
  newd->affected[i].next = oldd->affected[i].next;
  }
  
  /* Copy rest of char_file_u */
  newd->last_logon = oldd->last_logon;
  strcpy(newd->host, oldd->host);
  
  /* and add poofs, etc */
  strcpy(newd->poof_in, "");
  strcpy(newd->poof_out, "");
  strcpy(newd->afk, "");

  if (newd->hometown == 0)
    newd->hometown = 1;
  
  switch (newd->level) {
    case 31: 
      newd->level = LVL_IMMORT;
      break;
    case 32: 
      newd->level = LVL_GOD;
      break;
    case 33: 
      newd->level = LVL_GRGOD;
      break;
    case 34: 
      newd->level = LVL_IMPL;
      break;
  }
  if (newd->player_specials_saved.olc_zone == 0)
    newd->player_specials_saved.olc_zone = -1;
  printf("Done ");
}

void convert_plr(char *filename)
{
  FILE *fp;
  FILE *outfile;
  int i, size, recs;

  if (!(fp = fopen(filename, "rb"))) {
    printf("Can't open %s.", filename);
    exit(0);
  }
  
  outfile = fopen("players.new", "wb");
  
  
  fseek(fp, 0L, SEEK_END); // go to the end of the file
    size = ftell(fp); // tell us where we are (at the end, so file size)
    recs = size / sizeof(OldData);
    rewind(fp);

    if (size % sizeof(OldData)) {
      printf("Bad record count for old player file.\n");
      exit(1);
    }
    printf("size = %d, recs = %d\r\n", size, recs);
    printf("Converting: \r\n");
    
    for (size=0; size<recs; size++) {
      printf("%3d: Read ", size);
      
      fread(&OldBuf, sizeof(OldData), 1, fp);
      printf("Name: '%s' ", OldBuf.name);
      ConvertData(&OldBuf, &NewBuf);
      printf("Write ");
      printf("\r\n");
      fwrite(&NewBuf, sizeof(NewData), 1, outfile);
      
    }
    
  
  
  fclose(outfile);
  fclose(fp);
}



int main(int argc, char *argv[])
{
  printf("System Status:\r\n");
  printf("sizeof(char_file_u) = %d\r\n", sizeof(NewData));
  printf("sizeof(old_char_file_u) = %d\r\n", sizeof(OldData));
  printf("sizeof(long) = %d\r\n", sizeof(long));
  printf("sizeof(char) = %d\r\n", sizeof(char));
  
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    convert_plr(argv[1]);
  printf("\r\n");
  return 0;
}
