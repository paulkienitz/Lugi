/* DICTION.C
   This here is the stuff that translates words into meanings in Lugi. */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lugi.h"

#if defined(__GNUC__) || defined(__TINYC__)
#  define stricmp strcasecmp
int strcasecmp(const char*, const char*);
#endif


#define MAXMEANINGS 30


import char line1[], line2[];

// import char localshellwords[];

import void GetLine(str prompt, str line12);


PUBLIC str bestword[/* objectcount + 1  XXX */ meaningcount];
    /* pointers to the preferred words for at least orope..gleeps */

#ifdef _DEBUG
PUBLIC str appearanceword[appearancecount] = {
    "cabinet", "niche", "bunker", "buzzcr", "garden", "car", "driverseat", "pillpile", "nasa",
    "balcony", "arcturus", "fogarc", "fogsplash", "fogged", "burntplants", "cleanbathroom",
    "nasawreck", "ghostbunker", "scaredghost", "ghost", "tank", "cistern",
    "bathroom", "meadow", "seep", "kerosene", "bubble", "plainwhite",  "bstudy", "cube",
    "rat", "streetlight", "library", "laundry", "office", "caf", "birdcage", "operate", "sea",
    "joke", "wumpus", "alcove", "carvedfloor", "picturetube", "circuits", "airlock",
    "lizardhole", "device", "torture", "vent", "centralcr", "machine", "drydock",
    "sooty", "freebirds", "tar", "archive", "peekhole", "mensroom", "stuffed",
    "hackerroom", "rotate", "brightpoint", "indesc", "rec", "largex", "largey", "meat",
    "green", "pentagonal", "columns", "mirrorcube", "mtking", "waxarmy", "pancreas",
    "glowtube", "conpipe"
};
#endif

PUBLIC Meaning verb, it;    /* in the last command sentence entered */

PUBLIC str verbword, lastword, bogusword;

private str realwords[MAXMEANINGS]; /* to aid in fine discrimination */

private Meaning command[MAXMEANINGS];

private int comwords, currentword;  /* 0..MAXMEANINGS */


#define Dictentry struct dicTentrY

Dictentry {             /* a binary tree node */
    Dictentry *before, *after;
    str word;
    Meaning means;
};

typedef Dictentry *Dictp;

#define VOCABLIMIT 500

private Dictentry dick_space[VOCABLIMIT];   /* avoid malloc */

private Dictp dictionary[26];
/* the dictionary consists of one binary tree for each initial letter.  This
   fancy a structure is hardly necessary for such a small application, but
   what the hell.  This 26-binary-trees approach was sometimes used by compiler
   writers back in the days when they didn't have to deal with extended alphabets.
   (For unicode I'm thinking of a hash of hashes based on first character or two,
   if a single big dictionary collection isn't fine already for performance.) */

/* The word fields point to words in this block of text, where the spaces are
   replaced with nuls by the routine that sorts them out: */

