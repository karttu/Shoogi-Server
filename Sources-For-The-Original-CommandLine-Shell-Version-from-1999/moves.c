
#include "shoogi.h"

unsigned char board[BSIZE+4][BSIZE+4];

char *letters = " ?BRPLNSGK01234567HDTLNSGk";

/* 
Values assigned for each piece when counting up the game, if impasse occurs:
(rooks and bishops count 5 points each and all other pieces (except kings
which are ignored) count 1 point. Promotions are disregarded.)
  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
 */
int piecevalues[] =
 {
  0, 0, 5, 5, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 1, 1, 1, 1, 0, 0
 };


/* This is the true king. The one in array is 'jewel', i.e. like kanji
    for king but one surplus dot: */
struct piecename true_king =
   { "king",              "oushou",       "ou",       "2&>-", "2&",                 /* &#29579;&#23558; */ 0x738B, 0x5C06, 0x738B };

struct piecename piecenames[] =
 {
   { "<EMPTY>",           "<EMPTY>",      "<EMPTY>",  "#E#M#P#T#Y", "!!",           /* &#12288;&#12288; */ 0x3000, 0x3000, 0x3000 },
   { "<EDGE>",            "<EDGE>",       "<EDGE>",   "#E#D#G#E", "!)",             /* &#65311; */         0xFF1F, 0xFF1F, 0xFF1F }, /*?*/
   { "bishop",            "kakugyou",     "kaku",     "3Q9T", "3Q",                 /* &#35282;&#34892; */ 0x89D2, 0x884C, 0x89D2 },
   { "rook",              "hisha",        "hisha",    "Ht<V", "Ht",                 /* &#39131;&#36554; */ 0x98DB, 0x8ECA, 0x98DB },
   { "pawn",              "fuhyou",       "fu",       "JbJ<", "Jb",                 /* &#27497;&#20853; */ 0x6B69, 0x5175, 0x6B69 },
   { "lance",             "kyousha",      "kyou",     "9a<V", "9a",                 /* &#39321;&#36554; */ 0x9999, 0x8ECA, 0x9999 },
   { "knight",            "keima",        "kei",      "7KGO", "7K",                 /* &#26690;&#39340; */ 0x6842, 0x99AC, 0x6842 },
   { "silver",            "ginshou",      "gin",      "6d>-", "6d",                 /* &#37504;&#23558; */ 0x9280, 0x5C06, 0x9280 },
   { "gold",              "kinshou",      "kin",      "6b>-", "6b",                 /* &#37329;&#23558; */ 0x91D1, 0x5C06, 0x91D1 },
   { "king",              "gyokushou",    "gyoku",    "6L>-", "6L",                 /* &#29577;&#23558; */ 0x7389, 0x5C06, 0x7389 }, /* jewel */
/* Here JIS codes for 10 - 17 inside circles are used: (these are needed
   only if a bug occurs...) In Unicode: Use full-width digits 0-7. */
   { "<10>",              "<10>",         "<10>",     "-*",   "-*",                 /* &#65296; */         0xFF10, 0xFF10, 0xFF10 },
   { "<11>",              "<11>",         "<11>",     "-+",   "-+",                 /* &#65297; */         0xFF11, 0xFF11, 0xFF11 },
   { "<12>",              "<12>",         "<12>",     "-,",   "-,",                 /* &#65298; */         0xFF12, 0xFF12, 0xFF12 },
   { "<13>",              "<13>",         "<13>",     "--",   "--",                 /* &#65299; */         0xFF13, 0xFF13, 0xFF13 },
   { "<14>",              "<14>",         "<14>",     "-.",   "-.",                 /* &#65300; */         0xFF14, 0xFF14, 0xFF14 },
   { "<15>",              "<15>",         "<15>",     "-/",   "-/",                 /* &#65301; */         0xFF15, 0xFF15, 0xFF15 },
   { "<16>",              "<16>",         "<16>",     "-0",   "-0",                 /* &#65302; */         0xFF16, 0xFF16, 0xFF16 },
   { "<17>",              "<17>",         "<17>",     "-1",   "-1",                 /* &#65303; */         0xFF17, 0xFF17, 0xFF17 },
   { "promoted bishop",   "ryuuma",       "uma",      "N5GO", "GO",                 /* &#31452;&#39340; */ 0x7ADC, 0x99AC, 0x99AC },
   { "promoted rook",     "ryuuou",       "ryuu",     "N52&", "N5",                 /* &#31452;&#29579; */ 0x7ADC, 0x738B, 0x7ADC },
   { "promoted pawn",     "tokin",        "tokin",    "$H6b", "$H",                 /* &#12392;&#37329; */ 0x3068, 0x91D1, 0x3068 },
   { "promoted lance",    "narikyou",     "narikyou", "@.9a", "6b",                 /* &#25104;&#39321; */ 0x6210, 0x9999, 0x91D1 },
   { "promoted knight",   "narikei",      "narikei",  "@.7K", "6b",                 /* &#25104;&#26690; */ 0x6210, 0x6842, 0x91D1 },
   { "promoted silver",   "narigin",      "narigin",  "@.6d", "6b",                 /* &#25104;&#37504; */ 0x6210, 0x9280, 0x91D1 },
   { "<promoted gold>",   "<narikin>",    "<narikin>","@.6b", "6b",                 /* &#25104;&#37329; */ 0x6210, 0x91D1, 0x91D1 },
   { "<promoted king>",   "<narigyoku>",  "<narigyoku>", "@.6L", "6L",              /* &#25104;&#29577; */ 0x6210, 0x7389, 0x7389 }
 };


/* Kanji numbers from 0 to 10 in JIS: */
char *Jnumbers[11] =
 { "Nm", "0l", "Fs", ";0", ";M", "8^", "O;", "<7", "H,", "6e", "==" };

unsigned short int Unumbers[11] =
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


PIECES firstrank[9] =
 { LANCE, KNIGHT, SILVER, GOLD, KING, GOLD, SILVER, KNIGHT, LANCE };


/* Track of the location of the kings are kept in these vectors.
   X in first element, Y in second: */
int blacks_king[2];
int whites_king[2];
int *kings[2] = { blacks_king, whites_king };

PIECES blacks_hand[MAX_IN_HAND];
PIECES whites_hand[MAX_IN_HAND];

PIECES *hands[2] = { blacks_hand, whites_hand };

/* An array of pointers to functions returning int: */
/* int (*movefuns[])() = */
PFI movefuns[MAXFUNS];

int knight_moves[] =
 {
    1,  2,
   -1,  2,
   0,0
 };

int silver_moves[] =
 {
    0,  1, /* N  */
   -1,  1, /* NE */
   -1, -1, /* SE */
    1, -1, /* SW */
    1,  1, /* NW */
   0,0
 };

int gold_moves[] =
 {
    0,  1, /* N  */
   -1,  1, /* NE */
   -1,  0, /* E  */
    0, -1, /* S  */
    1,  0, /* W  */
    1,  1, /* NW */
   0,0
 };

int king_moves[] =
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
int xbishop_moves[] =
 {
    0,  1, /* N  */
   -1,  0, /* E  */
    0, -1, /* S  */
    1,  0, /* W  */
   0,0
 };

/* Extra moves for the promoted rook: */
int xrook_moves[] =
 {
   -1,  1, /* NE */
   -1, -1, /* SE */
    1, -1, /* SW */
    1,  1, /* NW */
   0,0
 };


