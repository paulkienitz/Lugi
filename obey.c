/* OBEY.C
   This is the Big Part of the game Lugi -- the part that obeys the player's
   commands.  Some of the routines this calls will have to be farmed out to
   other source files, called take.c, drop.c, put.c, fiddle.c, and gothere.c
   (and as a special case, embassy.c for cheat-testing). */


#include "lugi.h"


import Meaning lastdirection;

import bool peeked, plugged, moved, trunkopen, attached, strong,
            beastfollowing, runtalive, fungus, cabinetopen;
import bool cabinetWasOpen, trunkWasOpen, tickledLastTurn, movedbefore;

import Seconds fogtime;

import Place runtroom;

import ushort beenin;

import Sickness sickness;

import char crashmessage[];

import void PutThem(void), Ignite(void), Attack(void), FillIt(void), SprayIt(void), PourIt(void),
            GoToTheBathroom(bool numbertwo), TakeThem(void), OpenIt(void), DropThem(void),
            EatIt(void), ShutIt(void), Enter(void), Leave(void), Tickle(void), BreakSomething(void)
            /*, Shell(void) */;

import void Inventory(Place where), Update(Place peekroom, Place plugroom), Moov(Meaning d), OpenAndShutCase(bool ope);



private void Peruse(void)
{
    if (env == library) {
        puts("\nThe writing consists of small elliptical arcs scattered over a fine \
triangular grid.  There's a faded note scrawled on one book, written with \
something that didn't stick properly to the surface being written on.  \
All that's legible is:\n\n"\
"     \"...could learn to harness their properties,\n\
      these keys could revolutionize every...\"\n\n\
There's a signature at the bottom.  It looks like it might be that of I.M. FisK, the noted explorer.");
    } else if (env == office)
        puts("\n     MAN SETS FOOT ON MOON\n\
     BROOKLYN DODGERS PENNANT WINNERS\n\
     RED CHINA EXPLODES ATOMIC BOMB\n\
     I.M. FISK MISSING IN KWAJALEIN ATOLL\n\
     DEWEY DEFEATS TRUMAN");
    else if (env == mensroom) {
        puts("\nYou attempt to translate from the Lugonian:\n\n\
     There was once an explorer [lit. conqueror] named [\"Pfis'q\"]\n\
     Whose [shoulders? hips? knees?] were exceedingly brisk.\n\
     So swift was his action,\n\
     That [\"Ra!oyguo\", prob. scientific term] contraction\n\
     Contracted his [Ancient High Tongue: cattleprod] to a disk.");
    } else if (env == hackerroom) {
        puts("\nThe glowing words on the box say\n");
        puts(crashmessage);
    } else if (env == lizardhole) {
        puts("The markings on the forms are not in any Earthly language.");
    } else {
        if (env == machine)
            puts("You can't translate the signs under these hellish conditions.");
        else if (env == sea || env == bubble || env == airlock)
            puts("There's nothing here to read except the sign.");
        else puts("There's nothing here to read.");
        did = false;
    }
}



private void Peeek(void)
{
    Meaning m = GetWord();
    if (env == peekhole) {
        if (peeked) puts("The small Lugonian is alone in the room, staring blankly at the wall.");
        else {
            peeked = true;
            puts("You look through the hole, and see two Lugonians in a small room, apparently \
engaging in noisy reproductive activity.  Something looks wrong, and after \
a moment you see what it is:  the more passive of the two is only about \
three feet tall and has a very small skull.  The larger one is holding an \
old, heavily decorated, projectile weapon.\n");
            putsAlarm("Suddenly, he freezes, and looks right at your peephole!  He gives a scream \
of rage and charges out of the room!  You can hear him blundering around, \
nearer and nearer!  If I were you, I would run!");
        }
    } else if (env == car) {
        if (m == min) m = GetWord();
        if (!trunkopen && m == mshelf) {   // trunk must be explicitly named
            UnGetWord();
            OpenAndShutCase(true);
        } else {
            put("The car is an old Audi.  The trunk is ");
            if (trunkopen) puts("open.");
            else puts("locked.");
        }
    } else if (env == cabinet && !cabinetopen)
        if (m == min || m == mshelf) {
            OpenAndShutCase(true);
        } else puts("The cabinet is closed.");
    else
        puts("All you see is this:");
}



