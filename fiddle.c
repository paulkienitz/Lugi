/* FIDDLE.C
   Functions for doing miscellaneous things with objects in Lugi.
   Verbs handled here are open, shut, pour, fill, eat/drink, ignite, and spray. */


#include "lugi.h"


import bool trunkopen, cabinetopen, galloncanopen, clean, badhygiene,
            badsandwich, overdose, fungus, burnt, plantslunged;

import int nominations;

import ushort flies;

import str verbword, bestword[objectcount + 1];

import Seconds fogtime, thrfogtime, pilltime, planttime;

import const Place container[objectcount];

import Place dumproom, Shelf(Appearance a), ParsePlace(bool taking);

import Meaning Jug(Meaning ob), Understand(str werd);

import void Fumes(void), Enter(void), FairMoan(bool thrown), Blast(bool thrown);

import void Inventory(Place where), Nominate(Meaning ob), PutThisThere(Meaning ob, Place goal);

import int Weight(Place where);

import bool HereL(Meaning o), PlantsTooLate(void), ImplicitTake(Meaning ob, str name, Place source, bool specific);



// Unlike Here(), this applies even if obscured by fog.  This might get used more in a future version?
// Closing the trunk or cabinet does exclude items inside.  Should we make an exception for the cabinet?
PUBLIC bool IsInAreaOfEffect(Meaning ob, Place room)        
{
    ob = Jug(ob);
    if (location[ob] == inbag) ob = obag;
    return Have(ob) || location[ob] == room
                    || (room <= nrooms && location[ob] == Shelf(map[room].features))
                    || (room == parcturus && location[ob] == onarcshelf)
                    || (room == pcar && location[ob] == incar)
                    || (room == incar && IsInAreaOfEffect(ob, pcar));
}

// Normally we assume colorNormal is set, but when obeying a verb we assume colorEvent is set.

#define isspray(ob) (((int) (ob) < 32) && bit(ob) & (bit(olysol) | bit(oflysol) | bit(oblackflag)))

PUBLIC void SprayIt(void)
{
    Meaning what = GetWord();
    Meaning w2 = GetWord();
    bool found = true;
    did = false;
    if (what == mit) what = it;
    if (isspray(w2))
        what = mcans;
    if (isspray(what) && (found = ImplicitTake(what, lastword, location[what], true))) {
        if (what == olysol) {
            if (env == bathroom) {
                clean = did = true;
                map[yourroom].features = env = cleanbathroom;
                puts("Almost instantly, the scummy fixtures around you begin to sparkle!  \
The stink becomes a clean fresh smell.\n");
            }
            if (env == arcturus || env == fogarc) {
                thrfogtime = Timer();
                if (env == arcturus) {
                    env = fogarc;
                    did = true;
                    putsAlarm("Within seconds the lysol reacts with the alien atmosphere of Arcturus \
IV, forming a dense brown fog that nobody can see two inches through!");
                }
            } else if (env != balcony && env != meadow && env != streetlight) {
                if (env != fogged && env != fogsplash) {
                    putsAlarm("Within seconds, the disinfectant reacts with the semi-alien air of \
the embassy, forming a thick brown fog you can't see two inches through.");
                    did = true;
                }
                fogtime = Timer();
                if (env != kerosene) env = inwater ? fogsplash : fogged;
            }
            if (fungus) {
                if (did) putchar('\n');
                fungus = false;
                did = true;
                puts("The fungus on your arm stops itching.  It's gone!");
            }
            if (badsandwich && IsInAreaOfEffect(osandwich, yourroom))
                badsandwich = false;
        } else if (what == oflysol) {
            if (env == fogged || env == fogsplash) {
                fogtime = never;
                if (yourroom == incar) env = driverseat;   // unlikely
                else env = map[yourroom].features;
                puts("Like magic, the brown fog dissipates, and you can see!");
                did = true;
            }
            if (env == fogarc) {
                env = arcturus;
                thrfogtime = never;
                did = true;
                putsAlarm("Like magic, the brown fog dissipates, and the guards can see you!");
            }
            if (!flies && env != arcturus) {
                flies = 2;
                puts("The odor has drawn a couple of flies.");
                did = true;
            }
        } else {                /* oteeveeparty */
            if (flies && env != arcturus) {
                flies >>= 2;
                printf("%s of the flies die and fall on the floor.\n",
                       flies == 0 ? "The rest" : flies > 1 ? "Most" : "Some");
                did = true;
            }
            if (!fungus) {
                did = fungus = true;
                puts("Something starts to itch on your right arm.");
            }
        }
        if (!did) puts("You can't see any effects of the spray.");
        did = true;
        it = what;
    } else if (what == mall || what == meverything || what == mcans)
        printf("You can only %s one thing at a time.\n", verbword);
    else if (what && what < vnorth && found) {
        if (Understand(lastword) == mit) lastword = bestword[what];
        if (Here(what)) printf("That doesn't %s.\n", verbword);
        else printf("I don't see any %s here...\n", lastword);
    } else if (found)
        printf("I don't understand what you want to %s.\n", verbword);
}



