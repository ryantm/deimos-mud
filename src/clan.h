 #ifndef _CLAN_H_
 #define _CLAN_H_

 /* internal functions used in clan.c */
 void free_clan(struct clan_type *c);
 struct clan_type *enqueue_clan(void);
 void dequeue_clan(int clannum);
 void load_clans(void);
 void save_clans(void);
 void do_apply( struct char_data *ch, struct clan_type *cptr );
 void do_caccept( struct char_data *ch, struct clan_type *cptr, struct char_data *vict );
 void do_creject( struct char_data *ch, struct clan_type *cptr, struct char_data *vict );
 void do_cdismiss( struct char_data *ch, struct clan_type *cptr, struct char_data *vict);
 struct char_data *find_clan_char(struct char_data *ch, char *arg);
 int app_check( struct clan_type *cptr, struct char_data *applicant );
 void show_clan_who(struct char_data *ch, struct clan_type *clan);
 void perform_ctell(struct char_data *ch, char *message, struct clan_type *clan);
 
 /* ACMD's */
 ACMD(do_clan);

 #endif
