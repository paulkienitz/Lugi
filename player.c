/* PLAYER.C
   This is the part of Lugi that holds the data relevant to the exploring
   character in the game. */


#include "lugi.h"


import void PutDescription(Appearance a);

import Place Shelf(Appearance a);


import bool glued, plugged, plantslunged, wetmatch, goralive, lugitickled, tickledLastTurn, vented;

import str bestword[];

import Meaning roomit;

import const Meaning roomits[appearancecount];

import const Place container[objectcount];


PUBLIC bool overdose = false,           /* taken too many pills */
            badhygiene = false,         /* player has soiled himself or area */
            fungus = false,             /* fungus is growing on arm */
            attached = false,           /* thing is on leg */
            beastfollowing = false,     /* the emperor's animal is following you */
            bystanders = false,         /* you've killed innocent citizens */
            wipeout = false,            /* you've depopulated the city */
            tripped = false,            /* you're in small blind room with an object */
            peeked = false,             /* you've violated Lugonian privacy */
            strong = false,             /* you have super strength */
            galloncanopen = false,      /* the one gallon can is open */
            did = true,                 /* you've accomplished something this turn */
            cabinetWasOpen = false,     /* track whether cabinet was just opened */
            trunkWasOpen = false;       /* track whether trunk was just opened */

/* these would seem to belong in embassy.c, but they get used here: */
PUBLIC Seconds venttime = never,        /* when entered vent */
               dumptime = never,        /* when acetone poured */
               sandtime = never,        /* when sandwich taken or plague incremented */
               guardtime = never,       /* when guard turned around */
               planttime = never,       /* when plants lunged */
               pilltime = never,        /* when pills swallowed */
               fogtime = never,         /* when embassy fogged */
               thrfogtime = never;      /* when throneroom fogged */

PUBLIC ushort  flies = 0,                               /* how many flies buzzing around you */
               weightlimit = basic_weightlimit,         /* how much you can carry */
               beenin = 0;                              /* how many rooms explored */


PUBLIC Meaning lastdirection = vnorth;     /* direction of most recent move */

PUBLIC Sickness sickness = none;           /* progress of the lugonian plague */

PUBLIC Death death = alive;                /* how the player died (or won) */

PUBLIC Place   dumproom = nowhere,         /* where acetone was just spilled */
               yourroom;                   /* where in the map you are */

PUBLIC Appearance env, lastenv;            /* your current and previous surroundings */

private ushort weights[objectcount] = {     /* how heavy objects are */
        0, 2, 0, 5, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 0
};     /*  rope hammer nitro    galloncan..urine     acetone..sprays..statue */

private ushort invdepth = 0;


#define funnyplaces (pockets - nrooms)

PUBLIC str oddplaces[funnyplaces] = {
    "in the throne room",
    "visible inside the car",
    "in the cabinet",
    "in the niche",
    "in the car's trunk",
    "in the centrifuge's passenger cage",
    "on the little shelf",
    "in the bag",
    "in the flask",
    "in the tiny phial",
    "in the one gallon can",
    "being carried"
};



PUBLIC bool DEmpty(Place p)
{
    Meaning ob;
    if (!p) return true;
    if (gleepct[p]) return false;
    for (ob = firstobject; ob <= lastobject; ob++)
        if (location[ob] == p) return false;
    return true;
}



PUBLIC bool Empty(Place p)
{
    if (p <= nrooms && (map[p].features == cistern || map[p].features == tank)) return false;  // what about skulls?
    return DEmpty(p);
}



/* true if the player is carrying object o directly, not inside another
   object that is a container */
PUBLIC bool DHave(Meaning o)
{
    if (o == gleeps)
        return gleepct[pockets] > 0;
    else
        return isob(o) && location[o] == pockets;    // for e.g. beast or fungus use Here
}



/* true if the player is carrying object o, in or out of a container */
PUBLIC bool Have(Meaning o)
{
    if (o == gleeps)
        return gleepct[pockets] || (gleepct[inbag] && Have(obag))
                        || (gleepct[ingalloncan] && Have(ogalloncan));
    else if (isob(o))
        return location[o] == pockets || (location[o] == inbag && Have(obag))
                        || (location[o] == ingalloncan && Have(ogalloncan))    // closed cap is OK
                        || (location[o] == inflask && Have(oflask))
                        || (location[o] == intinybottle && Have(ophial));
    else return false;
}




