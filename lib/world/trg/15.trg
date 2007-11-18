#1500
GM Hangout plaque~
1 c 4
plaque~
%echo% hi
~
#1501
Forsaken~
2 d 0
Death is only the beginning~
wait 1 s
%echo% The bricks in the wall part in a cascading motion opening a secret entrance...
wait 1 s
%echo% You are swallowed up into darkness.
%teleport% all 1563
%force% %actor.name% look
~
#1502
Assult.Phobos.WestGate~
0 n 100
~
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
gossip I summon the Wizard BalDor!
~
#1503
Assassin Guild~
2 d 0
I solemnly swear I'm up to no good~
wait 1 s
%send% %actor.name% As the words escape your mouth, the sand below your feet seems to  become liquid and you fall right through.
%echoaround% %actor% %actor.name% disappears into the sands.
%teleport% %actor.name% 225
~
#1504
Strange Lot~
2 g 100
~
wait 1
%echo% The wind seems to whisper something into your ear.
~
#1507
OoD Please do not mess~
0 d 100
"The angel spreads his wings!"~
if(%speech% == "The angel spreads his wings!")
wait 1 s
say Welcome, %actor.name%
wait 1 s
%echo% The Cardinal raises his arms and a great white light engulfs the room...
%teleport% all 1564
look
~
#1541
&YWHY!&n~
0 d 0
SwifteIsTheGodofGods~
if(%speech% == "SwifteIsTheGodofGods")
wait 1 s
say Welcome, %actor.name%
wait 1 s
%echo% The Guard moves aside and you quickly walk down a darkened hallway.
%teleport% %actor% 1541
%force% all look
~
#1542
&DSS&n~
0 d 0
griffin~
if(%speech% == "griffin")
wait 1 s
say Welcome, %actor.name%
wait 1 s
%echo% The Guard moves aside and you quickly walk down a darkened hallway.
%teleport% %actor% 1542
%force% all look
~
#1550
New Player Trig~
0 h 100
~
wait 2 s
say Welcome to Deimos MUD This is the temporary Newbie start zone.
~
#1551
Runner.Go~
0 d 0
go~
if (%actor.level% < 61)
then
say You can't run the Races!
eval speech no
end
eval lap 3
if (%speech% == go)
say Lets go!
while %lap% != 0
wait %random.5% s
south
wait %random.5% s
west
wait %random.5% s
west
wait %random.5% s
west
wait %random.5% s
north
wait %random.5% s
north
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
east
wait %random.5% s
south
eval lap (%lap% - 1)
done
say Im Done!
end
~
#1552
Announcer.Winnder~
0 d 1
Im Done! go~
Nothing.
~
#1553
Bouncer~
2 g 100
~
Nothing.
~
#1555
CheckPoint.1~
0 h 100
~
%echo% CheckOne %actor.name%
mat zzzzimbo say %actor.name% has passed CheckPoint One
~
#1558
Bouncer~
0 h 100
~
if %actor.level% < 61
mteleport %actor.name% 1552
~
#1560
CheckPoint.2~
0 h 100
~
%echo% CheckTwo %actor.name%
mat zzzzimbo say %actor.name% has passed CheckPoint Two
~
#1561
Bouncer~
2 g 100
~
if %actor.sex% != NEUTRAL
then
%teleport% %actor.name% 1552
%force% %actor.name% north
end
~
#1581
Give Token~
0 f 100
~
%load% obj 1592
~
#1583
Ticket Giver~
0 f 100
~
%load% obj 1593
~
#1585
Orb Giver~
0 f 100
~
%load% obj 1594
~
#1586
Shard Load~
0 f 100
~
%load% obj 1597
~
$~
