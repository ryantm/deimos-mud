/* ************************************************************************
*   File: Guild.h                                                         *
*  Usage: GuildMaster's: Definition for everything needed by shop.c       *
*                                                                         *
* Written by Jason Goodwin.   jgoodwin@expert.cc.purdue.edu               *
************************************************************************ */

#ifndef __GUILD_H__
#define __GUILD_H__
 
struct guild_master_data {
int num;             			/* number of the guild */
int skills_and_spells[MAX_SKILLS+1];  	/*array to keep track of which things we'll train */
float charge;             		/* charge * skill level = how much we'll charge */
char *no_such_skill;  			/* message when we don't teach that skill */
char *not_enough_gold;  		/* message when the student doesn't have enough gold */
int type;   				/* does GM teach skills or spells? */
int gm;    				/* GM's rnum (converted to vnum when saving) */
int with_who;  				/* whom we dislike */
int open, close;  			/* when we will train */
SPECIAL(*func);  			/* secondary spec_proc for the GM */
};

/* Whom will we not train  */
#define TRAIN_NOGOOD		(1 << 0)
#define TRAIN_NOEVIL		(1 << 1)
#define TRAIN_NONEUTRAL		(1 << 2)
#define TRAIN_NOMAGE		(1 << 3)
#define TRAIN_NOCLERIC		(1 << 4)
#define TRAIN_NOWARRIOR		(1 << 5)
#define TRAIN_NOTHIEF		(1 << 6)
#define TRAIN_NOPALADIN		(1 << 7)
#define TRAIN_NORANGER		(1 << 8)
#define TRAIN_NOWARLOCK		(1 << 9)
#define TRAIN_NOCYBORG		(1 << 10)
#define TRAIN_NONECROMANCER	(1 << 11)
#define TRAIN_NODRUID		(1 << 12)
#define TRAIN_NOALCHEMIST	(1 << 13)
#define TRAIN_NOBARBARIAN	(1 << 14)

#define GM_NUM(i)  (gm_index[i].num)
#define GM_CHARGE(i) (gm_index[i].charge)
#define GM_TYPE(i) (gm_index[i].type)
#define GM_TRAINER(i) (gm_index[i].gm)
#define GM_WITH_WHO(i) (gm_index[i].with_who)
#define GM_OPEN(i) (gm_index[i].open)
#define GM_CLOSE(i) (gm_index[i].close)
#define GM_FUNC(i) (gm_index[i].func)
#define GM_COST(i,j,k) (int) (GM_CHARGE(i)*\
                              spell_info[j].min_level[(int)GET_CLASS(k)])

#define NOTRAIN_GOOD(i)		(IS_SET((GM_WITH_WHO(i)),TRAIN_NOGOOD))
#define NOTRAIN_EVIL(i)		(IS_SET((GM_WITH_WHO(i)),TRAIN_NOEVIL))
#define NOTRAIN_NEUTRAL(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NONEUTRAL))
#define NOTRAIN_MAGE(i)		(IS_SET((GM_WITH_WHO(i)),TRAIN_NOMAGE))
#define NOTRAIN_CLERIC(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NOCLERIC))
#define NOTRAIN_THIEF(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NOTHIEF))
#define NOTRAIN_WARRIOR(i)	(IS_SET((GM_WITH_WHO(i)), TRAIN_NOWARRIOR))
#define NOTRAIN_PALADIN(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NOPALADIN))
#define NOTRAIN_RANGER(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NORANGER))
#define NOTRAIN_WARLOCK(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NOWARLOCK))
#define NOTRAIN_CYBORG(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NOCYBORG))
#define NOTRAIN_NECROMANCER(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NONECROMANCER))
#define NOTRAIN_DRUID(i)	(IS_SET((GM_WITH_WHO(i)),TRAIN_NODRUID))
#define NOTRAIN_ALCHEMIST(i)	(IS_SET((GM_WITH_WHO(i)), TRAIN_NOALCHEMIST))
#define NOTRAIN_BARBARIAN(i)	(IS_SET((GM_WITH_WHO(i)), TRAIN_NOBARBARIAN))

#define GM_SPLSKL(i) (GM_TYPE(i) ?  "spell" : "skill")

#define MSG_TRAINER_NOT_OPEN 	 "I'm busy! Come back later!"
#define MSG_TRAINER_NO_SEE_CH 	 "I don't train someone I can't see!"
#define MSG_TRAINER_DISLIKE_ALIGN  "Get out of here before I get angry!"
#define MSG_TRAINER_DISLIKE_CLASS "I don't like your kind!"

void free_gm(struct guild_master_data *guild);
int real_gm(int vnum);

#endif