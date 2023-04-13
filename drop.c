/* DROP.C
   This handles the commands "drop" and "throw", plus accidental dropping
   like when things are too heavy, and events that happen when objects
   get jolted or broken open or spilled, in the game of Lugi.  Several verbs
   besides drop and throw can trigger these indirectly, such as open. */


#include "lugi.h"


import bool peeked, attached, bystanders, guardalive, burnt, beastfollowing,
                runtalive, galloncanopen, glued, goralive, wipeout;

import Seconds fogtime, guardtime;

import str bestword[objectcount + 1];

import Meaning Jug(Meaning ob);

import Warning lastwarning;

import void Nominate(Meaning ob), PutThisThere(Meaning ob, Place goal), BumpTimer(Seconds timeToLose);
import void ImplicitTakeOut(Meaning ob, Place where, str name);
import bool ImplicitTake(Meaning ob, str name, Place source, bool specific), GuardTooLate(void), PlantsTooLate(void);

import Place guardroom, runtroom, gorroom, dumproom;



#define Lossage  (Seconds) 100
#define Gainage  (- Lossage)

PUBLIC void Fumes()
{
    puts("A cloud of horribly foul smelling fumes rises from the flask!");
    location[ochemicals] = nowhere;
    did = true;
    if (guardalive && yourroom == guardroom) {
        if (env != fogged && env != fogsplash)
            puts("\nThe guard shrieks in dismay and runs out of the room!");
        guardroom = Distant(yourroom);
    }
    if (yourroom == runtroom) {
        if (env != fogged && env != fogsplash)
            puts("\nThe unhealthy runt shrieks and runs away!");
        runtroom = Distant(yourroom);
    }
    if (yourroom == parcturus) {
        putsAlarm("\nThe fumes drive the Lugimen into utter madness!\n\n\n\n\n\n\n");
        putsAlarm("Unable to escape the smell which is utterly intolerable to them, they run \
about shrieking and tearing at each other.  Many begin firing their weapons \
at random.  There are not many survivors; the Emperor is one of them, but \
you are not.");
        death = killed;                 /* death #22 */
        return;
    } else if (env == machine) {
        puts("\nThe Lugiman who was pouring the oil into the machine shrieks and runs \
away!  The machine starts to run hot...   hotter...   it shrieks and \
shudders...\n\n\n\n\n\n\n");
        putsAlarm(
"With a terrible crash that must be audible for miles, the machine tears \
itself asunder.  The huge mass of scorching hot metal collapses onto you, \
crushing you into a formless paste.");
        death = killed;                 /* death #23 */
        return;
    } else {
        if (env == vent) {
            BumpTimer(Lossage);
            puts("\nThe Lugimen eating lunch shriek and scramble for the exits to \
escape the smell!  But one brave young Lugiman leaps for the cook's Flysol spray \
and, straining to withstand the urge to flee, sprays the stuff all around the \
room.  The smell is quickly eradicated.  The other Lugonians, who are badly \
shaken, call him a hero.  Most of them resume eating, but some check their \
weapons and head out of the room to look for the perpetrator.  Over all, \
you've probably lost time.");
        } else if (lastwarning && env != meadow && env != balcony && env != streetlight) {
            BumpTimer(Gainage);
            puts("\nThe odor, which is intolerable to Lugimen, has probably gained you \
valuable time by driving back your pursuers.");
        }
        if (attached) puts("\nThe scaly thing on your leg starts purring.");
        if (peeked) putsAlarm("\nThe Lugiman whose privacy you violated is so enraged that he ignores the \
smell, normally intolerable to all Lugimen!");
    }
}



