/* BASICS.C
   This is low-level stuff that is widely used throughout Lugi.  It also
   contains system dependent stuff.  Note that we do not include stdio.h
   in any other source files; they use only functions defined here. */

/* Define AMIGA, WINDOWS, etc to select code here for a target platform.
   Create other ifdefs as necessary for sections specific to other machines,
   if you port this.  Define CRLF for machines that need \r\n for newline.
   To deactivate the --More -- pausing feature, use a do-nothing function
   for CheckWindowSize. */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#ifdef __STDC__
#  include <stdarg.h>
#endif
#ifndef __EMSCRIPTEN__
#  include <stdio.h>
#endif

#include "lugi.h"

#ifdef AMIGA
   // Note that the compiler should target AmigaDOS 1.x on a plain 68k, as 1.x was
   // probably sold on the majority of computers and most of them still have old roms. 
   // Also note that sometimes there are blank lines around #ifs and #endifs, due to
   // a bug in Aztec C that can fail to match them... somehow this is a workaround.
#  include <exec/io.h>
#  include <devices/conunit.h>
#  include <libraries/dosextens.h>
   // I don't know why this undef has to be done over again...
#  undef putchar
#endif

#if (defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS_386__)) && !defined(WINDOWS)
#  define WINDOWS
#endif

#ifdef WINDOWS
#  include <conio.h>
#  include <windows.h>
#  include <shlwapi.h>
#  pragma comment(lib, "Shlwapi.lib")   // somehow helps Visual Stupido, not needed for raw MSVC

   // Open Watcom 1.9 has a somewhat outdated windows.h, and tcc has a reduced one:
#  ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#    define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#  endif
#  ifdef __WATCOMC__
import DWORD WINAPI GetConsoleProcessList(LPDWORD, DWORD);
#  endif

#  define CRLF
#endif

#ifdef MSDOS
#  include <graph.h>
#  include <dos.h>
#  include <conio.h>
#  define CRLF
#endif

#if defined(__unix__) && !defined(__EMSCRIPTEN__)
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#  include <pwd.h>
#  define POZZIX
#endif

#ifdef __EMSCRIPTEN__
#  include <unistd.h>
int vsprintf(char *restrict s, const char *restrict format, va_list ap);
int vsscanf(const char *restrict s, const char *restrict format, va_list arg);

#  include <emscripten.h>
EM_JS(void, writeToSimulatedTerminal, (str msg), { \
        say(UTF8ToString(msg)); \
    })
EM_JS(void, writeBufferToSimulatedTerminal, (str msg, int maxChars), { \
        say(UTF8ToString(msg, maxChars)); \
    })
EM_JS(void, setColorOnSimulatedTerminal, (str cssclass), { \
        setClassOfSaying(UTF8ToString(cssclass)); \
    })
EM_JS(void, writeScoresToLocalStorage, (str contents), { \
        setPersistentCookie("scores", UTF8ToString(contents)); \
    })
EM_JS(void, readScoresToBufferFromLocalStorage, (str buf, size_t blen), { \
        var contents = getCookie("scores"); \
        stringToUTF8(contents || "", buf, blen); \
    })
EM_JS(void, cacheUserNameInLocalStorage, (str name), { \
        setPersistentCookie("playername", UTF8ToString(name)); \
    })
EM_JS(void, getCachedUserNameFromLocalStorage, (str buf, size_t blen), { \
        var contents = getCookie("playername"); \
        stringToUTF8(contents || "", buf, blen); \
    })
EM_JS(void, promptForMoreOnSimulatedTerminal, (), { \
        promptForMore(); \
    })
EM_JS(void, makeANoise, (), { \
        beep(); \
    })
EM_JS(void, logToBrowserConsole, (str label, str val), { \
        console.log(UTF8ToString(label) + " " + UTF8ToString(val)); \
    })        // for integer values use format32
EM_JS(int, getConsoleWidth, (), { \
        return consoleWidth(); \
    })
EM_JS(int, getConsoleHeight, (), { \
        return consoleHeight(); \
    })
EM_JS(void, gameHasEnded, (), { \
        quit(); \
    })

EM_ASYNC_JS(void, readToBufferFromSimulatedTerminal, (str prompt, str buf, size_t blen), { \
        var userInput = await ask(UTF8ToString(prompt)); \
        stringToUTF8(userInput || "", buf, blen); \
    })
EM_ASYNC_JS(void, waitForKeypressAndClearMorePrompt, (), { \
        await waitForKeypressAndClearPrompt(); \
    })
EM_ASYNC_JS(bool, checkDatabaseAccess, (), { \
        return await testDatabaseAccess(); \
    })
EM_ASYNC_JS(bool, checkIfUserNameNeeded, (int score), { \
        return await doesScoreNeedName(score); \
    })
