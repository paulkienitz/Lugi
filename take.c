/* TAKE.C
   This implements the crucial "take" command in Lugi, and the ability to
   implicitly take an object needed for some other command like "eat". */


#include "lugi.h"
#include <ctype.h>


import bool galloncanopen, glued, strong, beastfollowing, plugged;

import Sickness sickness;

import str bestword[objectcount + 1];

import Meaning roomit;

import const Place container[objectcount];

import void Inventory(Place where), Enter(void), Leave(void), Unplug(Place plugroom);

import ushort Weight(Place where), weightlimit;

import bool HereL(Meaning o);

import Meaning Jug(Meaning ob);

import Place ParsePlace(bool taking), Shelf(Appearance a);

import Meaning Understand(str werd);


PUBLIC int nominations = 0;

private bool silentfail;

private const str containames[11] = { "car", "cabinet", "niche", "trunk", "centrifuge",
                                      "shelf", "bag", "flask", "phial", "can", "inside" };



PUBLIC void Nominate(Meaning ob)
{
    register str nam = bestword[ob];
    if (ob == ogalloncan)
        put("One gallon can:  ");
    else if (ob == oblackflag)
        put("Black Flag:  ");
    else {
        putchar(toupper(*nam));
        put(nam + 1);
        put(":  ");
    }
    nominations++;
}



PUBLIC void ImplicitTakeOut(Meaning ob, Place where, str name)
{
    if (location[ob] == where && where != pockets && where > nrooms) {
        if (verb != vtake)
            printf("(take the %s out of the %s)\n",
                   name, containames[where - incar]);
        location[ob] = pockets;
        // XXX what about weight check?  we're only calling this for Have cases so far, but...
    }
}



PUBLIC void BadgyTrap(void)
{
    putsAlarm("\n\n\n\n\n\n\n\nIt's a booby trap for sentimental CIA \
agents!  Ports spring open in the walls, deadly muzzles protrude, and \
automatic Lugonian heat rays burn you to a crisp.");
    death = killed;                     /* death #35 */
}



private void Furniture(Meaning ob, str name)
{
    bool dood = true;
    if (HereL(ob)) switch (ob) {
        case fstove:
            puts("YYYYOOOOWWWWWWW!!  The stove is red hot!");
            break;
        case fnewspapers:
            puts("The newspapers crumble at a touch.  You'll have to read them here.");
            break;
        case fbooks:
            puts("The books are strapped down like telephone books.  You'll have to read them here.");
            break;
        case fwhips:
            puts("You can't take the whips for some reason.");
            break;
        case fchains:
            puts("The chains and stuff are all securely fastened to the floor and walls.");
            break;
        case fbadge:
            puts("You pick up the badge, and discover that a thin thread is attached to it...");
            BadgyTrap();
            break;
        case fpaintings:
            puts("All the furnishings are nailed in place so they don't fall down.");
            break;
        case fdevice:
            puts("The device gives you a powerful electric shock when you touch it!");
            break;
        case fsign:
            puts("The sign is epoxied in place.");
            break;
        case fcircuitry:
            puts("The circuitry is built solid as a brick wall, in typical Lugonian fashion.");
            break;
        case fcar:
            puts("You're not going to get it very far without starting it.");
            dood = false;
            break;
        case fmachine:
            puts("Did you say, pick up the machine?  Naah, I must be getting deaf.");
            dood = false;
            break;
        case fdirt:
            puts("You can't take any dirt for some reason.");
            break;
        case fbones:
            puts("A sudden feeling of sadness makes you leave the bones undisturbed.");
            dood = false;
            break;
        case flaundry:
            puts("The laundry is just too awkward to carry around.");
            break;
        case fgags:
            puts("The joke's on you!  You can't have a gag.");
            dood = false;
            break;
        case ftar:
            puts("The tar is too gooey to carry outside its 300 pound barrel.");
            break;
        case fagents:
            puts("The remains are fastened to their pedestals with seven foot metal spikes "
                 "stuck through the tops of their heads.");
            dood = false;
            break;
        case fbattleship:
            puts("Pick up the battleship?!  HO HO HO HO HA HA HA HO HO...");
            dood = false;
            break;
        case fcrystal:
            puts("You can't enter the vacuum chamber, and believe me, you don't want to.");
            dood = false;
            break;
        case ftrash:
            puts("Don't touch that garbage, you don't know where it's been!");
            dood = false;
            break;
        case fcentrifuge:
            if (env == nasa) puts("You can't take the centrifuge because it weighs 14 tons.");
            else puts("The cage is embedded into the floor.");
            dood = false;
            break;
        case fcudgel:
            puts("The runt is holding his cudgel out of your reach.");
            break;
        case fbox:
            puts("Your efforts to pick up the box are frustrated by a incomprehensible tangle \
of cables attached all over the back side of it.");
            break;
        case fliquid:
            if (env == columns) puts("The liquid is inaccessible inside the columns.");
            else printf("I'm not sure what %s you're referring to.\n", name);
            break;
        case fwindow:
            puts("The window is very solidly mounted into the wall.");
            break;
        // -------
        case fvermin:
            puts("The little animals are under the surface out of reach.");
            dood = false;
            break;
        case flizards:
            puts("The lizards are quick, and easily evade you.");
            break;
        case fplants:
            puts("If you try to take the plants, they'll eat you.");
            dood = false;
            break;
        case fghost:
            puts("If you can get a grip on the ghost, you're a better man than I am.");
            dood = false;
            break;
        case frats:
            puts("The rats are too fast for you to catch.");
            break;
        case fbirds:
            puts("The birds never come down where you can reach.");
            break;
        /* wumpus */
        case fgnome:
            puts("The gnome clings desperately, with mad strength, to its box.");
            break;
        case lscalything:
            puts("You can't break the scaly thing's grip on your leg.");
            break;
        case lgorilla:
            puts("Pick up the gorilla??   You're joking; it must weigh nearly as much as \
a full grown Lugiman!");
            break;
        case lguard:
            puts("The guard is far too big to carry.");
            break;
        case lrunt:
            puts("The runt is kinda scrawny, but he still weighs 400 pounds.");
            break;
        case lflies:
            puts("These Lugonian flies are much too quick to catch.");
            break;
        case lbeast:
            puts("The beast is too heavy to pick up.  Just let it follow you.");
            break;
        case lfungus:
            puts("You scratch at the fungus, and nothing comes loose.");
            break;
        default:
            silentfail = true;
            dood = false;
    } else {
        dood = false;
        if (env == arcturus && ob == lbeast && !beastfollowing)
            puts("The beast is out of reach, since you're stuck in the cage.");
        else
            printf("I don't see any %s here...\n", name);
    }
    did |= dood;
}



