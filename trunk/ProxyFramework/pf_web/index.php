<?php
    $start_ts = microtime(true);

    define("PF_DIR", "/usr/local/apache/ProxyFramework");	

    require_once(PF_DIR . '/DatabaseImpl.php');
    require_once(PF_DIR . '/ProxyFramework.php'); // included for defines
    //require_once(PF_DIR . '/UniqueIPDatabase.php');
    
    include('graph.php');

    set_time_limit(0);
    
    function chkAuth($u, $p) {
        $logins = array(
		"user" => "password"
	);

	if  (!isset($logins[$u]) || $logins[$u] != $p) {
	    print '403';
	    die;
	}
    }

    function formatTime($ts) {
	$ts = trim($ts);
	if ($ts == '') {
		return '0s ago';
	}

        $secs = time() - $ts;
	if ($secs < 0) {
		return "N/A";
	}

	$days = floor($secs / 86400);
	$hours = floor(($secs - $days*86400)/3600);
	$minutes = floor(($secs - $days*86400 - $hours*3600)/60);
	$secs = $secs - $days*86400 - $hours*3600 - $minutes*60;

	return ($days != 0 ? $days."d " : "").
		($hours != 0 ? $hours."h " : "").
		($minutes != 0 ? $minutes."m " : "").
		($secs != 0 ? $secs."s " : "")."ago";
    }

    function checkIfRunning() {
        $running = false; 
	
	if (file_exists(PF_DIR . "/" . PIDFILE)) {
            $fp = fopen(PF_DIR . "/" . PIDFILE, "r");
	    flock($fp, LOCK_SH); // acquire file lock

	    $buf = "";
	    while (!feof($fp)) {
	        $buf .= fgets($fp, 128);
	    }

            $line = explode("\n", $buf);
	    $pid = trim($line[0]);

	    if (count(explode("\n", shell_exec("ps --pid ".$pid))) > 2) {
	    	fclose($fp); // release file lock
	        return $pid;
	    }	    	   
	}

	if (is_resource($fp)) {
	    fclose($fp); // release file lock	
	}

	return 0;
    }

    function start() {
    	$pid = checkIfRunning();
	if (!$pid) {	          
           $descriptorspec = array(
	       0 => array("pipe", "r"),
	       1 => array("pipe", "w"),
	       2 => array("file", "/dev/null", "a"));

           $process = proc_open("cd " . PF_DIR . " && /usr/local/bin/php launch.php", $descriptorspec, $pipes);	   
	   if (is_resource($process)) {
	       fclose($pipes[1]);
               proc_close($process);
           }	  

	   sleep(1);
	}
    }

    function stop() {
        $pid = checkIfRunning();	
	if ($pid) {
	    posix_kill($pid, 15);
	    usleep(50000);
	}

	return $pid;
    }

    function restart() {
    	$pid = stop();

	// wait for main process to shut down
	while ($pid && count(explode("\n", shell_exec("ps --pid ".$pid))) > 2) {
	    usleep(50000);
	}

	start();
    }

    // temp
    $allowed = array('192.168.0.1', '192.168.0.2');

    $continue = false;
    foreach ($allowed as $ip) {
        if (strpos($_SERVER[REMOTE_ADDR], $ip) !== false) {
	    $continue = true;
	    break;
	}
    }

    if (!$continue) {    
        if (!isset($_SERVER['PHP_AUTH_USER'])) {
            header('WWW-Authenticate: Basic realm="proxy framework"');
            header('HTTP/1.0 401 Unauthorized');
	    echo '403';
	    exit;
        } else {
          chkAuth($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
	}	
    }

    if ($_SERVER['SERVER_PORT'] != 443) {
        header('Location: https://xyz.com/proxy_framework/');
        exit;
    }    

    if ($_GET[graph] == 1) {
        drawExisting();
    }

    $files = scandir(PF_DIR);    
    $newfiles = array();

    foreach ($files as $file) {
        if (strpos($file, "snapshot") !== false) {
	    $newfiles[] = $file;
	}
    }

    function my_array_intersect()
    {
    	$args = func_get_args();	
	$i = 0;
	$newargs = array();
	foreach ($args as $arg) {
		if ($arg && count($arg) > 0) {
			eval('$res'.$i.' = $arg;');
			$newargs[] = '$res'.$i;
			$i++;
		}
	}

	if (count($newargs) == 1) {
		eval('$result = ' . $newargs[0] . ';');
		return $result;
	} else if (count($newargs) > 1) {
		$eval_str = 'array_intersect(';
		$eval_str .= implode(", ", $newargs);
		$eval_str .= ');';
		//print("EVAL STR = $eval_str");
		eval('$result = ' . $eval_str);
		return $result;
	}
    }
       
    $file = PF_DIR."/".$newfiles[count($newfiles) - 1];
    $db = new DatabaseImpl($file);
    if (/*(!isset($_GET[showBad]) && */count($_GET) > 0)/* || (isset($_GET[showBad]) && count($_GET) > 1))*/ {
    	$results = array();	
    	for ($i = 0; $i < 8; $i++) {
		if (isset($_GET["key".$i])) {
		    eval('$res'.$i.' = $db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY | SEARCH_COMPARE, $i, '.($i == 4 ? '"<=".' : '').'$_GET["key".$i]);');		   
		}
	}

	$filter = isset($res0) || isset($res1) || isset($res2) || isset($res3) || isset($res4) || isset($res5) || isset($res6) || isset($res7);
	if ($filter) {
	    eval('$results = my_array_intersect($res0, $res1, $res2, $res3, $res4, $res5, $res6, $res7);');
    	
	    if (count($results) > 0) {	        
		$db->loadEntries($proxies, $results, null, null, $_GET[sortColumn], $_GET[sortOrder]);
	    }
	}
    } 
    
    if (!$filter) {
    	$db->loadEntries($proxies, null, null, null, $_GET[sortColumn], $_GET[sortOrder]);
    }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
	<head>
		<title>proxy framework</title>
		<style type="text/css">
			td { font-family: Verdana; font-size: 10px; border: 1px #ff0000 solid; }
			.quarantined { background-color: #ff0000; } 
			.good { background-color: cyan; }
			.ssl { background-color: yellow; }
			.fast { background-color: #00ff00; }
			h1 { font-family: Verdana; font-size: 150%; }
			body { font-family: Verdana; font-size: 10px; }
			a:link { color: black; text-decoration: underline; }
			a:visited { color: black; text-decoration: underline; }
			a:active { color: darkgreen; text-decoration: underline; }
			a:hover { color: darkgreen; text-decoration: underline; }
                        .warn { background-color: yellow; }
		        .error { background-color: red; }
                        .info { background-color: #00ff00; }
		</style>
		<meta http-equiv="refresh" content="60; url=https://xyz.com/proxy_framework/"/>
	</head>

	<body>
        <h1><?php print substr($file, strrpos($file, "/") + 1); ?></h1>
<?php
    define('LOG', '/usr/local/apache/ProxyFramework/log.txt');
    define('LIMIT', 10);
    $fp = fopen(LOG, "r");
    while (!feof($fp)) {
        $buf .= fgets($fp, 128);
    }
    fclose($fp);

    preg_match_all("/\[Level: (WARN|ERROR)\]\[Time: (.*?)\]\[Message: (.*?)\]/ms", $buf, $matches);

    function display($array) {
        if (count($array) < 3) {
            return;
        }

        $array[1] = array_reverse($array[1]);
        $array[2] = array_reverse($array[2]);
        $array[3] = array_reverse($array[3]);

        $rows = 0;
        for ($i = 0; $i < count($array[1]); $i++) {
            if ($array[1][$i] != 'DEBUG') {
                $lvl = strtolower($array[1][$i]);
                print "<tr>";
                print "<td class='$lvl'>".$array[1][$i]."</td>";
                print "<td class='$lvl'>".$array[2][$i]."</td>";
                print "<td class='$lvl'>".htmlspecialchars(strlen($array[3][$i]) > 400 ? substr($array[3][$i], 0, 400) : $array[3][$i])."</td>";
                print "</tr>";
                if (++$rows >= LIMIT) {
                    break;
                }
            }
        }
    }

    function getTableColumn($name, $index) {
        return "<td>".($index == 13 ? $name  : "<a href='". (count($_GET) > 0 ? "?" : "") . $_SERVER[QUERY_STRING] . (count($_GET) > 0 ? "&" : "?") . 'sortOrder=' . ($_GET['sortColumn'] == $index && $_GET['sortOrder'] == SORT_ASC ? SORT_DESC : SORT_ASC) . "&sortColumn=".$index."'>$name</a>")."</td>";
    }

    print '<table><tr><td colspan="3"><b>Log</b></td></tr><tr><td>Level</td><td>Time</td><td>Message</td></tr>';
    display($matches);
    print "</table>";
    print '<p>&nbsp;</p>';

    if ($_GET[start] == 1) {
	start();
    } else if ($_GET[stop] == 1) {
	stop();
    } else if ($_GET[restart] == 1) {
        restart();
    }

    $running = false;
    $state = "Stopped.";
    if (file_exists(PF_DIR . "/" . PIDFILE)) {
        $line = file(PF_DIR . "/" . PIDFILE);
	list($pid, $time) = $line;
        $pid = trim($pid);
	$time = trim($time);

        if (count(explode("\n", shell_exec("ps --pid ".$pid))) > 2) {
	    $running = true;
	    $state = "Up since ".formatTime($time);
	} else {
	    $state = " Last run ".formatTime($time);
	}
    }
    
    $start = $running ? "Already running" : "<a href='?start=1'>[O]</a>";
    $stop = $running ? "<a href='?stop=1'>[O]</a>" : "Not running";
    $restart = $running ? "<a href='?restart=1'>[O]</a>" : "Not running";
	
    print '<table><tr><td colspan="2"><b>System</b></td></tr>'.
    	  '<tr><td>State</td><td>'.$state.'</td></tr>'.
    	  '<tr><td>Start</td><td>'.$start.'</td></tr>'.
	  '<tr><td>Stop</td><td>'.$stop.'</td></tr>'.
	  '<tr><td>Restart</td><td>'.$restart.'</td></tr>';
    print '</table>';

    if (file_exists('/usr/local/apache/htdocs/proxy_framework/stats')) {
        $fp = fopen('/usr/local/apache/htdocs/proxy_framework/stats', "r+");
	flock($fp, LOCK_SH);
    	$lines = file('/usr/local/apache/htdocs/proxy_framework/stats');
	fclose($fp);
    }    

    $quarantined = $lines ? (int)trim($lines[2]) : 'N/A'; 
    $fast = $lines ? (int)trim($lines[3]) : 'N/A';
    $ssl = $lines ? (int)trim($lines[5]) : 'N/A';
    $good = $lines ? (int)trim($lines[1]) : 'N/A';

    print '<p>&nbsp;</p>';
    print '<table><tr><td colspan="3"><b>Stats</b></td></tr><tr><td>Color</td><td>Significance</td><td>Count</td></tr>';
    print '<tr><td class="quarantined">&nbsp;</td><td>Quarantined (bad)</td><td>'.$quarantined.'</td></tr>';
    print '<tr><td class="fast">&nbsp;</td><td>Fast</td><td>'.$fast.'</td></tr>';
    print '<tr><td class="ssl">&nbsp;</td><td>SSL</td><td>'.$ssl.'</td></tr>';
    print '<tr><td class="good">&nbsp;</td><td>Good (working)</td><td>'.$good.'</td></tr>';
    print '</table>';

    print '<p>&nbsp;</p><b>All time unique proxies</b>: ';
    if ($lines) {
    	print (int)trim($lines[0]);
    } else {
    	print 'N/A';
    }

    //$udata = new UniqueIPDatabase();
    //print $udata->getDatabase()->entryCount();    

    print '<p>&nbsp;</p>';
    print '<b>Proxy count over time</b><br>';
    print '<img src="?graph=1">';

    print '<p>&nbsp;</p><p>&nbsp;</p>';

    print '<a href="/proxy_framework/">Back to main</a><br>';
    if (!isset($_GET[showBad])) {
    	print '<a href="?showBad=1">[+] Show quarantined proxies</a>';
    } else {
        print '<a href="/proxy_framework/">[-] Hide quarantined proxies</a>';
    }
    print '<br>';

    $types = array('Proxy', 'Socks4', 'Socks5');
    $lvls = array('Transparent', 'Anonymous', 'Highly anonymous');
    $flags = array('0' => 'N', '1' => 'Y');

    print '<table><tr>'.getTableColumn("IP", 0).getTableColumn("Port", 1).getTableColumn("Shown IP", 2).getTableColumn("Country", 3).
        getTableColumn("Response time", 4).getTableColumn("SSL", 5).getTableColumn("Lvl", 6).getTableColumn("Type", 7).getTableColumn("Last checked", 9).
	getTableColumn("Times checked", 10).getTableColumn("Times found working", 11).getTableColumn("Quarantine retries", 13).'</tr>';
    for ($i = 0; $i < count($proxies); $i++) {       
	if ($proxies[$i][8] == 0) {
	    print '<tr>';
	
	    if ($proxies[$i][4] < 20) {
	        $cls = 'fast';
	    } else if ($proxies[$i][5] == 1) {
	        $cls = 'ssl';
	    } else {
	        $cls = 'good';
	    }

	    // Fields are, in order:
	    // host, port, shownHost, country, responseTime, sslSupport, anonimityLevel, type, quarantined,
	    // lastChecked, totalChecks, totalPassed, quarantineRetries	

    	    foreach ($proxies[$i] as $key => $value) {
	    	if ($key != 8) // skip quarantined field, no need to show it
		{
		    if ($key == 4) $avgspd += $value;
	            print "<td class='$cls'>".($key < 8 ? "<a href='". (count($_GET) > 0 ? "?" : "") . $_SERVER[QUERY_STRING] . (count($_GET) > 0 ? "&" : "?") . "key$key=".$value."'>" : "").($key == 7 ? $types[$value] : ($key == 6 ? $lvls[$value] : ($key == 9 ? formatTime($value) : ($key == 5 ? $flags[$value] : htmlspecialchars($value))))).($key < 8 ? "</a>" : "")."</td>";
		}
	    }
	    print '</tr>';
	} else if ($_GET[showBad] == 1) {
            print '<tr>';
            foreach ($proxies[$i] as $key => $value) {
	        if ($key != 8) {
		    print "<td class='quarantined'>".($key < 8 ? "<a href='". (count($_GET) > 0 ? "?" : "") . $_SERVER[QUERY_STRING] . (count($_GET) > 0 ? "&" : "?") . "key$key=".$value."'>" : "").($key == 7 ? $types[$value] : ($key == 6 ? $lvls[$value] : ($key == 9 ? formatTime($value) : ($key == 5 ? $flags[$value] : htmlspecialchars($value))))).($key < 8 ? "</a>" : "")."</td>";
		}
            }
            print '</tr>';
	}
    }

    print '</table>';
    if ($good) {
        print 'Avg spd: '.($avgspd/$good).'<BR/>';
    }

    print (microtime(true) - $start_ts);
?>
	</body>
</html>	
