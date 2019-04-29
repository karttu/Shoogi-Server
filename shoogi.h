
#include "stdio.h"
#include "ctype.h"
#include "errno.h"
#include "signal.h"

#ifdef UNIX
#include "pwd.h"
#ifdef SYSV
#include "unistd.h"
#else
#include "fcntl.h"
#endif
#endif

#ifdef CURSES
#include "curses.h"
#endif

/* Does anybody invent any better name? */
#define progname "ShogiServer"

#ifdef UNIX
#define SHOOGI_DIRECTORY "/home/mits/karttu/.public_html/shogi/lib/"
/* "/usr/games/lib/shoogi/" */
/* Use "/usr2/karttu/shoogi/" When testing. */
#else
#define SHOOGI_DIRECTORY "B:\\shoogi\\"
#endif

#define CLISTFILE       "clist.sho"
#define GLISTFILE       "glist.sho"
#define GLIST_TMPFILE   "glist.tmp"
#define PLAYERFILE      "players.sho"
#define WLISTFILE       "wlist.sho"
#define NEXTGIDFILE     "nextgid.sho"
#define ERRLOGFILE      "errors.sho"
#define HELPFILE        "shoogi.hlp"
#define MOTDFILE        "motd.sho"

/* Field indexes to PLAYERFILE: */
#define PLF_NAME        0
#define PLF_PLID        1
#define PLF_FLAGS       2
#define PLF_PID         3
#define PLF_COUNT       4
#define PLF_TIME        5
#define PLF_WINS        6
#define PLF_LOSSES      7
#define PLF_TIES        8
#define PLF_SPARE1      9
#define PLF_SPARE2      10
#define PLF_SPARE3      11
#define PLF_SPARE4      12
#define PLF_ADDRESS     13
#define PLF_PWD         14

#define MAXPLFIELDS     15 /* How many of the above ones. */

#define MAX_ADR_LENGTH   121
#define DEFAULT_PWDFIELD "*************" /* Thirteen asterisks. */

/* Some event codes: */
#define MESSAGE_SENT       1
#define MOVE_MADE          2
#define GAME_STARTED       3
#define GAME_FINISHED      4

/* Corresponding signals: */
#ifdef UNIX
#define SIG_MESSAGE_SENT   SIGUSR1
#define SIG_MOVE_MADE      SIGUSR1
#define SIG_GAME_STARTED   SIGUSR2
#define SIG_GAME_FINISHED  SIGUSR2
#else /* In MS-DOS pseudo-signals are used (but for internal use only!): */
#define SIG_MESSAGE_SENT   100+MESSAGE_SENT
#define SIG_MOVE_MADE      100+MOVE_MADE
#define SIG_GAME_STARTED   100+GAME_STARTED
#define SIG_GAME_FINISHED  100+GAME_FINISHED
#endif


#define SUPERUSER1 "ANTTI.KARTTUNEN"
#define MASTER_GID 10
#define PLAYER_UID 999 /* 211 in mits */

typedef unsigned long int MYINT;
typedef unsigned long int ULI;
typedef unsigned int  UINT;
typedef unsigned char COR;
typedef unsigned char PIECES;
typedef unsigned char UBYTE;

/* Pointer to Function returning Int:
 *  (see K&R pages 114-116,141,195 and two last lines of page 209) */
typedef int (*PFI)();

#define touli(X)  ((ULI) (X))
#define touint(X) ((UINT) (X))

/* If tolower & -upper macros given in ctype.h don't return character
    intact back when it's not uppercase or lowercase character
    respectively, then we replace their definitions with our own
    functions:
 */
#ifdef CRIPPLED_TOUPPER
#undef tolower
#undef toupper
#define tolower mytolower
#define toupper mytoupper
#endif


struct gid_list
 {
   struct gid_list *next;
   MYINT gid;
   int color;
 };

typedef struct gid_list *GIDLIST;

struct piecename
 {
   char *english;
   char *long_romaji;
   char *short_romaji;
   char *long_kanji;
   char *short_kanji;
   unsigned short int long_unikanji1;
   unsigned short int long_unikanji2;
   unsigned short int short_unikanji;
 };