char *getpiececode(dst,piece)
UBYTE *dst;
int piece;
{
    int c;
    UBYTE *org_dst = dst;

    c = letters[unpromote(color_off(piece))];
    if(promotedp(piece)) { *dst++ = '+'; }
    *dst++ = (whitep(piece) ? tolower(c) : c);
    *dst = '\0';

    return((char *) org_dst);
}

int checkshortmoves(vec,x,y,thispiece,dir,moves)
COR *vec;
register int x,y;
int dir,thispiece;
register int *moves;
{
    int count=0;
    int tent_piece;
    register int xx,yy;

    while(((xx = *moves++),(yy = *moves++),(xx || yy)))
     {
       tent_piece = square((x+xx),(y+(dir*yy)));
       if(((OPPONENT_OR_EMPTY == color_off(thispiece))
             && other_square_empty_or_enemys(thispiece,tent_piece))
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
int checklongmoves(vec,x,y,thispiece,xx,yy)
COR *vec;
register int x,y,xx,yy;
int thispiece;
{
    int count = 0;
    int tent_piece;

    do
     {
       x += xx;
       y += yy;
       tent_piece = square(x,y);
       if((OPPONENT_OR_EMPTY == color_off(thispiece))
             && other_square_empty_or_enemys(thispiece,tent_piece))
        {
          *vec++ = makecoord(x,y); count++;
        }
     } while(tent_piece == EMPTY);

/* Should never match if we were called with thispiece as OPPONENT_OR_EMPTY */
    if(square(x,y) == thispiece)
     { *vec = makecoord(x,y); return(1); }

    return(count);
}


int pawn_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int tent_piece = square(x,(y+dir));

    if(((OPPONENT_OR_EMPTY == color_off(thispiece))
          && other_square_empty_or_enemys(thispiece,tent_piece))
       || (tent_piece == thispiece))
     { *vec = makecoord(x,(y+dir)); vec[1] = 0; return(1); }
    *vec = 0;
    return(0);
}


int lance_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int count;

    count = checklongmoves(vec,x,y,thispiece,0,dir);
    vec[count] = 0;
    return(count);
}


int knight_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    return(checkshortmoves(vec,x,y,thispiece,dir,knight_moves));
}


int silver_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    return(checkshortmoves(vec,x,y,thispiece,dir,silver_moves));
}

int gold_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    return(checkshortmoves(vec,x,y,thispiece,dir,gold_moves));
}

int king_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    return(checkshortmoves(vec,x,y,thispiece,dir,king_moves));
}

int bishop_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int count;

    count =  checklongmoves(vec        ,x,y,thispiece,-1,1);
    count += checklongmoves((vec+count),x,y,thispiece,-1,-1);
    count += checklongmoves((vec+count),x,y,thispiece, 1,-1);
    count += checklongmoves((vec+count),x,y,thispiece, 1, 1);
    vec[count] = 0; /* End marker. */
    return(count);
}

int rook_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int count;

    count  = checklongmoves(vec        ,x,y,thispiece,0, 1);
    count += checklongmoves((vec+count),x,y,thispiece,-1,0);
    count += checklongmoves((vec+count),x,y,thispiece,0,-1);
    count += checklongmoves((vec+count),x,y,thispiece, 1,0);
    vec[count] = 0; /* End marker. */
    return(count);
}

int prom_bishop_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int count;
/* First check extra moves got by promotion, then the ordinary bishop moves: */
    count = checkshortmoves(vec,x,y,thispiece,dir,xbishop_moves);
/* Count & Bishop, good pals, eh? */
    return(count + bishop_((vec+count),x,y,thispiece,dir));
}


int prom_rook_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    int count;
/* First check extra moves got by promotion, then the ordinary rook moves: */
    count = checkshortmoves(vec,x,y,thispiece,dir,xrook_moves);
    return(count + rook_((vec+count),x,y,thispiece,dir));
}


int bugmove_(vec,x,y,thispiece,dir)
COR *vec;
register int x,y;
int thispiece,dir;
{
    errlog((E1,"**Fatal internal error: bugmove_(vec,%d,%d,%d,%d) called!\n",
             x,y,thispiece,dir));
    return(0);
}


int checkedp(vec,color)
COR *vec;
int color;
{
    register int x;

    /* If king is not on the board, then it can't be checked! */
    if(!(x = *(kings[color]))) { return(0); }
    return(find_threatening_pieces(vec,x,*(kings[color]+1),!color));
}


/* Not so easy as you may think...

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
   Hmm. voisiko matemaattisesti todistaa ettei voi olla sellaista
   tilannetta miss{ "mate with the pawn drop" olisi ainoa keino
   est{{ matti? T{ss{kin voi tuon l{hetin ensin sy|d{ tornilla.

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

int checkmatedp(color)
int color;
{
    register int *moves;
    register int xx,yy;
    int x,y;
    int off_x,off_y,off_piece,neighbor;
    int adjacent=0;
    int count;
    COR vec[MAXTHREATS],vec2[MAXTHREATS];

    x = *(kings[color]);
    y = *((kings[color])+1);

/*  count = find_threatening_pieces(vec,x,y,!color); */

    count = checkedp(vec,color);
    if(!count) { return(0); } /* Nobody is threatening this king right now. */
    
    off_x = get_x(*vec);
    off_y = get_y(*vec);

    /* First check if there's any escape locations: */
    moves = king_moves;
    /* Loop as long at least other one is non-zero: */
    while(((xx = *moves++),(yy = *moves++),(xx || yy)))
     {
       register int xxx,yyy;
       int result;

       xxx = x+xx; yyy = y+yy;
       /* Check if the offender is adjacent piece: */
/*     if(makecoord((xxx),(yyy)) == *vec) { adjacent=1; } */
       if((off_x == (xxx)) && (off_y == (yyy))) { adjacent=1; }
        /* If found empty place or one occupied by an enemy: */
       if(((neighbor = square((xxx),(yyy))) != EDGE) &&
          ((neighbor == EMPTY) || (getcolor(neighbor) != color)))
	{ /* Then clear the king's location tentatively:
 (It could obstruct the offender's view if remaining in the 'old' location) */
	  square(x,y) = EMPTY;
          /* And check whether there are threats to that adjacent square: */
	  if(!find_threatening_pieces(vec2,(xxx),(yyy),!color))
           {
	     check_message((_C_,"'s king could %s to the square %s",
	                   ((neighbor == EMPTY) ? "escape"
			                        : "capture"),
			   getcoord(xxx,yyy)));
             result=0;
           }
	  else { result=1; }
	  square(x,y) = ofcolor(KING,color); /* Restoration of the king. */
          if(!result) { return(result); }
	}
     }

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
    if(count = find_threatening_pieces(vec2,off_x,off_y,color))
     { /* And some of them can be used to capture the offender without
          checking the king: */
       if(!check_if_threatened(vec2,x,y,off_x,off_y,color))
        { return(0); }
     }

    off_piece = square(off_x,off_y);

    /* If piece is adjacent or it is knight, then it's sure mate,
        because we can put nothing between: */
    if(adjacent || (color_off(off_piece) == KNIGHT)) { return(1); }

    /* Check whether it is possible to move or drop any pieces between: */
    return(!interpose_possible(vec,x,y,off_x,off_y,color));
}

/*
   Checks with check_if_threatened every square between x,y and x2,y2
   and if finds some piece which could be interposed there without
   check, then return 1 immediately, otherwise zero.
 */
