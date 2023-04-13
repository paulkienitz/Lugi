/* PUT.C
   This handles the crucial "put" command in the game of Lugi, which is the
   single most complex command. */


#include "lugi.h"


import bool chemgiven, trunkopen, goralive, badsandwich, badhygiene,
                attached, wetmatch, cabinetopen, galloncanopen, glued, strong;

import Sickness sickness;

import const Place container[objectcount];

import Place gorroom, Shelf(Appearance a);

import Meaning Jug(Meaning ob);

import int Weight(Place where);

import bool HereL(Meaning o), ImplicitTake(Meaning ob, str name, Place source, bool specific);

import void Nominate(Meaning ob), Enter(void), Insolvent(Place goal), Fumes(void), PlugItIn(void);



// Which prepositions make sense with which verbs, ideally?
//
//             in/on/into       with/by          from/out(of)
//             --------------   --------------   ----------------
// take:       Maybe (source)   No               Yes (source)
// put:        Yes (dest)       Yes (indirect)   Maybe for gleeps
// drop/throw: Yes (dest)       No               Maybe for gleeps
// pour:       Yes (dest)       Not really       Maybe (source)
// fill:       No               Yes (subject)    Yes (source)
// piss:       Yes (dest)       No               No
// attack:     No               Yes (weapon)     No
//
// At present we support only two options: in/from/out (all meaning the same
// thing) for take, or in and with for put or pour.
// This is not compatible with the special way fill needs to use with.
PUBLIC Place ParsePlace(bool taking)
{
    import void DiscardLastWordGotten(void);
    int r = 0;
    bool bagflag = false;
    Place goal = nowhere, s = Shelf(env);
    Meaning m, p;
    do {
        m = GetWord();
        if (m == obag) bagflag = true;
        r++;
    } while (m && m != min && (!taking || m != mout) && (!taking || m != mfrom) && (taking || m != mwith));
    if (m) {           // initial "out" pre-parsed by TakeThem does not claim following word
        p = m;
        m = GetWord();
        if (m == mout || m == mfrom) {  // for "take ob out from place"... XXX also handle "in with"?
            m = GetWord();
            r++;
        }
        if (m != vplug) DiscardLastWordGotten();
        if (m >= firstalive && m <= lastalive)
            goal = (Have(m) ? yourroom : nowhere);
        else if (p == mwith) {
            if (isob(m))
                goal = Have(m) || Here(Jug(m)) ? location[Jug(m)] : nowhere;   // don't mess with liquids
            else if (m != gleeps)
                goal = Here(m) ? yourroom : nowhere;
            else if (gleepct[yourroom] > 0)
                goal = yourroom;
            else if (gleepct[s] > 0)
                goal = s;
            else if (gleepct[incar] > 0 && env == car)
                goal = incar;
            else if (gleepct[inbag] > 0 && Have(obag))
                goal = inbag;
            else if (gleepct[ingalloncan] > 0 && Have(ogalloncan))
                goal = ingalloncan;
        } else switch (Jug(m)) {
            case obag:
                goal = Have(obag) || (!taking && Here(obag)) ? inbag : nowhere;
                break;
            case fcar:
                goal = incar;
                break;
            case mshelf:
                if (!taking && env == cabinet)
                    goal = incabinet;           // even if it's shut
                else if (!taking && env == car)
                    goal = intrunk;             // even if it's shut
                else if (yourroom == parcturus || env == niche || env == cabinet || env == car)
                    goal = s;
                else goal = nowhere;
                break;
            case mcan:
            case ogalloncan:
                goal = Have(ogalloncan) || (!taking && Here(ogalloncan)) ? ingalloncan : nowhere;
                break;
            case oflask:
                goal = Have(oflask) || (!taking && Here(oflask)) ? inflask : nowhere;
                break;
            case fcentrifuge:
                goal = (env == nasa ? incentrifuge : nowhere);
                break;
            case vplug:
                break;
            case nomeaning:
                if (taking)
                    goal = pockets;      // used in take functions to mean nonspecific "out"
                else if (!bagflag && Have(obag)) {
                    goal = inbag;
                    puts("(putting into the bag)");
                } else if (s) {
                    goal = s;
                    switch (s) {
                        case inniche:      puts("(putting into the niche)"); break;
                        case incabinet:    puts("(putting into the cabinet)"); break;
                        case incar:        puts("(putting into the car)"); break;
                        case intrunk:      puts("(putting into the car's trunk)"); break;
                        case incentrifuge: puts("(putting into the centrifuge)"); break;
                        case onarcshelf:   puts("(putting onto the little shelf)");
                    }
                } else if (!taking && Here(obag)) {
                    goal = inbag;
                    puts("(putting into the bag)");
                } else if ((Have(oflask) || (!taking && Here(oflask))) && location[ochemicals] != inflask) {
                    goal = inflask;
                    puts("(putting into the flask)");
                } else if ((Have(ogalloncan) || (!taking && Here(ogalloncan))) && location[oacetone] != ingalloncan) {
                    goal = ingalloncan;
                    puts("(putting into the one gallon can)");
                }
        }
    } else
        goal = yourroom;
    if (goal == yourroom && env == driverseat) goal = incar;
    while (r-- > 0) UnGetWord();
    return goal;
}



