
/*
 * webboard.c & moves.c -- A simple HTML-based shoogi/chess-board
 *                         for experimental use.
 *
 *                         Based on earlier (and more complex)
 *                         Shoogi Server package (shoogi.c, etc.)
 *
 * Copyright (C) 1993-2001 by Antti Karttunen (Antti.Karttunen@iki.fi)
 *
 *
 *    Changes:
 *
 *   20. July  2001    karttu   Major rewrite, all the globals (that are not constant)
 *                              transferred to GIS-structure.
 *
 *                              Still some changes needed to make this truly
 *                              re-entrant. (No static buffers should be used).
 *
 *
 *   23. July  2001    karttu   Fixed the problem with QUEEN, started writing
 *                              castling procedures.
 *
 *                              Still to be done: Special movements for pawn:
 *                               double initial move, diagonal capturing,
 *                               capturing "en passant", promotion.
 *
 *
 */

#include <string.h>
#include "webboard.h"
#include "mv_proto.h" /* Created with grep '^[A-Za-z]' < moves.c | fgrep "struct" | fgrep "(" | sed -e 's/)/);/g' > mv_proto.h */



/* ************************************************************************************ */


#ifdef CRIPPLED_TOUPPER

unsigned int mytoupper(unsigned int c)
{
    if(((c) >= 'a') && ((c) <= 'z')) { return(c-('a'-'A')); }
    else { return(c); }
}

unsigned int mytolower(unsigned int c)
{
    if(((c) >= 'A') && ((c) <= 'Z')) { return(c+('a'-'A')); }
    else { return(c); }
}

#endif



int numberp(s)
char *s;
{
    while(*s) { if(!isdigit(*s++)) { return(0); } }

    return(1);
}

/* This returns 1 if beginning of s1 is equivalent to s2. */
int cutstrequ(s1,s2)
register char *s1,*s2;
{
    int len,result;
    char savec;

    len = strlen(s2);
    if(strlen(s1) < len) { return(0); } /* If s1 shorter than s2. */
    savec = s1[len];
    s1[len] = '\0';
    result = strcmp(s1,s2);
    s1[len] = savec;
    return(!result);
}





/*
This terminal definition should be used in Unix with curses when
 attron(A_ALTCHARSET) code is used: (i.e. now, if kanji 1 is used (JIS)!)
 Fortunately, we don't use curses now!

vt102-jis| (Modified vt102. rmacs and smacs replaced by JIS in&out-seqs by AK),
	am, mir, xenl, xon,
	cols#80, it#8, lines#24, vt#3,
	acsc=``aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~,
	bel=^G, blink=\E[5m$<2>, bold=\E[1m$<2>,
	clear=\E[H\E[J$<50>, cr=\r, csr=\E[%i%p1%d;%p2%dr,
	cub=\E[%p1%dD, cub1=\b, cud=\E[%p1%dB, cud1=\n,
	cuf=\E[%p1%dC, cuf1=\E[C$<2>,
	cup=\E[%i%p1%d;%p2%dH$<5>, cuu=\E[%p1%dA,
	cuu1=\E[A$<2>, dl=\E[%p1%dM, dl1=\E[M, ed=\E[J$<50>,
	el=\E[K$<3>, el1=\E[1K$<3>, enacs=\E(B\E)0, home=\E[H,
	ht=\t, hts=\EH, il=\E[%p1%dL, il1=\E[L, ind=\n,
	ka1=\EOq, ka3=\EOs, kb2=\EOr, kbs=\b, kc1=\EOp,
	kc3=\EOn, kcub1=\EOD, kcud1=\EOB, kcuf1=\EOC,
	kcuu1=\EOA, kent=\EOM, kf0=\EOy, kf1=\EOP, kf10=\EOx,
	kf2=\EOQ, kf3=\EOR, kf4=\EOS, kf5=\EOt, kf6=\EOu,
	kf7=\EOv, kf8=\EOl, kf9=\EOw, rc=\E8, rev=\E[7m$<2>,
	ri=\EM$<5>, rmacs=\E(J, rmkx=\E[?1l\E>, rmso=\E[m$<2>,
	rmul=\E[m$<2>, rs2=\E>\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h,
	sc=\E7,
	sgr=%?%p1%p2%|%p3%|%p4%|%p5%|%p6%|%p7%|%p8%|%p9%|%!%t\E(J%;\E[0%?%p1%p6%|%t;1%;%?%p2%t;4%;%?%p1%p3%|%t;7%;%?%p4%t;5%;m%?%p9%t\E$B%;,
	sgr0=\E[m^O$<2>, smacs=\E$B, smkx=\E[?1h\E=,
	smso=\E[1;7m$<2>, smul=\E[4m$<2>, tbc=\E[3g,
 */


output_stuff(buf,fp)
unsigned char *buf;
FILE *fp;
{
    unsigned char *p,codebuf[81]; /* I hope this is enough. */

    p = NULL;
     {
       while(*buf)
        {
	   {
	     if(p) /* If some ansi-codes collected, then print them: */
	      {
	        fprintf(fp,ANSI_TEMPLATE,codebuf);
		p = NULL;
              }
#ifndef UNIX
/* If not UNIX (e.g. MS-DOS or RSX-11M) then put CR before every LF: */
#ifndef TURBOC
/* But if file is opened as text file with TURBO-C, then it will put that
    CR automatically anyway: */
             if(*buf == '\n') { putc('\r',fp); }
#endif
#endif
	     putc(*buf,fp);
	   }
	  buf++;
	}
     }
}

/* ***************************************************************************************** */



         /*               0123456789ABCDEF0123456789 */
char *mv_piece_letters = " ?BRPLNSGKQ1234567HDTLNSGk";

/* 
Values assigned for each piece when counting up the game, if impasse occurs:
(rooks and bishops count 5 points each and all other pieces (except kings
which are ignored) count 1 point. Promotions are disregarded.)
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
 */
int mv_Shoogi_piecevalues[] =
 {
  0, 0, 5, 5, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 1, 1, 1, 1, 0, 0
 };

int mv_Chess_piecevalues[] =
 {
  0, 0, 5, 5, 1, 1, 1, 1, 1, 10, 5, 0, 0, 0, 0, 0, 0, 0, 5, 5, 1, 1, 1, 1, 0, 0
 };


#define value_of_piece(gis,p) (mv_shoogip(gis) ? mv_Shoogi_piecevalues[p] : mv_Chess_piecevalues[p])

/* This is the true king. The one in array is 'jewel', i.e. like kanji
    for king but one surplus dot: */
struct piecename mv_Shoogi_true_king =
   { "king",              "oushou",       "ou",       /* &#29579;&#23558; */ 0x738B, 0x5C06, 0x738B };

struct piecename mv_Shoogi_piecenames[] =
 {
   { "<EMPTY>",           "<EMPTY>",      "<EMPTY>",  /* &#12288;&#12288; */ 0x3000, 0x3000, 0x3000 },
   { "<EDGE>",            "<EDGE>",       "<EDGE>",   /* &#65311; */         0xFF1F, 0xFF1F, 0xFF1F }, /*?*/
   { "bishop",            "kakugyou",     "kaku",     /* &#35282;&#34892; */ 0x89D2, 0x884C, 0x89D2 },
   { "rook",              "hisha",        "hisha",    /* &#39131;&#36554; */ 0x98DB, 0x8ECA, 0x98DB },
   { "pawn",              "fuhyou",       "fu",       /* &#27497;&#20853; */ 0x6B69, 0x5175, 0x6B69 },
   { "lance",             "kyousha",      "kyou",     /* &#39321;&#36554; */ 0x9999, 0x8ECA, 0x9999 },
   { "knight",            "keima",        "kei",      /* &#26690;&#39340; */ 0x6842, 0x99AC, 0x6842 },
   { "silver",            "ginshou",      "gin",      /* &#37504;&#23558; */ 0x9280, 0x5C06, 0x9280 },
   { "gold",              "kinshou",      "kin",      /* &#37329;&#23558; */ 0x91D1, 0x5C06, 0x91D1 },
   { "king",              "gyokushou",    "gyoku",    /* &#29577;&#23558; */ 0x7389, 0x5C06, 0x7389 }, /* jewel */
/* Here JIS codes for 10 - 17 inside circles are used: (these are needed
   only if a bug occurs...) In Unicode: Use full-width digits 0-7. */
   { "<10>",              "<10>",         "<10>",     /* &#65296; */         0xFF10, 0xFF10, 0xFF10 },
   { "<11>",              "<11>",         "<11>",     /* &#65297; */         0xFF11, 0xFF11, 0xFF11 },
   { "<12>",              "<12>",         "<12>",     /* &#65298; */         0xFF12, 0xFF12, 0xFF12 },
   { "<13>",              "<13>",         "<13>",     /* &#65299; */         0xFF13, 0xFF13, 0xFF13 },
   { "<14>",              "<14>",         "<14>",     /* &#65300; */         0xFF14, 0xFF14, 0xFF14 },
   { "<15>",              "<15>",         "<15>",     /* &#65301; */         0xFF15, 0xFF15, 0xFF15 },
   { "<16>",              "<16>",         "<16>",     /* &#65302; */         0xFF16, 0xFF16, 0xFF16 },
   { "<17>",              "<17>",         "<17>",     /* &#65303; */         0xFF17, 0xFF17, 0xFF17 },
   { "promoted bishop",   "ryuuma",       "uma",      /* &#31452;&#39340; */ 0x7ADC, 0x99AC, 0x99AC },
   { "promoted rook",     "ryuuou",       "ryuu",     /* &#31452;&#29579; */ 0x7ADC, 0x738B, 0x7ADC },
   { "promoted pawn",     "tokin",        "tokin",    /* &#12392;&#37329; */ 0x3068, 0x91D1, 0x3068 },
   { "promoted lance",    "narikyou",     "narikyou", /* &#25104;&#39321; */ 0x6210, 0x9999, 0x91D1 },
   { "promoted knight",   "narikei",      "narikei",  /* &#25104;&#26690; */ 0x6210, 0x6842, 0x91D1 },
   { "promoted silver",   "narigin",      "narigin",  /* &#25104;&#37504; */ 0x6210, 0x9280, 0x91D1 },
   { "<promoted gold>",   "<narikin>",    "<narikin>",/* &#25104;&#37329; */ 0x6210, 0x91D1, 0x91D1 },
   { "<promoted king>",   "<narigyoku>",  "<narigyoku>",/* &#25104;&#29577; */ 0x6210, 0x7389, 0x7389 }
 };



struct piecename mv_Chess_piecenames[] =
 {
   { "<EMPTY>",           "<EMPTY>",      "<EMPTY>",  /* &#12288;&#12288; */ 0x3000, 0x3000, 0x3000 },
   { "<EDGE>",            "<EDGE>",       "<EDGE>",   /* &#65311; */         0xFF1F, 0xFF1F, 0xFF1F }, /*?*/
   { "bishop",            "kakugyou",     "kaku",     /* &#35282;&#34892; */ 0x89D2, 0x884C, 0x2657 },
   { "rook",              "hisha",        "hisha",    /* &#39131;&#36554; */ 0x98DB, 0x8ECA, 0x2656 },
   { "pawn",              "fuhyou",       "fu",       /* &#27497;&#20853; */ 0x6B69, 0x5175, 0x2659 },
   { "lance",             "kyousha",      "kyou",     /* &#39321;&#36554; */ 0x9999, 0x8ECA, 0x9999 },
   { "knight",            "keima",        "kei",      /* &#26690;&#39340; */ 0x6842, 0x99AC, 0x2658 },
   { "silver",            "ginshou",      "gin",      /* &#37504;&#23558; */ 0x9280, 0x5C06, 0x9280 },
   { "gold",              "kinshou",      "kin",      /* &#37329;&#23558; */ 0x91D1, 0x5C06, 0x91D1 },
   { "king",              "gyokushou",    "gyoku",    /* &#29577;&#23558; */ 0x7389, 0x5C06, 0x2654 },
   { "queen",             "reina",        "<10>",     /* &#65296; */         0xFF10, 0xFF10, 0x2655 },
 };



#define getptrtopstruct(gis,piece) ((mv_shoogip(gis)) ? ((whitep(piece) && (piece == makewhite(KING)))\
 ? &mv_Shoogi_true_king : &mv_Shoogi_piecenames[color_off(piece)]) : &mv_Chess_piecenames[color_off(piece)])


