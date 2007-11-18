#5800
The robin rock~
0 g 200
~
wait 1 s
say Good day, %actor.name%. How art thou feeling today?
~
#5801
Drunkard~
0 j 100
~
if (%object.vnum% == 5810)
then
wait 1 s
say Them whipper snappers... or snapper whippers... i dunno... they's suspicious I tells ye. Like they's hidin sompin... and thank ye for the drink!
~
#5802
Drunkard Bribe~
0 g 100
~
wait 1 s
say You need help! I cin tell! Jus get me a drink n I'll give you ezactly what you need. Jus remimber, the more X's... the better!
~
#5803
Tree door~
2 g 100
~
if (%direction% == east)
then
close door
lock door
else
end
~
#5804
Bridge Operator~
0 g 100
~
wait 1 s
say I'll never open the bridge for you! Never I say!
~
#5805
Drunk X~
0 j 100
~
if (%object.vnum% == 5810)
then
wait 1 s
say I ssaid the mir X's the better! This isss no good! But I'll take it, out of the goodness of my heart.
~
#5806
Drunk XX~
0 j 100
~
if (%object.vnum% == 5809)
then
wait 1 s
say Oooh! Two! Close, but no ssigar my friend. But I will tell you this... the one you need... Ahh! It's those pink elephants again! Stay away!
sleep
~
#5807
Tree door close~
0 g 100
~
Nothing.
~
#5816
Draw it in.~
2 g 100
~
if (%direction% == north)
then
%door% 5816 north flags [c]
~
#5830
How do you like dem apple pies?~
2 d 100
apple pie~
wait 1 s
%load% obj 5802
%echo% A tiny key is launched at your head, bounces off, and hits the ground.
~
#5847
How do you like dem apples?~
2 d 0
apples~
wait 1 s
%load% obj 5800
%echo% A strange key drops from the sky.
~
$~
