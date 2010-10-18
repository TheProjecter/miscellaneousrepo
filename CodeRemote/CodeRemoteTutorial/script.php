<?php
	function greet($now)
	{
		print "Hello world @ ".date("d-M-Y H:i:s", $now);
	}

	greet(time());
?>