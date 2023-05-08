/* EMBASSY.C
   This here contains the stuff that represents the environment that the
   player is exploring in the game Lugi. */
//#define OLDRANDOM

#include <stdlib.h>
#include "lugi.h"


import ushort weightlimit;

import Sickness sickness;

import str bestword[], oddplaces[];


PUBLIC bool guardalive = true,          /* Lugonian guard is alive */
            goralive = true,            /* gorilla is alive */
            runtalive = true,           /* allergic runt is alive */
            burnt = false,              /* plants have been burnt up */
            glued = true,               /* keys are glued down */
            clean = false,              /* the bathroom is clean */
            chemgiven = false,          /* the niche has dispensed chemicals */
            plugged = false,            /* centrifuge was just plugged in */
            guardsees = false,          /* the guard has noticed you */
            plantslunged = false,       /* the plants have noticed you */
            badsandwich = true,         /* the sandwich is infected */
            cabinetopen = false,        /* the cabinet is open */
            wetmatch = false,           /* the matches are soggy */
            trunkopen = false,          /* the car trunk is open */
            cheated = false;            /* the test cheat has been used */


PUBLIC Warning lastwarning;          /* how close the pursuers are behind you */


PUBLIC Place thingroom,              /* where scaly thing is */
             gorroom,                /* where gorilla is */
             guardroom,              /* where guard is */
             runtroom;               /* where allergic lugiman is */


PUBLIC Roomt map[nrooms + 1];        /* map[nowhere] is unused -- */
                                     /*    too bad we can't index from 1  */

PUBLIC Place location[objectcount];  /* where the objects are, ditto*/


PUBLIC uint32 gleepct[placecount];   /* how many gleeps in each place */
                                     /* gleepct[nowhere] = 0 always */


private const Place shelf[appearancecount] = {
        incabinet /*if open*/, inniche, nowhere, nowhere, nowhere,
        incar /*if trunk closed*/, nowhere, nowhere, incentrifuge,
        nowhere, onarcshelf, onarcshelf
        // the rest gets filled in with nowheres
};                      /* places within other places, by appearance */

PUBLIC const Place container[objectcount] = {
        nowhere, nowhere, inbag, nowhere, nowhere, nowhere, inflask,
        intinybottle, ingalloncan  /* the rest gets filled in with nowheres */
};                      /* places that are inside objects */


#define n nomeaning
PUBLIC const Meaning roomits[appearancecount] = {
        n, n, oskull, n, n, fcar, n, opills, /*fcentrifuge*/ n,    // cabinet..nasa
        n, n, n, n, n, n, n,                                       // balcony..cleanbathroom
        n, n, n, n, owater, owater,                                // nasawreck..cistern
        n, n, n, n, n, n, n, n,                                    // bathroom..cube
        frats, n, fbooks, flaundry, fnewspapers, fbones, n, n, n,  // rat..sea
        fgags, n, n, n, n, n, n,                                   // joke..airlock
        flizards, fdevice, fbadge, n, n, /*fmachine*/ n, n,        // lizardhole..drydock
        n, fbirds, ftar, /*fcrystal*/ n, n, n, fagents,            // sooty..stuffed
        fbox, n, n, n, n, n, n, fstove,                            // hackerroom..meat
        n, n, n, n, n, n, n, n, n                                  // green..conpipe
};                      /* default value of "it" for take commands when no objects present */
#undef n



#ifndef OLDRANDOM

import void SetLoadRange(int min, int max), SetRemove(bool(*filter)(int, uint32), uint32 filterarg);
import int SetRemoveValue(int v), SetTakeOne(int avoid, int avoidmod);

PUBLIC bool FilterPlaceByAppearanceMask(int p, uint32 mask)  // a SetRemove filter
{
    int a = (int) map[(Place) p].features;
    return a < 32 && bit(a) & mask;
}