int interpose_possible(vec,x,y,x2,y2,color)
COR *vec;
int x,y;
int x2,y2;
int color;
{
    register int x3,y3; /* Varying from x,y to x2, y2. */
    register int xx,yy; /* The directions. */

    xx = ((x2 - x) ? (((x2 - x) < 0) ? -1 :  1) : 0);
    yy = ((y2 - y) ? (((y2 - y) < 0) ? -1 :  1) : 0);
    x3 = x;
    y3 = y;

/*  while(!((x3 == x2) && (y3 == y2))) */
    for(x3 += xx, y3 += yy; (square(x3,y3) == EMPTY); x3 += xx, y3 += yy)
     {
       /* If any drop is possible then return 1 immediately: */
       if(any_drop_possible(hands[color],x3,y3,color)) { return(1); }
       /* If there is any own piece(s) which could move to this square: */
       if(find_threatening_pieces(vec,x3,y3,color)   &&
       /* And some of them can be used to avoid check: */
          !check_if_threatened(vec,x,y,x3,y3,color))
        { return(1); }
     }

    { /* Make some internal error checking: */
      int piece;
      piece = color_off(square(x3,y3));
      if(!((piece == LANCE) ||
           (unpromote(piece) == BISHOP) ||
           (unpromote(piece) == ROOK)))
       {
         errlog((E1,"**Internal error encountered when called:\n"));
         errlog((E1,"interpose_possible(vec,%d,%d,%d,%d,%d)\n",
                    x,y,x2,y2,color));
         errlog((E1,"x3=%d, y3=%d, xx=%d, yy=%d, piece=%d\n",
                    x3,y3,xx,yy,piece));
	 return(0);
       }
    }

    return(0);
}

/*
   Try to move every piece in turn from vec to x2, y2 and check
   whether x,y is threatened after that. If found case where it's
   not threatened, then return zero immediately.
   Otherwise return nonzero.
 */
int check_if_threatened(vec,x,y,x2,y2,color)
register COR *vec;
int x,y,x2,y2;
int color;
{
    register int x3,y3;
    int save_piece;
    register int save_piece2,own_king;
    int count=1;
    COR vec2[MAXTHREATS];

    save_piece = square(x2,y2);
    own_king = ofcolor(KING,color);

    while(*vec)
     {
       x3 = get_x(*vec);
       y3 = get_y(*vec);
       square(x2,y2) = save_piece2 = square(x3,y3);
       square(x3,y3) = EMPTY;
       if(save_piece2 == own_king)
        { /* If king used for capturing x2,y2 then check that new square of it: */
          if(!(count = find_threatening_pieces(vec2,x2,y2,!color))) { break; }
        }
       else
        { /* Otherwise check the threats to unmoved loc of king: */
          if(!(count = find_threatening_pieces(vec2,x,y,!color))) { break; }
	}
       square(x3,y3) = save_piece2;
       vec++;
     }

    square(x3,y3) = save_piece2;
    square(x2,y2) = save_piece;

    if(!count)
     {
       char tmpbuf[20];
       /* Must use this separately because getcoord uses static buffer: */
       strcpy(tmpbuf,getcoord(x2,y2));
       check_message((_C_,
        " could %s the offender by moving %s to %s",
        ((save_piece == EMPTY) ? "interpose" : "capture"),
	getcoord(x3,y3),tmpbuf));
     }
    return(count);
}


/*
   If there is any piece in hand which could be validly dropped to x,y
   then return 1, otherwise zero.
 */
int any_drop_possible(hand,x,y,color)
register PIECES *hand;
register int x,y;
int color;
{
    int i;

/* If there's no pieces hand at all, then return zero immediately: */
    if(!*hand) { return(0); }

/*  for(;*hand;hand++) Old code. */
/* Check hand for all pieces from bishop to king: */
    for(i=2; i < MAX_IN_HAND; i++)
     { /* If there is that kind of piece in hand, and it's possible to drop: */
       if(hand[i] && !illegal_drop(i,x,y,color))
        {
          check_message((_C_," could drop %s to %s",
                         getpiecename(ofcolor(i,color)),getcoord(x,y)));
	  return(1);
	}
     }

    return(0);
}


/* Find all pieces of color 'color' which threat the location x,y and
    put them into vec, and return the count of them.
 */
int find_threatening_pieces(vec,x,y,color)
COR *vec;
int x,y;
int color;
{
    register PFI fun;
    register int i;
    register int count=0;
    int dir;

    dir = getdir(color);

    for(i=2; i < MAXFUNS; i++)
     {
       if((fun = movefuns[i]) == bugmove_) { continue; }
       count += (fun)((vec+count),x,y,ofcolor(i,color),dir);
     }

    return(count);
}


/* Find all the available squares where the piece can 
    put them into vec, and return the count of them.
 */
int find_available_squares(COR *vec, int src_x, int src_y, int piece)
{
    int color = getcolor(piece);

    if(EMPTY == piece) { *vec = 0; return(0); }
/* Do it simple way. First find all the squares which contain pieces of
   type 'piece' and could  enter here: */
    return((*movefuns[color_off(piece)])
             (vec,src_x,src_y,ofcolor(OPPONENT_OR_EMPTY,color),
               getdir(opponents_color(color))));
}


/* Is the move src_x,src_y -> dest_x,dest_y valid with the piece of color
   'color' ?
 */
int valid_movep(int src_x,int src_y, int dest_x, int dest_y, int piece,int color)
{
    COR vec[MAXTHREATS];

/* Do it simple way. First find all the squares which contain pieces of
   type 'piece' and could  enter here: */
    return(((*movefuns[piece])
                  (vec,dest_x,dest_y,ofcolor(piece,color),getdir(color)))
            && /* And then check whether source coordinates are among those: */
           strchr(vec,makecoord(src_x,src_y)));
}

/* Parses the move notation given in movestr, and returns the corresponding
   piece as result (without color), and various information in the
   call_by_ref arguments:

   p_movetype: '-' if ordinary move, 'x' if capture, '*' if drop.
   p_promf:    1 if promoted piece moved, 0 otherwise.
   p_promtype: 0 if not specified, '=' if user wants to keep it same,
               '+' if (s)he wants to promote it.
   p_src_x, p_src_y: source coordinates for the piece. Zero if drop.
   p_dest_x, p_dest_y: destination coordinates.

   If the notation or move is invalid, then the appropriate error
   message is printed, and zero is returned.
 */
int parse_notation
(movestr,p_movetype,p_promf,p_promtype,p_src_x,p_src_y,p_dest_x,p_dest_y,hh)
char *movestr;
int *p_movetype,*p_promf,*p_promtype,*p_src_x,*p_src_y,*p_dest_x,*p_dest_y;
int hh;
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
    if(isalpha(*movestr) && (p = strchr(letters,*movestr)))
     {
       piece = (p - letters);
       if(*p_promf)
        {
          /* Can't specify +D, +H nor +G or +K */
	  if(promotedp(piece) || !promotablep(piece))
	   {
	     err2log((E2,
"?There's no such piece as promoted %s !\n",getpiecename(piece)));
             return(0);
	   }
	  else { piece = promoted(piece); }
	}
     }
    else { goto ertzu; } /* Not valid letter. */

    /* If the source location specified too: */
    if(isdigit(*(movestr+1)))
     { /* 0 is not valid digit: */
       if(!(*p_src_x = (*++movestr - '0'))) { goto ertzu; }

       /* Next character must be a letter between A-I: */
       if(!isalpha(*++movestr)) { goto ertzu; }
       *movestr = tolower(*movestr); /* Make letter coordinate lowercase */
       if((*p_src_y = ((*movestr - 'a')+1)) > BSIZE) { goto ertzu; }
     }

    movestr++;
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

    /* Then must follow the destination coordinates.
       First has to be a digit: */
    if(!isdigit(*++movestr)) { goto ertzu; }
    if(!(*p_dest_x = (*movestr - '0'))) { goto ertzu; } /* But not zero! */

    /* Next character must be a letter between A-I: */
    if(!isalpha(*++movestr)) { goto ertzu; }
    *movestr = tolower(*movestr); /* Make letter coordinate lowercase */
    if((*p_dest_y = ((*movestr - 'a')+1)) > BSIZE) { goto ertzu; }

    *p_promtype = *++movestr;
    if(*p_promtype && (*p_promtype != '+') && (*p_promtype != '='))
     { goto ertzu; } /* If something else than + or = follows */

    /* Don't accept moves like G-3c+ or +B-3b+ or S*7e= */
    if(!(*p_promtype &&
         (!promotablep(piece) || promotedp(piece) /* *p_promf */ ||
          (*p_movetype == '*') || (*p_movetype == '@'))))
     { return(piece); } /* However, if OK then return the piece. */

