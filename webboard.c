
/*
 *
 * webboard.c & moves.c -- A simple HTML-based shoogi/chess-board
 *                         for experimental use.
 *
 *                         Based on earlier (and more complex)
 *                         Shoogi Server package (shoogi.c, etc.)
 *
 * Copyright (C) 1993-2001 by Antti Karttunen (Antti.Karttunen@iki.fi)
 *
 *
 *
 *
 *
 *
 *
 *
 */


#include <string.h>
#include <stdlib.h>
#include "webboard.h"
#include "mv_proto.h" /* Created with grep '^[A-Za-z]' < moves.c | fgrep "struct" | fgrep "(" | sed -e 's/)/);/g' > mv_proto.h */


char _debug_flag_=0;

/* char E1[MAXBUF+3]; */
char E2[CHKMSGBUF+3];
char OB[(MAXBUF*3)+3]; /* Output Buffer for fout. */



int G_argc;
char **G_argv;



/* The following two functions were taken from AK's conjugat software,
   module conjtest.cpp.

 */
UCHAR *parse_url_value(UCHAR *url_piece)
{
    UCHAR *s,*t;
    UCHAR save_char;
    unsigned int tmp;

    s = t = url_piece;

    while(*s)
     {
       switch(*s)
        {
          case '+': { *t++ = ' '; s++; break; } /* Plus signs to spaces */
          case '%':
           { /* Double %% can be used for escaping a percent sign itself */
             if(*(s+1) == '%') { *t++ = '%'; s += 2; }
             else if(NOT isalnum(*(s+1))) /* Not followed by hex digits? */
              {
                *t++ = *s++;
              }
             else if(strlen((char *)s) >= 3)
              {
                save_char = *(s+3);
                *(s+3) = '\0';
                /* Convert two hex digits to int */
                sscanf(((const char *)s+1),"%x",&tmp);
                *(s+3) = save_char;
                *t++ = ((UCHAR) tmp);
                s += 3;
              }
             else /* Copy the trailing percent signs literally. */ 
              { *t++ = *s++; }
             break;
           }
          default:  { *t++ = *s++; break; }
        }
     }

    *t = '\0';
    return(url_piece);
}


int parse_url_query_string(char *query_string, char **p_MOVES, char **p_NEXT, char **p_OVIEW) /* First is input, the rest are outputs */
{
    char *ptr1,*ptr2,*ptr3;
    unsigned int count=0;

    ptr1 = query_string;
    while(ptr1)
     {
       if(NOT(ptr2 = (strchr(((char *)ptr1),'=')))) { break; }
       if((ptr3 = (strchr(((char *)ptr2),'&'))))
        { *ptr3++ = '\0'; } /* Overwrite the ampersand, and skip past. */
       *ptr2++ = '\0'; /* Overwrite the equal sign, and skip past. */
/* ptr1 points to the beginning of the variable name.
   ptr2 points two characters past the end of the variable name, i.e. one
    past the equal sign (=) which has been overwritten by zero '\0',
    that is to the beginning of the variable value, corresponding to
    variable name where ptr1 points to.
   ptr3 points two characters past the end of variable value, i.e. one
    past the ampersand (&) which has been overwritten by zero, that is
    to the beginning of the next variable name. Or if we have found the
    last name=value pair, then it contains the NULL.
 */


       if(!strcasecmp(((char *)ptr1),"MOVES"))
        { /* Also if ptr2 is an empty string. */
          if(*ptr2) *p_MOVES = SCP(parse_url_value((UCHAR *)(ptr2))); 
        }
       else if(!strcasecmp(((char *)ptr1),"NEXT"))
        {
          if(*ptr2) *p_NEXT = SCP(parse_url_value((UCHAR *)(ptr2)));
        }
       else if(!strcasecmp(((char *)ptr1),"OVIEW"))
        {
          if(*ptr2) *p_OVIEW = SCP(parse_url_value((UCHAR *)(ptr2)));
        }
 
       ptr1 = ptr3;
       count++;
     }

    return(count);
}



