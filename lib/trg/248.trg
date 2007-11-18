#24800
katamari pushmove~
0 i 100
~
wait 1 s
get all
~
#24801
katamari pickup~
0 e 0
is trying to push you to the~
eval oldroom %self.room%
wait 1 s
if (%self.room% != %oldroom%)
  get all
end
~
#24802
katamari drop~
0 l 100
~
eval dropthis %self.inventory%
drop %dropthis.name%
~
#24810
Moskau~
0 e 0
looks at you.~
wait 1 s
%send% %actor% &R       |
%send% %actor% &R      /
%send% %actor% &W   ()/\\
%send% %actor% &W    /  \\
%send% %actor% &W    |-./&R'-.
%send% %actor% &R    |  '-. '.
%send% %actor% &R    /     \\_ '._
%send% %actor% &n   
%send% %actor% &CMOS
wait 4
%send% %actor% &n
%send% %actor% &n
%send% %actor% &W    ()__&R___
%send% %actor% &W    /  |
%send% %actor% &W    |-./&R'. 
%send% %actor% &R    |  \\  )
%send% %actor% &R    |  /_/_
%send% %actor% &n  
%send% %actor% &CMOS
wait 2
%send% %actor% &n
%send% %actor% &n
%send% %actor% &W      _()_
%send% %actor% &W     /\\  /\\
%send% %actor% &R    /  &W--&R  \\
%send% %actor% &R   '  /  \\  '
%send% %actor% &R      >  <
%send% %actor% &n   
%send% %actor% &CMOSKAU
wait 4
%send% %actor% &n
%send% %actor% &n
%send% %actor% &R     ___&W__()
%send% %actor% &W        |  \\
%send% %actor% &R      .'&W\\.-|
%send% %actor% &R     (  /  |
%send% %actor% &R     _\\_\\  |
%send% %actor% &n  
%send% %actor% &CMOSKAU
wait 2
%send% %actor% &R        |
%send% %actor% &R         \\
%send% %actor% &R         &W/\\()
%send% %actor% &W        /  \\
%send% %actor% &R     .-'&W\\.-|
%send% %actor% &R   .' .-'  |
%send% %actor% &R_.' _/     \\
%send% %actor% &n  
%send% %actor% &CMOSKAU MOS
wait 4
%send% %actor% &n
%send% %actor% &n
%send% %actor% &R     ___&W__()
%send% %actor% &W        |  \\
%send% %actor% &R      .'&W\\.-|
%send% %actor% &R     (  /  |
%send% %actor% &R     _\\_\\  |
%send% %actor% &n  
%send% %actor% &CMOSKAU MOS
wait 2
%send% %actor% &n
%send% %actor% &n
%send% %actor% &W      _()_
%send% %actor% &W     /\\  /\\
%send% %actor% &R    /  &W--&R  \\
%send% %actor% &R   '  /  \\  '
%send% %actor% &R      >  <
%send% %actor% &n  
%send% %actor% &CMOSKAU MOSKAU
wait 4
%send% %actor% &n
%send% %actor% &n
%send% %actor% &W    ()__&R___
%send% %actor% &W    /  |
%send% %actor% &W    |-./&R'. 
%send% %actor% &R    |  \\  )
%send% %actor% &R    |  /_/_
%send% %actor% &n   
%send% %actor% &CMOSKAU MOSKAU
~
$~
