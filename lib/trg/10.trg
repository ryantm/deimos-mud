#1000
Hobbit rescue sherriff~
0 f 100
~
%load% mob 1015
%Echo% The Shiriff runs in out of nowhere in an attempt to save the hobbit.
%force% shir say Stop!  You're arrested for Assaulting Hobbits!
%force% shir kill %actor.name%
~
#1001
read me~
1 g 100
~
%echo% A small voice emanates from within the book, sounding something like 'Read me! Read me!'
~
#1008
Shhhh!~
0 d 1
*~
if (%actor.vnum% != 1000)
say Shhhhhh!
~
#1009
maggot call dogs~
0 k 100
~
eval myroom %self.room%
eval someone %myroom.people%
eval dogs 0
eval otherguy %self.fighting%
while %someone%
if %someone.vnum% == 1021
eval dogs %dogs% + 1
%force% %dogs%.dog kill %otherguy.name%
end
eval someone %someone.next_in_room%
done
if %dogs% == 0
%echo% &yFarmer Maggot shouts, 'Grip! Fang! Wolf! Come on, lads!'&n
%echo% &WThree dogs come dashing out into the lane towards you, barking fiercely!&n
%load% mob 1021
%force% dog kill %otherguy.name%
%load% mob 1021
%force% dog kill %otherguy.name%
%load% mob 1021
%force% dog kill %otherguy.name%
end
~
#1010
Keddeny speech~
0 bg 20
~
wait 1
eval index %random.4%
switch %index%
  case 1
    say Smashy smashy!
    break
  case 2
    say Double the beef!
    break
  default
    say People, relax.
    break
done
~
$~
