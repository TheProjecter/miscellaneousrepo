<?php
define("FW_ADDRESS", "tcp://127.0.0.1:1338");
define("WAIT_LIMIT", 30);

      function fw($ip, $port, $action, $rules = null, $logger = null, $proto = 'tcp', $ignore_id = false) {
          if ($ip == null && $port == null && $rules == null) {
	  	die('if ip and port are both null, rules must be non-null');
	  }
      
          do {
              $fail = 0;
              try {	      
                  $fp = @stream_socket_client(FW_ADDRESS, $errno, $errstr, 120) or $fail = 1;
		  stream_set_blocking($fp, 0);
              } catch (Exception $e) {
                  $fail = 1;
              }
              if (!$fp || !is_resource($fp)) {
                  $fail = 1;
              }

              if ($fail && $logger) {
                  $logger->logEntry(LoggingImpl::LEVEL_WARN, "Fw server down.");
              }
          } while ($fail);

	  try {
          	stream_set_timeout($fp, 120);

	  	if ($rules) {
	        	if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, "Committing rules: " . print_r($rules, true));  
	  		foreach ($rules as $rule) {
				stream_socket_sendto($fp, ($ignore_id ? '*' : posix_getpid())." ".$rule."|");
			}
			stream_socket_sendto($fp, "EOF");
	  	} else {
	        	if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, "Sending fw ".($action == '-A' ? "add" : "delete")." req for $ip:$port");
          		stream_socket_sendto($fp, ($ignore_id ? '*' : posix_getpid())." ".$action." INPUT -p ".$proto." -m ".$proto.($port == null ? "" : " -m multiport").($ip != null ? " -s " . $ip . "/32" : ""). " -j ACCEPT".($port == null ? "" : " --sports $port")."|EOF");
	  	}       
	  
          	$start = time();	  
          	while (is_resource($fp) && !feof($fp)) {
			$recv = stream_socket_recvfrom($fp, 1024);
			if (strpos($recv, "BYE") !== false) {
				if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, "received BYE");
				break;
			}

              		if (time() - $start >= WAIT_LIMIT) {	      	  				
                		if ($logger) {
		  			if ($rules) {
						$logger->logEntry(LoggingImpl::LEVEL_WARN, "Commit expired for ".print_r($rules, true).", exiting..");
					} else {
		  				$logger->logEntry(LoggingImpl::LEVEL_WARN, "Waited for over " . WAIT_LIMIT . " seconds on $ip:$port ".($action == '-A' ? "add" : "delete").", exiting..");
					}
		  		}
                  		$retry = true;
                  		break;
              		}

              		// wait for server to close connection
              		usleep(50000);
          	}	  

          	if ($fp && is_resource($fp)) {
              		fclose($fp);
          	}
	  } catch (Exception $e) {
	  	try {
			if ($fp) fclose($fp);
		} catch (Exception $ex) {
			// ignore
		}
	  	$retry = true;
	  }
	  
          if ($retry) {
	      if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, "Retrying send.");
              fw($ip, $port, $action, $rules, $logger);
	  } else if ($rules) {
	      if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, "Rules committed: ".print_r($rules, true));
          } else {
              if ($logger) $logger->logEntry(LoggingImpl::LEVEL_DEBUG, ($action == '-A' ? "Added" : "Deleted") . " fw entry for " . $ip . ":" . $port);
          }	  	  
      }

?>