unsigned short int mv_Unumbers[11] =
 {
   /* "Nm", &#38646; */ 0x96F6,
   /* "0l", &#19968; */ 0x4E00,
   /* "Fs", &#20108; */ 0x4E8C,
   /* ";0", &#19977; */ 0x4E09,
   /* ";M", &#22235; */ 0x56DB,
   /* "8^", &#20116; */ 0x4E94,
   /* "O;", &#20845; */ 0x516D,
   /* "<7", &#19971; */ 0x4E03,
   /* "H,", &#20843; */ 0x516B,
   /* "6e", &#20061; */ 0x4E5D,
   /* "==", &#21313; */ 0x5341
 };



char *mv_getpiecename(struct GIS *gis,PIECE piece)
{
    struct piecename *ptr;

    ptr = getptrtopstruct(gis,piece);

    switch(getpnamestyle(mv_flags(gis)))
     {
       case ENGLISH: { return(ptr->english); }
       case LONG_ROMAJI:
        { return(ptr->long_romaji);  }
       case SHORT_ROMAJI:
        { return(ptr->short_romaji); }
     }
}




char *mv_getpieceshort(struct GIS *gis,UBYTE *dst,PIECE piece)
{
    int c;
    char tmpbuf[3];

    *dst = 0; /* Initialize this to be empty. */

    switch(getboardstyle(mv_flags(gis)))
     {
       case ASCII:
        {
          c = mv_piece_letters[unpromote(color_off(piece))];
          dst[0] = (promotedp(piece) ? '+' : ' ');
          dst[1] = (whitep(piece) ? tolower(c) : c);
	  dst[2] = '\0';
	  break;
	}
       case UNIKANJI: case (UNIKANJI+HTML_BOARD):
        {
          sprintf(dst,"%s&#%u;%s",
              (((3 != 3) && whitep(piece)) ? "<FONT COLOR=\"red\">" : ""),
               getptrtopstruct(gis,piece)->short_unikanji + (mv_shoogip(gis) ? 0 : ((piece > 1) && blackp(piece))*6),
              (((3 != 3) && whitep(piece)) ? "</FONT>" : ""));
	  break;
	}
     }

    return((char *) dst);
}


/* If num is outside of 0-20 then the dst buf will be filled with garbage.
   Returns 1 or 2 depending whether one or two kanji are required.
 */
int mv_getkanjinum(struct GIS *gis,char *dst,int num)
{
    int cnt;

    *dst = '\0'; /* Initialize this to be empty. */

    cnt = 1;
    if(num > 10)
     {
        {
          sprintf(dst,"&#%u;", mv_Unumbers[10]);
        }
       num -= 10; cnt++;
     }

     {
       dst += strlen(dst);
       sprintf(dst,"&#%u;", mv_Unumbers[num]);
     }

    return(cnt);
}


/* Return a one-based index to the position in the coordinate vector vec
   where the coordinate-pair c is found,
   and zero if not found at all.
 */
int src_coordinates_present(COR c,COR *vec)
{
    COR *p = strchr(vec,c);

    if(!p) { return(0); }
    else { return((p-vec)+1); }
}

/* Just deletete the ith (one-based) coordinate-pair in coordinate vector vec: */
int delete_nth_coordinates(COR *vec, int i)
{
    strcpy(((vec+i)-1),(vec+i));
}


struct GIS *mv_new_gis(int gametype,int board_size)
{
    int i;

    struct GIS *gis = ((struct GIS *) calloc(1, sizeof(struct GIS)));
    if(NULL == gis)
     {
       fputs("mv_new_gis: Memory exhausted when tried to allocate a memory block of size: ",stdout);
       fprintf(stdout,"%u\n",sizeof(struct GIS));
       return(NULL);
     }

    gis->g_gametype = gametype;
    gis->g_bsize = board_size;

    if(NULL == (gis->g_board = ((unsigned char **) calloc(board_size+4, sizeof(unsigned char *)))))
     {
       fputs("mv_new_gis: Memory exhausted when tried to allocate for the board a memory block of size: ",stdout);
       fprintf(stdout,"%u\n",((board_size+4) * sizeof(unsigned char *)));
       return(NULL);
     }
    for(i = 0; i < (board_size+4); i++)
     {
       if(NULL == (*((gis->g_board)+i) = ((unsigned char *) calloc(board_size+4, sizeof(unsigned char)))))
        {
          fputs("mv_new_gis: Memory exhausted when tried to allocate for the board a memory block of size: ",stdout);
          fprintf(stdout,"%u\n",((board_size+4) * sizeof(unsigned char)));
          return(NULL);
        }
     }

    gis->g_kings[0] = gis->g_blacks_king;
    gis->g_kings[1] = gis->g_whites_king;

    gis->g_officers_moved[0] = gis->g_officers_moved[1] = 0;

    gis->g_hands[0] = gis->g_blacks_hand;
    gis->g_hands[1] = gis->g_whites_hand;


    gis->g_output_flags = DEFAULT_FLAGS;

    gis->g_movesmade = gis->g_movestat = 0;
    gis->g_captured_piece = gis->g_prev_moved_piece = 0;
    gis->g_prev_src = gis->g_prev_dst = 0;

    gis->g_b_impasse_points = DEFAULT_IMPASSE_POINTS;
    gis->g_w_impasse_points = DEFAULT_IMPASSE_POINTS;

    gis->g_blackname[0] = gis->g_whitename[0] = '\0';

    gis->g_names[0] = gis->g_blackname;
    gis->g_names[1] = gis->g_whitename;

    mv_initmovefuns(gis);
    mv_initboard(gis);


    return(gis);
}




/* **************************************************************************************** */


PIECE mv_Shoogi_firstrank[9] =
 { LANCE, KNIGHT, SILVER, GOLD, KING, GOLD, SILVER, KNIGHT, LANCE };

PIECE mv_Chess_firstrank[9] =
 { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };


int mv_Shoogi_knight_moves[] =
 {
    1,  2,
   -1,  2,
   0,0
 };


int mv_Chess_knight_moves[] =
 {
    1,  2,
   -1,  2,
   -2,  1,
   -2, -1,
   -1, -2,
    1, -2,
    2, -1,
    2,  1,
   0,0
 };

int mv_silver_moves[] =
 {
    0,  1, /* N  */
   -1,  1, /* NE */
   -1, -1, /* SE */
    1, -1, /* SW */
    1,  1, /* NW */
   0,0
 };

int mv_gold_moves[] =
 {
    0,  1, /* N  */
   -1,  1, /* NE */
   -1,  0, /* E  */
    0, -1, /* S  */
    1,  0, /* W  */
    1,  1, /* NW */
   0,0
 };

/* NOTE: DO NOT USE THIS ARRAY FOR ANY OTHER PIECE
   THAN KING, BECAUSE THE CODE IN mv_checkshortmoves
   USES THE CONDITION
((org_moves != mv_king_moves) || !mv_king_checked_in_xy_p(gis,vec2,color,(x+xx),(y+yy)))
   FOR CHECKING THAT THE SQUARE WHICH TO THE KING
   IS TRYING TO MOVE IS NOT THREATENED BY ENEMY PIECES.
 */

int mv_king_moves[] =
 {
    0,  1, /* N  */
   -1,  1, /* NE */
   -1,  0, /* E  */
   -1, -1, /* SE */
    0, -1, /* S  */
    1, -1, /* SW */
    1,  0, /* W  */
    1,  1, /* NW */
   0,0
 };

/* Extra moves for the promoted bishop: */
int mv_xbishop_moves[] =
 {
    0,  1, /* N  */
   -1,  0, /* E  */
    0, -1, /* S  */
    1,  0, /* W  */
   0,0
 };

/* Extra moves for the promoted rook: */
int mv_xrook_moves[] =
 {
   -1,  1, /* NE */
   -1, -1, /* SE */
    1, -1, /* SW */
    1,  1, /* NW */
   0,0
 };


char *mv_getpiececode(struct GIS *gis,UBYTE *dst,int piece)
{
    int c;
    UBYTE *org_dst = dst;

    c = mv_piece_letters[unpromote(color_off(piece))];
    if(promotedp(piece)) { *dst++ = '+'; }
    *dst++ = (whitep(piece) ? tolower(c) : c);
    *dst = '\0';

    return((char *) org_dst);
}

/* The functions mv_checkshortmoves, mv_checklongmoves
   and all the "piece-methods" from mv_pawn_ to mv_prom_rook_
   expect normally the destination square in coordinates x,y
   from which they then check all the squares where from
   the expected piece could jump from, and the squares which
   contain that kind of piece (variable thispiece) are then
   collected to vector vec.
   This mode of use is used for checking the validity of moves
   (function mv_valid_movep), to find any pieces of the opponent
   that threaten the square x,y (or one's own which could move there,
   to avoid a check for example) (function mv_find_threatening_pieces).

   However, if color_off(thispiece) is OPPONENT_OR_EMPTY, then
   these functions work in other direction:
   Then they expect that the source square of the piece is given
   by x,y and they try to find all the squares that are empty
   or occupied by opponent's pieces to which the piece could
   move (or capture) to.

   This way of calling is used by the function mv_find_available_squares
*/



int mv_checkshortmoves(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir,int *moves)
{
    int count=0;
    int tent_piece;
    register int xx,yy;
    int *org_moves = moves;
    COR vec2[MAXTHREATS];

    while(((xx = *moves++),(yy = *moves++),(xx || yy)))
     {
       tent_piece = mv_square(gis,(x+xx),(y+(dir*yy)));
       if(((OPPONENT_OR_EMPTY == color_off(thispiece))
             && other_square_empty_or_enemys(thispiece,tent_piece)
             && ((org_moves != mv_king_moves) || !mv_king_checked_in_xy_p(gis,vec2,getcolor(thispiece),(x+xx),(y+(dir*yy))))
          )
          || (tent_piece == thispiece))
        {
          *vec++ = makecoord((x+xx),(y+(dir*yy))); count++;
        }
     }

    *vec = 0;
    return(count);
}


/*
   This is used by pieces which can move more than one step at time
   (orthogonally or diagonally, i.e. not knight), that is:
   lance, bishop, rook and the latter two as promoted.
   (Also: The Queen in Chess)
 */
int mv_checklongmoves(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int xx,int yy)
{
    int count = 0;
    int tent_piece;

    do
     {
       x += xx;
       y += yy;
       tent_piece = mv_square(gis,x,y);
       if((OPPONENT_OR_EMPTY == color_off(thispiece))
             && other_square_empty_or_enemys(thispiece,tent_piece))
        {
          *vec++ = makecoord(x,y); count++;
        }
     } while(tent_piece == EMPTY);

/* Should never match if we were called with thispiece as OPPONENT_OR_EMPTY */
    if(mv_square(gis,x,y) == thispiece)
     { *vec = makecoord(x,y); return(1); }

    return(count);
}


int mv_Shoogi_pawn_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int tent_piece = mv_square(gis,x,(y+dir));

    if(((OPPONENT_OR_EMPTY == color_off(thispiece))
          && other_square_empty_or_enemys(thispiece,tent_piece))
       || (tent_piece == thispiece))
     { *vec = makecoord(x,(y+dir)); vec[1] = 0; return(1); }
    *vec = 0;
    return(0);
}


/* The code here is extremely kludgous, because the rules of
   the (western) Chess, of what relates to the movement of the pawn
   are so kludgous.
 */

