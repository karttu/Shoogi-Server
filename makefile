## @chmod 6755 $(PROGRAM)

CC = cc
CFLAGS        = -O2 -DUNIX -DLINUX -DCRIPPLED_TOUPPER -DUSE_SENDMAIL -funsigned-char -g
DEST	      = .
MANDEST       = /usr/man/man6
HDRS	      = webboard.h
LDFLAGS	      = -O2
LIBS	      = 
LINKER	      = cc
MAKEFILE      = Makefile
OBJS	      = webboard.o moves.o
PRINT	      = lpr
PROGRAM	      = webboard.cgi
SRCS	      = webboard.c moves.c

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
