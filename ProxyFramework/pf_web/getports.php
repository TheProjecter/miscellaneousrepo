<?php
	require_once("/usr/local/apache/ProxyFramework/DatabaseImpl.php");
	$files = scandir("/usr/local/apache/ProxyFramework/");
	$ports = new DatabaseImpl("/usr/local/apache/htdocs/proxy_framework/ports.db", array(5, 10));
	$ippdb =  new DatabaseImpl("/usr/local/apache/htdocs/proxy_framework/ipport.db", array(15, 5));
	$pcounts = array();
	$ipport = array();
	foreach ($files as $f) {
		if (strpos($f, "snapshot-") !== false) {
			print "processing file $f...\n";
			$db = new DatabaseImpl("/usr/local/apache/ProxyFramework/" . $f);
			$db->loadEntries($ent);
			foreach ($ent as $e) {
				if (!isset($pcounts[$e[1]])) {
					$pcounts[$e[1]] = 1;					
					$ipport[$e[0]][$e[1]] = 1;
					$ipp[] = array($e[0], $e[1]);
				} else if (!isset($ipport[$e[0]][$e[1]])) {
					$ipport[$e[0]][$e[1]] = 1;
					$ipp[] = array($e[0], $e[1]);
					$pcounts[$e[1]]++;
				}
			}
		}
	}

        $ps = array();
        foreach ($pcounts as $k => $v) {
            $ps[] = array($k, $v);
	}
	$ports->saveEntries($ps);
	$ippdb->saveEntries($ipp);
?>
