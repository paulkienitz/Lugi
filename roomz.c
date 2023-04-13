/* ROOMZ.C
   contains the descriptions of the rooms in Lugi, and nothing else. */


#include "lugi.h"


/* how the player sees the various rooms.  These are all constants: */

private str acabinet[] = {
"You are in a dingy white cubicle, with a cabinet at the far end.",
null };

private str aniche[] = {
"You're in a bare room with an empty niche in one wall, about three feet ",
"high.  There is a faint odor of foul chemicals.",
null };

private str abunker[] = {
"You are in a bloodstained bunker.  Several skulls are scattered about.",
null };

private str abuzzcr[] = {
"You're in a humming, buzzing control room, with many switches and buttons.  ",
"The floor is a glowing latticework of concentric rings and straight bars.",
null };

private str agarden[] = {
"You're in a garden of dry brown man eating plants.  The air is dessicated.",
null };

private str acar[] = {
"You're in a grimy car showroom with huge glass windows, luridly painted, ",
"looking out onto a busy street which you recognize -- it's a good five ",
"miles from the embassy.\n\nThere is one dust-coated car here, a model ",
"at least a dozen years old.",
null };

private str adriverseat[] = {
"You are behind the wheel of the only car in a big showroom, an Audi of the decade ",
"before last.  It's quite dusty and dirty inside, and there are stains on the ",
"upholstery.  There is plenty of garbage on the floor.",
null };

private str apillpile[] = {
"You're walking over a hill of pills and capsules.  You hear the sounds of ",
"small burrowing vermin.  There is a lot of dirt thrown about.  It's dark, ",
"and the roof is too high above to see.",
null };

private str anasa[] = {
"You are in a large round chamber holding a huge NASA astronaut centrifuge.",
null };

private str abalcony[] = {
"You're on a sunny balcony over a busy street.  There's a big crowd below.",
null };

private str aarcturus[] = {
"You are inside a small cylindrical metal cage, one of many along one wall ",
"of a huge room.  The only features inside the cage are a small metal shelf ",
"and a red button.  The room is crowded with hundreds of Lugimen, most of ",
"them dressed in gaudy uniforms and heavily armed.  The heavy gravity and ",
"noisome atmosphere tell you that you are no longer on Earth, but on ",
"Arcturus IV, the home planet of the Lugimen!  At the opposite end of the ",
"room, surrounded by servile Lugimen and brutal looking guards, a monstrous ",
"bloated horror sits in a massively bejeweled chair.  You realize that the ",
"hideous being in the chair is none other than the Grugza Emperor Ra-Lugi ",
"himself, the absolute ruler of the planet, in his own throne room!",
null };

private str afogarc[] = {
"You're standing in a small metal cage in the throne room of the Grugza ",
"Emperor Ra-Lugi.  Thick brown fog makes it impossible to see a thing.  ",
"You hear screams of Lugonian rage from the Emperor's guards, who can't see ",
"to shoot you.  You can feel a small metal shelf and a button in the cage.",
null };

private str afogsplash[] = {
"You're splashing through water in opaque brown fog.  You can't see a thing.",
null };

private str afogged[] = {
"You're stumbling around in opaque brown fog.  You can't see a thing.",
null };

private str aburntplants[] = {
"You're in a hot, smoky room choked with the ashes of man-eating plants.",
null
};

private str acleanbathroom[] = {
"You're in an ornate Louis XVI type bathroom.  It is sparkling clean.",
null };

private str anasawreck[] = {
"You are standing amid the wrecked remains of a NASA astronaut centrifuge.",
null };

private str aghostbunker[] = {
"You are in a bloodstained bunker.  Several skulls are scattered about.  ",
"There is a small iron door in one corner.",
null };

private str ascaredghost[] = {
"You're in the ghost room.  Every exit is open.  There is no iron door.",
null };

private str aghost[] = {
"You are in the ghost room!  A deathly cold voice cries out,\n",
"      \"You are trapped!  Nothing you have will work here!\n",
"      You will stay here until you beg for help!\"\n",
"The only exit is a small cast iron door in one corner.",
null };