// PourIt and FillIt are complicated, yet have almost no gameplay utility.
// Coding them was just a case of pointless indulgence.  But don't you dare neglect them.

PUBLIC void PourIt(void)
{
    Place goal = ParsePlace(false);
    Meaning o, what = GetWord();
    uint32 g;
    if (what == min || what == mout || what == mwith || what == mfrom)
        what = GetWord();
    // ambiguity: player might say "pour out water in flask" to mean "...from flask", whereas
    // normally we expect something like "pour water into gallon can" to refer to destination
    if (what == mit) what = it;
    if (liquid(what)) what = Jug(what);
    // TODO? special casery to handle pouring pills? or gleeps?
    if (goal == pockets || goal == nowhere) goal = yourroom;
    did = false;
    nominations = 0;
    if (isnoun(what) && !Have(what) && !Here(what)) {
        if (HereL(what))
            printf("I'm not sure which %s you're referring to.\n", lastword);
        else
            printf("I don't see any %s here...\n", lastword);
    } else {
        it = what;
        if (isob(what) && container[what]) {
            // implicittake the source container?  for now, pour things while leaving them in place
            bool arcspill = goal == onarcshelf && what != obag && Weight(container[what]);
            if (arcspill) goal = parcturus;
            for (o = firstobject; o <= lastobject; o++)
                if (location[o] == container[what]) {
                    Nominate(o);
                    if (o == ochemicals) {
                        puts("You break open the seal...");
                        Fumes();
                    } else
                        PutThisThere(o, goal);
                }
            g = gleepct[container[what]];
            if (g > 0) {
                Nominate(gleeps);
                // TODO: there should be code shared by GrabGleeps and PutGleeps instead of this duplication
                if (goal == inflask)
                    puts("They don't fit through the neck of the flask.");
                else if (what == ogalloncan)
                    puts("You can't get any gleeps back out of the one gallon can.");
                else if (env == nasa && goal == incentrifuge)
                    Enter();
                else {
                    did = true;
                    if (goal == inniche || goal == onarcshelf ||
                        (goal == yourroom && (env == ghost || env == ghostbunker))) {
                        puts("POP!  They disappear the moment they contact the surface.");
                        g = 0;
                    } else puts("OK");
                    gleepct[container[what]] = 0;
                    gleepct[goal] += g;
                }
            }
            if (arcspill && did) puts("\nI'm afraid everything spilled onto the floor.");
        }
        if (!nominations) printf("I don't understand what you want me to %s.\n", verbword);
    }
}