#define GleepLimit 15

private int SuckThemGleeps(Place p, bool *spoken)
{
    uint32 g = gleepct[p];
    int r = 1;
    if (!g || death) return 0;
    if (p == incentrifuge) Enter();             /* fatal */
    else if (p == incar && yourroom != incar) {
        if (!death) puts("You have to get into the car to take the gleeps there.");
        *spoken = true;
        return 0;
    } else if (p == ingalloncan) {
        if (!death && Have(ogalloncan)) {
            puts("You can't get any gleeps back out of the one gallon can.");
            *spoken = true;
        }
        return 0;
    } else if (p == inbag && !Have(obag))
        return 0;
    else if (g + gleepct[pockets] > GleepLimit) {
        g = GleepLimit - gleepct[pockets];
            r = -1;
    }
    gleepct[p] -= g;
    gleepct[pockets] += g;
    return r;
}


PUBLIC bool HasInside(Place p)    // places with insides, where it makes sense to put into and take out from
{
    return p > nrooms && p != parcturus && p != onarcshelf;   // true for pockets is a bit of a hack
}


/* How various commands are represented with parameters:

typical cmd       outie   specific  source    where gleeps are taken from
-----------       -----   --------  ------   ---------------------------
'take (gleeps)'   false   true      room     floor and shelf
'take out'        true    true      pockets  shelf (if HasInside), or bag if none there
'take all'        false   false     room     floor and shelf, try shelf2
'take all out'    true    false     pockets  shelf (if HasInside) and bag (not galloncan)
'take... from X'  either  either    X        source X only

...arrgh, should we just do like put and always assume all?  it would solve everything
*/
private bool GrabGleeps(Place source, bool specific, bool *spoken)
{
    int r = 0;
    Place s = Shelf(env), s2 = (!specific || !gleepct[intrunk]) && s == intrunk ? incar : nowhere;
    bool outie = HasInside(source);

    if (source != yourroom && source != pockets) {          // source explicitly named by user
        if (source == s || source == s2 || source == inbag || source == ingalloncan)
            r = SuckThemGleeps(source, spoken);
    } else {
        if (!outie)
            r |= SuckThemGleeps(yourroom, spoken);
        r |= SuckThemGleeps(s, spoken);
        if (!r || !specific)                 // you must say 'all' to target both shelves at once
            r |= SuckThemGleeps(s2, spoken);
        if ((source == inbag || (outie && !r) || (!specific && source == pockets)) && Have(obag))
            r |= SuckThemGleeps(inbag, spoken);
        if ((source == ingalloncan || (outie && !r) || (!specific && source == pockets)) && Have(ogalloncan))
            r |= SuckThemGleeps(ingalloncan, spoken);   // always fails
    }
    if (r && !death) {
        if (r < 0) puts("(You can't carry all the gleeps without putting them in a container.)");
        did = true;
        nominations++;
        if (gleepct[pockets] == 1) puts("Gleep:  OK");
        else printf("Gleeps:  You are now carrying %s.\n", format32u(gleepct[pockets]));
        return true;
    } else return false;
}



