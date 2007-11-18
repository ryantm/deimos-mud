 /* ***********************************************************************
  * File: clan.c                 From the Guild patch by Sean Mountcastle *
  * Usage: Main source code for clans.                                    *
  * Turned into its own file by Catherine Gore (Cheron)          May 2001 *
  *                                                                       *
  * For use with CircleMUD 3.0bpl17/OasisOLC 2.0 and ascii pfiles         *
  *********************************************************************** */

 #include "conf.h"
 #include "sysdep.h"

 #include "structs.h"
 #include "db.h"
 #include "utils.h"
 #include "interpreter.h"
 #include "constants.h"
 #include "screen.h"
 #include "comm.h"
 #include "handler.h"
 #include "clan.h"
 #include "pfdefaults.h"

 /* clan globals */
 extern struct room_data *world;
 struct clan_type *clan_info = NULL;    /* queue of clans  */
 int    cnum  = 0;                      /* number of clans */
 int  newclan = 0; 

 /* external functions */
 long asciiflag_conv(char *);
 void sprintbits(long bits, char *s);
 void store_mail(long to, long from, sh_int vnum, char *message_pointer);

 /* external globals */
 extern struct descriptor_data *descriptor_list;
 extern int top_of_p_table;
 extern struct player_index_element *player_table;