#define colcar(X) ((X)->color)
#define gidcar(X) ((X)->gid)
#define cdr(X)    ((X)->next)

#define addtoglist(line)   gen_addtolist((line),GLISTFILE)
#define rm_glist(gid,buff) gen_rm_list((gid),(buff),GLISTFILE)

#define addtoclist(line)   addtolist((line),CLISTFILE)

/* Used in GLISTFILE and WLISTFILE: */
#define EOFCHAR '/'


#define input stdin

/* #define ertzu_exit exit */

/* For MS-DOS: */
#ifndef UNIX

/* Now we use Turbo-C which has strchr instead of index:
   #define strchr index */
#define sleep(X)
#define getpid() ((int) (time(NULL) & 077777)) /* 15 lowest bits. */

#endif

extern int errno;

int bugmove_(),bishop_(),rook_(),pawn_(),lance_(),knight_(),
    silver_(),gold_(),king_(),prom_bishop(),prom_rook_();

void sighandler();

unsigned long int atol();

FILE *fopen();
FILE *myfopen();
FILE *lockfopen();

char *getenv();
char *getlogin();
char *whosking();
char *sprintmove();
char *strchr();
char *fgets();
char *malloc();
char *getcoord();
char *getfirstword();
char *reconstruct_cmdline();
char *skip_blankos();
char *nuke_newline();
char *conv_upper();
char *conv_scands();
char *getcardinalsuffix();
char *get_whole_filename();
char *getname();
char *mystrdup();
char *get_timebuf();
char *gen_rm_list();

char *getpiecename();
char *getpieceshort();
UBYTE *convert_jis();
char *convert_ous();

char **getplfields();
ULI  seekplrecord();
MYINT checknewmoves();
MYINT loadmoves();
MYINT readmove();
MYINT getplid();
MYINT get_or_addplid();
MYINT find_all_online();
MYINT readwlist();
MYINT addtowlist();
MYINT gen_addtolist();
MYINT rm_wlist();
MYINT getnextgid();
MYINT get_next_move();
MYINT undo_move();
MYINT put_next_move();
MYINT show_glist();
MYINT start_new_game();
MYINT remove_game();
MYINT add_new_game();
MYINT check_gamenum();

GIDLIST consgl();

/* How many attempts to open a locked file before giving up:
   (There is one second pause between each try.)
 */
#define MAX_ATTEMPTS 5

#define MAXWLIST     20
#define MAXREQUESTS  4

#define MAXBUF 8192

#define SMALLBUF 25
#define CHKMSGBUF 257

#define _C_ escapemsgbuf

#define ynq(p) ynq_aux((p),0)

#define fout(X) { sprintf X; output_stuff(OB,stdout); }
#define pout(X) output_stuff((X),stdout)
#ifndef CURSES
#define beep()  output_stuff("\7",stdout) /* Emit bell. */
#endif

#define errlog(X)       { sprintf X ; print_err(E1,1); }
#define errlog_only(X)  { sprintf X ; print_err(E1,0); }
#define err2log(X)      sprintf X

#define check_message(X) sprintf X

#define outchar(X) putchar(X)


#define getcapcolname(color) ((color) ? "White" : "Black")
#define getlowcolname(color) ((color) ? "white" : "black")

#define strequ(s,t) (!strcmp((s),(t)))

#define BANISHED 1

#define ENGLISH      0
#define LONG_ROMAJI  1
#define SHORT_ROMAJI 2
#define LONG_KANJI   3
#define SHORT_KANJI  4

#define NO_KANJI 0
#define JIS      1
#define EUC      2
#define SHJ      3

#define ASCII 0
#define MIXED 1
#define KANJI 2
#define UNIKANJI 3
#define HTML_BOARD 4

#define DEFAULT_FLAGS 0x00000000L
#define DEFAULT_IMPASSE_POINTS 24
#define NOTSET_SEEKPTR ((ULI) -1L)

