 /***************************************************************************
  * MOBProgram ported for CircleMUD 3.0 by Mattias Larsson		   *
  * Traveller@AnotherWorld (ml@eniac.campus.luth.se 4000) 		   *
  **************************************************************************/
 
 /***************************************************************************
  *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
  *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
  *                                                                         *
  *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
  *  Chastain, Michael Quan, and Mitchell Tse.                              *
  *                                                                         *
  *  In order to use any part of this Merc Diku Mud, you must comply with   *
  *  both the original Diku license in 'license.doc' as well the Merc       *
  *  license in 'license.txt'.  In particular, you may not remove either of *
  *  these copyright notices.                                               *
  *                                                                         *
  *  Much time and thought has gone into this software and you are          *
  *  benefitting.  We hope that you share your changes too.  What goes      *
  *  around, comes around.                                                  *
  ***************************************************************************/
 
 /***************************************************************************
  *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
  *  these routines should not be expected from Merc Industries.  However,  *
  *  under no circumstances should the blame for bugs, etc be placed on     *
  *  Merc Industries.  They are not guaranteed to work on all systems due   *
  *  to their frequent use of strxxx functions.  They are also not the most *
  *  efficient way to perform their tasks, but hopefully should be in the   *
  *  easiest possible way to install and begin using. Documentation for     *
  *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
  ***************************************************************************/
 
 #include "conf.h"
 #include "sysdep.h"
 #include "structs.h"
 #include "utils.h"
 #include "interpreter.h"
 #include "handler.h"
 #include "db.h"
 
 char buf2[MAX_STRING_LENGTH];
 
 extern struct index_data *mob_index;
 extern struct index_data *obj_index;
 extern struct room_data *world;
 
 extern void death_cry (struct char_data *ch);
 extern bool str_prefix (const char *astr, const char *bstr);
 extern int number_percent(void);
 extern int number_range(int from, int to);
 
 #define bug(x, y) { sprintf(buf2, (x), (y)); log(buf2); }
 
 
 /*
  * Local function prototypes
  */
 
 char *	mprog_next_command	(char* clist);
 int     mprog_seval		(char *lhs, char *opr, char *rhs);
 int	mprog_veval		(int lhs, char* opr, int rhs);
 int	mprog_do_ifchck		(char* ifchck, struct char_data* mob,
 				       struct char_data* actor, struct obj_data* obj,
 				       void* vo, struct char_data* rndm);
 char *	mprog_process_if	(char* ifchck, char* com_list, 
 				       struct char_data* mob, struct char_data* actor,
 				       struct obj_data* obj, void* vo,
 				       struct char_data* rndm);
 void	mprog_translate		(char ch, char* t, struct char_data* mob,
 				       struct char_data* actor, struct obj_data* obj,
 				       void* vo, struct char_data* rndm);
 void	mprog_process_cmnd	(char* cmnd, struct char_data* mob, 
 				       struct char_data* actor, struct obj_data* obj,
 				       void* vo, struct char_data* rndm);
 void	mprog_driver		(char* com_list, struct char_data* mob,
 				       struct char_data* actor, struct obj_data* obj,
 				       void* vo);
 
 /***************************************************************************
  * Local function code and brief comments.
  */
 
 /* Used to get sequential lines of a multi line string (separated by "\n\r")
  * Thus its like one_argument(), but a trifle different. It is destructive
  * to the multi line string argument, and thus clist must not be shared.
  */
 
 char *mprog_next_command(char *clist)
 {
 
   char *pointer = clist;
 
   if (*pointer == '\r')
     pointer++;  
   if (*pointer == '\n')
     pointer++;
 
   while (*pointer != '\n' && *pointer != '\0' && *pointer != '\r')
     pointer++;
   if (*pointer == '\n') {
     *pointer = '\0';
     pointer++; }
   if (*pointer == '\r') {
     *pointer = '\0';
     pointer++; }
     
   return (pointer);
 
 }
 
 /* we need str_infix here because strstr is not case insensitive */
 
 bool str_infix(const char *astr, const char *bstr)
 {
     int sstr1;
     int sstr2;
     int ichar;
     char c0;
 
     if ((c0 = LOWER(astr[0])) == '\0')
         return FALSE;
 
     sstr1 = strlen(astr);
     sstr2 = strlen(bstr);
 
     for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
         if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
             return FALSE;
     }
 
     return TRUE;
 }
 
 /* These two functions do the basic evaluation of ifcheck operators.
  *  It is important to note that the string operations are not what
  *  you probably expect.  Equality is exact and division is substring.
  *  remember that lhs has been stripped of leading space, but can
  *  still have trailing spaces so be careful when editing since:
  *  "guard" and "guard " are not equal.
  */
 int mprog_seval(char *lhs, char *opr, char *rhs)
 {
 
   if (!str_cmp(opr, "=="))
     return (!str_cmp(lhs, rhs));
   if (!str_cmp(opr, "!="))
     return (str_cmp(lhs, rhs));
   if (!str_cmp(opr, "/"))
     return (!str_infix(rhs, lhs));
   if (!str_cmp(opr, "!/"))
     return (str_infix(rhs, lhs));
 
   log("Improper MOBprog operator");
   return 0;
 
 }
 
 int mprog_veval(int lhs, char *opr, int rhs)
 {
 
   if (!str_cmp(opr, "=="))
     return (lhs == rhs);
   if (!str_cmp(opr, "!="))
     return (lhs != rhs);
   if (!str_cmp(opr, ">"))
     return (lhs > rhs);
   if (!str_cmp(opr, "<"))
     return (lhs < rhs);
   if (!str_cmp(opr, "<="))
     return (lhs <= rhs);
   if (!str_cmp(opr, ">="))
     return (lhs >= rhs);
   if (!str_cmp(opr, "&"))
     return (lhs & rhs);
   if (!str_cmp(opr, "|"))
     return (lhs | rhs);
 
   log("Improper MOBprog operator");
   return 0;
 
 }
 
 /* This function performs the evaluation of the if checks.  It is
  * here that you can add any ifchecks which you so desire. Hopefully
  * it is clear from what follows how one would go about adding your
  * own. The syntax for an if check is: ifchck (arg) [opr val]
  * where the parenthesis are required and the opr and val fields are
  * optional but if one is there then both must be. The spaces are all
  * optional. The evaluation of the opr expressions is farmed out
  * to reduce the redundancy of the mammoth if statement list.
  * If there are errors, then return -1 otherwise return boolean 1,0
  */
 int mprog_do_ifchck(char *ifchck, struct char_data *mob, struct char_data *actor,
 		     struct obj_data *obj, void *vo, struct char_data *rndm)
 {
 
   char buf[MAX_INPUT_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char opr[MAX_INPUT_LENGTH];
   char val[MAX_INPUT_LENGTH];
   struct char_data *vict = (struct char_data *) vo;
   struct obj_data *v_obj = (struct obj_data  *) vo;
   char     *bufpt = buf;
   char     *argpt = arg;
   char     *oprpt = opr;
   char     *valpt = val;
   char     *point = ifchck;
   int       lhsvl;
   int       rhsvl;
 
   if (*point == '\0') 
     {
       bug ("Mob: %d null ifchck", (int)mob_index[mob->nr].virtual); 
       return -1;
     }   
   /* skip leading spaces */
   while (*point == ' ')
     point++;
 
   /* get whatever comes before the left paren.. ignore spaces */
   while (*point != '(') 
     if (*point == '\0') 
       {
 	bug ("Mob: %d ifchck syntax error", mob_index[mob->nr].virtual); 
 	return -1;
       }   
     else
       if (*point == ' ')
 	point++;
       else 
 	*bufpt++ = *point++; 
 
   *bufpt = '\0';
   point++;
 
   /* get whatever is in between the parens.. ignore spaces */
   while (*point != ')') 
     if (*point == '\0') 
       {
 	bug ("Mob: %d ifchck syntax error", mob_index[mob->nr].virtual); 
 	return -1;
       }   
     else
       if (*point == ' ')
 	point++;
       else 
 	*argpt++ = *point++; 
 
   *argpt = '\0';
   point++;
 
   /* check to see if there is an operator */
   while (*point == ' ')
     point++;
   if (*point == '\0') 
     {
       *opr = '\0';
       *val = '\0';
     }   
   else /* there should be an operator and value, so get them */
     {
       while ((*point != ' ') && (!isalnum(*point))) 
 	if (*point == '\0') 
 	  {
 	    bug ("Mob: %d ifchck operator without value",
 		 mob_index[mob->nr].virtual); 
 	    return -1;
 	  }
 	else
 	  *oprpt++ = *point++; 
 
       *oprpt = '\0';
  
       /* finished with operator, skip spaces and then get the value */
       while (*point == ' ')
 	point++;
       for(;;)
 	{
 	  if ((*point != ' ') && (*point == '\0')) 
 	    break;
 	  else
 	    *valpt++ = *point++; 
 	}
 
       *valpt = '\0';
     }
   bufpt = buf;
   argpt = arg;
   oprpt = opr;
   valpt = val;
 
   /* Ok... now buf contains the ifchck, arg contains the inside of the
    *  parentheses, opr contains an operator if one is present, and val
    *  has the value if an operator was present.
    *  So.. basically use if statements and run over all known ifchecks
    *  Once inside, use the argument and expand the lhs. Then if need be
    *  send the lhs,opr,rhs off to be evaluated.
    */
 
   if (!str_cmp(buf, "rand"))
     {
       return (number(0,100) <= atoi(arg));
     }
 
   if (!str_cmp(buf, "ispc"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return 0;
 	case 'n': if (actor)
  	             return (!IS_NPC(actor));
 	          else return -1;
 	case 't': if (vict)
                      return (!IS_NPC(vict));
 	          else return -1;
 	case 'r': if (rndm) 
                      return (!IS_NPC(rndm));
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'ispc'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isnpc"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return 1;
 	case 'n': if (actor)
 	             return IS_NPC(actor);
 	          else return -1;
 	case 't': if (vict)
                      return IS_NPC(vict);
 	          else return -1;
 	case 'r': if (rndm)
 	             return IS_NPC(rndm);
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'isnpc'",
                mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isgood"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return IS_GOOD(mob);
 	case 'n': if (actor) 
 	          return IS_GOOD(actor);
 	          else { return -1; }
 	case 't': if (vict)
 	             return IS_GOOD(vict);
 	          else return -1;
 	case 'r': if (rndm)
 	             return IS_GOOD(rndm);
 	          else return -1;
 	default:
 	  bug("Mob: %d bad argument to 'isgood'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isfight"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return (FIGHTING(mob)) ? 1 : 0;
 	case 'n': if (actor)
 	             return(FIGHTING(actor)) ? 1 : 0;
 	          else return -1;
 	case 't': if (vict)
 	             return (FIGHTING(vict)) ? 1 : 0;
 	          else return -1;
 	case 'r': if (rndm)
 	             return (FIGHTING(rndm)) ? 1 : 0;
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'isfight'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isimmort"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return (GET_LEVEL(mob) > LVL_IMMORT);
 	case 'n': if (actor)
 	             return (GET_LEVEL(actor) > LVL_IMMORT);
   	          else return -1;
 	case 't': if (vict)
 	             return (GET_LEVEL(vict) > LVL_IMMORT);
                   else return -1;
 	case 'r': if (rndm)
 	             return (GET_LEVEL(rndm) > LVL_IMMORT);
                   else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'isimmort'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "ischarmed"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return IS_AFFECTED(mob, AFF_CHARM);
 	case 'n': if (actor)
 	             return IS_AFFECTED(actor, AFF_CHARM);
 	          else return -1;
 	case 't': if (vict)
 	             return IS_AFFECTED(vict, AFF_CHARM);
 	          else return -1;
 	case 'r': if (rndm)
 	             return IS_AFFECTED(rndm, AFF_CHARM);
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'ischarmed'",
 	       mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isfollow"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return (mob->master != NULL
 			  && mob->master->in_room == mob->in_room);
 	case 'n': if (actor)
 	             return (actor->master != NULL
 			     && actor->master->in_room == actor->in_room);
 	          else return -1;
 	case 't': if (vict)
 	             return (vict->master != NULL
 			     && vict->master->in_room == vict->in_room);
 	          else return -1;
 	case 'r': if (rndm)
 	             return (rndm->master != NULL
 			     && rndm->master->in_room == rndm->in_room);
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'isfollow'", 
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "isaffected"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return (mob->char_specials.saved.affected_by & atoi(arg));
 	case 'n': if (actor)
 	             return (actor->char_specials.saved.affected_by & atoi(arg));
 	          else return -1;
 	case 't': if (vict)
 	             return (vict->char_specials.saved.affected_by & atoi(arg));
 	          else return -1;
 	case 'r': if (rndm)
 	             return (rndm->char_specials.saved.affected_by & atoi(arg));
 	          else return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'isaffected'",
 	       mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "hitprcnt"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->points.hit / mob->points.max_hit;
 	          rhsvl = atoi(val);
          	  return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->points.hit / actor->points.max_hit;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->points.hit / vict->points.max_hit;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->points.hit / rndm->points.max_hit;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'hitprcnt'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "inroom"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->in_room;
 	          rhsvl = real_room(atoi(val));
 	          return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->in_room;
 		    rhsvl = real_room(atoi(val));
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->in_room;
 		    rhsvl = real_room(atoi(val));
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->in_room;
 		    rhsvl = real_room(atoi(val));
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'inroom'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "sex"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->player.sex;
 	          rhsvl = atoi(val);
 	          return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->player.sex;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->player.sex;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->player.sex;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'sex'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "position"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->char_specials.position;
 	          rhsvl = atoi(val);
 	          return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->char_specials.position;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->char_specials.position;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->char_specials.position;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'position'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "level"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = GET_LEVEL(mob);
 	          rhsvl = atoi(val);
 	          return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = GET_LEVEL(actor);
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else 
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = GET_LEVEL(vict);
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = GET_LEVEL(rndm);
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'level'",
                 mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "class"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->player.class;
 	          rhsvl = atoi(val);
                   return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->player.class;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else 
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->player.class;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->player.class;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'class'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "goldamt"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->points.gold;
                   rhsvl = atoi(val);
                   return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    lhsvl = actor->points.gold;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    lhsvl = vict->points.gold;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    lhsvl = rndm->points.gold;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'goldamt'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "objtype"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj->obj_flags.type_flag;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	         else
 		   return -1;
 	case 'p': if (v_obj)
 	          {
 		    lhsvl = v_obj->obj_flags.type_flag;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'objtype'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "objval0"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj->obj_flags.value[0];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'p': if (v_obj)
 	          {
		    lhsvl = v_obj->obj_flags.value[0];
     rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else 
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'objval0'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "objval1"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj->obj_flags.value[1];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'p': if (v_obj)
 	          {
 		    lhsvl = v_obj->obj_flags.value[1];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'objval1'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "objval2"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj->obj_flags.value[2];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'p': if (v_obj)
 	          {
 		    lhsvl = v_obj->obj_flags.value[2];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'objval2'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "objval3"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj->obj_flags.value[3];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'p': if (v_obj) 
 	          {
 		    lhsvl = v_obj->obj_flags.value[3];
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'objval3'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "number"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': lhsvl = mob->points.gold;
 	          rhsvl = atoi(val);
 	          return mprog_veval(lhsvl, opr, rhsvl);
 	case 'n': if (actor)
 	          {
 		    if IS_NPC(actor)
 		    {
 		      lhsvl = mob_index[actor->nr].virtual;
 		      rhsvl = atoi(val);
 		      return mprog_veval(lhsvl, opr, rhsvl);
 		    }
 		  }
 	          else
 		    return -1;
 	case 't': if (vict)
 	          {
 		    if IS_NPC(actor)
 		    {
 		      lhsvl = mob_index[vict->nr].virtual;
 		      rhsvl = atoi(val);
 		      return mprog_veval(lhsvl, opr, rhsvl);
 		    }
 		  }
                   else
 		    return -1;
 	case 'r': if (rndm)
 	          {
 		    if IS_NPC(actor)
 		    {
 		      lhsvl = mob_index[rndm->nr].virtual;
 		      rhsvl = atoi(val);
 		      return mprog_veval(lhsvl, opr, rhsvl);
 		    }
 		  }
 	         else return -1;
 	case 'o': if (obj)
 	          {
 		    lhsvl = obj_index[obj->item_number].virtual;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	case 'p': if (v_obj)
 	          {
 		    lhsvl = obj_index[v_obj->item_number].virtual;
 		    rhsvl = atoi(val);
 		    return mprog_veval(lhsvl, opr, rhsvl);
 		  }
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'number'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   if (!str_cmp(buf, "name"))
     {
       switch (arg[1])  /* arg should be "$*" so just get the letter */
 	{
 	case 'i': return mprog_seval(mob->player.name, opr, val);
 	case 'n': if (actor)
 	            return mprog_seval(actor->player.name, opr, val);
 	          else
 		    return -1;
 	case 't': if (vict)
 	            return mprog_seval(vict->player.name, opr, val);
 	          else
 		    return -1;
 	case 'r': if (rndm)
 	            return mprog_seval(rndm->player.name, opr, val);
 	          else
		    return -1;
 	case 'o': if (obj)
 	            return mprog_seval(obj->name, opr, val);
 	          else
 		    return -1;
 	case 'p': if (v_obj)
 	            return mprog_seval(v_obj->name, opr, val);
 	          else
 		    return -1;
 	default:
 	  bug ("Mob: %d bad argument to 'name'", mob_index[mob->nr].virtual); 
 	  return -1;
 	}
     }
 
   /* Ok... all the ifchcks are done, so if we didnt find ours then something
    * odd happened.  So report the bug and abort the MOBprogram (return error)
    */
   bug ("Mob: %d unknown ifchck", mob_index[mob->nr].virtual); 
   return -1;
 
 }
 /* Quite a long and arduous function, this guy handles the control
  * flow part of MOBprograms.  Basicially once the driver sees an
  * 'if' attention shifts to here.  While many syntax errors are
  * caught, some will still get through due to the handling of break
  * and errors in the same fashion.  The desire to break out of the
  * recursion without catastrophe in the event of a mis-parse was
  * believed to be high. Thus, if an error is found, it is bugged and
  * the parser acts as though a break were issued and just bails out
  * at that point. I havent tested all the possibilites, so I'm speaking
  * in theory, but it is 'guaranteed' to work on syntactically correct
  * MOBprograms, so if the mud crashes here, check the mob carefully!
  */
 
 char null[1];
 
 char *mprog_process_if(char *ifchck, char *com_list, struct char_data *mob,
 		       struct char_data *actor, struct obj_data *obj, void *vo,
 		       struct char_data *rndm)
 {
 
  char buf[ MAX_INPUT_LENGTH ];
  char *morebuf = '\0';
  char    *cmnd = '\0';
  int loopdone = FALSE;
  int     flag = FALSE;
  int  legal;
 
  *null = '\0';
 
  /* check for trueness of the ifcheck */
  if ((legal = mprog_do_ifchck(ifchck, mob, actor, obj, vo, rndm))) 
    if(legal != 0)
      flag = TRUE;
    else
      return null;
 
  while(loopdone == FALSE) /*scan over any existing or statements */
  {
      cmnd     = com_list;
      com_list = mprog_next_command(com_list);
      while (*cmnd == ' ')
        cmnd++;
      if (*cmnd == '\0')
      {
 	 bug ("Mob: %d no commands after IF/OR", mob_index[mob->nr].virtual); 
 	 return null;
      }
      morebuf = one_argument(cmnd,buf);
      if (!str_cmp(buf, "or"))
      {
 	 if ((legal = mprog_do_ifchck(morebuf,mob,actor,obj,vo,rndm)))
 	   if (legal != 0)
 	     flag = TRUE;
 	   else
 	     return null;
      }
      else
        loopdone = TRUE;
  }
  
  if (flag)
    for (; ;) /*ifcheck was true, do commands but ignore else to endif*/ 
    {
        if (!str_cmp(buf, "if"))
        { 
 	   com_list = mprog_process_if(morebuf,com_list,mob,actor,obj,vo,rndm);
 	   while (*cmnd==' ')
 	     cmnd++;
 	   if (*com_list == '\0')
 	     return null;
 	   cmnd     = com_list;
 	   com_list = mprog_next_command(com_list);
 	   morebuf  = one_argument(cmnd,buf);
 	   continue;
        }
        if (!str_cmp(buf, "break"))
 	 return null;
        if (!str_cmp(buf, "endif"))
 	 return com_list; 
        if (!str_cmp(buf, "else")) 
        {
 	   while (str_cmp(buf, "endif")) 
 	   {
 	       cmnd     = com_list;
 	       com_list = mprog_next_command(com_list);
 	       while (*cmnd == ' ')
 		 cmnd++;
 	       if (*cmnd == '\0')
 	       {
 		   bug ("Mob: %d missing endif after else",
 			mob_index[mob->nr].virtual);
 		   return null;
 	       }
 	       morebuf = one_argument(cmnd,buf);
	   }
    return com_list; 
        }
        mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);
        cmnd     = com_list;
        com_list = mprog_next_command(com_list);
        while (*cmnd == ' ')
 	 cmnd++;
        if (*cmnd == '\0')
        {
            bug ("Mob: %d missing else or endif", mob_index[mob->nr].virtual); 
            return null;
        }
        morebuf = one_argument(cmnd, buf);
    }
  else /*false ifcheck, find else and do existing commands or quit at endif*/
    {
      while ((str_cmp(buf, "else")) && (str_cmp(buf,"endif")))
        {
 	 cmnd     = com_list;
 	 com_list = mprog_next_command(com_list);
 	 while (*cmnd == ' ')
 	   cmnd++;
 	 if (*cmnd == '\0')
 	   {
 	     bug ("Mob: %d missing an else or endif",
 		  mob_index[mob->nr].virtual); 
 	     return null;
 	   }
 	 morebuf = one_argument(cmnd, buf);
        }
 
      /* found either an else or an endif.. act accordingly */
      if (!str_cmp(buf, "endif")) {
        return com_list;
 	}
      cmnd     = com_list;
      com_list = mprog_next_command(com_list);
      while (*cmnd == ' ')
        cmnd++;
      if (*cmnd == '\0')
        { 
 	 bug ("Mob: %d missing endif", mob_index[mob->nr].virtual); 
 	 return null;
        }
      morebuf = one_argument(cmnd, buf);
      
      for (; ;) /*process the post-else commands until an endif is found.*/
        {
 	 if (!str_cmp(buf, "if"))
 	   { 
 	     com_list = mprog_process_if(morebuf, com_list, mob, actor,
 					 obj, vo, rndm);
 	     while (*cmnd == ' ')
 	       cmnd++;
 	     if (*com_list == '\0')
 	       return null;
 	     cmnd     = com_list;
 	     com_list = mprog_next_command(com_list);
 	     morebuf  = one_argument(cmnd,buf);
 	     continue;
 	   }
 	 if (!str_cmp(buf, "else")) 
 	   {
 	     bug ("Mob: %d found else in an else section",
 		  mob_index[mob->nr].virtual); 
 	     return null;
 	   }
 	 if (!str_cmp(buf, "break"))
 	   return null;
 	 if (!str_cmp(buf, "endif"))
 	   return com_list; 
 	 mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);
 	 cmnd     = com_list;
 	 com_list = mprog_next_command(com_list);
 	 while (*cmnd == ' ')
 	   cmnd++;
 	 if (*cmnd == '\0')
	   {
      bug ("Mob:%d missing endif in else section",
 		  mob_index[mob->nr].virtual); 
 	     return null;
 	   }
 	 morebuf = one_argument(cmnd, buf);
        }
    }
 }
 
 /* This routine handles the variables for command expansion.
  * If you want to add any go right ahead, it should be fairly
  * clear how it is done and they are quite easy to do, so you
  * can be as creative as you want. The only catch is to check
  * that your variables exist before you use them. At the moment,
  * using $t when the secondary target refers to an object 
  * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
  * probably makes the mud crash (vice versa as well) The cure
  * would be to change act() so that vo becomes vict & v_obj.
  * but this would require a lot of small changes all over the code.
  */
 void mprog_translate(char ch, char *t, struct char_data *mob, struct char_data *actor,
                     struct obj_data *obj, void *vo, struct char_data *rndm)
 {
  static char *he_she        [] = { "it",  "he",  "she" };
  static char *him_her       [] = { "it",  "him", "her" };
  static char *his_her       [] = { "its", "his", "her" };
  struct char_data   *vict             = (struct char_data *) vo;
  struct obj_data    *v_obj            = (struct obj_data  *) vo;
 
  *t = '\0';
  switch (ch) {
      case 'i':
          one_argument(mob->player.name, t);
       break;
 
      case 'I':
          strcpy(t, mob->player.short_descr);
       break;
 
      case 'n':
          if (actor) {
 	   if (CAN_SEE(mob,actor)) {
              if (!IS_NPC(actor)) {
                strcpy(t, actor->player.name);
              } else
 	       one_argument(actor->player.name, t);
            } else
              strcpy(t, "Someone");
          }
       break;
 
      case 'N':
          if (actor) 
             if (CAN_SEE(mob, actor))
 	       if (IS_NPC(actor))
 		 strcpy(t, actor->player.short_descr);
 	       else
 	       {
 		   strcpy(t, actor->player.name);
 		   strcat(t, " ");
 		   strcat(t, actor->player.title);
 	       }
 	    else
 	      strcpy(t, "someone");
 	 break;
 
      case 't':
          if (vict) {
 	   if (CAN_SEE(mob, vict)) {
              if (!IS_NPC(vict))
                strcpy(t, vict->player.name);
              else
 	       one_argument(vict->player.name, t);
            } else
              strcpy(t, "Someone");
          }
 	 break;
 
      case 'T':
          if (vict) 
             if (CAN_SEE(mob, vict))
 	       if (IS_NPC(vict))
 		 strcpy(t, vict->player.short_descr);
 	       else
 	       {
 		 strcpy(t, vict->player.name);
 		 strcat(t, " ");
 		 strcat(t, vict->player.title);
 	       }
 	    else
 	      strcpy(t, "someone");
 	 break;
      
      case 'r':
          if (rndm) {
 	   if (CAN_SEE(mob, rndm)) {
              if (!IS_NPC(rndm))
                strcpy(t, rndm->player.name);
 	     else
                one_argument(rndm->player.name, t);
            } else
              strcpy(t, "Someone");
          }
       break;
 
      case 'R':
          if (rndm) 
             if (CAN_SEE(mob, rndm))
 	       if (IS_NPC(rndm))
 		 strcpy(t,rndm->player.short_descr);
 	       else
 	       {
		 strcpy(t, rndm->player.name);
 		 strcat(t, " ");
 		 strcat(t, rndm->player.title);
 	       }
 	    else
 	      strcpy(t, "someone");
 	 break;
 
      case 'e':
          if (actor)
 	   CAN_SEE(mob, actor) ? strcpy(t, he_she[(int) actor->player.sex ])
 	                         : strcpy(t, "someone");
 	 break;
   
      case 'm':
          if (actor)
 	   CAN_SEE(mob, actor) ? strcpy(t, him_her[(int) actor->player.sex ])
                                  : strcpy(t, "someone");
 	 break;
   
      case 's':
          if (actor)
 	   CAN_SEE(mob, actor) ? strcpy(t, his_her[(int) actor->player.sex ])
 	                         : strcpy(t, "someone's");
 	 break;
      
      case 'E':
          if (vict)
 	   CAN_SEE(mob, vict) ? strcpy(t, he_she[(int) vict->player.sex ])
                                 : strcpy(t, "someone");
 	 break;
   
      case 'M':
          if (vict)
 	   CAN_SEE(mob, vict) ? strcpy(t, him_her[(int) vict->player.sex ])
                                 : strcpy(t, "someone");
 	 break;
   
      case 'S':
          if (vict)
 	   CAN_SEE(mob, vict) ? strcpy(t, his_her[(int) vict->player.sex ])
                                 : strcpy(t, "someone's"); 
 	 break;
 
      case 'j':
 	 strcpy(t, he_she[(int) mob->player.sex ]);
 	 break;
   
      case 'k':
 	 strcpy(t, him_her[(int) mob->player.sex ]);
 	 break;
   
      case 'l':
 	 strcpy(t, his_her[(int) mob->player.sex ]);
 	 break;
 
      case 'J':
          if (rndm)
 	   CAN_SEE(mob, rndm) ? strcpy(t, he_she[(int) rndm->player.sex ])
 	                        : strcpy(t, "someone");
 	 break;
   
      case 'K':
          if (rndm)
 	   CAN_SEE(mob, rndm) ? strcpy(t, him_her[(int) rndm->player.sex ])
                                 : strcpy(t, "someone");
 	 break;
   
      case 'L':
          if (rndm)
 	   CAN_SEE(mob, rndm) ? strcpy(t, his_her[(int) rndm->player.sex ])
 	                        : strcpy(t, "someone's");
 	 break;
 
      case 'o':
          if (obj)
 	   CAN_SEE_OBJ(mob, obj) ? one_argument(obj->name, t)
                                    : strcpy(t, "something");
 	 break;
 
      case 'O':
          if (obj)
 	   CAN_SEE_OBJ(mob, obj) ? strcpy(t, obj->short_description)
                                    : strcpy(t, "something");
 	 break;
 
      case 'p':
          if (v_obj)
 	   CAN_SEE_OBJ(mob, v_obj) ? one_argument(v_obj->name, t)
                                      : strcpy(t, "something");
 	 break;
 
      case 'P':
          if (v_obj)
 	   CAN_SEE_OBJ(mob, v_obj) ? strcpy(t, v_obj->short_description)
                                      : strcpy(t, "something");
       break;
 
      case 'a':
          if (obj) 
           switch (*(obj->name))
 	  {
 	    case 'a': case 'e': case 'i':
             case 'o': case 'u': strcpy(t, "an");
 	      break;
             default: strcpy(t, "a");
          }
 	 break;
 
      case 'A':
          if (v_obj) 
           switch (*(v_obj->name))
 	  {
             case 'a': case 'e': case 'i':
 	    case 'o': case 'u': strcpy(t, "an");
 	      break;
             default: strcpy(t, "a");
           }
 	 break;
 
      case '$':
          strcpy(t, "$");
 	 break;
 
      default:
          bug("Mob: %d bad $var", mob_index[mob->nr].virtual);
 	 break;
        }
 
  return;
 
 }
 
 /* This procedure simply copies the cmnd to a buffer while expanding
  * any variables by calling the translate procedure.  The observant
  * code scrutinizer will notice that this is taken from act()
  */
 void mprog_process_cmnd(char *cmnd, struct char_data *mob, struct char_data *actor,
 			struct obj_data *obj, void *vo, struct char_data *rndm)
 {
   char buf[ MAX_INPUT_LENGTH ];
   char tmp[ MAX_INPUT_LENGTH ];
   char *str;
   char *i;
   char *point;
   int j;
 
   point   = buf;
   str     = cmnd;
 
   while (*str != '\0')
   {
     if (*str != '$')
     {
       *point++ = *str++;
       continue;
     }
     str++;
     mprog_translate(*str, tmp, mob, actor, obj, vo, rndm);
     i = tmp;
     ++str;
     while ((*point = *i) != '\0')
       ++point, ++i;
   }
   *point = '\0';
   str = buf;
   j = 1;
   while (j < MAX_INPUT_LENGTH-2)
   {
    if(str[j] == '\n') {
      str[j] = '\0';
      break;
      }
    if(str[j] == '\r') {
      str[j] = '\0';
      break;
      }
    if(str[j] == '\0')
      break;
    j++;
   }
 
   command_interpreter(mob, buf);
 
   return;
 
 }
 
 /* The main focus of the MOBprograms.  This routine is called 
  *  whenever a trigger is successful.  It is responsible for parsing
  *  the command list and figuring out what to do. However, like all
  *  complex procedures, everything is farmed out to the other guys.
  */
 void mprog_driver (char *com_list, struct char_data *mob, struct char_data *actor,
 		   struct obj_data *obj, void *vo)
 {
 
  char tmpcmndlst[ MAX_STRING_LENGTH ];
  char buf       [ MAX_INPUT_LENGTH ];
  char *morebuf;
  char *command_list;
  char *cmnd;
  struct char_data *rndm  = NULL;
  struct char_data *vch   = NULL;
  int        count = 0;
 
  if IS_AFFECTED(mob, AFF_CHARM)
    return;
 
  /* get a random visable mortal player who is in the room with the mob */
  for (vch = world[mob->in_room].people; vch; vch = vch->next_in_room)
    if (!IS_NPC(vch)
        &&  vch->player.level < LVL_IMMORT
        &&  CAN_SEE(mob, vch))
      {
        if (number(0, count) == 0)
 	 rndm = vch;
        count++;
      }
   
  strcpy(tmpcmndlst, com_list);
  command_list = tmpcmndlst;
  cmnd         = command_list;
  command_list = mprog_next_command(command_list);
  while (*cmnd != '\0')
    {
      morebuf = one_argument(cmnd, buf);
      if (!str_cmp(buf, "if"))
        command_list = mprog_process_if(morebuf, command_list, mob,
 				       actor, obj, vo, rndm);
      else
        mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);
      cmnd         = command_list;
      command_list = mprog_next_command(command_list);
    }
 
  return;
 
 }
 
 /***************************************************************************
  * Global function code and brief comments.
  */
 
 /* The next two routines are the basic trigger types. Either trigger
  *  on a certain percent, or trigger on a keyword or word phrase.
  *  To see how this works, look at the various trigger routines..
  */
 void mprog_wordlist_check(char *arg, struct char_data *mob, struct char_data *actor,
 			  struct obj_data *obj, void *vo, int type)
 {
 
   char        temp1[ MAX_STRING_LENGTH ];
   char        temp2[ MAX_INPUT_LENGTH ];
   char        word[ MAX_INPUT_LENGTH ];
   MPROG_DATA *mprg;
   char       *list;
  char       *start;
   char       *dupl;
   char       *end;
   int         i;
 
   for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
     if (mprg->type & type)
       {
 	strcpy(temp1, mprg->arglist);
 	list = temp1;
         while(isspace(*list)) list++;
 	for (i = 0; i < strlen(list); i++)
 	  list[i] = LOWER(list[i]);
 	strcpy(temp2, arg);
 	dupl = temp2;
 	for (i = 0; i < strlen(dupl); i++)
 	  dupl[i] = LOWER(dupl[i]);
 	if ((list[0] == 'p') && (list[1] == ' '))
 	  {
 	    list += 2;
 	    while ((start = strstr(dupl, list)))
 	      if ((start == dupl || *(start-1) == ' ')
 		  && (*(end = start + strlen(list)) == ' '
 		      || *end == '\n'
 		      || *end == '\r'
 		      || *end == '\0'))
 		{
 		  mprog_driver(mprg->comlist, mob, actor, obj, vo);
 		  break;
 		}
 	      else
 		dupl = start+1;
 	  }
 	else
 	  {
 	    list = one_argument(list, word);
 	    for(; word[0] != '\0'; list = one_argument(list, word))
 	      while ((start = strstr(dupl, word)))
 		if ((start == dupl || *(start-1) == ' ')
 		    && (*(end = start + strlen(word)) == ' '
 			|| *end == '\n'
 			|| *end == '\r'
 			|| *end == '\0'))
 		  {
 		    mprog_driver(mprg->comlist, mob, actor, obj, vo);
 		    break;
 		  }
 		else
 		  dupl = start+1;
 	  }
       }
 
   return;
 
 }
 
 void mprog_percent_check(struct char_data *mob, struct char_data *actor, struct obj_data *obj,
 			 void *vo, int type)
 {
  MPROG_DATA * mprg;
 
  for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
    if ((mprg->type & type)
        && (number(0,100) < atoi(mprg->arglist)))
      {
        mprog_driver(mprg->comlist, mob, actor, obj, vo);
        if (type != GREET_PROG && type != ALL_GREET_PROG)
 	 break;
      }
 
  return;
 
 }
 
 /* The triggers.. These are really basic, and since most appear only
  * once in the code (hmm. i think they all do) it would be more efficient
  * to substitute the code in and make the mprog_xxx_check routines global.
  * However, they are all here in one nice place at the moment to make it
  * easier to see what they look like. If you do substitute them back in,
  * make sure you remember to modify the variable names to the ones in the
  * trigger calls.
 */
 void mprog_act_trigger(char *buf, struct char_data *mob, struct char_data *ch,
 		       struct obj_data *obj, void *vo)
 {
 
   MPROG_ACT_LIST * tmp_act;
 
   if (IS_NPC(mob)
       && (mob_index[mob->nr].progtypes & ACT_PROG)
       && (mob != ch))
     {
       tmp_act = (MPROG_ACT_LIST *) malloc(sizeof(MPROG_ACT_LIST));
       if (mob->mpactnum > 0)
 	tmp_act->next = mob->mpact;
       else
 	tmp_act->next = NULL;
 
       mob->mpact      = tmp_act;
       mob->mpact->buf = str_dup(buf);
       mob->mpact->ch  = ch; 
       mob->mpact->obj = obj; 
       mob->mpact->vo  = vo; 
       mob->mpactnum++;
 
     }
   return;
 
 }
 
 void mprog_bribe_trigger(struct char_data *mob, struct char_data *ch, int amount)
 {
 
   MPROG_DATA *mprg;
   struct obj_data   *obj;
 
  if (IS_NPC(mob)
       && (mob_index[mob->nr].progtypes & BRIBE_PROG))
     {
       obj = create_money(amount);
       obj_to_char(obj, mob);
       mob->points.gold -= amount;
 
       for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
 	if ((mprg->type & BRIBE_PROG)
 	    && (amount >= atoi(mprg->arglist)))
 	  {
 	    mprog_driver(mprg->comlist, mob, ch, obj, NULL);
 	    break;
 	  }
     }
   
   return;
 
 }
 
 void mprog_death_trigger(struct char_data *mob, struct char_data *killer)
 {
 
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & DEATH_PROG))
    {
      mprog_percent_check(mob, killer, NULL, NULL, DEATH_PROG);
    }
 
  death_cry(mob);
  return;
 
 }
 
 void mprog_entry_trigger(struct char_data *mob)
 {
 
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & ENTRY_PROG))
    mprog_percent_check(mob, NULL, NULL, NULL, ENTRY_PROG);
 
  return;
 
 }
 
 void mprog_fight_trigger(struct char_data *mob, struct char_data *ch)
 {
 
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & FIGHT_PROG))
    mprog_percent_check(mob, ch, NULL, NULL, FIGHT_PROG);
 
  return;
 
 }
 
 void mprog_give_trigger(struct char_data *mob, struct char_data *ch, struct obj_data *obj)
 {
 
  char        buf[MAX_INPUT_LENGTH];
  MPROG_DATA *mprg;
 
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & GIVE_PROG))
    for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
      {
        one_argument(mprg->arglist, buf);
        if ((mprg->type & GIVE_PROG)
 	   && ((!str_infix(obj->name, mprg->arglist))
 	       || (!str_cmp("all", buf))))
 	 {
 	   mprog_driver(mprg->comlist, mob, ch, obj, NULL);
 	   break;
 	 }
      }
 
  return;
 
 }
 
 void mprog_greet_trigger(struct char_data *ch)
 {
 
  struct char_data *vmob;
 
  for (vmob = world[ch->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
    if (IS_NPC(vmob)
        && ch != vmob
        && CAN_SEE(vmob, ch)
        && (vmob->char_specials.fighting == NULL)
        && AWAKE(vmob)
        && (mob_index[vmob->nr].progtypes & GREET_PROG))
      mprog_percent_check(vmob, ch, NULL, NULL, GREET_PROG);
    else
      if (IS_NPC(vmob)
 	 && (vmob->char_specials.fighting == NULL)
 	 && AWAKE(vmob)
 	 && (mob_index[vmob->nr].progtypes & ALL_GREET_PROG))
        mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
 
  return;
 
 }
 
 void mprog_hitprcnt_trigger(struct char_data *mob, struct char_data *ch)
 {
 
  MPROG_DATA *mprg;
 
  if (IS_NPC(mob)
      && (mob_index[mob->nr].progtypes & HITPRCNT_PROG))
    for (mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next)
      if ((mprg->type & HITPRCNT_PROG)
 	 && ((100*mob->points.hit / mob->points.max_hit) < atoi(mprg->arglist)))
        {
 	 mprog_driver(mprg->comlist, mob, ch, NULL, NULL);
 	 break;
        }
  
  return;
 
 }
 
 void mprog_random_trigger(struct char_data *mob)
 {
   if (mob_index[mob->nr].progtypes & RAND_PROG)
     mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);
 
   return;
 
 }
 
 void mprog_speech_trigger(char *txt, struct char_data *mob)
 {
 
   struct char_data *vmob;
 
   for (vmob = world[mob->in_room].people; vmob != NULL; vmob = vmob->next_in_room)
     if ((mob != vmob) && IS_NPC(vmob) && (mob_index[vmob->nr].progtypes & SPEECH_PROG))
       mprog_wordlist_check(txt, vmob, mob, NULL, NULL, SPEECH_PROG);
   
   return;
 
 }
