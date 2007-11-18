#3745
kill1~
0 d 100
kill~
if (%speech% == kill)
kill %actor.name%
end
~
#3750
Damage~
2 bg 100
3~
 
%damage% actor 100
~
#3751
new trigger~
2 g 100
0~
say My trigger commandlist is not complete!
~
#3792
Purge~
0 d 1
kill *~
wait 1 s
say Yes %actor.name%, eliminating the %speech.cdr%
mgoto %speech.cdr%
wait 1 s
emote takes out a might sword and slices the %speech.cdr% in half!
%purge% %speech.cdr%
wait 1 s
mgoto %actor.name%
~
#3795
Greet~
0 g 100
~
say Hello %actor.name%, can I help you?
~
#3796
Slimer dropped~
1 h 100
~
wait 1 s
%load% mob 3764
%echo% Slimer expands to his true form.
%purge% %self%
~
#3797
Unfold Slimer~
1 g 100
~
if (%actor.name% != Kodesh)
return 0
%echo% Slimer says, '%actor.name% is not my master!'
~
#3798
speech~
0 d 100
Slimer purge weapon armor compact follow hi good bad stay destruct come I'm fine home equip~
Nothing.
if (%speech% == Slimer)
wait 1 s
%send% %actor.name% purge | weapon | armor | compact | follow | hi | good | bad | stay | destruct | equip
say Yes, %actor.name%?
end
if (%speech% == follow)
wait 1 s
say Following you, Lord.
follow Kodesh
end
if (%speech% == hi)
wait 1 s
say Hi %actor.name%, how was your day?
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
say Transforming...
%load% obj 3750
rem all
junk love
junk liquid
%echo% Slimer transforms into a tight grey box.
%purge% %self%
end
if (%speech% == stay)
wait 1 s
say Yes, %actor.name%.
follow Slimer
end
if (%speech% == destruct)
wait 1 s
wait 1 s
say Goodbye!
wait 1 s
%echo% Slimer recedes into itself for a few seconds...
wait 2 s
%echo% Slimer exlodes in a great display of fireworks!
%purge% %self%
end
if (%speech% == weapon)
wait 1 s
say Creating...
wait 1 s
%load% obj 3799
say Giving the weapon to my Lord...
give love Kodesh
force Kodesh wield love
end
if (%speech% == come)
wait 1
say I am sorry but I must go, my Lord is calling me.
wait 1 s
emote disappears in a loud bang!
mgoto Kodesh
emote appears in a big bang!
end
if (%speech% == armor)
wait 1 s
say Creating...
wait 1 s
%load% obj 898
say Giving the armor to my Lord...
give liquid Kodesh
end
if (%speech% == I'm fine)
wait 1 s
say oh, okay.
wait 1 s
smile
end
if (%speech% == home)
wait 1 s
say Yes %actor.name%, going home.
wait 1 s
emote disappears in a silver flash!
mgoto 1208
emote appears in a silver flash!
end
if (%speech% == equip)
wait 1 s
say Readying for battle!
wait 1 s
%load% obj 3799
get love
wield love
%load% obj 898
get liquid
wear liquid body
end
~
#3799
Goo Fight~
0 l 10
~
say Defense Mode Activated. Transformation beginning....
wait 1
Transformation finished
%echo% Slimer changes shape into a tight box.
rem all
junk love
junk liquid
%load% obj 3750
%purge% %self%
~
$~
