
#include "shoogi.h"

MYINT _uid_=0;        /* Player's user id. */
MYINT _gid_=0;        /* Game currently being played/shown. */
MYINT _blackplid_=0;  /* Black's plid. */
MYINT _whiteplid_=0;  /* White's plid. */
MYINT _ownplid_=0;    /* Own plid. */
MYINT _movesmade_=0;  /* Moves made so far. */
int   _movestat_=0;   /* Status returned by last call of check_move */
int   _captured_piece_=0; /* Last piece captured, if last move was capture. */
int   _b_impasse_points_ = DEFAULT_IMPASSE_POINTS;
int   _w_impasse_points_ = DEFAULT_IMPASSE_POINTS;

int _owncolor_=NOCOLOR; /* Initially not specified. */

ULI _ownseekptr_=NOTSET_SEEKPTR;
ULI _flags_=DEFAULT_FLAGS;
/* Opponent's flags, fetched by fetch_opponents_fields: */
ULI _opflags_=DEFAULT_FLAGS;

char _blackname_[SMALLBUF];
char _whitename_[SMALLBUF];
char *_ownname_;
char space_for_latest_move[MAXBUF+3];
char *_latest_move_=space_for_latest_move;

char *_names_[2] = { _blackname_, _whitename_ };

char _not_me_pattern_[SMALLBUF+1];
char *_everybody_ = "*";
char _dot_[MAXBUF+3];

/* Opponent's fields (from Playerfile).
   Stored here by the function fetch_opponents_fields: */
char *_opfields_[MAXPLFIELDS+1];
char spacefor_opfields[BUFSIZ+3]; /* And space for them. */

/* 0 if not completed, 1 if black (0) won, 2 if white (1) won: */
char _eog_=0;
char _debug_flag_=0;
char _su_flag_=0; /* Whether player is super user or not. */
/* If 1 then player is not allowed to jump to shell, nor write files
   (option -r): */
char _restricted_flag_=0;
char _get_or_addplid_=0;

int _sig_immediately_ = 0;
int _sig_received_    = 0;

GIDLIST _gidlist_=NULL;
char *wlist[MAXWLIST+2];
int _wlistsize_;

int _morelines_=22;

char *cmds[MAXCMDS+3];
int  cmdcnt;

char escapemsgbuf[CHKMSGBUF+3];
char checkmsgbuf[CHKMSGBUF+3];
char E1[MAXBUF+3];
char E2[CHKMSGBUF+3];
char OB[(MAXBUF*3)+3]; /* Output Buffer for fout. */


/* Signals used for various events: */
int notify_signals[]=
 {
   0,
   SIG_MESSAGE_SENT,
   SIG_MOVE_MADE,
   SIG_GAME_STARTED,
   SIG_GAME_FINISHED
 };


char *codenames[] =
 {
/* 0 */   "NONE",
/* 1 */   "BLINK",
/* 2 */   "BOLD",
/* 3 */   "REVERSE",
/* 4 */   "UNDERLINE",
/* 5 */   "BLACK",
/* 6 */   "RED",
/* 7 */   "GREEN",
/* 8 */   "YELLOW",
/* 9 */   "BLUE",
/* A */   "MAGENTA",
/* B */   "CYAN",
/* C */   "WHITE",
          NULL
 };

#ifdef CURSES

chtype curses_codes[] =
 {
/* 0 */   A_NORMAL,
/* 1 */   A_BLINK,
/* 2 */   A_BOLD,
/* 3 */   A_REVERSE,
/* 4 */   A_UNDERLINE,
/* 5 */   COLOR_PAIR(COLOR_BLACK+1),
/* 6 */   COLOR_PAIR(COLOR_RED+1),
/* 7 */   COLOR_PAIR(COLOR_GREEN+1),
/* 8 */   COLOR_PAIR(COLOR_YELLOW+1),
/* 9 */   COLOR_PAIR(COLOR_BLUE+1),
/* A */   COLOR_PAIR(COLOR_MAGENTA+1),
/* B */   COLOR_PAIR(COLOR_CYAN+1),
/* C */   COLOR_PAIR(COLOR_WHITE+1),
/* D */   A_NORMAL,
/* E */   A_NORMAL,
/* F */   A_NORMAL
 };

#endif

unsigned char ansi_codes[] =
 {
   0,  /* 0 NORMAL    */
   5,  /* 1 BLINK     */
   1,  /* 2 BOLD      */
   7,  /* 3 REVERSE   */
   4,  /* 4 UNDERLINE */
   30, /* 5 BLACK     */
   31, /* 6 RED       */
   32, /* 7 GREEN     */
   33, /* 8 YELLOW    */
   34, /* 9 BLUE      */
   35, /* A MAGENTA   */
   36, /* B CYAN      */
   37, /* C WHITE     */
   0,  /* D NORMAL    */
   0,  /* E NORMAL    */
   0   /* F NORMAL    */
 };


FILE *err_fp;

int G_argc;
char **G_argv;

char space_for_cmdbuf[MAXBUF+3],sparebuf[MAXBUF+3];