private void Expectorate(void)
{
    if (Here(lrunt)) {
        runtalive = false;
        runtroom = nowhere;
        tickledLastTurn = false;
        puts("You spit forcefully, and the saliva hits the crazed Lugonian runt in the \
face.  He stops dead and emits a harsh, tearing scream!  He runs out of \
the room, shrieking in mortal agony.  The echoing cries are soon \
intermingled with harsh, wracking coughs.  He'll be no more trouble.");
    } else if (env == bubble)
        puts("The spit runs down the side of the bubble into your shoe.");
    else if (env == cistern || env == tank)
        puts("There is now a bit of foam on the water.");
    else if (env == balcony)
        puts("The spit flies over the balcony.  You hear a voice below \
suddenly explode into foul language.");
    else if (env == ghost || env == ghostbunker)
        puts("The spit disappears in a puff of smoke.");
    else puts("There is now some spit drying on the floor.");
}



private void PushButton(void)
{
    if (env == buzzcr || yourroom == parcturus) {
        puts("There is a dazzling flash of light and a sudden wrenching discontinuity \
as the floor seems to lurch madly.  You recover in a moment...");
        if (env == buzzcr) {
            yourroom = parcturus;
            env = arcturus;
        } else {
            yourroom = pbuzzcr;
            if (fogtime < Timer()) env = fogged;
            else env = buzzcr;
        }
    } else if (env == centralcr) {
        death = killed;                 /* death #1 */
        puts("The great viewscreen glows to life...   It shows a monstrous, bloated \
Lugonian face -- which you recognize as the Grugza Emperor Ra-Lugi himself, \
the corrupt and brutal absolute ruler of the Lugimen, sitting on his \
jewel-encrusted throne!\n");
        putsAlarm("He sees you!  He screams in rage and raises a massive, ornate weapon!  He fires at you...\
\n\n\n\n\n\n\n\n...and a searing beam lashes out of the image of the weapon on the screen \
and burns you to a crisp where you stand.");
    } else {
        puts("I don't see any buttons or switches here...");
        did = false;
    }
}



private void PlugInCentrifuge(void)
{
    unsigned i, j;
    Place r;
    if (plugged) puts("It's already plugged in.");
    else {
        did = plugged = true;
        puts("With some difficulty, you manage to force the adapter into the Imperial \
Standard socket.  The centrifuge starts to spin, more and more rapidly.  \
You hear strange sounds from inside the passenger cage...\n\n\
POP-POP-POPOPOP-POP POP!  The gleeps inside the cage disappear, scattering \
at random all over the place.");
        gleepct[yourroom] += 5 + RRand(5);
        for (i = 0; i < gleepct[incentrifuge]; i++) {
            j = 0;
            do {
                r = Distant(nowhere);
                j++;
            } while ((j < GleepClumping && !gleepct[r])
                        || map[r].features == ghost || map[r].features == bunker
                        || map[r].features == ghostbunker);
            gleepct[r]++;
        }
        gleepct[incentrifuge] = 0;
    }
}



PUBLIC void PlugItIn(void)
{
    Meaning m = min;
    did = false;
    if (verb == vplug) do m = GetWord(); while (m && m != min);
    if (!m)
        puts("I don't understand what you mean about plugging.");
    else if (env == nasa)
        PlugInCentrifuge();
    else puts("I don't see anything you can plug in.");
}



PUBLIC void Unplug(Place plugroom)
{
    did = yourroom == plugroom;
    if (did)
        puts("OOOF -- it's stuck!  Oh Christ...");
    else 
        puts("I don't see anything plugged in here...");
}