#  define RemovePlaceAppearances(mask)  SetRemove(FilterPlaceByAppearanceMask, mask)
#  define TakeOneAppearance()           (Appearance) SetTakeOne(-1, -1)
#  define TakeOnePlace()                (Place) SetTakeOne(-1, -1)
#  define TakeOneDistantPlace(avoid)    (Place) SetTakeOne((int) avoid, nrooms)

#endif



#define GleepDensity 5
#define RoomDensity 10

#define randObMask   (bit(orope) | bit(obag) | bit(okeys) | bit(ofeather) | bit(omatch) \
                      | bit(ogalloncan) | bit(onitro) | bit(osandwich) | bit(ohammer))

#define abnormalMask (bit(meadow) | bit(balcony) | bit(kerosene) | bit(streetlight) | bit(cube))
#define noLugiMask   (bit(ghost) | bit(nasa) | abnormalMask)
#define floorMask    (bit(cistern) | bit(tank) | bit(bubble) | abnormalMask)
#define noMonkeyMask (bit(ghost) | bit(nasa) | bit(garden) | floorMask)


PUBLIC void MakeMap(void)          /* call when starting game */
{
    import Meaning RandDirection(Meaning d), Opposite(Meaning d);
    import Place Distant(Place avoid);
    register Place rix;
    Place nextroom, otherend;
    register Meaning m;
    register Appearance a;
    Appearance wetroom = RRand(2) ? tank : cistern;
    int i;
#ifdef OLDRANDOM
    bool roomsused[appearancecount];

    // map is already initialized to falses and nowheres
    for (a = firstappearance; a < firstoptional; a++)
        roomsused[a] = true;
    for (a = firstoptional; a <= lastappearance; a++)
        roomsused[a] = false;
    roomsused[wetroom] = true;
#else
    SetLoadRange(firstoptional, lastappearance);
    SetRemoveValue(wetroom);
#endif
    // prevent outward-facing directions from getting randomized -- clear these later
    map[pcar].paths[drec(veast)] = pockets;
    map[pbalcony].paths[drec(vsouth)] = pockets;
    for (rix = r1; rix <= pbalcony; rix++) {
        if (rix == pbalcony) nextroom = r1;
        else nextroom = rix + 1;
#ifdef OLDRANDOM
        do
            a = (Appearance) RRand(appearancecount);
        while (roomsused[a]);
        roomsused[a] = true;
#else
        a = TakeOneAppearance();
#endif
        map[rix].features = a;
        gleepct[rix] = !RRand(GleepDensity) && a != ghost && a != bunker;
        // first set of connections: link all rooms into a two-way loop with horizontal paths
        do
            m = RandDirection(vwest);        // XXX AAARRRGH INFINITE LOOP! (once)
        while (map[rix].paths[drec(m)] || map[nextroom].paths[drec(Opposite(m))]);
        map[rix].paths[drec(m)] = nextroom;
        map[nextroom].paths[drec(Opposite(m))] = rix;
    }
    map[pcabinet].features = cabinet;
    map[pniche].features = niche;
    map[pbunker].features = bunker;
    map[pbuzzcr].features = buzzcr;
    map[pgarden].features = garden;
    map[pcar].features = car;
    map[ppillpile].features = pillpile;
    map[pcistank].features = wetroom;
    map[pnasa].features = nasa;
    map[pbalcony].features = balcony;
    for (rix = r1; rix <= pbalcony; rix++) {
        // second set of connections: each room has a chance at an additional two-way connection that may be vertical
        for (i = 0; i < RoomDensity; i++)
            if (!RRand(4)) {    // effectively divides roomdensity by four
                m = RandDirection(vclimb);
                otherend = Distant(rix);
                if (!map[rix].paths[drec(m)] && !map[otherend].paths[drec(Opposite(m))]) {
                    map[rix].paths[drec(m)] = otherend;
                    map[otherend].paths[drec(Opposite(m))] = rix;
                }
            }
        // third set of connections: each room has a good chance at getting an additional one-way path out
        m = RandDirection(vclimb);
        if (!map[rix].paths[drec(m)])
            map[rix].paths[drec(m)] = Distant(rix);
    }

    // certain connections get removed for the sake of game logic, e.g. no holes in floor if standing water
    map[pcar].paths[drec(veast)] = nowhere;                 // car showroom window faces eastward
    map[pbalcony].paths[drec(vsouth)] = nowhere;            // balcony faces southward (may break loop?)
    for (rix = r1; rix <= pbalcony; rix++) {
        a = map[rix].features;
        if (a < 32 && bit(a) & floorMask)
            map[rix].paths[drec(vjump)] = nowhere;          // many rooms cannot have openings in floor
        if (a == meadow || a == streetlight || a == cube)
            map[rix].paths[drec(vclimb)] = nowhere;         // never climbable
        if (a == drydock)
            map[rix].paths[drec(vclimb)] = Distant(rix);    // always climbable
        if (a == ghost)
            for (i = 0; i < ndirec; i++)
                map[rix].paths[i] = nowhere;                // you get sealed in
    }

#ifdef OLDRANDOM
    // this crappy looping on Distant is inherited from the paleolithic Pascal implementation
    do {
        yourroom = Distant(pbalcony);
        env = map[yourroom].features;
    } while (env == ghost);
    lastenv = balcony;
    do {
        guardroom = Distant(yourroom);
        a = map[guardroom].features;
    } while (a < 32 && bit(a) & noLugiMask);
    if (RRand(2))
        do {
            runtroom = Distant(nowhere);
            a = map[runtroom].features;
        } while (runtroom == guardroom || (a < 32 && bit(a) & noLugiMask));
    else runtroom = nowhere;
    if (RRand(2)) {
        // *** ARGH this actually failed and put gorilla in cistern!
        do {
            gorroom = Distant(nowhere);
            a = map[gorroom].features;
        } while ((a < 32 && bit(a) & noMonkeyMask) || gorroom == guardroom);
        //map[gorroom].paths[drec(vjump)] = Distant(gorroom);
        for (i = drec(vnorth); i <= drec(vjump); i++)
            if (!map[gorroom].paths[i])
                map[gorroom].paths[i] = Distant(gorroom);
    } else gorroom = nowhere;
    if (RRand(2))
        do {
            thingroom = Distant(nowhere);
            a = map[thingroom].features;
        } while (a < 32 && bit(a) & abnormalMask);
    else thingroom = nowhere;

    for (m = firstobject; m <= lastobject; m++)
        if (bit(m) & randObMask) {
            do {
                rix = Distant(m == ohammer ? guardroom :
                              m == ofeather ? thingroom :
                              m == osandwich ? gorroom :
                              nowhere);
                a = map[rix].features;
            } while (a == cistern || a == tank || a == ghost || a == meadow || (m == omatch && a == garden));
            location[m] = rix;
        } else location[m] = nowhere;
#else
    SetLoadRange(1, nrooms);
    RemovePlaceAppearances(bit(ghost));
    yourroom = TakeOneDistantPlace(pbalcony);
    env = map[yourroom].features;
    lastenv = balcony;
    RemovePlaceAppearances(abnormalMask);
    thingroom = RRand(2) ? nowhere : TakeOnePlace();
    location[ofeather] = TakeOneDistantPlace(thingroom);
    RemovePlaceAppearances(noLugiMask);
    runtroom = RRand(2) ? nowhere : TakeOnePlace();
    guardroom = TakeOneDistantPlace(yourroom);
    location[ohammer] = TakeOneDistantPlace(guardroom);
    RemovePlaceAppearances(noMonkeyMask);
    if ((gorroom = RRand(2) ? nowhere : TakeOnePlace()))
        for (i = drec(vnorth); i <= drec(vjump); i++)
            if (!map[gorroom].paths[i])
                map[gorroom].paths[i] = Distant(gorroom);
    location[osandwich] = TakeOneDistantPlace(gorroom);
    for (m = firstobject; m <= lastobject; m++)
        if (bit(m) & randObMask && !location[m]) {
            do {
                rix = Distant(nowhere);
                a = map[rix].features;
            } while (a == cistern || a == tank || a == ghost || a == meadow || (m == omatch && a == garden));
            location[m] = rix;
        }
#endif

    gleepct[incentrifuge] = 100 + RRand(40);
    gleepct[intrunk] = 60 + RRand(30);    
    gleepct[inbag] = RRand(4) + 3;

    location[oflask] = pniche;         /* NOT inniche! */
    location[ostatuette] = onarcshelf;
    location[oacetone] = ingalloncan;
    location[olysol] = location[oflysol] = location[oblackflag] = incabinet;
    location[opheromone] = intinybottle;
}