private void PutGleeps(Place goal, bool quiet)
{
    Place s = Shelf(env);
    uint32 *gy = gleepct + yourroom, *gp = gleepct + pockets, *gs = gleepct + s,
           *gb = gleepct + inbag, *gc = gleepct + ingalloncan;
    uint32 gt, ogt;
    bool donom = quiet;

    if (s == incar && *gs && goal != incar && verb == vput) {
        if (!quiet) puts("You have to get into the car to reach the gleeps there.");
        gs = gleepct + nowhere;
        quiet = true;
    }
    if (((goal == inbag && !*gc) || (goal == ingalloncan && !*gb)) && !*gy && !*gp && !*gs) {
        if (!quiet) puts("They're all in there already.");
    } else if (goal == yourroom && (!*gs || verb != vput) && !Have(gleeps)) {
        if (!quiet) puts("You have none to put down.");
    } else if (goal == inflask) {
        if (donom) Nominate(gleeps);
        puts("They don't fit through the neck of the flask.");
    } else if (goal == ingalloncan && !galloncanopen) {
        if (donom) Nominate(gleeps);
        puts("The can is capped shut.");
    } else if (goal == incentrifuge)
        Enter();                        /* fatal */
    else {
        did = true;
        if (goal == inniche || goal == onarcshelf || (goal == yourroom && (env == ghost || env == ghostbunker))) {
            if (*gy + *gp) *gy = *gp = 0;
            else if (Have(obag) & *gb) *gb = 0;   // only empty bag if you haven't already lost others
            if (donom) Nominate(gleeps);
            puts("POP!  They disappear the moment they contact the surface.");
        } else if (goal == inbag || goal == ingalloncan) {
            ogt = gleepct[goal];
            gleepct[goal] += *gy + *gp + *gs;
            *gy = *gp = *gs = 0;
            if (goal == ingalloncan && Have(obag)) {
                *gc += *gb;
                *gb = 0;
            }
            if (donom) Nominate(gleeps);
            if (goal == inbag && Have(ogalloncan) && *gc && (verb == vput || verb == vfill))
                puts("You can't get any back out of the one gallon can.");
            if (gleepct[goal] > ogt) {
                if (goal == inbag) put("The bag ");
                else put("The one gallon can ");
                if (gleepct[goal] > 1)
                    printf("now holds %s.\n", format32u(gleepct[goal]));
                else puts("now holds one gleep.");
            }  // ...is there any case where that fails and we haven't said an excuse yet?
        } else {
            ogt = gleepct[goal];
            gt = *gp;
            *gp = 0;
            if (Have(obag) && *gb && (verb == vput || !gt)) {
                gt += *gb;
                *gb = 0;
                if (donom) Nominate(gleeps);
                donom = false;
                put("(take the gleeps out of the bag)  ");
            }
            if (verb == vput)
                gt += *gy + *gs, *gy = *gs = 0;
            else if (goal == yourroom)
                gt += *gy, *gy = 0;
            // are there any other else cases to cover?
            gleepct[goal] = gt;
            if (gleepct[goal] > ogt) {
                if (donom) Nominate(gleeps);
                puts("OK");
            } else if (!quiet) {
                if (donom) Nominate(gleeps);
                if (Have(ogalloncan) && gleepct[ingalloncan] > 0)
                    puts("You can't get any gleeps out of the one gallon can.");
                else puts("You have no gleeps to put there.");
            }
        }
    }
    gleepct[nowhere] = 0;       /* just in case of a slipup */
}



