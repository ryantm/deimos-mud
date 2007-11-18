#200
Bird the Bird~
0 d 1
*~
say %speech%
~
#207
Bird~
0 d 1
*~
say %speech%
~
#211
Vocab~
0 k 50
~
emote opens The Vocab Book and Sheds its unholy light upon you.
%damage% %actor% 1000
~
#219
Assassin Enter~
2 g 100
~
wait 1 s
%force% %actor.name% junk ASCII
~
#220
Jancis Door Close~
0 c 100
north~
say Hello %actor.name%!
tell Ockham bitch!
~
#250
Pimp Token (Mercenary)~
1 h 100
~
%echo% %actor% has created a Mercenary with his Pimp token.
mtransform 250
~
#280
Temple Healer~
0 m 2000
~
dg_cast 'heal' %actor.name%
~
$~
