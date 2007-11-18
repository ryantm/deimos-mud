#7000
Test 001~
2 b 100
has arrived.~
%asound% You hear the sound of a piece of POO!!!
wait 2 s
~
#7011
Rosco 01~
0 gi 100
has arrived.~
wait 1
say Hi. I'm Rosco. Let's bowl!
~
#7012
Rosco 02~
0 k 100
miss~
wait 1
say Oh dear. Miss!
~
#7013
Test 003~
0 abcdefghijklmno 100
~
#100
gateguard greet 
g 100
 
if (%direction% == north)
wait 1
emote snaps to attention as you approach.
wait 1
say You may not pass this point. Orders of His Majisty King Zyrik.
end 
#101
gateguard pay 10 
m 10
 
wait 1
unlock gate
open gate
wait 10
close gate
lock gate
#102
gateguard pay 1 
m 1
 
wait 1
say Bribery wont work on me you fool!
give %amount% coins %actor% 
#103
gateguard enter city 
e 0
leave south. 
wait 1
close gate
lock gate 
#104
gateguard gate opened 
e 0
The gate is opened from the other side 
wait 5
close gate
lock gate 
#99999
$ 
~
#7014
Zyrian Knight~
0 g 100
A pokemon is standing here.~
wait 1
kill pokemon
look
~
#7015
Zyrian Knight~
0 g 100
has arrived~
wait 1
kill pokemon
look
~
#7016
Zyrian Knight~
0 g 100
pokemon is DEAD~
wait 1
kill pokemon
~
#7085
Ewok01~
0 g 100
has arrived.~
wait a s
say Yi'chup nahgoo!
wait 2 s
emote does a happy dance!
~
#7086
Ewok02~
0 g 100
has arrived.~
wait 2 s
say Yub yub!
wait 2 s
emote dances happily! He's so happy! Yay!
~
#7088
Awaken~
0 c 100
%actor.name% lies down and falls asleep.~
wait 1
%force% %actor.name% wake
~
#7089
Zyre Morph 1~
0 l 75
None.~
%echo% Zyre suddenly grabs his head in pain and is surrounded by darkness.
wait 1
%echo% Zyre suddenly transforms into Metazyre!
mtransform -7089
~
#7090
Zyre Morph 2~
0 l 60
~
%echo% Metazyre shouts in pain as the darkness around him grows.
wait 1
%echo% Metazyre has transformed into Metazutos!
mtransform -7090
~
#7091
Zyre Morph 3~
0 l 50
~
%echo% Metazutos clenches his fists in anger as the darkness around him fuses with his body.
wait 1
%echo% Metazutos has transformed into Darzutos!
mtransform -7091
~
#7092
Zyre's Restoration~
0 l 4
~
wait 1
%echo% &DZyre&n glares at you and grins.
wait 1
%echo% &DZyre&n is surrounded by darkness, which then fuses with his body!&n
wait 1
mtransform -7088
%echo% &GZyre has been fully restored!&n
~
#7099
Arenburg Recall~
0 g 100
Nothing.~
%teleport% %actor.name% 8500
~
$~
