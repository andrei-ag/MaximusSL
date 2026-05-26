#ifndef __ST_USER__
#define __ST_USER__

#include "common.h"

struct _stamp   /* DOS-style datestamp */
{
  struct
  {
    word da : 5;
    word mo : 4;
    word yr : 7;
  } date;

  struct
  {
    word ss : 5;
    word mm : 6;
    word hh : 5;
  } time;
};


struct _dos_st
{
  word date;
  word time;
};

/* Union so we can access stamp as "int" or by individual components */

union stamp_combo
{
  dword ldate;
  struct _stamp msg_st;
  struct _dos_st dos_st;
};

typedef union stamp_combo SCOMBO;

struct   _usr
   {
      byte name[36];        /* Caller's name                               0*/
      byte city[36];        /* Caller's location                          36*/

      byte alias[21];       /* MAX: user's alias (handle)                 72*/
      byte phone[15];       /* MAX: user's phone number                   93*/

      word lastread_ptr;    /* MAX: a num which points to offset in      108*/
                            /* file -- Offset of lastread pointer will be   */
                            /* lastread_ptr*sizeof(int).                    */

      word timeremaining;   /* MAX: time left for current call (xtern pro110*/

      byte pwd[16];         /* Password                                  112*/
      word times;           /* Number of previous calls to this system   128*/
      byte help;            /* Help level                                130*/
      word group;           /* Group number (not implemented)            131*/
      byte video;           /* user's video mode (see GRAPH_XXXX)        133*/
      byte nulls;           /* Number of Nulls (delays) after <cr>       134*/

      byte bits;            /* Bit flags for user (number 1)             135*/

      word dob_year;        /* Date of birth: year (1900-)               136*/

      word bits2;           /* Bit flags for user (number 2)             138*/

      word max2priv;        /* Max 2.x priv level (NOT USED)             140*/
      char dataphone[19];   /* Data/business phone number                142*/
      byte struct_len;      /* len of struct, divided by 20. SEE ABOVE!  161*/
      word time;            /* Time on-line so far today                 162*/

      word delflag;         /* Used to hold baud rate for O)utside comman164*/
                            /* In USER.BBS, usr.flag uses the constants     */
                            /* UFLAG_xxx, defined earlier in this file.     */
      
      dword msgs_posted;    /* Total number of messages posted           166*/
      dword msgs_read;      /* Total number of messages read             170*/

      byte width;           /* Width of the caller's screen              174*/
      byte len;             /* Height of the caller's screen             175*/
      word credit;          /* Matrix credit, in cents                   176*/
      word debit;           /* Current matrix debit, in cents            178*/

      word xp_priv;         /* Priv to demote to, when time or minutes ru180*/
                            /* out.                                         */

      union stamp_combo xp_date;  /* Bit-mapped date of when user expires182*/
                                  /* If zero, then no expiry date.          */
 
      dword xp_mins;        /* How many minutes the user has left before 186*
                             * expiring.                                    */

      byte  xp_flag;        /* Flags for expiry.  See above XFLAG_XXX def190*/
      byte  sex;            /* Sex: SEX_MALE or SEX_FEMALE)              191*/

      union stamp_combo ludate;   /* Bit-mapped date of user's last call 192*/

      dword xkeys;          /* User's keys (all 32 of 'em)               196*/
      byte  lang;           /* The user's current language #             200*/
      sbyte def_proto;      /* Default file-transfer protocol            201*/

      dword up;             /* K-bytes uploaded, all calls               202*/
      dword down;           /* K-bytes downloaded, all calls             206*/
      sdword downtoday;     /* K-bytes downloaded, today                 210*/

      byte rsvd45[18];      /* Reserved by Maximus for future use        214*/

      word call;            /* Number of previous calls today            232*/

      byte compress;        /* Default compression program to use        234*/

      byte df_save;         /* Used for storing real usr.delflag in      235*
                             * LASTUSxx.BBS while in a door.                */

      dword extra;                                                     /*236*/
      SCOMBO date_1stcall;  /* Date of first call to system              240*/
      SCOMBO date_pwd_chg;  /* Date of last password change              244*/

      dword nup;            /* Number of files uploaded                  248*/
      dword ndown;          /* Number of files downloaded                252*/
      sdword ndowntoday;    /* Number of files downloaded today          256*/

      word time_added;      /* Time credited to the user for today       260*/
      byte msg[64];   /* Current message area                      262*/
      byte files[64]; /* Current file area                         326*/

      byte dob_day;         /* Date of birth: day (1-31)                 390*/
      byte dob_month;       /* Date of birth: month (1-12)               391*/
      dword point_credit;   /* Total points allocated                    392*/
      dword point_debit;    /* Total points used                         396*/
      SCOMBO date_newfile;  /* Date of last new-files check              400*/
      word priv;            /* Privilege level                           404*/
      byte rsvd6[54];       /* Reserved for future use                   406*/
                                                                       /*460*/
   };
#endif