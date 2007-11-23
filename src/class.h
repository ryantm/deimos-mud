/* ************************************************************************
*   File: class.h                                       Part of CircleMUD *
*  Usage: header file: prototypes of public class functions               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*  Created by YIL                                                         *	
************************************************************************ */

#ifndef __CLASS_H__
#define __CLASS_H__

#ifndef __CLASS_C__
extern int thaco[NUM_CLASSES][LVL_IMPL+1];
#endif

#define GET_THACO(class, level)	\
	((level) <= MAX_MORT_LEVEL ? thaco[(int) (class)][(int) (level)] : 1)

/* Extern functions */

int cmp_class(char *class_str);

int needed_exp(byte char_class, byte gain_class, int level);
int needed_gold(byte char_class, byte gain_class, int level);

#endif
