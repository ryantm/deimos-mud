#9000
Beginning Room~
0 g 100
~
stand
%load% obj 9000
get begin
mat secretary say Shut
wake
wait 1 s
say Welcome young adventurer, this is the world of Deimos, a MUD. Before you start heading out in the world, I highly suggest that you go through training.
wait 10 s
say Although, if you are skilled in many other muds, it is okay for you to go off into the world without the training involved.
wait 10 s
say To go north to train, type &Mnorth&n. Find an unlocked room. To go south to Phobos, type &Msouth&n.
wait 20 s
say Now, off with you!
say I'll give you 20 seconds to make your decision, or else i will transport you to Phobos.
wait 20 s
mat secretary say Open
%force% %actor.name% s
drop beginning
sac beginning
end
look
%purge% self
~
#9001
Which teacher?~
2 g 100
~
%purge% teacher
if (%actor.sex% == male)
%load% mob 9000
end
if (%actor.sex% == female)
%load% mob 9001
~
#9002
Hallway directions~
0 g 100
~
if (%actor.gold% == 0)
tell %actor.name% Your next training session is to the east.
end
if (%actor.gold% == 1)
tell %actor.name% Your next training session is to the west.
end
if (%actor.gold% == 2)
tell %actor.name% Your next training session is to the east.
end
if (%actor.gold% == 3)
tell %actor.name% Your next training session is to the west.
end
if (%actor.gold% == 4)
tell %actor.name% Your next training session is to the east.
end
if (%actor.gold% == 5)
tell %actor.name% Your next training session is to the west.
end
if (%actor.gold% == 6)
tell %actor.name% Your last training session is to the north.
end
~
#9003
NEXT!~
0 d 100
Open Shut~
if (%speech% == Open)
wait 1 s
say NEXT!
unlock door w
open door w
%force% %actor.next_in_room% w
end
if (%speech% == Shut)
close door w
lock door w
end
~
#9004
Create GI~
0 g 100
~
Nothing.
~
#9005
Closing the Doors~
0 d 100
Close~
if (%speech% == Close)
close door e
close door w
close door n
lock door n
lock door e
lock door w
end
~
#9008
Modool going west~
2 g 100
~
%load% mob 9009
%load% obj 9099
%force% modool get key
~
#9009
Modool Startup~
0 n 100
~
if (%self.room% == 9010)
then
wait 3 s
mat 9001 close door east
mat 9001 lock door east
end
if (%self.room% == 9020)
then
wait 3 s
mat 9001 close door west
mat 9001 lock door west
end
if (%self.room% == 9030)
then
wait 3 s
mat 9002 close door east
mat 9002 lock door east
end
if (%self.room% == 9040)
then
wait 3 s
mat 9002 close door west
mat 9002 lock door west
end
if (%self.room% == 9050)
then
wait 3 s
mat 9003 close door east
mat 9003 lock door east
end
if (%self.room% == 9060)
then
wait 3 s
mat 9003 close door west
mat 9003 lock door west
end
~
#9010
Basics~
0 d 100
Basic Basics~
if (%speech% == basics)
wait 1 s
%echo% Initiating Basics Program.
wait 5 s
emote beeps.
say Welcome to Modool's Basics Program! Here you will learn the basics about the utilities that will make your experience here easier and more fun.
wait 8 s
say First we will go over the &RAutos&n. These commands will give you more information and help in battle.
wait 7 s
say First, you'll probably want &RAutoExit&n. This command automatically displays the exits of the room when the description is shown. Very handy for finding your way around the MUD.
wait 10 s
say Here, I'll show you. Just &Ydon't do anything&n and I'll do it for you.
wait 4 s
%force% %actor.name% autoexit
wait 1 s
say There, I have made you type in &Mautoexit&n. Now, I will make you &Mlook&n which allows you to actually look at the room and see the people, objects, mobs, title of the room, and the description of the room.
wait 15 s
%force% %actor.name% look
wait 1 s
say Now you can see the room exits at the bottom of the total room description. It should look something like this: [Exits: e ]. This indicates that there is an exit going east. But don't leave yet!
wait 14 s
say The next couple of &RAutos&n we will be dealing with are for battle, so just keep them in mind for further use.
wait 8 s
say If you plan to go battling enemies along with friends, you will want to help them as they battle, right? This is where &RAutoassist&n comes in.
wait 10 s
say &RAutoassist&n will automatically help out your buddies if they start battling. But they will not help you if you start the battle. Only if they have &RAutoassist&n on will they then help you.
wait 15 s
say There are no visible changes when you activate this &RAuto&n so I'll leave it up to you to activate. Just enter &MAutoassist&n and it will activate.
wait 20 s
say After a battle there will be many spoils for the taking. To save you the trouble of typing "&Mget all corpse&n", there is an option called &RAutoloot&n.
wait 13 s
say &RAutoloot&n automatically loots the corpse for any and all items or equipment it may be carrying.
wait 10 s
say Along with items, there is also gold you can acquire from corpses. To automatically get the gold after a kill, activate &RAutogold&n.
wait 12 s
say &RAutoloot&n is the same as &RAutogold&n: it will also take gold from the corpse. However, if you wish to only recieve gold, turn off &RAutoloot&n and turn on &RAutogold&n.
wait 19 s
say If you're in a group, though, your friends may want some money, too. In this case, activate &RAutosplit&n, which will automatically split the gold evenly throughout your group.
wait 15 s
say Again, all these autos will not have a visible change, therefore I'll leave it up to your to activate it.
wait 20 s
say I think by now you have noticed a bar at the bottom after every command. There are some numbers and some letters in brackets <> or just a simple >.
wait 13 s
say This is your &RPrompt&n bar. This little wonder will display your hit points, mana points, and movement points (all of which are essential to Mudding).
wait 13 s
say There are three main ways to see the Prompt bar: There is &RPrompt none&n, which will only leave a >. There is &RPrompt all&n, which displays hit, mana, and move points (abbreviated with H/M/V).
wait 17 s
say Then there is the specific Prompt. Using &MPrompt &n[&MH&n/&MM&n/&MV&n] or any combination of the three. For example, if you put in &MPrompt HV&n, then it will display your hit and movement points. Also &MPrompt H&n = displays hit points only.
wait 20 s
say Pretty simple, huh? I think you're quick enough to try things out by yourself now...
wait 20 s
say If you have a program that displays color, you're in luck, this next section is on color. If not, take a breather and be jealous of those who do.
wait 12 s
say The &RColor&n command will change the amount of color you see as you go along in the MUD. There are four levels.
wait 11 s
say There is &RColor off&n, which will turn it off, &RSparse&n, which will only display a few, &RNormal&n, which will be a little more, and finally &RComplete&n which will dazzle your eyes with color... Try it out.
wait 25 s
say FINALLY, there is the &RSocial&n list which will show the different Emotes you can do. Just enter &MSocial&n and it will pop up. Then enter any of the words from the list to get a funny action to be preformed. You should try it.
wait 2 s
wink
wait 35 s
say Basics program complete! Say another topic to soak in more info on the world of Deimos MUD.
~
#9011
Modool Going East~
2 g 100
~
%load% mob 9010
%load% obj 9099
%force% modool get key
~
#9012
Communication~
0 d 100
communication~
if (%speech% == communication)
wait 1 s
%echo% Initiating Communication Program.
wait 5 s
emote beeps.
say Welcome to Modool's Communication Program! Here you will learn how to communicate with your friends on Deimos MUD.
wait 11 s
say First of all, there are three types of communication: &RMUD&n, &RDirect&n, and &RNormal&n.
wait 10 s
say &RMUD communication&n is the type of communication that, when said, will be heard by everyone on the MUD at that time.
wait 12 s
say Perhaps you may have heard some talk going on during our lessons such as: &C[someone] gossips, 'Hey people!'.&n These are actual people using &RMUD communication&n.
wait 14 s
say There are five different &RMUD communications&n: &RShout&n, &RHoller&n, &RGossip&n, &RGrats&n, and &RAuction&n.
wait 11 s
say The first 4 do the same thing: Proclaim what you are saying across the MUD. Only &RHoller&n is not blockable and &RShout&n has a limited range.
wait 16 s
say &RAuction&n is a bit different, not only does it proclaim what you're saying across the MUD, but it is an invitation to auction an item or piece of equipment.
wait 16 s
say For example, if you had a good piece of eq (equipment) but you then found a better one, you could enter &MAuction Really good eq: starting at 500g&n. Then other people would auction their amount of money until the final bidder was reached (you decide
wait 23 s
say Go ahead and try it: Enter &MGossip Hi people! I'm in training for MUD!&n and see what happens.
wait 26 s
say If you wish not to hear these, simply enter &MNo&n infront of the word. Such as typing: &MNogossip&n will turn your ability to hear &RGossip&n off.
wait 14 s
say The next category is &RDirect Communication&n. This is where you directly tell someone something, and no one else can hear it.
wait 13 s
say There are three different types of &RDirect Communication&n: &RAsk&n, &RTell&n, and &RWhisper&n
wait 12 s
say They are all basically the same except that when you are &RAsked&n or &RTell&ned by another person, you can simply enter &MReply [whatever]&n to reply to that person. &RWhisper&n is only available when the person is right next to you.
wait 19 s
say Finally, there is &RNormal Communication&n. There are two types: &RSay&n and &RMail&n.
wait 9 s
say &RSay&n is what usually everyone uses to talk. In fact, it's what I'm using to teach you. Simply type &MSay&n and whatever you want to say and it will say it to anyone in the room that you are in.
wait 20 s
say &RMail&n is not exactly commmunicaion, but it is a way to talk. First you must find the &BPost Office&n. There you can do three things: &RMail&n, &RCheck&n, and &RReceive&n. Only while at the &BPost Office&n can you do these actions.
wait 19 s
say If you wish to mail someone something, enter &MMail [Person's name]&n and commence writing the letter.
wait 9 s
say &RCheck&n will check to see if you have any mail. &RReceive&n will get the mail in the form of an item.
wait 20 s
say Communication program complete! Say another topic to soak in more info on the world of Deimos MUD.
~
#9013
Items~
0 d 100
Item items~
if (%speech% == items) 
then 
%echo% Initiating Items Program. 
wait 5 s 
emote beeps. 
say Welcome to the Item Familiarity Program. 
wait 6 s 
say This will teach you the basics of operating and manipulating &RItems&n. 
wait 10 s 
say &RItems&n are one of the most important factors of this game. 
wait 6 s 
say They determine how well you do in battle, how much stats you gain when you level, etc. 
wait 9 s 
say The first thing about &RItems&n you should know is how to pick them up. If you wish to do so, simply enter &MGet&n and the &RObject's name&n. For example, if there is a stick on the ground, you would type: &MGet stick&n and you will pick it up.
wait 20 s
say Once you have it, you can view the &RItem&n you picked up in your &RInventory&n by typing &Minventory&n.
wait 9 s
say If you don't want it anymore, just type &MDrop&n and the &RItem's&n name while it's in your &Rinventory&n.
wait 9 s
say You can observe it even closer by typing &MLook&n and the name of the &RItem&n. This might give you some additional info on it.
wait 11 s
say If the item looks like it is wearable, it is called &REquipment&n or &REQ&n for short. If you wish to wear it, type &Mwear&w and then the eq's name. 
wait 12 s 
say Your character will automaticly put the &REQ&n on in a slot if there is no other &REQ&n equipped there. However, you cannot if there that slot is filled and it will remain in your inventory.
wait 15 s 
say In order to see what you have equipped you can enter &MEquipment&n. This will bring up a list of all the slots you are currently using and what is there. 
wait 14 s 
say All items are designated to a certain slot. For example, a &GBracelet&n will usually only be able to be worn on the wrist. There are two &RWrist slots&n. Therefore, you can wear two pieces of &REQ&n that are made for wrists.
wait 19s
say Some &REQ&n have the ability to be worn in many places. If you wish to assign this piece to a specific slot, enter &MWear [EQ name] [Slot name]&n.
wait 15 s
say An example: &MWear cloth waist&n. This will put the &GCloth&n around your waist if &Ythere is nothing already there and it is able to be equipped there&n.
wait 15 s
say If you don't want to wear it anymore, just type &MRemove&n and the &RItem's&n name while you have it equipped.
wait 10 s
say In addition to what you can wear, there are also slots for &RWield&n and &RHold&n. The command &RWield&n is for &RWeapons&n while &RHold&n is for objects you cannot wear. For these, type &MWield&n or &MHold&n (respectively) and then the &REQ&n name.
wait 21 s 
say If you wish to give an &RItem&n to someone, just enter &MGive&n, then the &RItem's&n name, then the person's name.
wait 10 s
say Also, &MWear&n, &MDrop&n, &MRemove&n, &MGive&n and &MGet&n can be combined with &MAll&n to do that action to all that is in your &RInventory&n or on the ground at that moment.
wait 15 s
say If you wish to completely get rid of an object, you can simply enter &MJunk&n to dissolve it, &MSacrifice&n to get a coin for it and dissolve it, or &MDonate&n to send it to the &BDonation Room&n found in Phobos.
wait 18 s
say All of these commands have to have the &RItem's&n name after it and can also be affected by the &MAll&n addition.
wait 15 s
say Some &RItems&n are containers and can hold other &RItems&n which takes off the amount of &RItems&n in your &RInventory&n that limits the amount of &RItems&n you can have at a time. &RContainers&n have caps that must be opened to be used but can also 
wait 22 s
say To open them, simply enter &MOpen&n and the name of the container. Closing would use &MClose&n. To get an item out of a container, type &MGet&n, then the Item name, then the container name. Ex: &MGet stick bag&n.
wait 17 s
say To Put something in a container, enter &MPut&n then the object name, then the container. For each command, the cap &Ymust be open&n.
wait 12 s
say There is another type of &RItem&n called a &RScroll&n. These &RScrolls&n have magical properties and will do different things when you enter &MRecite&n and the scroll name and sometimes the object you want it to affect after that.
wait 18 s
say For example, the &GScroll of Identify&n will show all stats of an &RItem&n (say, a stick) when recited on it. To do this: you would type: &MRecite identify stick&n. This will then show the stats of the stick because you recited the scroll on it.
wait 20 s
say Other &RScrolls&n include the &GScroll of Recall&n and the &GScroll Written on Blue Paper&n.
wait 14 s
say Potions are much like &RScrolls&n in that they have magical properties. The difference is that they are &MQuaffed&n and they will only affect the user.
wait 13 s
say For example, a &GHealth Potion&n will recover your HP (Hit Points). To make it do this, you would enter: &MQuaff Potion&n and it would then be cast on you and your HP would have recovered.
wait 15 s
say There is one more type of &RItem&n that you should know about: That is &RFood&n and &RWater&n. Without these, You will not be able to regenerate your stats.
wait 14 s
say If you wish to eat, simply enter &MEat&n and the food's name. To drink, type &MDrink&n and the liquid container's name. Beware, though, water containers can run out of liquid and must be refilled either by spell, or by the fountain in &BPhobos&n.
wait 21 s
say To refill a container, type &MFill&n and the container's name, then the name of the liquid producer, such as a fountain.
wait 11 s
say If you are unsure about what the content of a piece of food or water is, you can &MTaste&n or &MSip&n each one to make sure it seems okay. I think you know how to do this now without me telling you.
wait 17 s
say Item Familiarity program complete! Say another topic to soak in more info on the world of Deimos MUD. 
end
~
#9014
Finished (East)~
0 d 100
finished Finished~
if (%speech% == finished) 
then 
%echo% Initiating "Free-Self" A.I.
wait 5 s 
emote beeps. 
say Finally done with the four subjects? Okay then. From now on your lessons will be less lecturing and more action. Oh boy! Subject Colors will also be turned off now.
wait 15 s 
say We will be moving from room to room soon so I'll refresh your memory on moving. There are six directions: north, south, east, west, up, and down.
wait 14 s
say You can either type the whole thing or just the first letter of the word. For example instead of entering north, you can enter n. Both will make you go north.
wait 15 s
say Doors can be opened with the command open door and then the direction. The command close door and then the direction will close it. You can also unlock and lock the doors using a similar command: Unlock (or lock) door then the direction.
wait 19 s
say This next section will tell you about the different types of environments rooms can have and how to deal with each of them.
wait 12 s
say So, lets go! Please don't do anything though, I'll put in the commands for you so we're not behind eachother.
wait 12 s
say Now we're going to go into a room to give you a feel of what its like to be controlled.
wait 8 s
%force% %actor.name% east
east
say There, I forced you to type east and you went east. Okay now lets try going down... see what happens.
wait 10 s
%force% %actor.name% down
say Notice you get the message: You need a boat to go there. This means that the room going down in a noswimming water room. You need a boat to go to these places. So now I'm gonna give you a boat.
wait 17 s
%load% obj 9012
%load% obj 9012
get boat
get boat
give boat %actor.name%
say Now you have a boat, and now we will go down again.
%force% %actor.name% down
down
say Whoo! Now we are in the water! All you need is a boat in your inventory and you can go in any water. You can also cast a spell called water breathing on yourself to swim in noswim water but that's for later on.
wait 19 s
say Now we're going straight up into the sky.
wait 6 s
%force% %actor.name% up
say As you can see, it says: You must be able to fly to enter! There is a spell called 'float' and you need it to go to these areas.
wait 12 s
say I will now cast 'float' on you, and we can go up. Becareful though, if your 'float' spell runs out, you will fall to your death and die a miserable one at that!
wait 14 s
dg_cast 'float' Modool
dg_cast 'float' %actor.name%
say Okay, up we go
%force% %actor.name% up
up
say Well there's nothing left to see here, lets go back down.
wait 5 s
%force% %actor.name% down
down
say This is a dark room. Notice it says "Someone says,". That means you can't tell who it is. Don't worry though, it's me. Here, you need a form of light such as a lantern to see here. 
wait 15 s
say Notice also that you can't see anything in your inventory... this can be a big disadvantage if your in a dark room with no light.
wait 11 s
%force% %actor.name% east
east
%load% obj 9014
get lantern
give lantern %actor.name%
say Here is a lantern. You need to hold it to get to work
%force% %actor.name% hold lantern
say Back to the darkness we go!
%force% %actor.name% west
west
wait 1 s
say As you can 'see'-
laugh
say You can see everything very clearly and see everything in your inventory.
wait 10 s
say Rooms tutorial complete. Continue to the east and say FIGHT to go into the final stage of this tutorial: Fighting.
~
#9015
Guilds~
0 d 100
Guild guilds~
if (%speech% == guilds) 
then 
%echo% Initiating Guilds Program. 
wait 5 s 
emote beeps. 
say This is the guilds portion of your training. This explains the essentials to guilds. 
wait 8 s 
say There are four guilds in this realm. They all have different &Rskills&n, &Rattributes&n, and &Rabilities&n. 
wait 9 s
say They are: &RMage&n, &RCleric&n, &RThief&n, and &RWarrior&n 
wait 4 s 
say &RMages&n are magical casters, their hit points are low but their mana is high. They excel in attack magic and harmful spells. 
wait 10 s 
say &RMages&n are slightly hard to manage, clearly the choice for a player who has suffered the rigors of MUDs before. 
wait 10 s 
say &RClerics&n are also magic casters, but their hitpoints are a little higher than &RMages&n and their mana is a little less. They are adept at healing spells and great for parties.
wait 15 s 
say &RClerics&n can heal themselves in battle and also have the powerful spell of &GSanctuary&n, which grants them double protection against damage. 
wait 13 s 
say &RThieves&n are the dark class; they have ability to use many hiding and secretive skills. Their hit points are much higher than the magic classes but they do not gain mana. 
wait 16 s 
say &RThieves&n have skills such as &GBackstab&n and &GSneak&n and generaly have to use other tactics in battle than just head on fighting. 
wait 11 s 
say &RWarriors&n are the pure fighting force guild. They excel in fighting and their hit points are the highest of any guild. 
wait 13 s 
say &RWarriors&n do not gain mana, either, but they have fighting skills. Usually a warrior just runs in to a battle headlong with out thinking, and has the courage to rescue their friends. 
wait 15 s 
say After a Player reaches the level of 30, they get a special privilege. It is called &RMulticlassing&n. 
wait 9 s 
say &RThe Guildmasters of Phobos&n allow you to level in &YONE&n extra guild so you can gain only &YSOME&n of that guild's skills. When you gain skills or spells in other guilds, they are not as strong as if that were your original guild.
wait 20 s 
say This can also be done again at level 45 and then again at level 60 
wait 10 s 
say Another thing I should tell you about is &RStat Buying&n. 
wait 10 s 
say Once you reach level 60 in your first guild, you may buy stats such as hit points and mana. 
wait 10 s 
say By paying a large sum of money to &RStan the Stat Shack Man&n, you can boost your hit points, mana and movement above their normal level.
wait 13 s 
say Each class has a different amount of stats they can buy up to before they have to stop. 
wait 12 s 
say Another thing you might want to be aware of is the &rSecret Guild&n that you can join once you reach the level of 25... 
wait 13 s 
say Hidden somewhere in the city of Phobos is a secret guild where it is possible to become a member of...
wait 10 s 
say This guild is secretive and gives certain special abilities to its members. 
wait 7 s 
say Now I will teach you about &RLeveling&n. 
wait 10 s 
say This MUD features &RIn Guild Leveling&n. This means, in order to level you must go to your guild and type &MGain Level&n. 
wait 10 s 
say You must also have the correct amount of experience, and if you are multiclassing, the correct amount of gold. 
wait 15 s 
say A map of Phobos is contained at &BCA&n or the &BCenter Agora&n (the center of &BPhobos&n), This map shows the way to each of the guilds. 
wait 10 s 
say Another thing that must be done at guilds is the &Rpracticing of skills and spells&n. 
wait 8 s 
say You don't automatically gain skills and spells, you must pick and choose which skills that would best suit you. 
wait 12 s 
say You must enter &MPractice&n at your guild to see which skills you have left unpracticed. 
wait 10 s 
say you can also practice &RWeapon Proficencies&n. These are how well you can handle certain types of weapons. 
wait 15 s 
say If you have "&GDaggers&n" praticed completely, and the weapon you are using is a &GDagger&n, you will do more damage. 
wait 15 s 
say Another important thing to know is the &RLevel&n command, when you type &MLevel&n it will display how much exp you need for each class. 
wait 12 s 
say Guilds program complete! Say another topic to soak in more info on the world of Deimos MUD. 
end 
~
#9016
Combat west~
0 d 100
Combat~
if (%speech% == Combat) 
then 
%echo% Initiating Combat Training Program
wait 5 s 
emote beeps. 
say This is the final stage of your learning, but it is also the longest. We will go over stats, battling, spells and skills, and other things that may help you on your way.
wait 16 s
say First of all, we have your statistics, or stats. To find out what your stats are, type score sheet, or just sc. This will bring up a large sheet concerning yourself.
%force% %actor.name% sc
wait 15 s
say The first part gives you your name, class, level of your primary class, and your age.
wait 8 s
say The second part will give you your HP (how much damage you can take until you die), Mana (your "energy" to cast spells), and Movement (restricts how far you can walk).
wait 15 s
say It then displays your Exp (Experience Points), which you need to level up. Kill monsters to get this, then level at your guild.
wait 11 s
say We'll go over those later. Next you have your Armor (the less you have, the less chance the enemy can hit you; goes from -100 to 100), then Alignment (1000=pure good, -1000=pure evil, 0=neutral), then your total play time, it also displays how much g
wait 21 s
say Next you have your Damroll (the more you have, the harder you hit), and your Hitroll (The more you have, the more chance for you to hit the enemy).
wait 12 s
say Next is something important to watch, you have STR (Strength, which makes you hit harder and makes you able to carry more weight), Wis (Wisdom, how many practices you will get when you level), Int (Intelligence, how much mana you will get when you le
wait 21 s
say There is also Dex (Dexterity, how much movement you get when you level), Con (Constitution, how much HP you will get when you level), and finally Cha (Charisma, lowers prices on products you buy).
wait 17 s
say The last three lines give you info on your inventory, stance, and affects (Spells on you at the moment. NOTE: not there if there are no spells on you).
wait 13 s
say Soon you will encounter your first battle, but first you need some eq. So here you go, some newbie armor, a sword, and a shield.
wait 11 s
%load% obj 9021
%load% obj 9022
%load% obj 9023
get armor
get shield
get sword
give armor %actor.name%
give sword %actor.name%
give shield %actor.name%
%force% %actor.name% wield sword
%force% %actor.name% wear armor
%force% %actor.name% rem lantern
%force% %actor.name% hold shield
wait 1 s
say Now that you have the eq, lets move east to fight your first mob!
wait 7 s
%force% %actor.name% west
west
%load% mob 9016
wait 1 s
say But before you rush into battle, you might want to see how tough this guy is in comparison to you. Type concider or just con to size yourself up to the mob.
wait 15 s
%force% %actor.name% con newbie
wait 1 s
say As you can see, it will probably say something like, 'The perfect match!'. This means you're about equal. Beware though, sometimes they can look weaker than they actually are!
wait 16 s
say So go ahead and take a whack at it! Type hit or kill to start a battle with the mob. You SHOULD win. Then say "Done" when you are finished.
end
~
#9017
Combat east~
0 d 100
Combat~
if (%speech% == Combat) 
then 
%echo% Initiating Combat Training Program
wait 5 s 
emote beeps. 
say This is the final stage of your learning, but it is also the longest. We will go over stats, battling, spells and skills, and other things that may help you on your way.
wait 16 s
say First of all, we have your statistics, or stats. To find out what your stats are, type score sheet, or just sc. This will bring up a large sheet concerning yourself.
%force% %actor.name% sc
wait 15 s
say The first part gives you your name, class, level of your primary class, and your age.
wait 8 s
say The second part will give you your HP (how much damage you can take until you die), Mana (your "energy" to cast spells), and Movement (restricts how far you can walk).
wait 15 s
say It then displays your Exp (Experience Points), which you need to level up. Kill monsters to get this, then level at your guild.
wait 11 s
say We'll go over those later. Next you have your Armor (the less you have, the less chance the enemy can hit you; goes from -100 to 100), then Alignment (1000=pure good, -1000=pure evil, 0=neutral), then your total play time, it also displays how much g
wait 21 s
say Next you have your Damroll (the more you have, the harder you hit), and your Hitroll (The more you have, the more chance for you to hit the enemy).
wait 12 s
say Next is something important to watch, you have STR (Strength, which makes you hit harder and makes you able to carry more weight), Wis (Wisdom, how many practices you will get when you level), Int (Intelligence, how much mana you will get when you le
wait 21 s
say There is also Dex (Dexterity, how much movement you get when you level), Con (Constitution, how much HP you will get when you level), and finally Cha (Charisma, lowers prices on products you buy).
wait 17 s
say The last three lines give you info on your inventory, stance, and affects (Spells on you at the moment. NOTE: not there if there are no spells on you).
wait 13 s
say Soon you will encounter your first battle, but first you need some eq. So here you go, some newbie armor, a sword, and a shield.
wait 11 s
%load% obj 9021
%load% obj 9022
%load% obj 9023
get armor
get shield
get sword
give armor %actor.name%
give sword %actor.name%
give shield %actor.name%
%force% %actor.name% wield sword
%force% %actor.name% wear armor
%force% %actor.name% rem lantern
%force% %actor.name% hold shield
wait 1 s
say Now that you have the eq, lets move east to fight your first mob!
wait 7 s
%force% %actor.name% east
east
%load% mob 9016
wait 1 s
say But before you rush into battle, you might want to see how tough this guy is in comparison to you. Type concider or just con to size yourself up to the mob.
wait 15 s
%force% %actor.name% con newbie
wait 1 s
say As you can see, it will probably say something like, 'The perfect match!'. This means you're about equal. Beware though, sometimes they can look weaker than they actually are!
wait 16 s
say So go ahead and take a whack at it! Type hit or kill to start a battle with the mob. You SHOULD win. Then say "Done" when you are finished.
end
~
#9020
Finished (West)~
0 d 100
Finished finished~
if (%speech% == finished) 
then 
%echo% Initiating "Free-Self" A.I.
wait 5 s 
emote beeps. 
say Finally done with the four subjects? Okay then. From now on your lessons will be less lecturing and more action. Oh boy! Subject Colors will also be turned off now.
wait 15 s 
say We will be moving from room to room soon so I'll refresh your memory on moving. There are six directions: north, south, west, east, up, and down.
wait 14 s
say You can either type the whole thing or just the first letter of the word. For example instead of entering north, you can enter n. Both will make you go north.
wait 15 s
say Doors can be opened with the command open door and then the direction. The command close door and then the direction will close it. You can also unlock and lock the doors using a similar command: Unlock (or lock) door then the direction.
wait 19 s
say This next section will tell you about the different types of environments rooms can have and how to deal with each of them.
wait 12 s
say So, lets go! Please don't do anything though, I'll put in the commands for you so we're not behind eachother.
wait 12 s
say Now we're going to go into a room to give you a feel of what its like to be controlled.
wait 8 s
%force% %actor.name% west
west
say There, I forced you to type west and you went west. Okay now lets try going down... see what happens.
wait 10 s
%force% %actor.name% down
say Notice you get the message: You need a boat to go there. This means that the room going down in a noswimming water room. You need a boat to go to these places. So now I'm gonna give you a boat.
wait 17 s
%load% obj 9012
%load% obj 9012
get boat
get boat
give boat %actor.name%
say Now you have a boat, and now we will go down again.
%force% %actor.name% down
down
say Whoo! Now we are in the water! All you need is a boat in your inventory and you can go in any water. You can also cast a spell called water breathing on yourself to swim in noswim water but that's for later on.
wait 19 s
say Now we're going straight up into the sky.
wait 6 s
%force% %actor.name% up
say As you can see, it says: You must be able to fly to enter! There is a spell called 'float' and you need it to go to these areas.
wait 12 s
say I will now cast 'float' on you, and we can go up. Becareful though, if your 'float' spell runs out, you will fall to your death and die a miserable one at that!
wait 14 s
dg_cast 'float' Modool
dg_cast 'float' %actor.name%
say Okay, up we go
%force% %actor.name% up
up
say Well there's nothing left to see here, lets go back down.
wait 5 s
%force% %actor.name% down
down
say This is a dark room. Notice it says "Someone says,". That means you can't tell who it is. Don't worry though, it's me. Here, you need a form of light such as a lantern to see here. 
wait 15 s
say Notice also that you can't see anything in your inventory... this can be a big disadvantage if your in a dark room with no light.
wait 11 s
%force% %actor.name% west
west
%load% obj 9014
get lantern
give lantern %actor.name%
say Here is a lantern. You need to hold it to get to work
%force% %actor.name% hold lantern
say Back to the darkness we go!
%force% %actor.name% east
east
wait 1 s
say As you can 'see'-
laugh
say You can see everything very clearly and see everything in your inventory.
wait 10 s
say Rooms tutorial complete. Continue to the west and say FIGHT to go into the final stage of this tutorial: Fighting.
q
~
#9021
Room 20~
2 g 100
~
%load% mob 9010
wait 1 s
%force% asdf say 1
~
#9022
Done (Make mob)~
0 d 100
Done done~
if (%speech% == Done)
then
wait 5 s
emote beeps.
say So you're done with the combat? Well, it seems you are ready for the real world.
wait 10 s
say I'll miss you... keep in touch with our friendly immortals and remember to save and rent often, always keep your hp and mana up. And always be open to new experiances. Remember, all this and more can be seen in the help file.
wait 20 s
say You are the weakest link. Good-bye.
mteleport %actor.name% 200
if (%self.room% == 9016)
then
wait 3 s
mat 9001 unlock door east
mat 9001 open door east
end
if (%self.room% == 9026)
then
wait 3 s
mat 9001 unlock door west
mat 9001 open door west
end
if (%self.room% == 9036)
then
wait 3 s
mat 9002 unlock door east
mat 9002 open door east
end
if (%self.room% == 9046)
then
wait 3 s
mat 9002 unlock door west
mat 9002 open door west
end
if (%self.room% == 9056)
then
wait 3 s
mat 9003 unlock door east
mat 9003 open door east
end
if (%self.room% == 9066)
then
wait 3 s
mat 9003 unlock door west
mat 9003 open door west
%purge% self
end
~
#9031
Room 30~
2 g 1
~
%load% mob 9010
wait 1 s
%force% asdf say 1
~
#9041
Room 40~
2 g 1
~
%load% mob 9010
wait 1 s
say 4
~
#9051
Room 50~
2 g 1
~
%load% mob 9010
wait 1 s
say 5
~
#9061
Room 60~
2 g 1
~
%load% mob 9010
wait 1 s
say 6
~
#9084
Start~
0 g 100
~
wait 2 s
say &RWelcome to DeimosMUD&n
wait 4 s
say I'm glad you have come here to adventure here and I hope you find this MUD suitable for your high standards.
wait 6 s
say You are standing at the beginning of The MUD School of Meat.
wait 4 s
say By typing NORTH you will enter the school and by typing SOUTH you will enter the game, with a slight detour thru the Newbie Library.
wait 6 s
say If this is your first time MUDing it would be advisable to go NORTH into the school. If it isn't it is advisable you still type INFO, so you can see the unique features of this MUD.
wait 6 s
say That is all I have to say to you! Good Luck. NORTH or SOUTH
~
#9085
mud school 1~
2 g 100
~
wait 2 s
%echo% Welcome to the DeimosMUD newbie school! Please take your time and read the room descriptions to gain a better knowledge about the mud.
~
#9089
newbie helper~
0 d 0
equip me~
wait 3
say Well, %actor.name%, let's see what you need.
wait 8
if (%actor.has(18601)% == 1)
say You already have a sword...
wait 12
else
%load% obj 18601
give sword %actor.name%
wait 12
e
n
d
if (%actor.has(18602)% == 1)
say You already have a vest...
wait 12
else
%load% obj 18602
give vest %actor.name%
wait 12
e
n
d
if (%actor.has(18603)% == 1)
say You already have a helmet...
wait 12
else
%load% obj 18603
give helm %actor.name%
wait 12
e
n
d
if (%actor.has(18612)% == 1)
say You already have some leggings...
wait 12
else
%load% obj 18612
give leggings %actor.name%
wait 12
e
n
d
if (%actor.has(18613)% == 1)
say You already have a pair of sleeves...
wait 12
else
%load% obj 18613
give sleeves %actor.name%
wait 12
e
n
d
wait 18
say I don't have anything else... Don't get killed now, %actor.name%!
~
#9090
transport~
0 d 0
playground~
wait 1
%send% %actor% Your appearance slowly fades...
%echoaround% %actor% %actor.name% slowly fades out of existence.
%teleport% %actor% 9091
%send% %actor% ...And you slowly materialize again.
%echoaround% %actor% %actor.name% materializes in front of you.
~
#9098
tEst~
0 g 100
~
say As you can see, it says: You must be able to fly to enter! There is a spell called 'float' and you need it to go to these areas.
wait 12 s
say I will now cast 'float' on you, and we can go up. Becareful though, if your 'float' spell runs out, you will fall to your death and die a miserable one at that!
wait 14 s
cast 'float' %actor.name%
cast 'float'
say Okay, up we go
%force% %actor.name% up
up
say Well there's nothing left to see here, lets go back down.
wait 5 s
say As you can see, it says: You must be able to fly to enter! There is a spell called 'float' and you need it to go to these areas.
say I push you to hell!
mforce %actor.name% east
wait 12 s
say I will now cast 'float' on you, and we can go up. Becareful though, if your 'float' spell runs out, you will fall to your death and die a miserable one at that!
wait 14 s
cast 'float' %actor.name%
cast 'float'
say Okay, up we go
%force% %actor.name% up
up
say Well there's nothing left to see here, lets go back down.
wait 5 s
trigedit 9014
~
#9099
Robot Close Doors~
0 d 100
east west~
%load% obj 9099
get key
if (%speech% == east)
close door east
lock door east
end
if (%speech% == west)
close door west
lock door west
end
~
$~
