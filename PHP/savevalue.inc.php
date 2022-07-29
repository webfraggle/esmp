<?php
if (isset($_GET['val']))
{
    file_put_contents('values.txt', date("Y-m-d H:i:s")."\t".$currentnumber."\t".$_GET['val']."\n", FILE_APPEND);
}
?>