private str atank[] = {
"You're in a welded steel tank with several inches of water at the bottom.",
null };

private str acistern[] = {
"You're waist deep in a pool of clear water, in a rock-lined cistern.",
null };

// rooms only used randomly in the map begin here

private str abathroom[] = {
"You're in an ornate but filthy bathroom.  The smell is almost intolerable.  ",
"This place could sure use some disinfectant.",
null };

private str ameadow[] = {
"You are in the middle of a peaceful meadow.  God knows how you got here.",
null };

private str aseep[] = {
"You're in a long corridor with seeps of slimy water oozing down the walls.  ",
"The walls are mottled green with algae.",
null };

private str akerosene[] = {
"You're in a pitch dark closet-sized room.  You can't see what's around you.  ",
"You gag on thick kerosene vapor.",
null };

private str abubble[] = {
"You are in a transparent bubble floating high in the air.  The city is ",
"spread out beneath you, and you see a large crowd gathered in front of ",
"the Lugonian Embassy, which is directly beneath you.  For just a moment, ",
"on the balcony above the crowd, you seem to see a strange shadowy shape ",
"moving about, but then it is gone.  A sign on the wall of the bubble ",
"reads, \"No Spitting\".",
null };

private str aplainwhite[] = {
"You're in a plain white cubicle.",
null };

private str abstudy[] = {
"You are in a brown study.",
null };

private str acube[] = {
"You're cramped inside a four foot cube of dully polished iron.",
null };

private str arat[] = {
"You're in the rat room.  Millions of scrawny rats swarm over you!",
null };

private str astreetlight[] = {
"You are standing under a street light on a foggy night.  Unaccountably, ",
"you are wearing a trench coat and a snap-brim Stetson.",
null };

private str alibrary[] = {
"You're in a library, with two dusty books.",
null };

private str alaundry[] = {
"You're wading through a pile of dirty laundry, in a filthy washroom.",
null };

private str aoffice[] = {
"You're in the most cluttered office you've ever seen!  Piles of papers ",
"lie every where.  Oddly, they're all old newspapers, not forms or memos.",
null };

private str acaf[] = {
"You're walking through an empty cafeteria.  Gnawed bones, of what type ",
"you are unable to tell, lie everywhere.",
null };

private str abirdcage[] = {
"You are in a two foot bird cage, apparently made of bakelite.",
null };

private str aoperate[] = {
"You are in a hospital operating room, which looks as though it hasn't ",
"been cleaned after the last three, or perhaps thirty, uses.",
null };

private str asea[] = {
"You are looking down over a vast -- no, an endless sea of shifting gray ",
"nothingness, a nothingness so absolute that you cannot bear to look at ",
"it, for you feel that you will go mad.  In the great distance, shrouded ",
"by mist and haze, are many huge forms, like gigantic statues, with ",
"shapes that, despite the obscurity of distance and shifting haze, appear ",
"malformed and hideous.  A sign on the wall reads, \"Chew Zqrgley's Gum!\"",
null };

private str ajoke[] = {
"You are in the joke room.  There are a lot of cloth gags here, and a ",
"funny smell.",
null };

private str awumpus[] = {
"You're in a cave, and I smell a wumpus.",
null };

private str aalcove[] = {
"You're in a sunny alcove under a bright skylight.",
null };

private str acarvedfloor[] = {
"You're walking on an ornately carved floor.  There is a lot of antique ",
"furniture on the ceiling.  There are upside down paintings on the walls.",
null };

private str apicturetube[] = {
"You are in a strange room that makes your hair stand on end.  It has glass ",
"walls and a steeply sloping floor, shaped like a pyramid pointed sideways, ",
"with rounded corners.  The floor and walls converge to a narrow tube behind ",
"you, from which protrudes a complex set of metal rods and plates.  The ",
"forward wall, which forms the base of the pyramid, is transparent.  Looking ",
"out, you see an enormous suburban living room.  A magnified well-to-do ",
"family (Junior is thirty feet tall) is staring at you with bland, mindless ",
"expressions.  They make no response to any gestures or signals, though their ",
"eyes follow you.",
null };