#define getseclevel(F)       touint((F)  &        0xFL)
#define getpnamestyle(F)     touint(((F) &       0xF0L) >> 4)
#define getboardstyle(F)     touint(((F) &      0xF00L) >> 8)
#define getkanjicode(F)      touint(((F) &     0x7000L) >> 12)
#define getdragonflag(F)     touint(((F) &     0x8000L) >> 15)
#define getblackcode(F)      touint(((F) &    0xF0000L) >> 16)
#define getwhitecode(F)      touint(((F) &   0xF00000L) >> 20)
#define getthreatenedcode(F) touint(((F) &  0xF000000L) >> 24)
#define getmailnotifyflag(F) touint(((F) & 0x10000000L) >> 28)

#define setseclevel(F,V)       (((F) & ~0xFL)    | touli(V))
#define setpnamestyle(F,V)     (((F) & ~0xF0L)   | touli((V) << 4))
#define setboardstyle(F,V)     (((F) & ~0xF00L)  | touli((V) << 8))
#define setkanjicode(F,V)      (((F) & ~0x7000L) | touli((V) << 12))
#define setdragonflag(F,V)     (((F) & ~0x00008000L) | (touli(V) << 15))
#define setblackcode(F,V)      (((F) & ~0x000F0000L) | (touli(V) << 16))
#define setwhitecode(F,V)      (((F) & ~0x00F00000L) | (touli(V) << 20))
#define setthreatenedcode(F,V) (((F) & ~0x0F000000L) | (touli(V) << 24))
#define setmailnotifyflag(F,V) (((F) & ~0x10000000L) | (touli(V) << 28))

/* Control characters from CTRL-N to CTRL-Z are used as internal markers
    of terminal/ansi modes: */
#define getcodeletter(C) ((C) + '\016')
#define getmodecode(C)   ((C) - '\016')

#define iscodeletter(C) (((C) >= getcodeletter(0)) && ((C) < '\033'))

#define NORMAL 0

#define ANSI_TEMPLATE   "\033[%sm"
#define ANSI_REVERSE    7

#define JIN  "\033$B"
#define JOUT "\033(J"

#define JIS_BLANK   "!!"
#define JIS_HYPHEN  "!]"
#define JIS_PROMOTE "@." /* NARI */
#define JIS_SAME    "F1" /* DOU  */
#define JIS_DROP    "BG" /* DA   (Strike, "drop") */

/*
   Bits and their meanings:

   7 6 5 4 3 2 1 0
       : : : piece
       : : :
       : : :
       : : +-> is promotable? 1 if not possible to promote (GOLD & KING).
       : :
       : +-> is promoted? 1 if it is.
       :
       +-> colour? 0 = BLACK, 1 = WHITE.

 */


#define EMPTY   0
#define EDGE    1
#define BISHOP  2
#define ROOK    3
#define PAWN    4
#define LANCE   5
#define KNIGHT  6
#define SILVER  7
#define GOLD    8
#define KING    9

/* Special value for checkshortmoves, checklongmoves and other movefuns,
   for searching from the neighbourhood all the empty or enemy squares
   (where the piece can be transferred to).
   Can be anded with the color WHITE (32.) to yield 42. */
#define OPPONENT_OR_EMPTY 10


/* Is p unpromoted piece? */
#define u_piecep(p)           (((p) > 1) && ((p) < 10))
#define promoted(p)           ((p) |  16)
#define unpromote(p)          ((p) & ~16)
#define promotedp(p)          ((p) &  16)
#define promotablep(p)        (!((p) &  8)) /* I.e. not GOLD nor KING */
#define moves_like_a_goldp(p) (color_off(p) > promoted(ROOK))

#define makewhite(p)          ((p) |  32)
#define makeblack(p)          ((p) & ~32)
#define color_off(p)          makeblack(p)
#define whitep(p)             ((p) &  32)
#define blackp(p)             (!whitep(p))
#define getcolor(p)           (!!whitep(p))

#define get_parity(m)         ((m) & 1)
/* Get the turn (0=black's, 1=white's) when m is the count of moves made
   so far: (Odd moves -> White's turn, Even moves -> Black's turn) */
#define get_turn(m)           get_parity(m) /* Just get the parity of m. */

