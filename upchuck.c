/* UPCHUCK.C
   This contains routines that update the player's situation in the game of
   Lugi.  */


#include "lugi.h"


PUBLIC bool moved = false, movedbefore = false, shortgame = false;

PUBLIC Meaning roomit = nomeaning;


import Place guardroom, thingroom, gorroom, dumproom, runtroom;

import Meaning RandDirection(Meaning d);

import Place Shelf(Appearance a);

import void DropThis(Meaning ob, bool donom);

import bool DEmpty(Place p);

import bool attached, guardalive, runtalive, burnt, strong, plugged,
                badsandwich, overdose, beastfollowing, galloncanopen,
                guardsees, plantslunged, goralive;

import Meaning lugitickled;

import Seconds venttime, fogtime, thrfogtime, guardtime, planttime,
dumptime, sandtime, pilltime;

import const Meaning roomits[appearancecount];

import Sickness sickness;

import Warning lastwarning;

import ushort flies, weightlimit, Weight(Place where);


#define SafeTime (Seconds) 5
#define Lossage  (Seconds) 100
#define FogFade  (Seconds) 50

#define beep() putchar(7)
/* this is a background color flash on the Amiga, unless you install a
sound hack */

PUBLIC bool vented = false;

PUBLIC Meaning tickledLastTurn = nomeaning;

private Seconds gameTimeOffset = 0;


PUBLIC void BumpTimer(Seconds timeToLose)   // timeToLose can be positive or negative
{
    gameTimeOffset -= timeToLose;
}



private void SafeStuff(void)                /* things that can't kill you */
{
    Meaning m;
    if (Have(oskull) && env == ghost) {
        env = map[yourroom].features = scaredghost;
        putsEvent("\nThere's a deathly cold shriek:  \"EEEEEEEEEEKKKK!! A SKULL!!!\"");
        for (m = vnorth; m <= vclimb; m++)
            map[yourroom].paths[drec(m)] = Distant(yourroom);
        map[yourroom].paths[drec(RandDirection(vwest))] = yourroom + 1;
    }
    if (venttime != never) {
        if (Timer() - venttime > SafeTime && !vented) {
            BumpTimer(Lossage);
            colorAlarm();
            beep();
            put("\nToo late, clod!  One of them spotted you, and a troop of ");
            beep();
            put("guards has got up and headed quickly out of the room!  ");
            beep();
            puts("You've probably lost valuable time!");
            beep();             /* well, kinda safe... */
            colorNormal();
            venttime = never;
            vented = true;
        } else if (env != vent) {
            putsEvent("They didn't see you.");
            venttime = never;
            vented = false;
        } // else just hanging out in the vent
    }
    if (env != vent)
        vented = false;
    if (yourroom == thingroom) {
        attached = true;
        thingroom = nowhere;
        putsAlarm ("\nSome kind of small, scaly ... THING just lunged out of a dark corner \
and attached itself to your leg with a painful grip!  It's DISGUSTING!");
    }
    if ((env == fogged || env == fogsplash) && Timer() - fogtime >= FogFade) {
        fogtime = never;
        if (yourroom == incar) env = driverseat;
        else env = map[yourroom].features;
        putsEvent("\nThe fog has gradually been dissipating, and now you can see through it.");
    }
    if (env == fogarc && Timer() - thrfogtime >= FogFade) {
        thrfogtime = never;
        env = arcturus;
        putsAlarm("\nThe fog has gradually been dissipating, and now the guards can see you!");
    }
    /* formerly, small furry like monkey snatches your skull occasionally... omitted */
}



#define G_theft       50

private void GrowGleeps(void)
{
    int32 n = 0;
    Place p;
    if (moved != movedbefore) {
        do {
            p = Distant(nowhere);
            n++;
        } while ((n < GleepClumping && !gleepct[p]) ||
                        map[p].features == ghost || map[p].features == bunker
                        || map[p].features == ghostbunker);
        n = (gleepct[p] >> 3) + 1;
        gleepct[p] += n;
    }
    if (n && p == yourroom && env != kerosene && env != fogged && env != fogsplash) {
        if (n == 1) puts("\nPOP!  A gleep just appeared here.");
        else
            printf("\nPOP!  %s gleeps just appeared here.\n", format32u(n));
    } else if (!RRand(G_theft) && gleepct[pockets] >= 8) {
        n = RRand32(gleepct[pockets] >> 2) + 1;
        gleepct[pockets] -= n;
        if (n == 1) puts("\nPOP!  A gleep just disappeared from your pockets.");
        else printf("\nPOP!  %s gleeps just disappeared from your pockets.\n", format32u(n));
    }
}