private str rawdictionary[meaningcount + 1] = {
    null,
    /* OBJECTS: */
    "rope grappling hook cord",
    "bag sack canvas",
    "sledgehammer sledge hammer maul mallet",
    "nitroglycerine nitro explosive explosives bomb canister",
    "keys keyring key glued glue",      // cheap synonym
    "flask beaker glassware glass",
    "phial bottle",                 /* XXX  "bottle" is biguous! */
    "gallon gal",
    "chemicals mysterious chem",
    "water h2o",
    "urine piss pee peepee",        /* can mean urinate */
    "match",
    "pheromone perfume",
    "feather quill",
    "acetone solvent",
    "skull skulls",
    "sandwich food bread",
    "lysol disinfectant",
    "flysol infectant",
    "insecticide flag blackflag raid",     // see hack in AskCommand for "black flag"... "raid" was used in early Lugi
    "statuette statue sculpture figurine",
    "pills capsules pill capsule drug drugs medicine dope caps tabs",
    "gleeps gleep",
    /* FURNITURE: */
    "stove",
    "newspapers papers paper newspaper",    /* papers could mean lizards */
    "books book tome tomes",    // volume?
    "whips whip lash lashes scourges scourge",
    "chains chain tongs maiden hibachi barbecue table",
    "badge",
    "furnishings furniture paintings painting",
    "device apparatus gizmo",   /* could mean machine, circuitry, box, etc. */
    "sign signs placard",       /* vermin could mean lizards or rats */
    "circuitry circuits circuit electronic electronics",    /* = device */
    "car automobile vehicle sedan",
    "machine mechanism mechanical", /* could mean device */
    "dirt soil loam",
    "bones bone skeleton",      /* could mean skull */
    "laundry clothes clothing",
    "gags gag",  /* XXX  handle "cloth"? */
    "tar barrels barrel creosote keg kegs",
    "agents stuffed agent colleague",
    "cudgel bludgeon truncheon club nightstick",  /* XXX  handle "stick"? */
    "box boxes",
    "battleship ship boat",  /* XXX  handle "vessel"? */
    "crystal",
    "garbage trash wrapper wrappers rubbish cruft",
    "excrement feces fecal doodoo poopoo poop poo caca",
    "centrifuge nasa cage",
    "liquid fluid juice",    /* this was originally mliquid rather than fliquid, and maybe still should be */
    "window",
    /* INHABITANTS (LIVING FURNITURE): */
    "vermin varmints varmint critter critters burrowing animals",
    "lizards forms lizard",
    "plants bushes flora plant bush tree trees",
    "ghost spirit apparition",
    "rats rat rodents rodent",
    "birds bird avian",
    "wumpus",
    "gnome hacker nerd nurd geek dweeb techie programmer gamer dork",
    /* ACTIIVE LIVING THINGS: */
    "thing scaly squamous",          /* "thing" could mean "it" */
    "gorilla ape monkey primate simian gorrila",
    "guard guards lugiman lugonian lugimen lugonians enemy enemies",     /* can also mean runt */
    "runt unhealthy allergic",
    "flies insects bugs bug insect fly",
    "beast animal creature",    /* can also mean vermin or gorilla or lizard */
    "fungus",   /* XXX  handle "infection" */
    /* VERBS: */
    "n north northward northwards",
    "s south southward southwards",
    "e east eastward eastwards",
    "w west westward westwards",
    "j jump descend downward downwards leap",
    "c climb ascend upward upwards",
    "f forward forwards onward onwards proceed",
    "b backward backwards retreat return back",
    "take carry pick get tote grab seize schlep shlep",
    "put set place give",
    "fill load",
    "drop discard",
    "throw toss hurl fling",
    "i inv inventory",
    "light ignite spark",   /* XXX  handle ambiguous "strike" */
    "attack kill murder slay fight hurt wound hit kick clobber beat punch injure maim whack",
    "open unlock uncap",
    "eat chew devour swallow",      /* can also mean drink */
    "drink gulp sip slurp imbibe guzzle chug chugalug",
    "enter inwards through",        /* "through" is cheap bandaid */
    "leave outside escape exit vacate",
    "help",
    "read peruse",
    "shut close cap",
    "spit expectorate",
    "run walk crawl scram go",
    "pour dump spill empty",
    "switch buttons switches controls button",
    "spray squirt spritz atomize",
    "shit defecate",            /* can also mean feces */
    "urinate",
    "fart",
    "fuck screw hump rape fornicate intercourse buttfuck swive",
    "flush",
    "plug",
    "unplug",
    "start",
    "find where explore locate",
    "break bust smash damage destroy wreck",
    "peek look examine peep peer",
    "blast detonate explode",
    "tickle",
//    "shell",       // replaced by local string in basics.c
//    "pause",       ...nope, not adding it
    "quit halt stop die",         // XXX  handle "give up"?

#ifdef _DEBUG
    "mantergeistmann cheatest ct",
#endif
    /* MULTIPURPOSE WORDS: */
    "up u",
    "down d",
    "in on onto into inside",
    "out",
    "it that him her",
    "all those every them both",
    "everything",
    "not dont never",
    "shelf trunk boot niche cabinet",        /* cheap synonyms */
    "can",              /* XXX  SOMEDAY understand "container"? */
    "cans spraycans",
    "spraycan",
    "from",
    "with by",     // near?  next (to)?  should we add a word mto, and what should it do?
    "a the an another some of other now",   /* mignore */
    null
};
/* Each of those strings consists of synonyms for the corresponding meaning.
   The one listed first is the one the program prefers to use.  We are not
   including the five-letter-abbreviation feature of the original Lugi.  The
   words in the dictionary MUST be all lowercase. */