EM_ASYNC_JS(void, saveToServerReturningScoresToBuffer, (str who, int y, int m, int d, int h, int n, int s, int32 howmuch, \
                                                        str buf, size_t blen, int32 *yearpos, int32 *alltimepos, int32 *bottompos), { \
        var scoreState = await saveToServerAndGetScores(UTF8ToString(who), y, m, d, h, n, s, howmuch); \
        if (scoreState && scoreState.formattedScores) { \
            stringToUTF8(scoreState.formattedScores, buf, blen); \
            HEAP32[yearpos >> 2]    = scoreState.yearpos; \
            HEAP32[alltimepos >> 2] = scoreState.alltimepos; \
            HEAP32[bottompos >> 2]  = scoreState.bottompos; \
            setPersistentCookie("scores", scoreState.formattedScores); \
        } else { \
            var backedUpScoresInCaseOfOutage = getCookie("scores"); \
            stringToUTF8(backedUpScoresInCaseOfOutage || "", buf, blen); \
            HEAP32[yearpos >> 2]    = -1; \
            HEAP32[alltimepos >> 2] = -1; \
            HEAP32[bottompos >> 2]  = -1; \
        } \
    })
#endif




#ifdef AMIGA

char crashmessage[] = "         \"Guru Meditation #8400000C.0069574F\"";
// char localshellwords[] = "newcli newshell shell";

#endif

#ifdef MSDOS

char crashmessage[] = "        \"(A)bort (R)etry (F)ail\"";
// char localshellwords[] = "shell command";

#endif

#ifdef WINDOWS

char crashmessage[] = "        \"The program has performed an illegal operation.\"";
// char localshellwords[] = "cmd";

#endif

#ifdef POZZIX

char crashmessage[] = "         \"Your system encountered a serious kernel problem.\"";
// char localshellwords[] = "shell sh csh tcsh ash bash ksh zsh";  // fish?

#endif

#ifdef __EMSCRIPTEN__

char crashmessage[] = "         \"500 Server Error\"";

#endif

/* crashmessage should be a typical text seen on your machine in the event of
   a system crash.  Like for the DEC-20 that Lugi was born on, it was
char crashmessage[] = "       \"%DECSYSTEM-20 NOT RUNNING\"";
   localshellwords should be a list of the command words that one could use
   to tell lugi to spawn a shell on your system.  The string should be one or
   more lowercase words with one space between each pair of words.  The most
   typical (if the feature were activated) would probably just be
char localshellwords[] = "shell";
   On an unsupported port, crashmessage is currently absent and it won't build.
*/



/* returns a random integer in 0 to range-1 */
unsigned RRand(unsigned range)
{
    register unsigned r = (unsigned) rand();
    if (!range) range = 1;
    return r % range;
}

#ifdef INT16
uint32 RRand32(uint32 range)
{
    register uint32 r = (((uint32) rand() << 16) | (unsigned) rand());
    if (!range) range = 1;
    return r % range;
}
#endif


void Randomize(void)
{
    srand((unsigned) time(null));
}


/* returns a random direction from vnorth..d (e.g. vnorth..vwest) */
Meaning RandDirection(Meaning d)
{
    if (d > vclimb) d = vclimb;
    return (Meaning) (RRand((int) d - (int) vnorth + 1) + (int) vnorth);
}


/* When D is in vnorth..vclimb, this returns the opposite direction */
Meaning Opposite(Meaning d)
{
    if (drec(d) & 1) return d - 1;
    else return d + 1;
}



// Returns a random place not near avoid in map, or any room if avoid == nowhere
Place Distant(Place avoid)
{
    register Place p;
    register int d, a;
    do {
        p = (Place) (RRand(nrooms) + 1);
        d = (int) p - (int) avoid;
        a = d < 0 ? -d : d;
    } while (avoid && (a < 2 || a == nrooms));
    return p;
}


// A fancier alternative to Distant: load a set of values, and take random items out from it,
// with a couple of means of filtering acceptable results.  In C++ this would be a class.

private int set[500], setTop = 0;
#define novalue (int) 0x8000

PUBLIC void SetLoadRange(int min, int max)
{
    int i;
    setTop = 0;
    for (i = min; i <= max && i < min + 500; i++)
        set[setTop++] = i;
}

private int SetRemoveAt(int pos)
{
    int r;
    if (pos < 0 || pos >= setTop)
        return novalue;
    r = set[pos];
    set[pos] = set[--setTop];
    return r;
}

PUBLIC int SetRemoveValue(int v)
{
    int i;
    for (i = 0; i < setTop; i++)
        if (set[i] == v)
            return SetRemoveAt(i);
    return novalue;
}

private void SetAdd(int v)
{
    if (v == novalue)
        return;
    SetRemoveValue(v);
    set[setTop++] = v;
}

PUBLIC void SetRemove(bool(*filter)(int, uint32), uint32 filterarg)
{
    int i;
    for (i = 0; i < setTop; i++)
        if (filter(set[i], filterarg))   // you provide a filter: true means reject
            SetRemoveAt(i--);
}

