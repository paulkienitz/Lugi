/* GOTHERE.C
   functions for making movements (including bowel) in the game Lugi --
   go various places, including into battle */


#include "lugi.h"


import bool GuardTooLate(void);

import bool goralive, strong, peeked, attached, glued, clean, badhygiene,
            beastfollowing, guardalive, guardsees, plantslunged;

import ushort flies;

import Meaning lastdirection, tickledLastTurn;

import Sickness sickness;

import Seconds fogtime, guardtime;

import str bestword[objectcount + 1];

import Place /*ParsePlace(bool taking),*/ Distant(Place p), runtroom;

import Meaning Jug(Meaning ob), Understand(str word);

import void Attack(void), DropThis(Meaning ob, bool donom);

bool pissed = false, shat = false;

Meaning lugitickled = nomeaning;



PUBLIC void GoToTheBathroom(bool numbertwo)
{
    //Place goal = ParsePlace();    // support e.g. "piss into flask"?  nah, forget it
    if (!numbertwo && pissed) {
        puts("Your bladder is empty.");
        did = false;
    } else if (numbertwo && shat) {
        puts("Your bowels are empty.");
        did = false;
    } else if (env == bathroom || env == cleanbathroom) {
        puts("Your waste matter fizzes as it hits the water.  The smell is horrible, \
and a scummy layer of eecccch settles on all the fixtures.");
        map[yourroom].features = env = bathroom;      /* un-clean the place */
        clean = false;      // hygiene is still ok
        if (numbertwo) shat = true;
        else pissed = true;
    } else if (env == mensroom) {
        puts("I just told you that the toilets and urinals are all clogged with dust.");
        did = false;
    } else if (numbertwo) {
        shat = badhygiene = true;
        puts("Since no toilet is available, you lower your pants and defecate onto the \
floor.  Within seconds, strange growths appear on the surface of your waste matter.  \
With fantastic speed, the excrement becomes a miniature jungle of fantastic organisms, \
and soon it shrinks away.  Then the growths whither, leaving behind only a bit of dust.");
    } else if (Have(oflask) && Empty(inflask)) {
        puts("Since nothing else is available, you urinate into the flask.");
        location[ourine] = inflask;
        pissed = true;    // hygiene is still ok
    } else if (inwater) {
        puts("Since nothing is available to urinate into, you piss into the water, \
causing a slight change in its overall color.");
        pissed = badhygiene = true;
    } else {
        puts("Since nothing is available to urinate into, you piss on the ground.  \
The puddle begins to dry.");
        location[ourine] = yourroom;
        pissed = badhygiene = true;
    }
}



private Meaning TargetOfAttack(Meaning m, bool t)
{
    ushort gar, gor, run, pla, bea, gno, fli, thi;
    ushort primaries, secondaries;
    // lizards, rats, vermin, birds, wumpus can be considered tertiaries...
    // ghost too, and fungus, and distant lugonians such as the emperor
    Meaning who = (m == mit ? nomeaning : m);

    if (who > lastalive || who < firstalive) {
        gar = guardalive && Here(lguard);
        gor = goralive && Here(lgorilla);
        run = Here(lrunt);
        pla = env == garden;
        bea = beastfollowing;
        thi = attached;
        gno = env == hackerroom;
        fli = flies && !t;
        primaries = gar + gor + run;
        secondaries = pla + bea + gno + thi + fli;
        //if (t) primaries += secondaries;
        if (primaries > 1 || (!primaries && secondaries > 1))
            return mall;    // ambiguous
        else if (gar) return lguard;
        else if (run) return lrunt;
        else if (gor) return lgorilla;
        else if (pla) return fplants;
        else if (bea) return lbeast;
        else if (gno) return fgnome;
        else if (fli) return lflies;
        else if (thi) return lscalything;
        else return nomeaning;
    }
    return who;
}



