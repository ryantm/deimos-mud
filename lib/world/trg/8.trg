#800
Ima BT~
0 abdehn 1
*~
Nothing.
~
#801
Killer Bee :)~
0 c 100
order ultima ~
%force% %arg%
return 1
~
#803
Lyra~
0 g 100
~
say Hi, what can I do for you, %actor.name%?
~
#806
dance~
1 ab 100
~
%echo% The boom box hops as its thumpin' tunes fill the room!
wait %random.10% s
~
#810
Quest~
0 c 100
list~
if (%cmd% == list)
then
wait 1 s
say I have every item ever created on every MUD in every time frame anywhere. All the time.
wait 2 s
%force% %actor.name% say How is that possible?
wait 3 s
%echo% Ride the light: Quest.
~
#815
Bob's Trigs~
0 d 100
hi fine sad bye hungry hurt~
if (%speech% == hi)
wait 1 s
say hey, %actor.name%. How are you?
%send% %actor% (Bob prefers either fine, hungry, or sad)
elseif (%speech% == fine)
wait 1 s
say Hey cool! Same here.
elseif (%speech% == sad)
wait 1 s
frown
wait 1 s
say awww... I'm sorry, here, take this.
%load% obj 899
give can %actor.name%
say It may cheer you up!
wait 1 s
smile
elseif (%speech% == bye)
wait 1 s
say bye!
wave %actor.name%
elseif (%speech% == hungry)
wait 1 s
say Oh you are? Here, take this
wait 1 s
%load% obj 2504
give slice %actor.name%
wait 1 s
smile
elseif (%speech% == hurt)
wait 1 s
cast 'heal' %actor%
else
end
~
#816
stuff~
0 n 100
~
say Please state the nature of your medical emergency.
~
#825
new trigger~
2 cg 100
~
say My trigger commandlist is not complete!
~
#844
Controller Answers.~
0 d 100
*~
if (%speech% == gtormf)
wait 1 s
say Svlmpearfhrf!
%echo% (Acknowledged)
else
wait 1 s
kill %actor.name%
say For, dviz!
%echo% (Die, scum!)
end
~
#845
Controller Trigs~
0 g 100
~
wait 1 s
say JSAY! Ejp sty yjpi?
%echo% (HALT! Who art thou?)
~
#846
Slave Trigs~
0 d 100
Mpe Ftoml Gppf~
if (%speech% == Mpe )
%load% obj 7
%load% obj 9
wait 1 s
emote scrambles to help the Almighty Controller 
wait 1 s
give parch Controller
give pen Controller
end
if (%speech% == Ftoml )
%load% obj 2110
wait 1 s
emote scrambles to help the Almighty Controller 
wait 1 s
give water Controller
end
if (%speech% == Gppf )
%load% obj 12
wait 1 s
emote scrambles to help the Almighty Controller 
wait 1 s
give food Controller
end
~
#847
Controller Rand~
0 ab 10
~
eval rand %Random.3%
if (%rand% == 1)
say Drtbrmyd 
%echo% (Servents )
wait 1 s
say Gppf 
%echo% (Food )
wait 10 + %random.100% s
junk food
end
if (%rand% == 2)
say Drtbrmyd 
%echo% (Servents )
wait 1 s
say Ftoml 
%echo% (Water )
wait 10 + %random.100% s
junk water
end
if (%rand% == 3)
say Drtbrmyd 
%echo% (Servents )
wait 1 s
say Qrm smf qsqrt 
%echo% (Pen and paper )
say Mpe 
%echo% (Now )
wait 10 s
wait 10 + %random.100% s
junk pen
junk paper
end
~
#850
Loader~
0 d 100
*~
%load% %speech%
drop all
~
#867
Treq's Trigs~
0 g 100
~
say Stop annoying me.
~
#878
Alpha Blade~
1 c 100
~
say My trigger commandlist is not complete!
~
#885
Force Down~
2 g 100
~
%force% %actor.name% d
~
#886
RandomLoad~
2 f 100
~
%load% mob %random.3200%
~
#887
Mine~
1 h 100
~
%echo% The timed mine blinks green!
wait 1 s
%echo% The timed mine blinks green!
wait 1 s
%echo% The timed mine blinks green!
wait 1 s
%echo% The timed mine blinks green!
wait 1 s
%echo% The timed mine blinks red!
wait 1 s
%echo% The timed mine explodes into a great ball of fire!
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%damage% %actor.next_in_room% 5000
%echo%
%purge% self
~
#888
Pimp Police~
0 h 100
~
If (%actor.level% < 61)
then
wait 4 s
say This is the Pimp Police!
wait 2 s
say Come out with your pants down!
wait 1 s
say If you do not come out with your pants down, we will be %force%d to come in there and take them off for you!
wait 5 s
%echo% The Pimp Police approches you... You're screwed now!
%force% %actor.name% rem all
%force% %actor.name% drop all
get all
laugh
mgoto 1227
drop all
mgoto %actor.name%
flee
flee
flee
flee
flee
else
end
~
#889
Groan and Moan~
0 ab 100
~
moan
wait 20 s
wait 13 s
groan
wait 33 s
~
#891
Follow Treq~
0 ab 100
~
Nothing.
~
#892
purge~
0 d 1
kill *~
wait 1 s
say Yes, %actor.name%, eliminating the %speech.cdr%.
mgoto %speech.cdr%
wait 1 s
emote takes out a laser and disintegrates the %speech.cdr%!
%purge% %speech.cdr%
wait 1 s
mgoto %actor.name%
~
#893
Load~
0 n 100
~
wait 1 s
say Please state the nature of your MUD emergency.
~
#894
Director Grumble~
0 ab 100
~
emote grumbles about his job.
wait 13 s
wait 20 s
~
#895
Greet~
0 g 100
~
say Hello, %actor.name%, how may I help you?
~
#896
SBC Dropped~
1 h 100
~
wait 1 s
%load% mob 899
%echo% StrikerBot folds out into its full form.
%purge% %self%
~
#897
SBC Trigs~
1 g 100
~
if (%actor.name% != Firrox)
return 0
%echo% StrikerBot says, '%actor.name% is not my Master!'
 