int mv_Chess_pawn_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int i,count = 0;
    int tent_piece,this_square;
    int en_passant_capture_possible;
    int color = getcolor(thispiece);

    if(OPPONENT_OR_EMPTY == color_off(thispiece)) /* Search available squares, where a pawn can move from here (x,y) */
     {
       if(EMPTY == (tent_piece = mv_square(gis,x,(y+dir)))) /* Can move straight forward? */
        {
          vec[count++] = makecoord(x,y+dir);
          if(mv_y_is_nth_rankp(gis,color,y,2) && (EMPTY == (tent_piece = mv_square(gis,x,(y+(2*dir))))))
           { vec[count++] = makecoord(x,y+(2*dir)); }
        }
       for(i = -1; (i <= +1); i +=2) /* Find also possible captures. */
        {
          tent_piece = mv_square(gis,(x+i),(y+dir));
          if(other_square_occupied_by_enemy(thispiece,tent_piece)
              || /* or it's "en passan"t capture, in which case the diagonally opposite square must be empty. */
             (mv_y_is_nth_rankp(gis,color,y,5)
               && mv_is_prev_move(gis,ofcolor(PAWN,!color),(x+i),mv_rel2absrank(gis,color,7),(x+i),mv_rel2absrank(gis,color,5)))
            )
           {
             vec[count++] = makecoord((x+i),y+dir);
           }
        }
     }
    else /* Find pawns that can move to or are threating this square x,y */
     {
       this_square = mv_square(gis,x,y);
       if((!movetype || ('-' == movetype)) && (EMPTY == this_square)) /* Can move straight forward? */
        {
          if(thispiece == (tent_piece = mv_square(gis,x,(y+dir)))) /* There's a pawn coming here straight forward? */
           {
             vec[count++] = makecoord(x,y+dir);
           }
          else if(mv_y_is_nth_rankp(gis,color,y,4)
                   && (EMPTY == tent_piece)
                   && (thispiece == mv_square(gis,x,(y+(2*dir))))
                 )
           {
             vec[count++] = makecoord(x,y+(2*dir));
           }
        }


/* movetype is 'x' if we are called from mv_valid_movep, but 'X' if we are called
   from mv_checkmatedp via mv_find_threatening_pieces, in which case we should
   look for one square left or right, instead of the diagonally adjacent
   square in case of "en passant" capture. */

       if((!movetype || ('x' == movetype))) /* Find (also) possible captures from two squares (x+-1,y+dir) */
        {
          en_passant_capture_possible
            = ((EMPTY == this_square) &&
                   (mv_y_is_nth_rankp(gis,color,y,6)
                     && mv_is_prev_move(gis,ofcolor(PAWN,!color),x,mv_rel2absrank(gis,color,7),x,mv_rel2absrank(gis,color,5))
                   )
              );

          for(i = -1; (i <= +1); i +=2)
           {
             tent_piece = mv_square(gis,(x+i),(y+dir)); /* The possible capturing pawn. */
             if((tent_piece == thispiece)
                 &&
                (other_square_occupied_by_enemy(tent_piece,this_square) || en_passant_capture_possible)
               )
              {
                vec[count++] = makecoord((x+i),(y+dir)); /* of the capturing pawn. */
                if(en_passant_capture_possible && (EMPTY == this_square))
                 {
                   *p_capt_x = x;
                   *p_capt_y = (y+dir); /* mv_rel2absrank(gis,color,5); */
                 }
              }
           }
        }
       else if('X' == movetype)
        {

/* Note that here we may have upto four (4) pawns that
   could capture the pawn in the square (x,5), two as ordinary captures from
   (x+-1,4), and two "en passant"-captures from (x+-1,5): */

          for(i = -1; (i <= +1); i +=2)
           {
             tent_piece = mv_square(gis,(x+i),(y+dir)); /* The possible capturing pawn. */
             if((tent_piece == thispiece) && other_square_occupied_by_enemy(tent_piece,this_square))
              {
                vec[count++] = makecoord((x+i),(y+dir)); /* of the ordinarily capturing pawn. */
              }

             if((mv_y_is_nth_rankp(gis,color,y,5)
                     && mv_is_prev_move(gis,ofcolor(PAWN,!color),x,mv_rel2absrank(gis,color,7),x,mv_rel2absrank(gis,color,5))
                )
                && (thispiece == (tent_piece = mv_square(gis,(x+i),y))))
              {
                vec[count++] = makecoord((x+i),y); /* of the "en passant" capturing pawn. */
              }
           }
        }
     }

    vec[count] = 0;
    return(count);
}


int mv_lance_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;

    count = mv_checklongmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,0,dir);
    vec[count] = 0;
    return(count);
}


int mv_Shoogi_knight_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_Shoogi_knight_moves));
}


int mv_Chess_knight_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_Chess_knight_moves));
}


int mv_silver_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_silver_moves));
}

int mv_gold_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_gold_moves));
}

int mv_Shoogi_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_king_moves));
}

int mv_Chess_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count = 0;
    char errmsgbuf[2049];

    if(OPPONENT_OR_EMPTY == color_off(thispiece)) /* Searching for available squares. */
     {
       if(mv_castling_possible(gis,getcolor(thispiece),errmsgbuf,0))
        {
          vec[count++] = makecoord(x+2,y); /* If short castling possible, then the king can move 2 squares right. */
        }

       if(mv_castling_possible(gis,getcolor(thispiece),errmsgbuf,1))
        {
          vec[count++] = makecoord(x-2,y); /* If long castling possible, then the king can move 2 squares left. */
        }
     }

    return(count + mv_checkshortmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_king_moves));
}

/* A common method. */
int mv_king_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    return(mv_shoogip(gis) ? mv_Shoogi_king_(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir)
                           : mv_Chess_king_(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir));
}

int mv_bishop_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;

    count =  mv_checklongmoves(gis,vec        ,x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1, 1);
    vec[count] = 0; /* End marker. */
    return(count);
}

int mv_rook_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;

    count  = mv_checklongmoves(gis,vec        ,x,y,p_capt_x,p_capt_y,thispiece,movetype,0, 1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,0);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,0,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1,0);
    vec[count] = 0; /* End marker. */
    return(count);
}


int mv_queen_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;

    /* Files and Ranks. */
    count  = mv_checklongmoves(gis,vec        ,x,y,p_capt_x,p_capt_y,thispiece,movetype,0, 1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,0);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,0,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1,0);

    /* And the diagonals. */
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,-1,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1,-1);
    count += mv_checklongmoves(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype, 1, 1);
    vec[count] = 0; /* End marker. */
    return(count);
}

int mv_prom_bishop_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;
/* First check extra moves got by promotion, then the ordinary bishop moves: */
    count = mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_xbishop_moves);
/* Count & Bishop, good pals, eh? */
    return(count + mv_bishop_(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,dir));
}


int mv_prom_rook_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    int count;
/* First check extra moves got by promotion, then the ordinary rook moves: */
    count = mv_checkshortmoves(gis,vec,x,y,p_capt_x,p_capt_y,thispiece,movetype,dir,mv_xrook_moves);
    return(count + mv_rook_(gis,(vec+count),x,y,p_capt_x,p_capt_y,thispiece,movetype,dir));
}


int mv_bugmove_(struct GIS *gis,COR *vec,int x,int y,int *p_capt_x,int *p_capt_y,PIECE thispiece,int movetype,int dir)
{
    errlog((E1,"**Fatal internal error: mv_bugmove_(gis,vec,%d,%d,%08lx,%08lx,%d,%d,%d) called!\n",
             x,y,((unsigned long int) p_capt_x),((unsigned long int) p_capt_y),thispiece,movetype,dir));
    return(0);
}


int mv_checkedp(struct GIS *gis,COR *vec,int color)
{
    register int x;

    /* If king is not on the board, then it can't be checked! */
    if(!(x = mv_kings_x(gis,color))) { return(0); }
    return(mv_find_threatening_pieces(gis,vec,x,mv_kings_y(gis,color),!color,'x'));
}

int mv_king_checked_in_xy_p(struct GIS *gis,COR *vec,int color,int try_x,int try_y)
{
    int old_x = mv_kings_x(gis,color);
    int old_y = mv_kings_y(gis,color);
    PIECE save_piece = mv_square(gis,try_x,try_y);
    int res;

    if(old_x) { mv_square(gis,old_x,old_y) = EMPTY; }

    /* Move king tentatively to the new location try_x,try_y :*/
    mv_square(gis,try_x,try_y) = ofcolor(KING,color);
    mv_kings_x(gis,color) = try_x;
    mv_kings_y(gis,color) = try_y;

    /* And check whether it is checked there: */
    res = mv_find_threatening_pieces(gis,vec,try_x,try_y,!color,'x');

    if(old_x) { mv_square(gis,old_x,old_y) = ofcolor(KING,color); } /* Restoration of the king. */
    mv_square(gis,try_x,try_y) = save_piece;
    mv_kings_x(gis,color) = old_x;
    mv_kings_y(gis,color) = old_y;

    return(res);
}


/* mv_checkmatedp - Not so easy as you may think...

   There is three ways to respond to check:

   1) Escape to some neighbouring square, possibly capturing
       an enemy piece being there.

   2) Capture the offending piece. If it is adjacent to king, then this
       is same as the case 1.

   3) a) Put some piece between the offender and the king. (From the board).
      b) Drop some piece between the offender and the king. (From the hand).

   If offending piece is adjacent to king, or it is knight, then
   the case 3 is not possible.

   If it is doublecheck, then only the case 1 is possible, unless
   the other piece is adjacent. If so, then also the case 2 is possible.

   Case 3 is possible only when the offender is lance, rook or bishop,
   which is not adjacent.

   Note that in all three cases the validity of moves should be checked.
   Especially in the case 3b should be tested whether the drop is valid.

   Even the "mate with the pawn drop" case should be checked, if the
   following kind of situation happens:
   If the white has just checked black by moving bishop from somewhere
   to 6a, then the black can't drop pawn to 5b because it would then
   mate the white's king. Instead (s)he can capture the bishop with
   rook the from 7a.

   Note that you cannot use the castling (in western Chess) to avoid
   a check which means we can avoid further contraptions here.

   Hmm. voisiko matemaattisesti todistaa ettei Shoogissa voi olla sellaista
   tilannetta miss{ "mate with the pawn drop" olisi ainoa keino
   est{{ matti? T{ss{kin voi tuon l{hetin ensin sy|d{ tornilla.
   (Miten niin...)

...7 6 5 4...
  +-+-+-+-+  
a :R b k n:
  +-+-+-+-+
b : :p: : :
  +-+-+-+-+
c : : : :K:
. +-+-+-+-+
.
.

 */

int mv_checkmatedp(struct GIS *gis,int color,char *escmsgbuf)
{
    register int *moves;
    register int xx,yy;
    int x,y, capt_x,capt_y;
    int off_x,off_y,off_piece,neighbor;
    int hidings;
    int count;
    char buf1[20];
    COR vec[MAXTHREATS],vec2[MAXTHREATS];
/*  unsigned int save_firstrank_bits; */

    x = mv_kings_x(gis,color);
    y = mv_kings_y(gis,color);

    count = mv_checkedp(gis,vec,color);
    if(!count) { return(0); } /* Nobody is threatening this king right now. */
    
    off_x = get_x(*vec); /* Get the (first) offender's coordinates. */
    off_y = get_y(*vec);

#ifdef OLD_WAY
    /* First check if there are any escape locations: */
    moves = mv_king_moves;
    /* Loop as long at least other one is non-zero: */
    while(((xx = *moves++),(yy = *moves++),(xx || yy)))
     {
       register int xxx,yyy;
       int result;

       xxx = x+xx; yyy = y+yy;

       if((off_x == (xxx)) && (off_y == (yyy))) { adjacent=1; }

        /* If found empty place or one occupied by an enemy: */
       if(((neighbor = mv_square(gis,(xxx),(yyy))) != EDGE) &&
          ((neighbor == EMPTY) || (getcolor(neighbor) != color)))
        {
          /* And check whether there are threats to that adjacent square: */
          if(!mv_king_checked_in_xy_p(gis,vec2,color,xxx,yyy))
           {
	     check_message((escmsgbuf,"'s king could %s to the square %s",
	                   ((neighbor == EMPTY) ? "escape"
			                        : "capture"),
			   mv_getcoord_b(gis,buf1,xxx,yyy)));
             return(0);
           }
        }
     }
#else
/* THIS NEW WAY GIVES ALL THE POSSIBLE ESCAPE-SQUARES: */
/* Castling cannot be used to avoid check, so let's use mv_Shoogi_king instead of mv_Chess_king,
   because the former doesn't know anything about the castling: */

    if(0 < (hidings = mv_Shoogi_king_(gis,vec2,x,y,&capt_x,&capt_y,ofcolor(OPPONENT_OR_EMPTY,color),0,get_attacking_direction(color))))
     {
       int pos;
       if(0 != (pos = src_coordinates_present(makecoord(off_x,off_y),vec2)))
        {
          delete_nth_coordinates(vec2,pos);

          check_message((escmsgbuf,"'s king could capture the offender at square %s",
                           mv_getcoord_b(gis,buf1,off_x,off_y)));
          if(hidings > 1) { strcat(escmsgbuf," or escape to the square "); }
        }
       else
        {
          check_message((escmsgbuf,"'s king could escape to the square "));
        }
       mv_sprintcoords_or(gis,escmsgbuf+strlen(escmsgbuf),vec2);
       return(0);
     }

#endif

    /* If it's double check and there's no escape for king, then it's
       sure mate, because we can't interpose nor capture the both offenders
       at the same time.
       (E.g. bishop is moved from the front of rook and then they both
        threat the king.)
       Note that the flag adjacent (got from previous loop) could contain
       the incorrect value "in principle" if there were more than one
       offenders, but we don't need to care about that because of this
       check here.
     */

    if(count > 1) { return(1); }

    /* If there are any own piece(s) which could capture this offending piece: */
    if(count = mv_find_threatening_pieces(gis,vec2,off_x,off_y,color,'X'))
     { /* And some of them can be used to capture the offender without
          checking the king: */
       if(!mv_check_if_threatened(gis,vec2,x,y,off_x,off_y,color,escmsgbuf,'X'))
        { return(0); }
     }

    off_piece = mv_square(gis,off_x,off_y);

    /* If piece is adjacent (don't need that check..., mv_interpose_possible will anyways
       return 0) or it is knight, then it's sure mate,
        because we can put nothing between: */
    if(/* adjacent || */ (color_off(off_piece) == KNIGHT)) { return(1); }

    /* Check whether it is possible to move or drop any pieces between:
       (it is silently assumed here that the offending piece is one
       of the long ranging, and straight moving officers,
       i.e. a lance, bishop, rook or queen) */
    return(!mv_interpose_possible(gis,vec,x,y,off_x,off_y,color,escmsgbuf));
}

