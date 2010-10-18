<?php
	define("HOST", "123.123.123.123);
	define("REVERSE1", "123.123.123.123.in-addr.arpa");
	define("REVERSE2", "some.host.name");
	define("REVERSE3", "some.other.hostname");

	include_once('ip2country.php');
	$ip2c=new ip2country();
	$ip2c->mysql_host='localhost';
	$ip2c->db_user='user';
	$ip2c->db_pass='password';
	$ip2c->db_name='ip2country';
	$ip2c->table_name='ip2c';
?>
HANK
<hr>
<?php
	$ip = trim($_SERVER["REMOTE_ADDR"]);
	print $ip;
?>
<hr>
<?php
	// first, test if transparent (0)
	foreach ($_SERVER as $key => $value) {
		if ((strpos($value, HOST) !== false ||
		    strpos($value, REVERSE1) !== false ||
		    strpos($value, REVERSE2) !== false ||
		    strpos($value, REVERSE3) !== false) &&
		    $key != "HTTP_HOST" && $key != "SERVER_ADDR" && $key != "SERVER_NAME" && $key != "SERVER_SIGNATURE") {
		    $transparent = true;
		    break;
		}
	}

	if ($transparent) {
		$lvl = 0;
	} else {
		// test if anonymous (1) or highly anonymous (2)
		$lvl = isset($_SERVER["HTTP_X_FORWARDED_FOR"]) || isset($_SERVER["HTTP_VIA"]) ? 1 : 2;	
	}

	print $lvl;
?>
<hr>
<?php
	// are we using ssl?
	print $_SERVER["SERVER_PORT"] == 443 ? 1 : 0;
?>
<hr>
<?php
	// country of origin	
	print $ip2c->get_country_code($ip);
?>