/*local functions */
void remove_member(struct clan_type *cptr, struct char_data *vict );
void remove_applicant(struct clan_type *cptr, struct char_data *vict );

 void free_clan(struct clan_type *c)
 {
   int    i;

   if (c != NULL) {
     free(c->name);
     free(c->leadersname);
     if (c->applicants[0] != NULL) {
       for (i = 0; i < 20; i++) {
         if (c->applicants[i] != NULL) {
           free(c->applicants[i]);
         }
       }
     }
     free(c);
   }
 }

 struct clan_type *enqueue_clan(void)
 {
   struct clan_type *cptr;

   /* This is the first clan loaded if true */
   if (clan_info == NULL) {
     if ((clan_info = (struct clan_type *) malloc(sizeof(struct clan_type))) == NULL) {
       fprintf(stderr, "SYSERR: Out of memory for clans!");
       exit(1);
     } else {
       clan_info->next = NULL;
       return (clan_info);
     }
   } else { /* clan_info is not NULL */
     for (cptr = clan_info; cptr->next != NULL; cptr = cptr->next)
       /* Loop does the work */;
       if ((cptr->next = (struct clan_type *) malloc(sizeof(struct clan_type))) == NULL) {
         fprintf(stderr, "SYSERR: Out of memory for clans!");
         exit(1);
       } else {
         cptr->next->next = NULL;
         return (cptr->next);
       }
   }
   return NULL;
 }

 void dequeue_clan(int clannum)
 {
   struct clan_type *cptr = NULL, *temp;

   if (clannum < 0 || clannum > cnum) {
     log("SYSERR: Attempting to dequeue invalid clan!\r\n");
     exit(1);
   } else {
     if (clan_info->number != clannum) {
       for (cptr = clan_info; cptr->next && cptr->next->number != clannum; cptr = cptr->next)
         ;
       if (cptr->next != NULL && cptr->next->number==clannum) {
         temp       = cptr->next;
         cptr->next = temp->next;
         free_clan(temp);
       }
     } else {
       /* The first one is the one being removed */
       cptr = clan_info;
       clan_info = clan_info->next;
       free_clan(cptr);
     }
   }
 }
 
 /* Puts clans in numerical order */
 void order_clans(void)
 {
   struct clan_type *cptr = NULL, *temp = NULL;

   if (cnum > 2) {
     for (cptr = clan_info; (cptr->next != NULL) && (cptr->next->next != NULL); cptr = cptr->next) {
       if (cptr->next->number > cptr->next->next->number) {
         temp = cptr->next;
         cptr->next = temp->next;
         temp->next = cptr->next->next;
         cptr->next->next = temp;
       }
     }
   }
 
   return;
 }

 /* Loads the clans from the text file */
 void load_clans(void)
 {
   FILE   *fl = NULL;
   int    clannum = 0, line_num = 0, i = 0, tmp, curmem = 0;
   char   name[80];
   char   *ptr;
   struct clan_type *cptr = NULL;
   bool   newc = FALSE;

   if ((fl = fopen(CLAN_FILE, "rt")) == NULL) {
     fprintf(stderr, "SYSERR: Unable to open clan file!");
     exit(0);
   }

   line_num += get_line(fl, buf);
   if (sscanf(buf, "%d", &clannum) != 1) {
     fprintf(stderr, "Format error in clan, line %d (number of clans)\n", line_num);
     exit(0);
   }
   /* Setup the global total number of clans */
   cnum = clannum;
   
   /* process each clan in order */
   for (clannum = 0; clannum < cnum; clannum++) {
     /* Get the info for the individual clans */
     line_num += get_line(fl, buf);
     if (sscanf(buf, "#%d", &tmp) != 1) {
       fprintf(stderr, "Format error in clan (No Unique GID), line %d\n", line_num);
       exit(0);
     }
     /* create some clan shaped memory space */
     if ((cptr = enqueue_clan()) != NULL) {
       cptr->number = tmp;

       /* setup the global number of next new clan number */
       if (!newc) {
         if (cptr->number != clannum) {
           newclan = clannum;
           newc = TRUE;
         }
       }

       if (newc) {
         if (newclan == cptr->number) {
           newclan = cnum;
           newc = FALSE;
         }
       } else
         newclan = cptr->number + 1;
 
       /* allocate space for applicants */
       for (i=0; i< 20; i++)
         cptr->applicants[i] = NULL;

       for (i=0; i< 100; i++)
         cptr->members[i] = NULL;
     
      /* Now get the name of the clan */
       line_num += get_line(fl, buf);
       if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
         *ptr = '\0';
       cptr->name = str_dup(buf);

       /* Now get the look member string */
       line_num += get_line(fl, buf);
       if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
         *ptr = '\0';

       cptr->member_look_str = str_dup(buf);
       /* Now get entrance room and direction and guard */
       line_num += get_line(fl, buf);
       if (sscanf(buf, "%d", &tmp) != 1) {
         fprintf(stderr, "Format error in clan, line %d (entrance room)\n", line_num);
         exit(0);
       }
       cptr->clan_entr_room = tmp;

       /* Skip this line it's just a header */
       line_num += get_line(fl, buf); // password header

       line_num += get_line(fl, buf); 
       if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
         *ptr = '\0';
       cptr->clan_password = str_dup(buf);

       line_num += get_line(fl, buf); // gold header
       
       line_num += get_line(fl, buf); 

       if (sscanf(buf, "%d", &(cptr->gold)) != 1) 
       {
         fprintf(stderr, "Format error in clan (No Gold), line %d\n", line_num);
         exit(0);
       }
        

        
       line_num += get_line(fl, buf); //leader header

       /* Loop to get Members' Names */
       for (;;) 
       {
         line_num += get_line(fl, buf);
         if (*buf == ';')
           break;
         sscanf(buf, "%s %d", name, &tmp);
         if (tmp == CLAN_LEADER) 
           cptr->leadersname = str_dup(name);
       }

       //line_num += get_line(fl, buf); // member Header

       /* Loop to get Members' Names */
       for (;;) 
       {
         line_num += get_line(fl, buf);
         if (*buf == ';')
           break;
         else if ((ptr = strchr(buf, '~')) != NULL)
           *ptr = '\0';
         cptr->members[curmem++] = strdup(buf);   
       }
       curmem = 0;

       // header caught by break statement

       /* Okay we have the leader's name ... now for the applicants */
       for (i = 0; i < 20; i++) {
         line_num += get_line(fl, buf);
         /* We're done when we hit the $ character */
         if (*buf == '$')
           break;
         else if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
           *ptr = '\0';
         cptr->applicants[i] = str_dup(buf);
       }
     } else break;
     /* process the next clan */
   }
   /* done processing clans -- close the file */
   fclose(fl);
 }


 void save_clans(void)
 {
   FILE   *cfile;
   int    clannum = 0, i;
   struct clan_type *cptr = clan_info;

   if (cptr == NULL) {
     fprintf(stderr, "SYSERR: No clans to save!!\n");
     return;
   }
   if ((cfile = fopen(CLAN_FILE, "wt")) == NULL) {
     fprintf(stderr, "SYSERR: Cannot save clans!\n");
     exit(0);
   }

   order_clans();

   /* The total number of clans */
   fprintf(cfile, "%d\r\n", cnum);
   /* Save each clan */
   while (clannum < cnum && cptr != NULL) {
     fprintf(cfile,  "#%d\r\n"
 		    "%s~\r\n"
         "%s~\r\n"
         "%d\r\n"
         "; Password\r\n"
         "%s~\r\n"
         "; Gold\r\n"
         "%d\r\n"
         "; Leader\r\n"
         "%s %d\r\n",
         cptr->number,   cptr->name,
         cptr->member_look_str,
         cptr->clan_entr_room,
         cptr->clan_password,
         cptr->gold,
         cptr->leadersname, CLAN_LEADER);
    fprintf(cfile, "; Members (Max of 100)\r\n");

     if (cptr->members[0] != NULL) 
     {
       for (i = 0; i < 100; i++) {
         if (cptr->members[i] == NULL)
           break;
         else
           fprintf(cfile, "%s~\r\n", cptr->members[i]);
       }
     }

     fprintf(cfile, "; Applicants (Max of 20)\r\n");

     if (cptr->applicants[0] != NULL) {
       for (i = 0; i < 20; i++) {
         if (cptr->applicants[i] == NULL)
           break;
         else
           fprintf(cfile, "%s~\r\n", cptr->applicants[i]);
       }
     }
     fprintf(cfile, "$\r\n");
     /* process the next clan */
     cptr = cptr->next;
     clannum++;
   }
   /* done processing clans */
   fclose(cfile);
 }

 /* Apply to a clan for membership */
 void do_apply( struct char_data *ch, struct clan_type *cptr )
 {
   int i;
   long leader;

   if (IS_NPC(ch))
     return;

   if (!strcmp(cptr->leadersname, "NoOne") ) 
   {
     send_to_char("The clan is currently without a leader.\r\n", ch);
     return;
   }

   for (i = 0; i < 20; i++) {
     if (cptr->applicants[i] == NULL)
       break;
     if (!strcmp(cptr->applicants[i], GET_NAME(ch))) {
       send_to_char("You have already applied to this clan.\r\n", ch);
       return;
     }
   }
     if ((cptr->applicants[i] == NULL) && (i < 20)) {
       cptr->applicants[i] = str_dup(GET_NAME(ch));
       save_clans();
       /* send some sort of mail to leader notifying him/her of application */
       leader = get_id_by_name(cptr->leadersname);
       sprintf(buf, "%s,\r\n\r\n"
         "   Hi there!  I'd like to apply to your clan.  I have some abilities\r\n"
         "that I feel will be useful to you.  Please feel free to contact me \r\n"
         "for any information that you might need about me.\r\n"
         "Hope to hear from you soon!\r\n"
         "\r\n%s\r\n",
         cptr->leadersname, GET_NAME(ch));
       store_mail(leader, GET_IDNUM(ch), -1, buf);
       /* now that the mail has been sent, let's continue. */
       sprintf(buf, "Your application for admittance to %s has been noted.\r\n"
         "You will receive notification when you have been accepted.\r\n",
         cptr->name);
       send_to_char(buf, ch);
     } else
       send_to_char("This clan is already being applied to by 20 other players, try again later.\r\n", ch);
     save_clans();
     return;

 }

 /* Accept an applicant into the clan */
 void do_caccept( struct char_data *ch, struct clan_type *cptr, struct char_data *vict )
 {
   int i=0;

   if (vict != NULL && (GET_CLAN(vict) < CLAN_MEMBER)) {
     GET_CLAN(vict)     = GET_CLAN(ch);
     GET_CLAN_RANK(vict) = CLAN_MEMBER;
     /* send some sort of mail to player notifying him/her of acceptance */
     sprintf(buf, "%s,\r\n\r\n"
       "   Congratulations!  You have been accepted into the ranks of\r\n"
       "%s.  The leader of our clan is %s.  Good luck.\r\n",
       GET_NAME(vict), cptr->name, cptr->leadersname);
     store_mail(GET_IDNUM(vict), GET_IDNUM(ch), -1, buf);
     /* now that the mail has been sent, let's continue. */
     GET_HOME(vict) = GET_HOME(ch);
     save_char(vict, NOWHERE);
     /* now remove the player from the petition list */
     remove_applicant(cptr, vict);

     for (i = 0; i < 100; i++)
     {
       if (cptr->members[i] == NULL)
       {
         cptr->members[i] = strdup(GET_NAME(vict));
         break;
       }
       if (strcmp(cptr->members[i], GET_NAME(vict)) == 0)
         break;  
     }

     save_clans();
   }
   return;
 }

 /* Reject an applicant from a clan/Withdraw an application */
 void do_creject( struct char_data *ch, struct clan_type *cptr, struct char_data *vict)
 {
   int i=0;
   long leader;
 
   if (vict != ch) {
     if (vict != NULL) {
       /* send some sort of mail to player notifying him/her of rejection */
       sprintf(buf, "%s,\r\n\r\n"
         "   You have been rejected from the clan of %s.\r\n"
         "If you are unsure of the reason why, please feel\r\n"
         "free to contact the leader, %s.\r\n",
         GET_NAME(vict), cptr->name, cptr->leadersname);
       store_mail(GET_IDNUM(vict), GET_IDNUM(ch), -1, buf);
       /* mail's been sent, let's move on */
     }
   } else {
       for (i=0; i<20; i++) {
         if (!str_cmp(cptr->applicants[i], GET_NAME(ch))) {
           send_to_char("You aren't applying to the clan...\r\n", ch);
           break;
         }
       }
 
       /* send some sort of mail to leader notifying him/her of withdrawal */
       leader = get_id_by_name(cptr->leadersname);
       sprintf(buf, "%s,\r\n\r\n"
         "   I have withdrawn my application from the clan.\r\n"
         "\r\n%s\r\n",
         cptr->leadersname, GET_NAME(ch));
       store_mail(leader, GET_IDNUM(ch), -1, buf);
       /* now that the mail has been sent, let's continue. */
   }
   GET_CLAN(vict) = CLAN_NONE;
   GET_CLAN_RANK(vict) = 0;
   save_char(vict, NOWHERE);
   /* now remove the player from the petition list */
   remove_applicant(cptr, vict);
   save_clans();
   return;
 }

 /* Dismiss a member from the clan/Resign from the clan */
 void do_dismiss( struct char_data *ch, struct clan_type *cptr, struct char_data *vict )
 {
   if (vict != ch){
     if (vict != NULL && GET_CLAN(vict) == GET_CLAN(ch)) {
       if (GET_CLAN_RANK(vict) >= GET_CLAN_RANK(ch)) {
         send_to_char("You may not dismiss those of equal or greater clan status than yourself!\r\n", ch);
         return;
       }
     /* send some sort of mail to player notifying him/her of dismissal */
     sprintf(buf, "%s,\r\n\r\n"
       "   You have been dismissed from the clan of %s.\r\n"
       "If you are unsure of the reason why, please feel\r\n"
       "free to contact the leader, %s.\r\n",
       GET_NAME(vict), cptr->name, cptr->leadersname);
     store_mail(GET_IDNUM(vict), GET_IDNUM(ch), -1, buf);
     remove_member(cptr, vict);
     }
   } else if (GET_CLAN_RANK(vict) == CLAN_LEADER) {
     GET_CLAN_RANK(vict) = CLAN_MEMBER;
     send_to_char("You have resigned as leader.\r\n", ch);
     cptr->leadersname = strdup("NoOne");
     save_clans();
     return;
   }
   GET_CLAN(vict) = CLAN_NONE;
   GET_CLAN_RANK(vict) = CLAN_NONE;
   GET_HOME(vict) = 1;

   remove_member(cptr, vict);
   save_char(vict, NOWHERE);
   save_clans();
   return;
 }

 /* 
  * Find the char, if not online, load char up. Currently unused. If you can get this to work
  * for you, please send me an update!  Where you get_player_vis in do_clan, substitute this
  * and theoretically you can do anything with the victim offline.
  * Theoretically.  Can also be used in cedit when saving the clan to set leader
  * at clan creation/edit.
  */
 struct char_data *find_clan_char( struct char_data *ch, char *arg )
 {
   struct char_data *vict = NULL, *cbuf = NULL;
 
   if (!(vict = get_player_vis(ch, arg, FIND_CHAR_WORLD))) {
     /* we need to load the file up :( */
    CREATE(cbuf, struct char_data, 1);
     clear_char(cbuf);
     if (load_char(arg, vict) > -1) {
       vict = cbuf;
     }
   }
   return(vict);
 }

 /* Check to see if victim is applying to the clan */
 int app_check( struct clan_type *cptr, struct char_data *applicant )
 {
   int i;

   for (i = 0; i < 20; i++) {
     if (cptr->applicants[i] != NULL)
       if (!strcmp(GET_NAME(applicant), cptr->applicants[i]) || is_abbrev(GET_NAME(applicant), cptr->applicants[i])) {
         return(TRUE);
       }
   }

   return(FALSE);
 }

 /* List members of the clan online */
 void show_clan_who(struct char_data *ch, struct clan_type *clan)
 {
   struct descriptor_data *d;

   sprintf( buf, "\r\n%sMembers of %s online\r\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%s\r\n",
     CCRED(ch, C_NRM), clan->name, CCNRM(ch, C_NRM) );
   send_to_char( buf, ch );
   for ( d = descriptor_list; d; d = d->next ) {
     if (!CAN_SEE(ch, d->character))
        continue;
     if ( !(d->connected) && (GET_CLAN(d->character) == GET_CLAN(ch)) ) {
       sprintf( buf, "%s %s", GET_NAME(d->character), GET_TITLE(d->character) );
       if (GET_CLAN_RANK(d->character) == CLAN_LEADER)
         strcat( buf, " (Leader)\r\n");
       else
         strcat( buf, "&n\r\n");
       send_to_char( buf, ch );
     }
   }

   return;
 }

 /* Send message to all members of clan */
 void perform_ctell( struct char_data *ch, char *message, struct clan_type *clan)
 {
   struct descriptor_data *d;
     for ( d = descriptor_list; d; d = d->next ) {
 	if (d->connected)
 	 continue;
        if (STATE(d) != CON_PLAYING)
         continue;

 	  if (GET_CLAN(d->character) == GET_CLAN(ch) ) {
 		*buf = '\0';
 		sprintf( buf, "&R%s clan tells> &c%s&n\r\n", GET_NAME(ch), message);
 		send_to_char( buf, d->character );
                continue;
 	       }
	  if (PRF_FLAGGED2(d->character, PRF2_HEARALLTELL) &&  GET_LEVEL(d->character) == LVL_IMPL) {
               *buf = '\0';
     	     sprintf( buf, ">> %s%s tells %s%s> %s%s\r\n",
      	           CCYEL(d->character, C_NRM), GET_NAME(ch), clan->name,
                   CCYEL(d->character, C_NRM), message, 
                   CCNRM(d->character, C_NRM));
                send_to_char(buf, d->character);
    	        }
                
 	}

 	return;
 }
 
 /* The main function */
 ACMD(do_clan)
 {
   struct clan_type *cptr = NULL;
   struct char_data *victim = NULL;
   int i, amount;
   bool app = FALSE;
   int player_i = 0;
   struct char_data *cbuf = NULL;


   if (IS_NPC(ch))
     return;

   if ( (GET_CLAN(ch) == CLAN_NONE || GET_CLAN(ch) == CLAN_UNDEFINED) && (subcmd != SCMD_CLAN_APPLY) && (subcmd != SCMD_CLAN_WITHDRAW) && (subcmd != SCMD_CLAN_LIST)) {
     send_to_char("You don't belong to any clan.\r\n", ch);
     return;
   }
   
   for (cptr = clan_info; cptr && cptr->number != GET_CLAN(ch); cptr = cptr->next);
   
   if (cptr == NULL) {
     send_to_char("That clan does not exist.\r\n", ch);
     return;
   }

   switch(subcmd){
 /***cwho ***/
   case SCMD_CLAN_WHO:
     show_clan_who(ch, cptr);
     break;
 /*** ctell ***/
   case SCMD_CLAN_TELL:
     skip_spaces(&argument);
     if (!*argument)
       send_to_char("Yes.. but what do you want to ctell?\r\n", ch);
     else
       perform_ctell( ch, argument, cptr);
     break;
 /*** capply ***/
   case SCMD_CLAN_APPLY:
     if (GET_CLAN(ch) >= CLAN_MEMBER) {
       send_to_char("You already belong to a clan.\r\n", ch);
       return;
     }
     skip_spaces(&argument);
     if (!*argument) {
       send_to_char("Which clan do you want to apply to for membership?\r\n", ch);
       for (cptr = clan_info; cptr; cptr = cptr->next) {
         if (cptr->number == 0) continue;
         sprintf(buf, "%2d %s\r\n", cptr->number, cptr->name);
         send_to_char(buf, ch);
       }
       return;
     }
     i = atoi(argument);
     for (cptr = clan_info; cptr && cptr->number != i; cptr = cptr->next);
     if (cptr == NULL) {
       send_to_char("That clan does not exist.\r\n", ch);
       return;
     }
     do_apply( ch, cptr );
     break;
 /*** creject ***/
   case SCMD_CLAN_REJECT:
     if (GET_CLAN_RANK(ch) < CLAN_LEADER) {
       send_to_char("Only the clan leader can reject applicants.\r\n", ch);
       return;
     }
     skip_spaces(&argument);
     if (!*argument) {
       send_to_char("Current applicants are:\r\n", ch);
       for (i = 0; i < 20; i++) {
         if (cptr->applicants[i] != NULL) {
           sprintf(buf, "%s\r\n", cptr->applicants[i]);
           send_to_char(buf, ch);
           app = TRUE;
         }
       }
       if (!app)
         send_to_char("None.\r\n", ch);
       send_to_char("Whom do you wish to reject from the clan?\r\n", ch);
       return;
     }
     if (!(victim = get_player_vis(ch, argument, FIND_CHAR_WORLD))) {
          CREATE(cbuf, struct char_data, 1);
           clear_char(cbuf);
          CREATE(cbuf->player_specials, struct player_special_data, 1);
          if ((player_i = load_char(argument, cbuf)) > -1) 
          {
             victim = cbuf;
          } 
          else 
          {
            free(cbuf);
            send_to_char("There is no such player.\r\n", ch);
            return;
          }

       if (!app_check(cptr, victim)) {
         send_to_char("&RThere is no one applying your clan by that name.&n\r\n", ch);
         return;
       } 
       do_creject( ch, cptr, victim );
       send_to_char("&GRejected.&n\r\n", ch);

      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf, GET_LOADROOM(cbuf));
      free_char(cbuf);
     } else {
       if (!app_check(cptr, victim)) {
         send_to_char("&RThere is no one applying your clan by that name.&n\r\n", ch);
         return;
       } 
       do_creject( ch, cptr, victim );
       send_to_char("Rejected.\r\n", ch);
     }
     break;
 /*** cwithdraw ***/
   case SCMD_CLAN_WITHDRAW:
     skip_spaces(&argument);
     if (!*argument) {
       send_to_char("Withdraw application to which clan?\r\n", ch);
       return;
     }
     if (!is_number(argument)) {
       send_to_char("You cannot withdraw someone else's application.\r\n", ch);
     }
     for (cptr = clan_info; cptr && cptr->number != atoi(argument); cptr = cptr->next);
     if (cptr == NULL) {
       send_to_char("That clan does not exist.\r\n", ch);
       return;
     }
     do_creject( ch, cptr, ch );
     return;
     break;
 /*** caccept ***/
   case SCMD_CLAN_ACCEPT: 
     if (GET_CLAN_RANK(ch) < CLAN_LEADER) {
       send_to_char("Only the clan leader can accept new members!\r\n", ch);
       return;
     }
     skip_spaces(&argument);
     if (!*argument) {
       send_to_char("Current applicants are:\r\n", ch);
       for (i = 0; i < 20; i++) {
         if (cptr->applicants[i] != NULL) {
           sprintf(buf, "%s\r\n", cptr->applicants[i]);
           send_to_char(buf, ch);
           app = TRUE;
         }
       }
       if (!app)
         send_to_char("None.\r\n", ch);
       send_to_char("Whom do you wish to accept into the clan?\r\n", ch);
       return;
     }
     if (!(victim = get_player_vis(ch, argument, FIND_CHAR_WORLD))) 
     {
          CREATE(cbuf, struct char_data, 1);
           clear_char(cbuf);
          CREATE(cbuf->player_specials, struct player_special_data, 1);
          if ((player_i = load_char(argument, cbuf)) > -1) 
          {
             victim = cbuf;
          } 
          else 
          {
            free(cbuf);
            send_to_char("There is no such player.\r\n", ch);
            return;
          }

       if (!app_check(cptr, victim))
         return;
       do_caccept( ch, cptr, victim );
       send_to_char("New member accepted.\r\n", ch);

      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf, GET_LOADROOM(cbuf));
      send_to_char("Saved in file.\r\n", ch);
      free_char(cbuf);
     } else {
       if (!app_check(cptr, victim))
         return;
       do_caccept( ch, cptr, victim );
       send_to_char("New member accepted.\r\n", ch);
     }
     break;
 /*** cdismiss ***/
   case SCMD_CLAN_DISMISS:
       if (GET_CLAN_RANK(ch) < CLAN_LEADER) {
         send_to_char("Only the clan leader can dismiss members.\r\n", ch);
         return;
       }
       skip_spaces(&argument);
       if (!*argument) {
         send_to_char("Whom do you wish to dismiss?\r\n", ch);
         return;
       }
       if (!(victim = get_player_vis(ch, argument, FIND_CHAR_WORLD))) {
          CREATE(cbuf, struct char_data, 1);
           clear_char(cbuf);
          CREATE(cbuf->player_specials, struct player_special_data, 1);
          if ((player_i = load_char(argument, cbuf)) > -1) 
          {
             victim = cbuf;
          } 
          else 
          {
            free(cbuf);
            send_to_char("There is no such player.\r\n", ch);
            return;
          }
       if (GET_CLAN(victim) != GET_CLAN(ch))
       {
          send_to_char("&RThey are not in this clan!&n\r\n", ch);
          return;
       }
         
       do_dismiss( ch, cptr, victim );
       send_to_char("&GDismissed.&n\r\n", ch);

      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf, GET_LOADROOM(cbuf));
      free_char(cbuf);
       } else {

        if (GET_CLAN(victim) != GET_CLAN(ch))
        {
           send_to_char("&RThey are not in this clan!&n\r\n", ch);
           return;
        }
         do_dismiss( ch, cptr, victim );
         send_to_char("&GDismissed.&n\r\n", ch);
       }
       break;
 /*** cresign ***/
   case SCMD_CLAN_RESIGN:  
     skip_spaces(&argument);
     if (*argument) {
       send_to_char("You cannot force other members to resign.\r\n", ch);
       return;
     }
     if (GET_CLAN_RANK(ch) == CLAN_LEADER)
       perform_ctell(ch, "I am resigning as leader.\r\n", cptr);
     else
       perform_ctell(ch, "I am resigning from the clan.\r\n", cptr);
     do_dismiss( ch, cptr, ch );
     break;
   case SCMD_CLAN_LIST:
       send_to_char("&RBelow is a list of Clans.&n\r\n", ch);
       
       for (cptr = clan_info; cptr; cptr = cptr->next) 
       {
         if (cptr->number == 0) continue;
         sprintf(buf, "%2d %10s %10d&n\r\n", cptr->number, cptr->name, cptr->gold);
         send_to_char(buf, ch);
       }
   break;

 /*** password changing ***/
   case SCMD_CLAN_PASSWORD: 
      if (GET_CLAN_RANK(ch) < CLAN_LEADER) 
      {
        send_to_char("&ROnly the clan leader can change the password!&n\r\n", ch);
        return;
      }
      skip_spaces(&argument);
      if (!*argument) 
      {
        sprintf(buf, "&RWe need a password to set a password, The password currently is: %s.&n\r\n", cptr->clan_password);
       send_to_char(buf, ch);
        return;
      }
 
      cptr->clan_password = strdup(argument);
      sprintf(buf, "&GThe new password is %s.&n\r\n", cptr->clan_password);
      send_to_char(buf, ch);
      save_clans();
   break;
   /*** Display Members ***/
   case SCMD_CLAN_MEMBER:
       app = FALSE;
       sprintf(buf,"&WMembers of %s&n.\r\n", cptr->name);
       send_to_char(buf, ch);

       for (i = 0; i < 100; i++) 
       {
         if (cptr->members[i] != NULL) 
         {
           sprintf(buf, "%-2d: %s\r\n", i, cptr->members[i]);
           send_to_char(buf, ch);
           app = TRUE;
         }
         else break;
       }
       if (!app)
       {
         send_to_char("&RNo members found!&n\r\n", ch);
       }    

   break;
   case SCMD_CLAN_BALANCE:
     sprintf(buf, "&WThe bank of %s, has: &Y%d&W Coins Deposited.&n\r\n", cptr->name, cptr->gold);
     send_to_char(buf, ch);
   break;
   case SCMD_CLAN_DEPOSIT:
    if (((cptr->number == 1 && GET_ROOM_VNUM(ch->in_room) == 1541) //WHY
        || (cptr->number == 2 && GET_ROOM_VNUM(ch->in_room) == 1542) //GOD
        || (cptr->number == 3 && GET_ROOM_VNUM(ch->in_room) == 1544))) //OOC
    {

     if ((amount = atoi(argument)) <= 0) 
     {
        send_to_char("&WHow much do you want to deposit?&n\r\n", ch);
        return;
     }
     if (GET_GOLD(ch) < amount) 
     {
       send_to_char("&WYou don't have that many coins!&n\r\n", ch);
       return;
     }
     GET_GOLD(ch) -= amount;
     cptr->gold += amount;
     sprintf(buf, "&WYou deposit&n &Y%d&n &Wcoins.&n\r\n", amount);
     send_to_char(buf, ch);
     save_clans();
     return;     
   }
   else
   {
      send_to_char("&WYou can only complete a deposit in your clan hall.&n\r\n",ch);
      return;
   }

   break;
   case SCMD_CLAN_TAKEOUT:

    if (((cptr->number == 1 && GET_ROOM_VNUM(ch->in_room) == 1541) //WHY
        || (cptr->number == 2 && GET_ROOM_VNUM(ch->in_room) == 1542) //GOD
        || (cptr->number == 3 && GET_ROOM_VNUM(ch->in_room) == 1544)) ) //OOC
    {

    if ((amount = atoi(argument)) <= 0) 
    {
      send_to_char("&WHow much do you want to withdraw?&n\r\n", ch);
      return;
    }
    if (cptr->gold < amount) 
    {
      send_to_char("&WThere isn't that many coins deposited!&n\r\n", ch);
      return;
    }
    GET_GOLD(ch) += amount;
    cptr->gold -= amount;
    sprintf(buf, "&WYou withdraw&n &Y%d&n &Wcoins.&n\r\n", amount);
    send_to_char(buf, ch);
    save_clans();
    return;
   }
   else
   {
      send_to_char("&WYou can only complete a deposit in your clan hall.&n\r\n",ch);
      return;
   }

   break;
   default:
     return;
   }
 }