ertzu:

    err2log((E2,"?Cannot parse notation:\n%s\n",origmovestr));
    for(i=(movestr - origmovestr); i--;) { strcat(E2," "); }
    strcat(E2,"^\n");
    for(i=(movestr - origmovestr); i--;) { strcat(E2," "); }
    strcat(E2,(*movestr ? "Invalid character!\n" : "Character(s) missing!\n"));
    return(0);
}


int handle_handicap(movestr,color,interactive)
char *movestr;
int color,interactive;
{
    int piece,movetype,promf,promtype;
    int src_x,src_y,dest_x,dest_y;
    register int i,result=1;
    register char *cordp,*next;
    unsigned char *squareloc;
    COR handicaps[MAXPIECES];

    if(_movesmade_ != 1)
     {
       err2log((E2,
"?Handicap move is only possible as the first move of game (Not %lu.)!\n",
         _movesmade_));
       return(0);
     }

    /* First, parse and check the coordinates: (collect them to handicaps) */
    i=0;
    cordp = movestr;
    while(cordp)
     { /* If comma found, then overwrite it so we can read the coordinate: */
       if(next = strchr(cordp,','))
        { *next = '\0'; }
       if(!(piece = parse_notation(cordp,&movetype,&promf,&promtype,
                                &src_x,&src_y,&dest_x,&dest_y,1)))
        { result = 0; } /* If parse error. */
       if(next) { *next = ','; } /* Restore the comma. */
       if(!result) { return(result); }
       if(square(dest_x,dest_y) != ofcolor(piece,!color))
        {
	  err2log((E2,"?There is no %s %s in the square %s!\n",
           getlowcolname(!color),
	   getpiecename(ofcolor(piece,!color)),
	   getcoord(dest_x,dest_y)));
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
       squareloc = &square(get_x(handicaps[i]),get_y(handicaps[i]));
       /* If removing king (heh, heh ;-) then clear its coordinates also,
          so that it can't be checked: */
       if((piece = *squareloc) == ofcolor(KING,!color))
        { *(kings[!color]) = *(kings[!color]+1) = 0; }
       *squareloc = EMPTY; /* Empty the location. */
       /* Impasse limit for white is reduced by whatever pieces he gives
           as handicap: */
       _w_impasse_points_ -= piecevalues[color_off(piece)];
     }

    return(1);
}


/*
   This makes lot of logical checks to see whether move is valid.
   However, first the parse_notation is called to see whether
   the move is syntactically valid. If move is erroneous then
   the appropriate error message is printed and zero is returned.
   Otherwise the move is made and non-zero is returned.
   Interactive is 1 if user can reply, 0 if loading moves from the
   file.
 */
int check_move(movestr,color,interactive)
char *movestr;
int color,interactive;
{
    int piece,your_piece,destpiece,movetype,promf,promtype;
    int src_x,src_y,dest_x,dest_y;
    int i;
    int do_it;
    char *prompt_string;
    COR vec[MAXTHREATS];

    err2log((E2,"")); /* Clear this first. */

    /* And these, just for sure: */
    piece = your_piece = destpiece = promf = promtype = 0;
    src_x = src_y = dest_x = dest_y = 0;
    _captured_piece_ = EMPTY;

    if(strequ(movestr,"RESIGN"))
     { movetype = 'R'; } /* As resign. */
    else if(cutstrequ(movestr,"HANDICAP:"))
     {
       int hh_result;

       if(!(hh_result = handle_handicap((movestr+9),color,interactive)))
        { return(hh_result); }
       movetype = '@';
     }
    else /* It's an ordinary move, drop, or capture, but not handicap. */
     {
       if(!(piece = parse_notation(movestr,&movetype,&promf,&promtype,
                                &src_x,&src_y,&dest_x,&dest_y,0)))
        { return(0); } /* If parse error. */
       your_piece = ofcolor(piece,color);
       destpiece  = square(dest_x,dest_y);
       _captured_piece_ = destpiece;

/* Debugging: */
       if(_debug_flag_)
        {
          fout((OB,
"**Debugging in check_move(%s,%d,%d): square(%d,%d)=%d, your_piece=%d\n",
                movestr,color,interactive,src_x,src_y,
	        square(src_x,src_y),your_piece));
          fout((OB,"piece=%d, destpiece=%d, dest_x=%d, dest_y=%d\n",
	            piece,destpiece,dest_x,dest_y));
          fout((OB,"movetype=%d,%c, promf=%d, promtype=%d,%c\n",
	            movetype,movetype,promf,promtype,promtype));
          fout((OB,"king: %d,%d\n",*(kings[color]),*(kings[color]+1)));
        }

       /* If destination square already occupied by player's own piece: */
       if((destpiece != EMPTY) && (getcolor(destpiece) == color))
        {
          err2log((E2,
"?Can't %s to %s, it's already occupied by your %s!\n",
           ((movetype == '*') ? "drop" : "move"),getcoord(dest_x,dest_y),
	     getpiecename(destpiece)));
          return(0);
        }

       /* If trying to capture to an empty square: */
       if((movetype == 'x') && (destpiece == EMPTY))
        {
          err2log((E2,
"?There's nothing to capture in the square %s !\n",
               getcoord(dest_x,dest_y)));
          return(0);
        }

       /* If trying to move to the square occupied by an enemy piece. */
       if((movetype == '-') && (destpiece != EMPTY))
        {
          err2log((E2,
"?There's an enemy piece in the square %s. If you want to capture it\nuse x instead of - in your notation.\n",
               getcoord(dest_x,dest_y)));
          return(0);
        }

       if(movetype == '*') /* It's drop. */
        { /* If not that kind of piece in your hand. */
          if(!check_piece_in_hand(piece,hands[color]))
           {
             err2log((E2,
"?You have no %s in your hand!\n",getpiecename(your_piece)));
             return(0);
           }

          /* If the destination square is already occupied. */
          if(destpiece != EMPTY)
           {
             err2log((E2,
"?There's an enemy piece in the square %s. Can't drop there.\n",
               getcoord(dest_x,dest_y)));
             return(0);
           }

          switch(illegal_drop(piece,dest_x,dest_y,color))
           {
	     case UNABLE_TO_MOVE:
	      {
                err2log((E2,
"?Can't drop %s to %s, because it would never be able to move from there!\n",
                     getpiecename(your_piece),getcoord(dest_x,dest_y)));
                return(0);
	      }
             case DOUBLE_PAWN:
              {
	        err2log((E2,
"?You already have an unpromoted pawn on the file %d !\n",dest_x));
                return(0);
	      }
	     case PAWN_DROP_MATE:
	      {
                err2log((E2,"?You cannot mate with the pawn drop!\n"));
	        return(0);
	      }
	   }

        } /* End of "if it's drop" */

       else if(src_x) /* Source address specified. */
        { /* If there's no that kind of piece in that square. */
          if(square(src_x,src_y) != your_piece)
           {
	     err2log((E2,
"?You have no %s in the square %s !\n",
              getpiecename(your_piece),getcoord(src_x,src_y)));
             return(0);
	   }

          if(!valid_movep(src_x,src_y,dest_x,dest_y,piece,color))
           {
             err2log((E2,
"?The %s doesn't move that way!\n",getpiecename(your_piece)));
          return(0);
	   }
        }
       else /* the source address not specified. */
        {
          int count;

          count = (*movefuns[piece])(vec,dest_x,dest_y,your_piece,getdir(color));

          if(!count) /* If can't find a piece which could move to dest. */
           {
             err2log((E2,"?From where???\n"));
	     return(0);
           }

          /* If there's more than one pieces which could move here: */
          if(count > 1)
           {
             err2log((E2,"?Specify which one: "));
	     sprintcoords_or(E2,vec);
	     strcat(E2,"\n");
	     return(0);
           }

/* There was exactly one piece which could move here,
   so get its coordinates: */
          src_x = get_x(*vec);
          src_y = get_y(*vec);

          /* Debugging check: */
          if(square(src_x,src_y) != your_piece)
           {
	     errlog((E1,
"**Internal error in check_move(%s,%d,%d): square(%d,%d)=%d != your_piece=%d\n",
             movestr,color,interactive,src_x,src_y,
	     square(src_x,src_y),your_piece));
	     errlog((E1,"piece=%d, destpiece=%d, dest_x=%d, dest_y=%d\n",
	            piece,destpiece,dest_x,dest_y));
             errlog((E1,"movetype=%d, promf=%d, promtype=%d\n",
	            movetype,promf,promtype));
	     return(0);
           }
        } /* else  the source address not specified. */

       /* Not a drop, nor a move of the promoted piece: */
       if((movetype != '*') && !promotedp(piece) /* !promf */)
        { /* If possible to promote the piece, but no = nor + specified: */
          if(in_promzonep(src_y,color) || in_promzonep(dest_y,color))
           {
             if(!promtype && promotablep(piece))
              {
                int c;

loop:
	        c = ynq("?Promote? <y,n,q>");
                if(c == 'y') { strcat(movestr,"+"); promtype = '+'; }
                else if(c == 'n') { strcat(movestr,"="); promtype = '='; }
                else if(c == 'q') { return(0); }
                else { goto loop; }
	      }
           }
 /* Else the piece is not in the promotion zone and is not going there!
    and it's not drop, not promoted piece and there was = or + specified: */
          else if(promtype)
           {
             if(promtype == '+')
              {
                err2log((E2,
"?Can't promote outside of the promotion zone!\n"));
                return(0);
	      }
             else /* promtype must be = */
              {
                err2log((E2,
"?Couldn't promote anyway, get that %c off from your notation!\n",
                  promtype));
                return(0);
	      }
           }
        } /* if((movetype != '*') && !promf) */

       /* If trying to move an unpromoted pawn, lance or knight
          to the last rank (or to second last in the case of knight): */
       if((promtype != '+') && (movetype != '*') &&
           unable_to_move(piece,dest_y,color))
        {
          err2log((E2,
"?Can't move %s to %s, because it would never be able to move from there!\n(You must promote it).\n",
             getpiecename(your_piece),getcoord(dest_x,dest_y)));
          return(0);
        }


       /* Now everything should be OK this far, let's try the move. */

       switch(movetype)
        {
          case '*': /* If a drop, then remove the piece from our own hand. */
           { 
             remove_piece_from_hand(piece,hands[color]);
	     break;
           }
          case 'x':
           {
             /* Add the captured piece to player's own hand.
	        (But clear its color first and unpromote it!) */
             add_piece_to_hand(color_off(unpromote(destpiece)),hands[color]);
             /* Fall through. */
           }
          case '-': /* If not a drop, then clear the source square. */
           {
             square(src_x,src_y) = EMPTY;
           }
        }

       square(dest_x,dest_y) =
         ((promtype == '+') ? promoted(your_piece) : your_piece);

       if(piece == KING)
        { /* If king moved, then set the new coordinates of him: */
          *(kings[color])   = dest_x;
          *(kings[color]+1) = dest_y;
        }

     } /* else  It's an ordinary move, drop, or capture, but not handicap. */

    if(interactive && (movetype == 'R'))
     {
       prompt_string = "Really want to resign? <p,o,n,y>";
       goto ask_loop;
     }
    else if(checkedp(vec,color))
     {
       do_it = KING_LEFT_IN_CHECK;
       if(interactive)
        {
          fout((OB,
"Your king would be in check! You can do this move, but your king may\n"));
          fout((OB,
"then be captured, resulting your loss. "));
          prompt_string = "Do it anyway? <p,o,n,y>";
          goto ask_loop;
	}
     }
    else if(interactive)
     {
       int c;

       do_it = NOTHING_SPECIAL;
       fout((OB,"Your move is OK. "));
       prompt_string = "Do it? <p,o,n,y>";
ask_loop:
       c = ynq(prompt_string); /* The Plastic Pony? */
       switch(tolower(c))
        {
	  case 'y': { break; }  /* Just keep do_it intact. */
	  case 'n': { do_it = 0; break; }
          case 'o': case 'p':   /* Print the board, one way or another. */
	   { /* Set _latest_move_ temporarily to point to movestr, so that */
	     _latest_move_ = movestr; /* printboard shows correct move. */
	     /* If user gives uppercase letter, then use just ascetic
	         default mode when printing the board (No kanji, no
		 ansi/vt102 attributes): */
	     printboard(((c == 'o') ? !color : color),stdout,
	             (isupper(c) ? DEFAULT_FLAGS : _flags_));
	     _latest_move_ = space_for_latest_move; /* Restore it. */
	     goto ask_loop;
	   }
	  default: { goto ask_loop; }
	}
     }
    else
     { do_it = 1; } /* When loading game from log file. */

    *escapemsgbuf = *checkmsgbuf = '\0';

    if(do_it)
     {
       if(movetype == 'R')
        {
	  do_it = RESIGNED;
	}
       /* If capturing opponent's king: */
       else if(destpiece == ofcolor(KING,!color))
        {
	  do_it = KING_CAPTURED;
/*	  strcpy(checkmsgbuf,"king is captured"); */
	}
       else if(do_it == KING_LEFT_IN_CHECK) /* Own king left in check. */
        {
	  strcpy(checkmsgbuf,"king is left in check");
	}
       else if(checkedp(vec,!color)) /* Opponent's king checked? */
        {
	  if(checkmatedp(!color)) /* Mated? */
	   {
	     do_it = KING_MATED;
	     strcpy(checkmsgbuf,"king is mated by ");
	   }
	  else /* No, just checked. */
	   {
	     do_it = KING_CHECKED;
	     strcpy(checkmsgbuf,"king is checked by ");
	   }
          sprintcoords_and(checkmsgbuf,vec);
	}
       else { do_it = NOTHING_SPECIAL; }
     }

    /* More ugly kludgous code. If player plays against itself, then
        don't do moves, when check_move is called first time with
	interactive=1, because then moves would be made two times each,
	which is not a good idea.
     */
    if(do_it && ((_blackplid_ != _whiteplid_) || !interactive))
     {
       /* If capturing opponent's king: */
       if(do_it == KING_CAPTURED)
	{ /* Then clear coordinates of him, so he is no more on the board: */
          *(kings[!color])   = 0;
          *(kings[!color]+1) = 0;
	}
       return(do_it);
     }
    else /* Don't do it. */
     { /* Restore the situation on the board and in the hands: */
       switch(movetype)
        {
	  case 'R': /* If resign, then nothing was changed. */
	   { return(do_it); }
          case '@': /* If handicap removal. */
	   { /* Then this is the easiest way to restore situation on board. */
	     init_board();
	     return(do_it);
           }
          case '*':
           { /* If a drop, then put piece back to our own hand. */
	     add_piece_to_hand(piece,hands[color]);
	     break;
           }
          case 'x':
           { /* If capture then remove the captured piece from our own hand. */
	     remove_piece_from_hand(color_off(unpromote(destpiece)),
	                             hands[color]);
	     /* Fall through. */
           }
/* If not a drop, then restore the moved piece back to the source square: */
          case '-':
           {
             square(src_x,src_y) = your_piece;
           }
        }

       /* And restore the destination square also. */
       square(dest_x,dest_y) = destpiece;

       if(piece == KING)
        { /* If king moved, then restore the old coordinates of him: */
          *(kings[color])   = src_x;
          *(kings[color]+1) = src_y;
        }

       return(do_it);
     }
}


int illegal_drop(piece,dest_x,dest_y,color)
int piece,dest_x,dest_y,color;
{
    int result;

    if(unable_to_move(piece,dest_y,color)) { return(UNABLE_TO_MOVE); }

    /* If it's able to move and is not pawn, then it's ok. */
    if(piece != PAWN) { return(0); }

    /* If pawn in the same file. */
    if(samepiece_in_file(dest_x,ofcolor(piece,color))) { return(DOUBLE_PAWN); }

    /* If pawn dropped just front of the opponent's king: */
    if(square(dest_x,dest_y+getdir(!color)) == ofcolor(KING,!color))
     {
/* Put pawn tentatively to its destination so that we see whether it
   checkmates opponent's king: */
       square(dest_x,dest_y) = ofcolor(PAWN,color);
       if(checkmatedp(!color)) { result=0; }
       else { result=1; }
       square(dest_x,dest_y) = EMPTY; /* Restore the situation. */
       if(!result) { return(PAWN_DROP_MATE); }
     }

    return(0);
}


/* Returns nonzero if the piece is pawn, lance or knight and y is end rank,
    or if the piece is knight and y is the second last rank.
 */
int unable_to_move(piece,y,color)
int piece,y,color;
{
    return(
           ((y == (color ? BSIZE : 1)) && (piece >= PAWN) && (piece <= KNIGHT))
             ||
           ((piece == KNIGHT) && (y == (color ? (BSIZE-1) : 2)))
	  );
}

/* Check whether y coordinate is in the promotion zone.
   Ranks a-c (1-3) for black (0) and g-i (7-9) for white (1).
 */
int in_promzonep(y,color)
int y,color;
{
    return(color ? (y > (BSIZE-3)) : (y <= 3));
}

char *getcoord(x,y)
int x,y;
{
    static char coordbuf[3];

    *coordbuf   = '0' + x;
    coordbuf[1] = ('a'-1)+y;
    coordbuf[2] = '\0'; /* Ending zero. */
    return(coordbuf);
}

/* Concatenate the coordinates found from the vec to dstbuf: */
sprintcoords_and(dstbuf,vec)
register char *dstbuf;
register COR *vec;
{
    while(*vec)
     {
       strcat(dstbuf,getcoord(get_x(*vec),get_y(*vec)));
       vec++;
       strcat(dstbuf,(*vec ? (*(vec+1) ? ", " : " and ") : ""));
     }
}

/* Concatenate the coordinates found from the vec to dstbuf: */
sprintcoords_or(dstbuf,vec)
register char *dstbuf;
register COR *vec;
{
    while(*vec)
     {
       strcat(dstbuf,getcoord(get_x(*vec),get_y(*vec)));
       vec++;
       strcat(dstbuf,(*vec ? (*(vec+1) ? ", " : " or ") : ""));
     }
}

/*
printcoords_or(vec)
COR *vec;
{
    while(*vec)
     {
       fout((OB,"%s",getcoord(get_x(*vec),get_y(*vec))));
       vec++;
       fout((OB,(*vec ? (*(vec+1) ? ", " : " or ") : "\n")));
     }
}
*/


/*
   Structure of the hand is as follows:
   First byte (index zero) is total count of pieces in hand, and
   in positions 2-9 there is counts for pieces from bishop to
   king, respectively. Position 1 is unused.
   Some of these functions could be macros also, but due to
   historical reasons they are not. (Means that I'm too lazy...)
 */

inithand(hand)
PIECES *hand;
{
    int i=0;

    /* Clear hand: */
    for(i=0; i < MAX_IN_HAND; ) { *(hand+i++) = 0; }
}


add_piece_to_hand(piece,hand)
int piece;
PIECES *hand;
{
    hand[piece]++; /* Increase the count of that piece. */
    ++*hand;       /* And increase the total count also. */
}

remove_piece_from_hand(piece,hand)
int piece;
PIECES *hand;
{
    hand[piece]--; /* Decrease the count of that piece. */
    --*hand;       /* And decrease the total count also. */
}

int check_piece_in_hand(piece,hand) /* Could be macro also. */
int piece;
PIECES *hand;
{
    /* Return the count of those pieces in this hand: */
    return(u_piecep(piece) ? hand[piece] : 0);
}


#ifdef OLD_CODE_COMMENTED_OUT

/* This adds piece to the end of hand. */
add_piece_to_hand(piece,hand)
int piece;
PIECES *hand;
{
    while(*hand) { hand++; }
    *hand++ = piece;
    *hand = 0;
}

/*
   This takes first piece of type 'piece' from hand, and deletes it.
   If there's no piece of that type, then return 0, otherwise the
   type of piece.
 */
int remove_piece_from_hand(piece,hand)
int piece;
PIECES *hand;
{
    /* Check if there is that kind of piece: */
    if(!(hand = ((PIECES *) strchr(hand,piece)))) { return(0); }

    /* Delete the piece, i.e. copy the remaining pieces one left: */
    strcpy(hand,(hand+1)); /* We can use this because PIECES = char */
    return(piece);
}

int check_piece_in_hand(piece,hand) /* Could be macro also. */
int piece;
PIECES *hand;
{
    /* Check if there is that kind of piece: */
    return(!!strchr(hand,piece));
}

#endif

int samepiece_in_file(x,piece)
register int x,piece;
{
    register int y;

    for(y=1; y < (BSIZE+1); y++)
     { if(square(x,y) == piece) { return(y); } }
    return(0);
}


initmovefuns()
{
   int i;

   /* First initialize everything to bugmove_ */
   for(i=0; i < MAXFUNS; i++)
    { movefuns[i] = bugmove_; }

   movefuns[BISHOP]           = bishop_;       /*  2 */
   movefuns[ROOK]             = rook_;         /*  3 */
   movefuns[PAWN]             = pawn_,         /*  4 */
   movefuns[LANCE]            = lance_;        /*  5 */
   movefuns[KNIGHT]           = knight_;       /*  6 */
   movefuns[SILVER]           = silver_;       /*  7 */
   movefuns[GOLD]             = gold_;         /*  8 */
   movefuns[KING]             = king_;         /*  9 */
   movefuns[promoted(BISHOP)] = prom_bishop_;  /* 18 */
   movefuns[promoted(ROOK)]   = prom_rook_;    /* 19 */
   movefuns[promoted(PAWN)]   = gold_;         /* 20 Promoted pawn.    */
   movefuns[promoted(LANCE)]  = gold_;         /* 21 Promoted lance.   */
   movefuns[promoted(KNIGHT)] = gold_;         /* 22 Promoted knight.  */
   movefuns[promoted(SILVER)] = gold_;         /* 23 Promoted silver.  */
}


initboard()
{
    strcpy(_latest_move_,"NONE");
    _movestat_ = _movesmade_ = 0;
    _eog_ = 0;
    init_board();
}


init_board()
{
    register int x,y;

    _b_impasse_points_ = _w_impasse_points_ = DEFAULT_IMPASSE_POINTS;
    _captured_piece_ = EMPTY;
    *escapemsgbuf = *checkmsgbuf = '\0';

    inithand(blacks_hand);
    inithand(whites_hand);

    /* Initialize kings' locations: */
    *blacks_king = *whites_king = 5; /* Both are in file 5 initially. */
    blacks_king[1] = BSIZE; /* And Y coordinates... */
    whites_king[1] = 1;    /*  ...too. */

    /* The upper edge. */
    for(y=-1; y < 1; y++)
     {
       for(x=-1; x < (BSIZE+3); x++) { square(x,y) = EDGE; }
     }

    /* The board itself. */
    for(; y < (BSIZE+1); y++) /* From a to i */
     {
       square(-1,y)        = EDGE;  /* Initialize the */
       square(0,y)         = EDGE; /*   right edge.  */
       for(x=1; x < (BSIZE+1); x++) { square(x,y) = EMPTY; }
       square((BSIZE+1),y) = EDGE;  /* Initialize the */
       square((BSIZE+2),y) = EDGE; /*   left edge.   */
     }

    /* The lower edge. */
    for(; y < (BSIZE+3); y++)
     {
       for(x=-1; x < (BSIZE+3); x++) { square(x,y) = EDGE; }
     }

    /* Now the board is empty. Then put the pieces: */

    /* White side. */
    for(y=1,x=1; x < (BSIZE+1); x++)
     { square(x,y) = makewhite(firstrank[x-1]); }
    square(8,2) = makewhite(ROOK);
    square(2,2) = makewhite(BISHOP);
    for(y=3,x=1; x < (BSIZE+1); x++)
     { square(x,y) = makewhite(PAWN); }

    /* Black side. */
    for(y=7,x=1; x < (BSIZE+1); x++)
     { square(x,y) = makeblack(PAWN); }
    square(8,8) = makeblack(BISHOP);
    square(2,8) = makeblack(ROOK);
    for(y=9,x=1; x < (BSIZE+1); x++)
     { square(x,y) = makeblack(firstrank[x-1]); }

}

/* Get count of pieces on board and hands.
   b1count will contain the count of black's pieces,
   and b2count the weighted count of ----- " -----.
   w1count and w2count for white's pieces, similarly.
 */
countpieces(b1count,w1count,b2count,w2count)
int *b1count,*w1count,*b2count,*w2count;
{
    *b2count = *w2count = 0;
    *b1count = count_hand(blacks_hand,b2count);
    *w1count = count_hand(whites_hand,w2count);
    count_pieces_on_board(b1count,w1count,b2count,w2count);
}


count_pieces_on_board(b1count,w1count,b2count,w2count)
int *b1count,*w1count,*b2count,*w2count;
{
    register int x,y;
    int piece;
    
    for(y=1; y < (BSIZE+1); y++) /* From a to i */
     {
       for(x=1; x < (BSIZE+1); x++) /* From 1 to 9 */
        {
	  if((piece = square(x,y)) != EMPTY)
	   {
	     if(whitep(piece))
	      {
	        ++*w1count;
		(*w2count) += piecevalues[color_off(piece)];
	      }
	     else
	      {
	        ++*b1count;
		(*b2count) += piecevalues[color_off(piece)];
	      }
	   }
	}
     }
}


/* Returns as result the count of pieces in hand, and in count2 the
    weighted count: */
int count_hand(hand,count2)
PIECES *hand;
int *count2;
{
    int i;

    for(i=2; i < MAX_IN_HAND; i++)
     { (*count2) += (*(hand+i) * piecevalues[i]); }

    return(*hand);
}


 
printboard(whichway,fp,flags)
int whichway; /* Which way the board is printed? */
FILE *fp;
ULI flags;
{
    char *create_interline();
    register int xx,x,yy,y;
    ULI save_flags;
    int piece;
    int b1count,w1count,b2count,w2count;
    unsigned char blackcode,whitecode,threatcode;
    char c;
    register char *op;
    char obuf[MAXBUF+3],interline[MAXBUF+3];

    save_flags = _flags_;
    _flags_ = flags;

    blackcode  = getblackcode(_flags_);
    whitecode  = getwhitecode(_flags_);
    threatcode = getthreatenedcode(_flags_);

    countpieces(&b1count,&w1count,&b2count,&w2count);

    sprintf(obuf,"%s - %-24s %2d %2d ",
      getcapcolname(!whichway),_names_[!whichway],
      (whichway ? b1count : w1count),(whichway ? b2count : w2count));
    printhand(hands[!whichway],obuf);
    output_stuff(obuf,fp);

    if(getboardstyle(_flags_) & (HTML_BOARD))
     { output_stuff("<TABLE BORDER>", fp); }

    printnumbers(whichway,obuf);
    output_stuff(obuf,fp);

    create_interline(interline);

    for(yy=1; yy < (BSIZE+1); yy++)
     {
       y = (whichway ? (BSIZE+1 - yy) : yy);

/*     if(yy != 1) */
        {
          if(!(getboardstyle(_flags_) & HTML_BOARD))
           { output_stuff(interline,fp); }
	  op = obuf;
          if(yy == 6) /* Print move information after centre rank. */
           {
             if(_captured_piece_ != EMPTY)
	      {
	        sprintf(obuf,
		  "     (Captured %s)",getpiecename(_captured_piece_));
		op += strlen(op);
	      }
	   }
	  *op++ = '\n';
	  *op = '\0';
	  output_stuff(obuf,fp);
	}

       op = obuf;

       if(getboardstyle(_flags_) & HTML_BOARD)
        {
          strcpy(op,"<TR><TH>");
          op += strlen(op);
        }
       else
        {
        }

       *op++ = (('a'-1)+y); /* Rank letter, from a to i */
       *op++ = ' ';

       for(xx = BSIZE; xx ; xx--)
        {
	  COR vec[MAXTHREATS];
          COR avail_squares[MAXFREEDOMS];
          int freedoms;
	  unsigned char fancy_flag;

          x = (whichway ? (BSIZE+1 - xx) : xx);

	  piece = square(x,y);

          fancy_flag=0;
          if(getboardstyle(_flags_) & HTML_BOARD)
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
	         find_threatening_pieces(vec,x,y,!getcolor(piece)))
	      { *op++ = getcodeletter(threatcode); fancy_flag=1; }
	   }

          getpieceshort(op,piece);
          op += strlen(op);

          if(getboardstyle(_flags_) & HTML_BOARD)
           {
             int i;
             *op++ = '"';
             freedoms = find_available_squares(avail_squares, x, y, piece);
#ifdef OLD_WAY_JUST_FOR_DEMONSTRATION
              {
                strcpy(op," onClick=\"alert('Square ");
                op += strlen(op);
                strcpy(op, getcoord(x,y));
                op += strlen(op);
                sprintf(op, " has %u freedoms')\"", freedoms);
                op += strlen(op);
              }
#else
             strcpy(op," onClick=\"domove('"); op += strlen(op);
             getpiececode(op, piece); op += strlen(op);
             strcpy(op, "','"); op += strlen(op);
             strcpy(op, getcoord(x,y)); op += strlen(op);
             sprintf(op, "',%u,'", freedoms);
             op += strlen(op);
             for(i=0; i < freedoms; i++)
              {
                strcpy(op,
                   getcoord(get_x(avail_squares[i]),get_y(avail_squares[i])));;
                op += strlen(op);
                if(i < (freedoms-1)) { *op++ = ','; }
              }
#endif
             strcpy(op,"')\">");
             op += strlen(op);
             if(whitep(piece)) { strcpy(op,"</FONT>"); op += strlen(op); }

           }

	  if(fancy_flag)
	   { *op++ = getcodeletter(NORMAL); }
	} /* for loop over x (one rank or row) */

       if(getboardstyle(_flags_) & HTML_BOARD)
        {
          strcpy(op,"</TH><TH>");
          op += strlen(op);
        }
       else
        {
          *op++ = ':';
          *op++ = ' ';
        }

       if(getboardstyle(_flags_) == ASCII)
        {
          *op++ = *obuf; /* Same rank letter as on the left edge. */
        }
       else if(getboardstyle(_flags_) == (UNIKANJI + HTML_BOARD))
        {
          sprintf(op,"&#%u;", Unumbers[y]);
          op += strlen(op);
        }
       else /* Otherwise, append JIS kanji number: */
        {
	  convert_jis(op, Jnumbers[y]);
          op += strlen(op);
        }

       if(getboardstyle(_flags_) & HTML_BOARD)
        {
          strcpy(op,"</TH></TR>");
          op += strlen(op);
        }

       if(yy == 5) /* Print move information after centre rank. */
        {
	  if(getboardstyle(_flags_) == ASCII) { *op++ = ' '; } /* Minor fix. */
	  if(!_movesmade_) { strcpy(op,"  No moves made.");  }
          else /* Watch out for extra-long moves, like handicaps! */
	   {  /* (They could spoil the output.) */
             sprintf(op,"  %s moved %s (%lu)",
                 getcapcolname(!get_parity(_movesmade_)),
                 _latest_move_,_movesmade_);
           }
	  op += strlen(op);
	}

       /*      if(getboardstyle(_flags_) & HTML_BOARD) { strcpy(op,"</TD></TR>"); op += strlen(op); } */

       *op++ = '\n';
       *op = '\0';
       output_stuff(obuf,fp);
     }

    if(getboardstyle(_flags_) & HTML_BOARD)
     {
     }
    else
     {
       output_stuff(interline,fp);
       output_stuff("\n",fp);
     }

    printnumbers(whichway,obuf);
    output_stuff(obuf,fp);

    if(getboardstyle(_flags_) & HTML_BOARD)
     {
       output_stuff("</TABLE>\n",fp);
     }

    sprintf(obuf,"%s - %-24s %2d %2d ",
      getcapcolname(whichway),_names_[whichway],
      (whichway ? w1count : b1count),(whichway ? w2count : b2count));
    printhand(hands[whichway],obuf);
    output_stuff(obuf,fp);

    _flags_ = save_flags; /* restore _flags_ */
}