private str acircuits[] = {
"You're in a cramped room full of exposed electronic circuitry.  The parts ",
"are unfamiliar.",
null };

private str aairlock[] = {
"You're in an airlock.  A sign says \"No Farting\".",
null };

private str alizardhole[] = {
"You're in a dirty burrow.  Dozens of lizards are busily filling out forms.",
null };

private str adevice[] = {
"You're in a perfectly sperical room with a complex device floating ",
"without visible support at the exact center.",
null };

private str atorture[] = {
"You're in a grisly chamber with whips, chains, tongs, an iron maiden with ",
"a skeleton in it, and a drainpipe in the middle of the floor.  ",
"There is a CIA badge on a table, next to a large hibachi.",
null };

private str avent[] = {
"You're in an air vent looking down over the Lugonian mess hall, where ",
"thousands of the fiends are eating a revolting lunch.",
null };

private str acentralcr[] = {
"You're in the great Central Control Room.  There are chairs, consoles ",
"with lots of controls and indicators, and a huge viewing screen.  ",
"It is deserted.",
null };

private str amachine[] = {
"You are in the bowels of a huge machine.  Gigantic levers and shafts and ",
"rotors grind madly, generating the most unbearable noise you have ever ",
"experienced.  Enormous rods and crankshafts hurl about, huge gears mesh ",
"with a deafening howl.  Great masses of metal are shrieking and roaring, as ",
"if bellowing in uncontrollable rage.  Huge chunks of steel swoop near your ",
"head and, involuntarily, you cower to the floor.  Hot oil is spurting ",
"everywhere, and as you watch, a Lugiman in greasy coveralls trundles a ",
"huge drum of the lubricant up onto a catwalk, and pours it into a hopper ",
"on the top of the machine.  Finishing the operation, he goes back down for ",
"another drum.  The walls of the room, and the flanks of the monstrous ",
"device itself, are plastered with safety warning signs.  Paths are marked ",
"out in orange paint on the floor; the nearest one is many yards away.  You ",
"suddenly realize it is broilingly hot -- the noise had wiped out all other ",
"sensation for the last minute.  If you don't get out of here soon, you ",
"may well be deafened, if you don't get caught by tons of flying metal ",
"first.  You are forced to dodge as great slabs of forged steel swing through ",
"the space you just occupied.  The noise, the heat, and the sheer terror ",
"are wearing you down fast.  You are already spattered with hot oil.  How ",
"much longer could any human being last in this environment?",
null };

private str adrydock[] = {
"You're in an indoor drydock, which holds an enormous old Chinese battleship.",
null };

private str asooty[] = {
"You're in a domed hall filled with sooty, sulphurous smoke.",
null };

private str afreebirds[] = {
"You're in a big airy chamber.  At least a dozen two foot birds wheel and ",
"soar near the ceiling, screeching happily.  They act like this freedom ",
"is a rare treat for them.",
null };

private str atar[] = {
"You're in a small room with several barrels of black tar-like stuff.  ",
"Written on each barrel in Lugonian is \"McDqrnalkt'u\".",
null };

private str aarchive[] = {
"You're in the Imperial Intelligence Central Data Archive.  All of the data ",
"is stored in a massive transparent crystal, sealed inside a vacuum chamber.",
null };

private str apeekhole[] = {
"You're in a narrow corridor with thin walls of fake wood panelling.",
/* "  Strange animal sounds come through one wall.  ",
"In that wall is a small hole, big enough to peek through.", */
null };

private str amensroom[] = {
"You're in a public men's room.  The toilets and urinals are clogged with ",
"dust.  There is something written on the wall, in Lugonian.",
null };