PUBLIC void Blast(bool thrown)
{
    int d;
    Meaning m;
    if (thrown && env != kerosene && env != driverseat && env != cube && env != bubble) {
        if (GuardTooLate() || PlantsTooLate())
            puts("You try to throw it quickly enough...");
        else if (yourroom == parcturus) {
            putsAlarm("BOOOOOOOMM!!\n\n\n\n\n\n\n\n\n\
With a frightening roar, the nitro detonates, causing this end of the \
ceiling to slowly collapse.  All the Lugimen run safely back, but you are \
trapped in the cage and can't emulate their escape, and you are squashed \
like a bug.");
            death = killed;             /* death #24 */
        } else if (env == balcony) {
            putsAlarm("BOOM -- The nitro hits the sidewalk below and detonates.  There are at least \
a dozen casualties.  You're in big trouble, if you get out of here alive.");
            bystanders = true;
        } else if (env == meadow)
            puts("BLLAAAAAAMM!!  The nitro detonates, destroying a bush \
and several square feet of grass.");
        else if (env == streetlight)
            puts("BLLAAAAAAMM!!  The nitro detonates, tearing up a bit of pavement.");
        else if (dumproom == yourroom) {
            putsAlarm("\n\n\n\n\n\n\n\nBLLAAAAAAMM!!  The nitro detonates, which ignites the cloud of \
vaporized acetone as well, extending the explosion to the inside of your lungs.");
            death = bloneparte;
        } else if (env == car) {
            putsAlarm("BLLAAAAAAMM!!");
            puts("The nitro detonates, and shatters the thick window glass.");
            if (guardalive && Here(lguard)) {
                puts("\nThe guard staggers to his feet, and then falls down dead.");
                guardalive = false;
            }
            putsAlarm("\n\n\n\n\n\n\n\nPicking yourself up, you find you can \
step unimpeded out into the city.  You've escaped from the embassy!");
            death = escaped;
        } else {
            bool solidfloor = env == cistern || env == tank;
            putsAlarm("BLLAAAAAAAMMM!!");
            if (solidfloor) puts("With a frightening roar, the nitro detonates, blasting \
openings in all four walls!  You are miraculously unscathed.");
            else puts("With a frightening roar, the nitro detonates, blasting openings in all four \
walls, and the floor and ceiling besides!  You are miraculously unscathed.");
            for (d = 0; d < (solidfloor ? 4 : ndirec); d++)
                map[yourroom].paths[d] = Distant(yourroom);
            map[yourroom].paths[RRand(drec(vwest) + 1)] = yourroom + 1;
            if (guardalive && Here(lguard)) {
                guardalive = false;
                guardtime = never;
                puts("\nThe guard staggers to his feet, and then falls down dead.");
                /* in older versions he hit the alarm before dying, and still dropped the pheromone */
            }
            if (Here(lrunt)) {
                runtroom = nowhere;
                runtalive = false;
                puts("\nThe unhealthy Lugonian runt catches the full force of the explosion.  You \
can't even tell where the body went.");
            }
            if (env == garden && !burnt) {
                puts("\nThe horrible plants are burnt to ashes instantly!");
                burnt = true;
                map[yourroom].features = env = burntplants;
            }
        }
    } else {
        if (location[onitro] == inbag)
            putsAlarm("\n\n\n\n\n\n\n\nKer-BLAAAMMO!!!  The nitro in the bag detonates and blows you to bits.");
        else
            putsAlarm("\n\n\n\n\n\n\n\nKer-BLAAAMMO!!!  The nitro detonates and blows you to bits.");
        death = bloneparte;             /* death #25 */
    }
    if (!death && location[onitro] == inbag) {
        puts("\nSince the nitro was in the bag when it detonated, said bag \
and all its contents have been completely destroyed.");
        for (m = firstobject; m <= lastobject; m++)
            if (location[m] == inbag) location[m] = nowhere;
        location[obag] = nowhere;
    }
    location[onitro] = nowhere;
}



PUBLIC void FairMoan(bool thrown)
{
    location[ophial] = nowhere;
    if (thrown && env == balcony) {
        puts("\n...and it shatters into a million pieces on the sidewalk below.  \
A powerful smell drifts up, a smell you recognize with dread...");
        putsAlarm("\nIt's Lugonian pheromone!  You hear yells and fast-running booted feet, and \
then hundreds of lust-crazed Lugimen dash out onto the street!  You are \
sickened by what follows...    When it's over, several of the innocent \
people that were on the street have been killed.  If you get out of here \
alive, you're going to be in a lot of trouble.");
        bystanders = true;
    } else {
        death = worsethandeath;         /* death #4 */
        puts("It breaks, and a powerful smell assaults your nostrils, a smell that \
instantly fills you with instinctive terror.  Suddenly you remember what it \
is...");
        if (thrown && (env == arcturus || env == fogarc))
            putsAlarm("\nIt's pure Lugonian pheromone!  Drawn by the overpowering, irresistible smell, \
the hundreds of Lugimen in the room all end up clustered around the spot where \
the phial broke, tangled into a ball of savage carnal lust.  They forget \
about you entirely, but more keep rushing into the room, and the orgy soon \
spreads throughout the hall... until it reaches you.");
        else putsAlarm("\nIt's pure Lugonian pheromone!  From every direction, guided by the \
overpowering, irresistible smell, hundreds of lust-crazed Lugimen suddenly \
dash to the spot!  Nothing stands in their way...");
        putsAlarm("\n\n\n\n\n\n\n\n\nThe resulting experience is a fate worse than death, \
so you don't really mind very much when they kill you and hang you in the meat locker afterward.");
    }
}



