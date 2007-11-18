 #ifndef _CEDIT_H_
 #define _CEDIT_H_

 void cedit_disp_hometowns(struct descriptor_data *d);
 void cedit_disp_menu(struct descriptor_data * d);
 void cedit_free_clan(struct clan_type *cptr);
 void cedit_parse(struct descriptor_data *d, char *arg);
 void cedit_setup_new(struct descriptor_data *d);

 #endif