private void ReallyPut(Meaning ob, Place goal)
{
    Appearance renv;
    did = true;
    if (!goal) return;
    if (yourroom == parcturus) renv = arcturus;
    else if (yourroom == incar) renv = driverseat;
    else renv = map[yourroom].features;
    if (ob == osandwich && location[osandwich] == yourroom && inwater) {
        puts("It's so soggy that it disintegrates when you try to pick it up.");
        location[osandwich] = nowhere;
        return;
    } else if (ob == ochemicals) {
        puts("You tear open the seal on the flask...");
        Fumes();
        return;
    } else location[ob] = goal;
    if (goal == incentrifuge)
        Enter();        /* fatal */
    else if (ob == oflask && goal == inniche) {
        Meaning oo;
        for (oo = firstobject; oo <= lastobject; oo++)
            if (location[oo] == inflask) location[oo] = nowhere;
        put("As you let go of it, a glass door closes over the front of the \
niche.  A mechanical arm comes out of an opening in the back of the niche, \
picks up the flask, and draws it back into the opening.  A moment later, ");
        if (chemgiven) {        /* duplicate urine in galloncan?  nah */
            location[ourine] = inflask;
            puts("it emerges, with the flask now full of steaming yellow liquid.  It sets \
it down and withdraws back behind the wall, and the glass door opens.");
        } else {
            location[ochemicals] = inflask;
            chemgiven = true;
            puts("it emerges, with the flask now full of mysterious chemicals, \
and sealed at the top with something like thick saran wrap.  \
The arm sets the flask down and withdraws back into the \
wall, and the glass door opens.");
        }
    } else if (goal == inniche) {
        puts("As you let go of it, a glass door closes over the front of the \
niche.  An opening appears in the back and a mechanical arm emerges, which \
picks it up, turns it over a couple of times as if examining it, and then \
sets it down.  The arm retreats back into the wall and the glass door opens.");
    } else if ((ob == oskull && (renv == bunker || env == ghostbunker)) ||
               (ob == opills && renv == pillpile)) {
        location[ob] = nowhere;
        puts("OK");
    } else if (goal == yourroom && (env == ghost || env == ghostbunker)) {
        if (ob == opills || ob == okeys)
            puts("They disappear in a puff of smoke.");
        else puts("It disappears in a puff of smoke.");
        location[ob] = nowhere;
    } else if (goralive && goal == gorroom && ob == osandwich && env != fogged && env != fogsplash) {
        location[osandwich] = nowhere;
        if (badsandwich) {
            goralive = false;
            puts("The gorilla grabs it and wolfs it down.  Soon, the mighty beast \
begins to look sick.  After about thirty seconds, it collapses.");
        } else puts("The gorilla grabs it and wolfs it down.");
    } else if (liquid(ob) && goal == yourroom && inwater) {
        if (ob == oacetone) {
            puts("Being lighter than water, it mostly floats on top.");
            Insolvent(yourroom);
        } else {
            location[ob] = nowhere;
            puts("It mixes into the water you're standing in.");
            if (ob == ourine) badhygiene = true;
        }
    } else if (ob == oacetone && goal == inflask) {   // avoid dividing it between places
        puts("You awkwardly attempt to pour it into the flask...");
        putsAlarm("Your hand slips -- the acetone can falls to the floor and the contents run out.");
        Insolvent(yourroom);
        location[ogalloncan] = yourroom;
    } else if (ob == oacetone)
        Insolvent(goal);
    else if (ob == okeys && glued) {
        puts("OK -- you break the glue with your enhanced strength.");
        glued = false;
    } else
        puts("OK");
    if (ob == omatch && ((goal == yourroom && inwater) || location[owater] == goal || location[ourine] == goal))
        wetmatch = true;
}