PUBLIC void GetHacked()
{
    putsAlarm("\n\n\n\n\n\n\n\nHey, what the hell is going on?!  This doesn't make \
any sense.  A moment ago you were struggling with a strange gnome, now you're in a \
different room entirely, all the stuff you had is gone, and ... whenever you try \
to move your body does the wrong thing!  And all of a sudden there's a big ugly \
Lugiman in front of you!  You try to yell and what comes out is gibberish \
that doesn't even sound like you.  The whole world just doesn't seem to be \
working right at all.  It's as if you're in somebody else's dream... or \
inside a defective simulation.  Then you notice that your score in this \
game has suddenly been unexplainably lowered, your bank account is overdrawn \
for no reason, your credit card has erroneous charges...\n\n\n\n\
Then the Lugiman kills you.");
    death = hacked;         /* death #30 */
    did = true;
}



PUBLIC void BreakSomething()
{
    import void BreakFlask(bool thrown), Blast(bool thrown), FairMoan(bool thrown), BadgyTrap(void);
    Meaning what = GetWord();
    if (what == vopen) what = GetWord();
    if (what == mit) what = it;
    what = Jug(what);
    if (what && Understand(lastword) == mit) lastword = bestword[what];
    if (what >= firstalive && what <= lastalive) {
        UnGetWord();
        verb = vkill;       // deactivates automatic dolou
        Attack();
        return;
    }
    if (what && !Have(what) && !Here(what)) {
        printf("I don't see any %s here...\n", lastword);
        did = false;
        return;
    }
    did = isob(what);
    switch (what) {
        case onitro:    // fatal
            puts("You smash the nitro container on the ground.");
            Blast(false);
            break;
        case oflask:
            puts("The flask shatters easily.");
            BreakFlask(false);
            break;
        case ophial:    // fatal
            puts("The phial shatters on the ground.");
            FairMoan(false);
            break;
        case omatch:
            puts("You bust up the match into little fragments.  No more match.");
            location[omatch] = nowhere;
            break;
        case ofeather:
            puts("You rip the feather into tiny shreds.  It is no more.");
            location[ofeather] = nowhere;
            break;
        case osandwich:
            puts("You pull the sandwich apart into loose pieces that go everywhere.  No more sandwich.");
            location[osandwich] = nowhere;
            break;
        case opills:
            if (env == pillpile)
                puts("You grind a few pills underfoot.  There are plenty more.");
            else {
                puts("You grind the pills underfoot until nothing intact remains.");
                location[opills] = nowhere;
            }
            break;
        case okeys:
            did = false;
            if (!glued)
                puts("The keys are not breakable.");
            else if (!strong)
                puts("The glue is too strong for you to break.");
            else {
                puts("Your enhanced strength breaks the keys loose from the glue.");
                glued = false;
                did = true;
            }
            break;
        case orope: case obag: case ohammer: case ogalloncan: case oskull:
        case olysol: case oflysol: case oblackflag: case ostatuette: case owater:
            printf("The %s is not breakable.\n", lastword);
            did = false;
            break;
        case gleeps:
            puts("Gleeps are unbreakable.");
            break;
        // ------
        case fstove:
            puts("The stove is too rugged.  Not to mention too hot to touch.");
            break;
        case fnewspapers:
            puts("You tear up some newspapers.  There are still hundreds more.");
            did = true;
            break;
        case fbooks:
            puts("The books are made of durable materials.");
            break;
        case fwhips:
            puts("The whips are too tough to damage.");
            break;
        case fchains:
            puts("The chains and tongs are unbreakable.");
            break;
        case fbadge:    // fatal
            puts("The one thing that breaks is a thin thread which was attached to the badge...");
            BadgyTrap();
            break;
        case fpaintings:
            puts("The paintings are fastened to the wall, and made of something tougher than canvas.");
            break;
        case fdevice:
            puts("When you contact the floating gizmo, it gives you an electric shock!");
            break;
        case fsign:
            puts("The sign is epoxied in place.");
            break;
        case fcircuitry:
            puts("The circuitry is built solid as a brick wall, in typical Lugonian fashion.");
            break;
        case fcar:
            puts("Your appreciation for fine automotive craftsmanship prevents you from harming the car.");
            break;
        case fmachine:
            puts("Every part of the machine seems to be a massive chunk of steel.  There is no apparent way to damage it.");
            break;
        case fdirt:
            puts("Stomp away at it, and it'll still be dirt.");
            break;
        case fbones:
            if (env == caf) {
                did = true;
                puts("You manage to crack a bone or two, which has little noticeable effect on the supply scattered around the room.");
            } else if (env == torture)
                puts("You can't get inside the iron maiden to break the bones.");
            else
                puts("The skulls are not breakable.");
            break;
        case flaundry:
            puts("The cloth seems to be more durable than ordinary fabric.");
            break;
        case fgags:
            puts("Now that's funny -- you can't destroy any gags.");
            break;
        case ftar:
            puts("There's nothing you can do to the tar.");
            break;
        case fagents:
            puts("You can't bring yourself to further desecrate the remains.");
            break;
        case fcudgel:
            puts("The runt is holding his cudgel out of your reach.");
            break;
        case fbox:     // fatal
            puts("You hit a random piece of the assemblage of boxes, knocking \
it over.  The gnome, looking poth panicked and furious, throws itself \
between you and the collection of stuff.  Blocking you with his back, it \
pokes furiously at a flat thing protruding under the glowing box.");
            GetHacked();
            break;
        case fbattleship:
            puts("Good luck trying to harm the battleship.  Is that a torpedo in your pocket?");
            break;
        case fcrystal:
            puts("You can't enter the vacuum chamber, and believe me, you don't want to.");
            break;
        case ftrash:
            puts("Don't touch that garbage, you don't know where it's been!");
            break;
        case fcentrifuge:
            if (env == nasa) puts("The centrifuge has heavy duty construction.");
            else puts("The cage is too tough to break out of.");
            break;
        case fwindow:
            if (Have(ohammer)) {
                if (strong) {
                    puts("Swinging the sledgehammer with enhanced strength, \
you shatter the tough material of the big window.");
                    putsAlarm("\n\n\n\n\n\n\n\nYou step through onto the \
street outside.  You've escaped the embassy!");
                    death = escaped;
                } else puts("Even with the sledgehammer, you aren't strong \
enough to break the glass.");
            } else {
                if (strong) puts("Even with super strength, you don't have something \
heavy enough to break the glass with.");
                else puts("The big window is so tough it seems impossible to break.");
            }
            break;
        case nomeaning:
            printf("I don't understand what you want me to %s.\n", verbword);
            break;
    }
}



