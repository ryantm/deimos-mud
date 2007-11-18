/* ******************************************************************** *
 * FILE	: assemblies.h			Copyright (C) 1999 Geoff Davis	*
 * USAGE: Definitions, constants and prototypes for assembly engine.	*
 * -------------------------------------------------------------------- *
 * 1999 MAY 07	gdavis/azrael@laker.net	Initial implementation.		*
 * ******************************************************************** */

#if !defined( __ASSEMBLIES_H__ )
#define __ASSEMBLIES_H__

/* ******************************************************************** *
 * Preprocessor constants.						*
 * ******************************************************************** */

/* Assembly type: Used in ASSEMBLY.iAssemblyType */
#define ASSM_ASSEMBLE		0	// Assembly must be assembled.
#define MAX_ASSM		1	// Number of assembly types.

/* ******************************************************************** *
 * Type aliases.							*
 * ******************************************************************** */

typedef struct assembly_data	ASSEMBLY;
typedef struct component_data	COMPONENT;

/* ******************************************************************** *
 * Structure definitions.						*
 * ******************************************************************** */

/* Assembly structure definition. */
struct assembly_data {
  long		lVnum;			/* Vnum of the object assembled. */
  long		lNumComponents;		/* Number of components. */
  unsigned char	uchAssemblyType;	/* Type of assembly (ASSM_xxx). */
  struct component_data *pComponents;		/* Array of component info. */
};

/* Assembly component structure definition. */
struct component_data {
  bool		bExtract;		/* Extract the object after use. */
  bool		bInRoom;		/* Component in room, not inven. */
  long		lVnum;			/* Vnum of the component object. */
};

/* ******************************************************************** *
 * Prototypes for assemblies.c.						*
 * ******************************************************************** */

void		assemblyBootAssemblies( void );
void		assemblySaveAssemblies( void );
void		assemblyListToChar( struct char_data *pCharacter );

bool		assemblyAddComponent( long lVnum, long lComponentVnum,
		  bool bExtract, bool bInRoom );
bool		assemblyCheckComponents( long lVnum, struct char_data
		  *pCharacter );
bool		assemblyCreate( long lVnum, int iAssembledType );
bool		assemblyDestroy( long lVnum );
bool		assemblyRemoveComponent( long lVnum, long lComponentVnum );

int		assemblyGetType( long lVnum );

long		assemblyCountComponents( long lVnum );
long		assemblyFindAssembly( const char *pszAssemblyName );
long		assemblyGetAssemblyIndex( long lVnum );
long		assemblyGetComponentIndex( ASSEMBLY *pAssembly,
		  long lComponentVnum );

ASSEMBLY*	assemblyGetAssemblyPtr( long lVnum );

/* ******************************************************************** */

#endif

