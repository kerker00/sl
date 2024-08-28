/*========================================
 *    sl.c: SL version 5.03
 *        Copyright 1993,1998,2014-2015
 *                  Toyoda Masashi
 *                  (mtoyoda@acm.org)
 *        Last Modified: 2014/06/03
 *========================================
 */
/* sl version 6.00 : Inlude many MR from Github and added ICE                */
/*                                              by Markus Müller    2024/08/26 */
/* sl version 5.03 : Fix some more compiler warnings.                        */
/*                                              by Ryan Jacobs    2015/01/19 */
/* sl version 5.02 : Fix compiler warnings.                                  */
/*                                              by Jeff Schwab    2014/06/03 */
/* sl version 5.01 : removed cursor and handling of IO                       */
/*                                              by Chris Seymour  2014/01/03 */
/* sl version 5.00 : add -c option                                           */
/*                                              by Toyoda Masashi 2013/05/05 */
/* sl version 4.00 : add C51, usleep(40000)                                  */
/*                                              by Toyoda Masashi 2002/12/31 */
/* sl version 3.03 : add usleep(20000)                                       */
/*                                              by Toyoda Masashi 1998/07/22 */
/* sl version 3.02 : D51 flies! Change options.                              */
/*                                              by Toyoda Masashi 1993/01/19 */
/* sl version 3.01 : Wheel turns smoother                                    */
/*                                              by Toyoda Masashi 1992/12/25 */
/* sl version 3.00 : Add d(D51) option                                       */
/*                                              by Toyoda Masashi 1992/12/24 */
/* sl version 2.02 : Bug fixed.(dust remains in screen)                      */
/*                                              by Toyoda Masashi 1992/12/17 */
/* sl version 2.01 : Smoke run and disappear.                                */
/*                   Change '-a' to accident option.                         */
/*                                              by Toyoda Masashi 1992/12/16 */
/* sl version 2.00 : Add a(all),l(long),F(Fly!) options.                     */
/*                                              by Toyoda Masashi 1992/12/15 */
/* sl version 1.02 : Add turning wheel.                                      */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.01 : Add more complex smoke.                                 */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.00 : SL runs vomiting out smoke.                             */
/*                                              by Toyoda Masashi 1992/12/11 */

#include <ctype.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "sl.h"

void add_smoke(int y, int x);
void add_man(int y, int x);
int add_C51(int x);
int add_D51(int x);
int add_TGV(int x);
int add_ICE(int x);
int add_sl(int x);
int my_mvaddstr(int y, int x, char *str);

int ACCIDENT  = 0;
int LOGO      = 0;
int FLY       = 0;
int LAND       = 0;
int C51       = 0;
int CONTINUOUS = 0;  // New variable to indicate continuous mode
int NUMBER    = -1;
int TGV       = 0;
int ICE = 0;
int EXIT = 0;

int my_mvaddstr(int y, int x, char *str)
{
    for ( ; x < 0; ++x, ++str)
        if (*str == '\0')  return ERR;
    for ( ; *str != '\0'; ++str, ++x)
        if (mvaddch(y, x, *str) == ERR)  return ERR;
    return OK;
}

void print_help()
{
    printf("Usage: program [options]\n");
    printf("Options:\n");
    printf("  -h, --help                 Display this help message and exit\n");
    printf("  -G, --TGV                  Run a french TGV \n");
    printf("  -i, --ICE                  Run a german ice 1 \n");
    printf("  -l, --little               Run a german ice 1 \n");
    printf("  -a, --accident             some parts of the train will shout out for help\n");
    printf("  -F, --fly                  The selected Train will fly \n");
    printf("  -c, --C51                  Select a different type of steam locomotive \n");
    printf("  -n, --number <number>      Specify a number of cars\n");
    printf("  -v               Enable verbose mode\n");
}
// Struct to support long options
struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"TGV", no_argument, 0, 'G'},
    {"ICE", no_argument, 0, 'i'},
    {"little", no_argument, 0, 'l'},
    {"accident", no_argument, 0, 'a'},
    {"number", required_argument, 0, 'n'},
    {0, 0, 0, 0}};