private str astuffed[] = {
"You are in a chamber containing...  stuffed agents.  Some have toothmarks, ",
"disguised with thick pancake makeup.  You are sickened.",
null };

private str ahackerroom[] = {
"You are in a grungy, dimly lit room.  The floor is covered with papers, ",
"clothes, fast food wrappers, and all manner of cruft.  There's a shrunken ",
"gnome here, sitting in front of a collection of strange boxes, ",
"one of which is glowing, tweaking them mysteriously.",
null };

private str arotate[] = {
"You're in a round room.  The floor is rotating, about two turns a minute.",
null };

private str abrightpoint[] = {
"You're in a round, bare room with a flat black metal floor.  A brilliant ",
"point of light, brighter than the sun, dazzles your eyes from high above, ",
"illuminating the empty area.",
null };

private str aindesc[] = {
"You're in a place that's...  well, it's indescribable.",
null };

private str arec[] = {
"You're in a vast haze-filled recreation hall.  Complex and massive ",
"devices tower in all directions with odd-looking controllers protruding ",
"from all sides, as if pinball machines were designed for intelligent ",
"octopi.",
null };

private str alargex[] = {
"You're in a small room with a big \"X\" painted on one wall.",
null };

private str alargey[] = {
"You're in a small room with a big \"Y\" painted on one wall.",
null };

private str ameat[] = {
"You're in the embassy's main meat locker.  No meat is actually here, though ",
"the place looks well used.  It's stiflingly hot, because of a smoky fire ",
"burning in a cast iron stove near one wall.",
null };

private str agreen[] = {
"You're in a green room devoid of features.",
null };

private str apentagonal[] = {
"You're in a pentagonal room sixty feet across and four feet high.",
null };

private str acolumns[] = {
"You are standing in an enormous room, hundreds of yards square, among rows ",
"of immense transparent columns filled with clear liquid.  Many bubbles, ",
"large and small, rise sluggishly through the fluid to the towering tops of ",
"the columns, which pass through the ceiling to God knows where.",
null };

private str amirrorcube[] = {
"You are inside a cube with mirrored sides.  You see thousands of reflections ",
"of yourself in every direction.",
null };

private str amtking[] = {
"You're in the Hall of the Mountain King.  Anyway, that's what it looks like.",
null };

private str awaxarmy[] = {
"You're in a wax-museum replica of a WWI army recruitment office.",
null };

private str apancreas[] = {
"You are inside a giant walk-through plastic model of a pancreas cell.",
null };

private str aglowtube[] = {
"You are inside a long, twisting tube of varying diameter.  The walls are ",
"warped and puckered grotesquely, and are made of some white waxy substance ",
"which, in places, glows brightly.",
null };

private str aconpipe[] = {
"You are inside a curving concrete pipe, eight feet in diameter.",
null };


typedef str *stray;

stray description[appearancecount] = {
    acabinet, aniche, abunker, abuzzcr, agarden, acar, adriverseat, apillpile, anasa,
    abalcony, aarcturus, afogarc, afogsplash, afogged, aburntplants, acleanbathroom,
    anasawreck, aghostbunker, ascaredghost, aghost, atank, acistern,
    abathroom, ameadow, aseep, akerosene, abubble, aplainwhite, abstudy, acube,
    arat, astreetlight, alibrary, alaundry, aoffice, acaf, abirdcage, aoperate, asea,
    ajoke, awumpus, aalcove, acarvedfloor, apicturetube, acircuits, aairlock,
    alizardhole, adevice, atorture, avent, acentralcr, amachine, adrydock,
    asooty, afreebirds, atar, aarchive, apeekhole, amensroom, astuffed,
    ahackerroom, arotate, abrightpoint, aindesc, arec, alargex, alargey, ameat,
    agreen, apentagonal, acolumns, amirrorcube, amtking, awaxarmy, apancreas,
    aglowtube, aconpipe
};



void PutDescription(Appearance e)
{
    register short i;
    stray d = description[e];
    for (i = 0; d[i]; i++)
        put(d[i]);
}
