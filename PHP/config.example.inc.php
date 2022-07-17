<?php
$width = 960;
$height = 540;

$startnr = 300;
$stopnr = 399;

# only for the date version
$startdate = strtotime("2022-07-03");
$stopdate = strtotime("2022-11-31 23:59:59");

$digits = 6;
$filenamepattern = "bbb_img/bbb_%0".$digits."d.png";
$conffile = "nr.serial";
?>