#define TakesEffect (Seconds) 40
#define Strength    (Seconds) 240
#define SafeDose    3

private void Pills(void)
{
    if (strong && Timer() - pilltime >= Strength) {
        beep();
        pilltime = never;
        strong = false;
        putsAlarm("\nOh no, your super strength is wearing off!");
        verb = vdrop;
    }
    if (Timer() - pilltime >= TakesEffect && (!strong || overdose)) {
        putsAlarm("\nThose pills you took are starting to take effect...");
        if (!overdose && RRand(SafeDose)) {
            strong = true;
            putsEvent("\nA sudden feeling of power courses through your body.\n\n\
You now have three times your normal strength!");
        } else {
            putsAlarm("\n\n\n\n\n\n\n\n\
BANG!  Your skull just exploded.  (I think.)\n\
The Lugimen chuck your poisoned body into the incinerator.");
            death = ohdee;              /* death #8 */
        }
    }
}


PUBLIC bool GuardTooLate(void)
{
    return guardalive && Timer() - guardtime > SafeTime;
}


PUBLIC bool PlantsTooLate(void)
{
    return !burnt && Timer() - planttime > SafeTime;
}


private void Guarden(void)
{
    if (!lugitickled) {
        if (tickledLastTurn) {                  /* death #36a/36b */
            if (tickledLastTurn == lrunt)
                putsAlarm("\n\n\n\n\n\n\n\nThe instant you stop tickling the Lugiman, \
the cudgel swings down at your head.  You are too close to dodge away in time.");
            else
                putsAlarm("\n\n\n\n\n\n\n\nThe instant you stop tickling the Lugiman, \
he gets an enraged look on his face and swings a fist as \
heavy as a bowling ball at your head, splattering your skull like a watermelon \
dropped from a high-rise.");
            death = killed;
            return;
        } else if ((guardsees && guardalive && Here(lguard)) || (GuardTooLate() && lugitickled != lguard)) {
            putsAlarm("\n\n\n\n\n\n\n\n\
The guard sets his heat gun for medium rare and cooks you for dinner.");
            death = eaten;              /* death #9 */
            return;
        } else if (guardalive && env == lastenv && Here(lguard)) {
            guardsees = true;
            if (lugitickled != lguard) {
                BumpTimer(Lossage);
                beep();
                putsAlarm("\nThe guard sees you!  He reaches for his weapon and \
hits the alarm!  You've probably lost valuable time!");
                beep();
            }
        } else if (guardsees && guardalive) {
            guardsees = false;
            guardtime = never;
            putsEvent("Whew, you got away in time.");
        }
        if (Here(lrunt) && lastenv == env) {
            putsAlarm("\n\n\n\n\n\n\n\n\
The unhealthy Lugiman, while standing as far away as he can manage, \
smashes his cudgel down on your head, which is transformed into something \
resembling tofu by the impact.");
            death = killed;                 /* death #12 */
            return;
        }
    }
    tickledLastTurn = lugitickled;
    lugitickled = nomeaning;
    if ((plantslunged && env == garden) || PlantsTooLate()) {
        putsAlarm("\n\n\n\n\n\n\n\nGosh, the plants ate you.");
        death = eaten;          /* death #10 */
        return;
    } else if (env == garden && lastenv == garden) {
        plantslunged = true;
        putsAlarm("\nThe plants are livelier than they look; they're moving toward you!");
    } else if (plantslunged && !burnt) {
        plantslunged = false;
        planttime = never;
        putsEvent("\nYou evaded the plants.");
    }
    if (env == fogged && !burnt && !death && map[yourroom].features == garden) {
        death = eaten;          /* death #11 */
        putsAlarm("\n\n\n\n\n\n\n\n\
You have stumbled into a garden of hungry man eating plants, which \
immediately wrap you up and start digesting you.");
    }
}



