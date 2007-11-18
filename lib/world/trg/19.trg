#1900
Serve Customers~
0 dh 100
here go hi fry cup sticks yes no~
if (%speech% == hi)
wait 1 s
say Hi! Welcome to Arby's! Will this order be for &YHere&W or to &YGo&W?&n
end
   if (%speech% == here)
   wait 1 s
%load% obj 1926
   emote gets a tray ready for your order.
say What can i get for you? Have you read our &Ymenu?&n
   end
   if (%speech% == go)
   wait 1 s
%load% obj 1927
say What can i get for you? Have you read our &Ymenu?&n
   emote gets a bag ready for your food.
   end
if (%speech% == fry)
wait 1 s
emote gets a Large Fry from the fry station behind here.
%load% obj 1915
say Will That Be All? (&YYes&W/&YNo&W)&n
end
if (%speech% == cup)
wait 1 s
emote gets a Large Cup from the cup dispenser.
say There is a drink machine around here somewhere. You can fill this up there.
%load% obj 1907
say Will That Be All? (&YYes&W/&YNo&W)&n
end
if (%speech% == sticks)
wait 1 s
emote gets an order of mozzaarella sticks from the fry station.
%load% obj 1920
say Will That Be All? (&YYes&W/&YNo&W)&n
end
if (%speech% == yes)
wait 1 s
put all tray
put all bag
say That will be 1000 coins!
end
if (%speech% == no)
wait 1 s
say Ok Then! What else can i get for you?
end
~
#1901
Protect Jessica~
0 k 100
~
emote looks at you, FROM THE OTHER SIDE OF THE COUNTER!
~
#1902
Pay For The Food~
0 m 1000
~
if (%amount% == 1000)
wait 1 s
say Here you go! Have a nice day and enjoy your meal!
give all %actor.name%
end
~
#1903
Fake Car In The Drive-Thru~
2 ab 100
~
%echo% A Big Truck beeps loudly behind you!
wait %random.400% s
~
$~