/*
   Checks with mv_check_if_threatened every square between x,y and x2,y2
   and if finds some piece which could be interposed there without
   check, then return 1 immediately, otherwise zero.
 */
int mv_interpose_possible(struct GIS *gis,COR *vec,int x,int y,int x2,int y2,int color,char *escmsgbuf)
{
    register int x3,y3; /* Varying from x,y to x2, y2. */
    register int xx,yy; /* The directions. */

    xx = ((x2 - x) ? (((x2 - x) < 0) ? -1 :  1) : 0);
    yy = ((y2 - y) ? (((y2 - y) < 0) ? -1 :  1) : 0);
    x3 = x;
    y3 = y;

/*  while(!((x3 == x2) && (y3 == y2))) */
    for(x3 += xx, y3 += yy; (mv_square(gis,x3,y3) == EMPTY); x3 += xx, y3 += yy)
     {
       /* If any drop is possible then return 1 immediately: */
       if(mv_any_drop_possible(gis,gis->g_hands[color],x3,y3,color,escmsgbuf)) { return(1); }
       /* If there are any own piece(s) which could move to this square: */
       if(mv_find_threatening_pieces(gis,vec,x3,y3,color,0)   &&
       /* And some of them can be used to avoid check: */
          !mv_check_if_threatened(gis,vec,x,y,x3,y3,color,escmsgbuf,0))
        { return(1); }
     }

    { /* Make some internal error checking, i.e. if we don't exit prematurely
         from the above loop with return(1) we should end to the offending piece
         which should be one of the long-ranging ones, i.e. a lance, a bishop,
         a rook or a queen: */
      int piece;
      piece = color_off(mv_square(gis,x3,y3));
      if(!((piece == QUEEN) || (piece == LANCE) ||
           (unpromote(piece) == BISHOP) ||
           (unpromote(piece) == ROOK)))
       {
         errlog((E1,"**Internal error encountered when called:\n"));
         errlog((E1,"mv_interpose_possible(gis,vec,%d,%d,%d,%d,%d)\n",
                    x,y,x2,y2,color));
         errlog((E1,"x3=%d, y3=%d, xx=%d, yy=%d, piece=%d\n",
                    x3,y3,xx,yy,piece));
	 return(0);
       }
    }

    return(0);
}


int mv_only_empty_squares_between(struct GIS *gis,int x,int y,int x2,int y2)
{
    register int x3,y3; /* Varying from x,y to x2, y2. */
    register int xx,yy; /* The directions. */

    xx = ((x2 - x) ? (((x2 - x) < 0) ? -1 :  1) : 0); /* Get the offsets. */
    yy = ((y2 - y) ? (((y2 - y) < 0) ? -1 :  1) : 0);
    x3 = x;
    y3 = y;

    for(x3 += xx, y3 += yy; (mv_square(gis,x3,y3) == EMPTY); x3 += xx, y3 += yy)
     {
     }

    return((x3 == x2) && (y3 == y2));
}


int mv_king_checked_in_first_n_squares(struct GIS *gis,COR *vec2,int x,int y,int x2,int y2,int color,int n)
{
    register int x3,y3; /* Varying from x,y to x2, y2. */
    register int xx,yy; /* The directions. */
    int i=1;

    xx = ((x2 - x) ? (((x2 - x) < 0) ? -1 :  1) : 0); /* Get the offsets. */
    yy = ((y2 - y) ? (((y2 - y) < 0) ? -1 :  1) : 0);
    x3 = x;
    y3 = y;

    for(x3 += xx, y3 += yy; (i++ <= n); x3 += xx, y3 += yy)
     {
       if(mv_king_checked_in_xy_p(gis,vec2,color,x3,y3)) { return(x3); }
     }

    return(0);
}


int mv_castling_possible(struct GIS *gis,int color,char *errmsgbuf,int long_castling)
{
       int threatened_file;
       COR vec[MAXTHREATS];
       char buf1[SMALLBUF],buf2[SMALLBUF];

       if(mv_shoogip(gis)) { return(0); }

       if((!long_castling && mv_short_castling_spoiled(gis,color))
           || (long_castling && mv_long_castling_spoiled(gis,color))
         )
        {
          err2log((errmsgbuf,
"?Can't castle anymore at %sside, either the king or the rook of that side has already moved\n",
           (long_castling ? "queen" : "king")));
 /* "(((gis)->g_officers_moved[color]) = %08lx)!\n",
 /* (unsigned long int)((gis)->g_officers_moved[color]) */
          return(0);
        }

       if(!mv_only_empty_squares_between(gis,mv_kings_x(gis,color),mv_kings_y(gis,color),
                                              (long_castling ? 1 : 8),mv_kings_y(gis,color)))
        {
          err2log((errmsgbuf,
"?Can't castle yet at %sside, because there are pieces between the squares %s and %s !\n",
           (long_castling ? "queen" : "king"),
           mv_getcoord_b(gis,buf1,mv_kings_x(gis,color),mv_kings_y(gis,color)),
           mv_getcoord_b(gis,buf2,(long_castling ? 1 : 8),mv_kings_y(gis,color))));
          return(0);
        }

       if(mv_checkedp(gis,vec,color)) /*  do_it = KING_LEFT_IN_CHECK; */
        {
          err2log((errmsgbuf,
"?Can't castle now, because the king is checked by ",
             mv_getcoord(gis,threatened_file,mv_kings_y(gis,color))));
          mv_sprintcoords_and(gis,(errmsgbuf+strlen(errmsgbuf)),vec);
          return(0);
        }


       if(0 !=
            (threatened_file = mv_king_checked_in_first_n_squares(gis,vec,mv_kings_x(gis,color),mv_kings_y(gis,color),
                                                     (long_castling ? 1 : 8),mv_kings_y(gis,color),
                                                     color,2))
         )
        {
          err2log((errmsgbuf,
"?Can't castle now at %sside, because the square %s is checked by ",
             (long_castling ? "queen" : "king"),
             mv_getcoord(gis,threatened_file,mv_kings_y(gis,color))));
          mv_sprintcoords_and(gis,(errmsgbuf+strlen(errmsgbuf)),vec);
          return(0);
        }

       return(1); /* The castling is possible. */
}


/*
   Try to move every piece in turn from coordinates specified in vec to x2, y2 and check
   whether x,y (where king is supposed to be) is threatened after that.
   If found case where it's not threatened, then return zero immediately.
   Otherwise return nonzero.

   Two usages:

   from mv_checkmatedp
    (* If there are any own piece(s) which could capture this offending piece: *)
    if(count = mv_find_threatening_pieces(gis,vec2,off_x,off_y,color,'X'))
     { (* And some of them can be used to capture the offender without
          checking the king: *)
       if(!mv_check_if_threatened(gis,vec2,x,y,off_x,off_y,color,escmsgbuf,'X'))
        { return(0); }
     }

    and from mv_interpose_possible:

    (* If there are any own piece(s) which could move to this square: *)
    (* And some of them can be used to avoid check: *)
    if(mv_find_threatening_pieces(gis,vec,x3,y3,color,'-')   &&
        !mv_check_if_threatened(gis,vec,x,y,x3,y3,color,escmsgbuf,'-'))
     { return(1); }

    Note that it's also possible to avoid a check with "en-passant" capture.
    In the former case we are interested about the "capturing coordinates"
    of the "en passant" capture, while in the latter only the destination
    coordinates of the capturing pawn interests us.


    Neither of these examples work... But there still
    might be some kinky situations, where the capturing
    square and the destination square of the "en passant"
    capturing pawn makes a crucial difference:

    We might have a situation like (NOT!):

   BN.... 7
   ...... 6
   ..Pp.. 5
   ...k.. 4

   abcdef


   Now imagine that the black has just checked the
   white king at b4 with his pawn move c7-c5.
   Now, even if the white responds with "en passant"
   capture d5xc6 (capturing the black pawn at c5)
   it doesn't help the king, because of the black bishop
   at a7.

   Here the interposing is needed (DOESN'T WORK EITHER):

   .N.B.. 7
   ...... 6
   ..Pp.. 5
   k..... 4

   abcdef

   x2,y2 is either the position of the offender that we try to capture
         with some of pieces at coordinates given in vec.
   or it is a position of some empty square.
 */
int mv_check_if_threatened(struct GIS *gis,COR *vec,int x,int y,int x2,int y2,int color,char *escmsgbuf,int movetype)
{
    register int x3,y3;
    int save_piece;
    register int save_piece2,own_king;
    int save_dest_piece, dest_x, dest_y;
    int count=1;
    COR vec2[MAXTHREATS];

    save_piece = mv_square(gis,x2,y2);
    own_king = ofcolor(KING,color);

    while(*vec)
     {
       x3 = get_x(*vec);
       y3 = get_y(*vec);
       mv_square(gis,x2,y2) = save_piece2 = mv_square(gis,x3,y3);
       if((ofcolor(PAWN,color) == save_piece2) && (y3 == y2)) /* It's "en passant" capture. */
        {
          dest_x = x2;
          dest_y = y2 + get_attacking_direction(color); /* (dest_x,dest_y) should be empty. */
        }
       else { dest_x = x2; dest_y = y2; }

       save_dest_piece = mv_square(gis,dest_x,dest_y);

       mv_square(gis,x3,y3) = EMPTY;

       if(save_piece2 == own_king)
        { /* If king used for capturing x2,y2 then check that new square of it: */
          count = mv_find_threatening_pieces(gis,vec2,x2,y2,!color,'x');
        }
       else
        { /* Otherwise check the threats to unmoved loc of king: */
          count = mv_find_threatening_pieces(gis,vec2,x,y,!color,'x');
	}
       mv_square(gis,x3,y3) = save_piece2;
       if((dest_x != x2) || (dest_y != y2)) { mv_square(gis,dest_x,dest_y) = save_dest_piece; }
       if(0 == count) { break; } /* Found safe (capturing/interposing) move. */
       vec++;
     }

    mv_square(gis,x3,y3) = save_piece2;
    mv_square(gis,x2,y2) = save_piece;

    if(!count)
     {
       char buf1[20],buf2[20],buf3[20];
       /* Must use this separately because mv_getcoord uses static buffer: */
      
       if(((dest_x != x2) || (dest_y != y2)))
        {
          check_message((escmsgbuf,
            " could %s the offender at %s by capturing en passant from %s to %s",
             ((save_piece == EMPTY) ? "interpose" : "capture"),
             mv_getcoord_b(gis,buf3,x2,y2),mv_getcoord_b(gis,buf2,x3,y3),mv_getcoord_b(gis,buf1,dest_x,dest_y)));
        }
       else
        {
          check_message((escmsgbuf,
           " could %s the offender by moving %s to %s",
             ((save_piece == EMPTY) ? "interpose" : "capture"),
             mv_getcoord_b(gis,buf2,x3,y3), mv_getcoord_b(gis,buf1,dest_x,dest_y)));
        }
     }
    return(count);
}


/*
   If there is any piece in hand which could be validly dropped to x,y
   then return 1, otherwise zero.
 */
int mv_any_drop_possible(struct GIS *gis,PIECE *hand,int x,int y,int color,char *escmsgbuf)
{
    int i;

    if(!mv_shoogip(gis)) { return(0); } /* Drops not possible in ordinary chess. */
/* If there's no pieces hand at all, then return zero immediately: */
    if(!*hand) { return(0); }

/*  for(;*hand;hand++) Old code. */
/* Check hand for all pieces from bishop to king: */
    for(i=2; i <= MAX_IN_HAND; i++)
     { /* If there is that kind of piece in hand, and it's possible to drop: */
       if(hand[i] && !mv_illegal_drop(gis,i,x,y,color,escmsgbuf))
        {
          check_message((escmsgbuf," could drop %s to %s",
                         mv_getpiecename(gis,ofcolor(i,color)),mv_getcoord(gis,x,y)));
	  return(1);
	}
     }

    return(0);
}


