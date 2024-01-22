/* LUGI.C
   This is the main program of the game Lugi, and stuff that is peripheral
   to the playing of the game, like the initial introduction and the
   totalling up of your score.

   This game is written in portable C, with all the system specific stuff
   confined to the source file basics.c, where there is stuff to cause all
   text output to word-wrap, and be automatically broken with "-- More --"
   prompts when it fills the height of the window.  One will need to write
   system specific code to measure the size of the output window for this to
   work elsewhere.  This should port to most any C compiler that supports
   prototypes, enum types, void, continuing strings over multiple lines with
   backslash-newline, with only functions in basics.c needing to be updated
   to support your platform.

   The source files are:
   lugi.c       main program, before & after game formalities
   obey.c       carries out player's commands, augmented by:
   put.c        handles "put ..." -- the game's most complex command
   take.c       handles picking up objects -- the second most complex
   drop.c       handles dropping and throwing, accidental or deliberate
   gothere.c    handles moving around and suchlike, also fighting enemies
   fiddle.c     handles miscellaneous object-using commands, like pour and fill
   player.c     stuff relating to the "protagonist"; Inventory(),
                  Weight(), Have(ob), your present Situation()
   embassy.c    stuff about the building; MakeMap(), Here(ob), cheat/test tool
   roomz.c      the descriptions of the rooms
   upchuck.c    Update()s the situation in response to player actions
   diction.c    handles reading and translating pseudo-English commands
   basics.c     low level and platform-specific stuff
   lugi.h       mostly the big enumerated types: Meaning, Place, and Appearance

   TO DO: support "kill ... with (object)" and "drink ... from (container)"
          support "pull plug", "out plug", and "plug out"
          "look in (container)" to inventory contents, opening it if it's galloncan
*/

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "lugi.h"


char copyright[] = "\n LUGI is copyright (c) 1980, 1981, 2023 by Jay Wilson and Paul Kienitz.\n";



import ushort beenin;

import bool shortgame, guardalive, runtalive, burnt, badsandwich, bystanders, attached,
            fungus, wipeout, beastfollowing, strong, clean, badhygiene, cheated;

import Sickness sickness;

import Place runtroom;

import str format32(int32 v);


typedef struct {
    int32     howmuch;
    struct tm when;
    char      who[USERNAMELEN];
} Score;

private int32 totalscore = 0;


private str introduction[] = {
 "\n\n\n         Pay attention, cause I'm only gonna say this once.\n\n",

 "You are a CIA agent faced with the dangerous task of exploring the Lugonian ",
 "Embassy.  The Lugonians, also popularly called Lugimen, are from the planet ",
 "Arcturus IV.  They are large, powerful, and dangerous beings, possessing ",
 "extremely advanced technology, including teleportation.  We have reason to ",
 "suspect that their intentions in establishing diplomatic relations with the ",
 "governments of Earth are not entirely in our best interest.  In fact, there ",
 "are reports that some Lugimen believe that humans have no use to Lugonians ",
 "except as food.  There are persistent rumors that several people, including ",
 "some missing CIA agents, have in fact been eaten, though all reputable ",
 "experts on the Lugonians state that these fears are groundless.  But even ",
 "the most conservative experts agree that the Lugimen are brutal, uncouth, ",
 "generally filthy, easily angered, and not to be trifled with.  Furthermore, ",
 "they are male chauvinist pigs, even by CIA standards.\n\n",

 "The building you will attempt to explore is constantly in flux, according ",
 "to the few who have gotten out alive, except for the \"show\" embassy in ",
 "front, where diplomats are entertained.  This part holds nothing of any ",
 "interest, and is completely isolated from the real Embassy inside.  Many ",
 "of the rooms inside are not physically connected with passageways, but ",
 "instead with teleportation gates.  These are usually just patches of wall ",
 "which show a characteristic shimmer.  You walk through them and find ",
 "yousrelf elsewhere.  Occasionally, such gates are completely invisible.  ",
 "The Lugonians rearrange the gates inside the building quite frequently, and ",
 "no map that we've been able to get out has been of any use to the next ",
 "explorer.  Frequent unnecessary change is a character trait which the ",
 "Lugonians show under many circumstances.  Their culture is prone to many ",
 "brief fads, including some adopted from Earth, though some things never ",
 "change, such as their fondness for packing heat, usually in the form of ",
 "powerful ray-blasters.\n\n",

 "Your mission is to explore as much of the building as you can and get out ",
 "alive, bringing out with you as many objects of Lugonian manufacture as you ",
 "can, and as many gleeps as you can pick up.  The only known exit point from ",
 "the inside is a balcony five floors above the street.  You won't have much ",
 "time inside, because they inevitably detect and track down intruders, so ",
 "move fast.  You might want to scribble a map as you move about, which will ",
 "probably be useful in finding your way around while you're in there, though ",
 "it would be of no use to any future explorers.\n\n",

 "You will move about the building by typing commands such as \"go north\" or ",
 "\"climb up\" or \"leave\", and interact with objects via commands such as ",
 "\"take flask\" or \"put the statuette into the bag\".  Common movement ",
 "commands can be abbreviated, like \"e\" for eastward or \"n\" for north.  ",
 "You can use \"j\" for jumping down (or just \"d\" for down), or say \"c\" or ",
 "\"u\" for climbing upward, and use \"i\" or \"inv\" for an ",
 "inventory of what you are carrying.  Also, \"f\" and \"b\" can be used to ",
 "continue forward or turn backward, relative to your last move.  A variety ",
 "of other verbs may be useful... maybe even \"kill\".\n\n",
 

 "Remember, this is a volunteer mission.  You need not elect to enter the ",
 "Embassy.  None of your fellow agents will think any less of you if you turn ",
 "down this extremely hazardous task.\n",
 null
};


