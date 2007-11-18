#1301
Green Orb Dropped~
1 h 100
~
wait 1 s
%echo% The orb opens in a bright green light!
%load% mob 1301
%purge% orb
~
#1311
Green Faerie Commands~
0 d 100
*~
%force% %speech% %forget% %actor.name%
%echo% The faerie forces %speech% to forget %actor.name%!
~
#1351
Sell Blind~
0 m 100
~
if (%amount% == 2000000)
then
%load% obj 1353
get trap
give trap %actor.name%
else
if (%amount% == 4000000)
then
say bah bah bah
~
#1352
Sell~
0 g 100
~
wait 1 s
say Hello traveler. I have some items that might be of use to you. 
wait 1 s
say I cannot tell you the price though, you must give me a certain number of coins. Try in the millions.
~
#1353
Blind Trap~
1 i 100
~
wait 1 s
%echoaround% %victim% The trap explodes in %victim.name%'s hand!
%send% %victim.name% The trap explodes and blinds you!
dg_cast 'blind' %victim.name%
%purge% %self%
~
#1354
TrapM: Blind~
0 g 100
~
wait 1 s
cast 'blind' %actor.name%
%purge% %self%
~
#1398
new trigger~
0 d 100
Treat~
Nothing.
~
#1399
Trick~
0 d 0
Trick or Treat~
Nothing.
~
$~