private void StartIt(void)
{
    if (env == car && Have(okeys)) {
        env = driverseat;
        puts("You get in the car...\n");
    }
    if (env == driverseat || env == car) {
        if (Have(okeys)) {
            if (env== car)
                puts("You get inside...\n");
            if (location[onitro] == intrunk) {
                puts("GrrrRRRrrrRRRrrr--VAROOOM!  Unexpectedly, the car lurches sharply forward...");
                putsAlarm("\n\n\n\n\n\n\n\n\
Ka-BLAAAMMM!!  Something behind you explodes like a bomb, killing you.");
                death = bloneparte;             /* death #3a... left over from a former boobytrap */
            } else if (location[onitro] == pockets) {
                puts("GrrrRRRrrrRRRrrr--VAROOOM!  Unexpectedly, the car lurches sharply forward... \
and you lose your grip on the nitroglycerine, which tumbles to the floor...");
                putsAlarm("\n\n\n\n\n\n\n\nKa-BLAAAMMM!!  You and the car are torn to bits.");
                death = bloneparte;             /* death #3b */
            } else {
                puts("GrrrRRRrrrRRRrrrRRRrrr--VAROOOM!  The car lunges forward...");
                putsAlarm("\n\n\n\n\n\n\n\nYou crash through the big window and \
out onto the street.  You've gotten out of the Embassy alive!");
                death = escaped;
                if (gleepct[intrunk] > 0)
                    puts("\n\nBehind you, in the trunk, you hear a rapid series of popping \
noises.  After a few seconds, the pops become less frequent, and then halt.");
                gleepct[intrunk] = 0;
            }
        } else {
            did = false;
            puts("You don't have the keys.");
        }
    }
    else if (env == nasa)
        PlugInCentrifuge();
    else {
        did = false;
        puts("I don't see anything here you can start.");
    }
}



private void FullInv(void)
{
    if (!verbword[1] && GetWord()) {            /*  "I blah blah..." */
        puts("I don't understand.");
        did = false;
        return;
    }
    puts("You are carrying:");
    Inventory(pockets);
    if (attached) puts("\nThere is a scaly thing on your leg.");
    if (beastfollowing)
        puts("\nA big ugly animal is following you around.");
    if (fungus) puts(
"\nThere's a patch of irritating fungus growing on your arm.  It itches.");
    if (strong && !sickness)
        puts("\nYou have three times your normal strength.");
    else if (sickness) {
        if (strong) put("\nYou have super strength, but you ");
        else put("\nYou ");
        switch (sickness) {
            case mild:  puts("are a bit queasy."); break;
            case moderate:  puts("are rather sick."); break;
            case severe:  puts("are severely ill."); break;
            case critical:  puts("are dying of the Lugonian Plague.");
        }
    }
    if (beenin > 1)
        printf("\nYou have explored %u rooms.\n", beenin);
}