private void Inform(void)
{
    char c;
    short i;
    do
        c = GetLetter("\nYa wanna detailed briefing, Charlie?    [type Y or N  (or S or L)]  ");
    while (c != 'y' && c != 'n' && c != 's' && c != 'l');
    if (c == 'y') {
        for (i = 0; introduction[i]; i++)
            put(introduction[i]);
        do
            c = GetLetter("\nWill you accept the mission, and enter the Embassy?    [Y or N]  ");
        while (c != 'y' && c != 'n');
        if (c == 'n') {
            death = chicken;    /* death #zero, so to speak */
            puts("\n\n\n\n\n\n\n\nOkay, we'll get someone else.     (you weenie)");
            return;
        }
    }
    while (c != 's' && c != 'l')
        c = GetLetter("\nDo you want a short game or a long game?     [S or L]  ");
    shortgame = c == 's';
    putsEvent("\nYou are concealed in a load of steak and electronic parts and left \
in an abandoned building.  After a while, there are noises, and then all is \
quiet.  You creep out of the crate, and through a small opening into another room...\n");
}



private str ReadScore(str line, Score *result)
{
    import int sscanf(const char *s, const char *f, ...);
    str a, b;
    int r;
    long h;
    memset(result, 0, sizeof(Score));
    // expect each line to be a signed decimal score, a space, then a timestamp in ISO format without timezone, then
    // a space and a pipe separator followed by an optional username, which we limit to 64 chars after trimming spaces
    r = sscanf(line, "%ld %u-%u-%uT%u:%u:%u |",
               &h, &result->when.tm_year, &result->when.tm_mon, &result->when.tm_mday,
               &result->when.tm_hour, &result->when.tm_min, &result->when.tm_sec);
    result->howmuch = (int32) h;
    if (result->when.tm_year) result->when.tm_year -= 1900;
    if (result->when.tm_mon) result->when.tm_mon--;
    if (!result->when.tm_mday) result->when.tm_mday = 1;
    result->when.tm_isdst = -1;
    a = strchr(line, '|');
    if (a)
        line = ++a;
    b = strchr(line, '\n');
    if (!b) b = line + strlen(line);
    line = b;
    if (a) {
        while (*a == ' ' && a < line) a++;
        while ((unsigned) b[-1] <= ' ' && b > a) b--;
        if (b >= a + USERNAMELEN) b = a + USERNAMELEN - 1;  // if name is overlong, score file was corrupted
        strncpy(result->who, a, b - a);
        result->who[b - a] = '\0';
    }
    return line;
}