#if defined(_DEBUG) && defined(MAP_TEST_HARNESS)

PUBLIC bool TestHarness_MakeMap()
{
    import str appearanceword[];
    import int rawprintf(const char *format, ...);
    unsigned seed;
    Place p;
    Meaning n;
    ulong noway = 0, oneway = 0, twoway = 0, loopbad = 0, floorbad = 0, collGR = 0, collLR = 0, collGLbad = 0;
    uint32 nogor = 0, gorfloor = 0, gorbad = 0, noscaly = 0, scaly = 0, unghostly = 0;
    uint32 norunt = 0, runt = 0, runtbad = 0, guardbad = 0, noghost = 0, ghostly = 0, lastly = 0;
    Appearance a;
    for (seed = 65536 * 4096; seed < 65536 * 8192 && /*for int16:*/ (seed != 0 || twoway == 0); seed++) {
        // all values lower than 65536 * 4096 have been checked for both algorithms, and up to 65536 * 8192 for the new algorithm
        srand(seed);
        MakeMap();
        // validate!  start with paths
        for (p = r1; p <= pbalcony; p++) {
            bool good1 = false, good2 = false;
            Place g1 = p == pbalcony ? r1 : (Place) (p + 1), g2 = p == r1 ? pbalcony : (Place)(p - 1);
            for (n = vnorth; n <= vclimb; n++) {
                if (map[p].paths[drec(n)] == g1)
                    good1 = true;
                if (map[p].paths[drec(n)] == g2)
                    good2 = true;
            }
            a = map[p].features;
            if (a == ghost) {
                ghostly++;
                for (n = orope; n < gleeps; n++)
                    if (location[n] == p) {
                        unghostly++;
                        printf("** seed %u INVALID: %s (at %d) has object present\n", seed, appearanceword[a], p);
                    }
            }
            if ((!good1 || !good2) && a != ghost) {
                loopbad++;
                // printf("** seed %u INVALID: loop path missing at room %d\n", seed, p);
            }
            for (n = vnorth; n <= vclimb; n++) {
                Place op = map[p].paths[drec(n)];
                if (!op)
                    noway++;
                else if (map[op].paths[drec(Opposite(n))] == p)
                    twoway++;       // don't forget to divide by two later
                else
                    oneway++;
            }
            if (a < 32 && bit(a) & floorMask && map[p].paths[drec(vjump)]) {
                floorbad++;
                printf("** seed %u INVALID: %s (at %d) has leak in floor\n", seed, appearanceword[a], p);
            }
        }
        if (lastly < ghostly)
            lastly = ghostly;
        else
            noghost++;
        // now validate entities -- gorilla first, as that's given us trouble
        if (!gorroom)
            nogor++;
        else {
            bool good1 = true;
            a = map[gorroom].features;
            if (a < 32 && bit(a) & noMonkeyMask) {
                gorbad++;
                good1 = false;
                printf("** seed %u INVALID: gorilla appeared in %s (at %d)\n", seed, appearanceword[a], gorroom);
            }
            for (n = vnorth; n <= vjump; n++)
                if (!map[gorroom].paths[drec(n)]) {
                    gorbad++;
                    good1 = false;
                    printf("** seed %u INVALID: no path from gorilla room (%d) in direction %d\n", seed, gorroom, drec(n));
                    goto skippy1;
                }
          skippy1:
            if (gorroom == guardroom) {
                collGLbad++;
                good1 = false;
                printf("** seed %u INVALID: gorilla appeared in %s (at %d) alongside guard\n", seed, appearanceword[a], gorroom);
            }
            if (good1)
                gorfloor++;
        }
        // guard and runt
        if (!runtroom)
            norunt++;
        else {
            runt++;
            if (runtroom == guardroom)
                collLR++;
            if (runtroom == gorroom)
                collGR++;
            a = map[runtroom].features;
            if (a < 32 && bit(a) & noLugiMask) {
                runtbad++;
                printf("** seed %u INVALID: runt appeared in %s (at %d)\n", seed, appearanceword[a], runtroom);
            }
        }
        a = map[guardroom].features;
        if (a < 32 && bit(a) & noLugiMask) {
            guardbad++;
            printf("** seed %u INVALID: guard appeared in %s (at %d)\n", seed, appearanceword[a], guardroom);
        }
        // misc
        if (thingroom)
            scaly++;
        else
            noscaly++;
        if (seed % 5000 == 0)
            rawprintf("%9u\b\b\b\b\b\b\b\b\b", seed);
        memset(map, 0, sizeof(map));
    }

    // did we learn anything?
    printf("Validated %s maps.  %s failed to have a loop path.\n", format32u(scaly + noscaly), format32u(loopbad));
    printf("There were %s one-way paths, %s two-way, and %s blocked.\n", format32u(oneway), format32u(twoway / 2), format32u(noway));
    printf("%s maps had invalid holes in the floor.\n", format32u(floorbad));
    printf("%s maps had a ghost room, %s did not; %s ghosts had an object.\n", format32u(ghostly), format32u(noghost), format32u(unghostly));
    printf("%s maps had a runt, %s did not.\n", format32u(runt), format32u(norunt));
    printf("%s maps had a scaly thing, %s did not.\n", format32u(scaly), format32u(noscaly));
    printf("%s maps had a gorilla, %s did not.\n", format32u(gorfloor + gorbad), format32u(nogor));
    printf("%s of those gorilla placements had invalid paths.\n", format32u(gorbad));
    printf("%s gorillas collided with guards, %s with runts.\n", format32u(collGLbad), format32u(collGR));
    printf("%s guards collided with runts.\n", format32u(collLR));
    printf("%s guards and %s runts were in invalid rooms.\n", format32u(runtbad), format32u(guardbad));
    return !!(loopbad + floorbad + gorbad + collGLbad + runtbad + guardbad);
}
// This has not identified any problems with the OLDRANDOM algorithm.  Could not replicate gorilla in cistern incident.
// It did find bugs with the new one, but it looks like that is now working properly.
#endif