PUBLIC void Insolvent(Place goal)
{
    puts("The powerful acetone fumes make your eyes sting horribly.");
    if (!goal) goal = yourroom;
    location[oacetone] = goal;
    if (glued && location[okeys] == goal) {
        puts("\nThe glue that was holding the keyring down dissolves in the solvent.");
        glued = false;
    }
    if (yourroom == parcturus) {
        beastfollowing = false;
        location[oacetone] = nowhere;
        puts("\nThe Emperor's big animal dives forward and laps up every trace of the \
acetone, and then goes back to sit by the Emperor's feet.");
    } else if (beastfollowing) {
        beastfollowing = false;
        location[oacetone] = nowhere;
        puts("\nThe Emperor's big animal leaps forward and laps up every trace of the \
acetone, and then wanders off.");
    } else {
        if (env == balcony || env == meadow || env == streetlight)
            puts("\nFortunately, a bit of breeze helps dissipate the fumes.");
        dumproom = yourroom;
    }
}



PUBLIC void BreakFlask(bool thrown)
{
    Meaning o;
    location[oflask] = nowhere;
    if (location[ochemicals] == inflask) {
        if (thrown && env == balcony) {
            puts("Foul smelling fumes rise up from the sidewalk.");
            location[ochemicals] = nowhere;
        } else Fumes();
    }
    for (o = firstobject; o <= lastobject; o++)
        if (location[o] == inflask)
            if ((o == owater || o == ourine) && inwater)
                location[o] = nowhere;
            else location[o] = yourroom;
}



private void DropBag(Place goal, bool thrown)
{
    location[obag] = goal;
    if ((inwater || env == meadow) && !thrown) {
        if (inwater)
            puts("OK -- it splashes into the water.");
        else
            puts("OK -- it lands softly in the grass.");
        return;
    } else if (goal)    // when goal is nowhere the outcome has already been messaged
        puts("OK");
    if (location[onitro] == inbag)
        Blast(thrown);
    else if (!death && location[ophial] == inbag) {
        puts("The tiny bottle of clear fluid is inside the bag...");
        FairMoan(thrown);
    } else if (!death) {
        if (location[oflask] == inbag) {
            puts("The flask is inside the bag, and it breaks open.");
            BreakFlask(thrown);
        }
        if (location[ogalloncan] == inbag && galloncanopen && location[oacetone] == ingalloncan && goal) {
            puts("The one gallon can was open, and spills its contents.");
            Insolvent(goal);
        }
    }
}



private void DropKeys(Place goal)
{
    Appearance nenv;
    location[okeys] = goal;
    if (goal && yourroom == parcturus)
        puts("OK");
    else if (env == bubble) {
        env = arcturus;
        yourroom = parcturus;
        puts("zzzzzzzzzzzzzziiiipp - POW!  There is a dazzling flash of light, and a \
sudden wrenching discontinuity during which you feel nothing under your \
feet.  In a moment, the world stabilizes...");
    } else if (goal) {
        puts("The moment they hit:       zzzzzzzzzzzzzzzzzziiiiiipp - POOF!\n\
You are engulfed in thick gray smoke...");
        yourroom = Distant(yourroom);
        nenv = map[yourroom].features;
        if (nenv == balcony || nenv == kerosene || nenv == meadow || Timer() < fogtime)
            env = nenv;
        else if (nenv == cistern || nenv == tank)
            env = fogsplash;
        else env = fogged;
        peeked = false;
    }
}