PUBLIC bool CreateDictionary(void)     /* call this during initialization */
{
    register int l;
    int glans = 0;
    Meaning m;
    Dictp *d;      /* pointer to pointer */
    str ww, wrd;
    str w;
    bool best;

//    rawdictionary[vshell] = localshellwords;    /* defined in basics.c */
    for (l = 0; l < 26; l++)
        dictionary[l] = null;
    for (m = firstobject; m <= lastmeaning; m++) {
/*        best = m <= lastobject + 1;  XXX */  best = true;
        if (!(w = rawdictionary[m])) {
            puts("\nINTERNAL ERROR:  not enough words in raw dictionary for all meanings.");
            return false;
        }
        while (*w) {
            for (ww = w; (unsigned) *ww > ' '; ww++) ;
            wrd = malloc(ww + 1 - w);
            strncpy(wrd, w, ww - w);
            wrd[ww - w] = '\0';
            d = dictionary + (*wrd - 'a');
            while (*d) {
                l = strcmp(wrd, (*d)->word);
                if (!l) {
                    printf("\nINTERNAL ERROR: word \"%s\" has two different meanings.\n", wrd);
                    return false;
                }
                if (l < 0) {
                    d = & (*d)->before;
                } else {
                    d = & (*d)->after;
                }
            }
            if (glans >= VOCABLIMIT) {
                puts("\nINTERNAL ERROR:  need to increase VOCABLIMIT bound.");
                return false;
            }
            *d = dick_space + glans++;
            (*d)->word = wrd;
            (*d)->means = m;
            (*d)->before = (*d)->after = null;
            if (best) bestword[m] = wrd;
            if (*ww) ww++;
            w = ww;
            best = false;
        }
    }
    bestword[ogalloncan] = "one gallon can";
    bestword[oblackflag] = "black flag";
    bestword[nomeaning] = "nothing";
    if (rawdictionary[m]) {
        puts("\nINTERNAL ERROR:  too many words in raw dictionary for given meanings.");
        return false;
    } else return true;
}



PUBLIC Meaning Understand(str werd)     // werd must consist solely of lowercase ascii letters a-z
{
    int firstletter = *werd - 'a';
    register Dictp dtree = dictionary[firstletter];
    int zult;

    while (dtree)
        if (!(zult = strcmp(werd, dtree->word)))
            return dtree->means;
        else if (zult < 0)
            dtree = dtree->before;
        else
            dtree = dtree->after;
    return nomeaning;
}



private void Snip(int n)
{
    int j;
    for (j = n + 1; j < comwords; j++) {
        command[j - 1] = command[j];
        realwords[j - 1] = realwords[j];
    }
    comwords--;
}


PUBLIC void DiscardLastWordGotten(void)
{
    if (currentword <= comwords)
        Snip(currentword--);
}


#define canMask (bit(oacetone) | bit(olysol) | bit(oflysol) | bit(oblackflag))

