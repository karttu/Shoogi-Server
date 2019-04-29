
#include "shoogi.h"

#ifdef CRIPPLED_TOUPPER

unsigned int mytoupper(c)
unsigned int c;
{
    if(((c) >= 'a') && ((c) <= 'z')) { return(c-('a'-'A')); }
    else { return(c); }
}

unsigned int mytolower(c)
unsigned int c;
{
    if(((c) >= 'A') && ((c) <= 'Z')) { return(c+('a'-'A')); }
    else { return(c); }
}

#endif


int takefirstword(result,buf)
char *result;
char *buf;
{
    char *p;

    p = skip_blankos(buf);
    if(!*p)
     {
       *buf = '\0';
       return(0);
     }
    while(*p && !isspace(*p)) { *result++ = *p++; }
    *result = '\0'; /* Terminate the result string. */
    strcpy(buf,p); /* Copy the rest of buf to beginning. */
    return(1);
}

char *getfirstword(result,buf)
char *result;
char *buf;
{
    char *p;

    p = skip_blankos(buf);
    if(!*p)
     {
       return(NULL);
     }

    while(*p && !isspace(*p)) { *result++ = *p++; }
    *result = '\0'; /* Terminate the result string. */
    return(p); /* Return rest of the buf. */
}


/* This takes a command line in bar and divides it to pieces which are
   put to null terminated vector foo. (Like argv argument of main).
   Returns the count of them. (Like argc).
 */
int hack_to_pieces(foo,bar)
register char **foo,*bar;
{
    register int count=0;

/* Okay, this would be nice if there would be a separate space allocated
    for the strings in foo:
    while(*(bar = skip_blankos(bar)))
     {
       while(*bar && !isspace(*bar)) { **foo++ = *bar++; }
       (* I hope that the following line is equal to **foo = '\0'; foo++; *)
       *(*foo++) = '\0';
       count++;
     }
 */
    /* Instead, we use the space in the bar, and overwrite the
       first blankos after non-blank-segements with ending zeros: */
    while(*(bar = skip_blankos(bar)))
     {
       count++;
       /* Set elements of foo to point to first non-blank-char: */
       *foo++ = bar;
       while(*bar && !isspace(*bar)) { bar++; } /* Skip the non-blanks */
       if(!*bar) { break; } /* If bar finished. */
       *bar++ = '\0';
     }

    *foo = NULL;
    return(count);
}

/* This concatenates the command line back together, starting from
    argument arg, and returns the resulting string (beginning from
    argument arg), or NULL if first arg is NULL.
   All string end zeros (except of the last one) are replaced by blankos,
   so if there were tabs instead between words, then result is not exactly
   the same as original command line.
 */
char *reconstruct_cmdline(arg)
register char **arg;
{
     register char *p;
     char *s;

     if(!*arg) { return(NULL); }
     s = *arg;

     while(*(arg+1))
      { /* Find the end of each argument word: */
        for(p=*arg; *p; p++) ;
	*p = ' '; /* And replace it by blanko. */
        arg++;
      }

     return(s);
}

char *skip_blankos(s)
register char *s;
{ /* On	some rotten systems is* macros don't work correctly with '\0' ! */
     while(*s && isspace(*s)) { s++; }
     return(s);
}

char *nuke_newline(s)
char *s;
{
    char *p;

    p = (s + strlen(s)); /* Get pointer pointing to the terminator zero. */
    /* If length of s was greater than zero, and there's newline as last
        character, then remove it: */
    if((p > s) && (*--p == '\n')) { *p = '\0'; }
    /* If there's also CR, then nuke it too: (Can happen with Turbo-C) */
    if((p > s) && (*--p == '\r')) { *p = '\0'; }

    return(s);
}


/* Convert string s to uppercase: */
char *conv_upper(s)
register char *s;
{
     char *orgs;

     orgs = s;

     while(*s = toupper(*s)) { s++; }
     return(orgs);
}


/* Convert string s so that scandinavian dotted letters {, } and |
    are replaced by corresponding letters without dots and rings
 */
char *conv_scands(s)
register char *s;
{
     unsigned char c;
     char *orgs;

     orgs = s;

     while(c = *s)
      {
        switch(c)
	 { /* A with dots and A with ring: */
	   case '[': case ']': { c = 'A'; break; }
	   case '{': case '}': { c = 'a'; break; }
	   case '\\': { c = 'O'; break; } /* O with dots. */
	   case '|':  { c = 'o'; break; }
	   case '^':  { c = 'U'; break; } /* U with dots. */
	   case '~':  { c = 'u'; break; }
	   default: { /* c = c; */ }
	 }
	*s++ = c;
      }
     return(orgs);
}

#ifdef This_is_fancy_but_we_dont_need_it_now

char *getcardinalsuffix(n)
MYINT n;
{
    switch(n)
     {
       case 1:  { return("st"); } /* First */
       case 2:  { return("nd"); } /* Second */
       case 3:  { return("rd"); } /* Third */
       default: { return("th"); } /* Fourth */
     }
}