PUBLIC void FillIt(void)
{
#ifdef _DEBUG
//    import void NominatePlace(Place p), colorDebug(void);
#endif
    Place source = nowhere, goal = nowhere;
    Meaning substance = nomeaning, goalob = nomeaning, prevword = nomeaning, word;
    str stuffword = null;
    bool foundone = false;

    // can't use ParsePlace, since the roles of place and subject are reversed...
    // supported syntax: fill container (from floor or environmental water),
    // fill container with liquid (or anything), fill container from source,
    // in the future maybe allow container to default?
    while ((word = GetWord())) {
        if (word == mignore)
            continue;
        if (prevword == mwith) {
            if (!isob(word) && word != gleeps && word != fliquid) continue;
            if (!substance) substance = word, stuffword = lastword;
            if (substance == mit) substance = it;
        } else if (prevword == mfrom && !source) {
            if (word == mshelf) {
                source = Shelf(env);
                if (!source)
                    source = pockets;       // room has no shelf
            } else if (isob(word) && (Have(word) || Here(word)))
                source = container[Jug(word)];
            // else assume they referred to the environment... be pickier and complain?
        } else if (isob(word) && !goal)
            goal = container[goalob = word];
        else if (word == mit)
            goal = container[goalob = word = it];
        else if (isnoun(word) && goalob)
            goal = pockets;        // two goals = fail
        prevword = word;
    }
    if (prevword == mfrom || !source)
        source = yourroom;
    if (!substance && source == yourroom && inwater && (goal == inflask || goal == ingalloncan))
        substance = owater;
    // default goal container?  nah
    if ((isob(substance) || substance == gleeps) && (!goalob || goal == pockets))
        it = substance;
    else if (isob(goalob) && goal != pockets && !substance)
        it = goalob;
#ifdef _DEBUG
//    colorDebug();
//    put("Source ");
//    NominatePlace(source);
//    put(", goal ");
//    NominatePlace(goal);
//    printf(", substance %s\n", substance ? bestword[substance] : "nomeaning");
//    colorEvent();
#endif

    did = false;
    if (!goal || goal == pockets || (!Have(goalob) && !Here(goalob)))
        printf("I don't understand what you want me to %s.\n", verbword);
    else if (goal == ingalloncan && !galloncanopen)
        puts("The one gallon can is capped shut.");
    else if ((substance && !Have(substance) && !Here(substance))
                    || (source && Empty(source)) || (!substance && !source) || source == pockets) {
        if (HereL(substance)) printf("I'm not sure which %s you're referring to.\n", stuffword);
        else printf("I don't understand what to %s it with.\n", verbword);
        // no ImplicitTake, let pour and fill operate on floor
    } else {
        if (isob(substance) || substance == gleeps) {
            Nominate(substance);
            PutThisThere(substance, goal);
        } else {
            for (word = firstobject; word <= lastobject; word++)
                if (word != goalob && (location[word] == source ||
                                       (word == owater && source == yourroom && Here(owater) && !HereS(Jug(owater))))) {
                    foundone = true;
                    Nominate(word);
                    PutThisThere(word, goal);
                }
            if (gleepct[source]) {
                foundone = true;
                Nominate(gleeps);
                PutThisThere(gleeps, goal);
            }
            if (!foundone)
                printf("There's nothing %s to fill it with.\n", source ? "there" : "here");
        }
    }
}


/******    -- this is how Open and Close were originally coded: in a word, inconsistently
PUBLIC void OpenIt(void)
{
    Meaning what = GetWord();   // XXX  while what not valid, getword?
    if (what == mit) what = it;
    if (liquid(what)) what = Jug(what);
    if (what == mall || what == meverything || what == mcans) {
        puts("You can only open one thing at a time.");
        return;
    }
    if (isnoun(what) && !Have(what) && !Here(what))
        printf("I don't see any %s here...\n", lastword); 
    else if (what == ogalloncan) {
        if (galloncanopen) puts("The can is already uncapped.");
        else {
            puts("You unscrew the metal cap on the one gallon can, which is attached with a \
piece of plastic.  It contains:");
            galloncanopen = did = true;
            Inventory(ingalloncan);
            if (location[oacetone] == ingalloncan)
                puts("The acetone fumes make your eyes sting a bit for a minute.");
        }
    } else if (what == oflask) {
        if (location[ochemicals] == inflask) {
            if (ImplicitTake(oflask, lastword, location[oflask], true)) {
                puts("You tear open the seal on the top...");
                Fumes();
            }
        } else puts("It's already open.");
    } else if (what == ophial) {
        puts("You pry at the tiny bottle's stopper, trying to crack the seal...");
        FairMoan(false);
    } else if (isob(what))
        puts("It doesn't open.");
    else if (env == cabinet)
        if (cabinetopen) puts("The cabinet is already open.");
        else {
            cabinetopen = did = true;
            puts("The cabinet is now open.");
        }
    else if (env == car)
        if (trunkopen) puts("The trunk is already open.");
        else if (!Have(okeys)) puts("You don't have the keys.");
        else {
            trunkopen = did = true;
            puts("The trunk is now open.");
        } // gah, no support for car door...
    else if (env == ghost || env == ghostbunker)
        puts("The door is already open.");
    else if (what == fliquid)
        printf("I'm not sure what %s you mean.\n", lastword);
    else puts("There's nothing to open.");
    if (isob(what)) it = what;
}



PUBLIC void ShutIt(void)
{
    Meaning ob = GetWord();
    if (ob == mit) ob = it;
    if (liquid(ob)) ob = Jug(ob);
    if (Have(ob) || Here(ob)) {
        Nominate(ob);
        if (ob == ogalloncan)
            if (galloncanopen) {
                puts("OK");
                galloncanopen = false;
            } else {
                puts("It's already shut.");
                did = false;
            }
        else if (ob == oflask) {
            if (location[ochemicals] == inflask)
                puts("It's already sealed shut.");
            else puts("You don't have any kind of stopper for it.");
            did = false;
        } else puts("There's no way to shut that.");
        if (isob(ob)) it = ob;
    } else {
        did = false;
        switch (env) {
            case cabinet:
                if (cabinetopen) {
                    did = true;
                    cabinetopen = false;
                    puts("OK");
                } else puts("The cabinet is already shut.");
                break;
            case car:
                if (trunkopen) {
                    did = true;
                    trunkopen = false;
                    puts("The trunk is now closed.");
                } else puts("It's already shut.");
                break;
            case ghost: case ghostbunker:
                puts("It doesn't stay shut.");
                break;
            case tar:
                puts("The barrels are already sealed shut.");
                break;
            default:
                puts("There's nothing here you can shut.");
        }
    }
}
******/