private void PreBiguate(void)
{
    import bool attached, beastfollowing;
    bool gallonflag = false, nound = false;
    int c, canplace = 0, sprayplace = 0;
    register Meaning cc;
/* #ifdef _DEBUG
puts("The original un-prebiguated meanings are:");
for (c = 0; c < comwords; c++) {
 putchar(' ');
 put(bestword[command[c]]);
}
puts(".  XXX");
#endif */

#ifdef _DEBUG
    if (verb == cheatest) return;
#endif
    // use surroundings to better interpret what the user probably means by ambiguous words
    for (c = 1; c < comwords; c++) {
        cc = command[c];
        if (cc == vspray) cc = command[c] = mspraycan;
        if (cc == mspraycan) sprayplace = c;
        if (cc == ogalloncan) gallonflag = true;
        if (cc == vshit) command[c] = ffeces;
        if (cc == vpiss) command[c] = ourine;
        if ((cc == vpour || cc == vshit) && verb == vtake) verb = vshit;     // "take a dump"
        if (cc == vwest && !stricmp(realwords[c], "w")) command[c]= mwith;
        if (cc == lguard && !Here(lguard) && Here(lrunt))
            command[c] = lrunt;
        if (cc == lbeast && env == pillpile && !beastfollowing)
            command[c] = fvermin;
        if (cc == lbeast && env == rat && !beastfollowing)
            command[c] = frats;
        if (cc == lbeast && env == lizardhole && !beastfollowing)
            command[c] = flizards;
        if (cc == fvermin && env == rat)
            command[c] = frats;
        if (cc == fvermin && env == lizardhole)
            command[c] = flizards;
        if (cc == lscalything)
            if (command[c - 1] == mall && !stricmp(realwords[c - 1], "every")) {
                Snip(c--);          /* "every thing" */
                cc = command[c] = meverything;
            } else if (!attached && !stricmp(realwords[c], "thing"))
                command[c] = mit;
        if (cc == oflask && !Here(oflask) && !Have(oflask) && env == car && !stricmp(realwords[c], "glass"))
            command[c] = fwindow;
        if (cc == fmachine && env == device) command[c] = fdevice;
        if ((cc == fmachine || cc == fdevice) && env == circuits) command[c] = fcircuitry;
        if (cc == fmachine && env == nasa) command[c] = fcentrifuge;
        if (cc == fdevice && env == hackerroom) command[c] = fbox;
        if (cc == fliquid) {
            bool ph = Have(opheromone) || Here(opheromone), ch = Have(ochemicals) || Here(ochemicals),
                 ac = Have(oacetone) || Here(oacetone), wa = Have(owater) || Here(owater), ur = Have(ourine) || Here(ourine);
            if (ph) command[c] = opheromone;
            else if (ch && !ac && !wa) command[c] = ochemicals;
            else if (ac && !ch && !wa) command[c] = oacetone;
            else if (wa && !ch && !ac) command[c] = owater;
            else if (ur && !wa && !ac && !ch) command[c] = ourine;
        }
        // now comes the tricky part... arbitrary rule: "cans" generally means spraycans and does not include
        // acetone, and "can" means spraycan only if acetone is not present or a spray is explicitly mentioned
        if (cc == mcan) {
            if (sprayplace || gallonflag) {
                Snip(c--);
                cc = command[c];  // "spray can" or "gallon can"
            }
            else canplace = c;
        }
        if ((cc == mcans || (cc < 32 && cc & canMask)) && canplace) {   // "cans" or "can of x" or "can cans"
            if (command[c] == oacetone) command[c] = ogalloncan;
            Snip(canplace);
            canplace = 0;
            // cc = command[--c];
        }
        if (cc >= olysol && cc <= oblackflag && (sprayplace || canplace)) {    // "can of x"
            if (sprayplace) Snip(sprayplace), c--;
            if (canplace) Snip(canplace), c--;  // probly already snipped
            sprayplace = canplace = 0;
        }
        if (nound && cc == mit)
            Snip(c);
        if (isnoun(cc))
            nound = true;
    }
    if (canplace)
        if (Have(ogalloncan) || Here(ogalloncan)) {
            command[canplace] = ogalloncan;
            canplace = 0;
        } else if (!sprayplace)
            sprayplace = canplace;
    if (sprayplace) {
        bool l = false, f = false, b = false;
        if (verb != vtake) {
            l = Have(olysol);
            f = Have(oflysol);
            b = Have(oblackflag);
        }
        if (!(l | f | b)) {
            l = Here(olysol);
            f = Here(oflysol);
            b = Here(oblackflag);
        }
        if (l + f + b == 1) {
            if (l) command[sprayplace] = olysol;
            else if (f) command[sprayplace] = oflysol;
            else command[sprayplace] = oblackflag;
        }
    }
}



PUBLIC void AskCommand(str prompt)
{
    register str w, ww, www;
    str buf;
    register Meaning m;
    bool wasgood = true, black_flag = false;
    bool dupcatch[meaningcount];

    comwords = 0;
    command[0] = nomeaning;
    bogusword = lastword = null;
    for (m = nomeaning; m <= lastmeaning; m++)
        dupcatch[m] = false;
    dupcatch[mignore] = true;

    do
        for (GetLine(prompt, buf = line2); *buf && (unsigned) *buf <= ' '; buf++) ;
    while (!*buf);
    w = buf;
    while (*w && !isalpha(*w)) w++;

    while (*w && comwords < MAXMEANINGS) {
        ww = w;
        while (isalpha(*w) || *w == '\'') {
            if (*w == '\'') {
                for (www = w + 1; *www && (isalpha(*www) || *www == '\''); www++)
                    www[-1] = *www;
                *--www = ' ';
            } else
                w++;
        }
        while (*w && !isalpha(*w)) w++;
        for (www = ww; isalpha(*www); www++)
            *www = tolower(*www);
        *www = 0;

        m = Understand(ww);
        if (m) {
            if (!dupcatch[m]) {
                command[comwords] = m;
                if (black_flag && !strcmp(ww, "flag"))
                    realwords[comwords++] = "black flag";   // kluge
                else realwords[comwords++] = ww;
                dupcatch[m] = true;
            }
            wasgood = true;
        } else {
            if (wasgood)
                bogusword = ww;
            wasgood = false;
        }
        black_flag = !strcmp(ww, "black");
    }
    verb = *command;
    verbword = lastword = *realwords;
    currentword = 0;
    PreBiguate();
}