printhand(hand,obuf)
PIECES *hand;
char *obuf;
{
    if(!*hand) /* If total count zero. */
     {
       strcat(obuf,"No pieces in hand.\n");
     }
    else
     {
       strcat(obuf,"Pieces in hand:"); print_hand(hand,obuf);
       strcat(obuf,"\n");
     }
}

print_hand(hand,obuf)
PIECES *hand;
register char *obuf;
{
    register int i,c,count;
    char tmpbuf[121];

    obuf += strlen(obuf); /* Find the end of obuf first. */

    for(i=2; i < MAX_IN_HAND; i++)
     {
       if((count = hand[i])) /* If there is one or more pieces of index i. */
        {
	  *obuf++ = ' '; /* Separating blank. */
	  getpieceshort(obuf,i);
	  /* If blank + letter, then delete blank: */
	  if(*obuf == ' ') { strcpy(obuf,(obuf+1)); }
          obuf += strlen(obuf);
	  /* Follow the letter with count, if there's more than one: */
	  if(count > 1)
	   {
	     if(getboardstyle(_flags_) != ASCII)
	      {
	        getkanjinum(tmpbuf,count);
		if(!(getboardstyle(_flags_) & UNIKANJI))
                 { convert_jis(obuf,tmpbuf); }
                else { strcpy(obuf,tmpbuf); }
	      }
	     else { sprintf(obuf,"%u",count); }
             obuf += strlen(obuf); /* Find the end of obuf again. */
           }
	}
     }
    *obuf = '\0'; /* Terminate the obuf with the ending zero. */
}


