#2707
Give Token~
0 j 100
token strength~
wait 2 s
%load% obj 2705
%echo% The ghost flashes and a Intelligence Ticket fall's to the floor.
%echo% The Ghost emits some pulsating light and is gone.
%purge% %self%
e
n
d
~
#2708
Eureka~
0 b 100
~
Nothing.
~
#2710
sentry~
0 b 100
~
switch %self.room%
case
2721
2714
2717
2709
2729
2735
2720
2719
emote scans around looking for intruders.
wait 2 s
n
break
case
2708
2713
2716
2723
emote pulls up his trousers and gives a grunt.
wait 1 s
e
break
case
2730
2707
2711
2715
2726
emote trips on a crack in the cobblestones and falls to the ground with a crash.
wait 1 s
emote quickly gets back to his feet looking around completely embarassed
wait 1 s
w
break
case
2724
2725
2728
2734
emote shouts Intruder at the Well !
wait 1 s
emote shouts ATTACK !!!
wait 1 s
emote leaves south.
mgoto 2721
sleep
break
default
emote grunts as he is completely confused.
wait 1 s
emote leaves.
mgoto 2712
emote arrives.
break
done
~
#2711
test room~
2 g 100
~
wait %random.500% + 100 s
wecho A little man appears out of nowhere and demands that you leave his home !
wait 3 s
wecho Please leave now ! My patience is wearing thin !!
wait 2 s
wecho The little man says I will be right back and you had better be gone !
wait 2 s
wecho The little man make a gesture and is gone !
wait 25 s
%load% mob 2730
~
#2712
Theseus AI~
0 hi 100
~
mkill guard
wait 2 s
say Hello Adventurer! I doith combat with these Infidels.
~
#2713
Load Bread~
0 d 0
bread please~
say One minute while I get some fresh bread out of the oven.
wait 45 s
%load% obj 2702
give bread %actor.name%
say Bless you %actor.name%
end
~
#2720
Greet PC's~
0 d 100
kill say where who xp coin mike store~
if (%speech% == where)
then 
wait 1 s
say is there someone or something I could help you find %actor.name% ?
elseif (%speech% == kill)
then wait 1 s
say Please do not kill me I will give you all my money just don't hurt me!
wait 1 s
emote gives you 20 coins.
wait 1 s
emote runs away as fast as his legs can carry him.
%purge% %self%
elseif (%speech% == who)
then
wait 1 s
say That would be north of the Town well.
wait 1 s
say Is there anything else I can help you with ?
elseif (%speech% == xp)
then
wait 1 s
say I hear that Matt the Minister is worth tons of xp.  Besides no one likes him here anyway.
elseif (%speech% == coin)
then
wait 1 s
say Please don't take my money I have 12 children at home that would suffer greatly if you took my last two pennies.
q
~
#2730
falling~
2 g 100
~
wait 1 s
wecho testing
~
#2742
undefined~
2 d 100
blah~
Nothing.
~
#2744
Balthazar greet~
0 g 100
~
if (%actor.level% < 61)
then
wait 1
Emote leaps to his feet as you enter his room.
wait 1 s
%echo% Balthazar let's out a gut wrenching laugh.
wait 1 s
say If you kill me give this token to the ghost at the end of the next test !!
wait 1 s
say But it is more likely that you will die !!
wait 1 s
say Death to the weak !
wait 1 s
kill %actor.name%
end
else
~
#2750
undefined~
2 g 0
~
Nothing.
~
#2751
Demand Key~
0 g 100
~
if (%direction% == south)
wait 1 s
%echo% A stone Statue move out of the shadows.
wait 1 s
%echo% The Statue Demands the key from you ! Better give it quick !
end
else
~
#2752
Demand Key #2~
0 j 100
key~
if (%actor.level% > 60)
then
halt
else
wait 10 s
if (%object.vnum% == 2705)
then 
%load% obj 2706
halt
else
wait 1 s
Say Death to all the Weak and Stupid !
wait 2 s
kill %actor.name%
~
$~