#define Sicker (Seconds) 40

private void Disease(void)
{
    static bool hadsandwich = false;
    if (badsandwich && Have(osandwich)) {
        hadsandwich = true;
        if (sickness == critical && Timer() - sandtime > Sicker >> 1) {
            death = sick;               /* death #13 */
            putsAlarm("\n\n\n\n\n\n\n\nYou have died of the Lugonian Plague.");
        } else if (Timer() - sandtime > Sicker) {
            sickness++;
            sandtime = Timer();
            switch (sickness) {
                case mild:
                    putsAlarm("\nYou are starting to feel queasy and weak, as if you had the flu.");
                    break;
                case moderate:
                    putsAlarm("\nYou are definitely getting sick; you feel dizzy, feverish, and nauseous.");
                    break;
                case severe:
                    beep();
                    putsAlarm("\nBoy, are you sick.  It's getting hard to go on.  \
Your head aches, your guts are cramped, your \
temperature is at least 104 degrees, and you have little strength left.  \
You can hardly walk in a straight line.");
                    break;
                case critical:
                    beep();
                    putsAlarm("\nYou are barely able to stagger along.  Your body feels like it's \
floating.  On your skin, you see the telltale green-black puckering that \
tells you that you have the Lugonian Plague, and may have very little \
time left.\n\nYOU ARE DYING.");
            }
        } else if (sandtime == never) sandtime = Timer();
    } else {
        if (hadsandwich)
            sandtime = Timer();
        if (sickness && Timer() - sandtime >= Sicker) {
            sickness--;
            sandtime = Timer();
            switch (sickness) {
                case severe:
                    putsAlarm("\nWhew!  The symptoms of the Lugonian Plague are \
starting to abate, just in the nick of time.  The \
puckers are smoothing out a bit and regaining some of their normal color.  \
It looks like you won't die of it after all.");
                    break;
                case moderate:
                    putsAlarm("\nThe symptoms of illness you are suffering are \
improving, though you are still sick as a dog.");
                    break;
                case mild:
                    putsAlarm("\nThe feelings of sickness are dropping to a mild level.");
                    break;
                case none:
                    putsAlarm("\nThe mild feeling of illness you had is gone now.  You are fully healthy.");
            }
        }
        hadsandwich = false;
    }
}



#define GameTime (Seconds) 1200

PUBLIC void Hurry(void)
{
    Seconds length = GameTime << !shortgame;
    Seconds left = length + gameTimeOffset - Timer();
    int frak = (20 * left) / length;
    if (left <= 0) {
        death = eaten;                  /* death #14 */
        putsAlarm("\n\n\n\n\n\n\n\nTROMP...TROMP...TROMP...TROMP --\n\n\
    \"Yu yu, ohgt zriktii su aq BERUWSH e ozn!\"   chomp, slobber, crunch...\n\
    (Aha, looks like we found us some DINNER!)");
    } else if (!frak)
        putsAlarm("\nTromp...Tromp...Tromp...Tromp...");
    else if (frak < 3 && lastwarning < cries) {
        lastwarning = cries;
        beep();
        putsAlarm("\nYou hear the sounds of distant cries and booted feet!");
    } else if (frak < 7 && lastwarning < detected) {
        lastwarning = detected;
        beep();
        putsAlarm("\nThe Lugonian detectors have picked you up!  Time is growing short!");
    } else if (frak < 13 && lastwarning < ontrail) {
        lastwarning = ontrail;
        putsAlarm("\nI think the Lugimen are on your trail now, Jack...");
    }
}



private void oops(Meaning o)
{
    if (!death && Weight(pockets) > weightlimit && DHave(o))
        DropThis(o, true);
}