main(argc,argv)
int argc;
char **argv;
{
    int sig;
    char *cmdbuf;
    char *query_string;

    G_argc = argc;
    G_argv = argv;

    _ownname_ = "NOT YET FETCHED";

    handle_options();

    init_screen();

    signal(SIGINT,sighandler);
    signal(SIGTERM,sighandler);
#ifdef UNIX
    signal(SIGHUP,sighandler);
    signal(SIGQUIT,sighandler);
    signal(SIGUSR1,sighandler);
    signal(SIGUSR2,sighandler);
#endif

    if(!(err_fp = myfopen(ERRLOGFILE,"Sa")))
     {
       fprintf(stderr,
"\n%s: Fatal internal error, can't open file %s for writing!\n",
         progname,ERRLOGFILE);
       exit(1);
     }

    initmovefuns();
    initboard();
    initwlist();
    *escapemsgbuf = '\0';

    _ownname_ = getname();
    _ownplid_ = get_or_addplid(_ownname_,0);
    /* Copy player name preceded with ! to _not_me_pattern_: */
    *_not_me_pattern_ = '!';
    strcpy((_not_me_pattern_+1),_ownname_);
    *_dot_ = '\0'; /* Initialize _dot_ to be empty. */

    if(getseclevel(_flags_) == BANISHED)
     {
       pout(("\n?Sorry, you are banished!\n"));
       end_screen();
       exit(0);
     }

    if(NULL != (query_string = getenv("QUERY_STRING"))) /* Started as CGI? */
     {
       char *b,*o;
       MYINT board_selected=0;
       int other_flag=0;

       b = strchr(query_string,'B');
       o = strchr(query_string,'O');
       if(b && ('=' == *(b+1))) { board_selected = atol(b+2); }
       if(o && ('=' == *(o+1))) { other_flag = atol(o+2); }
       _flags_ = setboardstyle(_flags_, (HTML_BOARD+UNIKANJI));

       fprintf(stdout, "Content-Type: %s\r\n\r\n", "text/html; charset=UTF-8");
       fprintf(stdout,"<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
       fprintf(stdout,"<TITLE>Situation of Game %u</TITLE>\n", board_selected);

       /*

function loop_over_csv_items(fun_to_call, csv_string, other_args)
{
   var expr, sep = ',';
   var inx1;
   var len;
 
// csv_string = "" + csv_string; // Force it to be really string.

   list = csv_string.split(sep);
   len = list.length;
   inx1 = 0;

   while(inx1 < len)
    {
      expr = fun_to_call + '("' + list[inx1] + '",' + inx1;
      if(other_args.length > 0) { expr += ',"' + other_args + '"'; }
      expr += ')';
//    alert('expression to evaluate=' + expr);
      eval(expr);
      inx1 = inx1+1;
    }
   return(len);
}

*/

       fprintf(stdout,
"<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n"
"<!-- Hide it! \n"
"var _AVAILABLE_SQUARES = \"\";\n"
"var _SOURCE_SQUARE = \"\";\n"
"var _PIECECODE = \"\";\n"
"function member(item,list)\n"
"{\n"
"      var len = list.length;\n"
"      var i = 0;\n"
"      while(i < len)\n"
"       {\n"
"         if(list[i] == item) { return(true); }\n"
"         i = i + 1;\n"
"       }\n"
"      return(false);\n"
"}\n"
"function domove(piececode,src_square,freedoms,avail_squares)\n"
"{\n"
"   if('' == document.BOARDFORM.MOVE.value)\n"
"    {\n"
"      if(0 == freedoms)\n"
"       {\n"
"         alert(\"You cannot make a move starting from this square \"\n"
"               + src_square);\n"
"         _AVAILABLE_SQUARES = '';\n"
"         _SOURCE_SQUARE = '';\n"
"         _PIECECODE = '';\n"
"         document.BOARDFORM.MOVE.value = '';\n"
"         return(false);\n"
"       }\n"
"      else\n"
"       {\n"
"         alert(\"Piece \" + piececode + \" in square \" + src_square + \" has \" + freedoms + \" freedoms: \"\n"
"           + avail_squares);\n"
"         _AVAILABLE_SQUARES = avail_squares;\n"
"         _SOURCE_SQUARE = src_square;\n"
"         _PIECECODE = piececode;\n"
"         document.BOARDFORM.MOVE.value = piececode + src_square;\n"
"         return(true);\n"
"       }\n"
"    }\n"
"   else\n"
"    {\n"
"      if(member(src_square,_AVAILABLE_SQUARES.split(',')))\n"
"       {\n"
"         if(' ' == piececode) // Just an ordinary move\n"
"          {\n"
"            document.BOARDFORM.MOVE.value\n"
"             = document.BOARDFORM.MOVE.value + '-';\n"
"          }\n"
"         else // It's a capture.\n"
"          {\n"
"            document.BOARDFORM.MOVE.value\n"
"             = document.BOARDFORM.MOVE.value + 'x';\n"
"          }\n"
"         document.BOARDFORM.MOVE.value\n"
"          = document.BOARDFORM.MOVE.value + src_square;\n"
"         document.BOARDFORM.submit();\n"
"       }\n"
"      else\n"
"       {\n"
"         alert(\"Piece \" + _PIECECODE + \" in square \" + _SOURCE_SQUARE\n"
"               + \" cannot move to the square \"  + src_square)\n"
"         _AVAILABLE_SQUARES = '';\n"
"         _SOURCE_SQUARE = '';\n"
"         _PIECECODE = '';\n"
"         document.BOARDFORM.MOVE.value = '';\n"
"         return(false);\n"
"       }\n"
"    }\n"
"   return(true);\n"
"}\n"
"// --End Hiding Here -->\n"
"</SCRIPT>\n");

       fprintf(stdout, "</HEAD><BODY BGCOLOR=\"#ffffff\">\n"
                       "<FORM NAME=\"BOARDFORM\" ACTION=\"shoogi.cgi\">");

       fprintf(stdout, "<INPUT TYPE=\"TEXT\" NAME=\"MOVE\">\n");
       fflush(stdout);
       show_board(0, board_selected, other_flag, NULL);
       fprintf(stdout, "</FORM></BODY></HTML>\n");
       exit(0);
     }

    /* Show message of the day, if there's any: */
    morefile(MOTDFILE,0);

    sprintf(sparebuf,"%s has connected to the %s.",_ownname_,progname);
    find_all_online(sparebuf,'M',_not_me_pattern_);

    fout((OB,"\nPlayer %s, plid: %lu\n",_ownname_,_ownplid_));

    STAT();

    while(1)
     { /* Clear space_for_cmdbuf first: */
       memset(space_for_cmdbuf,'\0',MAXBUF+1);
bigloop:
       cmdbuf = space_for_cmdbuf+1; /* Space for ! kludge. */
       sig = _sig_received_;
       _sig_received_ = 0;
       if(sig)
        {
	  pout(("\n"));
          switch(sig)
           {
	     case SIGINT:
	      {
	        pout(("?Please use the command QUIT for quitting!\n"));
	        break;
	      }
#ifndef UNIX
	     case SIG_GAME_STARTED:
#endif
	     case SIG_GAME_FINISHED: { readgidlist(); }
	     case SIG_MOVE_MADE: { checknewmoves(0); }
	   }
        }
/* Check the new messages currently always: */
       checkmessages();

       /* Print the prompt. */
       if(_gid_)
        {
          fout((OB,"Game %lu, Moves made %lu, ",_gid_,_movesmade_));
          if(_owncolor_ != NOCOLOR)
           { fout((OB,"You are %s, ",getcapcolname(_owncolor_))); }
	  if(_eog_) { fout((OB,"%s won!>",getcapcolname(_eog_-1))); }
          else
	   { fout((OB,"%s's turn>",getcapcolname(get_parity(_movesmade_)))); }
        }
       else { fout((OB,"%s>",progname)); } /* No game selected. */
#ifdef UNIX
       pout((cmdbuf)); /* Kludge to make input prettier... */
#endif

       _sig_immediately_ = 1;
       if(_sig_received_) { _sig_immediately_ = 0; continue; }

#ifdef CURSES
       if(getstr(cmdbuf),(*cmdbuf == 4)) /* If CTRL-D encountered. */
#else
       if(!fgets(cmdbuf,MAXBUF,input))
#endif
        {
          _sig_immediately_ = 0;
#ifndef CURSES
          if(feof(input))
#endif
	   {
	     pout(("\n"));
             errlog_only((E1,"\n**EOF encountered at main command loop.\n"));
             QUIT();
	   }
#ifndef CURSES
          else { goto bigloop; }
#endif
        }

       _sig_immediately_ = 0;

       
       cmdbuf = nuke_newline(skip_blankos(cmdbuf));

       /* Ugly kludge for !command 's */
       if((*cmdbuf == '!') || (*cmdbuf == '.'))
        { /* If command begins with ! or . */
          *(cmdbuf-1) = *cmdbuf; /* Move that punctuation char one leftward */
	  *cmdbuf = ' ';  /* Then insert one blank between. */
	  --cmdbuf;
	}

       /* Another ugly kludge to modify commands like "SET  KANJI ..."
           to "SET_KANJI ..." */
       else if((toupper(*cmdbuf)     == 'S') && 
               (toupper(*(cmdbuf+1)) == 'E') &&
	       (toupper(*(cmdbuf+2)) == 'T') &&
	       (isspace(*(cmdbuf+3))))
        {
	  strcpy((cmdbuf+4),skip_blankos(cmdbuf+3));
	  *(cmdbuf+3) = '_';
	}

       /* If no command given: */
       if(!(cmdcnt = hack_to_pieces(cmds,cmdbuf))) { continue; }
       conv_upper(*cmds); /* Convert first one (command) to uppercase. */

       if(strequ(*cmds,"Q") || strequ(*cmds,"QUIT") ||
          strequ(*cmds,"X") || strequ(*cmds,"EXIT")) { QUIT(); }

       else if(strequ(*cmds,"B") || strequ(*cmds,"BOARD")
               || strequ(*cmds,"REFRESH"))
        { BOARD(); }

       else if(strequ(*cmds,".")) { DOT(); }

       else if(strequ(*cmds,"G") || strequ(*cmds,"GAMES")) { GAMES(); }

       else if(strequ(*cmds,"?") || strequ(*cmds,"H") || strequ(*cmds,"HELP"))
        { HELP(); }

       else if(strequ(*cmds,"HANDICAP")) { HANDICAP(); }

       else if(strequ(*cmds,"M") || strequ(*cmds,"MOVES")) { MOVES(); }

       else if(strequ(*cmds,"N") || strequ(*cmds,"NEW")) { NEW(); }

       else if(strequ(*cmds,"PLAY"))   { PLAY(); }

       else if(strequ(*cmds,"REMOVE")) { REMOVE(); }

       else if(strequ(*cmds,"RESIGN")) { RESIGN(); }

       else if(strequ(*cmds,"SAY"))    { SAY(); }

       else if(strequ(*cmds,"SET") || strequ(*cmds,"SET_")) { SET(); }

       else if(strequ(*cmds,"SET_ADDRESS") || strequ(*cmds,"ADDRESS"))
        { SET_ADDRESS(); }

       else if(strequ(*cmds,"SET_DRAGON")) { SET_DRAGON(); }

       else if(strequ(*cmds,"SET_KANJI"))  { SET_KANJI(); }
       
       else if(strequ(*cmds,"SET_MAILNOTIFY"))  { SET_MAILNOTIFY(); }

       else if(strequ(*cmds,"SET_PSTYLE")) { SET_PSTYLE(); }

       else if(strequ(*cmds,"SET_BSTYLE") || strequ(*cmds,"STYLE"))
        { SET_BSTYLE(); }

       else if(strequ(*cmds,"SET_BLACK"))      { SET_BLACK(); }

       else if(strequ(*cmds,"SET_WHITE"))      { SET_WHITE(); }

       else if(strequ(*cmds,"SET_THREATENED")) { SET_THREATENED(); }

       else if(strequ(*cmds,"!") || strequ(*cmds,"SHELL") ||
               strequ(*cmds,"PUSH"))  { SHELL(); }

       else if(strequ(*cmds,"SHOUT"))  { SHOUT(); }

       else if(strequ(*cmds,"STAT") || strequ(*cmds,"S")) { STAT(); }

       else if(strequ(*cmds,"STATS") || strequ(*cmds,"USER")) { STATS(); }

       else if(strequ(*cmds,"TELL"))   { TELL(); }

       else if(strequ(*cmds,"U") || strequ(*cmds,"UNDO")) { UNDO(); }

       else if(strequ(*cmds,"W") || strequ(*cmds,"WHO"))
        { STATS(); } /* Previously WHO(); */

       else if(strequ(*cmds,"DUMPGL")) { dumpgidlist(); }
       else
        {
	  fout((OB,"?Unrecognized command: %s\n",*cmds));
	}
     } /* While */
}


void sighandler(sig)
int sig;
{
    signal(sig,sighandler); /* Reassign signal to this function. */

    switch(sig)
     {
       case SIGTERM:
#ifdef UNIX
       case SIGHUP:
       case SIGQUIT:
#endif
        {
	  _sig_received_ = sig;
          errlog((E1,
"\n%s: Signal (%u) received, quitting.\n",progname,sig));
          ertzu_exit(sig);
	}
     }

    if(!_sig_immediately_) { _sig_received_ = sig; return; }

    pout(("\n"));
    switch(sig)
     {
       case SIGINT:
	{
	  pout(("?Please use the command Quit for quitting!\n"));
	  break;
	}
/*     case SIG_GAME_STARTED: */
       case SIG_GAME_FINISHED: { readgidlist(); }
/*     case SIG_MESSAGE_SENT: { checkmessages(); } */
       case SIG_MOVE_MADE: { checkmessages(); checknewmoves(0); }
     }
}


QUIT()
{
    char buf[MAXBUF+3];

    sprintf(buf,"%s has just left the %s.",_ownname_,progname);
    /* Clear the pid from player list: */
    get_or_addplid(_ownname_,1); /* Maybe this could be... */
    find_all_online(buf,'M',_not_me_pattern_); /* done in the one and same function? */
    end_screen();
    exit(0);
}

ertzu_exit(stat)
int stat;
{
    char buf[MAXBUF+3];

    if(stat == 101)
     {
       sprintf(buf,
"%s has been disconnected from the %s due to an internal error.",
        _ownname_,progname);
     }
    else
     {
       sprintf(buf,
"%s has been disconnected from the %s with the signal %u.",
        _ownname_,progname,stat);
     }

    /* Clear the pid from player list, unless get_or_addplid
        itself has generated this error: */
    if(!_get_or_addplid_) /* If not involved with an error in that file. */
     {
       get_or_addplid(_ownname_,1); /* Maybe this could be... */
       find_all_online(buf,'M',_not_me_pattern_); /* done in the one and same function? */
     }
    else
     {
       errlog((E1,
"%s: Couldn't clear your pid, it has been left to player file as ghost pid.\n",
         progname));
     }
    end_screen();
    exit(stat);
}


PLAY()
{
    if(cmdcnt < 2)
     {
       pout((
"Usage: PLAY <move-notation>.\n"));
       pout((
"For example, PLAY P-7f  or  PLAY Gi4-h5\n"));
     }
    else { play_move(conv_upper(*(cmds+1))); }
}


RESIGN()
{
    play_move("RESIGN");
}


HANDICAP()
{
    char movebuf[MAXBUF+3];

    if(cmdcnt != 2)
     {
       pout((
"Usage: HANDICAP piece[,piece,...,piece]\n"));
       pout((
"For example, HANDICAP L@1a,R@8b removes white's left lance and rook.\n"));
       pout((
"(There should be no blanks between the coordinates, only commas!)\n"));
       return(0);
     }

    strcpy(movebuf,"HANDICAP:");
    strcat(movebuf,*(cmds+1));
    play_move(movebuf);
}

/* General move maker for PLAY, HANDICAP & RESIGN. */
play_move(newmove)
char *newmove;
{
    MYINT m,plid;
    int pid,old_movestat,old_captured_piece;
    char status,new_status;
    char tmpbuf[MAXBUF+3],movebuf[MAXBUF+3];
    char old_checkmsgbuf[MAXBUF+3],old_escapemsgbuf[MAXBUF+3];

    if(!own_gamep()) { return(0); }

    if(end_of_gamep()) { return(0); }

    if(!(status = lastmovep(&m,movebuf,tmpbuf))) { return(0); }

    if(!my_turnp(status,m)) { return(0); }

    strcpy(old_checkmsgbuf,checkmsgbuf);
    strcpy(old_escapemsgbuf,escapemsgbuf);
    old_captured_piece = _captured_piece_;
    old_movestat = _movestat_;

 /* strcpy(movebuf,newmove); */
 /* Increase this here, so that p and o in check_move show correct count: */
    _movesmade_++;
    if(_movestat_ = check_move(newmove,_owncolor_,1))
     {
       pout(("Move done!\n"));
/* Transfer old move (which was just read in from .lm file) to movelog: */
/* But not if there is zero move (like 0 W) in .lm file, or the last move
   was undone: */
       if(m && (status != 'U'))
        {
	  addtomovelog(_gid_,m,old_movestat,old_captured_piece,movebuf,tmpbuf,
	                old_checkmsgbuf,old_escapemsgbuf);
	}

       new_status = 'P';
       /* If playing this move second time, after undoing, then put status
           letter Q instead of P to .lm file, unless the move undone
	   was HANDICAP, which is always possible to undo.
	  Or if opponent's mailnotifyflag is set, then also undoing is
	   impossible: (but that is commented out now) */
/*     fetch_opponents_fields(_names_[color]); */
       if(/* getmailnotifyflag(_opflags_) || */
           ((status == 'U') && strequ(movebuf,newmove)
	                    && !cutstrequ(movebuf,"HANDICAP:")))
	{ new_status++; }
       /* And overwrite the .lm file with new move: */
       put_next_move(_gid_,_movesmade_,newmove,get_timebuf(),new_status);

/* Show some essential consequences of move, if there's any.
   However, if game against self, then don't print them here,
   as they are immediately printed in checknewmoves anyway.
 */
       if(_blackplid_ != _whiteplid_)
        {
          show_checks(stdout,_movestat_,!_owncolor_,_captured_piece_,
	                 checkmsgbuf,escapemsgbuf);
	}

       notify_opponent(!_owncolor_,MOVE_MADE);
     }
    else
     {
       /* Restore these variables. */
       _movesmade_--;
       strcpy(checkmsgbuf,old_checkmsgbuf);
       strcpy(escapemsgbuf,old_escapemsgbuf);
       _captured_piece_ = old_captured_piece;
       _movestat_ = old_movestat;

       if(*E2) { pout((E2)); }
       pout(("?Move not done!\n"));
     }
}


UNDO()
{
    MYINT m;
    int otherflag;

    if(!own_gamep()) { return(0); }

    if(end_of_gamep()) { return(0); }

    otherflag = ((cmdcnt > 1) && strequ(conv_upper(*(cmds+1)),"-O"));

    if((m = undo_move(_gid_,_owncolor_,otherflag)) == ERRMOVES)
     {
       errlog((E1,"Couldn't open a file %lu.lm for reading!\n",_gid_));
       return(0);
     }

    if(m == _movesmade_) { return(0); } /* Couldn't undo. */
    /* If undo succesfull then load the game in again, to get the previous
       situation: */
    if(loadmoves(_gid_,0,0) == ERRMOVES)
     {
       errlog((E1,"**Can't find movelog for the game %lu !\n",board));
       _movesmade_ = _gid_ = 0;
       return(0);
     }

    return(1);
}


int own_gamep()
{
    if((_owncolor_ == NOCOLOR) || !_gid_)
     {
       pout(("?This is not your game!\n"));
       return(0);
     }
    else { return(1); }
}

int end_of_gamep()
{
    if(_eog_ /* || EOG(_movestat_) */ )
     {
       pout(("?This game has been completed, can't do that anymore!\n"));
       return(1);
     }
    else { return(0); }
}

int lastmovep(ptr_to_m,movebuf,timebuf)
MYINT *ptr_to_m;
char *movebuf,*timebuf;
{
    unsigned char status;

    if((*ptr_to_m = get_next_move(_gid_,&status,movebuf,timebuf,0,0))
         == ERRMOVES)
     {
       fout((OB,
"?Can't find the latest move for the game %lu!\n",_gid_));
       pout((
"?The game is probably non-existent, finished or there is bug somewhere.\n"));
       return(0);
     }

    return(status);
}


int my_turnp(status,movecount)
unsigned char status;
MYINT movecount;
{
    if((status == 'P') || (status == ('P'+1)) ||
       (get_parity(movecount) != _owncolor_))
     {
       pout(("?It's not your turn!\n"));
       return(0);
     }
    else if(movecount != _movesmade_)
     {
       pout((
"?Please load the whole game in with MOVES -N command before you make your move.\n"));
       return(0);
     }
    return(1);
}

STAT()
{
    readgidlist();
    pout(("Games in which you are involved:\n"));
    if(!checknewmoves(1)) { pout(("NONE.\n")); }
}


STATS()
{
    MYINT plid,pid,count=0;
    ULI seekpos=0L;
    char *pattern;
    char **fields;
    char who_flag;

    who_flag = (**cmds == 'W');

    if(!*(cmds+1)) { pattern = (who_flag ? _everybody_ : _ownname_); }
    else           { pattern = conv_upper(*(cmds+1)); }

    while(fields = getplfields(pattern,&seekpos,who_flag))
     {
       count++;
       if(who_flag)
        {
	  fout((OB,"%-25s %s\n",fields[PLF_NAME],fields[PLF_TIME]));
        }
       else
        {
          pid = atol(fields[PLF_PID]);
          fout((OB,"Player %-25s plid: %lu   flags: %s\n",
                 fields[PLF_NAME],atol(fields[PLF_PLID]),fields[PLF_FLAGS]));
          fout((OB,"Currently %slogged on the %s.\n",
                 (pid ? "" : "not "),progname));
          fout((OB,"Last logged %s %s\n",(pid ? "in" : "out"),
	         fields[PLF_TIME]));
          fout((OB,
	    "Has used %s %lu times. %lu wins, %lu losses, %lu ties.\n",
                  progname,atol(fields[PLF_COUNT]),
		           atol(fields[PLF_WINS]),
			   atol(fields[PLF_LOSSES]),
                           atol(fields[PLF_TIES])));
          fout((OB,"Address: %s\n\n",fields[PLF_ADDRESS]));
	}
     }

    if(who_flag && (cmdcnt < 2))
     {
       fout((OB,"%lu player%s online.\n",count,
         ((count == 1) ? "" : "s"))); /* Use plural if count != 1. */
     }
    else if(count != 1)
     {
       fout((OB,"%lu names matched.\n",count));
     }
}


SHELL()
{
    int stat;
    int euid,egid;
    char *p;

    /* Let's check that this uid is allowed to go to shell, and restore
        the original uid & gid of user, so that (s)he can't do any
	practical jokes with the uid this program is installed: */
    if(!restore_real_uid(&euid,&egid)) { return(0); }

    if(cmdcnt > 1) /* If arguments given, then use them. */
     { p = reconstruct_cmdline(cmds+1); }
    else /* Otherwise use the default shell. */
     {
#ifdef UNIX
       if(!(p = getenv("SHELL")))   { p = "/bin/sh"; }
#else /* MS-DOS? */
       if(!(p = getenv("COMSPEC"))) { p = "COMMAND"; }
#endif
     }

    /* Restore the normal mode when we go to shell. (If curses used.) */
    end_screen();
    stat = system(p);

    _restore_effective_uid(euid,egid);

#ifdef CURSES
    /* Wait for user to press something before we clear the screen again: */
    fprintf(stderr,"<Press enter to return to %s>\n",progname);
    getch();
#endif

    init_screen();

    if((cmdcnt < 2) && stat)
     {
       errlog((E1,"Couldn't execute %s, stat=%u, errno=%u\n",
           p,stat,errno));
     }
}



SAY()
{
    if(!own_gamep()) { return(0); }

    if(cmdcnt < 2)
     {
       pout(("Usage: SAY The message text.\n"));
       pout(("(Sends a message to your opponent in this game.)\n"));
       return(0);
     }

    /* Send the message to the opponent: */
    return(gen_tell(_names_[!_owncolor_],'S'));
}

DOT()
{
    if(cmdcnt < 2)
     {
       pout(("Usage: . The message text.\n"));
       return(0);
     }

    return(gen_tell(".",'D'));
}

TELL()
{
    if(cmdcnt < 3)
     {
       pout(("Usage: TELL player.name The message text.\n"));
       return(0);
     }

    return(gen_tell(conv_upper(*(cmds+1)),'T'));
}

SHOUT()
{
    MYINT count;

    if(cmdcnt < 2)
     {
       pout(("Usage: SHOUT The message text.\n"));
       pout(("(Sends a message to all players currently online.)\n"));
       return(0);
     }

    count = find_all_online(reconstruct_cmdline(cmds+1),'B',_everybody_);
    fout((OB,"Message sent to %lu player%s.\n",count,
      ((count == 1) ? "" : "s"))); /* Use plural if count != 1. */
}

/* Now W and WHO commands are assigned to STATS() function.
WHO()
{
    MYINT count;

    count = find_all_online(NULL,0,_everybody_);
    fout((OB,"%lu player%s online.\n",count,
      ((count == 1) ? "" : "s")));
}
 */
 

SET()
{
    pout(("Enter HELP SET to see what set commands are available.\n"));
}


SET_ADDRESS()
{
    char *address;
    char *p=NULL;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET ADDRESS your-email-address  (Or use NONE if you have no address)\n"));
       return(0);
     }

    address = *(cmds+1);
    if(strlen(address) > MAX_ADR_LENGTH)
     {
       fout((OB,
"?Sorry, your address is too long. Max. length is %u.\n",
           MAX_ADR_LENGTH));
       return(0);
     }

    /* At least one @, %, !, / or : is required. */
    if((!strchr(address,'@') && !strchr(address,'%') && !strchr(address,'!') &&
       !strchr(address,'/') && !strchr(address,':')) ||
       /* Singlequotes and backslashes are illegal: */
       (p = strchr(address,'\'')) || (p = strchr(address,'\\')))
     { /* If no @ nor ! in address, then check whether it's NONE */
       conv_upper(address);
       if(!strequ(address,"NONE"))
        {
	  fout((OB,
"?Invalid address: %s\n",address));
          if(p) { fout((OB,"Illegal character: %c\n",*p)); }
          return(0);
	}
     }

    update_address(_ownname_,address);

    return(1);
}