PUBLIC Place Shelf(Appearance a)
{
    Place s = shelf[a];
    if (a == car && trunkopen)
        return intrunk;
    /* you can't pick stuff up from incar while outside of the car, though
       you can put stuff into the car from outside
       and you can see it there... you can pick up stuff from intrunk, but
       not if the trunk is closed, in which case you can't see it */
    if (s == incabinet && !cabinetopen)
        return nowhere;      /* can't see, take, or put when closed */
    return s;
}



PUBLIC Meaning Jug(Meaning ob)
{
    if (ob > lastobject) return ob;
    else if (location[ob] == ingalloncan) return ogalloncan;
    else if (location[ob] == inflask) return oflask;
    else if (location[ob] == intinybottle) return ophial;
    else return ob;
}



/* True if object o is in the player's sight.  Less liberal than Here() below;
   this one is false if the object "blends into the background".  Unlike before,
   DOES NOT return true for objects the player is carrying.  Also does not
   return true for objects inside containers -- calling Jug is up to you.
   Does return true for living things in room or on body, and for kicked
   objects in fogarc or kerosene. */
PUBLIC bool HereS(Meaning o)
{
    import bool attached, beastfollowing, fungus;
    import ushort flies;
    Place s = Shelf(env);
    Place s2 = s == intrunk ? incar : nowhere;

    if (env == fogged || env == fogsplash) return false;
    if (isob(o))
        return location[o] == yourroom || (s && location[o] == s) || (s2 && location[o] == s2);
    else if (o == gleeps)
        return gleepct[yourroom] > 0 || gleepct[s] > 0 || gleepct[s2] > 0;
    else switch (o) {
        case fstove:      return env == meat;
        case fnewspapers: return env == office;
        case fbooks:      return env == library;
        case fwhips:
        case fchains:
        case fbadge:      return env == torture;
        case fpaintings:  return env == carvedfloor;
        case fdevice:     return env == device;
        case fsign:       return env == sea || env == bubble || env == airlock || env == machine;
        case fcircuitry:  return env == circuits;        
        case fcar:        return env == car;
        case fmachine:    return env == machine;
        case fdirt:       return env == pillpile;
        case fbones:      return env == caf || env == bunker || env == ghostbunker || env == torture;
        case flaundry:    return env == laundry;
        case fgags:       return env == joke;
        case ftar:        return env == tar;
        case fagents:     return env == stuffed;
        case fbattleship: return env == drydock;
        case fcrystal:    return env == archive;
        case ftrash:      return env == hackerroom || env == driverseat;
        case fcentrifuge: return env == nasa || yourroom == parcturus;  // "cage" applies to both
        case fliquid:     return env == columns;      // in other rooms it may be biguated to owater
        case fwindow:     return env == car;
        case fvermin:     return env == pillpile || env == rat || env == lizardhole;
        case flizards:    return env == lizardhole;
        case fplants:     return env == garden;
        case fghost:      return env == ghost || env == ghostbunker;
        case frats:       return env == rat;
        case fbirds:      return env == freebirds;
        case fwumpus:     return false;
        case fbox:
        case fgnome:      return env == hackerroom;
        case lscalything: return attached;
        case lgorilla:    return yourroom == gorroom;
        case lguard:      return yourroom == guardroom;
        case fcudgel:
        case lrunt:       return yourroom == runtroom;
        case lflies:      return flies > 0;
        case lbeast:      return beastfollowing;
        case lfungus:     return fungus;
        default:          return false;
    }
    return false;       /* some dumb compilers think it's not returning anything */
}



