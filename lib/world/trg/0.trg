#1
memory test trigger~
0 o 100
~
* assign this to a mob, force the mob to mremember you, then enter the
* room the mob is in while visible (not via goto)
say I remember you, %actor.name%!
~
#2
mob greet test~
0 g 100
~
if %direction%
  say Hello, %actor.name%, how are things to the %direction%?
else
* if the character popped in (word of recall, etc) this will be hit
  say Where did YOU come from, %actor.name%?
end
~
#3
obj get test~
1 g 100
~
%echo% You hear, 'Please put me down, %actor.name%'
~
#4
room test~
2 g 100
~
wait 10
wsend %actor.name% you enter a room
~
#5
car/cdr test~
0 d 100
test~
say speech: %speech%
say car: %speech.car%
say cdr: %speech.cdr%
~
#6
subfield test~
0 c 100
test~
* test to make sure %actor.skill(skillname)% works
say your hide ability is %actor.skill(hide)% percent.
*
* make sure %actor.eq(name)% works too
eval headgear %actor.eq(head)%
if %headgear%
  say You have some sort of helmet on
else
  say Where's your headgear?
  halt
end
say Fix your %headgear.name%
~
#7
object otransform test~
1 jl 7
test~
* test of object transformation (and remove trigger)
* test is designed for objects 3020 and 3021
* assign the trigger then wear/remove the item
* repeatedly.
%echo% Beginning object transform.
if %self.vnum% == 3020
  otransform 3021
else
  otransform 3020
end
%echo% Transform complete.
~
#8
makeuid and remote testing~
2 c 100
test~
* makeuid test ---- assuming your MOBOBJ_ID_BASE is 200000,
* this will display the names of the first 10 mobs loaded on your MUD,
* if they are still around.
eval counter 0
while (%counter% < 10)
  makeuid mob 200000+%counter%
  %echo% #%counter%      %mob.id%   %mob.name%
  eval counter %counter% + 1
done
%echoaround% %actor% %actor.name% cannot see this line.
*
*
* this will also serve as a test of getting a remote mob's globals.
* we know that puff, when initially loaded, is id 200000. We'll use remote
* to give her a global, then %mob.globalname% to read it.
makeuid mob 200000
eval globalname 12345
remote globalname %mob.id%
%echo% %mob.name%'s "globalname" value is %mob.globalname%
~
#9
mtransform test~
0 g 100
~
* mtransform test
* as a greet trigger, entering the room will cause
* the mob this is attached to, to toggle between mob 1 and 99.
%echo% Beginning transform.
if %self.vnum%==1
  mtransform -99
else
  mtransform -1
end
%echo% Transform complete.
~
#10
attach test~
0 d 100
attach~
attach 9 %self.id%
~
#11
attach test~
0 d 100
detach~
detach 9 %self.id%
~
#12
spellcasting test~
0 c 100
kill~
* This command trigger will disallow anyone from trying to
* use the kill command, and will toss a magic missile at them
* for trying.
dg_cast 'magic missile' %actor%
return 0
~
#13
Healer.1~
0 m 1
purge~
if(%amount% == 1000)
dg_cast 'heal' %actor%
else
end
~
#14
BeamMeUp~
1 c 0
beam me up~
wait 1 s
say This Trigger Sucks!
%teleport% %actor% 200
~
#15
Lightsaber~
0 k 70
~
%send% %actor% The Jedi Knight slashes you with his deadly lightsaber.
%echoaround% %actor% %actor.name% is slashed with the Jedi Knight's deadly lightsaber.
%damage% 50
~
#16
Ferry~
2 c 0
board boat~
%teleport% %actor% 2
%send% %actor% You enter the boat.
wait 3 s
%send% %actor% The boat reaches its destination and you get off.
%teleport% all 3
end
~
#18
BetaBox~
0 d 100
Paper Pen Flag~
if (%speech% == Paper)
     say Yes %actor.name% here is some Parchment
     %load% obj 7
     give parchment %actor.name%
elseif (%speech% == Pen)
     say Yes %actor.name% here is a Pen.
      %load% obj 9
     give pen %actor.name%
elseif (%speech% == Flag)
     say Yes %actor.name% here You have been flagged.(Not Functional Yet)
     set %actor.name% Beta on
else
end
~
#19
Meta Physician~
0 d 1
Heal me~
cast 'heal' %actor.name%
~
#20
Up~
2 g 100
~
wait 2 s
%send% %actor% A giant spring launches you into the air, Where did that come from?
%force% %actor% up
end
~
#21
Blood Red Orb~
1 h 100
~
wait 1 s
%purge%
%send% %actor.name% The orb splits into in a great flash of light... You open your eyes...
%echo% %actor.name%'s blood red orb splits apart and englufs the room in a aura of light!
%purge% self
~
#22
&MPink Orb&n~
1 h 100
~
   wait 1 s
   %send% %actor.name% The orb hovers above your head deciding your destiny.
   wait 2
   switch %random.3%
   case 1   
   %send% %actor.name% Unlucky you, you got death.
   wait 1
   %damage% %actor% 10000
   %purge% self
   break
   case 2
   wait 1
   %send% %actor.name% You look like a poor fellow here have some gold.
   %load% obj 19
   %force% %actor.name% get gold
   %purge% self
   break
   default
   break
   end