PUBLIC int SetTakeOne(int avoid, int avoidmod)   // this acts like Distant
{
    int a1, a2, a3, r;
    if (avoid <= 0)
        return SetRemoveAt(RRand(setTop));
    a1 = SetRemoveValue(avoid <= 1 ? avoid + avoidmod - 1 : avoid - 1);
    a2 = SetRemoveValue(avoid);
    a3 = SetRemoveValue(avoid >= avoidmod ? avoid + 1 - avoidmod : avoid + 1);
    r = SetRemoveAt(RRand(setTop));
    SetAdd(a1);
    SetAdd(a2);
    SetAdd(a3);
    return r;
}
// end of the pseudo-class for random set retrieval


private Seconds startime = 0;
private bool paws = true;


// seconds elapsed since StartTimer() called... whole-second roundoff means that when you check if say
// 5 seconds have elapsed, it in practice tests for a slightly random duration between 4 and 6 seconds
Seconds Timer(void)
{
    register Seconds t;
    if (paws) return startime;
    else {
        t = (Seconds) time(null) - startime;
        if (t < 0) return 0;
        else return t;
    }
}


void StartTimer()    /* resets and resumes */
{
    startime = (Seconds) time(null);
    paws = false;
}


void PauseTimer()
{
    if (!paws) {
        paws = true;
        startime = (Seconds) time(null) - startime;
    }
}


void ResumeTimer()
{
    if (paws) {
        paws = false;
        startime = (Seconds) time(null) - startime;
    }
}

/* if the timer is paused, startime holds the reading paused at, else it holds
   the time() value considered to be "zero" */


#define MAX_INPUT_LENGTH 255
#define MAX_OUTPUT_WIDTH 80

PUBLIC char line1[MAX_INPUT_LENGTH + 1];  /* used in GetLetter */
PUBLIC char line2[MAX_INPUT_LENGTH + 1];  /* used in AskCommand */

private bool haveCheckedAnsi = false;


#define ENDLESS 9999

private ushort linesout = 0;
private int ansiColorBits = 0;  // 0 = no ansi or color, 1 = monochrome ansi, 2 - 8 = 4 to 256 colors (don't bother with rgb), -1 = use CSS classes

private str argpath = null, scorepath = null;
private void *oldCurDir = null;

#ifdef AMIGA

private ushort windowheight = ENDLESS, windowwidth = 76;

#else
#  ifdef MSDOS

private ushort windowheight = 24, windowwidth = 79;

#  else

private ushort windowheight = 23, windowwidth = 79;

#  endif
#endif

/* The height is actually the height minus one to give a bit of overlap, and
width is real width minus one to leave room for the cursor at the right margin.
77 is the default width for an Amiga console window with a size 8 font; other
systems should mostly use 80, or 40 for cheap lo-res things.  For GUI systems
it depends on how the user sizes the window.  For large displays we like to
limit the max width to 80 for readability and a bit of old-sk00l aesthetics. */

/* (ya know I _could_ add proportional font support ...     naah...) */

#define BOZE 400L

private char bout[BOZE + 3];    /* cushion for newline chars and whatever */
private short bound = 0, lined = 0;


#ifdef WINDOWS

private HANDLE outHandle = (HANDLE) 0;
private DWORD consoleMode = (DWORD) -1;
#  define ScrollPause() Sleep(12)

#endif

#ifdef POZZIX

#  define ScrollPause() usleep(12000)

#endif

#ifdef __EMSCRIPTEN__

#  define ScrollPause()  emscripten_sleep(12)

#endif

#ifndef ScrollPause
#  define ScrollPause() ((void) 0)
#endif

