
#include "stdio.h"
#include "ctype.h"
#include "errno.h"

typedef unsigned long int MYINT;
typedef unsigned long int ULI;
typedef unsigned int  UINT;
typedef unsigned char COR; /* The coordinates fit to one byte. */
typedef unsigned char PIECE;
typedef unsigned char UBYTE;

typedef unsigned char UCHAR;


#define UCP(X) ((unsigned char *) ((X)))
#define SCP(X) ((char *) ((X)))

#define NO  !
#define NOT !
#define empty_stringp(X) (!*(X))


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

#define input stdin

/* #define ertzu_exit exit */

extern int errno;


#define MAXBUF 8192

#define SMALLBUF 85
#define CHKMSGBUF 257


#define ynq(p) ynq_aux((p),0)

#define fout(X) { sprintf X; output_stuff(OB,stdout); }
#define pout(X) output_stuff((X),stdout)

#define errlog(X)       { fprintf X ; }
#define errlog_only(X)  { fprintf X ; }
#define err2log(X)      sprintf X

#define check_message(X) sprintf X

#define ertzu_exit(X) exit(X)

#define outchar(X) putchar(X)


#define getcapcolname(color) ((color) ? "White" : "Black")
#define getlowcolname(color) ((color) ? "white" : "black")

#define strequ(s,t) (!strcmp((s),(t)))


#define ENGLISH      0
#define LONG_ROMAJI  1
#define SHORT_ROMAJI 2

#define NO_KANJI 0
#define JIS      1
#define EUC      2
#define SHJ      3

#define ASCII 0
#define MIXED 1
#define KANJI 2
#define UNIKANJI 3
#define HTML_BOARD 4

#define DEFAULT_FLAGS setboardstyle(0x00000000L,(UNIKANJI+HTML_BOARD))
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
#define KNIGHT  6 /* Either Chess or Shoogi Knight */
#define SILVER  7
#define GOLD    8
#define KING    9
#define QUEEN  10

/* Special value for checkshortmoves, checklongmoves and other movefuns,
   for searching from the neighbourhood all the empty or enemy squares
   (where the piece can be transferred to).
   Can be anded with the color WHITE (32.) to yield 43. */
#define OPPONENT_OR_EMPTY 11 /* Shouldn't be any of the above values !!! */


/* Is p unpromoted piece? */
#define u_piecep(p)           (((p) > EDGE) && ((p) <= QUEEN))
#define promoted(p)           ((p) |  16)
#define unpromote(p)          ((p) & ~16)
#define promotedp(p)          ((p) &  16)
#define Shoogi_promotablep(p) (!((p) &  8)) /* I.e. not GOLD nor KING */
#define moves_like_a_goldp(p) (color_off(p) > promoted(ROOK))

#define mv_promotablep(gis,p) (mv_shoogip(gis) ? Shoogi_promotablep(p) : (PAWN == (p)))

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

#define get_attacking_direction(c) getdir(!c)

#define other_square_empty_or_enemys(P1,P2)\
 ((EMPTY == P2) || ((EDGE != P2) && (getcolor(P1) != getcolor(P2))))

#define other_square_occupied_by_enemy(P1,P2) ((EDGE != P2) && (EMPTY != P2) && (getcolor(P1) != getcolor(P2)))

/* With 9x9 board, we get maximum coordinate as
   10011001 in binary = 9*16 + 9 = 153. */

#define makecoord(x,y) ((COR) (((x) << 4) + (y)))
#define makecoord_with_dest_square(x,y,dest_x,dest_y) ((COR) (((x) << 4) + (y)) + (((dest_x) << 12) + ((dest_y) << 8)))

#define get_x(c) ((c) >> 4) /* (((c) >> 4) & 15) */
#define get_y(c) ((c) & 15)

#define get_dest_x(c) (((c) >> 12) ? (((c) >> 12) && 15) : get_x(c))
#define get_dest_y(c) (((c) >> 8) ? (((c) >> 8) && 15) : get_y(c))

extern int BSIZE;
#define MAXBSIZE 9

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

/* From bishop (2) to queen (10). There can't be any promoted pieces in hand. */
#define MAX_IN_HAND 10

/* I think true maximum is 6 (for gold) so this should be enough: */
#define MAXVEC 10

/* In Shoogi one square can be threated by max. 10 pieces.
   (Eight from sides, and by two horses.), but by 16 in Chess
   (eight possible knight moves)
 */
#define MAXTHREATS 65 /* Was 17 before, Was 11 for SHOOGI */

#define MAXSQUARES (MAXBSIZE*MAXBSIZE)
#define MAXFREEDOMS (MAXSQUARES+10)

#define MAXFUNS 25
#define MAXCMDS MAXBUF

#define TMP_BOARD 1
#define ERRMOVES (-2L)
#define NOCOLOR   2



