<?php
	define ("TOP", 30);
	require_once('/usr/local/apache/ProxyFramework/DatabaseImpl.php');
	$db = new DatabaseImpl('/usr/local/apache/htdocs/proxy_framework/ports.db');
	$db->loadEntries($ent);

	usort($ent, create_function('$a,$b', 'return $a[1] < $b[1] ? 1 : -1;'));

	$sum = 0;
	foreach ($ent as $e) {
		$sum += (int)$e[1];
	}

	function prettyprint($a, $sum) {
		printf("%7s %7s %8s\n", "Port", "Count", "Pct.");
		print "-----------------------------\n";
		$i = 0;
		$total_pct = 0;
		$total = 0;
		foreach ($a as $e) {
			$pct = (((float)$e[1] / (float)$sum) * 100);
			$total_pct += $pct;
			$total += $e[1];
			printf("%7s %7s %8.3f%%\n", $e[0], $e[1], $pct);
			if (++$i >= TOP) {
				break;
			}
		}
		print "-----------------------------\n";
		printf("%7s %7s %8.3f%%\n", "Total", $total, $total_pct);
	}

	prettyprint($ent, $sum);
?>
