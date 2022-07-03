<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);

$width = 960;
$height = 540;

$startnr = 127;
$stopnr = 399;
$digits = 6;

$confstr = file_get_contents("nr.serial");
if ($confstr !== false)
{
    $currentnumber = unserialize($confstr);
    $currentnumber++;

} else {
    $currentnumber = $startnr;
}
file_put_contents("nr.serial",serialize($currentnumber));

// TODO: last image, ending picture
$filename = sprintf("bbb_img/bbb_%0".$digits."d.png", $currentnumber);
//$filename = sprintf("sw_ep3/sw3_%0".$digits."d.png", $currentnumber);

$size = $width*$height*0.5+4;

$im = imagecreatefrompng($filename);

header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");
header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename="pic.eink"');
header("Content-length: $size");




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