PUBLIC void OpenAndShutCase(bool ope)
{
    Meaning what = GetWord();
    str already = ope ? "open" : "closed";
    if (what == mit)
        if (Have(it) || Here(it)) what = it;
        else what = nomeaning;
    if (liquid(what)) what = Jug(what);
    if (what == mall || what == meverything || what == mcans) {
        printf("You can only %s one thing at a time.\n", verbword);
        return;
    }
    /* XXX  while what not valid, getword? */
    did = false;
    if (isnoun(what) && !Have(what) && !Here(what))
        printf("I don't see any %s here...\n", lastword);
    else if (location[oflask] == incar)
        puts("You have to get inside the car to mess with it.");
    else {
        if (isob(what)) it = what;
        if (what == ogalloncan) {
            if (ope == galloncanopen)
                printf("The can is already %s.\n", ope ? "uncapped" : "capped");
            else {
                galloncanopen = ope;
                did = true;
                if (ope) {
                    puts("You unscrew the metal cap on the one gallon can, which is attached \
with a piece of plastic.  It contains:");
                    Inventory(ingalloncan);
                    if (location[oacetone] == ingalloncan)
                        puts("The acetone fumes make your eyes sting a bit for a minute.");
                } else
                    puts("OK -- the can is now securely capped.");
            }
        } else if (what == oflask) {
            if (ope && location[ochemicals] == inflask) {
                puts("You tear open the seal on the top...");
                Fumes();
            } else if (ope) puts("It's already open.");
            else if (location[ochemicals] == inflask)
                puts("It's already sealed shut.");
            else puts("You don't have any kind of stopper for it.");
        } else if (what == ophial && ope) {
            puts("You pry at the tiny bottle's stopper, trying to crack the seal...");
            FairMoan(false);
        } else if (isob(what))
            puts(ope ? "It doesn't open." : "There's no way to shut that.");
        else if (env == cabinet) {
            if (cabinetopen == ope)
                printf("The cabinet is already %s.\n", already);
            else {
                cabinetopen = ope;
                did = true;
                printf("The cabinet is now %s.\n", already);
            }
        } else if (env == car) {
            if (trunkopen == ope) printf("The trunk is already %s.\n", already);
            else if (ope) {
                if (!Have(okeys)) puts("You don't have the keys.");
                else {
                    trunkopen = did = true;
                    puts("The trunk is now open.");
                } // gah, no support for car door...
            } else {
                trunkopen = false;
                did = true;
                puts("The trunk is now closed.");
            }
        } else if (env == ghost || env == ghostbunker)
            puts(ope ? "The door is already open." : "It won't stay shut.");
        else if (env == tar)
            puts("The barrels are tightly sealed.");
        else if (what == fliquid)
            printf("I'm not sure what %s you mean.\n", lastword);
        else printf("There's nothing to %s.\n", verbword);
    }
}



