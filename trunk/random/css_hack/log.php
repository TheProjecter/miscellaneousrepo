<?php
    define("LOG", "css.txt");
    if (preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\-(\d{7})-(\d+)/', $_SERVER['QUERY_STRING'], $matches)) {
        $fp = fopen(LOG, "a+");
        fwrite($fp, $matches[0]."\n");
        fclose($fp);
    }
?>
