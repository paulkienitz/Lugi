/* LUGI.H
   These are the complex types used for the adventure game Lugi, in which
   there are more ways to die than there are rooms in the map. */

#include <string.h>

#undef  private
#define private static
#define import extern
#define PUBLIC

#ifdef putchar
#  undef putchar
#endif
#if defined(_MSC_VER) || defined(__EMSCRIPTEN__)
// note that UNICODE is defined for MSVC, which is good so GetCurrentDirectory etc won't
// fail due to funny chars... Watcom is apparently not supporting UNICODE at this time
#  define putchar Lugi_putchar
#  define printf  Lugi_printf
#  define puts    Lugi_puts
#  define sscanf  Lugi_sscanf
#  ifndef __STDC__    // allowing Microsoft extensions undefines this
#    define __STDC__ 0
#  endif
#endif

#if defined(AZTEC_C) && !defined(INT32)
#  define INT16
typedef long int32;
typedef unsigned long uint32;
import uint32 RRand32(uint32 range);
#endif
#if (defined(__WATCOMC__) && (defined(MSDOS)) || (defined(__DOS__)) && !defined(INT32) && !defined(__FLAT__))
#  define INT16
typedef long int32;
typedef unsigned long uint32;
import uint32 RRand32(uint32 range);
#endif
#if !defined(INT16)
typedef int int32;
typedef unsigned int uint32;
#  define RRand32 RRand
#endif

// the original intent was to limit names to 64 chars; we allow extra for multibyte users
#ifdef __EMSCRIPTEN__
#  define USERNAMELEN 256
#else
#  define USERNAMELEN 128
#endif

typedef short bool;
#define false 0
#define true 1

#define null ((void *) 0)

typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef char *str;

#define bit(B) ((uint32) 1 << (int) (B))


/* First we have the list of all the different meanings that a word can be
   understood to have: */

typedef enum {
        nomeaning,
                        /* objects: */
        orope, obag, ohammer, onitro, okeys, oflask, ophial, ogalloncan,
        ochemicals, owater, ourine, omatch, opheromone, ofeather, oacetone,
        oskull, osandwich, olysol, oflysol, oblackflag, ostatuette, opills,
                        /* the object that isn't: */
        gleeps,
                        /* furniture (immovable pseudo-objects): */
        fstove, fnewspapers, fbooks, fwhips, fchains, fbadge, fpaintings,
        fdevice, fsign, fcircuitry, fcar, fmachine, fdirt, fbones, flaundry,
        fgags, ftar, fagents, fcudgel, fbox, fbattleship, fcrystal, ftrash,
        ffeces, fcentrifuge, fliquid, fwindow,
                        /* inhabitants (living furniture): */
        fvermin, flizards, fplants, fghost, frats, fbirds, fwumpus, fgnome,
                        /* active living entities: */
        lscalything, lgorilla, lguard, lrunt, lflies, lbeast, lfungus,
                        /* verbs!: */
        vnorth, vsouth, veast, vwest, vjump, vclimb, vforward, vbackward,
        vtake, vput, vfill, vdrop, vthrow, vinventory, vignite, vkill, vopen,
        veat, vdrink, venter, vleave, vhelp, vread, vshut, vspit, vrun, vpour,
        vbutton, vspray, vshit, vpiss, vfart, vfuck, vflush, vplug, vunplug,
        vstart, vfind, vbust, vlook, vdetonate, vtickle, /* vshell, */ vquit,
#ifdef _DEBUG
        cheatest,
#endif
                        /* miscellaneous and ambiguous: */
        mup, mdown, min, mout, mit, mall, meverything, mnot, mshelf, mcan,
        mcans, mspraycan, mfrom, mwith, mignore
        // fliquid arguably belongs in this category too, but moving it here adds problems
} Meaning;


#define lastmeaning mignore
#define lastobject  opills
#define firstobject orope
#define lastalive   lfungus
#define firstalive  fvermin
#define lastnoun    lfungus
#define firstverb   vnorth
#define lastverb    vquit

#define meaningcount ((int) lastmeaning + 1)
#define objectcount  ((int) lastobject + 1)

#define isob(O)  ((O) && (O) <= lastobject)
/* true if O is a genuine object, false for gleeps, furniture, etc */
#define isnoun(O) ((O) && (O) <= lastnoun)

#define _liquidMask1 (bit(owater) | bit(ochemicals) | bit(ourine))
#define _liquidMask  (_liquidMask1 | bit(opheromone) | bit(oacetone))
#define liquid(O) ((O) <= lastobject && bit((O)) & _liquidMask)

#define ndirec 6