void option(int argc, char *const argv[])
{
    int c;
    extern int ACCIDENT, LOGO, FLY, LAND, C51, TGV, NUMBER, CONTINUOUS, ICE, EXIT;

    while ((c = getopt_long(argc, argv, "haFlLcrGin:", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'a':
            ACCIDENT = 1;
            break;
        case 'F':
            FLY = 1;
            break;
        case 'L':
            LAND = 1;
            break;
        case 'l':
            LOGO = 1;
            break;
        case 'c':
            C51 = 1;
            break;
        case 'r':
            CONTINUOUS = 1;
            break; // Handle -r option
        case 'G':
            TGV = 1;
            break;
        case 'i':
            ICE = 1;
            break;
        case 'h':
            print_help();
            EXIT = 1;
            break;
        case 'n':
            NUMBER = atoi(optarg);
            break;
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    int x = 0;
    int ch = 0;
    option(argc, argv);
    if (EXIT)
        return 0;
    initscr();

    /* Ignore following signals */
    
    /* Interrupt from keyboard, Control-C */
    signal(SIGINT, SIG_IGN);
    
    /* Quit from keyboard, Control-\ */
    signal(SIGQUIT, SIG_IGN);
    
    /* Stop process execution, Ctrl-Z */
    signal(SIGSTOP, SIG_IGN);
    
    /* Stop process issued from tty */
    signal(SIGTSTP, SIG_IGN);
    
    cbreak();
    
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    /*
     *   first Non prototype TGV was orange
     */
    if (TGV == 1) {
        if (has_colors())
        {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        }
        else
        {
            TGV = 0;
        }
    }
    if (ICE == 1)
    {
        if (has_colors())
        {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_WHITE);
            init_pair(2, COLOR_RED, COLOR_WHITE);
            bkgd(COLOR_PAIR(1));
        }
    }

    int until = -80;
    if (NUMBER > 0)
    {
        until = -80 - (ICELENGTH * NUMBER);
    }

    do
    {
        for (x = COLS - 1; x >= until; --x)
        { // Loop the train from right to left
            if (LOGO == 1)
            {
                if (add_sl(x) == ERR)
                    break;
            }
            else if (C51 == 1)
            {
                if (add_C51(x) == ERR)
                    break;
            }
            else if (ICE == 1)
            {
                if (add_ICE(x) == ERR)
                {
                    break;
                }
            }
            else if (TGV == 1)
            {
                if (add_TGV(x) == ERR)
                    break;
            }
            else
            {
                if (add_D51(x) == ERR)
                    break;
            }
            ch = getch(); // Get user input
            if (ch == 'q')
            {                   // Check if 'q' is pressed
                CONTINUOUS = 0; // Set CONTINUOUS to 0 to stop the loop
                break;
            }
            refresh();
            if (TGV)
                usleep(20000);
            else if (ICE)
            {
                usleep(15000);
            }

            else
                usleep(40000);
        }
    } while (CONTINUOUS); // Repeat the loop if in continuous mode
    // Close the file
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();

    return 0;
}

int add_sl(int x)
{
    static char *sl[LOGOPATTERNS][LOGOHEIGHT + 1]
        = {{LOGO1, LOGO2, LOGO3, LOGO4, LWHL11, LWHL12, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL21, LWHL22, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL31, LWHL32, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL41, LWHL42, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL51, LWHL52, DELLN},
           {LOGO1, LOGO2, LOGO3, LOGO4, LWHL61, LWHL62, DELLN}};

    static char *coal[LOGOHEIGHT + 1]
        = {LCOAL1, LCOAL2, LCOAL3, LCOAL4, LCOAL5, LCOAL6, DELLN};

    static char *car[LOGOHEIGHT + 1]
        = {LCAR1, LCAR2, LCAR3, LCAR4, LCAR5, LCAR6, DELLN};

    if (NUMBER < 0)
        NUMBER = 2;

    int i, j, y, py = 0;
    int LOGOLENGTH = 42 + 21*NUMBER;

    if (x < - LOGOLENGTH)  return ERR;
    y = LINES / 2 - 3;

    if (LAND == 1) {
        y = 1.5 * LINES - (x / 6) - (COLS / 6) - LOGOHEIGHT;
    }
    if (FLY == 1) {
        y = (x / 6) + LINES - (COLS / 6) - LOGOHEIGHT;
        py = 2;
    }
    for (i = 0; i <= LOGOHEIGHT; ++i) {
        my_mvaddstr(y + i, x, sl[(LOGOLENGTH + x) / 3 % LOGOPATTERNS][i]);
        my_mvaddstr(y + i + py, x + 21, coal[i]);
        for (j = 2; j <= NUMBER + 1; ++j)
            my_mvaddstr(y + i + py*j, x + 21*j, car[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 1, x + 14);
        for (j = 2; j <= NUMBER + 1; ++j) {
            add_man(y + 1 + py*j, x + 3 + 21*j);  add_man(y + 1 + py*j, x + 11 + 21*j);
        }
    }
    add_smoke(y - 1, x + LOGOFUNNEL);
    return OK;
}


int add_D51(int x)
{
    static char *d51[D51PATTERNS][D51HEIGHT + 1]
        = {{D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL11, D51WHL12, D51WHL13, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL21, D51WHL22, D51WHL23, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL31, D51WHL32, D51WHL33, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL41, D51WHL42, D51WHL43, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL51, D51WHL52, D51WHL53, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL61, D51WHL62, D51WHL63, D51DEL}};
    static char *coal[D51HEIGHT + 1]
        = {COAL01, COAL02, COAL03, COAL04, COAL05,
           COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    if (NUMBER < 0)
        NUMBER = 1;

    int y, i, j, dy = 0;
    int D51LENGTH = 54 + 29*NUMBER;

    if (x < - D51LENGTH)  return ERR;
    y = LINES / 2 - 5;

    if (LAND == 1) {
        y = 1.5 * LINES - (x / 7) - (COLS / 7) - D51HEIGHT;
        dy = -1;
    }
    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - D51HEIGHT;
        dy = 1;
    }
    for (i = 0; i <= D51HEIGHT; ++i) {
        my_mvaddstr(y + i, x, d51[(D51LENGTH + x) % D51PATTERNS][i]);
        for (j = 1; j <= NUMBER; ++j)
            my_mvaddstr(y + i + dy*j, x + 24 + 29*j, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 2, x + 43);
        add_man(y + 2, x + 47);
    }
    add_smoke(y - 1, x + D51FUNNEL);
    return OK;
}
int add_ICE(int x)
{
    static char *ice[ICEPATTERNS][ICEHEIGHT + 1] = {{ICE1TZ0, ICE1TZ1, ICE1TZ2, ICE1TZ3, ICE1TZ4,
                                                     ICE1WE0, ICEDEL},
                                                    {ICE1TZ0, ICE1TZ1, ICE1TZ2, ICE1TZ3, ICE1TZ4,
                                                     ICE1WE1, ICEDEL}};
    static char *icew[ICEPATTERNS][ICEHEIGHT + 1] = {{ICE1WG0, ICE1WG1, ICE1WG2, ICE1WG3, ICE1WG4, ICE1WG5, ICEDEL},
                                                     {ICE1WG0, ICE1WG1, ICE1WG2, ICE1WG3, ICE1WG4, ICE1WG6, ICEDEL}};
    static char *iceb[ICEPATTERNS][ICEHEIGHT + 1] = {{ICE1TY0, ICE1TY1, ICE1TY2, ICE1TY3, ICE1TY4,
                                                      ICE1TY5, ICEDEL},
                                                     {ICE1TY0, ICE1TY1, ICE1TY2, ICE1TY3, ICE1TY4,
                                                      ICE1TY6, ICEDEL}};
    int y, i, j, dy = 0;
    int ICEWLENGHT = (ICELENGTH) * (NUMBER + 2);
    
    if (x < -ICEWLENGHT)
        return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - ICEHEIGHT;
        dy = 1;
    }

    attron(COLOR_PAIR(2));
    for (i = 0; i <= ICEHEIGHT; ++i)
    {
        my_mvaddstr(y + i, x, ice[(ICEWLENGHT + x) / 5 % ICEPATTERNS][i]);
        for (j = 1; j <= NUMBER; j++)
            my_mvaddstr(y + i + dy * j, x + (ICELENGTH - 1) * j, icew[(ICEWLENGHT + x) / 5 % ICEPATTERNS][i]);
        my_mvaddstr(y + i, x + ICELENGTH - (1 * NUMBER + 1) + NUMBER * ICELENGTH, iceb[(ICEWLENGHT + x) / 5 % ICEPATTERNS][i]);
    }
    attroff(COLOR_PAIR(2));
    return OK;
}

int add_TGV(int x)
{
    static char *tgv[TGVPATTERNS][TGVHEIGHT + 1]
      = {{TGVSTR0, TGVSTR1, TGVSTR2, TGVSTR3, TGVSTR4, TGVSTR5, TGVSTR6,
            TGVWHL1, TGVDEL},
           {TGVSTR0, TGVSTR1, TGVSTR2, TGVSTR3, TGVSTR4, TGVSTR5, TGVSTR6,
            TGVWHL2, TGVDEL}};
    static char *vagoon[TGVHEIGHT + 1]
        = {TGVVAG0, TGVVAG1, TGVVAG2, TGVVAG3, TGVVAG4, TGVVAG5, TGVVAG6, TGVVAG7, TGVDEL};

    int y, i, dy = 0;

    if (x < - TGVLENGTH)  return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - TGVHEIGHT;
        dy = 1;
    }
    attron(COLOR_PAIR(2));
    for (i = 0; i <= TGVHEIGHT; ++i) {
        my_mvaddstr(y + i, x, tgv[((TGVLENGTH + x) / 2) % TGVPATTERNS][i]);
        my_mvaddstr(y + i + dy, x + 55, vagoon[i]);
    }
    attroff(COLOR_PAIR(2));

    if (ACCIDENT == 1) {
        add_man(y + 2, x + 14);

        add_man(y + dy + 3, x + 85);
        add_man(y + dy + 3, x + 90);
        add_man(y + dy + 3, x + 95);
        add_man(y + dy + 3, x + 100);
    }
    return OK;
}

int add_C51(int x)
{
    static char *c51[C51PATTERNS][C51HEIGHT + 1]
        = {{C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH11, C51WH12, C51WH13, C51WH14, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH21, C51WH22, C51WH23, C51WH24, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH31, C51WH32, C51WH33, C51WH34, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH41, C51WH42, C51WH43, C51WH44, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH51, C51WH52, C51WH53, C51WH54, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH61, C51WH62, C51WH63, C51WH64, C51DEL}};
    static char *coal[C51HEIGHT + 1]
        = {COALDEL, COAL01, COAL02, COAL03, COAL04, COAL05,
           COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    if (NUMBER < 0)
        NUMBER = 1;

    int y, i, j, dy = 0;
    int C51LENGTH = 58 + 29*NUMBER;

    if (x < - C51LENGTH)  return ERR;
    y = LINES / 2 - 5;

    if (LAND == 1) {
        y = 1.5 * LINES - (x / 7) - (COLS / 7) - C51HEIGHT;
        dy = -1;
    }
    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - C51HEIGHT;
        dy = 1;
    }
    for (i = 0; i <= C51HEIGHT; ++i) {
        my_mvaddstr(y + i, x, c51[(C51LENGTH + x) % C51PATTERNS][i]);
        for (j = 1; j <= NUMBER; j++)
            my_mvaddstr(y + i + dy*j, x + 26 + 29*j, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 3, x + 45);
        add_man(y + 3, x + 49);
    }
    add_smoke(y - 1, x + C51FUNNEL);
    return OK;
}


void add_man(int y, int x)
{
    static char *man[2][2] = {{"", "(O)"}, {"Help!", "\\O/"}};
    int i;

    for (i = 0; i < 2; ++i) {
        my_mvaddstr(y + i, x, man[(42 + 21*NUMBER + x) / 12 % 2][i]);
    }
}


void add_smoke(int y, int x)
#define SMOKEPTNS        16
{
    static struct smokes {
        int y, x;
        int ptrn, kind;
    } S[1000];
    static int sum = 0;
    static char *Smoke[2][SMOKEPTNS]
        = {{"(   )", "(    )", "(    )", "(   )", "(  )",
            "(  )" , "( )"   , "( )"   , "()"   , "()"  ,
            "O"    , "O"     , "O"     , "O"    , "O"   ,
            " "                                          },
           {"(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)",
            "(@@)" , "(@)"   , "(@)"   , "@@"   , "@@"  ,
            "@"    , "@"     , "@"     , "@"    , "@"   ,
            " "                                          }};
    static char *Eraser[SMOKEPTNS]
        =  {"     ", "      ", "      ", "     ", "    ",
            "    " , "   "   , "   "   , "  "   , "  "  ,
            " "    , " "     , " "     , " "    , " "   ,
            " "                                          };
    static int dy[SMOKEPTNS] = { 2,  1, 1, 1, 0, 0, 0, 0, 0, 0,
                                 0,  0, 0, 0, 0, 0             };
    static int dx[SMOKEPTNS] = {-2, -1, 0, 1, 1, 1, 1, 1, 2, 2,
                                 2,  2, 2, 3, 3, 3             };
    int i;

    if (x % 4 == 0) {
        for (i = 0; i < sum; ++i) {
            my_mvaddstr(S[i].y, S[i].x, Eraser[S[i].ptrn]);
            S[i].y    -= dy[S[i].ptrn];
            S[i].x    += dx[S[i].ptrn];
            S[i].ptrn += (S[i].ptrn < SMOKEPTNS - 1) ? 1 : 0;
            my_mvaddstr(S[i].y, S[i].x, Smoke[S[i].kind][S[i].ptrn]);
        }
        my_mvaddstr(y, x, Smoke[sum % 2][0]);
        S[sum].y = y;    S[sum].x = x;
        S[sum].ptrn = 0; S[sum].kind = sum % 2;
        sum ++;
    }
}