private bool Inventory1(Place where, Meaning ob)
{
    PUBLIC void Inventory(Place where);    // forward declaration for recursing
    static str libel[objectcount] = {
        null,
        "coil of rope with grappling hook",
        "canvas bag, which contains:",
        "sledgehammer",
        "container of nitroglycerine",
        "oddly shimmering keys",
        "clear flask, which contains:",
        "delicate glass phial, which contains:",
        "one gallon can, which contains:",
        "mysterious chemicals",
        "water",
        "warm urine",
        "match",
        "clear, volatile-looking fluid",
        "feather",
        "acetone (a solvent)",
        "bloodstained skull",
        "half-eaten sandwich",
        "Lysol disinfectant",
        "Flysol infectant",
        "Black Flag insecticide",       /* "Teevee party tonight!" */
        "statuette",
        "pills and capsules"
    };
    register int i;
    if (location[ob] == where) {
        invdepth++;
        for (i = 0; i < invdepth; i++) put("    ");
        if (ob == ogalloncan && !galloncanopen)
            puts("one gallon can labeled \"ACETONE\", capped");
        else {
            if (wetmatch && ob == omatch) put("wet ");
            puts(libel[ob]);
            if (container[ob])
                Inventory(container[ob]);
        }
        invdepth--;
        return true;
    }
    else return false;
}


/* lists the objects in a certain place recursively */
PUBLIC void Inventory(Place where)
{
    uint32 g = gleepct[where];
    bool nonempty = false;
    register Meaning ob;
    register int i;

    if (invdepth > 10)
        puts("And so on.  AARRGH!!  Something contains itself??");
    else {
        for (ob = firstobject; ob <= lastobject; ob++)
            if (!container[ob])     // skip containers initially, show them last in most readable order
                nonempty |= Inventory1(where, ob);
        if (g > 0) {
            nonempty = true;
            for (i = 0; i <= invdepth; i++) put("    ");
            if (g == 1) puts("a gleep");
            else printf("%s gleeps\n", format32u(g));
        }
        nonempty |= Inventory1(where, ophial);       // the containers we skipped above
        nonempty |= Inventory1(where, oflask);
        nonempty |= Inventory1(where, ogalloncan);
        nonempty |= Inventory1(where, obag);
        if (!nonempty) {
            for (i = 0; i <= invdepth; i++) put("    ");
            puts("nothing.");
        }
    }
}



private void ShowGleeps(Place where, str lay)
{
    register uint32 g = gleepct[where];          /* where can be nowhere */
    if (g) {
        if (g == 1)
            put("There's a gleep ");
        else if (g < 24)
            printf("There are %s gleeps ", format32u(g));
        else if (g < 200)
            put("There are dozens of gleeps ");
        else if (g < 2000)
            put("There are hundreds of gleeps ");
        else
            put("There are thousands of gleeps ");
        put(lay);
        puts(".");
    }
}