#endif

int pertospc(s)
char *s;
{
    register char *p;
    register int count=0;

    /* Change all periods to spaces: */
    p = s;
    while((p = strchr(p,'.'))) { *p++ = ' '; count++; }
    return(count);
}


int spctoper(s)
char *s;
{
    register char *p;
    register int count=0;

    /* Change all spaces to periods: */
    p = s;
    while((p = strchr(p,' '))) { *p++ = '.'; count++; }
    return(count);
}


char *mystrdup(s)
char *s;
{
     register char *t;

     if(!(t = ((char *) malloc(strlen(s)+1))))
      {
        errlog((E1,
"**Internal error in mystrdup: memory exhausted, s=%u:\n%s\n",
               strlen(s),s));
        ertzu_exit(101);
      }

     strcpy(t,s); /* Copy string to new location. */
     return(t);
}


/*
   This copies to buffer z (and returns it) the date and time in form:
   10-MAY-92,20:20:22
 */
char *get_timebuf()
{
    char *ctime();
    long tloc;
    register char *p;
    static char z[21];

    time(&tloc);      /* Get the time to tloc. */
    p = ctime(&tloc); /* Convert it to ascii, standard unix form. */

    /* Then convert that to our own format: */

    z[0]  = p[8];          /* Day of month. */
    z[1]  = p[9];
    z[2]  = '-';
    z[3]  = p[4];          /* Month. */
    z[4]  = toupper(p[5]);
    z[5]  = toupper(p[6]);
    z[6]  = '-';
    p[24] = '\0';
    strcpy((z+7),(p+22));  /* Year, 2 digits. */
    z[9] = ',';
    p[19] = '\0';
    strcpy((z+10),(p+11)); /* Time, 8 characters. */
    /* Substitute zero for blank in day of month: */
    if(*z == ' ') { *z = '0'; }
    return(z);
}



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

   Pattern can contain following characters:

    ?   Matches one character which can be anything.
    *   Matches zero or more of any characters.

    <   Start of the "group-expression", which contains some chars,
         and must end in >
        If first char after < is ^ then its semantics are negated.
        If first char after < or ^ is > then it's not understood yet as end
         delimiter.
    Examples:
        <abc>          Matches any of the letters a, b and c.
        <^0123456789>  Matches anything, except digits.
        <>>            Matches >
        <^>>           Matches anything except >

    @   Matches character last matched to ? or group-expression.
         For example ?*@ matches to all strings which begin with same
         character they end.
        However, if pattern starts with @ then it sets the group
         start & end characters, e.g. pattern: @{tuu<ba{123}pasuuna
         matches to anything which begins tuu<ba then after that is
         1, 2 or 3 and after that pasuuna.

     Any other characters match just to themselves.

   Note that unix-like [0-9] (corresponding to <0123456789>) is not
   implemented yet.
*/


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  !FALSE
#endif


char GROUP_BEG_CHAR = '<';
char GROUP_END_CHAR = '>';

unsigned char *string_org;
unsigned char last_matched=0;


/* Check whether pattern and string match, and returns 0 if they not.
   If they match then return integer whose high byte is the last character
   matched to ? or <> expression (if any), and whose low byte is
   index +2 to string, to that point which matched to the first
   non-* and non-? expression.

   Note that value of last_matched is updated in double-recursive
   function, although it is static variable.
   Intuitively, this should cause some problems, but I haven't yet
   discovered any pattern which would match incorrectly.
 */
int wildcard(pattern,string)
unsigned char *pattern,*string;
{
    unsigned int wild_aux();
    unsigned int i;
    char negate_flag;

    last_matched = 0;
    string_org = string;

#ifdef COMMENTED_OUT
    if(*pattern == '@') /* Set group-expression delimiter characters. */
     { /* Set GROUP_BEG_CHAR to be char after @, and if it is not '\0' */
       if(GROUP_BEG_CHAR = *++pattern) { pattern++; } /* then skip it also. */
       GROUP_END_CHAR = get_end_delimiter(GROUP_BEG_CHAR);
     }
#endif

    if(*pattern == '!')
     {
       negate_flag = 1; pattern++;
     }
    else { negate_flag = 0; }

    i = wild_aux(pattern,string,1);
    if(negate_flag) { i = !i; }
    return(i ? ((last_matched << 8)|i) : 0);

}


unsigned int wild_aux(pattern,string,fnwc)
unsigned char *pattern,*string;
unsigned int fnwc; /* First Non-WildCard index */
{

loop:

    if(!*pattern && !*string) /* if BOTH are in the end */
     { return(fnwc); }
    /* Asterisk may match emptiness at the end of string: */
    if((*pattern == '*') && !*string)
     { pattern++; goto loop; }
    if(!*pattern || !*string) return(FALSE); /* If only OTHER is in the end */
    if(*pattern == GROUP_BEG_CHAR)
     {
       if(group_match(&pattern,&string)) { goto jalava; }
       else { return(FALSE); }
     }
    if(*pattern == '?') /* Question-mark in pattern ? */
     { pattern++; last_matched = *string++; goto loop; }
    if((*pattern == '@') && last_matched)
     {
       if(*string == last_matched) { goto silava; }
       else { goto sulava; }
     }
    if(*pattern == '*')
     {
       unsigned int muu,kuu;
       /* Save the value of last_matched at this level... */
       kuu = last_matched; /* if next level of recursion fucks it up. */
       if(muu = wild_aux(pattern,string+1,fnwc)) { return(muu); }
       last_matched = kuu; /* Restore value of last_matched at this level */ 
       return(wild_aux(pattern+1,string,fnwc));
#ifdef VANHA_PASKA
 /* It (*) matches several chars?: */
       return(wild_aux(pattern,string+1,fnwc)
               || /* Matches one character: (not really necessary)
              wild_aux(pattern+1,string+1,fnwc)
               ||  */             /* Or it matches 0 characters? */
              wild_aux(pattern+1,string,fnwc));
#endif
     }
    else sulava: if(*pattern == *string) /* Same characters ? */
     {
silava:
       pattern++; string++;
jalava:
       if(fnwc == 1) { fnwc = ((string - string_org)+1); }
       goto loop;
     }
    else { return(FALSE); }
}


int group_match(pat_adr,str_adr)
unsigned char **pat_adr,**str_adr;
{
        register unsigned char *pat;
        register unsigned char c,positive_flag;

        /* Take current char. from string, and advance string by one: */
        c = *(*str_adr)++;
        pat = (*pat_adr)+1; /* Skip group beginning char */

/* positive_flag is on if there is no negation-sign (^) in the beginning: */
        if(*pat == '^') { positive_flag = 0; pat++; }
        else { positive_flag = 1; }

        while(*pat)
         {
           if(*pat == c) /* If found c from the pattern. */
            { /* If group ending char not found, then return false: */
              if(!(pat = (unsigned char *) strchr((pat+1),GROUP_END_CHAR)))
	       { return(FALSE); }
              else
               {
                 /* Set pattern to point one after group_end_char: */
nakki:           *pat_adr = (pat+1);
                 if(positive_flag) /* Set last_matched char. */
                  { last_matched = c; }
                 return(positive_flag);
               }
            }
           if(*++pat == GROUP_END_CHAR)
            {
/* If there was negation-sign (^) in the beginning, meaning that
    positive_flag was 0, then set it to 1, and jump to nakki
    to return true result. Because we are here it means that
    c doesn't match to group, so if ^ in the beginning, then
    return true.
 */           /* He he hee hee hee, some sick code again: */
              positive_flag = !positive_flag;
              goto nakki;
            }
         }

        return(FALSE); /* If no group_ending_character */
}




GIDLIST consgl(gid,color,lista)
MYINT gid;
int color;
GIDLIST lista;
{
    register GIDLIST z;

    if(!(z = ((GIDLIST) malloc(sizeof(struct gid_list)))))
     {
       errlog((E1,
"**Internal error: cons(%lu,%d,lista): memory exhausted!\n",gid,color));
       ertzu_exit(101);
     }

    z->gid   = gid;
    z->color = color;
    z->next  = lista;
    return(z);
}

freegidlist(lista)
register GIDLIST lista;
{
    register GIDLIST cdr_of_lista;

    while(lista)
     {
       cdr_of_lista = cdr(lista);
       free(lista);
       lista = cdr_of_lista;
     }
}


HELP()
{
    FILE *fp;
    MYINT linecount;
    int arg_i,level,lev,i;
    char nonstop=0;
    char c;
    char buf[MAXBUF+3];

    /* Convert arguments to uppercase first: */
    for(arg_i=1; *(cmds+arg_i); ) { conv_upper(*(cmds+arg_i++)); }

    arg_i=1;
    linecount = _morelines_;

    if((cmdcnt > 1) && strequ(*(cmds+1),"-N"))
     {
       arg_i++;
       cmdcnt--;
       nonstop=1;
     }

    if(cmdcnt < 2) /* If no topic specified then list HELP HELP */
     {
       *(cmds+arg_i)   = "HELP";
       *(cmds+arg_i+1) = NULL;
       cmdcnt++;
     }

    if(!(fp = myfopen(HELPFILE,"r")))
     { return(0); }

    if(strequ(*(cmds+arg_i),"*"))
     { help_star(fp,nonstop); return(1); }

    while(fgets(buf,MAXBUF,fp))
     {
alku:
       if((*buf == '-') && (level = match_topic(buf,arg_i)))
        {
	  if(!nonstop && !--linecount)
	   {
	     c = tolower(more("--More-- <y,n,<space>,<cr>,q>"));
             if(c == 'n') { linecount= _morelines_; continue; }
             else if(c == 'q') { break; }
             else if((c == ' ') || (c == 'y')) { linecount = _morelines_; }
             else { linecount = 1; }
	   }

	  for(i=level; i--; ) { pout((" ")); } /* Print dashes (-) */
	  pout((buf+level));           /* as blanks. */
	  while(fgets(buf,MAXBUF,fp))
	   {
	     if((level >= (lev = getlevel(buf))) && lev)
	      { goto alku; } /* If new topic which is superior to this. */
	     if(!nonstop && !--linecount)
	      {
	        c = tolower(more("--More-- <y,n,<space>,<cr>,q>"));
		if(c == 'n') { linecount = _morelines_; break; }
		else if(c == 'q') { goto ulos; }
		else if((c == ' ') || (c == 'y')) { linecount = _morelines_; }
		else { linecount = 1; }
	      }

	     if((*buf == '\f') && !nonstop) /* If CTRL-L encountered, */
	      { /* Then ask more for next line, if not exited before that */
	        linecount=1;
	      }
	     for(i=lev; i--; ) { pout((" ")); }
	     pout((buf+lev));
	   }
        }
     }

ulos:
    fclose(fp);
    return(1);
}


help_star(fp,nonstop)
FILE *fp;
int nonstop;
{
    MYINT linecount;
    int i,level;
    char *s;
    char buf[MAXBUF+3];

    linecount = _morelines_;

    while(fgets(buf,MAXBUF,fp))
     {
       if(level = getlevel(buf))
        {
	  s = skip_blankos(buf+level);
	  if(!s) { continue; }
	  if(!nonstop && !--linecount)
	   {
	     char c;
	     c = tolower(more("--More-- <y,n,<space>,<cr>,q>"));
	     if((c == 'n') || (c == 'q')) { break; }
	     else if((c == ' ') || (c == 'y')) { linecount = _morelines_; }
	     else { linecount = 1; }
	   }
	  for(i=level; i--; ) { pout((" ")); }
	  pout((s));
        }
     }

    fclose(fp);
}



int match_topic(s,arg_i)
register char *s;
int arg_i;
{
    register char *next;
    int level,matched=0;

    level = getlevel(s);

    conv_upper(s);
    s = skip_blankos(s+level);

    while(s && *s)
     {
       if(next = strchr(s,','))
        { *next = '\0'; }
       matched = match1_aux(s,arg_i);
       if(next) { *next = ','; s = skip_blankos(next+1); }
       else { s = NULL; }
       if(matched) { return(level); }
     }

    return(0);
}


int match1_aux(s,arg_i)
register char *s;
register int arg_i;
{
    char wordbuf[MAXBUF+3];

    /* Loop so long as there is words in s: */
    do
     {
       if(!*(cmds+arg_i)) { break; }
       if(match2_aux(s,arg_i)) { return(1); }
     } while((s = getfirstword(wordbuf,s)));
    return(0);
}


int match2_aux(s,arg_i)
char *s;
register int arg_i;
{
    char wordbuf[MAXBUF+3];

    /* Loop through command line arguments: */
    while(*(cmds+arg_i))
     {
       /* However, if no more words in s, then it's false: */
       if(!(s = getfirstword(wordbuf,s))) { return(0); }
       /* Or if differing word found: */
       if(!strequ(*(cmds+arg_i),wordbuf)) { return(0); }
       arg_i++;
     }

    return(1);
}


int getlevel(s)
register char *s;
{
    register int level=0;

    while(*s++ == '-') { level++; }

    return(level);
}


int morefile(filename,nonstop)
char *filename;
int nonstop;
{
    MYINT linecount;
    FILE *fp;
    char buf[MAXBUF+3];

    if(!(fp = myfopen(filename,"Sr")))
     { return(0); }

    linecount = _morelines_;

    while(fgets(buf,MAXBUF,fp))
     {
       if(!nonstop && !--linecount)
        {
          char c;
          c = tolower(more("--More-- <y,n,<space>,<cr>,q>"));
          if((c == 'n') || (c == 'q')) { break; }
          else if((c == ' ') || (c == 'y')) { linecount = _morelines_; }
          else { linecount = 1; }
	}
       pout((buf));
     }

    fclose(fp);

    return(1);
}


int ynq_aux(prompt,attr)
char *prompt;
int attr;
{
    int c,sig;

/* First check if any new signals have been received: */
looop:
    if(sig = _sig_received_)
     {
       _sig_received_ = 0;
       pout(("\n"));
       switch(sig)
        {
          case SIGINT:
	   {
	     pout(("?Please use the command QUIT for quitting!\n"));
	     break;
	   }
/*	  case SIG_GAME_STARTED: */
	  case SIG_GAME_FINISHED: { readgidlist(); }
/*        case SIG_MESSAGE_SENT: { checkmessages(); } */
          case SIG_MOVE_MADE: { checkmessages(); checknewmoves(0); }
	}
     }

#ifdef CURSES

    if(attr) { attron(attr); }
    pout((prompt));
    if(attr) { attroff(attr); }

    /* Some curses functions: */
    cbreak();    /* Return character immediately, without waiting CR. */
    noecho();    /* Do not echo it. */
    /* When waiting for input signals are acknowledged immediately: */
    _sig_immediately_ = 1;
    if(_sig_received_) { _sig_immediately_ = 0; goto looop; }
    c = getch(); /* Get the character. */
    _sig_immediately_ = 0;
    if(c == EOF) { goto looop; }
    echo();      /* Then put echo, and */
    nocbreak();  /* cooked mode back. */
/*  fout((OB,"Debugging: c = %u.\n",c)); */

    if(!attr) { pout(("\n")); }

    return(c);
#else

    if(attr) { fout((OB,"\033[%dm",attr)); }
    pout((prompt));
    if(attr) { fout((OB,"\033[0m")); } /* Clear video modes. */

    /* When waiting for input signals are acknowledged immediately: */
    _sig_immediately_ = 1;
    if(_sig_received_) { _sig_immediately_ = 0; goto looop; }
    c = getch(); /* Get the character. */
    _sig_immediately_ = 0;
    if(c == EOF) { goto looop; }

    if(!attr) { pout(("\n")); } /* Some ugly kludge I guess? */

    return(c);

/* When waiting for input signals are acknowledged immediately:
   (some old code, commented out):
    _sig_immediately_ = 1;
    if(_sig_received_) { _sig_immediately_ = 0; goto looop; }
    if(!fgets(input_buf,MAXBUF,input))
     {
       _sig_immediately_ = 0;
       if(feof(input)) { return(0); }
       goto looop;
     }
    _sig_immediately_ = 0;
    return(tolower(*input_buf));
 */
#endif
}

int more(prompt)
char *prompt;
{
    int len;
    char c;
    char blankos[MAXBUF+3];

#ifdef CURSES
    c = ynq_aux(prompt,A_REVERSE);
    memset(blankos,' ',(len = strlen(prompt)));
    blankos[len] = '\0'; /* And terminate. */
    fout((OB,"\r%s\r",blankos)); /* Overwrite the prompt string. */
#else
/*  c = ynq(prompt); Old code, replaced by this: */
    c = ynq_aux(prompt,ANSI_REVERSE);
    memset(blankos,' ',(len = strlen(prompt)));
    blankos[len] = '\0'; /* And terminate. */
    fout((OB,"\r%s\r",blankos)); /* Overwrite the prompt string. */
    
#endif
    return(c);
}



print_err(s,to_user_too)
char *s;
int to_user_too;
{
    if(to_user_too) { pout((s)); }
    fprintf(err_fp,"%s %lu %s %d %lu %lu %lu %lu %s",
     get_timebuf(),_ownplid_,_ownname_,_owncolor_,_gid_,_movesmade_,
      _blackplid_,_whiteplid_,
      s);
    fflush(err_fp);
}

init_screen()
{
#ifdef CURSES
    int i;

    initscr(); /* For curses. */
    start_color();
    scrollok(stdscr,TRUE);
    idlok(stdscr,TRUE);
    _morelines_ = LINES-1; /* LINES is curses variable. */
    for(i=COLOR_BLACK; i <= COLOR_WHITE; i++)
     { /* Initialize eight color pairs, so that background color is
           always black (zero): */
       init_pair((i+1),i,0);
     }
#endif
}

end_screen()
{
#ifdef CURSES
    endwin(); /* For curses. */
#endif
}


/* Like fseek, but with error checking: */
int myfseek(fp,offset,origin,funname)
FILE *fp;
register long offset;
register int origin;
char *funname;
{
    int fseek();
    register int result;
        
    if(result = fseek(fp,offset,origin))
     {
       errlog((E1,
"**Internal error in myfseek(fp,%ld,%d,%s) -> %d, errno=%d\n",
          offset,origin,funname,result,errno));
       ertzu_exit(101);
     }
    return(result);
}


myrewind(fp,funname)
FILE *fp;
char *funname;
{
    int status;

    if((status = fseek(fp,0L,0))) /* Try to rewind fp to beginning. */
     {
       errlog((E1,
"**Internal error in myrewind (called from %s): fseek=%d  errno=%d\n",
                 funname,status,errno));
       ertzu_exit(101);
     }
}


int myunlink(filename,funname)
char *filename,*funname;
{
    int status;

    filename = get_whole_filename(filename);

    if((status = unlink(filename))) /* Try to delete filename. */
     {
       errlog((E1,
"**Internal error in myunlink(%s,%s)=%d  errno=%d\n",
                 filename,funname,status,errno));
     }
    return(status);
}


FILE *myfopen(filename,mode)
char *filename,*mode;
{
    FILE *fp;
    char silent=0;
#ifdef TURBOC
    char newmode[11];

    strcpy(newmode,mode);
    strcat(newmode,"b"); /* Turbo-C wants that b as binary mode. */
    mode = newmode;
#endif

    filename = get_whole_filename(filename);

    if(toupper(*mode) == 'S')
     {
       silent=1;
       mode++;
     }

    if(!(fp = fopen(filename,mode)))
     {
       if(!silent)
        {
	  errlog((E1,"**Can't open file %s with the mode \"%s\" !\n",
                     filename,mode));
        }
       return(NULL);
     }
    else { return(fp); }
}


#ifdef NOF_LOCK
FILE *lockfopen(char *filename,char *mode)
{ return(myfopen(filename,(isupper(*mode) ? (mode+1) : mode))); }
int lockfclose(FILE *fp) { return(fclose(fp)); }

#else

FILE *lockfopen(filename,mode)
char *filename,*mode;
{
    int i;
    char silent=0,dont_sleep=0;
    FILE *fp;
#ifdef TURBOC
    char newmode[11];

    strcpy(newmode,mode);
    strcat(newmode,"b"); /* Turbo-C wants that b as binary mode. */
    mode = newmode;
#endif

    filename = get_whole_filename(filename);

    if(toupper(*mode) == 'S')
     {
       silent=1;
       mode++;
     }
    else if(toupper(*mode) == 'T')
     {
       silent=1;
       dont_sleep=1;
       mode++;
     }

    for(i=0; i < MAX_ATTEMPTS; i++)
     {
       if((fp = fopen(filename,mode)))
        {
#ifdef UNIX
          int status;

try_again:
          if((status = lockf(fileno(fp),F_LOCK,0)))
	   {
	     if(_sig_received_ == SIGINT)
	      {
	        fclose(fp);
	        errlog((E1,
"\n**lockfopen(%s,%s) interrupted while waiting for file to be unlocked.\n",
                    filename,mode));
                return(NULL);
	      }
	     else if(_sig_received_) { goto try_again; }

	     fclose(fp);
	     errlog((E1,
"\n**lockfopen(%s,%s) couldn't lock a file, try again later!\n",
                    filename,mode));
             errlog((E1,"status=%d  errno=%d  fileno(fp)=%u\n",
                    status,errno,fileno(fp)));
	     return(NULL);
	   }
#endif
	  return(fp);
	}
       else if(dont_sleep) { break; }
       else { sleep(1); } /* Sleep one second, and then retry. */
     }

    if(!silent)
     {
       errlog((E1,
       "**Can't open file %s with the mode \"%s\" (after %u attempts)\n",
             filename,mode,i));
     }
    return(NULL);
}


/* Release locks associated with fp and close it: */
int lockfclose(fp)
FILE *fp;
{
#ifdef UNIX
    int status;

    fflush(fp); /* Flush it first. */
    /* Then try to rewind it back to beginning: */
    myrewind(fp,"lockfclose");

    if((status = lockf(fileno(fp),F_ULOCK,0)))
     {
       errlog((E1,
"**Internal error in lockfclose: can't unlock! status=%d  errno=%d\n",
          status,errno));
       ertzu_exit(101);
     }
#endif

    return(fclose(fp));
}

#endif


char *get_whole_filename(name)
char *name;
{
    static char filnambuf[MAXBUF+3];

    strcpy(filnambuf,SHOOGI_DIRECTORY);
    strcat(filnambuf,name);
    return(filnambuf);
}


handle_options()
{
    char c,*s,**argp;

    argp = G_argv;

    while(s = *++argp)
     {
       if(*s == '-') /* Options? */
	{
          while(c = *++s)
	   {
             switch(c)
	      {
	        case 'r': { _restricted_flag_ = 1; break; }
                default:
		 {
                   fprintf(stderr,"\n%s: invalid option: -%c !\n",
                            progname,c);
                   exit(1);
		 }
	      }
	   }
	}
       else /* Player's name given? */
	{
/*        _tentative_name_ = s; */
	}
     }
}


char *getname()
{
    char *s,*t;

#ifdef UNIX
    struct passwd *p;

    if((_uid_ = getuid()) == PLAYER_UID)
     {
#endif
       if(!(s = getenv("LUSER")))
        {
          fprintf(stderr,
"\n%s: Fatal internal error, environment variable LUSER not defined!\n",
           progname);
	  fprintf(stderr,"(It should be in the form: firstname.lastname)\n");
	  exit(1);
	}
#ifdef UNIX
       if(!(strchr(s,'.'))) /* If no dot in the pattern. */
	{
          if((p = getpwnam(s))) /* Uh huh, sanoi Uppo Nalle, an ugly fix! */
           { goto raketti_spagettia; } /* If found login name... */
        }
     }
    else
     {
       if(!(s = getlogin()))
        { /* If couldn't find with getlogin, then try with getpwuid: */
	  if(!(p = getpwuid(_uid_)))
	   {
ertzu:
             fprintf(stderr,
"\n%s: Fatal internal error, can't find login name for you!\n",
           progname);
	     exit(1);
	   }
	  else { s = p->pw_gecos; }
	}
       else
        {
          if(!(p = getpwnam(s))) { goto ertzu; }
          else
           {
raketti_spagettia:
             s = p->pw_gecos;
           }
        }

       /* Cut the GECOS field at comma (if there's any), so that we
           get the real name: */
       if((t = strchr(s,','))) { *t = '\0'; }
     }

#endif

    s = mystrdup(s);

    spctoper(s);    /* Spaces to periods. */
 /* And convert scandinavian letters to corresponding letters without dots: */
    conv_scands(s);
    conv_upper(s);  /* And to uppercase. */

    _su_flag_ = (
#ifdef UNIX
                 (getgid() == MASTER_GID) ||
#endif
                 strequ(s,SUPERUSER1));

    /* If player is master user (or we are in MS-DOS), and
       there is command line argument -nnew.name, then new.name
       is used instead of default name associated with user.
       This is used for testing purposes.
     */
    if(
#ifdef UNIX
       _su_flag_ &&
#endif
       (G_argc > 1) && cutstrequ(*(G_argv+1),"-n"))
     {
       s = mystrdup(2+*(G_argv+1));
       if(!*s)
        {
	  fprintf(stderr,"%s: Illegal name!\n",progname);
	  exit(1);
	}
       spctoper(s);    /* Spaces to periods. */
       conv_scands(s); /* And convert scandinavian letters. */
       conv_upper(s);  /* And to uppercase. */
     }
       
    if(strlen(s) > (SMALLBUF-2))
     {
       s[SMALLBUF-2] = '\0'; /* Cut it so that it fits to SMALLBUF. */
     }

    return(s);
}


int restore_real_uid(euidp,egidp)
int *euidp,*egidp;
{
#ifdef OLD_CODE_COMMENTED_OUT
#ifdef UNIX
    if((_uid_ = getuid()) == PLAYER_UID)
#else
    if(!_su_flag_) /* With MS-DOS */
#endif
#endif

    if(_restricted_flag_)
     {
       pout(("?Sorry, you cannot do that with this user id!\n"));
       return(0);
     }

#ifdef UNIX
    /* Save the effective uid. (= uid of executable file). */
    *euidp = geteuid();
    setuid(_uid_);    /* Restore real uid. */
    *egidp = getegid(); /* Same for effective gid. */
    setgid(getgid());
#endif
    return(1);
}


int _restore_effective_uid(euid,egid)
int euid,egid;
{   /* Restore the effective uid & gid of the executable file: */
#ifdef UNIX
    setuid(euid);
    setgid(egid);
#endif
}


#define getptrtopstruct(piece) ((whitep(piece) && (piece == makewhite(KING)))\
? &true_king : &piecenames[color_off(piece)])