/* Returns true if the object o is in the player's sight. */
PUBLIC bool Here(Meaning o)
{
    if (HereS(o))
        return true;
    if (Jug(o) != o && HereS(Jug(o)))
        return true;
    if (o == owater && (inwater || env == seep))
        return true;
    if (o == opills && env == pillpile)
        return true;
    if (o == oskull && (env == bunker || env == ghostbunker))
        return true;
    return false;
}


/* Returns true if more than one object that's present fits the word. */
PUBLIC bool HereL(Meaning o)
{
    if (Here(o))
        return true;
    if (o == fliquid && (Have(owater) || Here(owater) || Have(ochemicals) || Here(ochemicals) ||
                         Have(oacetone) || Here(oacetone) || Have(ourine) || Here(ourine) ||
                         Have(opheromone) || Here(opheromone)))
        return true;
    return false;
}


#ifdef _DEBUG
// this section implements a cheating back door for test porpoises

PUBLIC void NominatePlace(Place p)
{
    import str appearanceword[];
    if (p == nowhere)
        put("nowhere");
    else if (p < nowhere || p > pockets)
        printf("(%d)", p);
    else if (p > nrooms)
        put(oddplaces[p - nrooms - 1]);
    else
        printf("in %s (room %d)", appearanceword[map[p].features], p);
}

