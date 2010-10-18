<?php
	$pid = pcntl_fork();
	if ($pid) {
	} else {
	    exec("/usr/local/bin/php daemon.php & 2>> /usr/local/apache/ProxyFramework/daemon.log");
	    exit;
	}
?>