/* Find all pieces of color 'color' which threat the location x,y and
    put them into vec, and return the count of them.
 */
int mv_find_threatening_pieces(struct GIS *gis,COR *vec,int x,int y,int color,int movetype)
{
    register PMF fun;
    register int i;
    register int count=0;
    int dir;
    int capt_x, capt_y;
    int *p_capt_x = &capt_x;
    int *p_capt_y = &capt_y;

    dir = getdir(color);

    for(i=2; i < MAXFUNS; i++)
     {
       if((fun = mv_movefun(gis,((PIECE)i))) == mv_bugmove_) { continue; }
       count += (fun)(gis,(vec+count),x,y,p_capt_x,p_capt_y,ofcolor(i,color),movetype,dir);
     }

    return(count);
}


/* Is the move src_x,src_y -> dest_x,dest_y valid with the piece of color
   'color' ?
 */
int mv_valid_movep(struct GIS *gis,int src_x,int src_y,int dest_x,int dest_y,int *p_capt_x,int *p_capt_y,int piece,int movetype,int color)
{
    COR vec[MAXTHREATS];

/* Do it simple way. First find all the squares which contain pieces of
   type 'piece' and could  enter here: */
    int s = ((*mv_movefun(gis,color_off(piece)))
                  (gis,vec,dest_x,dest_y,p_capt_x,p_capt_y,ofcolor(piece,color),movetype,getdir(color)));

    /* And then check whether source coordinates are among those: */
    if(s) { return(src_coordinates_present(makecoord(src_x,src_y),vec)); }
    else { return(0); }
}



/* Find all the available squares where the piece can move to,
   put them into vec, and return the count of them.
 */
int mv_find_available_squares(struct GIS *gis,COR *vec, int src_x, int src_y, int piece, int color)
{
    int capt_x, capt_y, movetype=0;
    int *p_capt_x = &capt_x;
    int *p_capt_y = &capt_y;

    if((EMPTY == piece) || (getcolor(piece) != color)) { *vec = 0; return(0); }

    return((*mv_movefun(gis,color_off(piece)))
             (gis,vec,src_x,src_y,p_capt_x,p_capt_y,ofcolor(OPPONENT_OR_EMPTY,color),
               movetype,getdir(opponents_color(color))));
}



int mv_handle_handicap(struct GIS *gis,char *movestr,int color,char *errmsgbuf)
{
    int piece,movetype,promf,promtype;
    int src_x,src_y,dest_x,dest_y;
    register int i,result=1;
    register char *cordp,*next;
    unsigned char *squareloc;
    COR handicaps[MAXPIECES];

    if(gis->g_movesmade != 1)
     {
       err2log((errmsgbuf,
"?Handicap move is only possible as the first move of game (Not %lu.)!\n",
         gis->g_movesmade));
       return(0);
     }

    /* First, parse and check the coordinates: (collect them to handicaps) */
    i=0;
    cordp = movestr;
    while(cordp)
     { /* If comma found, then overwrite it so we can read the coordinate: */
       if(next = strchr(cordp,','))
        { *next = '\0'; }
       if(!(piece = mv_parse_notation(gis,cordp,&movetype,&promf,&promtype,
                                &src_x,&src_y,&dest_x,&dest_y,1,errmsgbuf)))
        { result = 0; } /* If parse error. */
       if(next) { *next = ','; } /* Restore the comma. */
       if(!result) { return(result); }
       if(mv_square(gis,dest_x,dest_y) != ofcolor(piece,!color))
        {
	  err2log((errmsgbuf,"?There is no %s %s in the square %s!\n",
           getlowcolname(!color),
	   mv_getpiecename(gis,ofcolor(piece,!color)),
	   mv_getcoord(gis,dest_x,dest_y)));
	  return(0);
	}
       handicaps[i++] = makecoord(dest_x,dest_y);
       if(next) { cordp = next+1; }
       else { cordp = NULL; }
     }
    /* Terminate the handicap vector (not really necessary): */
/*  handicaps[i] = 0; */


    /* Then remove the pieces from the board: */
    while(i--)
     {
       squareloc = &mv_square(gis,get_x(handicaps[i]),get_y(handicaps[i]));
       /* If removing king (heh, heh ;-) then clear its coordinates also,
          so that it can't be checked: */
       if((piece = *squareloc) == ofcolor(KING,!color))
        { mv_kings_x(gis,!color) = mv_kings_y(gis,!color) = 0; }
       *squareloc = EMPTY; /* Empty the location. */
       /* Impasse limit for white is reduced by whatever pieces he gives
           as handicap: */
       gis->g_w_impasse_points -= value_of_piece(gis,color_off(piece));
     }

    return(1);
}



int mv_valid_as_digit(struct GIS *gis,char c)
{
  if(!isdigit(c)) { return(0); }
  if('0' == c) { return(-1); }
  if((c - '0') > mv_bsize(gis)) { return(-1); }
  else { return(c - '0'); }
}

int mv_valid_as_letter(struct GIS *gis,char c)
{
    unsigned char k;
    if(!isalpha(c)) { return(0); }
    k = tolower(c)-('a'-1);
    if(k > mv_bsize(gis)) { return(-1); }
    else { return(k); }
}

int mv_valid_first(struct GIS *gis,char c)
{
    return(mv_shoogip(gis) ? mv_valid_as_digit(gis,c) : mv_valid_as_letter(gis,c));
}

int mv_valid_second(struct GIS *gis,char c)
{
    return(mv_shoogip(gis) ? mv_valid_as_letter(gis,c) : mv_valid_as_digit(gis,c));
}


/* Returns 0 if no coordinates found here, Returns -1
   if erroneous coordinates given, and positive integer
   if valid move coordinates specified.
 */
int mv_parse_coordinate(struct GIS *gis,char **p_movestr,int *p_x,int *p_y)
{
  int s = mv_valid_first(gis,**(p_movestr));
  if(s <= 0) { return(s); }
  else { *p_x = s; }
  if((*p_y = mv_valid_second(gis,*(++*p_movestr))) <= 0) { return(-1); }
  ++*p_movestr;
  return(1);
}

/* Parses the move notation given in movestr, and returns the corresponding
   piece as result (without color), and various information in the
   call_by_ref arguments:

   p_movetype: '-' if ordinary move, 'x' if capture, '*' if drop.
   p_promf:    1 if promoted piece moved, 0 otherwise.
   p_promtype: 0 if not specified, '=' if user wants to keep it same,
               '+' if (s)he wants to promote it.
   In chess, we return in p_promtype either 'B', 'R', 'N' or 'Q' which should follow after the =

   p_src_x, p_src_y: source coordinates for the piece. Zero if drop.
   p_dest_x, p_dest_y: destination coordinates.

   If the notation or move is invalid, then the appropriate error
   message is printed, and zero is returned.
 */
int mv_parse_notation
(struct GIS *gis,char *movestr,int *p_movetype,int *p_promf,int *p_promtype,int *p_src_x,int *p_src_y,int *p_dest_x,int *p_dest_y,int hh,char *errmsgbuf)
{
    char *origmovestr,*p;
    int i,piece;

    origmovestr = movestr;

    *p_src_x = *p_src_y = 0; /* By default, source is not specified. */

    if(!*movestr) { goto ertzu; } /* No null strings, please ! */

    if(*movestr == '+') /* + is optional in the beginning. */
     {
       *p_promf = 1;
       movestr++;
     }
    else { *p_promf = 0; }

    *movestr = toupper(*movestr); /* Make first letter uppercase. */

    /* Check that first character is valid letter: */
    if(isalpha(*movestr) && (p = strchr(mv_piece_letters,*movestr)))
     {
       piece = (p - mv_piece_letters);
       if(*p_promf)
        {
          /* Can't specify +D, +H nor +G or +K */
	  if(promotedp(piece) || !mv_promotablep(gis,piece))
	   {
	     err2log((errmsgbuf,
"?There's no such piece as promoted %s !\n",mv_getpiecename(gis,piece)));
             return(0);
	   }
	  else { piece = promoted(piece); }
	}
     }
    else { goto ertzu; } /* Not valid letter. */

    movestr++; /* Skip past the piece identifier. After which MAY follow the source coordinates: */
    if(mv_parse_coordinate(gis,&movestr,p_src_x,p_src_y) < 0)
     { goto ertzu; }

    /* Get movetype (and convert X to x): */
    *p_movetype = *movestr = tolower(*movestr);

    /* If movetype not valid: */
    switch(*p_movetype)
     { /* Only notations of the form L@1a are allowed with handicaps: */
       case '@':
        { if(!hh) { goto ertzu; } break; }
       case '-':
       case 'x':
       case '*':
        { if(hh) { goto ertzu; } break; }
       default: 
        { goto ertzu; }
     }

    /* Don't accept notations like G5b*4b: */
    /* But nowadays accept erroneous notations like +L*5e and +B@2b here,
       because they are caught later.
     */
    if(((*p_movetype == '*') || (*p_movetype == '@')) &&
        ( /* *p_promf || */ *p_src_x))
     { goto ertzu; }

    /* Then MUST follow the destination coordinates. */
    movestr++; /* Skip past the movetype identifier. */
    if(mv_parse_coordinate(gis,&movestr,p_dest_x,p_dest_y) <= 0)
     { goto ertzu; }

    *p_promtype = *movestr;
    if(*p_promtype && (*p_promtype != '+') && (*p_promtype != '=')) /* Fix for chess! */
     { goto ertzu; } /* If something else than + or = follows */

    if((!mv_shoogip(gis)) && ('=' == *p_promtype))
     {
       *p_promtype = mytoupper(*++movestr);
       if(!strchr("QNRB",*p_promtype)) { goto ertzu; }
     }

    /* Don't accept moves like G-3c+ or +B-3b+ or S*7e= */
    if(!(*p_promtype &&
         (!mv_promotablep(gis,piece) || promotedp(piece) /* *p_promf */ ||
          (*p_movetype == '*') || (*p_movetype == '@'))))
     { return(piece); } /* However, if OK then return the piece. */

ertzu:

    err2log((errmsgbuf,"?Cannot parse notation:\n%s\n",origmovestr));
    for(i=(movestr - origmovestr); i--;) { strcat(errmsgbuf," "); }
    strcat(errmsgbuf,"^\n");
    for(i=(movestr - origmovestr); i--;) { strcat(errmsgbuf," "); }
    strcat(errmsgbuf,(*movestr ? "Invalid character!\n" : "Character(s) missing!\n"));
    return(0);
}



char *strchr2(char *s,char c1,char c2)
{
    char *p1,*p2;
    
    p1 = strchr(s,c1);
    p2 = strchr(s,c2);
    if(!p1) { return(p2); }
    if(!p2) { return(p1); }
    if(p1 < p2) { return(p1); }
    else { return(p2); }
}


int mv_execute_moves(struct GIS *gis,char *movelist, char *checkmsgbuf, char *errmsgbuf, char *escmsgbuf)
{
    char *p,*q;
    char save_q;
    int whosturn = !mv_shoogip(gis); /* Black (= 0) starts in Shoogi, White (= 1) in Chess. */
    int finished = 0;


    for(p=movelist,q=strchr2(p,'/',','); (p != NULL); p=(q ? q+1 : NULL), q=(q ? strchr2(q+1,'/',',') : NULL))
     {
       if(q) { save_q = *q; *q = '\0'; }
       gis->g_latest_move[MAXBUF-1] = '\0';
       strncpy(gis->g_latest_move,p,MAXBUF-1);
       if(q) { *q = save_q; }
       gis->g_movesmade++;

       if(!(gis->g_movestat = mv_check_move(gis,gis->g_latest_move,whosturn,errmsgbuf)))
        {
          return(ERRMOVES);
	}
       finished = mv_find_checkings(gis,whosturn,checkmsgbuf,escmsgbuf);
       whosturn = !whosturn;
     }

    return(gis->g_movesmade);

}