private Place Find(Appearance a)
{
    int i;
    for (i = 1; i <= nrooms; i++)
        if (map[i].features == a)
            return (Place) i;
    return nowhere;
}


private void SummonObject(Meaning ob)
{
    import int Weight(Place where);
    import void Inventory(Place p);
    Meaning o;
    if (ob == ochemicals || ob == owater || ob == ourine) {
        for (o = firstobject; o <= lastobject; o++)
            if (location[o] == inflask)
                location[o] = nowhere;
        location[ob] = inflask;
    }
    if (ob == oacetone) location[ob] = ingalloncan;
    if (ob == opheromone) location[ob] = intinybottle;
    if (ob == osandwich) badsandwich = true;
    if (ob == omatch) wetmatch = false;
    ob = Jug(ob);
    if (GetWord() == mup)
        location[ob] = pockets;
    else if (!Here(ob) && !Have(ob))
        location[ob] = yourroom;
    it = ob;
    if (Have(ob))
        Inventory(pockets);
}

private void SetUpScenario(void)
{
    import bool beastfollowing, galloncanopen;
    Meaning s = GetWord();
    if (s == fcar) {
        yourroom = pcar;
        env = car;
        trunkopen = true;
        location[olysol] = pcar;
        location[oblackflag] = inbag;
        location[obag] = pockets;
        location[oflysol] = intrunk;
        location[oflask] = intrunk;
        location[owater] = inflask;
        location[ourine] = nowhere;
        location[ochemicals] = nowhere;
        location[ostatuette] = incar;
        gleepct[pockets] = 0;
        gleepct[inbag] = 1;
        gleepct[pcar] = 2;
        gleepct[intrunk] = 4;
        gleepct[incar] = 8;
    } else if (s == ostatuette || s == lbeast) {
        yourroom = parcturus;
        env = fogarc;
        beastfollowing = false;
        location[ostatuette] = parcturus;
        location[oflysol] = parcturus;
        location[oflask] = parcturus;
        location[ourine] = inflask;
        location[owater] = nowhere;
        location[ochemicals] = nowhere;
        location[opills] = onarcshelf;
        location[ofeather] = onarcshelf;
        location[ogalloncan] = inbag;
        location[oacetone] = ingalloncan;
        location[obag] = pockets;
        gleepct[inbag] = 0;
        gleepct[parcturus] = GetWord() ? 0 : 3;     // reject gleeps with e.g. "ct fill beast in"
        gleepct[onarcshelf] = 0;
        gleepct[pockets] = 0;
    } else if (s == owater) {
        yourroom = pcistank;
        env = map[pcistank].features;
        location[oflask] = pockets;
        location[ourine] = inflask;
        location[owater] = nowhere;
        location[ochemicals] = nowhere;
        location[ogalloncan] = pockets;
        location[oacetone] = nowhere;
        location[obag] = pockets;
        galloncanopen = true;
    } else if (s == vleave) {
        yourroom = pbalcony;
        env = balcony;
        location[onitro] = pbalcony;
        location[ogalloncan] = pbalcony;
        location[ophial] = pbalcony;
        location[oacetone] = ingalloncan;
        location[okeys] = pockets;
        location[osandwich] = inbag;
        location[ochemicals] = inflask;
        location[oflask] = inbag;
        location[obag] = pockets;        
    } else if (s == gleeps) {
        location[obag] = pockets;
        gleepct[inbag] = 3000 + RRand(17000);
    }
}

