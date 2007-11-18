#3101
Deadly Treasure~
1 g 100
~
if (%actor.level% < 61)
%echoaround% %actor.name% The wrath of GOD smites %actor.name%!
%send% %actor.name% The wrath of GOD smites you!
%damage% %actor.name% 9999
return 0
end
~
#3108
Block Exit~
3 e 100
opens~
close door
growl
~
#3110
Give gold~
0 d 0
here to help~
d
get treasure
u
say I appreciate the help you are offering.  This is for risking your life for my kingdom.
wait 5
give treasure %actor.name%
wait 10
bow
~
#3114
Evil Greeting~
0 g 100
~
wait 1
say So you wish to die to my master, Lord Grendel!
cackle
~
#3198
Block them in!~
0 h 100
~
wait 1
close rock
%send% %actor.name% You feel trapped as the large rock closes over the hole.
~
#3199
Close Door~
0 e 100
open~
close door
say You will not escape the wrath of Gandalf!
~
$~
