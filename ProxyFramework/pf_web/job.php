#!/usr/local/bin/php
<?php    
    define("PF_DIR", "/usr/local/apache/ProxyFramework");
    define("JOB_PIDFILE", "/usr/local/apache/htdocs/proxy_framework/job.pid");
    define("X_AXIS_FORMAT", "%d.%02d.%02d");

    require_once(PF_DIR . '/DatabaseImpl.php');
    require_once(PF_DIR . '/ProxyFramework.php'); // included for defines
    require_once(PF_DIR . '/UniqueIPDatabase.php');
    require_once(PF_DIR . '/ProxyChecker.php');

    require_once('/usr/local/apache/htdocs/proxy_framework/graph.php');    
    require_once('/usr/local/apache/htdocs/proxy_framework/Stats.php');    

    $start = microtime(true);

    if (file_exists(JOB_PIDFILE)) {        
        $line = file(JOB_PIDFILE);
        $pid = trim($line[0]);
        if (count(explode("\n", shell_exec("ps --pid ".$pid))) > 2) {
            die("An instance of the job is already running with PID $pid\n");
        } else {
            // no process with that PID, can safely remove the existing PID file
            print "Found dangling PID file, removing...\n";
            unlink(JOB_PIDFILE);
        }        
    }
    

    $fp = fopen(JOB_PIDFILE, "w+");
    fwrite($fp, posix_getpid());
    fclose($fp);

    $files = scandir(PF_DIR);
    $newfiles = array();    

    $stats = new Stats();        
    $ts = $stats->getLastUpdate();   
    
    foreach ($files as $file) {
        if (strpos($file, "snapshot") !== false) {
	    // fnt is YYYY.mm.dd.HH.ii.ss
	    sscanf($file, "snapshot-%d.%d.%d.%d.%d.%d", $y, $m, $d, $h, $i, $s);
	    $time = mktime($h, $i, $s, $m, $d, $y);	    
	    if ($time > $ts || $ts == 0) {	    
                $newfiles[] = $file;
	    }
        }
    }

    print "finding files took: " . (microtime(true) - $start) . " secs\n";
    $start = microtime(true);

    // count the number of snapshots
    $c = count($newfiles);

    // get data we are interested in from each snapshot (quarantined/fast/ssl/good)
    $bad = $stats->getStats(Stats::BAD);    
    $fast = $stats->getStats(Stats::FAST);
    $ssl = $stats->getStats(Stats::SSL);
    $good = $stats->getStats(Stats::GOOD);

    //$xAxis = array();
    $files = $stats->getDates(X_AXIS_FORMAT);

    print "fetching stats took: " . (microtime(true) - $start) . " secs\n";
    $start = microtime(true);

    for ($j = 0; $j < $c; $j++) {	
        $file = PF_DIR."/".$newfiles[$j];	

	sscanf($newfiles[$j], "snapshot-%d.%d.%d.%d.%d.%d", $y, $m, $d, $h, $i, $s);	
	$date = sprintf("%d.%02d.%02d.%02d.%02d.%02d", $y, $m, $d, $h, $i, $s);
	print "date: $date\n";

        $db = new DatabaseImpl($file);

        $count = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 8, 1));
	$stats->addStats($date, Stats::BAD, $count);
	$bad[] = $count;

        $count = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_COMPARE | SEARCH_ARRAY, 4, "<20", null, null, null, 0));
	$stats->addStats($date, Stats::FAST, $count);
	$fast[] = $count;

        $count = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 5, 1, null, null, 0));
	$stats->addStats($date, Stats::SSL, $count);
	$ssl[] = $count;

        $count = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 8, 0));
	$stats->addStats($date, Stats::GOOD, $count);
	$good[] = $count;
        //$xAxis[] = $i;
	
	$files[] = sprintf(X_AXIS_FORMAT, $y, $m, $d);
    }
    print "gathering data for graph took: " . (microtime(true) - $start) . " secs\n";

    $start = microtime(true);
    drawGraph($files, array($bad, $good, $ssl, $fast));
    print "drawing graph took: " . (microtime(true) - $start) . " secs\n";

    $start = microtime(true);
    $fp = fopen('/usr/local/apache/htdocs/proxy_framework/stats', 'w+');
    flock($fp, LOCK_EX);

    $udata = new UniqueIPDatabase();
    fwrite($fp, $udata->getDatabase()->entryCount() . "\n");

    $files = scandir(PF_DIR);
    $newfiles = array();

    foreach ($files as $file) {
        if (strpos($file, "snapshot") !== false) {
            $newfiles[] = $file;
        }
    }

    $file = PF_DIR."/".$newfiles[count($newfiles) - 1];
    $db = new DatabaseImpl($file);

    $good = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 8, 0));
    fwrite($fp, "$good\n");
    $bad = $db->entryCount() - $good;
    fwrite($fp, "$bad\n");
    $fast = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_COMPARE | SEARCH_ARRAY, 4, "<20", null, null, null, 0));
    fwrite($fp, "$fast\n");
    $slow = $good - $fast;
    fwrite($fp, "$slow\n");
    $ssl = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 5, 1, null, null, 0));
    fwrite($fp, "$ssl\n");
    $http = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 7, PROXY, 0));
    fwrite($fp, "$http\n");
    $socks4 = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 7, SOCKS4, 0));
    fwrite($fp, "$socks4\n");
    $socks5 = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 7, SOCKS5, 0));
    fwrite($fp, "$socks5\n");

    function statsByCountry($db, $fp) {
        $countries = file('/usr/local/apache/htdocs/proxy_framework/countries.txt');
        $codes = file('/usr/local/apache/htdocs/proxy_framework/codes.txt');
        for ($i = 0; $i < count($countries); $i++) {
            $codes[$i] = trim($codes[$i]);
            $count = count($db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 3, $codes[$i], null, null, null, null, 0));
	    fwrite($fp, $count . ($i < count($countries) - 1 ? "\n" : ""));
        }
    }

    statsByCountry($db, $fp);
    fclose($fp);
    print "computing stats took: " . (microtime(true) - $start) . " secs\n";
    
    unlink(JOB_PIDFILE);
?>