struct GIS
 {
   int g_gametype; /* 0 for Chess, 1 for Shoogi */
   int g_bsize; /* 8 for Chess, 9 for Shoogi */
   unsigned char **g_board; /* [MAXBSIZE+4][MAXBSIZE+4]; */
   int g_blacks_king[2]; /* Track of the location of the kings are kept in these vectors */
   int g_whites_king[2]; /*  X in first element, Y in second. */
   int *g_kings[2]; /* = { g_blacks_king, g_whites_king }; */
   int g_officers_moved[2]; /* = { for g_blacks_officers_moved, g_whites_officers_moved }; */
   PIECE g_blacks_hand[MAX_IN_HAND+2];
   PIECE g_whites_hand[MAX_IN_HAND+2];
   PIECE *g_hands[2]; /* = { g_blacks_hand, g_whites_hand }; */
   PFI g_movefuns[MAXFUNS];


   ULI g_output_flags;

   int g_movesmade;     /* Moves made so far. */
   int g_movestat;       /* Status returned by last call of check_move */
   PIECE g_captured_piece; /* Last piece captured, if the last move was capture. */

   PIECE g_prev_moved_piece; /* The piece moved by opponent just before this move to be made. */
   COR g_prev_src; /* And the source ... */
   COR g_prev_dst; /* ... and the destination coordinates of that move. (for "en passant" captures) */

   int g_b_impasse_points; /* = DEFAULT_IMPASSE_POINTS; */
   int g_w_impasse_points; /* = DEFAULT_IMPASSE_POINTS; */

   

   char g_latest_move[MAXBUF+3];

   char g_blackname[SMALLBUF]; /* = {'\0'}; */
   char g_whitename[SMALLBUF]; /* = {'\0'}; */
   char *g_names[2]; /* = { g_blackname, g_whitename }; */

 };


/* Pointer to Piece Movement Function: */
typedef int (*PMF)(struct GIS *gis,COR *vec,int x,int y,int *capt_x,int *capt_y,PIECE thispiece,int movetype,int dir);

#define mv_shoogip(gis) ((gis)->g_gametype)
#define mv_bsize(gis) ((gis)->g_bsize)
#define mv_square(gis,x,y) ((gis)->g_board[(x)+1][(y)+1])
#define mv_flags(gis) ((gis)->g_output_flags)
#define mv_movefun(gis,i) ((PMF) (gis)->g_movefuns[i])

#define mv_kings_x(gis,color) (*(gis->g_kings[color]))
#define mv_kings_y(gis,color) (*(gis->g_kings[color]+1))

#define QUEENSIDE_ROOK_HAS_MOVED    1 /* 2^0 */
#define KING_HAS_MOVED             16 /* 2^4 */
#define KINGSIDE_ROOK_HAS_MOVED   128 /* 2^7 */


/* Check whether castlings are permanently impossible: */
#define mv_short_castling_spoiled(gis,color) (0 != (((gis)->g_officers_moved[color]) & (KINGSIDE_ROOK_HAS_MOVED | KING_HAS_MOVED)))
#define mv_long_castling_spoiled(gis,color) (0 != (((gis)->g_officers_moved[color]) & (QUEENSIDE_ROOK_HAS_MOVED | KING_HAS_MOVED)))

/* Remember to call this also when the opponent captures
   one of the player's officers in its starting square!
   (We don't want to allow a castling with non-existent rook,
   or with one which has moved from another board side after
   the original one was captured by the enemy!)
 */
#define mv_set_firstrank_bits(gis,color,file) (((gis)->g_officers_moved[color]) |= (1 << ((file)-1)))
#define mv_all_firstrank_bits(gis,color) ((gis)->g_officers_moved[color])

#define mv_y_is_nth_rankp(gis,color,y,nthrank) (color ? ((y) == (nthrank)) : ((y) == (1+mv_bsize(gis)-(nthrank))))
#define mv_rel2absrank(gis,color,y) ((color) ? (y) : (1+mv_bsize(gis)-(y)))


#define mv_set_prev_move(gis,piece,src_x,src_y,dst_x,dst_y)\
 ((gis->g_prev_moved_piece=(piece)),(gis->g_prev_src = makecoord(src_x,src_y)),(gis->g_prev_dst = makecoord(dst_x,dst_y)))

#define mv_is_prev_move(gis,piece,src_x,src_y,dst_x,dst_y)\
 (((piece)==gis->g_prev_moved_piece)&&(makecoord(src_x,src_y)==gis->g_prev_src)&&(makecoord(dst_x,dst_y)==gis->g_prev_dst))

struct piecename
 {
   char *english;
   char *long_romaji;
   char *short_romaji;
   unsigned short int long_unikanji1;
   unsigned short int long_unikanji2;
   unsigned short int short_unikanji;
 };


#define getowncolor()\
 ((_ownplid_ == _blackplid_) ? 0 : ((_ownplid_ == _whiteplid_) ? 1 : NOCOLOR))

extern char _debug_flag_;

#define E1 stdout
extern char E2[];
extern char OB[];


extern FILE *err_fp;

extern int G_argc;
extern char **G_argv;