#define drec(V)  ((ushort) (V) - (ushort) vnorth)
/* V should be a Meaning in vnorth..vclimb */

/* You can tell this is translated from Pascal, can't you? */


/* And now we have the tokens that represent the various possible appearances
   that a room can have -- these correspond to text descriptions of rooms: */

typedef enum {
        cabinet, niche, bunker, buzzcr, garden, car, driverseat, pillpile, nasa,
        balcony, arcturus, fogarc, fogsplash, fogged, burntplants, cleanbathroom,
        nasawreck, ghostbunker, scaredghost, ghost, tank, cistern,
        // appearances from here on are used randomly, those above are used more purposefully
        bathroom, meadow, seep, kerosene, bubble, plainwhite, bstudy, cube,
        rat, streetlight,  // appearances from here on can't be referenced by 32 bit mask:
        library, laundry, office, caf, birdcage, operate, sea,
        joke, wumpus, alcove, carvedfloor, picturetube, circuits, airlock,
        lizardhole, device, torture, vent, centralcr, machine, drydock,
        sooty, freebirds, tar, archive, peekhole, mensroom, stuffed,
        hackerroom, rotate, brightpoint, indesc, rec, largex, largey, meat,
        green, pentagonal, columns, mirrorcube, mtking, waxarmy, pancreas,
        glowtube, conpipe
} Appearance;


#define firstappearance cabinet
#define lastappearance  conpipe
#define firstoptional   ghost      // tank and cistern are mandatory between them -- a special case

#define appearancecount  ((int) lastappearance + 1)

/* More appearences are always welcome; just update the array of text
   descriptions to correspond. */


/* And now, the range of places a thing can be in.  Each place that the
   player can be in [r1..incar] has an appearence.  Those places in this
   range which have fixed appearences have names corresponding to the correct
   appearences with the letter p prepended.  Places named r<nn> have random
   appearences. */

typedef enum {
        nowhere,           /* the 35 rooms of the map: */
        r1, r2, r3, pcabinet, r5, r6, r7, pniche, r9, r10, r11, pbunker,
        r13, r14, r15, pbuzzcr, r17, r18, r19, pgarden, r21, r22, pcar,
        r24, r25, pnasa, r27, r28, ppillpile, r30, r31, pcistank, r33, r34,
        pbalcony,          /* plus two special places the player can go: */
        parcturus, incar,  /* and places where objects but not living things can be: */
        incabinet, inniche, intrunk, incentrifuge, onarcshelf,
        inbag, inflask, intinybottle, ingalloncan, pockets
} Place;

#define nrooms  ((int) pbalcony)
/* nrooms is the number of different places the player can be in, not
   counting parcturus and incar which are not part of the regular room map */

#define placecount ((int) pockets + 1)

typedef struct {
    Appearance features;
    Place paths[ndirec];
    bool explored;
} Roomt;

typedef enum {none, mild, moderate, severe, critical} Sickness;

typedef enum {
    alive /* !death */, escaped, eaten, jumped, killed, chicken, sick, ohdee,
    worsethandeath, bloneparte, sluggard, stupidity, hacked
} Death;

typedef enum {nosight, ontrail, detected, cries} Warning;

/* typedef struct {
    Meanimg preposition;
    Meaning nominal;
} PrepositionPhrase; */


typedef int32 Seconds;                  /* measures of duration */
#define never 0x7fffffffL


#define GleepClumping 10
/* higher number means more likelihood of new gleeps appearing with old ones */

#define basic_weightlimit 9


        /* Some imports that everybody always uses: */

import Place yourroom;
import Appearance env, lastenv;
import Meaning verb, it;
import Death death;

import str verbword, lastword, bogusword;
import bool did;

import Meaning GetWord(void);
import Meaning UnGetWord(void);
import char GetLetter(str prompt);

import unsigned RRand(unsigned range);

import bool Here(Meaning o), HereS(Meaning o), Have(Meaning o), DHave(Meaning o);
import bool Empty(Place p);

import Seconds Timer(void);

import Place Distant(Place avoid);
import Meaning Opposite(Meaning d);

// no stdio except in basics.c, which defines all IO functions we use:
import int putchar(int ch);
import int puts(const char *s);
import int printf(const char *f, ...);
import int sprintf(char *str, const char *format, ...);
import void put(register const char* s);
import str format32u(uint32 v);

import void colorEvent(void), colorAlarm(void), colorNormal(void);
import void putsEvent(str s), putsAlarm(str s);
import void printfDebug(const char *f, ...);


import Place location[objectcount];

import Roomt map[nrooms + 1];

import uint32 gleepct[placecount];

#define inwater (env == cistern || env == tank || env == fogsplash)