~
#23
Green Orb~
1 h 100
~
wait 1 s
%send% %actor.name% The green orb splits open on the ground and showers you with greek sparks! You feel refreshed.
%echoaround% %actor.name% %actor.name%'s green orb splits open on the ground and showers of greek sparks cover %actor.name%!
%damage% %actor.name% -5000
%purge% self
~
#25
Kill Trigger~
0 d 1
kill *~
If (%actor.name% == ockham)
say Attacking %speech.cdr%
mgoto %speech.cdr%
kill %speech.cdr%
~
#27
Mood~
1 c 100
Frown Smile~
if (%command% == frown)
%echo% Lyra is sad.
if (%command% == smile)
%echo% Lyra is Happy
~
#28
Orb Commands~
1 h 100
~
wait 1 s
%teleport% %actor.name% 1214
%send% %actor.name% The orb splits into in a great flash of light... You open your eyes...
%echoaround% %actor.name% %actor.name%'s blue orb splits apart and englufs %actor.name% in a ray of light!
%teleport% self %actor.name%
%echoaround% %actor.name% %actor.name% appears suddenly in a flash of light!
%purge% self
~
#30
HunterKiller~
0 m 1
~
if (%amount% < 1000)
     say What a Cheapskate
elseif (%amount% <= %currentbribe%)
     say My current contract is for %currentbribe% You'll have to pay me more to use my services.
elseif (%amount% > %currentbribe%)
     eval currentbribe %amount%
     say Okay %actor.name%, Who do you wish for me to hunt down and kill.
     attach 31 %self%
~
#31
HunterKiller.2~
0 d 1
*~
     say Ahhh Yes %actor.name%, I will hunt %speech% for you
     mhunt %speech%
     detach mob 19 31
~
#34
make turkeys~
0 d 0
happy thanksgiving~
say Congratulations! Happy Thanksgiving to you. 
%load% obj 36
give drumstick %actor.name%
~
#35
Will.Help~
0 d 0
Commands~
switch %actor.level%
case < 61
     say Invalid Operating Level Please ask for Immortal Supervision.
     break
case > 61
      if (%actor.level% < 66)
          say Operating Code Level 3: Regular Immortal.
          wait 1
          say Commands Available:
          wait 1
          say Return           Shutdown
          wait 1
          say Activate
          end
          end
      if (%actor.name% == Ockham)
          say Operating Code Level 1: Master
          say Commands Available:
          wait 1
          say Return          Shutdown
          wait 1
          say Activate        Follow
          wait 1
          say Delete          Cast 
          wait 1          
          say Hunt            Bring
          wait 1
          say Kill            Emote
          wait 1
          say OtherCommand
          end
          end
      if (%actor.level% == 66)
          say Operating Code Level 2: Implementor
          wait 1
          say Commands Available:
          wait 1
          say Return           Shutdown
          wait 1
          say Activate         Follow
          wait 1
          say Cast
          wait 1
          end
          end
          break
default
say Invalid Operateing Level
end
~
#40
ChipTeleport~
2 b 100
~
%echo% Chip.Tick
if (%random.3% == 1)
     eval chiproom %random.99% + 800
     mat %chiproom% load obj 11
     mat %chiproom% %echo% The Golden Chip appears here suddenly.
     %echo% The Golden Chip disapears from this realm.
     %purge% chip
~
#41
GoldChip~
1 g 100
~
%send% %actor.name% You have found a Golden Chip, You have to redeam it at Town of Phobos.
eval chipgot 1
global chipgot
wait 30 s
eval chipgot 0
global chipgot
%purge% zzzzzzzkillem
~
#42
MobGoldEmblem~
0 ab 100
~
%Echo% Random Increment.
if (%random.3% == 1)
     mgoto kalkalcomo
     %send% Ockham I Found the Emblem
~
#43
Mob Chip Loader~
0 ab 100
~
%echo% Random.Tick
if (% == 1)
%echo% Entering Chip Teleport
wait 1
mgoto zzzzzzzkillem
mpurge zzzzzzzkillem
mgoto 10
wait 1
%Echo% Purge Completed
end
~
#50
Boat.Dock~
2 g 100
~
if (%time.hour% == 5 && %time.hour% == 10 && %time.hour% == 12 && %time.hour% == 2 && %time.hour% == 7)
     %send% %actor% The Santa Maria is docked here waiting for passengers.
~
#59
Weee~
2 d 100
hello~
switch %random.2%
case 1
%echo% Greetings Fool
break
case 2
%echo% Greetings Ockham
break
done
~
#60
unused~
2 g 100
~
Nothing.
~
$~
