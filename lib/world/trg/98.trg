#9849
Cloak Woman Fight 01~
0 d 1
%actor.name% says, "hello"~
wait 1 s
grin
wait 1 s
say So you want to go to the other world...
wait 1 s
say Well you'll have to get past me, first.
wait 1 s
%echo% The cloaked woman grabs hold of her walking stick with both hands.
wait 3 s
kill %actor.name%
~
#9850
Cloak Woman Fight 02~
0 d 1
%actor.name% says, 'Hello.'~
wait 1 s
grin
wait 1 s
say So you want to go to the other world...
wait 1 s
say Well you'll have to get past me, first.
wait 1 s
%echo% The cloaked woman grabs hold of her walking stick with both hands.
wait 3 s
kill %actor.name%
~
#9851
Cloaked Woman Teleport~
0 l 75
~
wait 1 s
say I suppose if you can handle me, you can go to the southern one...
wait 1 s
%echo% The cloaked woman lifts her left hand which glows briefly as the world around you turns to white and then back to normal again.
%teleport% all 7042
wait 1 s
%echo% The cloaked woman drinks from a small potion bottle and turns to face the window.
mtransform -9850
~
#9870
Anti-Trig-Exp A1~
0 efgk 100
Zyre~
kill Zyre
scan
~
#9871
Anti-Trig-Exp A2~
0 efgk 100
Zyre: directly to the south~
s
kill Zyre
~
#9872
Anti-Trig-Exp A3~
0 efgk 100
Zyre: close by to the south~
s
s
kill Zyre
~
#9873
Anti-Trig-Exp A4~
0 efgk 100
Zyre: a ways to the south~
s
s
s
kill Zyre
~
#9874
Anti-Trig-Exp A5~
6 efgk 100
Zyre: far to the south~
s
s
s
s
kill Zyre
~
#9875
Anti-Trig-Exp A6~
0 efgk 100
Zyre: very far to the south~
s
s
s
s
s
kill Zyre
~
#9876
Anti-Trig-Exp A7~
0 efgk 100
Zyre: extremely far to the south~
s
s
s
s
s
s
kill Zyre
~
#9877
Anti-Trig-Exp A8~
0 efgk 100
Zyre leaves north.~
n
kill Zyre
~
#9878
Anti-Trig-Exp A9~
0 efgk 100
Zyre leaves south.~
s
kill Zyre
~
#9879
Anti-Trig-Exp A10~
0 efgk 100
Zyre leaves east.~
e
kill Zyre
~
#9880
Anti-Trig-Exp A11~
0 efgk 100
Zyre leaves west.~
w
kill Zyre
~
#9881
Anti-Trig-Exp A12~
0 efgk 100
Zyre leaves up.~
u
kill Zyre
~
#9882
Anti-Trig-Exp A13~
0 efgk 100
Zyre leaves down~
d
kill Zyre
~
#9883
Anti-Trig-Exp A14~
0 efgk 100
is here~
kill Zyre
~
#9897
Thorn Damage I~
2 g 100
~
%damage% %actor% 6
~
#9898
Thorn Damage II~
2 g 100
~
%damage% %actor.name% 6
~
$~