SET_DRAGON()
{
    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET DRAGON [OLD | NEW]  (Set how kanji for a 'dragon' is output.)\n"));
       return(0);
     }

    conv_upper(*(cmds+1));
    if(strequ(*(cmds+1),"OLD"))      { _flags_ = setdragonflag(_flags_,1); }
    else if(strequ(*(cmds+1),"NEW")) { _flags_ = setdragonflag(_flags_,0); }
    else { goto ertzu; }

    return(1);
}


SET_KANJI()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout(("Usage: SET KANJI [1,2,3]  (Set kanji code to be used.)\n"));
       pout((" 1 = JIS."));
#ifdef CURSES
       pout((" (Your terminal type must be set to vt102-jis !)"));
#endif
       pout(("\n 2 = EUC (= DEC-Kanji).\n"));
       pout((" 3 = Shift-JIS.\n"));
       return(0);
     }

    n = atoi(*(cmds+1));
    if((n < 1) || (n > 3)) { goto ertzu; }

    _flags_ = setkanjicode(_flags_,n);

    return(1);
}


SET_MAILNOTIFY()
{
    ULI flags;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET MAILNOTIFY [YES | NO]\n"));
       return(0);
     }

    conv_upper(*(cmds+1));
    if(strequ(*(cmds+1),"YES"))      { flags = setmailnotifyflag(_flags_,1); }
    else if(strequ(*(cmds+1),"NO"))  { flags = setmailnotifyflag(_flags_,0); }
    else { goto ertzu; }

    if(flags != _flags_) /* Flags changed? */
     { update_flags(_ownname_,flags); _flags_ = flags; }

    return(1);
}


SET_PSTYLE()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET PSTYLE [0,1,2,3,4]  (Set language used when referring to pieces.)\n"));
       pout((" 0 = English\n"));
       pout((" 1 = Romaji long\n"));
       pout((" 2 = Romaji short\n"));
       pout((" 3 = Kanji  long\n"));
       pout((" 4 = Kanji  short\n"));
       return(0);
     }

    n = atoi(*(cmds+1));
    if((n < 0) || (n > 4)) { goto ertzu; }

    if(((n == LONG_KANJI) || (n == SHORT_KANJI)) &&
         (getkanjicode(_flags_) == NO_KANJI))
     { no_kanjicodeselected(); return(0); }

    _flags_ = setpnamestyle(_flags_,n);

    return(1);
}


SET_BSTYLE()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout(("Usage: SET BSTYLE [0,1,2]  (Set style how the board is output.)\n"));
       pout((
" 0 = Ascii.\n"));
       pout((
" 1 = Mixed romaji & kanji, black is shown with kanji, white with romaji.\n"));
       pout((
" 2 = Kanji, both players' pieces are shown with kanji.\n"));
       return(0);
     }

    n = atoi(*(cmds+1));
    if((n < 0) || (n > 2)) { goto ertzu; }

    if(((n == MIXED) || (n == KANJI)) &&
         (getkanjicode(_flags_) == NO_KANJI))
     { no_kanjicodeselected(); return(0); }


    if((n == KANJI) && (getblackcode(_flags_) == getwhitecode(_flags_)))
     {
       fout((OB,
"Warning: You have %s attributes set for black & white pieces. You are\n",
             (getblackcode(_flags_) ? "same" : "no")));
       pout((
"advised to set other or both of them with SET BLACK and/or SET WHITE command.\n"));
     }

    _flags_ = setboardstyle(_flags_,n);

    return(1);
}


SET_BLACK()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET BLACK <attribute-name>\n"));
       return(0);
     }

    if((n = find_modecode(*(cmds+1))) == -1) { return(0); }

    _flags_ = setblackcode(_flags_,n);

    return(1);
}

SET_WHITE()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET WHITE <attribute-name>\n"));
       return(0);
     }

    if((n = find_modecode(*(cmds+1))) == -1) { return(0); }

    _flags_ = setwhitecode(_flags_,n);

    return(1);
}

SET_THREATENED()
{
    int n;

    if(cmdcnt < 2)
     {
ertzu:
       pout((
"Usage: SET THREATENED <attribute-name>\n"));
       return(0);
     }

    if((n = find_modecode(*(cmds+1))) == -1) { return(0); }

    _flags_ = setthreatenedcode(_flags_,n);

    return(1);
}


no_kanjicodeselected()
{
    pout((
"?This option requires kanji terminal. You must first set kanji code to be used\n(JIS, EUC or Shift-JIS) with a SET KANJI command.\n"));
}

int find_modecode(modename)
char *modename;
{
    int i;
    char modebuf[23];

    if(strequ(modename,"?")) { goto help; }
    strncpy(modebuf,modename,20);
    conv_upper(modebuf);
/* Append * and use wildcard function for matching so it suffices that
    user enters only first couple of letters: */
    strcat(modebuf,"*");

    for(i=0; codenames[i]; i++)
     {
       if(wildcard(modebuf,codenames[i]))
        {
#ifdef CURSES
          if((i > 4) && !has_colors())
	   {
	     pout(("Warning: this terminal type doesn't support colors!\n"));
           }
#endif
	  return(i);
	}
     }

    fout((OB,"?Invalid attribute name: %s   ",modename));
help:
    pout((OB,"Available attributes are:\n"));
    for(i=0; codenames[i];)
     {
       fout((OB,"%s\n",codenames[i++]));
     }
    return(-1);
}


gen_tell(name,status)
char *name;
char status;
{
    int pid;
    MYINT plid;
    ULI seekpos=0L;
    char **fields;
    char dot_used;

    if((dot_used = strequ(name,".")) && !*_dot_)
     {
       pout((
"?No previous usage of tell, please specify the whole name with tell command!\n"));
       return(0);
     }

    if(!(fields = getplfields((dot_used ? _dot_ : name),&seekpos,0)))
     {
       fout((OB,"?Can't find a player called %s\n",name));
       return(0);
     }

    plid = atol(fields[PLF_PLID]);
    pid  = atoi(fields[PLF_PID]);
    name = fields[PLF_NAME];
    strcpy(_dot_,name); /* Copy the latest name used to _dot_ */

    fout((OB,"Sending message to player %s.\n",name));
    gen2tell(name,plid,pid,((status == 'D') ? 'T' : status), /* DOT = TELL */
              reconstruct_cmdline(cmds+1+(status == 'T')));
}


gen2tell(name,plid,pid,status,message)
char *name;
MYINT plid;
int pid;
char status;
char *message;
{
#ifdef UNIX /* Check that there is process with pid 'pid': */
    if(pid && kill(pid,0)) { pid = 0; }
#endif
    /* If player is not online and this is not called from SHOUT command,
        then ask whether player really wants to leave a message: */
    if(!pid && (status != 'B'))
     {
       fout((OB,"?Player %s is not logged on. Leave the message anyway? ",
          name));
       if(tolower(ynq("<y/n>")) != 'y')
        {
	  pout(("?Message not left.\n"));
	  return(0);
	}
     }

    /* Don't leave the message if status == 'B' (i.e. SHOUT) and pid is zero */
    if(pid || (status != 'B'))
     { addtomlist(plid,message,status); }

    if(pid) { notify(pid,MESSAGE_SENT); }    
}


int checkmessages()
{
    FILE *fp;
    char *p,*verb;
    char stat;
    char buf[MAXBUF+3];
    char filename[SMALLBUF];
    char frombuf[SMALLBUF],timebuf[SMALLBUF];

    sprintf(filename,"%lu.msg",_ownplid_);

    if(!(fp = lockfopen(filename,"Tr+")))
     { return(0); }

    while(fgets(buf,MAXBUF,fp))
     {
       beep();
       stat = *buf;
       if(!(p = getfirstword(frombuf,(buf+1))))
        { strcpy(frombuf,"???"); goto next; }
       if(!(p = getfirstword(timebuf,p)))
        { next: strcpy(timebuf,"???"); p = buf; }
       else { p = skip_blankos(p); }
       switch(toupper(stat))
        {
          case 'M': { verb = NULL; break; }
	  case 'B': { verb = (("shouts")); break; }
	  case 'S': { verb = (("says"));   break; }
	  case 'T': { verb = (("tells you"));  break; }
	  default:  { verb = (("growls")); p = buf; } /* Bug there. */
        }
       if(verb)
        {
	  fout((OB,"%s %s",frombuf,verb));
          fout((OB," (%s):\n%s",timebuf,p));
	}
       else /* System Message. */
        {
	  fout((OB,"<%s (%s)>\n",nuke_newline(p),timebuf));
	}
     }

    myunlink(filename,"checkmessages");
/* Is it possible that some other process could open the file between
   closing and unlinking? If it is, then that process' message is lost. */
    lockfclose(fp);
    return(1);
}