private str WriteScore(str buf, Score *tosave)
{
    // "never" timestamps are written as 1900-01-01T00:00:00
    int cc = sprintf(buf, "%ld %04u-%02u-%02uT%02u:%02u:%02u |%s\n",
                     (long) tosave->howmuch, tosave->when.tm_year + 1900, tosave->when.tm_mon + 1, tosave->when.tm_mday + !tosave->when.tm_mday,
                     tosave->when.tm_hour, tosave->when.tm_min, tosave->when.tm_sec, tosave->who);
    return buf + cc;
}


private void DisplayScore(Score *s)
{
    char datebuf[16];
    if (s->when.tm_year > 0)
        sprintf(datebuf, "%u/%u/%04u", s->when.tm_mon + 1, s->when.tm_mday, s->when.tm_year + 1900);
    else strcpy(datebuf, "(...never)");
    printf("   %12s    %10s    %s\n", format32(s->howmuch), datebuf, s->who);
}


private int InsertScore(Score *scoreset, int scorecount, Score *current)
{
    int p, q;
#ifdef ONE_SCORE_PER_PERSON
    // suitable for lan or timesharing use, where people have locally distinct usernames and might compete
    for (p = 0; p < scorecount; p++)
        // empty usernames have null semantics -- they are not equal to each other, and all stay
        if (scoreset[p].howmuch > current->howmuch && current->who[0] && !strcmp(scoreset[p].who, current->who))
            return -2;                  // the same user already has a higher score in the list

#endif       // one compiler messes up unless this has blank lines around it!

    // alternative is suitable for single-user-ish use, where one person might want to see a history of scores
    for (p = scorecount; p > 0 && scoreset[p - 1].howmuch < current->howmuch; p--)
        ;
    if (p >= scorecount) return -1;     // the user's score was below the bottom one in the list
    for (q = scorecount - 2; q >= p; q--)
        scoreset[q + 1] = scoreset[q];
    scoreset[p] = *current;
    return p;
}


// This is suitable for small installations where simultaneous players are unusual.
private void SaveTopTenLocally(int currentscore, struct tm now,
                               int32 *yearplace, int32 *alltimeplace, int32 *bottomplace,
                               Score *topyear, Score *topalltime, Score *bottom)
{
    import str LoadScoreFile(void), UserName(void);
    import void SaveScoreFile(str contents);
    Score current;
    int i, j;
    static char outbuf[4096];   // we don't have much stack in some old platforms
    str scores, out = outbuf;

    current.howmuch = currentscore;
    current.when = now;
    strcpy(current.who, UserName());  // will not return oversize result

    if ((scores = LoadScoreFile())) {
        // the score file contains fourteen lines: top ten for year, top three all time, and bottom 
        for (i = j = 0; i < 10; i++, j++) {
            scores = ReadScore(scores, topyear + j);
            if (mktime(&current.when) - mktime(&topyear[j].when) > (time_t) 86400 * (time_t) 365)
                j--;     // discard from list
        }
        for (; j < 10; j++)
            memset(topyear + j, 0, sizeof(Score));
        for (i = 0; i < 3; i++)
            scores = ReadScore(scores, topalltime + i);
        ReadScore(scores, bottom);
    }

    *yearplace = InsertScore(topyear, 10, &current);
    *alltimeplace = InsertScore(topalltime, 3, &current);
    *bottomplace = -1;
    if (currentscore < bottom[0].howmuch && !cheated)
        *bottomplace = 0, bottom[0] = current;
    for (i = 0; i < 10; i++)
        out = WriteScore(out, topyear + i);
    for (i = 0; i < 3; i++)
        out = WriteScore(out, topalltime + i);
    WriteScore(out, bottom);
    SaveScoreFile(outbuf);
}


