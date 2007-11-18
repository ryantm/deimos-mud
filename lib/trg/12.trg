#1200
Assistant.Greeting~
0 g 100
~
look %actor.name%
say Hello %actor.name%! What can I do for you?
~
#1201
Assistant.KillCommand~
0 g 100
~
say Hey %actor.name%! Don't make any trouble. I don't wanna have to kick you out!
~
#1202
Prostitute~
0 m 1
~
if (%amount% < 1000)
     say I wouldn't do ANYTHING for that!
else
     say I'll do you for that.
     if (%actor.sex% == male)
          %send% %actor% The Prostitute turns into a Female.
          wait 3 s
          %send% %actor% The Prostitute begins to kiss you all over.
          wait 3 s
          %send% %actor% The Prostitute slowly unbuttons your shirt.
          wait 3 s
          %send% %actor% The Prostitute rips off her blouse and begins to take off her bra.
          wait 3 s
          %send% %actor% You kiss her all over and slide off her underwear.
          wait 3 s
          %send% %actor% The Prostitute brings you to a satisfying orgasm.
          wait 3 s
          say That's it pal.
          wait 3 s
          %send% %actor% The Prostitute packs it up.
          wait 3 s
          donate %amount% coins
     if (%actor.sex% == female)
          say Prostitute for Females will be coded in soon :)
          give %amount% coins %actor%
end
~
#1203
Sword Carrier Commands~
0 d 100
weapon armor follow hi good bad stat suicide come home~
if (%speech% == follow)
wait 1 s
say Yes I am following Mitsurugi.
follow Mitsurugi
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
if (%speech% == stay)
wait 1 s
say Yes, %actor.name%.
follow sword
end
if (%speech% == suicide)
wait 2 s
say Yes, %actor.name%, I will relieve you of my existence!
%echo% The sword carrier drives his sword through his heart!
%purge% %self%
end
if (%speech% == weapon)
wait 1 s
say Processing...
%load% obj 1901
say Here you go, %actor.name%.
give broom %actor.name%
end
if (%speech% == come)
wait 1 s
tell Mitsurugi %actor.name% has requsted my presence!
wait 1 s
emote runs off to the east!
mgoto %actor.name%
emote arrives from the west!
end
if (%speech% == home)
wait 1 s
tell Mitsurugi %actor.name% sent me home!
emote runs home!
mgoto 1223
end
if (%speech% == armor)
wait 1 s
tell Mitsurugi %actor.name% is requesting armor!
wait 1 s
say Here you go..
%load% obj 1924
give turd %actor.name%
end
~
#1204
Die Command~
0 d 1
kill *~
If (%actor.name% == mitsurugi)
say Attacking %speech.cdr%
mgoto %speech.cdr%
kill %speech.cdr%
~
#1209
SilverBOT load~
1 n 100
~
wait 2 sec
%echo% The &n&BSilver&bBOT&n folds out, and stands up straight.
%echo% The &n&BSilver&bBOT&n says 'What can i do for you master Apocalypse'
~
#1210
kiss of death~
0 k 100
~
emote kisses you on the cheek, don't blush now :)
emote rubs her body against yours, dosn't that feel nice?
~
#1214
yah~
2 g 100
~
wait 1 s
%echo% %actor.name%
%actor.blacksuit%@#@$#$#
%actor.name% + 5 = %actor.name%
q
~
#1224
Chaos~
2 b 100
~
wait %random.275%s
%echo% Chaos starts to practice his knife throwing on the back of Nightmare's chair.
~
#1225
Nightmare's Test Trigger~
0 gi 100
~
if (%speech% == 'Friend')
wait 2 s
%echo% Chaos says 'Then you may enter without fear.'
end
if (%speech% == 'Foe')
wait 2 s
%echo% Chaos says 'Then beware! For your time is limited!'
end
if (%speech% == 'I am your master')
wait 2 s
%echo% Chaos says 'If that is true, you will speak the password!'
end
if (%Speech% == 'Your soul is mine')
wait 2 s
%echo% Chaos says 'Then good day master. Nice to see you.'
wait 1 s
%echo% Chaos grins evily.
end
Wait 1 s
%echo% Chaos glares at you as you enter.
wait 2 s
%echo% Chaos says 'Friend or foe?'
wait 2 s
%echo% Chaos says 'Respond!'
end
~
#1226
Friend or Foe?~
0 d 1
Friend Foe I am your master Your soul is mine mine. master. i~
if (%speech% == friend)
wait 2 s
say Then you may enter without fear
end
if (%speech% == foe)
wait 2 s
say Then beware! For your time is limited!
end
if (%speech% == I am your master)
wait 2 s
say If that is true, you will speak the password.
end
if (%speech% == Your soul is mine)
wait 2 s
say Then good day master. Nice to see you.
wait 1 s
emote grins evilly
end
~
#1227
Treq's Office Trigs~
2 ab 100
~
%echo% The room flashes as it surges with electricity!
wait %random.400% s
~
#1297
capslock~
2 g 100
~
wait 1 s
%send% %actor% YOUR CAPS LOCK LIGHT BEGINS TO BLINK.
~
$~
