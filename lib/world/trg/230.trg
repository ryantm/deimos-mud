#23000
The Northern Forest Tanner~
0 j 100
~
if (%object.vnum% != 23000)
  say What am I supposed to do with this?
  return 0
else
  wait 10
  nod
  wait 20
  say I can do this...
  switch %random.5%
    case 0
      wait 10
      say Hmm maybe not, I failed..
    break
    case 1
      %load% o 23001
    break
    case 2
      %load% o 23001
    break
    case 3
      %load% o 23002
    break
    case 4
      %load% o 23002
    break
    case 5
      %load% o 23001
    break
  done
  wait 20
  junk scales
  drop all
end
~
$~