/*
These are the circumstances whereunder ImplicitTake shall be called, and the
several behaviours to be expected of it for each, if taking may be allowed:

typical cmd          location of ob       response          specific  return
-----------          --------------       --------          -------   ------
'take this'          floor, shelf         Nominate: OK      true      true
'take this'          pockets, container   You already have  true      true
'(any verb) this'    out of sight         I don't see any   true      false
'take all/cans'      floor, shelf         Nominate: OK      false     true
'take all/cans'      pockets, container   nothing           false     true
'take all/cans'      out of sight         nothing           false     false
'take this out'      shelf, container     Nominate: OK      true      true
'(other verb) this'  floor, shelf         (take the)/Nom:   true      true
'(other verb) this'  pockets              nothing           true      true
'(other verb) this'  container            (take out)/Nom:   true      true   -- XXX this doesn't nominate?
   ~~~ these cases will probably not be used: ~~~
'(other verb) all'   floor, shelf         (take the)        false     true
'(other verb) all'   container            (take out)        false     true
'(other verb) all'   out of sight         nothing           false     false

There is a case where this produces no output: when you go "take out x" or "take x from place"
and x is present but not where the preposition applies.  This sets silentfail -- caller must handle.
*/
PUBLIC bool ImplicitTake(Meaning ob, str name, Place source, bool specific)
{
    bool h, spoken = false, outie = HasInside(source);
    Meaning j = Jug(ob);
    Place frum = ob <= lastobject ? location[j] : nowhere;
    Place s = Shelf(env), s2 = s == intrunk ? incar : nowhere;

#if _DEBUG
//    printfDebug("ImplicitTake %s from %d %s, %s\n", bestword[ob], source,
//                source >= incar ? containames[source - incar] : "room", specific ? "specific" : "nonspecific");
#endif
    if (liquid(ob) && (!outie || HasInside(location[j])))
        ob = j;
    if (outie && ob <= lastobject)
        h = (Here(ob) && HasInside(frum)) || (!DHave(ob) && Have(ob));
    else if (outie)
        h = (ob == gleeps && (gleepct[s] || gleepct[s2] ||
                              (gleepct[inbag] && Have(obag)) ||
                              (gleepct[ingalloncan] && Have(ogalloncan))));
    else h = specific ? Here(ob) : HereS(ob);
    if (h && source != yourroom && source != pockets) {   // command specifically named a location to take from
        // must be where specified, but take "car" to mean trunk if it's accessible and stuff is there
        if (ob == gleeps) {
            if (source == incar && s == intrunk && gleepct[intrunk] > 0)
                source = intrunk;
            h = gleepct[source] > 0;
        } else
            h = location[ob] == source || (source == incar && s == intrunk && location[ob] == intrunk);
    }

    if (!name || Understand(name) == mit) name = bestword[ob];
    if (h) {
        if (ob > gleeps)
            Furniture(ob, name);
        else if (ob == gleeps) {
            // we could check alll to handle "all gleeps" differently... nah, we're good
            if (verb != vtake && (gleepct[pockets] > 0 || (Have(obag) && gleepct[inbag] > 0))) {
                Nominate(gleeps);
                return true;
            } else if (!(h = GrabGleeps(source, specific, &spoken)) && specific && !spoken && !death) {
                if (outie) puts("There aren't any gleeps to take out.");
                else puts("There are no gleeps here to take.");
            }
            it = ob;
            return h;
        } else if (Have(ob)) {
            if (outie && !DHave(ob)) {
                ImplicitTakeOut(ob, location[ob], name);   // always succeeds
                Nominate(ob);
                it = ob;
                did = true;
                puts("OK");
            } else if (specific) {
                if (ob == oskull)
                    puts("You already have one skull, and that's plenty.");
                else if (ob == opills)
                    puts("You already have plenty of pills.");
                else puts("You already have plenty of water.");
            }
        } else {   // XXX check specific before overnominating?  allow two levels of nonspecific for low nomination?
            if (verb != vtake)
                printf("(take the %s)\n", name);
            // note that blindness in kerosene or fogarc does not prevent picking up named objects by feel
            Nominate(ob);
            it = ob;
            if (ob == osandwich && location[osandwich] == yourroom && inwater) {
                puts("It's so soggy that it disintegrates when you try to pick it up.");
                location[osandwich] = nowhere;
                did = true;
                return false;
            }
            if (frum == incar && yourroom == pcar) {
                puts("You have to get in the car to take it.");
                return false;
            }
            if (glued && ob == okeys && (!strong || sickness > mild)) {
                puts("The glue is too strong for you to break.");
                return false;
            }
            if (outie && liquid(ob)) {
                puts("To get it out, you'll have to pour it on the floor.");
                return false;
            }
            /* for these next two, remember that liquid(ob) is only true at
               this point if the liquid is not in a container */
            if (liquid(ob) && !(ob == owater && inwater)) {
                puts("You can't pick up a thin layer of liquid.");
                return false;
            }
            if (ob == owater) {
                if (Have(oflask) && Empty(inflask))
                    location[owater] = inflask;
                else if (Have(ogalloncan) && Empty(ingalloncan)) {
                    galloncanopen = true;
                    location[owater] = ingalloncan;
                } else {
                    puts("You need to put it in a container.");
                    return false;
                }
            } else
                location[ob] = pockets;
            if (Weight(pockets) > weightlimit) {
                location[ob] = frum;    /* un-take */
                puts("You can't carry all that junk.  Drop something heavy first.");
                return false;
            }
            did = true;
            if (location[ob] == inflask)
                puts("OK -- you fill the flask with it.");
            else if (location[ob] == ingalloncan)
                puts("OK -- you put some in the one gallon can.");
            else if (glued && ob == okeys) {
                glued = false;
                puts("OK.  The glue is no match for your enhanced strength.");
            } else if (!galloncanopen && ob == ogalloncan) {
                put("OK -- it's capped, and it feels ");
                if (Weight(ingalloncan))
                    puts("full.");
                else puts("empty.");
            } else if (container[ob]) {
                puts("OK -- it contains:");
                Inventory(container[ob]);
            } else if (verb != vdrop && verb != vthrow)
                puts("OK");
        }
    } else if (specific) {
        if (Have(ob)) {
            if (verb == vtake)
                printf("You already have the %s.\n", name);
            else
                Nominate(ob);
        } else {
            Furniture(ob, name);        /* probably "I don't see any", or if outie it can set silentfail true */
            return false;
        }
    }
    return true;
}



