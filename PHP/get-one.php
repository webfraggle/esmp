<?php
/***********************++
 * Version which gets only one picture
 * 
 */


ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

include_once 'config.inc.php';

$currentnumber = 3620;

$filename = sprintf($filenamepattern, $currentnumber);

$size = $width*$height*0.5;

$im = imagecreatefrompng($filename);

if (isset($_GET['png']))
{
    header('Content-Type: image/png');
    imagepng($im);
    exit;
}


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

imagedestroy($im);
?>