PUBLIC void EatIt(void)
{
    Meaning ob = GetWord();
    bool suck, hg, freewater;

    if (ob == mit) ob = it;
    freewater = inwater || env == seep;
    if (!ob && freewater)
        ob = owater;
    if (isnoun(ob)) {
        hg = Have(gleeps);
        suck = (ob == gleeps ? hg : (ob == owater && freewater) || (ob == opills && env == pillpile) || ob == ffeces)
               || ImplicitTake(ob, lastword, yourroom, true);
        if (ob == owater && Have(owater) && location[ourine] == location[owater] && !freewater)
            ob = ourine;     // a fine blend -- two liquids one can
        if (suck) {
            if (ob == ogalloncan && location[oacetone] == ingalloncan) ob = oacetone;
            else if (ob == oflask && location[owater] == inflask) ob = owater;
            else if (ob == oflask && location[ochemicals] == inflask) ob = ochemicals;  // urine?  can blend?
            switch (ob) {
                case owater:
                    if (env == seep)
                        puts("You try to collect together enough to drink, but you can't.");
                    else if (freewater) {
                        Nominate(owater);
                        puts("It's refreshing, despite the slight rotted taste.");
                    } else if (location[owater] == inflask) {
                        location[owater] = nowhere;
                        puts("The water in the flask is refreshing, despite a slightly funny taste.");
                    } else /* water in galloncan */
                        puts("You drink a little... it stings your throat.  There \
seems to be a trace of acetone still in there.");
                    break;
                case ochemicals:
                    puts("You break open the membrane sealing the flask...");
                    Fumes();
                    break;
                case ourine:
                case ffeces:
                    death = stupidity;                  /* death #29 */
                    badhygiene = true;
                    if (ob == ourine) put("Steeling your nerves, you drink it down.  ");
                    else put("Well, there's only one way to obtain some for consumption...  \
you lower your pants and do your business onto the floor.  Then, steeling \
your nerves and holding your nose, you grab a handful and start eating.  ");
                    puts("Five seconds later, you find yourself vomiting repeatedly and violently.");
                    putsAlarm("\n\n\n\n\n\n\n\nThe Lugimen find you curled up on the floor \
dry-heaving, and hand you over to the cook, \
who looks disgusted and throws you into the garbage grinder.");
                    break;
                case opheromone:
                    put("You try to crack open open the sealed top...  ");
                    FairMoan(false);
                    break;
                case oacetone:
                    if (!galloncanopen) puts("You unscrew the cap...");
                    puts("(glug-glug-glug-glug-glug-glug......)              (burp!)");
                    putsAlarm("\n\n\n\n\n\n\n\n\nAcetone happens to be toxic.");
                    death = stupidity;                  /* death #31 */
                    break;
                case osandwich:
                    if (badsandwich) {
                        death = killed;                 /* death #32 */
                        puts("(munch, munch, munch...)");
                        putsAlarm("\n\n\n\n\n\n\n\nIt's infected; you get sick and die.");
                    } else {
                        location[osandwich] = nowhere;
                        puts("(munch, munch...)    it tastes like breast of toad and velveeta.");
                    }
                    break;
                case opills:
                    if (Have(owater) || HereS(owater) || inwater) {
                        puts("You swallow a few of your pills, washing \
them down with water.  Nothing happens just yet...");
                        if (!inwater && (location[owater] == inflask || location[owater] == ingalloncan))
                            location[owater] = nowhere;
                        if (pilltime == never)
                            pilltime = Timer();
                        else {
                            overdose = true;
                            puts("\n(You know, that was probably an overdose...)");
                        }
                    } else puts("You try to swallow the pills, but you need \
water to wash them down with.");
                    break;
                case gleeps:
                    if (hg)
                        Nominate(gleeps);
                    puts("You try to fit one into your mouth, and POP!  It disappears.");
                    if (gleepct[pockets]) gleepct[pockets]--;
                    else if (Have(obag) && gleepct[inbag]) gleepct[inbag]--;
                    break;
                case fliquid:
                    did = false;
                    break;          // ImplicitTake has said the excuse
                default:
                    puts("That can't be swallowed.");
                    did = false;
            }
            if (ob == ochemicals) it = oflask;
            else if (isob(ob)) it = ob;
        } else did = false;         // ImplicitTake has said don't see any
    } else if (ob == mall) {
        puts("You can only swallow one thing at a time.");
        did = false;
    } else {
        puts("I don't understand what you want to swallow.");
        did = false;
    }
}



PUBLIC void Ignite(void)
{
    import bool wetmatch;
    /* parse a noun someday? */
    if (Have(omatch)) {
        location[omatch] = nowhere;
        if (wetmatch)
            puts("Your match is wet, and falls apart when you try to light it.");
        else if (env == kerosene || yourroom == dumproom) {
            putsAlarm("\n\n\n\n\n\n\n\nKA-BLAAAAAAAAMMM!!  The highly flammable vapor explodes, killing you.");
            death = bloneparte;                 /* death #33 */
        } else if (PlantsTooLate())
            puts("You try to get the match lit quickly enough...");
        else if (env == garden) {
            puts("The dry plants lunge at you!  An outstretched tendril \
touches your lit match ... and it ignites!  The fire spreads rapidly, and \
the dessicated plants burn to ashes in seconds!  You are \
left hot and soot-stained.");
            burnt = true;
            plantslunged = false;
            planttime = never;
            env = map[yourroom].features = burntplants;
            if (location[onitro] == yourroom) {
                puts("");
                Blast(true);
            }
            if (Have(onitro)) {
                death = bloneparte;             /* death #34 */
                puts("\nUnfortunately...");
                putsAlarm("\n\n\n\n\n\n\n\nBLAAAMMMM!!!  Your nitro detonates and blows you to bits.");
            }
        } else
            puts("Your match burns brightly for a moment and then goes out.");
    } else {
        puts("You have nothing that you can light.");
        did = false;
    }
}