int mv_find_checkings(struct GIS *gis,int color, char *checkmsgbuf, char *escmsgbuf)
{
    int finished = 0;
    COR vec[MAXTHREATS];
    *checkmsgbuf = '\0';
    *escmsgbuf = '\0';

    if(mv_checkedp(gis,vec,!color)) /* Opponent's king checked? */
     {
       if(mv_checkmatedp(gis,!color,escmsgbuf)) /* Mated? */
        {
          finished = 1;
          strcpy(checkmsgbuf,"king is mated by ");
        }
       else /* No, just checked. */
        {
          strcpy(checkmsgbuf,"king is checked by ");
        }
       mv_sprintcoords_and(gis,checkmsgbuf,vec);
     }
    return(finished);
}


/*
   This makes lot of logical checks to see whether move is valid.
   However, first the mv_parse_notation is called to see whether
   the move is syntactically valid. If move is erroneous then
   the appropriate error message is printed and zero is returned.
   Otherwise the move is made and non-zero is returned.
   Interactive is 1 if user can reply, 0 if loading moves from the
   file.
 */
int mv_check_move(struct GIS *gis,char *movestr,int color, char *errmsgbuf)
{
    PIECE piece,your_piece,destpiece;
    int movetype,promf,promtype;
    int src_x,src_y,dest_x,dest_y,capt_x,capt_y;
    int i;
    int do_it = NOTHING_SPECIAL;
    int long_castling = 0;
    COR vec[MAXTHREATS];
    char escmsgbuf[2049];

    err2log((errmsgbuf,"")); /* Clear this first. */
    *escmsgbuf = '\0';

    /* And these, just for sure: */
    piece = your_piece = destpiece = promf = promtype = 0;
    src_x = src_y = dest_x = dest_y = capt_x = capt_y = 0;
    gis->g_captured_piece = EMPTY;

    if(strequ(movestr,"RESIGN"))
     { movetype = 'R'; } /* As resign. */
    else if(cutstrequ(movestr,"HANDICAP:"))
     {
       int hh_result;

       if(!(hh_result = mv_handle_handicap(gis,(movestr+9),color,errmsgbuf)))
        { return(hh_result); }
       movetype = '@';
     }
    else if(!mv_shoogip(gis) && (strequ(movestr,"0-0") || (strequ(movestr,"0-0-0") && (long_castling = 1))))
     {
       if(mv_castling_possible(gis,color,errmsgbuf,long_castling))
        {
          mv_square(gis,mv_kings_x(gis,color),mv_kings_y(gis,color)) = EMPTY;
          mv_kings_x(gis,color) += ((long_castling ? -1 : 1)*2); /* y (rank) stays same */
          mv_square(gis,mv_kings_x(gis,color),mv_kings_y(gis,color)) = ofcolor(KING,color);
          mv_square(gis,mv_kings_x(gis,color)+(long_castling ? 1 : -1),mv_kings_y(gis,color))
             = mv_square(gis,(long_castling ? 1 : 8),mv_kings_y(gis,color)); /* Transfer the rook. */
          mv_square(gis,(long_castling ? 1 : 8),mv_kings_y(gis,color)) = EMPTY;
          mv_set_firstrank_bits(gis,color,(long_castling ? 1 : 8));
          mv_set_firstrank_bits(gis,color,mv_kings_x(gis,color));
          mv_set_prev_move(gis,ofcolor(KING,color),5,mv_kings_y(gis,color),mv_kings_x(gis,color),mv_kings_y(gis,color));
          return(1);
        }
       else
        {
          return(0); /* And errmsgbuf contains the reason. */
        }
     }
    else /* It's an ordinary move, drop, or capture, but not handicap. */
     {
       if(!(piece = mv_parse_notation(gis,movestr,&movetype,&promf,&promtype,
                                &src_x,&src_y,&dest_x,&dest_y,0,errmsgbuf)))
        { return(0); } /* If parse error. */
       your_piece = ofcolor(piece,color);
       destpiece  = mv_square(gis,dest_x,dest_y);
       capt_x = dest_x;
       capt_y = dest_y; /* Might be overwritten in mv_Chess_pawn_ (called
                           for example by mv_valid_movep (if the piece is pawn)
                           if this is "en passant" capture. */

/* Debugging: */
       if(_debug_flag_)
        {
          fout((OB,
"**Debugging in mv_check_move(gis,%s,%d): mv_square(gis,%d,%d)=%d, your_piece=%d\n",
                movestr,color,src_x,src_y,
	        mv_square(gis,src_x,src_y),your_piece));
          fout((OB,"piece=%d, destpiece=%d, dest_x=%d, dest_y=%d\n",
	            piece,destpiece,dest_x,dest_y));
          fout((OB,"movetype=%d,%c, promf=%d, promtype=%d,%c\n",
	            movetype,movetype,promf,promtype,promtype));
          fout((OB,"king: %d,%d\n",mv_kings_x(gis,color),mv_kings_y(gis,color)));
        }

       /* If destination square already occupied by player's own piece: */
       if((destpiece != EMPTY) && (getcolor(destpiece) == color))
        {
          err2log((errmsgbuf,
"?Can't %s to %s, as it's occupied by your %s!\n",
           ((movetype == '*') ? "drop" : "move"),mv_getcoord(gis,dest_x,dest_y),
	     mv_getpiecename(gis,destpiece)));
          return(0);
        }

/* We had here the test
       if((movetype == 'x') && (destpiece == EMPTY))
   but now it is moved after we know the final values of
   capt_x and capt_y, because if they are different from
   dest_y and dest_x, then it is "en passant" capture. */

       /* If trying to move to the square occupied by an enemy piece. */
       if((movetype == '-') && (destpiece != EMPTY))
        {
          err2log((errmsgbuf,
"?There's an enemy piece in the square %s. If you want to capture it\nuse x instead of - in your notation.\n",
               mv_getcoord(gis,dest_x,dest_y)));
          return(0);
        }

       if(movetype == '*') /* It's drop. */
        { /* If not that kind of piece in your hand. */
          if(!mv_check_piece_in_hand(gis,piece,gis->g_hands[color]))
           {
             err2log((errmsgbuf,
"?You have no %s in your hand!\n",mv_getpiecename(gis,your_piece)));
             return(0);
           }

          /* If the destination square is already occupied. */
          if(destpiece != EMPTY)
           {
             err2log((errmsgbuf,
"?There's an enemy piece in the square %s. Can't drop there.\n",
               mv_getcoord(gis,dest_x,dest_y)));
             return(0);
           }

          switch(mv_illegal_drop(gis,piece,dest_x,dest_y,color,escmsgbuf))
           {
	     case UNABLE_TO_MOVE:
	      {
                err2log((errmsgbuf,
"?Can't drop %s to %s, because it would never be able to move from there!\n",
                     mv_getpiecename(gis,your_piece),mv_getcoord(gis,dest_x,dest_y)));
                return(0);
	      }
             case DOUBLE_PAWN:
              {
	        err2log((errmsgbuf,
"?You already have an unpromoted pawn on the file %d !\n",dest_x));
                return(0);
	      }
	     case PAWN_DROP_MATE:
	      {
                err2log((errmsgbuf,"?You cannot mate with the pawn drop!\n"));
	        return(0);
	      }
	   }

        } /* End of "if it's drop" */

       else if(src_x) /* Source address specified. */
        { /* If there's no that kind of piece in that square. */
          if(mv_square(gis,src_x,src_y) != your_piece)
           {
	     err2log((errmsgbuf,
"?%s has no %s in the square %s !\n",
              getcapcolname(color),mv_getpiecename(gis,your_piece),mv_getcoord(gis,src_x,src_y)));
             return(0);
	   }


          if(!mv_valid_movep(gis,src_x,src_y,dest_x,dest_y,&capt_x,&capt_y,piece,movetype,color))
           {
             err2log((errmsgbuf,
"?The %s doesn't move that way!\n",mv_getpiecename(gis,your_piece)));
             return(0);
	   }
        }
       else /* the source address not specified. */
        {
          int count;

          count = (*mv_movefun(gis,color_off(piece)))(gis,vec,dest_x,dest_y,&capt_x,&capt_y,your_piece,movetype,getdir(color));

          if(!count) /* If can't find a piece which could move to dest. */
           {
             err2log((errmsgbuf,"?From where???\n"));
	     return(0);
           }

          /* If there's more than one pieces which could move here: */
          if(count > 1)
           {
             err2log((errmsgbuf,"?Specify which one: "));
	     mv_sprintcoords_or(gis,errmsgbuf,vec);
	     strcat(errmsgbuf,"\n");
	     return(0);
           }

/* There was exactly one piece which could move here,
   so get its coordinates: */
          src_x = get_x(*vec);
          src_y = get_y(*vec);

          /* Debugging check: */
          if(mv_square(gis,src_x,src_y) != your_piece)
           {
	     errlog((E1,
"**Internal error in mv_check_move(gis,%s,%d): mv_square(gis,%d,%d)=%d != your_piece=%d\n",
             movestr,color,src_x,src_y,
	     mv_square(gis,src_x,src_y),your_piece));
	     errlog((E1,"piece=%d, destpiece=%d, dest_x=%d, dest_y=%d\n",
	            piece,destpiece,dest_x,dest_y));
             errlog((E1,"movetype=%d, promf=%d, promtype=%d\n",
	            movetype,promf,promtype));
	     return(0);
           }
        } /* else  the source address not specified. */


       gis->g_captured_piece = mv_square(gis,capt_x,capt_y);

       /* If trying to capture to an empty square: */
       if(('x' == movetype) && (EMPTY == gis->g_captured_piece))
        {
          err2log((errmsgbuf,
"?There's nothing to capture in the square %s !\n",
               mv_getcoord(gis,capt_x,capt_y)));
          return(0);
        }

       /* Not a drop, nor a move of the promoted piece: */
       if((movetype != '*') && !promotedp(piece) /* !promf */)
        {
          /* If possible to promote the piece, but no = nor + specified: */
          if(mv_in_promzonep(gis,src_y,color) || mv_in_promzonep(gis,dest_y,color))
           {
             if(!promtype && mv_promotablep(gis,piece))
              {
                if(mv_shoogip(gis))
                 { err2log((errmsgbuf,"?You must specify whether to promote or not!\n")); }
                else
                 { err2log((errmsgbuf,"?You must specify to which officer to promote that pawn!\n")); }
                return(0);
	      }
           }
 /* Else the piece is not in the promotion zone and is not going there!
    and it's not drop, not promoted piece and there was = or + specified: */
          else if(promtype)
           {
             if(!mv_shoogip(gis) || ('+' == promtype))
              {
                err2log((errmsgbuf,
"?Can't promote outside of the promotion zone!\n"));
                return(0);
	      }
             else /* promtype must be = */
              {
                err2log((errmsgbuf,
"?Couldn't promote anyway, get that %c off from your notation!\n",
                  promtype));
                return(0);
	      }
           }
        } /* if((movetype != '*') && !promf) */

       /* If trying to move an unpromoted pawn, lance or knight
          to the last rank (or to second last in the case of knight): */
       if(mv_shoogip(gis) && (promtype != '+') && (movetype != '*') &&
           mv_unable_to_move(gis,piece,dest_y,color))
        {
          err2log((errmsgbuf,
"?Can't move %s to %s, because it would never be able to move from there!\n(You must promote it).\n",
             mv_getpiecename(gis,your_piece),mv_getcoord(gis,dest_x,dest_y)));
          return(0);
        }


       /* Now everything should be OK this far, let's try the move. */

       switch(movetype)
        {
          case '*': /* If a drop, then remove the piece from our own hand. */
           { 
             mv_remove_piece_from_hand(gis,piece,gis->g_hands[color]);
	     break;
           }
          case 'x':
           {
             /* Add the captured piece to player's own hand.
	        (But clear its color first and unpromote it!) */
             mv_add_piece_to_hand(gis,color_off(unpromote(gis->g_captured_piece)),gis->g_hands[color]);
             /* Fall through. */
           }
          case '-': /* If not a drop, then clear the source square. */
           {
             mv_square(gis,src_x,src_y) = EMPTY;
             if(mv_y_is_nth_rankp(gis,color,src_y,1)) { mv_set_firstrank_bits(gis,color,src_x); }
           }
        }

       if(mv_shoogip(gis))
        {
          mv_square(gis,dest_x,dest_y) =
            ((promtype == '+') ? promoted(your_piece) : your_piece);
        }
       else if(promtype)
        {
          char *p;

          if(NULL == (p = strchr(mv_piece_letters,promtype)))
           {
             err2log((errmsgbuf,
"?Cannot promote to unknown piece type '%c' !\n", promtype));
             return(0);
           }
          else
           {
             mv_square(gis,dest_x,dest_y) = ofcolor((p - mv_piece_letters),color);
	   }
        }
       else { mv_square(gis,dest_x,dest_y) = your_piece; }

       if((capt_x != dest_x) || (capt_y != dest_y)) { mv_square(gis,capt_x,capt_y) = EMPTY; }

       if(mv_y_is_nth_rankp(gis,!color,capt_y,1)) { mv_set_firstrank_bits(gis,!color,capt_x); }

       mv_set_prev_move(gis,your_piece,src_x,src_y,dest_x,dest_y);

       if(piece == KING)
        { /* If king moved, then set the new coordinates of him: */
          mv_kings_x(gis,color) = dest_x;
          mv_kings_y(gis,color) = dest_y;
        }

     } /* else  It's an ordinary move, drop, or capture, but not handicap. */

    if(mv_checkedp(gis,vec,color))
     {
          err2log((errmsgbuf,
"?Can't leave %s's %s in check! It is checked by ",
             getcapcolname(color),mv_getpiecename(gis,KING)));
          mv_sprintcoords_and(gis,(errmsgbuf+strlen(errmsgbuf)),vec);
          return(0);
     }

    else { return(1); } /* Success. */


}