PUBLIC void HearAndObey(void)
{
#ifdef _DEBUG
    import void PunkinEater(void);
#endif
    char c;
    Place peekroom = (peeked ? yourroom : nowhere), plugroom = (plugged ? yourroom : nowhere);
    if (verb == ourine) verb = vpiss;
    if (verb == vrun)
        if (!(verb = GetWord())) verb = vrun;
    if (verb == min) verb = venter;
    if (verb == mout)
        if (GetWord() == vplug) verb = vunplug;
        else verb = vleave;
    cabinetWasOpen = cabinetopen;
    trunkWasOpen = trunkopen;

    //putchar('\n');
    colorEvent();
    did = (verb >= firstverb && verb <= lastverb) || verb == mup || verb == mdown;
    if (!did) {
        if (verb && verb < vnorth)
            if (!strcmp(verbword, "fly"))
                puts("You can't fly.");
            else {
                puts("I don't understand what you want to do with it.");
                it = verb;
            }
#ifdef _DEBUG
        else if (verb == cheatest)
            PunkinEater();
#endif
        else puts("I don't understand.");
        verb = nomeaning;
    } else switch (verb) {
        case vnorth: case vsouth: case veast: case vwest:
        case vjump: case vclimb: case mdown: case mup:
            Moov(verb); break;
        case vforward:
            Moov(lastdirection); break;
        case vbackward:
            Moov(Opposite(lastdirection)); break;
        case vtake:
            TakeThem(); break;
        case vput:
            PutThem(); break;  /* XXX  use PutThem for drop, throw */
        case vfill:
            FillIt(); break;
        case vdrop: case vthrow:
            DropThem(); break;
        case vinventory:
            FullInv(); break;
        case vignite:
            Ignite(); break;
        case vkill:
            Attack(); break;
        case vopen: case vshut:
            OpenAndShutCase(verb == vopen); break;
        case veat: case vdrink:
            EatIt(); break;
        case venter:
            Enter(); break;
        case vleave:
            Leave(); break;
        case vhelp:
            puts("\n\n\n\n\n\n\n\nThe Lugimen help themselves to those who need help!");
            death = eaten;          /* death #5 */
            return;
        case vread:
            Peruse(); break;
        case vspit:
            Expectorate(); break;
        case vrun:
            puts("Which direction?");
            verb = nomeaning;  // suppress dolou
            did = false;
            break;
        case vpour:
            PourIt(); break;
        case vbutton:
            PushButton(); break;
        case vspray:
            SprayIt(); break;
        case vshit: case vpiss:
            GoToTheBathroom(verb == vshit); break;
        case vfart:
            if (yourroom == parcturus) {
                puts("You can't summon up a fart right now.");
                did = false;
            } else if (env == balcony || env == meadow || env == streetlight)
                puts("Pfffpppppppttttt!  You fart with conviction.  There is now a bad smell.");
            else {
                puts("Pfffpppppppppttttt!  You fart with gusto.  The fart gases react with the \
semi-alien atmosphere, forming an opaque brown fog that thickens rapidly \
until you can't see two inches through it.");
                env = inwater ? fogsplash : fogged;
                fogtime = Timer();
            }
            break;
        case vfuck:
            puts("Nothing around here has the correct organs and/or orifices."); break;
        case vflush:
            if (env == bathroom) puts("Flub, gisssshhhh -- aa ga ga gurgurGABLOOOP!  hhhhsssssssssssssss.....");
            else if (env == mensroom) puts("You try the levers, and nothing happens.");
            else {
                puts("I don't see anything you can flush here...");
                did = false;
            }
            break;
        case vplug:
            PlugItIn(); break;
        case vunplug:
            Unplug(plugroom); break;
        case vstart:
            StartIt(); break;
        case vfind:
            puts("You'll have to do that yourself."); break;
        case vbust:
            BreakSomething();
            break;
        case vlook:
            Peeek(); break;
        case vdetonate:
            if (Have(onitro) || Here(onitro)) {
                puts("You give the nitro a good shaking, and...");
                putsAlarm("\n\n\n\n\n\n\n\nBLAAAAMMM!  -- blow yourself to bits.");
                death = bloneparte;         /* death #7 */
                return;
            } else {
                puts("I don't see anything here that will detonate.");
                did = false;
            }
            break;
        case vtickle:
            Tickle(); break;  /*
        case vshell:
            if (GetWord()) {
                puts("I don't understand.");
                did = false;
            } else Shell();
            break;           */
        case vquit:
            colorNormal();
            do
                c = GetLetter("Do you really want to stop the game?  [Y or N]  ");
            while (c != 'y' && c != 'n');
            if (c == 'y') {
                putsAlarm("\n\n\n\n\n\n\n\nYou sit on the floor until the Lugimen come to kill you.");
                death = chicken;                    /* death #6 */
                return;
            }
            did = false;
            break;
    }
    colorNormal();
    if (verb == vfuck || verb == vfind)
        did = false;
    if (plugroom && plugroom != yourroom) plugged = false;
    if (did && !death) {
        Update(peekroom, plugroom);
        movedbefore = moved;
        moved = lastenv != env;
    }
    if (!did && !death && verb)
        puts("\nWhat you wanna do, Lou?");
}