PUBLIC void PutThisThere(Meaning ob, Place goal)
{
    if (ob == gleeps)
        PutGleeps(goal, false);
    else if (!isob(ob))
        ImplicitTake(ob, null, yourroom, true);     // furniture excuse
    else if (goal == intrunk && !trunkopen)
        puts("The trunk isn't open.");              // maybe say "You have to open the trunk first."
    else if (goal == incabinet && !cabinetopen)
        puts("The cabinet isn't open.");
    else if (location[ob] == goal)
        puts("It's there already.");
    else if ((env == seep && ob == owater && !(Have(owater) || HereS(owater))) || (Jug(ob) == ourine && Here(ourine)))
        puts("There's not enough liquid here to collect together.");
    else if (goal == inbag && (ob == ohammer || ob == obag))
        puts("It doesn't fit in the bag.");
    else if (goal == onarcshelf && !(Empty(onarcshelf) && (ob == ophial ||
                                        ob == opills || ob == ofeather ||
                                        ob == okeys || ob == ostatuette)))
        puts("It doesn't fit on the teeny little shelf.");
    else if ((goal == ingalloncan || location[ob] == ingalloncan) && !galloncanopen)
        puts("The can is capped shut.");
    else if (goal == inflask && location[ochemicals] == inflask)
        puts("The flask is sealed shut.");
    else if (liquid(ob) && goal == ingalloncan && location[oacetone] == ingalloncan)
        puts("The can is too full.");
    else if (liquid(ob) && goal == inflask && Weight(inflask))
        puts("The flask is already full.");
    else if (liquid(ob) && goal == inbag)
        puts("The bag is too porous to contain anything liquid.");
    else if (goal == inflask && !(ob == opills || ob == ofeather || ob == omatch
                                  || (liquid(ob) && !Weight(inflask))))
        puts("It doesn't fit in the flask.");
    else if (goal == ingalloncan && !(ob == opills || ob == ofeather || ob == gleeps || ob == omatch
                                      || ob == ophial /*triple nest possible*/ || liquid(ob)))
        puts("It doesn't fit through the opening of the can.");
    else if (location[ob] == incar && yourroom != incar)
        puts("You have to get into the car to retrieve it.");
    // we do not call ImplicitTake, so we have to do our own enforcing of its rules for unpickupable items
    else if (ob == okeys && glued && (!strong || sickness > mild))
        puts("The glue is too strong for you to break.");
    else if (liquid(ob) && location[ob] == yourroom)        // is this check needed?
        puts("You can't pick up a thin layer of liquid.");
    // we let Update handle weight limit
    else
        ReallyPut(ob, goal);
}



PUBLIC void PutThem(void)
{
    import int nominations;
    bool allflag = false, nuthin = true;
    Place goal = ParsePlace(false);
    Meaning ob = GetWord();
    if (ob == min || ob == mwith)
        ob = GetWord();
    did = false;
    if (!ob) {
        if (bogusword) printf("I don't see any %s here...\n", bogusword);
        else printf("I don't understand what you want me to %s.\n", verbword);
        return;
    }
    if (ob == mdown) ob = GetWord();
    for (; ob; ob = GetWord()) {
        if (ob == mit) ob = it;
        if (ob == mall)
            allflag = true;
        else if (ob == meverything) {
            allflag = nuthin = true;
            break;
        } else if (ob == mcans) {
            for (ob = olysol; ob <= oblackflag; ob++)
                if ((Have(ob) || Here(ob)) && location[ob] != goal) {
                    Nominate(ob);
                    if (!goal)
                        printf("I don't understand where you want to %s it.\n", verbword);
                    else
                        PutThisThere(ob, goal);
                    nuthin = false;
                }
        } else if (isnoun(ob)) {
            if (liquid(ob) && !(goal == ingalloncan || goal == inflask || goal == intinybottle))
                ob = Jug(ob);
            if (Have(ob) || Here(ob)) {
                it = ob;
                Nominate(ob);
                if (!goal)
                    printf("I don't understand where you want to %s it.\n", verbword);
                else
                    PutThisThere(ob, goal);
                nuthin = false;
            } else if (HereL(ob)) {
                printf("I'm not sure which %s you're referring to.\n", lastword);
                nuthin = false;
            } else {
                printf("I don't see any %s here...\n", lastword);
                nuthin = false;
            }
        } else if (ob == vplug) {
            PlugItIn();
            return;
        }
    }
    if (allflag && nuthin) {
        // avoid retrying things already attempted?
        nominations = 0;
        if (!goal) {
            printf("I don't understand where you want to %s them.\n", verbword);
            return;
        }
        for (ob = firstobject; ob <= lastobject && !death; ob++)
            if ((Have(ob) || Here(ob)) && ob == Jug(ob) && location[ob] != goal && container[ob] != goal
                                       && !(location[ob] == incar && yourroom != driverseat)) {
                Nominate(ob);
                PutThisThere(ob, goal);
            }
        if (Have(gleeps) || Here(gleeps))
            PutGleeps(goal, true);
        nuthin = nominations == 0;
    }
    if (nuthin)
        printf("I don't understand what you want me to %s.\n", verbword);
}