PUBLIC void Tickle(void)
{
    Meaning m = GetWord();
    Meaning who = TargetOfAttack(m, true);

    if (who == mall) {
        did = false;
        printf("You'll have to be specific about what to %s.\n", verbword);
        return;
    } else if (who && !Here(who)) {
        did = false;
        if (who == lbeast && yourroom == parcturus)
            puts("You can't reach the beast from here.");
        else printf("I don't see any %s here...\n", lastword);
        return;
    }
    switch (who) {
        case lscalything:
            if (Have(ofeather)) {
                puts("You tickle the scaly thing with the feather.  It squeaks \
loudly and wriggles around, its claws digging painfully into your calf \
muscle... after a few seconds it suddenly lets go and runs out of the room.");
                attached = false;
            } else
                puts("You tickle the scaly thing on your leg, with your \
fingers.  It gives a brief squeak and wriggles a bit, but does not loosen its \
grip.  Perhaps you need a better way to tickle it.");
            break;
        case lguard:
            if (!guardalive) {
                puts("The dead guard, naturally, is unresponsive to tickling.");
                break;
            } /* else fall through: */
        case lrunt:
            puts("You tickle the Lugiman.  He shrieks and squirms in \
in helpless convulsions!  He's powerless to do anything to you, as long as you \
DON'T STOP tickling him!");
            lugitickled = who;
            break;
        case lgorilla:
            if (!goralive)
                puts("You tickle the gorilla, and yup, it's definitely dead.");
            else
                puts("You tickle the gorilla.  He likes it.  But he still \
blocks your way.");
            break;
        case lbeast:
            puts("The Emperor's animal ignores the tickling.");
            break;
        case fgnome:
            puts("The gnome reacts to being tickled by twitching and looking irritated.  \
After a moment, it turns back to its boxes and starts rapidly poking a flat thing in \
front of one of them.  Its expression turns into something like a smirk.");
            GetHacked();
            break;
        case fplants:
            puts("You attempt to tickle the pulpy stems...");
            plantslunged = true;
            break;
        case nomeaning:
            puts("There's nothing here you can tickle.");
            did = false;
            break;
        default:
            puts("That's not something that can be tickled.");
            did = false;
    }
}