PUBLIC void PunkinEater(void)
{
    import void AskCommand(str prompt), Situation(void), colorDebug(void), colorCommand(void);
    import void BumpTimer(Seconds timeToLose), Hurry(void);
    import ushort flies, beenin;
    import bool attached, beastfollowing, fungus, strong, galloncanopen;
    import str appearanceword[];
    Place destination = nowhere;
    int i;
    Meaning o;
    bool recognized = false;

    colorDebug();
    o = GetWord();
    if (o) verb = o, verbword = lastword;
    else if (bogusword && *bogusword) verb = mignore, verbword = bogusword;
    else AskCommand("\n\n[][] Name the place, object, or being to summon:  ");
    colorDebug();
    cheated = true;
    did = false;
    if (verb <= lastobject) {    // this can recreate dispersed liquids:
        SummonObject(verb);
    } else switch (verb) {
        case mall:
        case meverything:
            for (o = firstobject; o <= lastobject; o++)
                SummonObject(o);
            break;
        case gleeps:
            gleepct[yourroom] += 20;
            break;
        // critters
        case lscalything:
            attached = true;
            thingroom = nowhere;
            puts("A scaly thing just grabbed your leg.");
            break;
        case lgorilla:
            gorroom = yourroom;
            goralive = true;
            break;
        case lrunt:
            runtroom = yourroom;
            runtalive = true;
            break;
        case lguard:
            guardroom = yourroom;
            guardalive = true;
            guardsees = false;
            break;
        case lflies:
            flies += 12;
            break;
        case lbeast:
            beastfollowing = true;
            puts("A big animal enters the room.");
            break;
        case lfungus:
            fungus = true;
            puts("Your arm starts to itch.");
            break;
        // special purpose code verbs: inventory to list locations, look (or exit) to list rooms,
        // help to gain strength, start to lose time, fill x to set up test scenarios, or liquid for reserve blend
        case vinventory:
            i = 0;
            for (o = firstobject; o <= lastobject; o++)
                if (location[o] == nowhere) i++;
                else {
                    printf("%s is ", bestword[o]);
                    NominatePlace(location[o]);
                    putchar('\n');
                }
            printf("(the other %d are nowhere)\n", i);
            break;
        case vhelp:
            strong = true;
            weightlimit = 40;
            puts("Strength surges through your body.");
            break;
        case vfill:
            SetUpScenario();
            break;
        case vleave:
        case vlook:
            for (i = 1; i < nrooms; i++)
                if (!map[i].explored)
                    printfDebug("%d=%s ", i, appearanceword[map[i].features]);
            printfDebug(" -- %d explored.\n", beenin);
            break;
        case vstart:
            if (!lastwarning)
                BumpTimer(450), Hurry();
            break;
        case vread:
            colorAlarm();
            put("Here is a line of multicolored ");
            colorNormal();
            put("content which changes its hue ");
            colorEvent();
            put("several times across the span of ");
            colorCommand();
            put("the content it displays.  ");
            colorDebug();
            puts("I hope this helps.");
            break;
        case fbooks:
            puts("Cheating?  I didn't see any cheating...");
            cheated = false;
            break;
        case fliquid:    // our special reserve blend
            if (location[ogalloncan] != pockets) location[ogalloncan] = yourroom;
            location[opills] = ingalloncan;
            location[owater] = ingalloncan;
            location[ourine] = ingalloncan;
            location[oacetone] = nowhere;
            galloncanopen = true;
            gleepct[ingalloncan] += 7;
            it = ogalloncan;
            break;
        case mcan:
            if (location[ogalloncan] != pockets) location[ogalloncan] = yourroom;
            location[oacetone] = nowhere;
            galloncanopen = true;
            gleepct[ingalloncan] += 7;
            break;
        // places
        case mshelf:
            destination = !strcmp(verbword, "cabinet") ? pcabinet : pniche;
            break;
        case vbutton:
            destination = pbuzzcr;
            break;
        case fdirt:
            destination = ppillpile;
            break;
        default:
            for (i = 0; i < appearancecount && !destination; i++)
                if (roomits[i] == verb || !strcmp(appearanceword[i], verbword))
                    recognized = true, destination = Find((Appearance) i);
            if (!destination)
                if (recognized) puts("Sorry, that room isn't present in the current map.");
                else puts("Not recognized.");
    }
    if (destination)
        yourroom = destination, env = map[yourroom].features;
    colorNormal();
    Situation();
}
#endif