MYINT checknewmoves(stat_of_all)
int stat_of_all;
{
    MYINT m,gid,gamecount=0;
    int color;
    GIDLIST glist;
    char status;
    char movebuf[MAXBUF+3],timebuf[SMALLBUF];

alku:
    /* Check all the games in gidlist, whether new move has been made
        in any of them: */
    for(glist=_gidlist_; glist; glist = cdr(glist))
     {
       color = colcar(glist);
       gid   = gidcar(glist);
       if((m=get_next_move(gid,&status,movebuf,timebuf,color,1)) == ERRMOVES)
        {
ertzu:
          errlog((E1,
"**Internal error in checknewmoves(): Can't open a file %lu.%s for reading!\n",
               gid,((m == ERRMOVES) ? "lm" : "m")));
          ertzu_exit(101);
        }

/* If it is our turn to move, then letter is lowercase, otherwise upper: */
       if(get_parity(m) == color) { status = tolower(status); }
       /* If new move was made by an opponent: */
       if((status == 'p') || (status == ('p'+1)))
        {
	  gamecount++;
/* Debugging.
	  fout((OB,"_owncolor_=%u, color=%u, _movesmade_=%lu, m=%lu\n",
	          _owncolor_,color,_movesmade_,m));
 */
	  /* If that game is not loaded in, or is loaded only partially,
	      then load it again, completely: */
	  if((gid != _gid_) || (m != (_movesmade_+1)))
	   {
	     _gid_ = gid;
	     /* Then load the moves of that game: */
	     if(loadmoves(_gid_,0,0) == ERRMOVES) { goto ertzu; }
	   }

	  strcpy(_latest_move_,movebuf);

	  _owncolor_ = color;
	  _movesmade_ = m;

#ifdef SOME_KLUDGOHUMUNGOUS_CODE_COMMENTED_OUT
          if(strequ(movebuf,"RESIGN")) /* If opponent has resigned. */
	   {
	     _movestat_ = RESIGNED;
             fout((OB,"Game %lu: your opponent has resigned.\n",_gid_));
	   }
	  else if(cutstrequ(movebuf,"HANDICAP:"))
	   {
	     if(!(_movestat_ = check_move(movebuf,_owncolor_,0)))
	      { goto ertzu2; }
	     fout((OB,
"Game %lu: your opponent has removed the handicap pieces.\n",_gid_));
             pout(("Now it's your opponent's turn again.\n"));
	     /* Cheat the system so that it's thought that black has made
	         this handicap move, and now it should be white's turn
		  again: (argh, this code is growing too kludgy... ;-) */
	     _movesmade_ = m = 1;
             put_next_move(_gid_,_movesmade_,movebuf,get_timebuf(),'S');
	     goto write_to_log; /* And too much noodles. */
	   }
#endif
          /* Do the move, and check its validity: */
       /* else */ if(_movestat_ = check_move(movebuf,!_owncolor_,0))
           {
             beep();

             fout((OB,"Game %lu: your opponent has made a move: %s\n",
	             _gid_,movebuf));
             show_checks(stdout,_movestat_,_owncolor_,_captured_piece_,
	           checkmsgbuf,escapemsgbuf);
           }
          else /* If move was invalid. */
           {
ertzu2:
             errlog((E1,
"**Internal error in checknewmoves, m=%lu, movebuf=%u/%s\n",
	          m,strlen(movebuf),movebuf));
	     errlog((E1,E2));
	     ertzu_exit(101);
           }

          /* Game finished? Move it from glist to clist and do some final
	     cleanup. */
          if(EOG(_movestat_))
	   { /* Some local variables: */
             MYINT ngid,blackplid,whiteplid;
	     char  blackname[SMALLBUF],whitename[SMALLBUF];

	     /* Move the last move from .lm file to movelog: */
	     addtomovelog(_gid_,m,_movestat_,_captured_piece_,movebuf,timebuf,
	                   checkmsgbuf,escapemsgbuf);

/*
Try to remove the gid from glist: (and copy the corresponding line to movebuf)
 */
	     if(!rm_glist(_gid_,movebuf))
	      {
	        errlog((E1,
"**Internal error in checknewmoves, couldn't find a gid %lu from %s!\n",
                     _gid_,GLISTFILE));
		errlog((E1,"_movestat_=%d\n",_movestat_));
	        goto ertzu2;
	      }

             readgidlist(); /* Clear the old gidlist and read the new one. */

             sscanf(movebuf,"%ld %ld %ld %s %s",
	       &ngid,&blackplid,&whiteplid,blackname,whitename);

           /* Mark this game as completed: (_eog_ contains winner color + 1) */
             _eog_ = ((_movestat_ == RESIGNED) ? _owncolor_ : !_owncolor_)+1;

	     pout((((_movestat_ == RESIGNED) ? "YOU WON!\n" : "YOU LOST!\n")));
	     sprintf(movebuf,"%u %c %4lu %lu %lu %lu %s %s %s",
                      (_eog_-1),
		      ((_movestat_ == RESIGNED) ? 'R' :
		          ((_movestat_ == KING_MATED) ? 'M' : 'C')),
			  _movesmade_,ngid,blackplid,whiteplid,
			   blackname,whitename,get_timebuf());

             addtoclist(movebuf); /* And add line to completed games list. */

	     /* Try to delete the last move file: */
	     sprintf(timebuf,"%lu.lm",_gid_);
	     myunlink(timebuf,"checknewmoves");
             /* Don't do this if playing against itself. */
             if(_blackplid_ != _whiteplid_)
	      { /* Increment wins & losses counts in players file: */
	        update_wins_et_losses(_names_[_eog_-1],_names_[!(_eog_-1)]);
	        notify_opponent(!_owncolor_,GAME_FINISHED);
              }
 /* Start the loop again, because we read new gidlist and if we continue
    following this old one, we follow just garbage. (because old gidlist
    was freed) */
	     goto alku;
	   }

	} /* if(status == 'p') */
       else if(stat_of_all) /* Called from STAT command. */
        {
	  gamecount++;
          fout((OB,"Game %lu: ",gid));
          switch(status)
	   {
	     case 'P': case 'P'+1:
	      {
	        pout((
"you have made your move, but your opponent hasn't seen it yet.\n"));
                break;
	      }
	     case 'U':
	      {
	        pout((
"your opponent's latest move has been undone.\n"));
                break;
              }
	     case 'u':
	      {
	        pout((
"your latest move has been undone. It's your turn to move.\n"));
                break;
              }
	     case 'S': case 'S'+1:
	      {
	        pout((
"your opponent has seen your move, but hasn't replied yet.\n"));
                break;
              }
	     case 's': case 's'+1:
	      {
	        pout((
"you have seen your opponent's move. It's your turn to move.\n"));
                break;
              }
             default:
	      {
                errlog((E1,
"**Internal error in checknewmoves, status=%c, m=%lu, movebuf=%u/%s timebuf=%s\n",
	          status,m,strlen(movebuf),movebuf,timebuf));
		break;
	      }
	   }
	}
     }

    return(gamecount);
}


/*
   Add movecounth move (in movebuf) plus its timestamp (in timebuf)
   to the movelog of the game gid, and optionally also checkmsg
   and escapemsg if they are specified.
 */
addtomovelog(gid,movecount,movestat,capt,movebuf,timebuf,checkmsg,escapemsg)
MYINT gid,movecount;
int movestat,capt;
char *movebuf,*timebuf;
char *checkmsg,*escapemsg;
{
    FILE *fp;
    char tmpbuf[MAXBUF+3];

    /* Open the movelog file: */
    sprintf(tmpbuf,"%lu.m",gid);

    if(!(fp = lockfopen(tmpbuf,"a")))
     {
       ertzu_exit(101);
     }

    fputs(sprintmove(movecount,tmpbuf,movebuf,timebuf),fp);
    if(capt != EMPTY)
     { fprintf(fp,"  The %s is captured.",getpiecename(capt)); }
    fprintf(fp,"\n"); /* And a newline. */

    if(*checkmsg)
     {
       fprintf(fp,"# %s's %s.\n",
        getcapcolname((movestat == KING_LEFT_IN_CHECK) ? !_owncolor_
	                                               : _owncolor_),
	 checkmsg);
     }
    if(*escapemsg)
     {
       fprintf(fp,"# %s%s.\n",getcapcolname(_owncolor_),escapemsg);
     }

    lockfclose(fp);
}


show_checks(fp,movestat,color,capt,checkmsg,escapemsg)
FILE *fp;
int movestat,color,capt;
char *checkmsg,*escapemsg;
{
    if(capt != EMPTY)
     {
       sprintf(OB,"The %s is captured.\n",getpiecename(capt));
       output_stuff(OB,fp);
     }

    if(*checkmsg)
     {
       sprintf(OB,"%s %s.\n",
        whosking((movestat == KING_LEFT_IN_CHECK) ? !color : color),
	 checkmsg);
       output_stuff(OB,fp);
     }
    if(*escapemsg)
     {
       sprintf(OB,"%s%s.\n",getcapcolname(color),escapemsg);
       output_stuff(OB,fp);
     }
}


char *whosking(color)
int color;
{
    static char whosbuf[SMALLBUF];

/* Use color names if it is not own game, or if it is game against self: */
    if((_owncolor_ == NOCOLOR) || (_blackplid_ == _whiteplid_))
     {
       sprintf(whosbuf,"%s's",getcapcolname(color));
     }
    else /* Otherwise use strings Your or Opponent's */
     {
       strcpy(whosbuf,((color == _owncolor_) ? "Your" : "Opponent's"));
     }
    return(whosbuf);
}


char *sprintmove(movecount,destbuf,movebuf,timebuf)
MYINT movecount;
char *destbuf,*movebuf,*timebuf;
{
    sprintf(destbuf,"%lu%s%s%s# %s",
                movecount,(get_parity(movecount) ? "\t" : "\t\t"),
	        movebuf,(get_parity(movecount) ? "\t\t" : "\t"),timebuf);
    return(destbuf);
}


dumpgidlist()
{
    GIDLIST glist;

    for(glist=_gidlist_; glist; glist = cdr(glist))
     {
       fout((OB,"gid=%lu  color=%u\n",gidcar(glist),colcar(glist)));
     }
}


#ifdef SOME_KLUDGOHUMUNGOUS_CODE_COMMENTED_OUT

int getcheckmsg(checkmsgbuf,color,movestat)
char *checkmsgbuf;
int color,movestat;
{
    int result;
    COR vec[MAXTHREATS];

    *checkmsgbuf = *escapemsgbuf = '\0'; /* Clear them. */
    result =
     ((movestat == KING_CAPTURED) ? KING_CAPTURED :
       (checkedp(vec,color) ? (checkmatedp(color) ? KING_MATED : KING_CHECKED)
       : 0));

    if(result)
     {
       if(result == KING_CAPTURED)
        { sprintf(checkmsgbuf,"king was captured"); }
       else
        {
          sprintf(checkmsgbuf,"king is %s by ",
                    ((result == KING_MATED) ? "mated" :
	              ((strlen(vec) == 2) ? "doublechecked" : "checked")));
          sprintcoords_and(checkmsgbuf,vec);
	}
     }

    return(result);
}

#endif


/*
   Usage:

   board        Print current board.
   board -o     Print current board as seen from opponent's view point.
   board num    Print board num as seen from viewpoint of player to move.
   board -o num Print board num as seen from other player's view point.
 */
int BOARD()
{
    MYINT org_gid;
    char *filename;
    int other_flag=0;
    int argi=1;
    int euid,egid;
    MYINT gid_to_use;

    org_gid = _gid_;

    if((cmdcnt > 1) && (**(cmds+argi) == '>') && *(1+*(cmds+argi)))
     {
       filename = (1+*(cmds+argi));
       argi++;
       cmdcnt--;
     }
    else { filename = NULL; }

    if((cmdcnt > 1) && strequ(conv_upper(*(cmds+argi)),"-O"))
     {
       argi++;
       cmdcnt--;
       other_flag=1;
     }

    if(cmdcnt > 1)
     {
       if(numberp(*(cmds+argi))) { gid_to_use = atol(*(cmds+argi)); }
       else
        {
	  pout(("Usage:  BOARD [>filename] [-o] [gid]\n"));
	  return(0);
	}
     }

    if(!gid_to_use)
     {
       pout((
"?No game selected, use  BOARD <gid>  to display board from game <gid>.\n"));
       return(0);
     }

    show_board(org_gid, gid_to_use, other_flag, filename);
}