private void SaveTopTenToServer(int currentscore, struct tm now,
                                int32 *yearplace, int32 *alltimeplace, int32 *bottomplace,
                                Score *topyear, Score *topalltime, Score *bottom)
{
    import bool WillScoreNeedUserName(int score);
    import void RememberUserName(str username), GetLine(str prompt, str line12);
    import str GetScoresFromDatabaseAfterUpdate(str who, struct tm when, int32 howmuch, int32 *yearplace, int32 *alltimeplace, int32 *bottomplace);
    import str UserName(void);
    import char line1[];
    str scores, un = UserName(), qq = "\nWhat name should I use for your high score?  ";
    int i;

    if (!un || !*un)
        if (!WillScoreNeedUserName(currentscore))
	    un = "(anonymous)";       // allow low scorers to hide their shame
	else {
            do
                // XXX array index out of bounds in readToBufferFromSimulatedTerminal?  not replicating
                for (GetLine(qq, un = line1); *un && (unsigned) *un <= ' '; un++) ;
            while (!*un);
            RememberUserName(un);
        }
    scores = GetScoresFromDatabaseAfterUpdate(un, now, currentscore, yearplace, alltimeplace, bottomplace);
    for (i = 0; i < 10; i++)
        scores = ReadScore(scores, topyear + i);
    for (i = 0; i < 3; i++)
        scores = ReadScore(scores, topalltime + i);
    ReadScore(scores, bottom);
}


private void TopTen(int32 currentscore)
{
    import bool ScoreDatabaseAvailable();
    import int Width(void);
    Score *topyear, *topalltime, *bottom;
    time_t t = time(null);
    struct tm now;
    int32 yearplace, alltimeplace, bottomplace;
    int i;
    char a;

    now = *localtime(&t);
    topyear    = calloc(10, sizeof(Score));
    topalltime = calloc( 3, sizeof(Score));
    bottom     = calloc( 1, sizeof(Score));

    if (ScoreDatabaseAvailable())
        SaveTopTenToServer(cheated ? currentscore + 10000000 : currentscore, now,
                           &yearplace, &alltimeplace, &bottomplace,
                           topyear, topalltime, bottom);
    else
        SaveTopTenLocally(currentscore, now, &yearplace, &alltimeplace, &bottomplace,
                          topyear, topalltime, bottom);

    if (alltimeplace == 0 && currentscore >= 1000 && Width() >= 78)
        putsEvent("\nW*O*W!!!!    YOUR SCORE IS THE\a\n\
   @      @         @                   @@@@@@@    @@@@@    @     @   @@@@@@@\n\
  @ @     @         @                      @         @      @@   @@   @\n\
 @   @    @         @                      @         @      @ @ @ @   @\n\
@@@@@@@   @         @         @@@@@@@      @         @      @  @  @   @@@@\n\
@     @   @         @                      @         @      @     @   @\n\
@     @   @         @                      @         @      @     @   @\n\
@     @   @@@@@@@   @@@@@@@                @       @@@@@    @     @   @@@@@@@\a\n\
\n\
@     @    @@@@@      @@@@@    @     @   @@@@@@@    @@@@@    @@@@@@@    ##\n\
@     @      @       @     @   @     @   @         @     @      @       ##\n\
@     @      @      @          @     @   @         @            @       ##\n\
@@@@@@@      @      @          @@@@@@@   @@@@       @@@@@       @       ##\n\
@     @      @      @    @@@   @     @   @               @      @       ##\n\
@     @      @       @     @   @     @   @         @     @      @\n\
@     @    @@@@@      @@@@@    @     @   @@@@@@@    @@@@@       @       ##\a\n\n");
    else if (alltimeplace == 0 && currentscore >= 1000)
        putsEvent("\nW*O*W!!!!    YOUR SCORE IS THE ALL-TIME HIGHEST!");
    else if (alltimeplace == 0)
        putsEvent("\nWOW, your score is the highest so far!");
    else if (alltimeplace > 0)
        putsEvent("\nWOW, you've got one of the top three scores of all time!");
    else if (yearplace >= 0)
        putsEvent("\nHey, you've made it onto the list of the top ten high scores of the last year!");
    else if (yearplace < -1)
        putsEvent("\nThat would have made it into the top ten, but it's not your best score.");
    else if (bottomplace >= 0)
        putsEvent("\nCONGRATULATIONS!!  You have achieved the lowest score of all time!");
    if ((currentscore > 0 && bottomplace < 0 && yearplace < 0) || cheated) {
        a = GetLetter("Ya wanna see the high score list?  ");
        if (a == 'y') bottomplace = 1;
    }
    if (yearplace < 0 && bottomplace < 0) return;

    puts("\nTop ten scores of the last year:");  /* \n\
          Score    Date          Username\n\
          -----    ----          --------");  */
    for (i = 0; i < 10; i++)
        DisplayScore(topyear + i);
    puts("\nAll-time highest and lowest scores:");  /* \n\
          Score    Date          Username\n\
          -----    ----          --------");  */
    for (i = 0; i < 3; i++)
        DisplayScore(topalltime + i);
    putchar('\n');
    DisplayScore(bottom);
}



