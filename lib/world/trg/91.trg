#9102
load mob~
2 d 100
assassin~
say My trigger commandlist is not complete!
~
#9103
Load~
2 d 1
mutated rat beast fuzzy orc guard gang member~
if (%speech% == mutated rat)
%load% mob 9100
%Echo% A mutated rat appears from its cage!
end
if (%speech% == beast)
%load% mob 9102
%Echo% A Beast is released from it's holding cell!
end
if (%speech% == fuzzy)
%load% mob 9101
%Echo% A Fuzzy escapes from its keepers!
end
if (%speech% == gang member)
%load% mob 9103
%Echo% A Gang Member appears from the shadows!
end
if (%speech% == orc guard)
%load% mob 9104
%Echo% An Orc Guard jumps in!
end
~
$~