int show_board(MYINT org_gid, MYINT gid_to_use, int other_flag, char *filename)
{
    FILE *fp;
    int color;
    int argi=1;
    int euid,egid;

    _gid_ = gid_to_use;

    if(_gid_ != org_gid) /* If gid changes. */
     {
       if(loadmoves(_gid_,1,0) == ERRMOVES) /* Then load game in. */
        {
          fout((OB,"?Can't find movelog for the game %lu !\n",_gid_));
          _gid_ = 0;
	  return(0);
        }
     }

       if(filename)
        {
          if(!restore_real_uid(&euid,&egid)) { return(0); }
          if(!(fp = fopen(filename,"a")))
           {
	     _restore_effective_uid(euid,egid);
	     fout((OB,"?Can't open file %s for writing!\n",filename));
	     return(0);
	   }
        }
       else { fp = stdout; }
       

       /* If playing against itself, then set color to be of that side
           which is to move: */
       if((_blackplid_ == _whiteplid_) && (_blackplid_ == _ownplid_))
        {
	  _owncolor_ = get_parity(_movesmade_);
	  /* Except if other_flag specified, then change the color: */
	  if(other_flag) { _owncolor_ = !_owncolor_; }
	  printboard(_owncolor_,fp,_flags_);
	}
       else
        {
          color = _owncolor_ = getowncolor();
          if(color == NOCOLOR) { color = get_parity(_movesmade_); }
          printboard((other_flag ? !color : color),fp,_flags_);
        }

    if(filename)
     {
       _restore_effective_uid(euid,egid);
       fclose(fp);
     }
}



int MOVES()
{
    int arg_i=1;
    char nonstop=0;

    if((cmdcnt > 1) && strequ(conv_upper(*(cmds+1)),"-N"))
     {
       arg_i++;
       cmdcnt--;
       nonstop=1;
     }

    if(cmdcnt > 1)
     {
       if(numberp(*(cmds+arg_i))) { _gid_ = atol(*(cmds+arg_i)); }
       else
        {
ertzu:
          pout(("Usage: MOVES [-n] [gid]\n"));
          return(0);
	}
     }

    if(!_gid_)
     {
       pout(("?No default game!\n"));
       goto ertzu;
     }

    if(loadmoves(_gid_,1,(2-nonstop)) == ERRMOVES)
     {
       fout((OB,"?Can't find movelog for the game %lu !\n",_gid_));
       _gid_ = 0;
     }
/* Set the _owncolor_ according to this game whose log was just shown:
   (Fixed 30-NOV-92) */
    _owncolor_ = getowncolor();

}


NEW()
{
    int pid;
    ULI zero=0L;
    char *pattern,*mp,*comment;
    char buf[MAXBUF+3];
    char movebuf[MAXBUF+3];

    if(cmdcnt < 2) /* No args. */
     {
       show_wlist();
ertzu:
       pout((
"Enter HELP NEW to see all the usages of NEW command.\n"));
     }
    else if(numberp(*(cmds+1))) /* If numeric argument. */
     {
       start_new_game(atol(*(cmds+1)));
     }
    else /* There is pattern, move and possibly some comment. */
     {
       if(cmdcnt < 3) /* But if no move specified. */
        {
	  pout(("?NEW: missing argument.\n"));
	  goto ertzu;
	}

       pattern = conv_upper(*(cmds+1));

       if(!getplfields(pattern,&zero,0))
        {
          fout((OB,
"?There is no player which would match to %s ! Leave the challenge anyway? ",
                  pattern));
          if(tolower(ynq("<y/n>")) != 'y')
           {
	     pout(("?Challenge not left.\n"));
	     return(0);
	   }
	}

       strcpy(movebuf,*(cmds+2)); /* Get the copy of move. */

       conv_upper(movebuf);

       mp = movebuf;

       if(*movebuf == 'W') /* If this player wants to play white. */
        { /* And if waiting for black's move, then do nothing now: */
          if(!*(movebuf+1)) { }
	  else
	   {
             _gid_ = 0;
             initboard();
             _owncolor_= 1; /* We are white. */
             _whiteplid_ = _ownplid_;
             _blackplid_ = 0;
             strcpy(_whitename_,_ownname_);
             strcpy(_blackname_,pattern);
	     mp++;
	     goto benin;
           }
	}
       else /* This player wants to play black. */
        { /* NEW pattern RESIGN  won't work correctly anyway: */
	  if(strequ(mp,"RESIGN"))
	   {
	     pout(("Pretty depressive mood today, eh?\n"));
	     goto biafra;
	   }
          _gid_ = 0;
          initboard();
          _owncolor_= 0; /* We are black. */
          _blackplid_ = _ownplid_;
          _whiteplid_ = 0;
          strcpy(_blackname_,_ownname_);
          strcpy(_whitename_,pattern);
benin: /* Do the move (and check that it is valid) and print the board. */
	  _movesmade_ = 1; /* One move made. */
	  strcpy(_latest_move_,mp);
          if(!(_movestat_ = check_move(mp,0,1)))
           {
             pout((E2));
	     pout(("?Challenge not left.\n"));
biafra:
	     return(0);
           }
	  else
	   {
	     --_movesmade_;
	   }
        }

       comment = reconstruct_cmdline(cmds+3);

       sprintf(buf,"%-20s %-20s %-7s %s%s%s",
           _ownname_,pattern,movebuf,get_timebuf(),
           (comment ? "  " : ""),(comment ? comment : ""));
       addtowlist(buf);

/*     add_new_game(movebuf,reconstruct_cmdline(cmds+3)); */
     }
}


/*
MYINT add_new_game(move,comment)
char *move,*comment;
{
    char buf[MAXBUF+3];

    sprintf(buf,"%-20s %-7s %s%s%s",_ownname_,move,get_timebuf(),
     (comment ? "  " : ""),(comment ? comment : ""));
    return(addtowlist(buf));
}
 */


MYINT start_new_game(n)
MYINT n;
{
    FILE *fp;
    MYINT oplid;
    ULI zero=0L;
    char *movebuf;
    char **fields;
    char buf1[MAXBUF+3];
    char buf2[SMALLBUF];
    char pattern[MAXBUF+3];
    char space_for_movebuf[MAXBUF+3],timebuf[SMALLBUF];

    if(!check_gamenum(n)) { return(0); }

    strcpy(buf1,wlist[n]);

    movebuf = space_for_movebuf;
    if(sscanf(buf1,"%s %s %s %s",buf2,pattern,movebuf,timebuf) != 4)
     { goto ertzu; }

    if(!wildcard(pattern,_ownname_))
     {
       fout((OB,
"?Sorry, your name doesn't match to %s !\n",pattern));
       return(0);
     }

    /* If can't find & remove this match request from wlist file,
       then that user has probably just removed it from file: */
    if(!rm_wlist(buf1)) { return(0); }

    if(!(fields = getplfields(buf2,&zero,0)))
     {
       errlog((E1,"**Can't find a player with name %s\n",buf2));
       return(0);
     }

    oplid = atol(fields[PLF_PLID]); /* Get opponent's plid. */

    fout((OB,"Starting a game with player %s.\n",buf2));

    _gid_ = getnextgid();
    initboard();

    if(*movebuf == 'W') /* If opponent wants to play white. */
     {
       _blackplid_ = _ownplid_;
       strcpy(_blackname_,_ownname_);
       _whiteplid_ = oplid;
       strcpy(_whitename_,buf2);
       _owncolor_ = 0;

       if(!*(movebuf+1))
        { /* If nothing specified after W, then it's black to move. */
          _movesmade_ = 0;
	  goto togo;
        }
/* However, if there is something after W, then we (black) are forced to
    do that move: */
       else { movebuf++; }
     }
    else /* Opponent (black) plays first move. */
     {
       _blackplid_ = oplid;
       strcpy(_blackname_,buf2);
       _whiteplid_ = _ownplid_;
       strcpy(_whitename_,_ownname_);
       _owncolor_ = 1;
     }

       _movesmade_ = 1;

       if(!(_movestat_ = check_move(movebuf,0,0)))
        {
ertzu:
	  errlog((E1,"**Internal error in start_new_game, buf=%u/%s\n",
	    strlen(buf1),buf1));
	  errlog((E1,E2));
	  errlog((E1,"movebuf=%u/%s\n",strlen(movebuf),movebuf));
	  ertzu_exit(101);
	}

togo:
    /* Open movelogfile: */
    sprintf(buf2,"%lu.m",_gid_);
    if(!(fp = lockfopen(buf2,"w")))
     {
       ertzu_exit(101);
     }
    fprintf(fp,"# %-20s  %-20s\n",timebuf,get_timebuf());
    fprintf(fp,"# %-20s  %-20s\n",_blackname_,_whitename_);
    fprintf(fp,"# %-20lu  %-20lu\n",_blackplid_,_whiteplid_);
/* Commented out. Now this is done later, when opponent replies something.
    if(_movesmade_)
     {
       fputs(sprintmove(_movesmade_,buf1,movebuf,timebuf),fp);
       fprintf(fp,"\n");
     }
 */
    lockfclose(fp);

    /* If we (black) were forced to do some move. */
    if(!_owncolor_ && _movesmade_)
     {
       put_next_move(_gid_,_movesmade_,movebuf,timebuf,('P'+1));
     }
    else
     {
       put_next_move(_gid_,_movesmade_,movebuf,get_timebuf(),('S'+1));
     }

    sprintf(buf1,"%-10lu %-10lu %-10lu %-20s %-20s",
                   _gid_,_blackplid_,_whiteplid_,_blackname_,_whitename_);

    addtoglist(buf1);

    readgidlist();

    notify_opponent(!_owncolor_,GAME_STARTED);

/* Actually not needed, because when GAME_STARTED signal is received,
   then also checknewmoves is executed, as with MOVE_MADE.
    if(!_owncolor_ && _movesmade_)
     { notify_opponent(!_owncolor_,MOVE_MADE); }
 */

/*
    _gidlist_ = consgl(_gid_,_owncolor_,_gidlist_);
    addtopglist(_blackplid_,_gid_,0);
    if(_blackplid_ != _whiteplid_) { addtopglist(_whiteplid_,_gid_,1); }
 */
}


MYINT remove_game(n)
MYINT n;
{
    char buf1[MAXBUF+3],buf2[MAXBUF+3];

    if(!check_gamenum(n)) { return(0); }
    
    strcpy(buf1,wlist[n]);
    takefirstword(buf2,buf1);
/*  pertospc(buf2); */
    pout(("Remove this match request:\n"));
    fout((OB,"%-20s %s\n",buf2,skip_blankos(buf1)));
    if(tolower(ynq("<y/n>")) != 'y') { return(0); }
    if(!strequ(buf2,_ownname_) && !_su_flag_)
     {
       pout(("?It's not your request!\n"));
       return(0);
     }

    return(rm_wlist(wlist[n]));
}


MYINT check_gamenum(n)
MYINT n;
{
    if(!n || (n > _wlistsize_) || !wlist[n])
     {
       pout(("?Number out of range.\n"));
       return(0);
     }
    else { return(n); }
}

REMOVE()
{
    if(cmdcnt < 2)
     {
       pout((
"?You must specify the request number! (Use NEW to see them.)\n"));
       return(0);
     }
    return(remove_game(atol(*(cmds+1))));
}

GAMES()
{
    show_glist();
}




MYINT loadmoves(gid,readlastmove,output_them)
MYINT gid;
int readlastmove,output_them;
{
    MYINT m,linecount;
    register int whosturn;
    int sc;
    char status,c,still_looping;
    FILE *fp;
    char buf[MAXBUF+3];
    char buf2[MAXBUF+3];
    char timebuf[MAXBUF+3];

    sprintf(buf,"%lu.m",gid);

    if(!(fp = myfopen(buf,"Sr")))
     {
       return(ERRMOVES);
     }

    initboard();
    whosturn = 0;
    linecount = 1;

    if(!fgets(buf,MAXBUF,fp)) { goto ertzu; }
    if(output_them) { pout((buf)); }
    if(!fgets(buf,MAXBUF,fp)) { goto ertzu; }
    if(output_them) { pout((buf)); }
    if((sscanf(buf,"%c %s %s",&status,_blackname_,_whitename_) != 3) ||
       (status != '#')) { goto ertzu; }
    if(!fgets(buf,MAXBUF,fp)) { goto ertzu; }
    if(output_them) { pout((buf)); }
    if((sscanf(buf,"%c %ld %ld",&status,&_blackplid_,&_whiteplid_) != 3) ||
       (status != '#')) { goto ertzu; }

    still_looping = 1;
    while(still_looping)
     {
       if(fgets(buf,MAXBUF,fp)) /* If still lines in gid.m movelog file? */
        {
          if((*buf != '#') &&
              (((sc = sscanf(buf,"%ld %s",&m,buf2)) != 2) ||
	      ((_movesmade_+1) != m)))
           {
	     errlog((E1,
"**Internal error in loadmoves(%lu,%d,%d): m (%lu) != _movesmade_+1 (%lu+1). sc=%u\n",
               gid,readlastmove,output_them,m,_movesmade_,sc));
	     errlog((E1,"buf=%u\n%s\n",strlen(buf),buf));
	     return(ERRMOVES);
	   }
        }
    /* Now read the last move only if readlastmove argument is specified,
       and if the last move is not undone. */
       else if(readlastmove
        && ((m = get_next_move(gid,&status,buf2,timebuf,0,0)) != ERRMOVES)
	&& m /* Don't read "moves" like 0 W from .lm file. */
        && (status != 'U'))
        {
          still_looping = 0; /* Don't do this second time. */
 /* Important bug fix at 30-NOV-92. Now BOARD command loads the whole game
     even if there are comments as last lines of gid.m file: */
	  *buf = '\0';
          if(output_them) /* Construct moveline for the last move. */
           {
             sprintmove(m,buf,buf2,timebuf);
	     if((status == 'P') || (status == ('P'+1)))
	      { strcat(buf," (Not yet seen by an opponent)\n"); }
             else if(status == 'U') /* This case never encountered. */
	      { strcat(buf," (Undone)\n"); }
	     else { strcat(buf,"\n"); }
	   }
        }
       else { break; }

       if(output_them)
        {
	  if((output_them == 2) && _morelines_ && !--linecount)
	   {
moreloop:
	     c = more("--More-- <p,o,n,y,<space>,<cr>,q>");
	     switch(tolower(c))
	      {
	        case 'n': case 'q': { goto out_of_loop; }
	        case 'p':
	         {
		   printboard(whosturn,stdout,
		               (isupper(c) ? DEFAULT_FLAGS : _flags_));
		   goto moreloop;
		 }
	        case 'o':
	         {
		   printboard(!whosturn,stdout,
		               (isupper(c) ? DEFAULT_FLAGS : _flags_));
		   goto moreloop;
		 }
	        case ' ': case 'y': { linecount = _morelines_; break; }
	        default: { linecount = 1; break; }
	      }
	   }
          pout((buf));
	}

       if(*buf == '#') { continue; } /* Skip comments. */

       _movesmade_++;
       strcpy(_latest_move_,buf2);

       if(!(_movestat_ = check_move(_latest_move_,whosturn,0)))
        {
	  errlog((E1,"**Internal error in loadmoves(%lu,%d,%d):\n",
	    gid,readlastmove,output_them));
	  errlog((E1,E2));
	  errlog((E1,
	   "buf=%u/%s _latest_move_=%u/%s  whosturn=%u  _movesmade_=%u\n",
	           strlen(buf),buf,strlen(_latest_move_),_latest_move_,
		   whosturn,_movesmade_));
          return(ERRMOVES);
	}

       /* Don't try to read gid.lm file if End Of Game is detected here: */
       if(EOG(_movestat_) && still_looping)
        { 
	  readlastmove = 0;
          /* Mark this game as completed: */
          _eog_ = ((_movestat_ == RESIGNED) ? !whosturn : whosturn)+1;
        }
       whosturn = !whosturn;
     }
out_of_loop: ;

    return(_movesmade_);

ertzu:
    errlog((E1,
"**Internal error in loadmoves(%lu,%d,%d), invalid line in file:\n",
      gid,readlastmove,output_them));
    errlog((E1,"%u/%s\n",strlen(buf),buf));
    return(ERRMOVES);
}



