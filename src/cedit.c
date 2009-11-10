  /* ************************************************************
  * File: cedit.c     From the guild patch by Sean Mountcastle *
  * Usage: For editing clan information               May 2001 *
  * Made into seperate file by Catherine Gore                  *
  *                                                            *
  * For use with CircleMUD 3.0/OasisOLC 2.0                    *
  ************************************************************ */

 #include "conf.h"
 #include "sysdep.h"

 #include "structs.h"
 #include "utils.h"
 #include "comm.h"
 #include "interpreter.h"
 #include "db.h"
 #include "genolc.h"
 #include "oasis.h"
 #include "improved-edit.h"
 #include "dg_olc.h"
 #include "constants.h"
 #include "clan.h"
 #include "cedit.h"

 /* external globals */
 extern const sh_int mortal_start_room;
 extern struct index_data *mob_index;
 extern struct char_data *mob_proto;
 extern struct room_data *world;
 extern int cnum;
 extern int newclan;

 /* external functions */
 extern struct char_data *find_clan_char(struct char_data *ch, char *arg);

 void cedit_disp_menu(struct descriptor_data * d)
 {
 
   if (!OLC_CLAN(d))
     cedit_setup_new(d);

   get_char_colors(d->character);

   sprintf(buf,
     "-- Clan number : [%s%d%s]\r\n"
     "%s1%s) Name        : %s%s\r\n"
     "%s2%s) Leader      : %s%s\r\n"
     "%s4%s) Clan Abbrev : %s%s\r\n"
     "%sP%s) Purge this Clan\r\n"
     "%sQ%s) Quit\r\n"
     "Enter choice : ",
     
     cyn, OLC_CLAN(d)->number, nrm,
     grn, nrm, yel, OLC_CLAN(d)->name,
     grn, nrm, yel, OLC_CLAN(d)->leadersname,
     grn, nrm, yel, OLC_CLAN(d)->member_look_str ?
     OLC_CLAN(d)->member_look_str : OLC_CLAN(d)->name,
     grn, nrm,
     grn, nrm
     );
   send_to_char(buf, d->character);

   OLC_MODE(d) = CEDIT_MAIN_MENU;
 }
 

 void cedit_free_clan(struct clan_type *cptr)
 {
   dequeue_clan(cptr->number);
 }
 
 void cedit_parse(struct descriptor_data *d, char *arg)
 {
   switch (OLC_MODE(d)) {
   case CEDIT_CONFIRM_SAVE:
     switch (*arg) {
     case 'y':
     case 'Y':
       save_clans();
       sprintf(buf, "OLC: %s edits clan %d", GET_NAME(d->character), OLC_CLAN(d)->number);
       mudlog(buf, CMP, LVL_GOD, TRUE);
       send_to_char("Clan saved to disk.\r\n", d->character);
       cleanup_olc(d, CLEANUP_STRUCTS);
       break;
     case 'n':
     case 'N':
       /* free everything up, including strings etc */
       cleanup_olc(d, CLEANUP_STRUCTS);
       break;
     default:
       send_to_char("Invalid choice!\r\nDo you wish to save your changes? : ", d->character);
       break;
     }
     return;

     case CEDIT_MAIN_MENU:
       switch (*arg) {
       case 'q':
       case 'Q':
         if (OLC_VAL(d)) {
           /*. Something has been modified .*/
           send_to_char("Do you wish to save your changes? : ", d->character);
           OLC_MODE(d) = CEDIT_CONFIRM_SAVE;
         } else
           cleanup_olc(d, CLEANUP_STRUCTS);
         return;
       case '1':
         send_to_char("Enter clan name:-\r\n| ", d->character);
         OLC_MODE(d) = CEDIT_NAME;
         break;
       case '2':
         send_to_char("Enter clan leader's name: ", d->character);
         OLC_MODE(d) = CEDIT_LEADERSNAME;
         break;
       case '4':
         send_to_char("Enter clan member who string:-\r\n| ", d->character);
         OLC_MODE(d) = CEDIT_MBR_LOOK_STR;
         break;
       case 'p':
       case 'P':
         if (GET_LEVEL(d->character) >= LVL_IMPL) {
           newclan = OLC_CLAN(d)->number;  /* next new clan will get this one's number  */
           /* free everything up, including strings etc */
           cleanup_olc(d, CLEANUP_ALL);
           cnum--;
           send_to_char("Clan purged.\r\n", d->character);
         } else {
           send_to_char("Sorry you are not allowed to do that at this time.\r\n",d->character);
           cedit_disp_menu(d);
         }
         return;
       default:
         send_to_char("Invalid choice!", d->character);
         cedit_disp_menu(d);
         break;
       }
       return;

       case CEDIT_NAME:
         if (OLC_CLAN(d)->name)
           free(OLC_CLAN(d)->name);
         OLC_CLAN(d)->name = str_dup(arg);
         break;

       case CEDIT_LEADERSNAME:
         if (OLC_CLAN(d)->leadersname)
           free(OLC_CLAN(d)->leadersname);
         OLC_CLAN(d)->leadersname = str_dup(arg);
         break;

       case CEDIT_MBR_LOOK_STR:
         if (OLC_CLAN(d)->member_look_str)
           free(OLC_CLAN(d)->member_look_str);
         OLC_CLAN(d)->member_look_str = str_dup(arg);
         break;
         
       default:
         /* we should never get here */
         mudlog("SYSERR: Reached default case in cedit_parse",BRF,LVL_GOD,TRUE);
         break;
    }
    /* If we get this far, something has been changed */
    OLC_VAL(d) = 1;
    cedit_disp_menu(d);
 }
 
 /*
 * Create a new clan with some default strings.
 */
 void cedit_setup_new(struct descriptor_data *d)
 {
   int i;

   if ((OLC_CLAN(d) = enqueue_clan()) != NULL) {
     OLC_CLAN(d)->name = str_dup("Unfinished Clan");
     OLC_CLAN(d)->number = newclan;
     OLC_CLAN(d)->leadersname = str_dup("NoOne");
     OLC_CLAN(d)->member_look_str = NULL;
     OLC_CLAN(d)->clan_entr_room = 1;
     OLC_CLAN(d)->clan_password = str_dup("none");
     OLC_CLAN(d)->gold = 0;
     for (i=0; i<20; i++)
       OLC_CLAN(d)->applicants[i] = NULL;
   } else
     fprintf(stderr, "SYSERR: Unable to create new clan!\r\n");
   cnum++;
   if (newclan == cnum)
     newclan++;
   else
     newclan = cnum;
   cedit_disp_menu(d);
   OLC_VAL(d) = 0;
 }
