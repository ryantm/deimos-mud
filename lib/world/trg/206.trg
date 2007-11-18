#20600
crypt entry~
2 d 0
facilis descensus averno~
%echoaround% %actor% %actor.name% suddenly appears to melt into the floor!
%send% %actor% Everything goes black as you feel your body dissolving; then you open your eyes.
%teleport% %actor% 20624
%force% %actor% look
~
#20601
cross to elders~
0 j 100
~
if (%object.vnum% == 20610 && %self.vnum% == 20612)
wait 1 s
%echo% The Vampire Elder shudders and somehow looks more...solid?
mtransform 20613
%purge% cross
wait 1 s
scream
kill %actor.name%
end
~
#20602
prince~
0 j 100
~
if (%object.vnum% == 20608 && %self.vnum% == 20609)
wait 1 s
%echo% The Vampire Prince shudders and somehow looks more...solid?
mtransform 20610
%purge% cross
wait 1 s
scream
kill %actor.name%
end
~
#20603
prince nova~
0 an 100
An ornate golden cross gleams on the ground here.~
dg_cast 'novastorm'
~
#20680
bomb simple~
1 i 100
~
%send% %actor% You activate the timer on a bomb.
%echoaround% %actor% %actor.name% activates the timer on a bomb.
eval mytime %random.10% + 5
eval count 1
while (%count% < %mytime%)
  wait 1 s
  %echo% Beep
  eval count %count% + 1
done
wait 1 s
%echo% Beeeep...
wait 1 s
eval mytarget %self.carried_by%
if (%mytarget%)
  %send% %mytarget% &RBOOM! The bomb explodes in your face with with a deadly blast!&n
  %echoaround% %mytarget% &RBOOM! A bomb goes off in %mytarget.name%'s face!&n
  %damage% %mytarget% 10000
else
  %echo% &RBOOM! A bomb detonates violently, but nobody is close enough to be hurt!&n
end
%purge% %self%
~
#20690
resanc~
0 e 0
stares at you and utters the words, 'marsor kariq'.~
wait 1
dg_cast 'sanc' %self.name%
~
#20691
bomb give~
1 i 100
~
context %self.id%
if (%myon% != 1)
  %send% %actor% You activate the timer on a bomb.
  %echoaround% %actor% %actor.name% activates the timer on a bomb.
  eval mytime %random.10%
  global mytime
  otimer 1
  eval myon 1
  global myon
end
eval mytarget %victim%
global mytarget
~
#20692
bomb drop~
1 h 100
~
context %self.id%
eval mytarget 0
global mytarget
~
#20693
bomb get~
1 g 100
~
context %self.id%
eval mytarget %actor%
global mytarget
~
#20694
bomb timer~
1 f 100
~
context %self.id%
eval mytime %mytime% - 1
global mytime
if (%mytime% == 0)
  if %mytarget% == 0
    %echo% &R***BOOM!*** A bomb detonates violently--fortunately, nobody is close enough to be hurt!&n
  else
    %send% %mytarget% &R***BOOM!*** A bomb explodes in your face with with a deadly blast!&n
    %echoaround% %mytarget% &R***BOOM!*** A bomb goes off in %mytarget.name%'s face!&n
    %damage% %mytarget% 10000
    %purge% %self%
  end
else
  if (%mytime% == 1)
    %echo% Beeeeeep...
  else
    %echo% Beep
  end
  otimer 1
end
~
#20697
lab greet~
0 h 100
~
wait 1
tackle %actor.name%
wait 5
say PV=nRT! YOUR HEAD A SPLODE!
wait 5
cack
~
#20698
lab mua~
0 n 100
~
wait 1
say You cannot kill me!
wait 5
mua
~
#20699
lab regen~
0 f 100
~
%load% mob 20698
~
$~