char **getplfields(pattern,seekptr,only_online)
char *pattern;
ULI *seekptr;
int only_online;
{
    MYINT linecnt,plid;
    int pid,killstatus=0;
    FILE *fp;
    char **result;
    char *funname="getplfields";
    static char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       return(0);
     }

    myfseek(fp,*seekptr,0,funname); /* Seek to the *seekptr */

    result = NULL;

    while(cut_line(fp,seekptr,seekpositions,fields) != EOF)
     {
       linecnt++;
       pid = 0;
       /* If name matches to pattern? */
       if(wildcard(pattern,fields[PLF_NAME]) &&
        /* And only_online is not specified, or there is a non-zero pid: */
          (!only_online || (pid = atoi(fields[PLF_PID]))))
        {
	  plid = atol(fields[PLF_PLID]);
#ifdef UNIX
          /* Check whether this pid is valid? */
          if(pid && (killstatus = kill(pid,0)))
           {
             errlog_only((E1,
              "NOT ONLINE: %s %lu %u %s line=%lu, errno=%u\n",
                 fields[PLF_NAME],plid,pid,fields[PLF_TIME],linecnt,errno));

             /* Overwrite the pid field with zero. */
             myfseek(fp,seekpositions[PLF_PID],0,funname);
             fprintf(fp,"%010u",0); fflush(fp);
             /* And restore the file pointer back to current location: */
             myfseek(fp,*seekptr,0,funname);
	     continue;
	   }
#endif
          result = fields;
	  break;
        }
     } /* while */

    lockfclose(fp);
    return(result);
}

ULI seekplrecord(fp,pattern,funname)
FILE *fp;
char *pattern;
char *funname;
{
    /* If this pattern is equal to player's name and _ownseekptr_ is set
        then shortcut directly to the players record: */
    if(strequ(pattern,_ownname_) && (_ownseekptr_ != NOTSET_SEEKPTR))
     {
       myfseek(fp,_ownseekptr_,0,funname);
       return(_ownseekptr_);
     }

    return(0);
}


/* This returns player id of player with name 'name',
   but if that is not found from file then adds it to
   the end of file with new player id, and returns that.
   Also updates the pid & time fields of the user.
   If clearpid is nonzero, then player's (corresponding to name)
   pid is overwritten with zero, and flags is overwritten with
   value of _flags_.
 */
MYINT get_or_addplid(name,clearpid)
char *name;
int clearpid;
{
    MYINT plid,maxplid=0,linecnt=0,newcount;
    ULI flags;
    FILE *fp;
    ULI charcount=0;
    int newpid;
    int i;
    char player_found=0; /* A flag. */
    char *funname = "get_or_addplid";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       ertzu_exit(101);
     }

/*  myrewind(fp,funname); */
    charcount = seekplrecord(fp,name,funname);

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       plid = atol(fields[PLF_PLID]);
       if(plid > maxplid) { maxplid = plid; }
       if(strequ(name,fields[PLF_NAME]))
        {
	  player_found=1;
	  _ownseekptr_ = seekpositions[PLF_NAME];
	  break;
	}
     }

    if(player_found)
     {
       sscanf(fields[PLF_FLAGS],"%lx",&flags);
       newcount = atol(fields[PLF_COUNT]);

       if(getseclevel(flags) == BANISHED)
        {
	  newpid = 0;
	  newcount++;
	}
       else
        {
	  if(clearpid)
	   {
	     newpid = 0;
	   }
	  else
	   {
	     newpid = getpid();
	     newcount++;
	   }
        }
       
       /* Update the pid, count & time fields: */
       /* Seek to the position of flags: */
       myfseek(fp,seekpositions[PLF_FLAGS],0,funname);
       /* And overwrite it with a new pid (or with zero if clearpid != 0) */
       fprintf(fp,"%08lx %010u %010lu %s",
                (clearpid ? _flags_ : flags),
		newpid,
		newcount,
		get_timebuf());
       lockfclose(fp);
       _get_or_addplid_ = 0;
       _flags_ = flags;
       return(plid);
     }
    /* This is a new player, add it to end of file with new plid: */
    else if(!clearpid)
     {
       _flags_ = DEFAULT_FLAGS;
       _ownseekptr_ = charcount;
       myfseek(fp,charcount,0,funname); /* Seek to the eof */
       /* Not found from file, append name to it: */
       fprintf(fp,"%-21s %010lu %08lx %010u %010lu %s",
                name,++maxplid,_flags_,getpid(),1L,get_timebuf());
       for(i=((MAXPLFIELDS-PLF_TIME)-1-2);i--;) /* Fill the rest with zeros. */
        { /* 2 there ^ stands for fields PLF_ADDRESS & PLF_PWD which are not
	      numeric. */
	  fprintf(fp," %010u",0L);
	}
       fprintf(fp," NONE");
       for(i=(MAX_ADR_LENGTH-4); i--;) /* 4 is length of "NONE" */
        { putc(' ',fp); } /* Fill the rest of field with blanks. */
       fprintf(fp," %s",DEFAULT_PWDFIELD);
       fprintf(fp,"\n");
       lockfclose(fp);
       _get_or_addplid_ = 0;
       return(maxplid);
     }
    else
     {
       lockfclose(fp);
       errlog((E1,"**Internal error in %s(%s,%d): name not found!\n",
          funname,name,clearpid));
       ertzu_exit(101);
     }
}



MYINT find_all_online(shoutmsg,status,pattern)
char *shoutmsg;
int status;
char *pattern;
{
    MYINT plid,linecnt=1,count=0;
    FILE *fp;
    ULI charcount=0;
    int pid,killstatus=0;
    char *funname = "find_all_online";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(shoutmsg) /* If shouting, then open in locked mode. */
     {
       if(!(fp = lockfopen(PLAYERFILE,"r+")))
        {
          ertzu_exit(101);
        }
     }
    else /* If taking who list, then open in shared mode. */
     {
       if(!(fp = myfopen(PLAYERFILE,"r")))
        {
          ertzu_exit(101);
        }
     }

/*  myrewind(fp,funname); */
    charcount = seekplrecord(fp,pattern,funname);

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       linecnt++;

       /* If found player online and his name matches to pattern? */
       if((pid = atoi(fields[PLF_PID])) && wildcard(pattern,fields[PLF_NAME]))
        {
	  plid = atol(fields[PLF_PLID]);
#ifdef UNIX
          if((killstatus = kill(pid,0)))
           { /* Check whether this pid is valid? */
/*           if(!shoutmsg && super_user())
	      { pout((" <Not really online???>")); } */
             errlog_only((E1,
              "NOT ONLINE: %s %lu %u %s line=%lu, errno=%u\n",
                 fields[PLF_NAME],plid,pid,fields[PLF_TIME],linecnt,errno));
             if(shoutmsg)
              {
                /* Overwrite the pid field with zero. */
                myfseek(fp,seekpositions[PLF_PID],0,funname);
                fprintf(fp,"%010u",0); fflush(fp);
                /* And restore the file pointer back to current location: */
                myfseek(fp,charcount,0,funname);
              }
        }
       else
#endif
            if(!shoutmsg)
	     { fout((OB,"%-25s %s\n",fields[PLF_NAME],fields[PLF_TIME])); }

            if(!killstatus)
             {
               count++;
               if(shoutmsg)
                { gen2tell(fields[PLF_NAME],plid,pid,status,shoutmsg); }
             }
        } /* if((pid = atoi(fields[PLF_PID]))) */
     } /* while(cut_line(fp,&charcount,seekpositions,fields) != EOF) */

    if(shoutmsg) { lockfclose(fp); }
    else { fclose(fp); }

    _get_or_addplid_ = 0;
    return(count);
}



int update_wins_et_losses(winner,loser)
char *winner,*loser;
{
    FILE *fp;
    ULI charcount=0;
    char *funname = "update_wins_et_losses";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       ertzu_exit(101);
     }

/*  myrewind(fp,funname); */

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       if(strequ(winner,fields[PLF_NAME]))
        { /* If winner found, then increment his/her wins: */
          myfseek(fp,seekpositions[PLF_WINS],0,funname);
          fprintf(fp,"%010lu",(atol(fields[PLF_WINS])+1));
	  fflush(fp);
          /* And restore the file pointer back to current location: */
          myfseek(fp,charcount,0,funname);
        }

       if(strequ(loser,fields[PLF_NAME]))
        { /* If loser found, then increment his/her losses: */
          myfseek(fp,seekpositions[PLF_LOSSES],0,funname);
          fprintf(fp,"%010lu",(atol(fields[PLF_LOSSES])+1));
	  fflush(fp);
          /* And restore the file pointer back to current location: */
          myfseek(fp,charcount,0,funname);
        }
     }

    lockfclose(fp);
}


/* Modified from the previous one. Currently not used, because we haven't
    still coded any mechanism for detecting ties.
 */
int update_ties(player1,player2)
char *player1,*player2;
{
    FILE *fp;
    ULI charcount=0;
    char *funname = "update_ties";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       ertzu_exit(101);
     }

/*  myrewind(fp,funname); */

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       if(strequ(player1,fields[PLF_NAME]))
        { /* If player1 found, then increment his ties: */
          myfseek(fp,seekpositions[PLF_TIES],0,funname);
          fprintf(fp,"%010lu",(atol(fields[PLF_TIES])+1));
	  fflush(fp);
          /* And restore the file pointer back to current location: */
          myfseek(fp,charcount,0,funname);
        }

       if(strequ(player2,fields[PLF_NAME]))
        { /* If player2 found, then increment his ties: */
          myfseek(fp,seekpositions[PLF_TIES],0,funname);
          fprintf(fp,"%010lu",(atol(fields[PLF_TIES])+1));
	  fflush(fp);
          /* And restore the file pointer back to current location: */
          myfseek(fp,charcount,0,funname);
        }
     }

    lockfclose(fp);
}


int update_address(player,address)
char *player,*address;
{
    FILE *fp;
    ULI charcount=0;
    int i;
    char *funname = "update_address";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       ertzu_exit(101);
     }

    charcount = seekplrecord(fp,player,funname);

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       if(strequ(player,fields[PLF_NAME]))
        { /* If player found, then change his address: */
          myfseek(fp,seekpositions[PLF_ADDRESS],0,funname);
          fprintf(fp,"%s",address);
          for(i=(MAX_ADR_LENGTH-strlen(address)); i--;)
           { putc(' ',fp); } /* Fill the rest of field with blanks. */
	  fflush(fp);
	  break; /* And break from the loop. */
        }
     }

    lockfclose(fp);
}


int update_flags(player,flags)
char *player;
ULI flags;
{
    FILE *fp;
    ULI charcount=0;
    char *funname = "update_flags";
    char *fields[MAXPLFIELDS+1];
    ULI seekpositions[MAXPLFIELDS+1];

    /* This flag tells to ertzu_exit that it doesn't try to call
       this function second time (which would result the recursive
       idiot loop), if error occurs during processing of this: */
    _get_or_addplid_ = 1;
    if(!(fp = lockfopen(PLAYERFILE,"r+")))
     {
       ertzu_exit(101);
     }

    charcount = seekplrecord(fp,player,funname);

    while(cut_line(fp,&charcount,seekpositions,fields) != EOF)
     {
       if(strequ(player,fields[PLF_NAME]))
        { /* If player found, then change his flags: */
          myfseek(fp,seekpositions[PLF_FLAGS],0,funname);
          fprintf(fp,"%08lx",flags);
	  fflush(fp);
	  break; /* And break from the loop. */
        }
     }

    lockfclose(fp);
}


