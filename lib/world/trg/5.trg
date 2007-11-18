#500
Titan~
0 k 100
~
dg_cast 'fireball' %actor.name%
~
#513
Kalwrathi dies~
0 f 100
~
%echo% &RKalwrathi says, 'You slaying my mortal form means nothing, I exsist as pure malice and I will be BACK'.&n
%purge% sword
~
#545
Door Test~
2 g 100
~
%door% 545 north flags b
~
#553
Down~
2 g 100
~
if (%direction% == up)
wait 1
%echo% The ground falls out from under you!
%echo% You are Falling...
%echo% Falling..
wait 1
%echo% Falling..
wait 1
%echo% Falling..
wait 1
%echo% You have landed in a Mud puddle. SPLASH!
~
$~
