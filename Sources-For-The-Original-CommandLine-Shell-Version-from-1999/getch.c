
#include <stdio.h>
#include <string.h>
#if defined(SYSV) || defined(LINUX)
#include <termio.h>
#else
#include <sgtty.h>
#endif

/*
   Getkey.c

   Written 01/16/1991 by John Gordon, adapted from code supplied by Tim Evans

   The purpose of this program is to set ioctl to "raw" mode, read a keypress,
   reset the ioctl mode, and return the character read.

   History:
   01/16/91 by John Gordon - initial version adapted from Tim's code

   Name changed to getch by Antti Karttunen.
*/

#if defined(SYSV) || defined(LINUX)
#define GET_PARAMETERS TCGETA
#define SET_PARAMETERS TCSETA
#else /* For BSD: */
#define GET_PARAMETERS TIOCGETP
#define SET_PARAMETERS TIOCSETP
#endif


int getch()
{
  int c;
  struct
#if defined(SYSV) || defined(LINUX)
          termio
#else /* BSD */
          sgttyb
#endif
                 tsav, tchg;

  if (ioctl(0, GET_PARAMETERS, &tsav) == -1)
  {
    perror("getkey: can't get original settings");
    exit(2);
  }

  tchg = tsav;

#if defined(SYSV) || defined(LINUX)
/*  tchg.c_lflag &= ~(ICANON | ECHO);  Original */
  tchg.c_lflag &= ~(ICANON | ISIG | ECHO);
  tchg.c_cc[VMIN] = 1;
  tchg.c_cc[VTIME] = 0;
#else /* BSD */
  tchg.sg_flags = CBREAK;
#endif

  if (ioctl(0, SET_PARAMETERS, &tchg) == -1)
  { 
    perror("getkey: can't initiate new settings");
    exit (3);
  }

  c = getchar();

  if(ioctl( 0, SET_PARAMETERS, &tsav) == -1)
  {
    perror("getkey: can't reset original settings");
    exit(4);
  }

  return(c);

} /* end getkey() */