private void CheckWindowSize()
{
#ifdef AMIGA
    import void *AllocMem(uint32, uint32), *FindTask(str);
    import long dos_packet(void*, int32, int32);     /* a Manx convenience */
    struct InfoData *ind;

    static struct Process *me;
    static struct ConUnit *cu = null;

    if (!cu && (ind = AllocMem((long) sizeof(struct InfoData), 0L))) {
        me = FindTask(null);    /* first time only */
        if (dos_packet(me->pr_ConsoleTask,
                       (long) ACTION_DISK_INFO, (long) ind >> 2))
            cu = (void *) ((struct IOStdReq *) ind->id_InUse)->io_Unit;
        else
            cu = (void *) -1;   /* probly output redirected away from console */
        FreeMem(ind, (long) sizeof(struct InfoData));
    }
    if (cu && ~(long) cu) {
        windowheight = cu->cu_YMax;
        windowwidth = cu->cu_XMax + 1;
        ansiColorBits = 2;      // more is unusual, with unpredictable palette; less will still be compatible
    }
#endif

#ifdef WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!outHandle)
        outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleMode == (DWORD) -1)
        GetConsoleMode(outHandle, &consoleMode);
    if (GetConsoleScreenBufferInfo(outHandle, &csbi)) {
        windowwidth = csbi.srWindow.Right - csbi.srWindow.Left;
        windowheight = csbi.srWindow.Bottom - csbi.srWindow.Top;

        if (!haveCheckedAnsi && SetConsoleMode(outHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            // a surprisingly recent feature addition                   ^^^
            // I'm semi-confident that it will return false in earlier Windows versions
            DWORD newMode;   // ...but let's make extra sure
            if (GetConsoleMode(outHandle, &newMode) && newMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                ansiColorBits = 4;   // if you use finer colors it may round them down
            haveCheckedAnsi = true;
        }
    }
#endif

#ifdef POZZIX
    str term;
    struct winsize w;
    if (ioctl(0, TIOCGWINSZ, &w) >= 0) {
        windowheight = w.ws_row - 1;
        windowwidth  = w.ws_col - 1;
        if (!haveCheckedAnsi && (term = getenv("TERM")) && strncmp(term, "xterm", 5) == 0) {
            // xterm is the only flavor of terminal support worth checking for nowadays
            ansiColorBits = 1;
            if (strncmp(term + 5, "-256", 4) == 0)
                ansiColorBits = 8;
            else if (strncmp(term + 5, "-16", 3) == 0)
                ansiColorBits = 4;
            else if (strncmp(term + 5, "-color", 6) == 0)
                ansiColorBits = 3;
            haveCheckedAnsi = true;
        }
    }
#endif

#ifdef __EMSCRIPTEN__
    windowheight = getConsoleHeight();
    windowwidth = getConsoleWidth();
    ansiColorBits = -1;
#endif

#ifdef MSDOS
    struct videoconfig vc;
    int t;
    char c;
    if (_getvideoconfig(&vc)) {
        windowwidth = vc.numtextcols - 1;
        windowheight = vc.numtextrows - 1;
    }
    if (!haveCheckedAnsi) {
        while (kbhit()) getch();
        fputs("\n\x1B[6n", stdout);
        fflush(stdout);
        for (t = 0; t < 5 && !kbhit(); t++)
            delay(40);   // wait up to 1/5 second for automatic response
        if (kbhit()) {
            c = getch();
            if (c == '\x1B' && kbhit()) {
                c = getch();
                if (c == '[')
                    ansiColorBits = vc.monitor == _MONO || vc.monitor == _ANALOGMONO ? 1 : 4;
            }
            while (kbhit()) getch();           // flush
        }
        if (!ansiColorBits) fputs("\r         \r", stdout);
        haveCheckedAnsi = true;
    }
#endif
    /* for others not supported yet this just lets the default values stand */
}


PUBLIC int Width() { return windowwidth; }



private str MorePromptText = " -- More -- ";
private str MorePromptClear = "\r            \r";

private void WaitForKey()
{
#ifdef AMIGA
    char x[4];
    set_raw();                  // for SAS C this will need a local version
    Delay(10L);                 // 0.2 sec
    fread(&x, 1, 1, stdin);
    set_con();
    fputs(MorePromptClear, stdout);
#endif

#ifdef MSDOS
    getch();
    fputs(MorePromptClear, stdout);
#endif

#ifdef WINDOWS
    getch();
    fputs(MorePromptClear, stdout);
#endif

#ifdef POZZIX
    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    fputs(MorePromptClear, stdout);
#endif

#ifdef __EMSCRIPTEN__
    waitForKeypressAndClearMorePrompt();
#endif
}


void flushw()
{
    if (!bound) return;
#ifdef __EMSCRIPTEN__
    writeBufferToSimulatedTerminal(bout, bound);
#else
    fwrite(bout, 1, bound, stdout);
    fflush(stdout);
#endif
    if (bound > 0 && bout[bound - 1] == '\n')
        ScrollPause();   // scroll slow enough to follow visually
    bound = 0;
#ifdef AZTEC_C
    Chk_Abort();            /* hear control-C during long output */
#endif
}


private str lastColor = null, nextColor = null;

private void nextColorImmediate()
{
#ifndef __EMSCRIPTEN__
    if (nextColor) {
        // nextColor values must be constants and length <= 12
        fprintf(stdout, "\x1B[%sm", nextColor);
    }
#else
    setColorOnSimulatedTerminal(nextColor);
#endif
}

private void useNextColor()
{
    if (lastColor != nextColor) {
        flushw();
        nextColorImmediate();
        lastColor = nextColor;
    }
}


/* this function buffers all output written by put, puts, and printf.  It
word-wraps paragraphs and inserts "-- More --" prompts when the window is full.
Note that it DOES NOT correctly handle tab characters, so don't send it any. */

PUBLIC int putchar(int ch)
{
    static bool spaceat = false;
    register char c = (char) ch;

    if (!c || (spaceat && c == ' '))
        return 0;
    spaceat = false;
    useNextColor(); 
#ifdef MSDOS
    if (c == 7)
        return fputc(c, stdout);    /* beep immediately */
#endif
#ifdef __EMSCRIPTEN__
    if (c == 7) {
        makeANoise();
        return c;
    }
#endif
    if (linesout < windowheight - 1 || windowheight == ENDLESS) {
        if (c == '\n' || bound >= BOZE) {
#ifdef CRLF
            bout[bound++] = '\r';
#endif
            bout[bound++] = '\n';
            linesout++;
            lined = 0;
            flushw();
        } else {
            bout[bound++] = c;
            lined++;
            // Could add support for proportional spacing here?  Let's not.
            if (lined > windowwidth || lined > MAX_OUTPUT_WIDTH) {
                ushort top, i;
#ifdef CRLF
                char r = 0;
#endif
                char n;

                top = bound;    // find end of word:
                while (bound >= 0 && bout[--bound] != ' ') ;
                while (bound >= 0 && bout[--bound] == ' ') ;
                if (bound < 0) bound = lined > 0 ? 0 : top - 1;
                else bound++;
#ifdef CRLF
                r = bout[bound];
                bout[bound++] = '\r';
#endif
                n = bout[bound];
                bout[bound++] = '\n';
                i = bound;
                flushw();
                linesout++;
                lined = 0;
                bout[--i] = n;
#ifdef CRLF
                bout[--i] = r;
#endif
                while (i <= top && bout[i] == ' ') i++;
                for (bound = 0; bound < top - i; bound++)
                    bout[bound] = bout[bound + i], lined++;
                spaceat = !bound;
            }
        }
    } else {
        // if we land here, flushw has just happened
#ifndef __EMSCRIPTEN__
        fputs(MorePromptText, stdout);
        fflush(stdout);
#else
        promptForMoreOnSimulatedTerminal();
#endif
        WaitForKey();
        nextColorImmediate();
        bout[bound++] = c;
        lined++;
        linesout = c == '\n' ? 1 : 0;
    }
    return (unsigned) c;
}



/* the following two definitions are basically redundant for some compilers
... for Aztec this nearly duplicates the library definitions (actually they
use something called aputc which putchar also calls), but so what, we do them
explicitly anyway ... makes debugging easier. */

PUBLIC void put(register const char* s)
{
    while (*s)
        putchar(*(s++));
}



PUBLIC int puts(const char *s)
{
    put(s);
    putchar('\n');
    return 0;
}


PUBLIC int printf(const char *f, ...)
{
    static char buf[3000];
    va_list a;
    va_start(a, f);
    vsprintf(buf, f, a);
    put(buf);
    va_end(a);
    return 0;
}


PUBLIC int rawprintf(const char *f, ...)
{
    int r = 0;
#ifndef __EMSCRIPTEN__
    va_list a;
    va_start(a, f);
    r = vfprintf(stdout, f, a);
    fflush(stdout);
    va_end(a);
#endif
    // don't implement for emscripten, it's just used for test harness
    return r;
}


#if defined(_MSC_VER) || defined(__EMSCRIPTEN__)
PUBLIC int sscanf(const char *s, const char *f, ...)
{
    int r;
    va_list a;
    va_start(a, f);
    r = vsscanf(s, f, a);
    va_end(a);
    return r;
}
#endif

private str format32uimpl(str buf, uint32 v)
{
    if (v <= 999)
        buf += sprintf(buf, "%u", (uint) v);
    else {
        buf = format32uimpl(buf, (uint32) (v / 1000));
        buf += sprintf(buf, ",%03u", (uint) (v % 1000));
    }
    return buf;
}

private str formattingbuf()      // supplies up to four buffer spaces in rotation, so you
{                                // can pass up to four format32[u] calls to e.g. printf
    static int offset = 0;
    static char buf4[64];
    offset = (offset + 16) % 64;
    return buf4 + offset;
}

PUBLIC str format32(int32 v)
{
    str buf = formattingbuf();
    str b = buf;
    if (v < 0)
        *b++ = '-', v = -v;
    format32uimpl(b, (uint32) v);
    return buf;
}

PUBLIC str format32u(uint32 v)
{
    str buf = formattingbuf();
    if (v <= 9999)
        // as it happens, all our signed outputs want to be column-aligned, and unsigned ones are
        // in normal text flow, so we use the no-comma-for-four-digits rule only for unsigned
        sprintf(buf, "%u", (uint) v);
    else
        format32uimpl(buf, v);
    return buf;
}


PUBLIC void colorNormal(void)      // light gray
{
    switch (ansiColorBits) {
        case 2:  nextColor = "0"; break;           // default, typically black on light gray for amiga 2.x, white on blue for 1.x
        case 3:  nextColor = "0;37"; break;        // usually #ffffff on black
        case 4:  nextColor = "0;2;37"; break;      // usually #c0c0c0 on black
        case 8:  nextColor = "0;38;5;252"; break;  // usually #d0d0d0 on black
        case -1: nextColor = null; break;
        default: nextColor = null;
    }
}


PUBLIC void colorCommand(void)     // bright green
{
    switch (ansiColorBits) {
        case 2:  nextColor = "0"; break;           // no soup for you
        case 3:  nextColor = "32"; break;
        case 4:  nextColor = "1;32"; break;
        case 8:  nextColor = "38;5;10"; break;
        case -1: nextColor = "command"; break;
        default: nextColor = null;
    }
}


PUBLIC void colorEvent(void)       // bright cyan
{
    switch (ansiColorBits) {
        case 2:  nextColor = "0"; break;           // no soup for you
        case 3:  nextColor = "36"; break;
        case 4:  nextColor = "1;36"; break;
        case 8:  nextColor = "38;5;14"; break;
        case -1: nextColor = "event"; break;
        default: nextColor = null;
    }
}


PUBLIC void colorAlarm(void)       // bright yellow or gold
{
    switch (ansiColorBits) {
        case 2:  nextColor = "33"; break;   // highlight color, usually blue or orange on amiga
        case 3:  nextColor = "33"; break;
        case 4:  nextColor = "1;33"; break;
        case 8:  nextColor = "38;5;220"; break;    // usually #ffd700
        case -1: nextColor = "alarm"; break;
        default: nextColor = null;
    }
}


PUBLIC void colorDebug(void)       // magenta
{
    switch (ansiColorBits) {
        case 2:  nextColor = "32"; break;      // usually inverse of default color, low contrast w/ bg
        case 3:  nextColor = "35"; break;
        case 4:  nextColor = "1;35"; break;
        case 8:  nextColor = "38;5;13"; break;
        case -1: nextColor = "debug"; break;
        default: nextColor = null;
    }
}


PUBLIC void putsEvent(str s)
{
    str old = nextColor;
    colorEvent();
    puts(s);
    nextColor = old;
}


PUBLIC void putsAlarm(str s)
{
    str old = nextColor;
    colorAlarm();
    puts(s);
    nextColor = old;
}


PUBLIC void printfDebug(const char *f, ...)
{
#ifdef _DEBUG
    static char buf[3000];
    va_list a;
    str old = nextColor;
    colorDebug();
    va_start(a, f);
    vsprintf(buf, f, a);
    put(buf);
    va_end(a);
    nextColor = old;
#endif
}



PUBLIC void GetLine(str prompt, str line12)
{
    size_t i;
#ifndef __EMSCRIPTEN__
    linesout = 0;          // assume the prompt will fit on one line, don't go More right before it
    put(prompt);
    flushw();
    colorCommand();
    useNextColor();
    if (!fgets(line12, MAX_INPUT_LENGTH, stdin))
        line12[0] = 0;
    if (feof(stdin)) {
        clearerr(stdin);   // otherwise a ^D in unix can cause an infinite loop... ^Z in msdos lands here too
        putchar('\n');
    }
#else
    colorCommand();
    useNextColor();
    readToBufferFromSimulatedTerminal(prompt, line12, MAX_INPUT_LENGTH);
    // (the simulated console always assumes prompt fits on one line, as it is unable to intentionally break it in the middle)
#endif
    colorNormal();
    for (i = strlen(line12) - 1; i >= 0 && (line12[i] == '\r' || line12[i] == '\n'); i--)
        line12[i] = 0;
    linesout = 0;
    lined = 0;
    CheckWindowSize();
}


/*
void Shell()
{
#ifdef AMIGA
    Place p;
    Roomt *r;
    import BPTR Open(void);
    BPTR ramm;
    static char lin[500];

    puts("**** I don't know how to spawn a shell yet.");
    puts("So what I'll do instead is dump the map into RAM:LugiMap.");
    if (!(ramm = Open("ram:LugiMap", MODE_NEWFILE)))
        puts("On second thought, no I won't.");
    else {
        for (p = nowhere; p <= pbalcony; p++) {
            r = map + p;
            sprintf(lin, "Room %2d is appearance %2d, paths "
                         "%2d %2d %2d %2d %2d %2d, %s gleeps.  %sexplored.\n",
                         p, r->features, r->paths[0], r->paths[1],
                         r->paths[2], r->paths[3], r->paths[4], r->paths[5],
                         format32u(gleepct[p]), (r->explored ? "" : "Un"));
            Write(ramm, lin, (long) strlen(lin));
        }
        Close(ramm);
    }
#else
    import bool did;
    puts("Shell command is not implemented yet.");
    did = false;
// intentional crash?    *((int*) 0x00000F0C) = 0;
#endif
}
*/



private str ReplaceTail(str buf, str newtail)
{
    str p;
#if defined (WINDOWS) || defined(MSDOS)
    p = strrchr(buf, '\\');
#else
    p = strrchr(buf, '/');
#endif
#if !defined(__unix__) && !defined(__EMSCRIPTEN__)
    if (!p) p = strrchr(buf, ':');
#endif
    if (!p) return newtail;
    *++p = '\0';
    strcpy(p, newtail);
    return buf;
}



private str HighScoreFilePath()
{
    str hope = "Lugi-high-scores.txt";
    static char pathbuf[1024];
#ifdef AMIGA
    str fallback = "S:Lugi-high-scores.txt";    // used under AmigaDOS 1.x
    struct Process *me = ((struct Process *) FindTask(null));
    if ((*(struct Library **) 4)->lib_Version >= 36 && me->pr_HomeDir) {
        oldCurDir = (void *) (ulong) me->pr_CurrentDir;
        CurrentDir(me->pr_HomeDir);   // bad return value happens from CurrentDir??
        if (me->pr_CurrentDir)
            return hope;
    }
    return fallback;
#endif

#ifdef MSDOS
    // I have not learned of a reliable means of finding the app's home dir
    hope = "LUGSCORE.TXT";
    oldCurDir = null;
#endif

#ifdef WINDOWS
    static TCHAR oldcdbuf[1000];
    TCHAR newcdbuf[1000], *p;
    int r = GetCurrentDirectory(1000, oldcdbuf);
    if (r > 0 && r < 1000) {
        oldCurDir = oldcdbuf;
        r = GetModuleFileName(null, newcdbuf, 1000);
        if (r > 0 && r < 1000) {
            p = PathFindFileName(newcdbuf);
            if (p > newcdbuf) {
                *p = 0;
                SetCurrentDirectory(newcdbuf);
            } else oldCurDir = null;
        } else oldCurDir = null;
    } else oldCurDir = null;
    if (oldCurDir)
        return hope;
#endif

#ifdef POZZIX
    static char oldcdbuf[1000];
    int r = readlink("/proc/self/exe", pathbuf, 1000);  // is this Linux only?
    if (r > 0 && r < 1000) {
        pathbuf[r] = '\0';
        ReplaceTail(pathbuf, "");
        if (getcwd(oldcdbuf, 1000)) {
            oldCurDir = oldcdbuf;
            if (chdir(pathbuf))
                oldCurDir = null;
        }
    }
    if (oldCurDir)
        return hope;
#endif

    // our fallback is argv[0]
    strncpy(pathbuf, argpath, 1000);
    pathbuf[999] = '\0';
//printfDebug("using argv0 \"%s\" as no home dir found\n", pathbuf);
    return ReplaceTail(pathbuf, hope);
}


// We don't care about the file format here, we just persist it.  See lugi.c for what's in it.
PUBLIC str LoadScoreFile(void)
{
    static char contents[4096];
#ifndef __EMSCRIPTEN__
    FILE *fp;
    size_t c;
    scorepath = HighScoreFilePath();
    fp = fopen(scorepath, "r");
//if (!fp) printfDebug("score file %s not found, errno is %d\n", scorepath, errno);
    if (!fp) return null;
    c = fread(contents, 1, 4096, fp);
//printfDebug("score file contained %d bytes\n", c);
    contents[c] = '\0';
    fclose(fp);
#else
    readScoresToBufferFromLocalStorage(contents, sizeof(contents));
#endif
    return contents;
}


PUBLIC void SaveScoreFile(str contents)
{
#ifndef __EMSCRIPTEN__
    FILE *fp;
    fp = fopen(scorepath, "w");
    if (fp) {
        fwrite(contents, 1, strlen(contents), fp);
        fclose(fp);
    }
#else
    writeScoresToLocalStorage(contents);
#endif
}


PUBLIC bool ScoreDatabaseAvailable()
{
#ifdef __EMSCRIPTEN__
    return checkDatabaseAccess();
#else
    return false;
#endif
}


PUBLIC bool WillScoreNeedUserName(int score)
{
#ifdef __EMSCRIPTEN__
    return checkIfUserNameNeeded(score);
#else
    return false;
#endif
}


PUBLIC void RememberUserName(str username)
{
#ifdef __EMSCRIPTEN__
    cacheUserNameInLocalStorage(username);
#endif
}


PUBLIC str GetScoresFromDatabaseAfterUpdate(str who, struct tm when, int32 howmuch, int32 *yearplace, int32 *alltimeplace, int32 *bottomplace)
{
#ifdef __EMSCRIPTEN__
    static char contents[4096];  // max 14*288 - names nay contain 64 four-byte chars
    saveToServerReturningScoresToBuffer(who, when.tm_year + 1900, when.tm_mon + 1, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec,
                                        howmuch, contents, sizeof(contents), yearplace, alltimeplace, bottomplace);
logToBrowserConsole("returned scores", contents);
    return contents;
#else
    return null;
#endif
}


PUBLIC str UserName()
{
#ifdef WINDOWS
    static char un[USERNAMELEN];
    DWORD ln = USERNAMELEN;
    return GetUserNameA(un, &ln) ? un : "";
    // if your login name is too long, you lose and this returns nothing
#endif
#ifdef POZZIX
    struct passwd *p = getpwuid(getuid());
    return p && strlen(p->pw_name) < USERNAMELEN ? p->pw_name : "";
    // again, too long = you lose
#endif
#ifdef __EMSCRIPTEN__
    static char un[USERNAMELEN];
    getCachedUserNameFromLocalStorage(un, USERNAMELEN);
    return un;
    // in this case the name is typed in by the player, with full utf8 supported,
    // and long names are truncated safely here at the last codepoint that fits...
    // but note that the back end score storage does its own different truncation,
    // which on paulkienitz.net is 64 codepoints... this must not out-truncate that
#endif
    return "";     // not applicable on single-user systems
}



#ifdef AMIGA

#  include <workbench/workbench.h>
#  include <workbench/startup.h>
#  include <clib/icon_protos.h>

/* _wb_parse gets called before main does, if launched from an icon */

extern int _argc, Enable_Abort;

void *IntuitionBase, *IconBase = null;

void _wb_parse(me, wbm) register struct Process *me; struct WBStartup *wbm;
{
    import BPTR Open(str, long);
    import void *OpenLibrary(str, uint32), CloseLibrary(struct Library*);
    import void FreeDiskObject(struct DiskObject*);
    // the default console to open fits on the minimum size workbench:
    str winspec = "CON:0/2/640/198/ the game of LUGI ", cp;
    struct DiskObject *dop = null;
    register BPTR wind;
    struct FileHandle *pwind;
    
    if (IconBase = OpenLibrary("icon.library", 0L))
        if (dop = GetDiskObject(wbm->sm_ArgList->wa_Name))
            if (cp = FindToolType((UBYTE **) dop->do_ToolTypes, "WINDOW"))
                winspec = cp;
    if (wind = Open(winspec, (long) MODE_OLDFILE))
        pwind = (void *) (wind << 2);
    if (dop) FreeDiskObject(dop);
    if (IconBase) CloseLibrary(IconBase);
    IconBase = null;
    me->pr_CIS = wind;
    if (wind && (me->pr_ConsoleTask = (void *) (pwind->fh_Type)))
        me->pr_COS = Open("*", (long) MODE_OLDFILE);
    if (!wind || !me->pr_COS) {
        if (IntuitionBase = OpenLibrary("intuition.library", 0L)) {
            DisplayBeep(null);
            CloseLibrary(IntuitionBase);
        }
        if (wind) Close(wind);
        exit(10);
    }
    _argc = 0;
    Enable_Abort = 1;
}

PUBLIC void _abort()
{
    fputs("^C\r\n", stdout);
    exit(10);
}

#endif



PUBLIC void local_startup(str argv0)
{
    argpath = argv0;
    CheckWindowSize();
    if (ansiColorBits > 0) {
        MorePromptText = "\x1B[0;7m -- More -- \x1B[0m";   // ANSI inverse color
        MorePromptClear = "\r\x1B[J";                      // ANSI clear to end of screen
        colorNormal();
    }

#ifdef MSDOS
    // XXX TODO: make sure ^C can interrupt the program -- that isn't working (at least with Watcom in MS-DOS Player)
    // signal(SIGINT, abort);     // ...this does not work in Takeda Toshiya's MS-DOS Player... try dosbox?
#endif

#ifdef AMIGA
    Enable_Abort = true;
#endif
}



PUBLIC void local_shutdown()
{
#ifdef WINDOWS
    DWORD process;
#endif

#ifndef __EMSCRIPTEN__
    if (ansiColorBits > 0) fputs("\x1B[0m", stdout), fflush(stdout);
#endif

#ifdef AMIGA
    if (oldCurDir) CurrentDir((BPTR) oldCurDir);
    // do like this on any system which can open a console window for an app launched GUI-wise:
    if (!_argc) {
        put(ansiColorBits ?
            "\n                    \x1B[7m-- PRESS ANY KEY TO ERASE WINDOW --\x1B[0m " :
            "\n                    -- PRESS ANY KEY TO ERASE WINDOW -- ");
        flushw();
        WaitForKey();
    }
#endif

#ifdef WINDOWS
    if (oldCurDir) SetCurrentDirectory(oldCurDir);
    // ### BUG: THIS DOES NOT WORK IN WATCOM, it thinks it's in a popup when it's not
    if (GetConsoleProcessList(&process, 1) <= 1) {
        put(ansiColorBits ?
            "\n                    \x1B[7m-- PRESS ANY KEY TO ERASE WINDOW --\x1B[0m " :
            "\n                    -- PRESS ANY KEY TO ERASE WINDOW -- ");
        flushw();
        WaitForKey();
        ScrollPause();      // there seems to be a race condition between the above output and SetConsoleMode
    }
    SetConsoleMode(outHandle, consoleMode);
#endif

#ifdef POZZIX
    if (oldCurDir) chdir(oldCurDir);
#endif

#ifdef __EMSCRIPTEN__
    gameHasEnded();
#endif
}