~
#898
UC command~
0 d 100
StrikerBot purge weapon armor compact follow hi good bad stay destruct come I'm fine home equip~
 
if (%speech% == StrikerBot)
wait 1 s
%send% %actor.name% purge | weapon | armor | compact | follow | hi | good | bad | stay | destruct | equip
say Yes, %actor.name%?
end
if (%speech% == follow)
wait 1 s
say Following you, master.
follow Firrox
end
if (%speech% == hi)
wait 1 s
say Hi, %actor.name%, how was your day?
eval ack %ack% + 1
end
if (%speech% == good)
 
wait 1
say I'm glad to hear it.
 
end
 
if (%speech% == bad)
wait 1 s
say Oh, I'm sorry
 
end
if (%speech% == compact)
wait 1 s
say Compacting...
%load% obj 895
rem all
junk blade
junk liquid
%echo% StrikerBot folds up into a neat little half-football.
%purge% %self%
end
if (%speech% == stay)
wait 1 s
say Yes, %actor.name%.
follow StrikerBot
end
if (%speech% == destruct)
wait 1 s
say Yes, %actor.name%, I shall self destruct in
say 5
wait 1 s
say 4
wait 1 s
say 3
wait 1 s
say 2
wait 1 s
say 1
wait 1 s
say Bye!
wait 1 s
%echo% StrikerBot explodes in a dazzling display of shrapnel and sparks!
%purge% %self%
end
if (%speech% == weapon)
wait 1 s
say Processing...
wait 1 s
%load% obj 899
say Giving the weapon into proper hands...
give alpha Firrox
force Firrox wield alpha
end
if (%speech% == come)
wait 1 s
say I am sorry, but I must go, my Master is calling for me.
wait 1 s
emote dissolves before your eyes!
mgoto Firrox
emote materializes before your eyes!
end
if (%speech% == armor)
wait 1 s
say Processing...
wait 1 s
%load% obj 898
say Giving the armor into proper hands...
give liquid Firrox
end
if (%speech% == I'm fine)
wait 1 s
say Oh, okay.
wait 1 s
smile
end
if (%speech% == home)
wait 1 s
say Yes, %actor.name%, going home.
wait 1 s
emote dissolves before your eyes!
mgoto 1227
emote materializes before your eyes!
end
if (%speech% == equip)
wait 1 s
say Equipping offensive weaponry!
wait 1 s
%load% obj 899
get alpha
wield alpha
%load% obj 898
get liquid
wear liquid body
end
~
#899
SB Fighting~
0 l 10
~
say Damage overload. Initiating defensive mode.
%load% obj 895
%echo% StrikerBot folds up into a neat little half-football.
rem all
junk blade
junk liquid
%purge% %self%
~
$~