#ifdef OLD_CODE_COMMENTED_OUT

printhand(hand,fp)
PIECES *hand;
FILE *fp;
{
    if(!*hand)
     {
       fprintf(fp,"No pieces in hand.\n");
     }
    else
     {
       fprintf(fp,"Pieces in hand: "); print_hand(hand,fp);
       fprintf(fp,"\n");
     }
}


print_hand(hand,fp)
PIECES *hand;
FILE *fp;
{
    int piece,c;

    while(*hand)
     {
       piece = *hand;
       c = letters[unpromote(color_off(piece))];
       if(promotedp(piece)) { fprintf(fp,"+"); } /* Shouldn't be possible! */
       fprintf(fp,"%c",(whitep(piece) ? tolower(c) : c));
       if(!*++hand) { break; }
       fprintf(fp," ");
     }
}

#endif


char *create_interline(op)
register char *op;
{
    register int x;

    strcpy(op,"  "); op += 2;
    for(x=1; x < (BSIZE+1); x++)
     {
       *op++ = '+';
       if(getboardstyle(_flags_) == ASCII)
	{
	  *op++ = '-';
	  *op++ = '-';
        }
       else
        {
          convert_jis(op,JIS_HYPHEN);
          op += strlen(op);
        }
     }

    *op++ = '+';
    *op = '\0';
    return(op);
}


