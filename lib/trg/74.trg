#7400
SilverBOT load~
0 n 100
~
wait 2 s
%echo% The SilverBOT says 'Hello Apocalypse, what can I do for you?'
~
#7401
SilverBOT Get~
1 g 100
~
if (%actor.name% != Apocalypse)
return 0
%echo% &n&BSilver&bBOT&n says, '%actor.name%  is not my master.. I only work for Apocalypse.
~
#7402
SilverBOT speech~
0 d 100
weapon follow destruct~
if (%speech% == weapon)
wait 1 s
say Creating The Ultimate Weapon, and Putting the Ultimate Weapon in safe hands....
%load% o 7400
get s
give s apoc
end
if (%speech% == follow)
wait 1 s
say Of Course %actor.name%....
wait 1 s
follow apoc
end
if (%speech% == destruct)
wait 1 s
say Goodbye %actor.name%, it has been a pleasure serving you..
say &RSelf Destructing In 5&n
wait 1 s
say &RSelf Destructing In 4&n
wait 1 s
say &RSelf Destructing In 3&n
wait 1 s
say &RSelf Destructing In 2&n
wait 1 s
say &RSelf Destructing In 1&n
wait 1 s
%echo% KABOOM!
wait 1 s
emote materializes before your eyes!
%purge% %self%
end
~
#7409
Slave~
1 n 100
~
say My trigger commandlist is not complete!
~
$~