/* 21 should be enough. 3 + 4 + 3 + 1 = 11. (or 4+3+2+3+4+1 = 17) */
static char pnamebuf[21];

char *getpiecename(piece)
int piece;
{
    struct piecename *ptr;

    ptr = getptrtopstruct(piece);

    switch(getpnamestyle(_flags_))
     {
       case ENGLISH: { return(ptr->english); }
       case LONG_ROMAJI:
        { return(convert_ous(pnamebuf,ptr->long_romaji));  }
       case SHORT_ROMAJI:
        { return(convert_ous(pnamebuf,ptr->short_romaji)); }
       case LONG_KANJI:
	{ return((char *) convert_jis(pnamebuf,ptr->long_kanji)); }
       case SHORT_KANJI:
        { return((char *) convert_jis(pnamebuf,ptr->short_kanji)); }
     }
}




char *getpieceshort(dst,piece)
UBYTE *dst;
int piece;
{
    int c;
    char tmpbuf[3];

    *dst = 0; /* Initialize this to be empty. */

    switch(getboardstyle(_flags_))
     {
       case ASCII:
        {
          c = letters[unpromote(color_off(piece))];
          dst[0] = (promotedp(piece) ? '+' : ' ');
          dst[1] = (whitep(piece) ? tolower(c) : c);
	  dst[2] = '\0';
	  break;
	}
       case MIXED:
        {
	  if(whitep(piece))
	   {
             c = letters[color_off(piece)];
             tmpbuf[0] = '#'; /* # plus ascii letter = JIS romaji letter. */
/* King and promoted pieces in uppercase, other ones in lowercase: */
             tmpbuf[1] =
	      ((promotedp(piece) || (color_off(piece) == KING)) ? c
	                                                        : tolower(c));
	     tmpbuf[2] = '\0';
	     convert_jis(dst,tmpbuf);
	     break;
	   }
	  else { piece = color_off(piece); } /* I.e. make it black. */
	  /* And fall through... */
	}
       case KANJI:
        {
          convert_jis(dst,getptrtopstruct(piece)->short_kanji);
	  break;
	}
       case UNIKANJI: case (UNIKANJI+HTML_BOARD):
        {
          sprintf(dst,"%s&#%u;%s",
              (((1 == 2) && whitep(piece)) ? "<FONT COLOR=\"red\">" : ""),
               getptrtopstruct((piece))->short_unikanji,
              (((1 == 2) && whitep(piece)) ? "</FONT>" : ""));
	  break;
	}
     }

    return((char *) dst);
}