main(argc,argv)
int argc;
char **argv;
{
    char *query_string;
    int moves_made;
    char *old_moves = "";
    char *next_move = "";
    char *moves_so_far = "";
    char *oview = NULL;
    char *progname;
    struct GIS *gis = NULL;
    char chkmsgbuf[16385];
    char errmsgbuf[16385];
    char escmsgbuf[16385];

    *chkmsgbuf = *errmsgbuf = *escmsgbuf = '\0';

    G_argc = argc;
    G_argv = argv;

    progname = strrchr(argv[0],'/'); /* We are on Unix, no backslashes */
    if(NULL == progname) { progname = argv[0]; }
    else { progname++; }

    if(NULL != (query_string = getenv("QUERY_STRING"))) /* Started as CGI? */
     {
       char *b,*o;
       MYINT board_selected=0;

       fprintf(stdout, "Content-Type: %s\r\n\r\n", "text/html; charset=UTF-8");
       fprintf(stdout, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
       fprintf(stdout, "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");

       argv++; argc--;
       parse_url_query_string(query_string,&old_moves,&next_move,&oview);


       if(strstr(progname,"sho"))
        {
          if(NULL == (gis = mv_new_gis(1,9))) /* Gametype = Shoogi, Boardsize = 9. */
           {
             fputs("</PRE>\n<P></BODY></HTML>",stdout);
             exit(1);
           }
        }
       else
        {
          if(NULL == (gis = mv_new_gis(0,8))) /* Gametype = Chess, Boardsize = 8. */
           {
             fputs("</PRE>\n<P></BODY></HTML>",stdout);
             exit(1);
           }
        }

       mv_flags(gis) = setboardstyle(mv_flags(gis), (HTML_BOARD+UNIKANJI));

       if(*old_moves || *next_move)
        {
          if(!(moves_so_far = ((char *) malloc(strlen(old_moves)+strlen(next_move)+3))))
           {
             fprintf(stdout,
               "<TITLE>***Internal error: memory exhausted</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\">\n");
             fflush(stdout);
             fprintf(stdout,
               "Could not allocate a string of %d bytes, for the combination of:\n<BR>MOVES=", (strlen(old_moves)+strlen(next_move)+3));
             fputs(old_moves,stdout);
             fprintf(stdout,"\n<BR>NEXT=");
             fputs(next_move,stdout);
             fprintf(stdout,"\n<P></BODY></HTML>");
             exit(0);
           }
          else
           {
             strcpy(moves_so_far, old_moves);
             if(*old_moves) { strcat(moves_so_far,"/"); }
             strcat(moves_so_far, next_move);
           }


          if(ERRMOVES == (moves_made = mv_execute_moves(gis,moves_so_far,chkmsgbuf,errmsgbuf,escmsgbuf)))
           {
             fprintf(stdout,
               "<TITLE>The move %d given (%s) is erroneous!</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\">\n"
               "The %d. move (%s) in the movelist:\n<BR>\"<B>%s</B>\"\n<BR>is erroneous:\n<BR><PRE>%s</PRE>\n<P></BODY></HTML>",
               gis->g_movesmade,gis->g_latest_move,gis->g_movesmade,gis->g_latest_move,moves_so_far,errmsgbuf);
             exit(0);
           }

          fprintf(stdout,"<TITLE>Situation after %d moves, latest valid move %s</TITLE>\n", moves_made, gis->g_latest_move);

        }
       else
        {
          fprintf(stdout,"<TITLE>Initial Situation</TITLE>\n");
        }

       fprintf(stdout,
"<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n"
"<!-- Hide it! \n"
"var _AVAILABLE_SQUARES = \"\";\n"
"var _SOURCE_SQUARE = \"\";\n"
"var _PIECECODE = \"\";\n"
"function get_x_difference(dst,src)\n"
"{\n"
"      return(dst.charCodeAt(0) - src.charCodeAt(0));\n"
"}\n"
"function get_y(sqr)\n"
"{\n"
"      return(sqr.charCodeAt(1) - '0'.charCodeAt(0));\n"
"}\n"
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
"function ask_which_officer()\n"
"{\n"
"    if(confirm('Do you want to promote this pawn to queen or knight?'))\n"
"     {\n"
"       if(confirm('Do you want to promote this pawn to queen? (answering no means knight)'))\n"
"        { return('Q'); }\n"
"       else { return('N'); }\n"
"     }\n"
"    else\n"
"     {\n"
"       if(confirm('Do you want to promote this pawn to rook? (answering no means bishop)'))\n"
"        { return('R'); }\n"
"       else { return('B'); }\n"
"     }\n"
"}\n"
"function domove(piececode,square,freedoms,avail_squares)\n"
"{\n"
"   if('' == document.BOARDFORM.NEXT.value)\n"
"    {\n"
"      if(0 == freedoms)\n"
"       {\n"
"         alert(\"You cannot make a move starting from this square \"\n"
"               + square);\n"
"         _AVAILABLE_SQUARES = '';\n"
"         _SOURCE_SQUARE = '';\n"
"         _PIECECODE = '';\n"
"         document.BOARDFORM.NEXT.value = '';\n"
"         return(false);\n"
"       }\n"
"      else\n"
"       {\n"
"         alert(\"Piece \" + piececode + \" in square \" + square + \" has \" + freedoms + \" freedoms: \"\n"
"           + avail_squares);\n"
"         _AVAILABLE_SQUARES = avail_squares;\n"
"         _SOURCE_SQUARE = square;\n"
"         _PIECECODE = piececode;\n"
"         document.BOARDFORM.NEXT.value = piececode + square;\n"
"         return(true);\n"
"       }\n"
"    }\n"
"   else\n"
"    {\n"
"      if(member(square,_AVAILABLE_SQUARES.split(',')))\n"
"       { var dir = get_x_difference(square,_SOURCE_SQUARE);\n"
"         if(('K' == _PIECECODE.toUpperCase()) && (2 == Math.abs(dir)))\n"
"          {\n"
"            if(2 == dir) { document.BOARDFORM.NEXT.value = '0-0'; }\n"
"            else { document.BOARDFORM.NEXT.value = '0-0-0'; }\n"
"          }\n"
"         else if(('P' == _PIECECODE.toUpperCase()) && (' ' == piececode))\n"
"          {\n"
"            if(0 == dir)\n"
"             {\n"
"               document.BOARDFORM.NEXT.value\n"
"                = document.BOARDFORM.NEXT.value + '-' + square;\n"
"             }\n"
"            else // It's a capture.\n"
"             {\n"
"               document.BOARDFORM.NEXT.value\n"
"                = document.BOARDFORM.NEXT.value + 'x' + square;\n"
"             }\n"
"          }\n"
"         else if(' ' == piececode) // Just an ordinary move\n"
"          {\n"
"            document.BOARDFORM.NEXT.value\n"
"             = document.BOARDFORM.NEXT.value + '-' + square;\n"
"          }\n"
"         else // It's a capture.\n"
"          {\n"
"            document.BOARDFORM.NEXT.value\n"
"             = document.BOARDFORM.NEXT.value + 'x' + square;\n"
"          }\n"
"         if((('p' == _PIECECODE) && (8 == get_y(square)))\n"
"            || (('P' == _PIECECODE) && (1 == get_y(square))))\n"
"          {\n"
"            document.BOARDFORM.NEXT.value += '=' + ask_which_officer();\n"
"          }\n"
"         document.BOARDFORM.submit();\n"
"       }\n"
"      else\n"
"       {\n"
"         alert(\"Piece \" + _PIECECODE + \" in square \" + _SOURCE_SQUARE\n"
"               + \" cannot move to the square \"  + square)\n"
"         _AVAILABLE_SQUARES = '';\n"
"         _SOURCE_SQUARE = '';\n"
"         _PIECECODE = '';\n"
"         document.BOARDFORM.NEXT.value = '';\n"
"         return(false);\n"
"       }\n"
"    }\n"
"   return(true);\n"
"}\n"
"// --End Hiding Here -->\n"
"</SCRIPT>\n");

       fprintf(stdout, "</HEAD><BODY BGCOLOR=\"#ffffff\">\n"
                       "<FORM NAME=\"BOARDFORM\" ACTION=\"%s\"><TABLE>", progname);

       fprintf(stdout, "<TR><TH>Moves made:</TH><TD><INPUT TYPE=\"HIDDEN\" NAME=\"MOVES\" VALUE=\"%s\">%s</TD></TR>\n",
                    moves_so_far,moves_so_far);
       fprintf(stdout, "<TR><TH>Next move (%s's):</TH><TD><INPUT TYPE=\"TEXT\" NAME=\"NEXT\"></TD></TR>\n",
                    getcapcolname(get_parity(moves_made + !mv_shoogip(gis))));
       if(gis->g_movesmade)
        {
          fprintf(stdout,"<TR><TD COLSPAN=2><B>%s moved %s (%lu)</B></TDH></TR>",
                    getcapcolname(!get_parity(gis->g_movesmade + !mv_shoogip(gis))),
                    gis->g_latest_move,gis->g_movesmade);
        }
       if(*chkmsgbuf)
        {
          fprintf(stdout, "<TR><TD COLSPAN=2><B>%s's %s</B></TD></TR>\n",getcapcolname(get_parity(moves_made + !mv_shoogip(gis))),chkmsgbuf);
        }
       if(*escmsgbuf)
        {
          fprintf(stdout, "<TR><TD COLSPAN=2>(<EM>but %s %s</EM>)</TD></TR>\n",getcapcolname(get_parity(moves_made + !mv_shoogip(gis))),escmsgbuf);
        }       
       fputs("</TABLE><P><BR>\n",stdout);
       fflush(stdout);


        {
          int orientation = !mv_shoogip(gis); /* From Black's (= 0) perspective in Shoogi, from White's (= 1) in Chess. */
	  /* Except if other_flag specified, then change the orientation to be the currently playing player's one: */
	  if(oview)
           {
             orientation = get_parity(moves_made + !mv_shoogip(gis)); /* Black (= 0) starts in Shoogi, White (= 1) in Chess. */
             /* _orientation_ = !_orientation_; */
           }
	  mv_printboard(gis,orientation,stdout,mv_flags(gis));
        }
       fprintf(stdout, "</FORM></BODY></HTML>\n");
       exit(0);
     }


}