#define ofcolor(p,c) ((c) ? makewhite(p) : makeblack(p))
#define opponents_color(c) (!(c))

#define getdir(c) ((c) ? -1 : 1)

#define other_square_empty_or_enemys(P1,P2)\
 ((EMPTY == P2) || ((EDGE != P2) && (getcolor(P1) != getcolor(P2))))

#define makecoord(x,y) ((COR) (((x) << 4) + (y)))

#define get_x(c) ((c) >> 4)
#define get_y(c) ((c) & 15)

#define square(x,y) (board[(x)+1][(y)+1])

#define BSIZE 9

#define NOTHING_SPECIAL     1
#define KING_LEFT_IN_CHECK  2
#define KING_CHECKED        3
#define KING_MATED          4
#define KING_CAPTURED       5
#define RESIGNED            6

/* End of Game? */
#define EOG(STAT) ((STAT) >= 4)

/* Error codes for the illegal_drop: */
#define UNABLE_TO_MOVE  1
#define DOUBLE_PAWN     2
#define PAWN_DROP_MATE  3

/* 20 pieces per side, so at least this is enough! */
#define MAXPIECES 41

/* From bishop (2) to king (9). There can't be any promoted pieces in hand. */
#define MAX_IN_HAND 10

/* I think true maximum is 6 (for gold) so this should be enough: */
#define MAXVEC 10

/* In Shoogi one square can be threated by max. 10 pieces.
   (Eight from sides, and by two horses.), but by 16 in Chess
   (eight possible knight moves)
 */
#define MAXTHREATS 17 /* Was 11 for SHOOGI */

#define MAXSQUARES (BSIZE*BSIZE)
#define MAXFREEDOMS (MAXSQUARES+10)

#define MAXFUNS 25
#define MAXCMDS MAXBUF

#define TMP_BOARD 1
#define ERRMOVES (-2L)
#define NOCOLOR   2

#define getowncolor()\
 ((_ownplid_ == _blackplid_) ? 0 : ((_ownplid_ == _whiteplid_) ? 1 : NOCOLOR))

extern MYINT _uid_;
extern MYINT _gid_;
extern MYINT _blackplid_;
extern MYINT _whiteplid_;
extern MYINT _ownplid_;
extern MYINT _movesmade_;
extern   int _movestat_;
extern   int _captured_piece_;
extern int   _b_impasse_points_;
extern int   _w_impasse_points_;

extern int _owncolor_;

extern ULI _ownseekptr_;
extern ULI _flags_;
extern ULI _opflags_;

extern char _blackname_[];
extern char _whitename_[];
extern char *_ownname_;
extern char space_for_latest_move[];
extern char *_latest_move_;
extern char *_names_[];

extern char _not_me_pattern_[];
extern char *_everybody_;
extern char _dot_[];

extern char *_opfields_[];
extern char spacefor_opfields[];

extern char _eog_;
extern char _debug_flag_;
extern char _su_flag_;
extern char _restricted_flag_;
extern char _get_or_addplid_;

extern int _sig_immediately_;
extern int _sig_received_;

extern GIDLIST _gidlist_;
extern char *wlist[];
extern int _wlistsize_;
extern int _morelines_;

extern char *cmds[];
extern int  cmdcnt;

/* extern char scorebuf[]; */
extern char escapemsgbuf[];
extern char checkmsgbuf[];
extern char E1[];
extern char E2[];
extern char OB[];

extern int notify_signals[];

extern char *codenames[];

#ifdef CURSES
extern chtype curses_codes[];
#endif
extern unsigned char ansi_codes[];


extern unsigned char board[BSIZE+4][BSIZE+4];

extern int piecevalues[];
extern char *letters;
extern struct piecename true_king;
extern struct piecename piecenames[];
extern char *Jnumbers[];
extern unsigned short int Unumbers[];
extern PIECES firstrank[];
extern int blacks_king[];
extern int whites_king[];
extern int *kings[];
extern PIECES blacks_hand[];
extern PIECES whites_hand[];
extern PIECES *hands[];

extern FILE *err_fp;

extern int G_argc;
extern char **G_argv;