/* If num is outside of 0-20 then the dst buf will be filled with garbage.
   Returns 1 or 2 depending whether one or two kanji are required.
 */
int getkanjinum(dst,num)
char *dst;
int num;
{
    int cnt;

    *dst = '\0'; /* Initialize this to be empty. */

    cnt = 1;
    if(num > 10)
     {
       if(getboardstyle(_flags_) == (UNIKANJI + HTML_BOARD))
        {
          sprintf(dst,"&#%u;", Unumbers[10]);
        }
       else { strcpy(dst,Jnumbers[10]); }
       num -= 10; cnt++;
     }

    if(getboardstyle(_flags_) == (UNIKANJI + HTML_BOARD))
     {
       dst += strlen(dst);
       sprintf(dst,"&#%u;", Unumbers[num]);
     }
    else strcat(dst,Jnumbers[num]);
    return(cnt);
}



UBYTE *convert_jis(dst,src)
UBYTE *dst,*src;
{
    UBYTE *org_dst;
    
    org_dst = dst;
    if(getkanjicode(_flags_) == JIS)
     {
       strcpy(dst,JIN);
       dst += strlen(dst);  
     }

    while(*src)
     { convjis(dst,dst+1,*src,*(src+1)); dst += 2; src += 2; }
    *dst = '\0';

    if(getkanjicode(_flags_) == JIS)
     {
       strcat(dst,JOUT);
     }

    return(org_dst);
}