PUBLIC void ShowObjects(void)
{
    import bool cabinetopen, trunkopen;
//#ifdef STRICT_SINGLE_NEW_OBJECT_IT
//    Meaning ob, newit = mit;
//#else
    Meaning ob;
//#endif
    bool linefed = false, onfloor;
    static str nane[objectcount] = {
        null,
        "There's a coil of rope with a grappling hook on one end ",
        "There's a roomy canvas bag ",
        "There's a heavy sledgehammer lying ",
        "There's a sealed container sitting ",
        "There's an oddly shimmering set of keys ",
        "There's an empty flask ",
        "There's a tiny, delicate, sealed glass phial ",
        "There's a one gallon can labeled \"ACETONE\" sitting ",
        "(chemicals) ",
        "There's some water spilled ",
        "There's a pool of urine slowly drying ",
        "There's an unused match ",
        "(pheromone) ",
        "There's a feather (a rather unearthly looking one) ",
        "There's a fuming puddle of rapidly evaporating acetone ",
        "There's a grisly bloodstained human skull ",
        "There's a half-eaten old sandwich ",
        "There's a spray can of Lysol disinfectant ",
        "There's a spray can of Lugonian Flysol infectant ",
        "There's a spray can of Black Flag insecticide ",
        "There's a graceless little statuette standing ",
        "There's a handful of pills and capsules "
    };

    tripped = false;
    colorEvent();
    for (ob = firstobject; ob <= gleeps; ob++)
        if (HereS(ob)) {
            if (!linefed) { linefed = true; putchar('\n'); }
            onfloor = (ob == gleeps ? gleepct[yourroom] > 0 : location[ob] == yourroom);
            if (onfloor && (env == kerosene || env == fogarc)) {
                if (!liquid(ob)) {
                    tripped = true;
//#ifdef STRICT_SINGLE_NEW_OBJECT_IT
//                    if (env != lastenv) newit = (newit == mit ? ob : nomeaning);
//#else
                    if (env != lastenv) it = ob;
//#endif
                }
            } else {
                if (ob == gleeps) {
                    Place s = Shelf(env);
                    Place s2 = s == intrunk ? incar : nowhere;
                    if (DHave(oskull)) {
                        puts("POP!  There were some gleeps in here, but they all disappeared!");
                        gleepct[yourroom] = 0;
                        gleepct[s] = 0;
                        gleepct[s2] = 0;
                    }
                    ShowGleeps(yourroom, "here");
                    if (s)
                        ShowGleeps(s, oddplaces[s - nrooms - 1]);
                    if (s2)
                        ShowGleeps(s2, oddplaces[s2 - nrooms - 1]);
                } else {
                    if (ob == oflask && !Empty(inflask))
                        put("There's a clear flask with something in it ");
                    else put(nane[ob]);
                    if (onfloor && ((ob == omatch || ob == osandwich || ob == oacetone) && inwater))
                        put("floating around here");
                    else if (ob == okeys && glued)
                        put("glued to the floor here");
                    else if (onfloor)
                        put("here");
                    else put(oddplaces[location[ob] - nrooms - 1]);
                    if (ob == onitro) puts(", labeled:\n\
     \"DANGER, EXPLOSIVE!  Contains Nitroglycerine, Handle With Care!\"");
                    else puts(".");
                    if (ob == ourine) badhygiene = true;
                }
            }
            if (!tripped)         // else "it" refers to a foot tripper if any, as that will be mentioned last
                if (ob < gleeps) {
                    if (env != lastenv || (location[ob] == incabinet && !cabinetWasOpen && cabinetopen) || (location[ob] == intrunk && !trunkWasOpen && trunkopen))
//#ifdef STRICT_SINGLE_NEW_OBJECT_IT
//                        newit = (newit == mit ? ob : nomeaning);    // opening something to reveal contents not previously seen can make it "it"
//#else
                        it = ob;        // opening something to reveal contents not previously seen makes it "it"
//#endif

                } else {
                    if (env != lastenv || (gleepct[incabinet] && !cabinetWasOpen && cabinetopen) || (gleepct[intrunk] && !trunkWasOpen && trunkopen))
//#ifdef STRICT_SINGLE_NEW_OBJECT_IT
//                        newit = (newit == mit ? ob : nomeaning);    // and gleeps can come along too
//#else
                        it = ob;        // and gleeps can come along too
//#endif
                }
        }
    if (tripped)
        puts("Your foot hits some object you can't see.");
    colorNormal();
//#ifdef STRICT_SINGLE_NEW_OBJECT_IT
//    // New rule: we set a new "it" only if exactly one item becomes newly visible.  And if more than one appears,
//    // "it" becomes unusable.  *** UNFORTUNATELY MOST VERBS ARE NOT YET PREPARED for handling it == nomeaning cases.
//    if (newit != mit) it = newit;
//#endif
}


#define doorMask (bit(bubble) | bit(balcony) | bit(meadow) | bit(arcturus) \
                        | bit(fogged) | bit(fogsplash) | bit(driverseat) | bit(ghost) \
                        | bit(fogarc) | bit(kerosene) | bit(ghostbunker))