PUBLIC void Update(Place peekroom, Place plugroom)
{
    GrowGleeps();
    SafeStuff();
    Pills();
    if (death) return;

    if (flies >= 10000) {
        colorAlarm();
        put("\n\n\n\n\n\n\n\nTen thousand muscular Lugonian flies are quite \
capable of lifting your entire weight off the ground.  They land \
all over you, hang on tight, and lift you into the air!  Exerting their utmost \
combined strength, the flies ");
        if (env != balcony) put("carry you through the air, passing through different rooms and corridors, \
until you come to a balcony five floors above the street.  Straining, they ");
        puts("raise you up so that you can see the crowd below the balcony...\n\n\
and toss you over the side onto the street.");
        colorNormal();
        death = jumped;                 /* death #26, or involuntary escape */
        return;
    } else if (flies)
        flies += (flies >> 2) + RRand((flies >> 2) + 2) + 1;

    if (yourroom == peekroom) {
        putsAlarm("\n\n\n\n\n\n\nThe maddened Lugiman suddenly barges around \
the corner and meets you face to face!  Screaming in rage at your violation of \
his privacy, he raises his archaic ceremonial weapon, decorated with ancient \
engraved designs and writings in a forgotten tongue, and blows your brains out.");
        death = sluggard;               /* death #16 */
        return;
    }

    if (lastenv == arcturus && env == arcturus) {
        putsAlarm("\n\n\n\n\n\n\n\nThe Grugza Emperor's guards and officers \
fire their weapons, converting your body into a grease spot.");
        death = killed;                 /* death #17 */
        return;
    }
    if (env == fogarc && (Have(oacetone) || Here(oacetone)) && galloncanopen && !beastfollowing) {
        putsEvent("\nThe big ugly beast that was sitting by the Emperor's feet \
squeezes itself amazingly through the bars and into the cage with you.  \
It must have bones of rubber, or no bones at all.");
        beastfollowing = true;
    }

    if (plugroom == yourroom) {
        putsAlarm("\n\n\n\n\n\n\nWith a tremendous crash, the centrifuge flies \
into fragments.  A piece hits you in the temple and kills you on the spot.");
        death = sluggard;               /* death #18 */
        return;
    } else if (plugroom) {
        if (env != arcturus && env != fogarc && env != meadow && env != streetlight)
            putsEvent("\nA tremendous crash reverberates through the building...");
        plugged = false;
        map[plugroom].features = nasawreck;
    }

    if (dumproom)
        if (Timer() - dumptime > SafeTime) {
            if (env == balcony || env == meadow || env == streetlight) {
                putsEvent("The aetone has dried up.");
                dumproom = nowhere;
                dumptime = never;
                location[oacetone] = nowhere;
            } else {
                putsAlarm("\n\n\n\n\n\n\n\
You were too late!   *  *  *  *    The fumes are overcoming you...\n\n\
The floor spins, your legs buckle under you, and everything goes black.");
                death = sluggard;           /* death #19 */
                return;
            }
        } else if (yourroom != dumproom) {
            dumproom = nowhere;
            dumptime = never;
            location[oacetone] = nowhere;
            if (env != meadow && env != machine && env != fogarc && env != bubble
                              && env != streetlight && dumproom != parcturus)
                putsEvent("\nBehind you are the sounds of a large animal slurping up liquid.");
        }  // else you get another chance to leave on time (yes this was always allowed)

    Disease();
    if (death) return;
    weightlimit = ((basic_weightlimit + (strong ? basic_weightlimit << 2 : 0)) << 1) / (2 + (ushort) sickness);
    if (Weight(pockets) > weightlimit) {
        putsAlarm("\nYou aren't strong enough to carry all that junk around any more!  You drop stuff...");
        oops(ohammer);
        oops(orope);
        oops(ogalloncan);
        if (Weight(inbag) > 2) oops(obag);
        oops(oskull);
        oops(olysol);
        oops(oflysol);
        oops(oblackflag);
        oops(ostatuette);
        oops(obag);
        oops(oflask);
        /* we do not drop the nitro or the sandwich */
    }
    if (death) return;
    Guarden();
    if (death) return;
    Hurry();

    roomit = env != lastenv && roomits[env] && DEmpty(yourroom) && DEmpty(Shelf(env)) ? roomits[env] : nomeaning;
}
