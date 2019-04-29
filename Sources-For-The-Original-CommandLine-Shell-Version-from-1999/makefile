## @chmod 6755 $(PROGRAM)

CC = cc
CFLAGS        = -O2 -DUNIX -DLINUX -DCRIPPLED_TOUPPER -DUSE_SENDMAIL -funsigned-char -g
DEST	      = .
MANDEST       = /usr/man/man6
HDRS	      = shoogi.h
LDFLAGS	      = -O2
LIBS	      = 
LINKER	      = cc
MAKEFILE      = Makefile
OBJS	      = shoogi.o shoogi2.o moves.o getch.o
PRINT	      = lpr
PROGRAM	      = shoogi
SRCS	      = shoogi.c shoogi2.c moves.c getch.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@chmod u+s,g+s $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

#$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
#		@make -f $(MAKEFILE) DEST=$(DEST) 
###
