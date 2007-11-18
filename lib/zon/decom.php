#!/usr/bin/php -f
<?php

$DIR = "new";

#find stale gz's
#system("/usr/bin/find $DIR -iname *.gz > TMP");

#Open the temp file
$FP = fopen("./TMP","r");

if ( !$FP )
{ echo "Can't open temp file.\n"; die; }

while ( !feof($FP) )
{
fscanf($FP,"%s\n",$FL);

#echo "FILE: $FL\n";

## We make EVERYTHING a .gz and try to uncompress it
## all. Then go back and remove the stale .gz files

$OLDFILE = $FL;
$NEWFILE = str_replace(".gz","",$FL);

echo "RENAMING: $OLDFILE -> $NEWFILE\n";

# Make sure permissions stay intact
system("/bin/cp -fp $OLDFILE $NEWFILE");
system("/bin/rm -f $OLDFILE");
}

?>