char *get_clan_name(struct char_data *ch){
   struct clan_type *cptr = NULL;
   
   if (IS_NPC(ch))
     return "Not PC";

   if ( (GET_CLAN(ch) == CLAN_NONE || GET_CLAN(ch) == CLAN_UNDEFINED)) {
     return "None";
   }
   
   for (cptr = clan_info; cptr && cptr->number != GET_CLAN(ch); cptr = cptr->next);
   
   if (cptr == NULL) {
     send_to_char("That clan does not exist.\r\n", ch);
     return "None";
   }
   return cptr->name;
}
char *get_clan_abbrev(struct char_data *ch){
   struct clan_type *cptr = NULL;
   
   if (IS_NPC(ch))
     return "Not PC";

   if ( (GET_CLAN(ch) == CLAN_NONE || GET_CLAN(ch) == CLAN_UNDEFINED)) {
     return "";
   }
   
   for (cptr = clan_info; cptr && cptr->number != GET_CLAN(ch); cptr = cptr->next);
   
   if (cptr == NULL) {
     send_to_char("That clan does not exist.\r\n", ch);
     return "";
   }
   return cptr->member_look_str ? cptr->member_look_str: cptr->name;
}

void remove_member(struct clan_type *cptr, struct char_data *vict )
{
     int i = 0; bool deleted = FALSE;

     for (i = 0; i < 100; i++) 
     {
       if (cptr->members[i]) 
       {
         if (!strcmp(GET_NAME(vict), cptr->members[i]))
         {
           free(cptr->members[i]);
           cptr->members[i] = NULL;
           deleted = TRUE;
         }
         if (deleted)
         {
           cptr->members[i] = cptr->members[i+1];
         }
       }
     }
}

void remove_applicant(struct clan_type *cptr, struct char_data *vict )
{
     int i = 0; bool deleted = FALSE;

     for (i = 0; i < 20; i++) 
     {
       if (cptr->applicants[i]) 
       {
         if (!strcmp(GET_NAME(vict), cptr->applicants[i]))
         {
           free(cptr->applicants[i]);
           cptr->applicants[i] = NULL;
           deleted = TRUE;
         }
         if (deleted)
         {
           cptr->applicants[i] = cptr->applicants[i+1];
         }
       }
     }
}