PUBLIC void Attack(void)     // this verb does not automatically say dolou
{
    Meaning m = GetWord();
    Meaning who = TargetOfAttack(m, false);
    bool muscle = strong && sickness <= mild;

    did = false;
    if (m == mit) m = it;
//    if (isob(m) && Here(m)) {    // if we ever reactivate this, include furniture
//        UnGetWord();
//        BreakSomething();
//        return;
//    }
    if (!who) switch (env) {
        case garden: who = fplants; break;
        case pillpile: who = fvermin; break;
        case arcturus: case fogarc: case vent:
            colorNormal();
            puts("How?");
            verb = nomeaning;   // suppress dolou
            return;
        case ghost: who = fghost; break;
        case rat: who = frats; break;
        case wumpus: who = fwumpus; break;
        case picturetube:
            puts("You can't get at them through the glass.");
            return;
        case lizardhole: who = flizards; break;
        case machine: who = lguard; break;
        case peekhole:
            if (peeked) {
                did = true;
                puts("You poise yourself, ready to attack the Lugiman the moment he appears...");
            } else printf("There's nothing here to %s.\n", verbword);
            return;
        case freebirds: who = fbirds; break;
        case hackerroom: who = fgnome; break;
        default:
            printf("There's nothing here to %s.\n", verbword);
            return;
    }
    if (who && Here(who)) {
        switch (who) {
            case fvermin:
                puts("The critters never come out in the open.");
                break;
            case flizards:
                puts("You stomp on a lizard and kill it.  Another one drags the body away.");
                did = true;
                break;
            case fplants:
            case lfungus:
                colorNormal();
                puts("How?");
                verb = nomeaning;   // suppress dolou
                break;
            case fghost:
                puts("The ghost is already dead.");
                break;
            case frats:
                puts("A few of the rats are now dead.  Congratulations.");
                did = true;
                break;
            case fbirds:
                puts("The birds never come within reach.");
                break;
            case fgnome:
                puts("You lock your forearm around the gnome's neck and clamp down hard.  The \
gnome desperately reaches for a box and taps its fingers on a strange \
flat thing protruding from the front.  And suddenly...");
                GetHacked();
                break;
            case lscalything:
                death = killed;         /* death #27 */
                did = true;
                puts("You do your best to kill the scaly thing clinging to your leg.");
                putsAlarm("\n\n\n\n\n\n\n\nIt responds by biting into your calf muscle \
with fangs that inject poison which reduces you to agonizing convulsions within seconds, \
and a very very painful death minutes later.");
                break;
            case lgorilla:
                if (goralive) {
                    did = true;
                    if (muscle) {
                        goralive = false;
                        puts("You advance to the gorilla and throw a karate punch at its face.  Your \
tremendous strength smashes its skull.  It falls dead.");
                    } else puts("You attack the gorilla fiercely, and it effortlessly throws you aside.");
                } else puts("The gorilla is already dead.");
                break;
            case lguard:
                if (guardalive) {
                    did = true;
                    if (GuardTooLate() || tickledLastTurn)
                        puts("You start to attack, but too late...");
                    else if (muscle || location[ohammer] == pockets) {
                        location[ophial] = yourroom;
                        guardalive = false;
                        guardsees = false;
                        guardtime = never;
                        if (muscle) puts("Confident in your newfound strength, you attack \
the guard unarmed.  Before you know it, he's dead on the floor!  A small, delicate glass \
bottle rolls out of his pocket.");
                        else {
                            puts("Desperately, you swing your sledgehammer at the guard's \
head.  Fortunately, Lugonian reflexes are slow, and the blow kills the guard!  A small, delicate \
glass bottle rolls out of his pocket onto the floor.");
                        }
                    } else {
                        puts("Desperately, you attack the guard without a weapon...");
                        guardsees = true;
                    }
                } else puts("The guard is already dead.");
                break;
            case lrunt:
                runtroom = Distant(yourroom);
                did = true;
                puts("You move in to attack the runt.  He shrieks something \
about his allergy and runs out of the room.");
                break;
            case lflies:
                puts("These Lugonian flies dodge much better than Earth flies.");
                break;
            case lbeast:
                death = killed;         /* death #28 */
                did = true;
                puts("You wrap your fingers around the throat of the big ugly beast \
and try to kill it by strangulation.");
                putsAlarm("\n\n\n\n\n\n\n\nIt fights back desperately, and tears your guts out.");
                break;
        }
    } else if (yourroom == parcturus && (who == lguard || who == lbeast)) {
        colorNormal();
        puts("How?");
        verb = nomeaning;   // suppress dolou
    } else if (who == lguard && env == machine)
        puts("The Lugiman in coveralls is too far away to get at.");
    else if (who == m)
        printf("I don't see any %s here...\n", lastword);
    else if (who == mall)
        printf("You'll have to be specific about what to %s.\n", verbword);
    else printf("I don't understand what you want me to %s.\n", verbword);
}