int mv_illegal_drop(struct GIS *gis,int piece,int dest_x,int dest_y,int color,char *escmsgbuf)
{
    int result;

    if(mv_unable_to_move(gis,piece,dest_y,color)) { return(UNABLE_TO_MOVE); }

    /* If it's able to move and is not pawn, then it's ok. */
    if(piece != PAWN) { return(0); }

    /* If pawn in the same file. */
    if(mv_samepiece_in_file(gis,dest_x,ofcolor(piece,color))) { return(DOUBLE_PAWN); }

    /* If pawn dropped just front of the opponent's king: */
    if(mv_square(gis,dest_x,dest_y+getdir(!color)) == ofcolor(KING,!color))
     {
/* Put pawn tentatively to its destination so that we see whether it
   checkmates opponent's king: */
       mv_square(gis,dest_x,dest_y) = ofcolor(PAWN,color);
       if(mv_checkmatedp(gis,!color,escmsgbuf)) { result=0; }
       else { result=1; }
       mv_square(gis,dest_x,dest_y) = EMPTY; /* Restore the situation. */
       if(!result) { return(PAWN_DROP_MATE); }
     }

    return(0);
}


/* Returns nonzero if the piece is pawn, lance or knight and y is end rank,
    or if the piece is knight and y is the second last rank.
 */
int mv_unable_to_move(struct GIS *gis,int piece,int y,int color)
{
    return(
           ((y == (color ? mv_bsize(gis) : 1)) && (piece >= PAWN) && (piece <= KNIGHT))
             ||
           ((piece == KNIGHT) && (y == (color ? (mv_bsize(gis)-1) : 2)))
	  );
}

/* Check whether y coordinate is in the promotion zone.
   In Shoogi, ranks a-c (1-3) for black (0) and g-i (7-9) for white (1).
   in Chess, rank 1 for black, and rank 8 for white.
 */
int mv_in_promzonep(struct GIS *gis,int y,int color)
{
    if(mv_shoogip(gis)) { return(color ? (y > (mv_bsize(gis)-3)) : (y <= 3)); }
    else { return(color ? (y > (mv_bsize(gis)-1)) : (y <= 1)); }
}

char *mv_getcoord(struct GIS *gis,int x,int y)
{
    static char coordbuf[3];

    *coordbuf   = (mv_shoogip(gis) ? '0' : ('a'-1)) + x;
    coordbuf[1] = (mv_shoogip(gis) ? ('a'-1) : '0') + y;
    coordbuf[2] = '\0'; /* Ending zero. */
    return(coordbuf);
}

char *mv_getcoord_b(struct GIS *gis,char *coordbuf,int x,int y)
{
    *coordbuf   = (mv_shoogip(gis) ? '0' : ('a'-1)) + x;
    coordbuf[1] = (mv_shoogip(gis) ? ('a'-1) : '0') + y;
    coordbuf[2] = '\0'; /* Ending zero. */
    return(coordbuf);
}

/* Concatenate the coordinates found from the vec to dstbuf: */
int mv_sprintcoords_and(struct GIS *gis,char *dstbuf,COR *vec)
{
    while(*vec)
     {
       strcat(dstbuf,mv_getcoord(gis,get_x(*vec),get_y(*vec)));
       vec++;
       strcat(dstbuf,(*vec ? (*(vec+1) ? ", " : " and ") : ""));
     }
}

/* Concatenate the coordinates found from the vec to dstbuf: */
int mv_sprintcoords_or(struct GIS *gis,char *dstbuf,COR *vec)
{
    while(*vec)
     {
       strcat(dstbuf,mv_getcoord(gis,get_x(*vec),get_y(*vec)));
       vec++;
       strcat(dstbuf,(*vec ? (*(vec+1) ? ", " : " or ") : ""));
     }
}


/*
   Structure of the hand is as follows:
   First byte (index zero) is total count of pieces in hand, and
   in positions 2-9 there is counts for pieces from bishop to
   king, respectively. Position 1 is unused.
   Some of these functions could be macros also, but due to
   historical reasons they are not. (Means that I'm too lazy...)
 */

int mv_inithand(struct GIS *gis,PIECE *hand)
{
    int i=0;

    /* Clear hand: */
    for(i=0; i <= MAX_IN_HAND; ) { *(hand+i++) = 0; }
}


int mv_add_piece_to_hand(struct GIS *gis,int piece,PIECE *hand)
{
    hand[piece]++; /* Increase the count of that piece. */
    ++*hand;       /* And increase the total count also. */
}

int mv_remove_piece_from_hand(struct GIS *gis,int piece,PIECE *hand)
{
    hand[piece]--; /* Decrease the count of that piece. */
    --*hand;       /* And decrease the total count also. */
}

int mv_check_piece_in_hand(struct GIS *gis,int piece,PIECE *hand) /* Could be macro also. */
{
    /* Return the count of those pieces in this hand: */
    return(u_piecep(piece) ? hand[piece] : 0);
}


int mv_samepiece_in_file(struct GIS *gis,int x,int piece)
{
    register int y;

    for(y=1; y < (mv_bsize(gis)+1); y++)
     { if(mv_square(gis,x,y) == piece) { return(y); } }
    return(0);
}


int mv_initmovefuns(struct GIS *gis)
{
   int i;

   /* First initialize everything to mv_bugmove_ */
   for(i=0; i < MAXFUNS; i++)
    { gis->g_movefuns[i] = (PFI) mv_bugmove_; }

   gis->g_movefuns[BISHOP]           = (PFI) mv_bishop_;       /*  2 */
   gis->g_movefuns[ROOK]             = (PFI) mv_rook_;         /*  3 */
   gis->g_movefuns[PAWN]             = (PFI) (mv_shoogip(gis) ? mv_Shoogi_pawn_   : mv_Chess_pawn_);   /*  4 */
   gis->g_movefuns[LANCE]            = (PFI) mv_lance_;        /*  5 */
   gis->g_movefuns[KNIGHT]           = (PFI) (mv_shoogip(gis) ? mv_Shoogi_knight_ : mv_Chess_knight_); /*  6 */
   gis->g_movefuns[SILVER]           = (PFI) mv_silver_;       /*  7 */
   gis->g_movefuns[GOLD]             = (PFI) mv_gold_;         /*  8 */
   gis->g_movefuns[KING]             = (PFI) (mv_shoogip(gis) ? mv_Shoogi_king_ : mv_Chess_king_);     /*  9 */
   gis->g_movefuns[QUEEN]            = (PFI) mv_queen_;        /* 10 */
   gis->g_movefuns[promoted(BISHOP)] = (PFI) mv_prom_bishop_;  /* 18 */
   gis->g_movefuns[promoted(ROOK)]   = (PFI) mv_prom_rook_;    /* 19 */
   gis->g_movefuns[promoted(PAWN)]   = (PFI) mv_gold_;         /* 20 Promoted pawn.    */
   gis->g_movefuns[promoted(LANCE)]  = (PFI) mv_gold_;         /* 21 Promoted lance.   */
   gis->g_movefuns[promoted(KNIGHT)] = (PFI) mv_gold_;         /* 22 Promoted knight.  */
   gis->g_movefuns[promoted(SILVER)] = (PFI) mv_gold_;         /* 23 Promoted silver.  */
}


int mv_initboard(struct GIS *gis)
{
    strcpy(gis->g_latest_move,"NONE");
    gis->g_movestat = gis->g_movesmade = 0;
    if(mv_shoogip(gis)) { mv_Shoogi_init_board(gis); } else { mv_Chess_init_board(gis); }
}


int mv_Chess_init_board(struct GIS *gis)
{
    register int x,y;

    gis->g_b_impasse_points = gis->g_w_impasse_points = DEFAULT_IMPASSE_POINTS;
    gis->g_captured_piece = EMPTY;

    mv_inithand(gis,gis->g_blacks_hand);
    mv_inithand(gis,gis->g_whites_hand);

    /* Initialize kings' locations: */
    *gis->g_blacks_king = *gis->g_whites_king = 5; /* Both are in file 'e' (= the 5th file) initially. */
    gis->g_blacks_king[1] = mv_bsize(gis); /* And Y coordinates... */
    gis->g_whites_king[1] = 1;    /*  ...too. */

    /* The upper edge. */
    for(y=-1; y < 1; y++)
     {
       for(x=-1; x < (mv_bsize(gis)+3); x++) { mv_square(gis,x,y) = EDGE; }
     }

    /* The board itself. */
    for(; y < (mv_bsize(gis)+1); y++) /* From a to i */
     {
       mv_square(gis,-1,y)        = EDGE;  /* Initialize the */
       mv_square(gis,0,y)         = EDGE; /*   right edge.  */
       for(x=1; x < (mv_bsize(gis)+1); x++) { mv_square(gis,x,y) = EMPTY; }
       mv_square(gis,(mv_bsize(gis)+1),y) = EDGE;  /* Initialize the */
       mv_square(gis,(mv_bsize(gis)+2),y) = EDGE; /*   left edge.   */
     }

    /* The lower edge. */
    for(; y < (mv_bsize(gis)+3); y++)
     {
       for(x=-1; x < (mv_bsize(gis)+3); x++) { mv_square(gis,x,y) = EDGE; }
     }

    /* Now the board is empty. Then put the pieces: */

    /* White side. */
    for(y=1,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makewhite(mv_Chess_firstrank[x-1]); }
    for(y=2,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makewhite(PAWN); }

    /* Black side. */
    for(y=7,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makeblack(PAWN); }
    for(y=8,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makeblack(mv_Chess_firstrank[x-1]); }

}


int mv_Shoogi_init_board(struct GIS *gis)
{
    register int x,y;

    gis->g_b_impasse_points = gis->g_w_impasse_points = DEFAULT_IMPASSE_POINTS;
    gis->g_captured_piece = EMPTY;

    mv_inithand(gis,gis->g_blacks_hand);
    mv_inithand(gis,gis->g_whites_hand);

    /* Initialize kings' locations: */
    *gis->g_blacks_king = *gis->g_whites_king = 5; /* Both are in file 5 initially. */
    gis->g_blacks_king[1] = mv_bsize(gis); /* And Y coordinates... */
    gis->g_whites_king[1] = 1;    /*  ...too. */

    /* The upper edge. */
    for(y=-1; y < 1; y++)
     {
       for(x=-1; x < (mv_bsize(gis)+3); x++) { mv_square(gis,x,y) = EDGE; }
     }

    /* The board itself. */
    for(; y < (mv_bsize(gis)+1); y++) /* From a to i */
     {
       mv_square(gis,-1,y)        = EDGE;  /* Initialize the */
       mv_square(gis,0,y)         = EDGE; /*   right edge.  */
       for(x=1; x < (mv_bsize(gis)+1); x++) { mv_square(gis,x,y) = EMPTY; }
       mv_square(gis,(mv_bsize(gis)+1),y) = EDGE;  /* Initialize the */
       mv_square(gis,(mv_bsize(gis)+2),y) = EDGE; /*   left edge.   */
     }

    /* The lower edge. */
    for(; y < (mv_bsize(gis)+3); y++)
     {
       for(x=-1; x < (mv_bsize(gis)+3); x++) { mv_square(gis,x,y) = EDGE; }
     }

    /* Now the board is empty. Then put the pieces: */

    /* White side. */
    for(y=1,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makewhite(mv_Shoogi_firstrank[x-1]); }
    mv_square(gis,8,2) = makewhite(ROOK);
    mv_square(gis,2,2) = makewhite(BISHOP);
    for(y=3,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makewhite(PAWN); }

    /* Black side. */
    for(y=7,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makeblack(PAWN); }
    mv_square(gis,8,8) = makeblack(BISHOP);
    mv_square(gis,2,8) = makeblack(ROOK);
    for(y=9,x=1; x < (mv_bsize(gis)+1); x++)
     { mv_square(gis,x,y) = makeblack(mv_Shoogi_firstrank[x-1]); }

}

