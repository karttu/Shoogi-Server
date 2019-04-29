
#include "webboard.h"

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
#ifdef CURSES_RELATED_CODE_COMMENTED_OUT
	  if(iscodeletter(*buf))
	   {
	     if(!p) { p = codebuf; }
	     else { *p++ = ';'; }
	     sprintf(p,"%u",ansi_codes[getmodecode(*buf)]);
	     p += strlen(p);
           }
	  else
#endif
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