PUBLIC void Enter(void)
{
    import void Moov(Meaning d);         /* forward declaration */
    did = (env == ghost || env == ghostbunker || env == car || env == nasa || env == drydock);
    switch (env) {
        case ghost: env = ghostbunker; break;
        case ghostbunker: env = ghost; break;
        case cabinet: puts("You don't fit in the cabinet."); break;
        case car:
            env = driverseat;
            yourroom = incar;
            break;
        case nasa:
            death = killed;             /* death #2 */
            puts("You clamber half way into the centrifuge cage...   A small, noisy Lugonian, \
perhaps a child, runs into the room and, squeaking with glee, plugs in the \
centrifuge.  It starts to spin...  In moments you are pinned in place...");
            putsAlarm("\n\n\n\n\n\n\n\nYou are unable to either climb into the chair \
or drop out the door as the awful force of the centrifuge's spin presses you harder and \
harder against the hard edge of the cage's door frame.  You can't breathe... \
your ribs are cracking...  In your last moments of conciousness you catch \
glimpses of a small deformed Lugonian entering the room, and the child (if \
such it is) running toward it, looking guilty.  Then, as bones break and \
tissues tear, all senses fade.");
            break;
        case drydock: Moov(vclimb); break;
        case archive: puts("It's not smart to enter a vacuum chamber."); break;
        default: puts("There's nothing here you can enter.");
    }
}