convjis(leftdst,rightdst,leftsrc,rightsrc)
UBYTE *leftdst,*rightdst;
UBYTE leftsrc,rightsrc;
{
    /* N5 is simplified Japanese version of dragon (RYUU),
         and N6 is the original old one: */
    if(getdragonflag(_flags_) && (leftsrc == 'N') && (rightsrc == '5'))
     { rightsrc++; }

    switch(getkanjicode(_flags_))
     {
       case NO_KANJI: case EUC:
        { /* Just set eighth bit on: */
	  *leftdst  = (leftsrc  | 0x80);
	  *rightdst = (rightsrc | 0x80);
	  break;
	}
       case SHJ:
        { /* If Shift-JIS then do some contrived computations: */
          if(leftsrc & 1) { rightsrc += 0x1f; }
          else { rightsrc += 0x7d; }
          if(rightsrc >= 0x7f) { rightsrc++; }
          leftsrc = (leftsrc - 0x21 >> 1) + 0x81;
          if(leftsrc > 0x9f) { leftsrc += 0x40; }
          /* and fall through down */
	}
       case JIS:
        { /* Keep them intact: */
	  *leftdst  = leftsrc;
	  *rightdst = rightsrc;
	  break;
	}
     }
}


/* Convert all 'OU's to 'OO's, regardless of the case, and also convert
    the whole stuff to uppercase: */