/*
// ****************  XXX BUG: THIS HAD AN UNEXPLAINED CRASH WHICH THE NEW ONE MAY NOT FIX!
// Namely, array out of bounds in dictionary[firstletter] in Understand, where firstletter = *werd - 'a',
// even though the input was a perfectly safe "ct urine" with no funny characters.
PUBLIC void BAD_AskCommand(str prompt)
{
    register str w, ww, www;
    str buf = line2;
    register Meaning m;
    bool wasgood = true, black_flag = false;
    bool dupcatch[meaningcount];

    do
        for (GetLine(prompt, buf); *buf && (unsigned) *buf <= ' '; buf++) ;
    while (!*buf);
    w = buf;
    comwords = 0;
    command[0] = nomeaning;
    bogusword = lastword = null;
    for (m = nomeaning; m <= lastmeaning; m++)
        dupcatch[m] = false;
    dupcatch[mignore] = true;
    while (*w && comwords < MAXMEANINGS) {
        ww = w;
        while (isalnum(*w) || *w == '\'') {
            if (*w == '\'') {
                for (www = w + 1; *www && isalnum(*www); www++)
                    www[-1] = *www;
                *--www = ' ';
            } else
                w++;
        }
        if (*w) *w = ' ';
        while (*w && !isalnum(*w)) w++;
        for (www = ww; (unsigned) *www > ' '; www++)
            *www = tolower(*www);
        *www = 0;

        m = Understand(ww);
        if (m) {
            if (!dupcatch[m]) {
                command[comwords] = m;
                if (black_flag && !strcmp(ww, "flag"))
                    realwords[comwords++] = "black flag";   // kluge
                else realwords[comwords++] = ww;
                dupcatch[m] = true;
            }
            wasgood = true;
        } else {
            if (wasgood)
                bogusword = ww;
            wasgood = false;
        }
        black_flag = !strcmp(ww, "black");
    }
    verb = *command;
    verbword = lastword = *realwords;
    currentword = 0;
    PreBiguate();
}
*/



PUBLIC Meaning GetWord(void)
{
    if (++currentword >= comwords) {
        currentword = comwords;
        return nomeaning; 
    } else {
        lastword = realwords[currentword];
        return command[currentword];
    }
}


/* to rewind all the way to the verb, go "while (UnGetWord()) ;" */
PUBLIC Meaning UnGetWord(void)
{
    if (currentword) {
        lastword = realwords[--currentword];
        return command[currentword];
    }
    else
        return nomeaning;
}


/*
// for more advanced grammatical processing, which we rarely need
PUBLIC Meaning ScanAheadForWord(Meaning wordToFind)
{
    int w;
    Meaning r;
    // This is used to search for a special modifier word like 'out' or 'from' or 'into'.
    // When one is found, it returns not that word, but the word after it if applicable,
    // like if you search for mout in the command 'take flask out of bag' it returns obag.
    // If no word comes after, it returns mignore, or nomeaning if the search word is absent.
    for (w = currentword; w < comwords; w++)
        if (command[w] == wordToFind) {
            Snip(w);
            if (w < comwords) {
                r = command[w];
                Snip(w);
                return r;
            } else return mignore;
        }
    return nomeaning;
}
*/



PUBLIC char GetLetter(str prompt)
{
    int i;
    GetLine(prompt, line1);
    for (i = 0; line1[i]; i++)
        if (isalpha(line1[i]))
            return tolower(line1[i]);
    return ' ';
}