PUBLIC void Leave(void)     // this verb does not automatically say dolou
{
    char c;
    did = false;
    if (env == balcony)
        if (Have(orope)) {
            did = true;
            if (attached) puts("You start climbing down the rope, toward freedom, \
but the people see the hideous thing stuck to your leg, and throw you back!");
            else {
                death = escaped;
                putsAlarm("\n\n\n\n\n\n\n\nYou climb down the rope to freedom!");
                if (beastfollowing) puts("\nThe Emperor's animal, with surprizing nimbleness, climbs down after you.");
            }
        } else {
            puts("It's five floors down, and you don't have a rope.");
            colorNormal();
            do
                c = GetLetter("Do you want to jump for it?     [Y or N]  ");
            while (c != 'y' && c != 'n');
            if (c == 'y') {
                death = jumped;
                did = true;
            }
        }
    else if (env == ghost || env == ghostbunker)
        Enter();
    else if (env == car)
        puts("You can't find a door to the outside, and you can't break the thick glass.");
    else if (yourroom == incar) {
        env = fogtime < Timer() ? fogged : car;
        yourroom = pcar;
        did = true;
    } else {
        colorNormal();
        puts("How?");
        verb = nomeaning;  // suppress dolou
    }
}



PUBLIC void Moov(Meaning dir)
{
    Place path;
    Meaning m;
    did = false;
    if (verb == mdown) dir = vjump;
    else if (verb == mup) dir = vclimb;
    else if (verb == vclimb) {
        m = GetWord();
        if (m == min) {
            Enter();
            return;
        } else if (m == mout) {
            verb = vleave;
            Leave();
            return;
        } else if (m == mdown) {
            if (Have(orope)) dir = vjump;
            else {
                puts("You can't climb down without a rope.  You'll have to jump down.");
                return;
            }
        }
    }
    if (lastdirection == vjump) m = veast;
    else m = Opposite(lastdirection);
    if (goralive && dir != m && Here(lgorilla) && !(strong && sickness <= mild))
        puts("The gorilla blocks your way!");
    else if (env == balcony && dir == vjump) {
        if (verb == vclimb || verb == mdown) Leave();
        else death = jumped;
    } else if (yourroom == parcturus)
        if (strong && sickness <= mild) {
            death = killed;             /* death #21 */
            puts("You rend apart the bars of the cage with irresistible \
strength.  The tearing metal emits piercing shrieks...\n\n\n\n\n\n\n");
            if (env == fogarc)
                putsAlarm("...and some of the guards fire their weapons at the sound, reducing your \
body to a grease spot.");
            else putsAlarm("...and the guards open fire, reducing your flesh to a grease spot.");
        } else
            puts("You aren't strong enough to break out of the cage.");
    else if (env == driverseat)
        puts("You have to get out of the car first.");
    else {
        path = map[yourroom].paths[drec(dir)];
        if ((env != meadow || dir >= vjump) && !path)
            puts("...No way through!");
        else if (dir == vclimb && !(DHave(orope) || (strong && sickness <= mild) || env == drydock))
            puts("You can't climb up without a rope.");
        else {
            did = true;
            lastdirection = dir;
            if (env == meadow && (!path || RRand(3)))
                puts("  OK");           /* no movement */
            else {
                if (goralive && dir != m && Here(lgorilla))
                    puts("The gorilla blocks your way, but you push it aside.");
                if (env == drydock && dir == vclimb)
                    puts("You climb up the structure and into the ship itself.  Inside, \
it is clearly no ordinary battleship...");
                else if (dir == vclimb && !strong)
                    puts("You fling the rope up, and it catches on something...");
                else if (verb <= vbackward)
                    puts("  OK");
                if (beastfollowing)
                    puts("The Grugza Emperor's animal follows you.");
                if (dir == vclimb && Have(ohammer) && !(strong && sickness <= mild)) {
                    puts("\nAs you climb, the heavy hammer slips from your grasp!");
                    verb = vdrop;
                    if (location[ohammer] == inbag)
                        DropThis(obag, true);
                    else DropThis(ohammer, true);
                }
                yourroom = path;
                env = map[yourroom].features;
                peeked = false;
                if (fogtime <= Timer() && env != balcony && env != kerosene
                                        && env != meadow && env != ghostbunker
                                        && env != ghost && env != streetlight)
                    env = inwater ? fogsplash : fogged;
            }
        }
    }
}