/* Have(ob) must be true when you call this: */
PUBLIC void DropThis(Meaning ob, bool donom)
{
    Place goal;
    bool thrown = verb == vthrow;
    did = true;

    if (donom) Nominate(ob);
    if (thrown && env == balcony) {
        puts("It sails over the balcony and down toward the street.");
        goal = nowhere;
    } else if (yourroom == parcturus) {
        if (thrown) {
            puts("It flies across the throne room.");
            goal = nowhere;
        } else goal = parcturus;
    } else if (env == driverseat) goal = incar;
    else goal = yourroom;
    if ((env == ghost || env == ghostbunker) && ob != ophial )
        PutThisThere(ob, goal);
    else switch (ob) {
        case obag:
            DropBag(goal, thrown);
            break;
        case ohammer:
            if (goal) puts("CLUNK!");
            location[ohammer] = goal;
            if (goal && location[onitro] == goal) {
                putsAlarm("\n\n\n\n\n\n\n\nKer-BLLAAAAMM!  I'm afraid the jolt of the \
heavy hammer falling has detonated the nitroglycerine.  Bits of your flesh smear the area.");
                death = bloneparte;         /* death #20 */
            }
            break;
        case onitro:
            if (!thrown && inwater) {
                location[onitro] = yourroom;
                puts("OK -- it splashes into the water without detonating.");
            } else if (!thrown && env == meadow) {
                location[onitro] = yourroom;
                puts("OK -- it lands in the soft grass without detonating.");
            } else Blast(thrown);
            break;
        case okeys:
            DropKeys(goal);
            break;
        case oflask:
        case ophial:
            if (!thrown && inwater) {
                location[ob] = yourroom;
                puts("OK -- it splashes into the water, and doesn't break.");
            } else if (!thrown && env == meadow) {
                location[ob] = yourroom;
                puts("OK -- it lands unbroken in the soft grass.");
            } else if (ob == oflask) {
                puts("It breaks on impact.");
                BreakFlask(thrown);
            } else FairMoan(thrown);
            break;
        case ogalloncan:
            if (!galloncanopen && goal) {
                PutThisThere(ogalloncan, goal);
            } else {
                location[ogalloncan] = goal;
                if (goal) {
                    if (location[oacetone] == ingalloncan)
                        Insolvent(yourroom);
                    else if (location[owater] == ingalloncan || location[ourine] == ingalloncan) {
                        if (location[owater] == ingalloncan) location[owater] = yourroom;
                        if (location[ourine] == ingalloncan) location[ourine] = yourroom;
                        puts("The liquid runs out of the open can onto the floor.");
                    }
                    else puts("OK");
                }
            }
            break;
        case osandwich:
            if (thrown && env == balcony) {
                location[osandwich] = nowhere;
                wipeout = true;
            } else if (thrown && !(goralive && yourroom == gorroom)) {
                puts("It flies to pieces when it hits.  No more sandwich.");
                location[osandwich] = nowhere;
            } else PutThisThere(osandwich, goal);
            break;
        default:
            if (goal) PutThisThere(ob, goal);
            else location[ob] = nowhere;
    }
}



PUBLIC void DropThem(void)
{
    Meaning ob = GetWord(), oldit = it;
    bool allflag = false, anyflag = false;
    did = false;
    if (!ob) {
        if (bogusword) printf("I don't see any %s here...\n", bogusword);
        else printf("I don't understand what you want me to %s.\n", verbword);
        return;
    }
    for ( ; ob; ob = GetWord()) {
        if (ob == mall || ob == meverything) {
            allflag = true;
            continue;
        }
        if (ob == mcans) {
            for (ob = olysol; ob <= oblackflag; ob++) {
                if (Have(ob)) {
                    ImplicitTakeOut(ob, location[ob], lastword);
                    DropThis(ob, true);
                }
            }
            if (!did) printf("You don't have any %s to %s.\n", lastword, verbword);
        } else {
            if (ob == mit) {
                ob = oldit;
                lastword = bestword[ob];
            } else
                it = ob;
            if (liquid(ob)) ob = Jug(ob);
            if (Have(ob)) {
                if (ob < gleeps) ImplicitTakeOut(ob, location[ob], lastword);
                DropThis(ob, true);
            } else if (HereS(ob) && ob < gleeps) {
                if (location[ob] == yourroom && verb == vdrop)
                    printf("The %s is on the floor already.\n", lastword);
                else if (ImplicitTake(ob, null, location[ob], true))        // no gleeps, named obs only
                    DropThis(ob, false);
            } else printf("You don't have any %s to %s.\n", lastword, verbword);
        }
    }
    if (allflag) {
        for (ob = firstobject; ob <= gleeps && !death; ob++)
            if (Have(ob))
                anyflag = true, DropThis(ob, true);
        if (!anyflag) printf("You have nothing to %s.\n", verbword);
    }
}