int cut_line(fp,charcountp,seekvec,resultvec)
FILE *fp;
ULI *charcountp,*seekvec;
char **resultvec;
{
    int count=0;
    int c;
    char *p,*q;
    static char linebuf[BUFSIZ+3];

    q = linebuf;
    p = NULL;

    while(1)
     {
       c = getc(fp);
       if((c == EOF) /* || (c == EOFCHAR) */ )    { return(EOF); }
       ++*charcountp;

       if(isspace(c))
        {
/* If first blank after non-blanks, then complete the string with '\0'
    set p NULL, and increment count and pointer to linebuf.
    (Also set next element of resultvec to be NULL in case there
     might not be more elements on this line.) */
	  if(p) { *p = '\0'; *(resultvec+(++count)) = p = NULL; q++; }
/* If end of line: (newline is also included in white space class) */
          if(c == '\n') { return(count); }
	}
       else /* Non-blank character, should be collected. */
        {
	  if(!p) /* First non-blank after blanks? */
	   { /* Set seek position pointer to that char into seekvec: */
	     *(seekvec+count) = *charcountp-1;
	     p = *(resultvec+count) = q;
	   }
	  q++; /* Update pointer to linebuf. */
	  *p++ = c; /* Also stuff the character there. */
	}
     }
}




MYINT readwlist()
{
    MYINT linecnt=0,windex=0;
    register char *p;
    FILE *fp;
    char buf[MAXBUF+3];

    if(!(fp = lockfopen(WLISTFILE,"Sr+")))
     {
       return(0);
     }

    while(fgets(buf,MAXBUF,fp))
     {
       linecnt++;
       /* Skip the empty lines: */
       if((p = skip_blankos(buf)),!*p) { continue; }
       if(*p == EOFCHAR) { break; }
/* Also the comment lines:
       if((*p == '#') || (*p == ';')) { continue; }
 */ 
       if((++windex > MAXWLIST))
        {
	  errlog((E1,
"**Internal error in readwlist(): there is more than %u lines in file %s\n",
            MAXWLIST,WLISTFILE));
	  errlog((E1,"linecnt=%lu  windex=%lu  buf=%u:\n",
                      linecnt,windex,strlen(buf),buf));
	  ertzu_exit(101);
        }

       /* If there was previously something: */
       if(wlist[windex]) { free(wlist[windex]); }
       wlist[windex] = mystrdup(nuke_newline(buf));
     }

    lockfclose(fp);
    
    if(wlist[windex+1]) { free(wlist[windex+1]); }
    wlist[windex+1]    = NULL;

    return(windex);
}


MYINT addtowlist(line)
char *line;
{
    MYINT linecnt=1,windex=0,own_requests=0,result;
    unsigned long int charcount=0;
    int c;
    char differ;
    register char *p;
    FILE *fp;

    /* If using Turbo-C on MS-DOS use mode "rb+" (if that exists),
        so that getc won't skip the CR's (and charcount will be
	correct after the loops).
     */
    if(!(fp = lockfopen(WLISTFILE,"Sr+")) && !(fp = lockfopen(WLISTFILE,"w+")))
     {
       return(0);
     }

    /* In this bigger loop we roll so long as we encounter first
        non-space character from the beginning of each line. */
    while(1)
     {
       c = getc(fp);
susipuku:
       if(c == EOF)     { break; }
       if(c == EOFCHAR) { break; }
       charcount++;
       /* Skip spaces, tabs & cr's: */
       if((c == ' ') || (c == '\t') || (c == '\r')) { continue; }
       if(c == '\n') { linecnt++; continue; }

       /* Else it's some other character: */
       if((++windex > MAXWLIST))
        {
	  errlog((E1,
"**Internal error in addtowlist(): there is more than %u lines in file %s\n",
            MAXWLIST,WLISTFILE));
	  errlog((E1,"linecnt=%lu  windex=%lu\n",
                      linecnt,windex));
	  ertzu_exit(101);
        }

       /* In this smaller loop we roll on till the end of line.
          It is also checked whether beginning of line matches to
	  player name, and if that is case, then own_requests is
	  incremented: */
       for(differ=0,p=_ownname_;;)
        {
	  if(!differ && (c != *p++)) { differ=1; }
	  c = getc(fp);
	  /* Back to bigloop */
	  if((c == EOF) || (c == '\n')) { goto susipuku; }
	  charcount++;
	  if((c == ' ') || (c == '\t'))
	   {
	     if(!differ && !*p) { own_requests++; }
	     differ = 1;
	   }
	}
     }

    if((own_requests >= MAXREQUESTS) && !_su_flag_)
     {
       fout((OB,
"?You already have %lu match requests in the list, can't add more.\n",
         own_requests));
	return(0);
     }
    else if(windex >= MAXWLIST)
     {
       fout((OB,
"?Sorry, can't add more new match requests, the list is full (%lu).\n",
         windex));
       result = 0;
     }
    else
     {
       /* Set the current file position to point to EOFCHAR so it will be
          overwritten. (Or the stuff is appended to the end of file
	  if no EOFCHAR where encountered, and c is real EOF). */
       myfseek(fp,charcount,0,"addtowlist");
       fputs(line,fp);
       fprintf(fp,"\n%c\n",EOFCHAR);
       result=windex;
     }

    lockfclose(fp);
    
    return(result);
}

/*

MYINT addtopglist(plid,gid,color)
MYINT plid,gid,color;
{
    char tb[SMALLBUF],tb2[SMALLBUF];
    sprintf(tb,"%lu.g",plid);
    sprintf(tb2,"%-10lu %u",gid,color);
    return(gen_addtolist(tb2,tb));
}

MYINT rm_pglist(plid,gid)
{
    char tb[SMALLBUF];
    sprintf(tb,"%lu.g",plid);
    return(gen_rm_list(gid,tb));
}

*/

/* Append line 'line' to the end of glist file.
   There should be no EOFCHAR's anywhere on those lines written
   to glistfile.
 */
MYINT gen_addtolist(line,filename)
char *line;
char *filename;
{
    unsigned long int charcount=0;
    int c;
    FILE *fp;

    /* If using Turbo-C on MS-DOS use mode "rb+" (if that exists),
        so that getc won't skip the CR's (and charcount will be
	correct after the loop).
     */
    if(!(fp = lockfopen(filename,"Sr+")) && !(fp = lockfopen(filename,"w+")))
     {
       ertzu_exit(101);
     }

    /* Scan the file character by character until the EOF or
       EOFCHAR is found: */
    while(((c = getc(fp)),((c != EOF) && (c != EOFCHAR))))
     {
       charcount++;
     }

    /* Set the current file position to point to EOFCHAR so it will be
       overwritten. (Or the stuff is appended to the end of file
       if no EOFCHAR where encountered, and c is real EOF). */
    myfseek(fp,charcount,0,"gen_addtolist");
    fputs(line,fp);
    fprintf(fp,"\n%c\n",EOFCHAR);

    lockfclose(fp);
    
    return(1);
}


addtolist(line,filename)
char *line,*filename;
{
    FILE *fp;

    if(!(fp = lockfopen(filename,"a")))
     {
       ertzu_exit(101);
     }
    fputs(line,fp);
    fprintf(fp,"\n");
    lockfclose(fp);
}

addtomlist(plid,line,stat)
MYINT plid;
char *line;
char stat;
{
    char buf[MAXBUF+3];
    char filename[SMALLBUF];

    sprintf(filename,"%lu.msg",plid);

    sprintf(buf,"%c %s %s %s",stat,_ownname_,get_timebuf(),line);

    addtolist(buf,filename);
}



/* Remove the first line from wlist which matches 'line' */
MYINT rm_wlist(line)
char *line;
{
    MYINT linecnt=0,windex=0,result;
    int i;
    register char *p;
    char removed=0;
    FILE *fp;
    char buf[MAXBUF+3];

    if(!(fp = lockfopen(WLISTFILE,"r+")))
     {
       return(0);
     }

    while(fgets(buf,MAXBUF,fp))
     {
       linecnt++;
       /* Skip the empty lines: */
       if((p = skip_blankos(buf)),!*p) { continue; }
       if(*p == EOFCHAR) { break; }
/* Also the comment lines:
       if((*p == '#') || (*p == ';')) { continue; }
 */ 
       if((windex >= MAXWLIST))
        {
	  errlog((E1,
"**Internal error in rm_wlist(): there is more than %u lines in file %s\n",
            MAXWLIST,WLISTFILE));
	  errlog((E1,"linecnt=%lu  windex=%lu  buf=%u:\n",
                      linecnt,windex,strlen(buf),buf));
	  ertzu_exit(101);
        }

       nuke_newline(buf);

       /* If found the line, and similar is not already removed, then
          don't take this line to wlist: */
       if(!removed && strequ(line,buf))
        {
	  removed = 1; /* Set the removed flag on. */
	}
       else
        {
          /* If there was previously something: */
          if(wlist[++windex]) { free(wlist[windex]); }
          wlist[windex] = mystrdup(buf);
        }
     }

    wlist[windex+1] = NULL;

    if(!removed) /* Didn't find the line. */
     {
       pout((
"?There is no more that line in the list. Use NEW to see the list again.\n"));
       result = 0;
       goto pois;
     }
    else { result = 1; }

    myrewind(fp,"rm_wlist");

    /* Now write the lines back: */
    for(i=1; ((i < MAXWLIST) && wlist[i]); i++)
     {
       fputs(wlist[i],fp);
       fprintf(fp,"\n");
     }
    fprintf(fp,"%c\n",EOFCHAR);

pois:
    lockfclose(fp);

    return(result);
}


/* Remove the first line from filename where gid 'gid' is first
    (or only) element of line, and copy that line to destbuf,
    which is returned as result. However, if that kind of line
    is not found, then NULL is returned.
 */
char *gen_rm_list(gid,destbuf,filename)
MYINT gid;
char *destbuf;
char *filename;
{
    MYINT g;
    MYINT removed=0;
    FILE *fp,*tmp_fp;
    char *funname="gen_rm_list";
    char *p,*z;
    char buf[MAXBUF+3];

    z = NULL;

    if(!(fp = lockfopen(filename,"r+")))
     {
       ertzu_exit(101);
     }

    if(!(tmp_fp = myfopen(GLIST_TMPFILE,"w+")))
     {
       ertzu_exit(101);
     }

    while(fgets(buf,MAXBUF,fp))
     {
       /* Skip the empty lines: */
       if((p = skip_blankos(buf)),!*p) { continue; }
       if(*p == EOFCHAR) { break; }
/* Also the comment lines:
       if((*p == '#') || (*p == ';')) { continue; }
 */
       sscanf(buf,"%ld",&g);
       if(g == gid) /* If found the line to be removed. */
        {
	  strcpy(destbuf,buf); /* Copy it to destbuf. */
	  z = destbuf; /* Which is returned as result. */
	}
       else /* Otherwise, write the line to tempfile. */
        {
          fputs(buf,tmp_fp);
        }
     }

    fflush(tmp_fp); /* Flush tmpfile. */

    /* Then rewind both files back to beginning: */
    myrewind(fp,funname);
    myrewind(tmp_fp,funname);

    /* Now write the lines back: */

    while(fgets(buf,MAXBUF,tmp_fp))
     {
       fputs(buf,fp);
     }
    fprintf(fp,"%c\n",EOFCHAR);

    myunlink(GLIST_TMPFILE,funname);
    fclose(tmp_fp);

    lockfclose(fp);

    return(z);
}


/* Loop through GLISTFILE, and collect all games by this player to
   the list _gidlist_.
 */
readgidlist()
{
    MYINT gid;
    MYINT plid1,plid2;
    int color;
    FILE *fp;
    char *p;
    char buf[MAXBUF+3];

    freegidlist(_gidlist_);
    _gidlist_ = NULL;

    if(!(fp = myfopen(GLISTFILE,"r")))
     {
       return(0);
     }

    while(fgets(buf,MAXBUF,fp))
     {
       /* Skip the empty lines: */
       if((p = skip_blankos(buf)),!*p) { continue; }
       if(*p == EOFCHAR) { break; }
/* Also the comment lines:
       if((*p == '#') || (*p == ';')) { continue; }
 */
       if(sscanf(p,"%ld %ld %ld",&gid,&plid1,&plid2) != 3)
        {
	  errlog((E1,
"**Internal error in readgidlist(), invalid line: %u/%s\n",
             strlen(buf),buf));
	  ertzu_exit(101);
	}

       if(plid1 == _ownplid_)
        { _gidlist_ = consgl(gid,0,_gidlist_); }
       if(plid2 == _ownplid_)
        { _gidlist_ = consgl(gid,1,_gidlist_); }

     }

    fclose(fp);
}


/* This reads the current highest game-id stored in nextgidfile,
   overwrites it with one bigger game-id, and returns that old
   game-id.
 */
