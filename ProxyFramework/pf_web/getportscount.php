<?php
	require_once("/usr/local/apache/ProxyFramework/DatabaseImpl.php");
	$db = new DatabaseImpl("/usr/local/apache/htdocs/proxy_framework/ports.db");
	print "port count = " . $db->entryCount() . "\n";
?>