printnumbers(whichway,obuf)
int whichway;
register char *obuf;
{
    register int i;
    char tmpbuf[3];

    if(getboardstyle(_flags_) & (HTML_BOARD))
     { strcpy(obuf,"<TR><TH></TH>"); }
    else { strcpy(obuf,"  "); }
    obuf += strlen(obuf);

    for(i=BSIZE; i ;)
     {
       *obuf++ = ' ';
       if(getboardstyle(_flags_) == ASCII)
        {
          *obuf++ = ' ';
          /* I hope that BSIZE is never greater than 9, otherwise the results
              are messy: */
          *obuf++ = ('0' + (whichway ? (BSIZE+1-i--) : i--));
	}
       else if(getboardstyle(_flags_) == (UNIKANJI + HTML_BOARD))
        {
          sprintf(obuf,"<TH>&#%u;</TH>", (0xFF10 + (whichway ? (BSIZE+1-i--) : i--)));
          obuf += strlen(obuf);
        }
       else /* Use JIS number. */
        {
          tmpbuf[0] = '#';
          tmpbuf[1] = ('0' + (whichway ? (BSIZE+1-i--) : i--));
	  tmpbuf[2] = '\0';
	  convert_jis(obuf,tmpbuf);
          obuf += strlen(obuf); /* Find the end of obuf again. */
	}
     }

    if(getboardstyle(_flags_) & (HTML_BOARD))
     { strcpy(obuf,"<TH></TH></TR>"); obuf += strlen(obuf); }

    *obuf++ = '\n';
    *obuf = '\0';
}