/* Get count of pieces on board and hands.
   b1count will contain the count of black's pieces,
   and b2count the weighted count of ----- " -----.
   w1count and w2count for white's pieces, similarly.
 */
int mv_countpieces(struct GIS *gis,int *b1count,int *w1count,int *b2count,int *w2count)
{
    *b2count = *w2count = 0;
    *b1count = mv_count_hand(gis,gis->g_blacks_hand,b2count);
    *w1count = mv_count_hand(gis,gis->g_whites_hand,w2count);
    mv_count_pieces_on_board(gis,b1count,w1count,b2count,w2count);
}


int mv_count_pieces_on_board(struct GIS *gis,int *b1count,int *w1count,int *b2count,int *w2count)
{
    register int x,y;
    int piece;
    
    for(y=1; y < (mv_bsize(gis)+1); y++) /* From a to i */
     {
       for(x=1; x < (mv_bsize(gis)+1); x++) /* From 1 to 9 */
        {
	  if((piece = mv_square(gis,x,y)) != EMPTY)
	   {
	     if(whitep(piece))
	      {
	        ++*w1count;
		(*w2count) += value_of_piece(gis,color_off(piece));
	      }
	     else
	      {
	        ++*b1count;
		(*b2count) += value_of_piece(gis,color_off(piece));
	      }
	   }
	}
     }
}


/* Returns as result the count of pieces in hand, and in count2 the
    weighted count: */
int mv_count_hand(struct GIS *gis,PIECE *hand,int *count2)
{
    int i;

    for(i=2; i <= MAX_IN_HAND; i++)
     { (*count2) += (*(hand+i) * value_of_piece(gis,i)); }

    return(*hand);
}


 
int mv_printboard(struct GIS *gis,int whichway,FILE *fp,ULI flags)
{
    char *mv_create_interline();
    register int xx,x,yy,y;
    ULI save_flags;
    int piece;
    int b1count,w1count,b2count,w2count;
    unsigned char blackcode,whitecode,threatcode;
    char c;
    register char *op;
    char obuf[MAXBUF+3],interline[MAXBUF+3];

    save_flags = mv_flags(gis);
    mv_flags(gis) = flags;

    blackcode  = getblackcode(mv_flags(gis));
    whitecode  = getwhitecode(mv_flags(gis));
    threatcode = getthreatenedcode(mv_flags(gis));

    mv_countpieces(gis,&b1count,&w1count,&b2count,&w2count);

    sprintf(obuf,"%s - %s %2d %2d ",
      getcapcolname(!whichway),gis->g_names[!whichway],
      (whichway ? b1count : w1count),(whichway ? b2count : w2count));
    mv_printhand(gis,gis->g_hands[!whichway],!whichway,obuf);
    output_stuff(obuf,fp);

    if(getboardstyle(mv_flags(gis)) & (HTML_BOARD))
     { output_stuff("<TABLE BORDER>", fp); }

    mv_print_file_labels(gis,whichway,obuf);
    output_stuff(obuf,fp);

    mv_create_interline(gis,interline);

    for(yy=1; yy < (mv_bsize(gis)+1); yy++)
     {
       y = (whichway ? (mv_bsize(gis)+1 - yy) : yy);

/*     if(yy != 1) */
        {
          if(!(getboardstyle(mv_flags(gis)) & HTML_BOARD))
           { output_stuff(interline,fp); }
	  op = obuf;
          if(yy == 6) /* Print move information after centre rank. */
           {
             if(gis->g_captured_piece != EMPTY)
	      {
	        sprintf(obuf,
		  "     (Captured %s)",mv_getpiecename(gis,gis->g_captured_piece));
		op += strlen(op);
	      }
	   }
	  *op++ = '\n';
	  *op = '\0';
	  output_stuff(obuf,fp);
	}

       op = obuf;

       if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
        {
          strcpy(op,"<TR><TH>");
          op += strlen(op);
        }
       else
        {
        }

       if(mv_shoogip(gis)) { *op++ = (('a'-1)+y); } /* Rank letter, from a to i */
       else { *op++ = ('0'+y); }
       *op++ = ' ';

       for(xx = mv_bsize(gis); xx ; xx--)
        {
	  COR vec[MAXTHREATS];
          COR avail_squares[MAXFREEDOMS];
          int freedoms;
	  unsigned char fancy_flag;

          x = (whichway ? (mv_bsize(gis)+1 - xx) : xx);

	  piece = mv_square(gis,x,y);

          fancy_flag=0;
          if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
           {
             sprintf(op,"</TH><TH>%s<INPUT TYPE=\"button\" VALUE=\"",
                        (whitep(piece) ? "<FONT COLOR=\"red\">" : ""));

             op += strlen(op);
           }
          else { *op++ = ':'; }


	  if(piece != EMPTY)
	   {
	     if(blackcode && blackp(piece))
	      { *op++ = getcodeletter(blackcode); fancy_flag=1; }
	     if(whitecode && whitep(piece))
	      { *op++ = getcodeletter(whitecode); fancy_flag=1; }
             if(threatcode &&
	         mv_find_threatening_pieces(gis,vec,x,y,!getcolor(piece),0))
	      { *op++ = getcodeletter(threatcode); fancy_flag=1; }
	   }

          mv_getpieceshort(gis,op,piece);
          op += strlen(op);

          if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
           {
             int i;
             *op++ = '"';
             freedoms = mv_find_available_squares(gis,avail_squares, x, y, piece, get_parity(gis->g_movesmade + !mv_shoogip(gis)));

             strcpy(op," onClick=\"domove('"); op += strlen(op);
             mv_getpiececode(gis,op, piece); op += strlen(op);
             strcpy(op, "','"); op += strlen(op);
             strcpy(op, mv_getcoord(gis,x,y)); op += strlen(op);
             sprintf(op, "',%u,'", freedoms);
             op += strlen(op);
             for(i=0; i < freedoms; i++)
              {
                strcpy(op,
                   mv_getcoord(gis,get_x(avail_squares[i]),get_y(avail_squares[i])));;
                op += strlen(op);
                if(i < (freedoms-1)) { *op++ = ','; }
              }
             strcpy(op,"')\">");
             op += strlen(op);
             if(whitep(piece)) { strcpy(op,"</FONT>"); op += strlen(op); }

           }

	  if(fancy_flag)
	   { *op++ = getcodeletter(NORMAL); }
	} /* for loop over x (one rank or row) */

       if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
        {
          strcpy(op,"</TH><TH>");
          op += strlen(op);
        }
       else
        {
          *op++ = ':';
          *op++ = ' ';
        }

       if(mv_shoogip(gis))
        {
          if(mv_shoogip(gis)) { sprintf(op,"&#%u;", mv_Unumbers[y]); }
          else { sprintf(op,"%c",('0' + y)); }
          op += strlen(op);
        }
       else
        {
          *op++ = ('0'+y);  /* Same rank letter as on the left edge for Chess */
          *op++ = ' ';
        }

       if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
        {
          strcpy(op,"</TH></TR>");
          op += strlen(op);
        }

#ifdef THIS_PART_COMMENTED_OUT
       if(yy == 5) /* Print move information after centre rank. */
        {
	  if(getboardstyle(mv_flags(gis)) == ASCII) { *op++ = ' '; } /* Minor fix. */
	  if(!gis->g_movesmade) { strcpy(op,"  No moves made.");  }
          else /* Watch out for extra-long moves, like handicaps! */
	   {  /* (They could spoil the output.) */
             sprintf(op,"  %s moved %s (%lu)",
                 getcapcolname(!get_parity(gis->g_movesmade + !mv_shoogip(gis))),
                 gis->g_latest_move,gis->g_movesmade);
           }
	  op += strlen(op);
	}
#endif

       /*      if(getboardstyle(mv_flags(gis)) & HTML_BOARD) { strcpy(op,"</TD></TR>"); op += strlen(op); } */

       *op++ = '\n';
       *op = '\0';
       output_stuff(obuf,fp);
     }

    if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
     {
     }
    else
     {
       output_stuff(interline,fp);
       output_stuff("\n",fp);
     }

    mv_print_file_labels(gis,whichway,obuf);
    output_stuff(obuf,fp);

    if(getboardstyle(mv_flags(gis)) & HTML_BOARD)
     {
       output_stuff("</TABLE>\n",fp);
     }

    sprintf(obuf,"%s - %s %2d %2d ",
      getcapcolname(whichway),gis->g_names[whichway],
      (whichway ? w1count : b1count),(whichway ? w2count : b2count));
    mv_printhand(gis,gis->g_hands[whichway],whichway,obuf);
    output_stuff(obuf,fp);

    mv_flags(gis) = save_flags; /* restore mv_flags(gis) */
}


int mv_printhand(struct GIS *gis,PIECE *hand,int color,char *obuf)
{
    if(!*hand) /* If total count zero. */
     {
       if(mv_shoogip(gis)) { strcat(obuf,"No pieces in hand.\n"); } else { strcat(obuf,"\n"); }
     }
    else
     {
       if(mv_shoogip(gis)) { strcat(obuf,"Pieces in hand:"); } else { strcat(obuf,"Pieces captured from opponent:"); }
       mv_print_hand(gis,hand,color,obuf);
       strcat(obuf,"\n");
     }
}

int mv_print_hand(struct GIS *gis,PIECE *hand,int color,char *obuf)
{
    register int i,c,count;
    char tmpbuf[121];

    obuf += strlen(obuf); /* Find the end of obuf first. */

    for(i=2; i <= MAX_IN_HAND; i++)
     {
       if((count = hand[i])) /* If there is one or more pieces of index i. */
        {
	  *obuf++ = ' '; /* Separating blank. */
	  mv_getpieceshort(gis,obuf,ofcolor((PIECE)i,((!mv_shoogip(gis))^color)));
	  /* If blank + letter, then delete blank: */
	  if(*obuf == ' ') { strcpy(obuf,(obuf+1)); }
          obuf += strlen(obuf);
	  /* Follow the letter with count, if there's more than one: */
	  if(count > 1)
	   {
	     if(getboardstyle(mv_flags(gis)) != ASCII)
	      {
	        mv_getkanjinum(gis,tmpbuf,count);
                strcpy(obuf,tmpbuf);
	      }
	     else { sprintf(obuf,"%u",count); }
             obuf += strlen(obuf); /* Find the end of obuf again. */
           }
	}
     }
    *obuf = '\0'; /* Terminate the obuf with the ending zero. */
}


char *mv_create_interline(struct GIS *gis,char *op)
{
    register int x;

    strcpy(op,"  "); op += 2;
    for(x=1; x < (mv_bsize(gis)+1); x++)
     {
       *op++ = '+';

	{
	  *op++ = '-';
	  *op++ = '-';
        }
     }

    *op++ = '+';
    *op = '\0';
    return(op);
}


int mv_print_file_labels(struct GIS *gis,int whichway,char *obuf)
{
    register int i;
    char tmpbuf[3];

    if(getboardstyle(mv_flags(gis)) & (HTML_BOARD))
     { strcpy(obuf,"<TR><TH></TH>"); }
    else { strcpy(obuf,"  "); }
    obuf += strlen(obuf);

    for(i=mv_bsize(gis); i ;)
     {
       *obuf++ = ' ';
       if(getboardstyle(mv_flags(gis)) == ASCII)
        {
          *obuf++ = ' ';
          /* I hope that mv_bsize(gis) is never greater than 9, otherwise the results
              are messy: */
          *obuf++ = ('0' + (whichway ? (mv_bsize(gis)+1-i--) : i--));
	}
       else
        {
          if(mv_shoogip(gis))
           {
             sprintf(obuf,"<TH>&#%u;</TH>", (0xFF10 + (whichway ? (mv_bsize(gis)+1-i--) : i--)));
           }
          else { sprintf(obuf,"<TH>%c</TH>", (('a'-1) + (whichway ? (mv_bsize(gis)+1-i--) : i--))); }
          obuf += strlen(obuf);
        }
     }

    if(getboardstyle(mv_flags(gis)) & (HTML_BOARD))
     { strcpy(obuf,"<TH></TH></TR>"); obuf += strlen(obuf); }

    *obuf++ = '\n';
    *obuf = '\0';
}


