# Generated automatically from Makefile.in by configure.
# CircleMUD 3.0 Makefile.in - Makefile template used by 'configure'
#

# C compiler to use
CC = gcc

# Path to cxref utility
CXREF = cxref

# Any special flags you want to pass to the compiler
MYFLAGS = -Wall

#flags for profiling (see hacker.doc for more information)
PROFILE = 

##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################
BINDIR = ../bin
DEIMOSNAME = $(BINDIR)/archive/deimos-`date +'%Y-%m-%d-%H:%M'`	

#extedned debug
CFLAGS = -ggdb -O2 $(MYFLAGS) $(PROFILE)

LIBS =  -lcrypt 

OBJFILES = act.comm.o act.informative.o act.item.o act.movement.o \
  act.offensive.o act.other.o act.social.o act.wizard.o alias.o ban.o boards.o \
  class.o comm.o config.o constants.o db.o fight.o genmob.o genobj.o \
  genolc.o genshp.o genwld.o genzon.o graph.o handler.o house.o improved-edit.o \
  interpreter.o limits.o magic.o mail.o medit.o mobact.o modify.o oasis.o \
  objsave.o oedit.o olc.o random.o redit.o sedit.o shop.o spec_assign.o \
  spec_procs.o spell_parser.o spells.o tedit.o utils.o weather.o zedit.o \
  assemblies.o assedit.o dg_comm.o dg_db_scripts.o dg_event.o dg_handler.o \
  dg_mobcmd.o dg_misc.o dg_objcmd.o dg_scripts.o dg_triggers.o dg_wldcmd.o \
  dg_olc.o hedit.o prgrun.o diskio.o cedit.o clan.o freezetag.o screen.o

default: deimos

all:
	$(MAKE) $(BINDIR)/deimos

deimos:
	$(MAKE) $(BINDIR)/deimos

$(BINDIR)/deimos : $(OBJFILES)
	$(CC) -o $(BINDIR)/deimos $(PROFILE) $(OBJFILES) $(LIBS)

deploy:
	mkdir -p ${BINDIR}/archive
	cp ${BINDIR}/deimos ${DEIMOSNAME}
	ln -sf ${DEIMOSNAME} ${BINDIR}/deimos-live

clean:
	rm -f *.o