private void score(str e, int32 s)
{
    if (shortgame && s > 0) s = s * 3 / 2;
    if (s < -99999 || s > 999999)
        printf("%-32s %11s points\n", e, format32(s));
    else printf("%-35s %8s points\n", e, format32(s));
    totalscore += s;
}



private void RateScore(int32 score)
{
    if (score <= -1000)
        puts("The CIA is appalled at your performance and hurries to cover up its involvement.");
    else if (score < 0)
        puts("That is a miserable score, strictly desk jockey class.");
    else if (score < (shortgame ? 225 : 150))
        puts("The CIA director privately calls your performance \"really sloppy\".");
    else if (score < (shortgame ? 450 : 300))
        puts("Your work is unofficially evaluated as \"well, not fantastic...\"");
    else if (score < (shortgame ? 750 : 500))
        puts("Your superior tells you that you did a half-way decent job.");
    else if (score < (shortgame ? 1200 : 800))
        puts("The CIA director praises you for an \"excellent job.  Really good!\"");
    else if (score < (shortgame ? 3000 : 2000))
        puts("Your accomplishments are called \"astounding\"!  The president himself \
decorates you.  You have a great future, just beginning, in the CIA.");
    else
        puts("Your accomplishments are so amazing, so unbelievable, that while half of \
Washington is congratulating you, the other half is convinced you are a Lugonian spy!");
}