PUBLIC void TakeThem()
{
    Meaning ob, m, oldit = it;
    bool alll = false, nuthin = true, every = false, sifa = false;
    Place source;
    did = false;
    m = GetWord();
    if (!strcmp(verbword, "get"))
        if (m == mout) {
            m = GetWord();
            if (!m) {
                Leave();
                return;
            } else
                m = UnGetWord();
        } else if (m == min) {
            Enter();
            return;
        }
        // ...let's not support turning "take out guard" or "take down guard" into attack
    colorEvent();
    if (m == mout) {
        m = GetWord();
        source = ParsePlace(true);    // might handle odd case like "take out X from bag"
        if (source == yourroom) source = pockets;   // secret code for "out" with no particular place named (for this verb only)
    } else source = ParsePlace(true);

    for (ob = m; ob && ob != min && ob != mout; ob = GetWord()) {
        silentfail = false;
        if (ob < vnorth || ob == mit || ob == mcans /*|| ob == mspraycans*/) {
            if (ob == mit)
                ob = roomit ? roomit : oldit;
            if (ob == mcans /*|| ob == mspraycans*/) {
                if (ImplicitTake(olysol, null, source, false) |   // no shortcutting, try all three
                        ImplicitTake(oflysol, null, source, false) |
                        ImplicitTake(oblackflag, null, source, false))
                    silentfail = false;
            } else       // XXX PreBiguate should eliminate mcans if they say e.g. 'cans of lysol and flysol'
                ImplicitTake(ob, lastword, source, true);
            if (silentfail)
                sifa = true;
            else
                nuthin = false;
        } else if (ob == vplug) {
            Unplug(plugged ? yourroom : nowhere);
            return;
        } else {
            if (ob == meverything) every = true;
            if (ob == mall) alll = true;
        }
    }
    if (every || (alll & nuthin)) {
        nominations = 0;
        for (ob = firstobject; ob <= gleeps && !death; ob++)
            if (ob == Jug(ob))
                ImplicitTake(ob, null, source, false);
        if (!nominations) puts("There's nothing here you can take.");
    } else if (nuthin)
        if (sifa && nuthin)
            puts("That isn't in there.");
        else if (!m && bogusword)
            printf("I don't see any %s...\n", bogusword);
        else printf("I don't understand what you want me to %s.\n", verbword);
    colorNormal();
}
