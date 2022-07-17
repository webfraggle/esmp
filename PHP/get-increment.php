<?php
error_reporting(E_ERROR | E_PARSE);

include_once 'config.inc.php';

$confstr = file_get_contents($conffile);
if ($confstr !== false)
{
    $currentnumber = unserialize($confstr);
    $currentnumber++;

} else {
    $currentnumber = $startnr;
}

# restart
if ($currentnumber > $stopnr) $currentnumber = $startnr;

file_put_contents("nr.serial",serialize($currentnumber));

$filename = sprintf($filenamepattern, $currentnumber);

$size = $width*$height*0.5;

$im = imagecreatefrompng($filename);

include_once 'savevalue.inc.php';

header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");
header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename="pic.eink"');
header("Content-length: ".($size+4)); // first 4 Bytes are the stream length




// first 4 bytes are the size
print pack("N", $size);

for ($y=0; $y < $height; $y++) { 
for ($x=0; $x < $width; $x+=2) { 
        $color1 = floor(imagecolorsforindex($im, imagecolorat($im,$x,$y))['red']/16);
        $color2 = floor(imagecolorsforindex($im, imagecolorat($im,$x+1,$y))['red']/16);

        $byte = $color1;
        $byte <<= 4;
        $byte |= $color2; 
        
        // print ($color1)." ";
        // print ($color2)." ";
        // print decbin($color1)." ";
        // print decbin($color2)." ";
        // print decbin($byte)." \n";
        print pack("C", $byte);
    
    }
}
//print "!";
?>