char *convert_ous(dst,src)
UBYTE *dst,*src;
{
    char *org_dst;
    
    org_dst = ((char *) dst);

    while(*dst++ = toupper(*src++))
     { /* If the preceding letter was 'o' and this one is 'u': */
       if((tolower(*(src-1)) == 'o') && (tolower(*src) == 'u'))
        {
       /* *dst++ = (isupper(*src) ? 'O' : 'o'); Commented out. */
          *dst++ = 'O'; /* Then change it to 'O'. */
	  src++;
	}
     }

    *dst = '\0';

    return(org_dst);
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

#ifdef CURSES
    if(fp == stdout)
     {
       while(*buf)
        {
	  if(iscodeletter(*buf))
	   {
	     chtype attr;
	     attr = curses_codes[getmodecode(*buf)];
	     if(attr == A_NORMAL) { attrset(attr); }
	     else { attron(attr); }
	   }
	  else if(*buf == '\033') /* ESC? */
           {
             if((*(buf+1) == '$') && /* JIS in sequence? */
                ((*(buf+2) == 'B') || (*(buf+2) == '@')))
              { attron(A_ALTCHARSET); buf += 2; }
/*            { goto direct_out; }  Commented out. */
             if((*(buf+1) == '(') && (*(buf+2) == 'J')) /* JIS out seq? */
              { attroff(A_ALTCHARSET); buf += 2; }
#ifdef COMMENTED_OUT /* Doesn't seem to work properly. */
              {
direct_out: /* Output JIS-in & out-sequences directly, not via curses: */
                   refresh(); /* but first flush curses stuff. */
                   putc(*buf,fp);
                   putc(*(buf+1),fp);
                   putc(*(buf+2),fp);
                   fflush(fp); /* Maybe this is needed? */
                   buf += 2;
              }
#endif
           }
	  else { addch(*buf); }
	  buf++;
	}
       refresh();
     }
    else
#endif
     {
       while(*buf)
        {
	  if(iscodeletter(*buf))
	   {
	     if(!p) { p = codebuf; }
	     else { *p++ = ';'; }
	     sprintf(p,"%u",ansi_codes[getmodecode(*buf)]);
	     p += strlen(p);
           }
	  else
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