MYINT getnextgid()
{
    MYINT nextgid;
    FILE *fp;
    int s;
    char buf[SMALLBUF+3];

    if(!(fp = lockfopen(NEXTGIDFILE,"r+")))
     {
       ertzu_exit(101);
     }

    if(!fgets(buf,SMALLBUF,fp) || ((s = sscanf(buf,"%ld",&nextgid)) != 1))
     {
       errlog((E1,
"**Internal error in getnextgid: file %s is invalid, s=%d, buf=%u/%s!\n",
        NEXTGIDFILE,s,strlen(buf),buf));
       ertzu_exit(101);
     }

    myrewind(fp,"getnextgid");
    fprintf(fp,"%lu\n",(nextgid+1));
    lockfclose(fp);
    return(nextgid);
}


/*
   Read the movecount, and also move & time to movebuf & timebuf.
   If update argument is non-zero, then first character of file is
   changed to R (as read), and time is updated.
 */
MYINT get_next_move(gid,status,movebuf,timebuf,color,update)
MYINT gid;
char *status,*movebuf,*timebuf;
int color;
int update;
{
    MYINT movecount;
    FILE *fp;
    char *funname="get_next_move";
    char *p;
    char namebuf[MAXBUF+3];
    char buf[MAXBUF+3];

    sprintf(namebuf,"%lu.lm",gid);

    if(!(fp = lockfopen(namebuf,"Tr+")))
     {
       return(ERRMOVES);
     }

    if(!fgets(buf,MAXBUF,fp))
     {
ertzu:
       errlog((E1,
"**Internal error in %s: file %s is invalid! buf=%u/%s\n",
         funname,namebuf,strlen(buf),buf));
       ertzu_exit(101);
     }

    if(sscanf(buf,"%c %ld %s %s",status,&movecount,movebuf,timebuf) != 4)
     { goto ertzu; }
    *status = toupper(*status);
    if(!isalpha(*status)) { goto ertzu; }

/* If update option, and opponent (not this player) has played something: */
    if(update && ((*status == 'P') || (*status == ('P'+1))) &&
      (get_parity(movecount) == color))
     {
       myrewind(fp,funname);

       /* Overwrite the first status letter with S if it was P,
          and with T if it was Q. Keep other information intact.
	*/
       fprintf(fp,"%c",(*status + ('S'-'P')));
/*
       fprintf(fp,"%c %lu %s %s\n",
         (*status + ('S'-'P')),movecount,movebuf,get_timebuf());
 */
     }

    lockfclose(fp);
    return(movecount);
}


MYINT undo_move(gid,color,otherflag)
MYINT gid;
int color,otherflag;
{
    MYINT movecount;
    FILE *fp;
    int sc,result;
    char *funname="undo_move";
    char *p,*msg=NULL;
    char status;
    char buf[MAXBUF+3];
    char timebuf[SMALLBUF];
    char namebuf[SMALLBUF];
    char movebuf[MAXBUF+3];

    sprintf(namebuf,"%lu.lm",gid);

    if(!(fp = lockfopen(namebuf,"Tr+")))
     {
       return(ERRMOVES);
     }

    if(!fgets(buf,MAXBUF,fp))
     {
ertzu:
       lockfclose(fp);
       errlog((E1,
"**Internal error in %s(%lu,%u,%u): file %s is invalid! sc=%d, buf=%u/%s\n",
         funname,gid,color,otherflag,namebuf,sc,strlen(buf),buf));
       ertzu_exit(101);
     }

    if((sc=sscanf(buf,"%c %ld %s %s",&status,&movecount,movebuf,timebuf)) != 4)
     { goto ertzu; }
    status = toupper(status);

/*  if(!isalpha(status)) { goto ertzu; } */

/* If it is our turn to move, then letter is lowercase, otherwise upper: */
    if(get_parity(movecount) == color) { status = tolower(status); }

    switch(status)
     {
       case 's': /* If we have seen opponent's move, but not replied yet. */
        {
	  if(!otherflag)
	   {
	     msg =
"?If you want to undo your opponent's move use UNDO with -o option.\n";
             result = _movesmade_; break;
	   }
	  else { goto undo_it; }
	}
       case 'P':
        {/* If we have played the last move, and opponent hasn't seen it yet */
	  if(otherflag)
	   {
	     msg =
"?If you want to undo your own move use UNDO *without* -o option.\n";
             result = _movesmade_; break;
	   }
undo_it:
	  if(_movesmade_ < movecount)
	   {
             sprintf(buf,
"?Please load the whole game in before you undo a move. (%lu != %lu)\n(Use MOVES -N command)\n",
             _movesmade_,movecount);
             msg = buf;
             result = _movesmade_; break;
           }
	  /* Else it's okay to undo: */
          myrewind(fp,funname); /* Rewind the file for overwriting. */

          /* If movecount was zero, then keep it intact, otherwise subtract
              one from it: */
          fprintf(fp,"U %lu %s %s\n",(movecount ? (movecount-1) : movecount),
                    movebuf,get_timebuf());

          msg = "Last move undone!\n";

          result = movecount-1;
	  break;
	}
       case 'P'+1:
       case 'S'+1:
       case 'p'+1:
       case 's'+1:
        {
          msg =
"?The last move is compulsory, it can't be undone anymore!\n";
          result = _movesmade_; break;
	}
       case 'p': /* If opponent has played, */
       case 'U': /* or undone his/her move, then it's not our turn! */
        {
          msg = "?It's not your turn!\n";
          result = _movesmade_; break;
	}
       case 'u': /* We have just undone. */
        {
          msg = "?Your move has been already undone!\n";
          result = _movesmade_; break;
        }
       case 'S': /* If opponent has seen the move. */
        {
	  msg = 
"?Your opponent has already seen your move, can't undo it anymore.\nHowever, you could beg your opponent to undo it with UNDO -o command.\n";
          result = _movesmade_; break;
        }
       default: { goto ertzu; }
     }

    lockfclose(fp);
    if(msg) { pout(msg); }
    return(result);
}



MYINT put_next_move(gid,movecount,movebuf,timebuf,status)
MYINT gid,movecount;
char *movebuf,*timebuf;
char status;
{
    FILE *fp;
    char *funname="put_next_move";
    char namebuf[MAXBUF+3];

    sprintf(namebuf,"%lu.lm",gid);

    if(!(fp = lockfopen(namebuf,"w")))
     {
       ertzu_exit(101);
     }

    fprintf(fp,"%c %lu %s %s\n",status,movecount,movebuf,timebuf);

    strcpy(_latest_move_,movebuf);

    lockfclose(fp);
    return(movecount);
}



/* Is this really necessary? */
initwlist()
{
    register int i;

    for(i=MAXWLIST; i ; (wlist[--i] = NULL)) ;
}

MYINT show_glist()
{
    FILE *fp;
    MYINT games=0;
    MYINT gid,plid1,plid2,moves;
    char status;
    char *p;
    char buf[MAXBUF+3];
    char foobuf[MAXBUF+3];
    char timebuf[SMALLBUF];

    if(!(fp = myfopen(GLISTFILE,"r")))
     {
       return(0);
     }

    while(fgets(buf,MAXBUF,fp))
     {
       /* Skip the empty lines: */
       if((p = skip_blankos(buf)),!*p) { continue; }
       if(*p == EOFCHAR) { break; }

       games++;
       if(!(p = getfirstword(foobuf,p))) { continue; }
       gid = atol(foobuf);
       if(!(p = getfirstword(foobuf,p))) { continue; }
       plid1 = atol(foobuf);
       if(!(p = getfirstword(foobuf,p))) { continue; }
       plid2 = atol(foobuf);

       fout((OB,"%-4lu %s",gid,nuke_newline(skip_blankos(p))));
       if((moves = get_next_move(gid,&status,foobuf,timebuf,0,0)) == ERRMOVES)
        {
	  pout((" ???\n"));
	}
       else
        {
	  fout((OB," %-3lu %-7s %c %s\n",moves,foobuf,status,timebuf));
	}
     }

    fclose(fp);

    if(!games)
     { pout(("No active games.\n")); }
    return(games);
}

int show_wlist()
{
    register int i;
/*
    char buf1[MAXBUF+3],buf2[MAXBUF+3];

    *buf2   = '?';
    buf2[1] = '\0';
 */

    if(!(_wlistsize_ = readwlist()))
     {
       pout(("No new games needing a player.\n"));
     }
    else
     {
       for(i=1; ((i <= MAXWLIST) && wlist[i]); i++)
        {
/*
	  strcpy(buf1,wlist[i]);
          takefirstword(buf2,buf1);
	  pertospc(buf2);
	  fout((OB,"%-2u  %-20s %s\n",i,buf2,skip_blankos(buf1)));
 */
          fout((OB,"%-2u  %s\n",i,wlist[i]));
        }
     }
}



notify(pid,event)
int pid,event;
{
#ifdef UNIX
    kill(pid,notify_signals[event]);
#else /* In MS-DOS, we send 'pseudo-signal' internally to us: */
    if(_blackplid_ == _whiteplid_)
     {
       _sig_received_ = notify_signals[event];
     }
#endif
}

notify_opponent(color,event)
int color,event;
{
    int pid;

#ifdef OLD_CODE_COMMENTED_OUT
/* If playing against self, then opponent is ourself, and its pid is our pid:
 */
    if(_blackplid_ == _whiteplid_)
     { pid = getpid(); }
    else
#endif
    fetch_opponents_fields(_names_[color]);

    pid = atoi(_opfields_[PLF_PID]);

#ifdef UNIX
    /* If opponent has mailnotifyflag turned on and his address is set: */
    if(getmailnotifyflag(_opflags_) && !strequ(_opfields_[PLF_ADDRESS],"NONE"))
     {
       send_report_with_mail(event,_opfields_[PLF_ADDRESS],_opflags_);
     }
#endif

#ifdef UNIX /* Check that there is process with pid 'pid': */
    if(pid && kill(pid,0)) { pid = 0; }
#endif
    if(pid) { notify(pid,event); }
}


fetch_opponents_fields(opponents_name)
{
    ULI zero=0L;
    char **fields;

    if(!(fields = getplfields(opponents_name,&zero,0)))
     {
       errlog((E1,
"fetch_opponents_fields(%s): Can't find a player with that name!\n",
            opponents_name));
       return(0);
     }

    /* Make copy of these fields to opfields: */
    copy_fields(_opfields_,spacefor_opfields,fields);
    sscanf(_opfields_[PLF_FLAGS],"%lx",&_opflags_);
}


#ifdef UNIX

int send_report_with_mail(event,address,flags)
int event;
char *address;
ULI flags;
{
    int i,save_owncolor;
    FILE *popen();
    FILE *fp;
    char *funname = "send_report_with_mail";
    char subject[81];
    char cmd[BUFSIZ];

    switch(event)
     {
       case GAME_FINISHED: { return(0); }
       case GAME_STARTED:
        {
       /* We drop to MOVE_MADE only if owncolor is black, and move was made: */
          if(!(!_owncolor_ && _movesmade_)) { return(0); }
        }
       case MOVE_MADE:
        {
          sprintf(subject,"Game %lu Move %lu: %s (%s vs. %s)",
           _gid_,_movesmade_,_latest_move_,_blackname_,_whitename_);
          break;
        }
       default:
        {
         errlog((E1,
"**Internal error in %s(%u,%s,%08lx): event is invalid!\n",
          funname,event,address,flags));
         return(0);
        }
     }

/* Note: it should be guaranteed that there's no singlequotes nor
    backslashes in subject (i.e. check also player's names & move notations)
    or address:
 */

#ifdef USE_SENDMAIL
    sprintf(cmd,"/usr/lib/sendmail '%s'",address);
#else
    sprintf(cmd,"/usr/bin/mailx -i -s '%s' '%s'",subject,address);
#endif

    if(!(fp = popen(cmd,"w")))
     {
       errlog((E1,
"**Internal error in %s(%u,%s,%08lx): Couldn't execute:\n%s\n",
         funname,event,address,flags,cmd));
       return(0);
     } 

    /* Okay, now we have pipe open to mailx, let's feed some stuff for it:
       We should be sure that there's no lines beginning with tilde (~)
       or containing only single period (.), because they will mess up
       the mailx.
    */

#ifdef USE_SENDMAIL
/* If we are using sendmail then we must give Subject:-header as first
    line of it: */
    output_stuff("Subject: ",fp);
    output_stuff(subject,fp);
    output_stuff("\n",fp); /* And blank line between header & body. */
#endif

    /* First print current board from opponent's view point and with
        his/her flags: */
    printboard(!_owncolor_,fp,flags);

    output_stuff(" \n",fp); /* Put blank line after board diagram. */

    /* Then report also his king's status: */

    /* Set _owncolor_ to NOCOLOR, so that whosking (called by show_checks)
        will use colour instead of possessives when referring to king: */
    save_owncolor = _owncolor_;
    _owncolor_ = NOCOLOR;
    show_checks(fp,_movestat_,!save_owncolor,_captured_piece_,
	         checkmsgbuf,escapemsgbuf);
    _owncolor_ = save_owncolor; /* Restore _owncolor_ */

    pclose(fp);

    return(1);
}

#endif


/* Make copy of srcfields to destfields, using destspace for storing
   the copies: */
copy_fields(destfields,destspace,srcfields)
char **destfields,*destspace,**srcfields;
{
    char *p;

    while(p = *srcfields++)
     {
       *destfields++ = destspace;
       while(*destspace++ = *p++) ; /* Copy the contents of the field. */
     }
    *destfields = NULL; /* Terminate destination fields with NULL. */
}

