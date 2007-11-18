#1400
Lyra~
0 d 100
Lyra where is Lyra~
if (%speech% == Lyra)
wait 1 s
say Yes, %actor.name%?
Say I am Lyra's personal secretary.  She is out of her office right now.
end
~
#1401
vending machine~
0 d 100
skittles twix doritos MnM's Sweet Tarts Lemonheads~
if (%speech% == Skittles)
unlock machine
open machine
get skittles machine
close machine
lock machine
say That will be 50 coins, please.  Maybe if you stupid kids weren't so violent, you could buy your own candy!
end
if (%speech% == Twix)
unlock machine
open machine
get twix machine
close machine
lock machine
say That will be 50 coins, please.  Maybe if you stupid kids weren't so violent, you could buy your own candy!
end
if (%speech% == Doritos)
unlock machine
open machine
get doritos machine
close machine
lock machine
say That will be 50 coins, please. Maybe if you stupid kids weren't so violent, you could buy your own candy!
End
if (%speech% == MnM's)
unlock machine
open machine
get MnM's machine
close machine
lock machine
say That will be 50 coins, please.  Maybe if you stupid kids weren't so violent, you could buy your own candy!
End
if (%speech% == Lemonheads)
unlock machine
open machine
get Lemonheads machine
close machine
lock machine
say That will be 50 coins please.  Maybe if you stupid kids weren't so violent, you could buy your own candy!
End
if (%speech% == Sweet Tarts)
unlock machine
open machine
get sweet machine
close machine
lock machine
say That will be 50 coins, please.  Maybe if you stupid kids weren't so violent, you could buy your own candy!
End
~
#1402
money~
0 m 50
~
if (%amount% == 50)
wait 1 s
say There you are, now get outta here!
give all %actor.name%
end
~
#1403
bell~
2 ab 100
~
%echo% The bell rings and scares the heck outta you!
wait %random.100% s
~
#1404
lunchlady~
0 bg 100
~
eval diceroll %random.5%
if (%diceroll% == 1%)
say Eat the damn food!  I don't CARE if you don't like it! Stop complaining!
end
if (%diceroll% == 2%)
say Watch it or i'll spit in your food!
end
if (%diceroll% == 3%)
glare %actor.name%
end
if (%diceroll% == 4%)
say Mmmm 100 percent pure RAT!
end
if (%diceroll% == 5%)
mua
end
~
#1405
lights~
2 ab 100
~
%echo% The flourescent lights flicker.
wait %random.100% s
end
~
#1406
Jocks~
0 bg 100
~
eval diceroll %random.5%
if (%diceroll% == 1%)
say What are you looking at, freak?
end
if (%diceroll% == 2%)
say Oohhh don't YOU look strong..
end
if (%diceroll% == 3%)
say Wanna take this outside?
end
if (%diceroll% == 4%)
say Oh my god, what a loser!
end
if (%diceroll% == 5%)
say Get outta here before I pound you into smithereens!
end
wait %random.100% s
~
#1407
Preppie girl~
0 abg 100
~
eval diceroll %random.5%
if (%diceroll% == 1%)
say You crack me up.
end
if (%diceroll% == 2%)
say EEeeewww!!! Cooties!!
end
if (%diceroll% == 3%)
say Don't YOU wish you were going to Harvard..
end
if (%diceroll% == 4%)
em flips her hair at you.
end
if (%diceroll% == 5%)
say Freak.. get away, you're contaminating my air!
end
~
#1408
preppie guy~
0 bg 100
~
eval diceroll %random.3%
if (%diceroll% == 1%)
say Well, hello, my special friend.
end
if (%diceroll% == 2%)
say Hey, don't you look dazzling today..wanna go out Saturday night?
end
if (%diceroll% == 3%)
smile %actor.name%
end
~
#1409
trophy case~
1 g 100
~
%echo% An alarm goes off wildly notifying everyone that %actor.name% is a THIEF!
~
#1410
lunch janitor~
0 g 100
~
say Ticket, please.
~
#1411
tray lady~
0 g 100
~
%load% obj 1413
%load% obj 1414
%load% obj 1415
put milk tray
put patty tray
give tray %actor.name%
say Have a nice day.
end
~
#1412
pay lady~
0 gm 100
~
%pyoink% tray %actor.name%
say You owe 250 coins for your lunch.
if (%amount% == 250)
give tray %actor.name% 
say Thank you, enjoy your lunch!
~
$~