private void ShowRoom(void)
{
    colorNormal();
    if (env == balcony && fogtime <= Timer())
        puts("You are on a balcony over a busy street.  Thick brown fog is \
pouring out of the room behind you and dissipating into the air.");
    else if (env == machine && lastenv == machine && verb != vlook)
        puts("You are in the bowels of a huge machine, etc.\n\
The Lugiman is still pouring enormous amounts of oil into the machine.");
    else if (env == arcturus) {
        PutDescription(arcturus);
        if (!beastfollowing) puts("\n\nA huge doglike alien beast is lounging by the Grugza Emperor's feet.");
        else putchar('\n');
        putsAlarm("\n\
Suddenly, the Emperor's small, feral eyes meet yours!  He bellows; his \
guards and officers leap forward, drawing their weapons!  THINGS LOOK BAD...");
    } else if (env == peekhole) {
        PutDescription(peekhole);
        if (peeked) puts("  One wall has a small hole, big enough to peek through.");
        else puts("  Strange animal sounds come through one wall.  \
In that wall is a small hole, big enough to peek through.");
    } else {
        PutDescription(env);
        if (env == vent && !vented)
            putsAlarm("\n\nYou'd better GET OUT QUICK, before they spot you!");
        else if (env == nasa && !plugged)
            puts("  It isn't plugged in.");
        else putchar('\n');
    }
    if ((env >= 32 || !(bit(env) & doorMask)) && sickness < severe) {
        int d;
        static char dc[7] = "NSEWDU";
        put("Open exits: ");
        for (d = 0; d < ndirec; d++)
            if (map[yourroom].paths[d] && !(goralive && Here(lgorilla) &&
                                (lastdirection == vjump ? d != drec(veast) :
                                 d != drec(Opposite(lastdirection))))) {
                putchar(' ');
                putchar(dc[d]);
            }
        putchar('\n');
    }
}



/* This tells the player what he's gotten himself into. */
PUBLIC void Situation(void)
{
    import bool goralive, guardalive, guardsees;
    put("\n\n\n");
    ShowRoom();
    ShowObjects();
    colorEvent();
    if (env == nasa && plugged)
        putsAlarm("\nThe centrifuge is running wildly, about to tear itself apart!");
    if (Here(lgorilla))
        if (goralive)
            puts("\nThere's a huge hungry-looking gorilla here, blocking your way!");
        else puts("\nThere's a big gorilla here.  It looks dead.");
    if (flies > 0 && yourroom != parcturus)
        printf("\nThere are %s flies buzzing annoyingly around your head.\n", format32u((int32) flies));
    colorNormal();
    if (Here(lrunt))
    {
        if (tickledLastTurn)
            putsEvent("\nThere's a small, unhealthy looking Lugiman here, a real runt, \
struggling to hold onto his cudgel.");
        else {
            putsEvent("\nThere's a small, unhealthy looking Lugiman here, a real runt, holding a \
cudgel.  He screams at you in Lugonian -- something about \"filthy Earth \
vermin\" or \"stinking Earth germs\", and something about allergies and \
something about not being able to take any more.");
            putsAlarm("\nHe's raising his club, preparing to bash your skull in!");
        }
    }
    if (Here(lguard))
        if (!guardalive)
            putsEvent("\nThere's a dead Lugonian guard here.");
        else if (tickledLastTurn) putsEvent("\nThere's a Lugonian guard here, struggling to regain control of himself.");
        else if (guardsees) putsAlarm("\nThere's a Lugonian guard here, with his weapon drawn!  GET OUT FAST!!");
        else putsEvent("\nThere's a lone Lugiman on guard here, with his back turned.");
    if (plantslunged && env == garden)
        putsAlarm("\nThe plants lunge closer!  GET OUT FAST, Joe, or they'll get you!");      
    if (yourroom == dumproom && env != balcony && env != meadow && env != streetlight)
        putsAlarm("\nYou'd better GET OUT QUICK, or the acetone fumes will knock you out!");

//printfDebug("'it' is currently %s", bestword[it]);
//if (roomit) printfDebug("... or maybe %s\n", bestword[roomit]); else printfDebug("\n");
    puts("\nWhat you wanna do, Lou?");
    if (guardsees) guardtime = Timer();
    if (plantslunged) planttime = Timer();
    if (yourroom == dumproom && dumptime == never) dumptime = Timer();
    if (lastenv != vent && env == vent && venttime == never) venttime = Timer();
}



/* this returns the weight of all the objects in place p. */

PUBLIC ushort Weight(Place p)
{
    register ushort w = 0;
    Meaning ob;
    for (ob = firstobject; ob <= lastobject; ob++)
        if (location[ob] == p) {
            w += weights[ob];
            if (container[ob])
                w += Weight(container[ob]);
        }
    if (p == inbag) return w - (w / 3);   /* the bag lightens things a bit */
    else return w;
}