private void Points(void)
{
    int s, undealtwith, missed = 0;
    Meaning o;
    int32 gloat;

    if (death == jumped) { /* death #15 */
        putsAlarm("\n|     A\n\
|___]    A\n\
|          A\n\
|\n\
|            A\n\
|\n\
|\n\
|             A\n\
|\n\
|\n\
|             !");
        if (Have(onitro)) {
            putsAlarm("|          \\  `  %\n\
|        * !BLAMM!! =    (your nitro detonated.)\n\
================================");
            bystanders = true;
        } else {
            putsAlarm("|\n\
|          >splat!<\n\
================================");
            if (strong) {
                putsEvent("\n!!!  My god, you survived the fall!");
                death = escaped;     /* un-death */
                if (beastfollowing)
                    putsEvent("\nThe Emperor's animal does not follow you down.");
                beastfollowing = false;
            }
        }
    }
    if (death == escaped && (sickness >= moderate || (badsandwich && Have(osandwich))))
        wipeout = true;
    if (wipeout) {
        bystanders = false;
        putsAlarm("\nUnfortunately, you have carried the dreaded Lugonian Plague out to the \
city, and wiped out eighty percent of the population!");
    }
    if (wipeout || death != escaped)
        fungus = false;
    if (fungus) putsAlarm("\nOops, you've spread the irritating fungus to the general population; the \
people of the city are quite annoyed at you.");
    GetLetter("\n         Press Return To See Your Score: ");
    if (yourroom == incar) {
        for (o = firstobject; o <= lastobject; o++)
            if (location[o] == incar || location[o] == intrunk)
                location[o] = pockets;
        gleepct[pockets] += gleepct[incar];
    }

    puts("\n\n\n\n******************** YOUR SCORE ********************");
    switch (death) {
        case escaped:    score("Escaping alive:", 200); break;
        case eaten:      score("Being eaten:", -200); break;
        case jumped:     score("Jumping to your death:", -300); break;
        case killed:     score("Getting killed:", -200); break;
        case chicken:    score("Chickening out:", -500); break;
        case sick:       score("Getting sick and dying:", -200); break;
        case ohdee:      score("Overdosing on pills:", -300); break;
        case worsethandeath:
                         score("Suffering fate worse than death:", -200); break;
        case bloneparte: score("Blowing yourself to bits:", -300); break;
        case sluggard:   score("Being too slow:", -200); break;
        case hacked:     score("Being hacked:", -65536); break;
        case stupidity:  score("Dying of sheer stupidity:", -300); break;
    }
    if (!guardalive) score("Killing the guard:", 50);
    if (!runtalive)  score("Eliminating the unhealthy runt:", 50);
    if (burnt)       score("Destroying man eating plants:", 50);
    if (fungus)      score("Spreading the fungus:", -100);
    if (attached && death == escaped)
                     score("Disgusting thing on your leg:", -50);
    if (bystanders)  score("Killing innocent pedestrians:", -1000);
    if (clean)       score("Cleaning the bathroom:", 10);
    if (badhygiene)  score("Poor personal hygiene:", -20);
    if (death == escaped) {
        s = Have(okeys) + Have(ochemicals) + Have(opheromone) +
                        Have(ofeather) + Have(oflysol) + Have(ostatuette) +
                        Have(osandwich) + Have(opills);
        missed = 8 - s;
        score("Bringing out alien objects:", 20 * s);
        gloat = gleepct[pockets];
        if (Have(obag)) gloat += gleepct[inbag];
        if (Have(ogalloncan)) gloat += gleepct[ingalloncan];
        if (shortgame) gloat <<= 1;
        score("Bringing out gleeps:", gloat);
        if (beastfollowing)
            score("Bringing out Emperor's aminal:", 100);
    }
    score("Rooms explored:", (death == escaped ? beenin * 4 : beenin));
    if (beenin == nrooms)
        score("Exploring every room:", 200);
    undealtwith = (int) guardalive + (int) !burnt + (int) (runtroom && runtalive) + (int) !beastfollowing;
    if (!undealtwith)
        score("Taking out all dangerous entities:", 200);
    if (wipeout)
        score("Wiping out the city:", -1000000);
    if (cheated)
        score("Cheating:", -10000000);
    if (death == escaped && beenin == nrooms && !missed && !undealtwith
                         && !wipeout && !bystanders && !fungus && gloat > 0)
        score("ONE HUNDRED PERCENT RUN:", 1000);    // don't require bathroom/hygiene
    puts("                                ------------");
    printf("TOTAL SCORE:                    %12s POINTS.\n", format32(totalscore));
    puts("****************************************************\n");

    if (death == escaped && (missed || undealtwith)) {
        char obz[20], enemeez[40];
        if (!missed) strcpy(obz, "");
        else sprintf(obz, missed == 1 ? "one alien object" : "%d alien objects", missed);
        if (!undealtwith) strcpy(enemeez, "");
        else sprintf(enemeez, undealtwith == 1 ? "one live alien entity" : "%d live alien entities", undealtwith);
        printf("(You left behind %s%s%s.)\n\n", obz, missed && undealtwith ? " and " : "", enemeez);
    }
    RateScore(!cheated ? totalscore : totalscore + 10000000);
    TopTen(totalscore);
}



int main(int argc, char **argv)
{
    import bool CreateDictionary(void);
    import void AskCommand(str prompt);
    import void Randomize(void), MakeMap(void), StartTimer(void), Situation(void),
                HearAndObey(void), local_startup(str), local_shutdown(void);
#if defined(_DEBUG) && defined(MAP_TEST_HARNESS)
    bool TestHarness_MakeMap(void);
#endif

    local_startup(argv[0]);
#if defined(_DEBUG) && defined(MAP_TEST_HARNESS)
    if (argc == 2 && !strcmp(argv[1], "testmaps")) {
        int r = TestHarness_MakeMap();
        local_shutdown();
        return r;
    }
#endif
    puts("\n\nThis here is the game of Lugi, by Jay Wilson and Paul Kienitz.");
    if (!CreateDictionary()) exit(10);   // diction.c
    Randomize();
    MakeMap();                           // embassy.c
    Inform();
    StartTimer();
    while (!death) {
        if (did) Situation();            // player.c
        lastenv = env;
        AskCommand("  ");    /* an aesthetically pleasing prompt */
        HearAndObey();                   // obey.c
        if (yourroom <= pbalcony && !map[yourroom].explored) {
            beenin++;
            map[yourroom].explored = true;
        }
    }
    Points();
    putsEvent("\nBye...  Wasn't that FUN?");
    local_shutdown();
    return 0;
}
