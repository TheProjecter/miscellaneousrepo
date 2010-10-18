#!/usr/local/bin/php
<?php       	
	define("SYNTAX", '/^\-(A|D) INPUT \-p (tcp|udp|icmp) \-m (tcp|udp|icmp) \-m multiport \-s [0-9a-zA-Z\.\-]+\/32 \-j ACCEPT \-\-[ds]{1,1}ports [0-9,]+$/');
	define("SYNTAX_PORT", '/^\-(A|D) INPUT \-p (tcp|udp|icmp) \-m (tcp|udp|icmp) \-m multiport \-j ACCEPT \-\-[ds]{1,1}ports [0-9,]+$/');
	define("SYNTAX_IP", '/^\-(A|D) INPUT \-p (tcp|udp|icmp) \-m (tcp|udp|icmp) \-s [0-9a-zA-Z\.\-]+\/32 \-j ACCEPT$/');

	define("NS_FILE", "/etc/resolv.conf");
	define("THREAD_COUNT", 45);
        define("ENTRY_INDEX", 0);
        define("DNS_INDEX", 1);
        define("EVICT_INTERVAL", 30*60);
        define("EVICT_CHECK_INTERVAL", 10*60);

	set_time_limit(0);
	error_reporting(E_ERROR | E_WARNING);
	function exception_error_handler($errno, $errstr, $errfile, $errline ) {
		throw new ErrorException($errstr, 0, $errno, $errfile, $errline);
	}
	set_error_handler('exception_error_handler', error_reporting());

	function debug($str) {
	    $fp = fopen("/root/fwdaemon.txt", "a+");
	    flock($fp, LOCK_EX);
	    $msg = date("[m/d/Y H:i:s ".posix_getpid()."] ") . $str . "\n";
	    print $msg;
	    fwrite($fp, $msg);
	    fclose($fp);
	}

	function iptables($str, $action) {
	    shell_exec("iptables $action $str");		
	}

        function getips($id, $host, $action) {		
		lock(DNS_INDEX);		
		$dns = getVar(DNS_INDEX);

		global $ns;
                $ips = array();				
		// don't delete dns everytime, leave it be and it will be evicted when not used, it's overkill otherwise
		if ($action == '-A') {
		    lock();		
		    $adds = getVar();
		}

		if ($action == '-A') {
	        	foreach ($ns as $n) {				
		    		$c = "INPUT -p udp -m udp -m multiport -s $n/32 -j ACCEPT --sports 53";				
				$exists = $adds[$c];
				if ($exists && my_array_search($id, $adds[$c]) !== false) {
					continue;
				}
				$adds[$c][] = $id . ":" . time();
		    		if (!$exists) {
		    			debug("added entry $c");				
		    			iptables($c, $action);
				}
			}
		}
		
		if (!$dns[$host]) { // should never happen since it's stored in cache when added
		    $buf = shell_exec("hostx $host");
		}

		// don't delete ns entries, leave them there for reuse
		/*
		if ($action == '-D') {
		    foreach ($ns as $n) {
		    	$c = "INPUT -p udp -m udp -m multiport -s $n/32 -j ACCEPT --sports 53";
			if (checkDeleted($adds, $id, $c)) {
				debug("deleted entry $c");
				iptables($c, $action);
			}
		    }		    
		}
		*/

		if ($action == '-A') {
		    setVar($adds);
		    unlock();
		}

		// if dns value is already cached
		if ($dns[$host]) {
			unlock(DNS_INDEX);
			debug("used cached value for $host");
			return $dns[$host];
		}

                if (strpos($buf, "not exist")) {
		    unlock(DNS_INDEX);
                    return $ips;
                }

                $lines = explode("\n", $buf);
                for ($i = 0; $i < count($lines) - 1; $i++) {
                        if (!strpos($lines[$i], "A\t")) {
                                continue;
                        }

                        $parts = explode("\t", $lines[$i]);
                        $ip = $parts[count($parts) - 1];
                        $ips[] = trim($ip);
                }
		$dns[$host] = $ips;
		setVar($dns, DNS_INDEX);
		unlock(DNS_INDEX);

                return $ips;
        }

	function getns() {
		$lines = file(NS_FILE);
		$ns = array();
		foreach ($lines as $n) {
		        $match = array();			
			if (strpos($n, "nameserver ") !== false && preg_match('/[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}/', $n, $match)) {
				$ns[] = trim($match[0]);
			}
		}
		return $ns;
	}

	function checkDeleted(&$adds, $id, $cmd, $set = true) {
		if (!$adds[$cmd] || !is_array($adds[$cmd])) {
		    return true;
		}

		if ($id == '*') {
			$change = true;
			unset($adds[$cmd]);
		} else {
			for ($i = count($adds[$cmd]) - 1; $i >= 0; $i--) {
		    		$parts = explode(":", $adds[$cmd][$i]);
		    		if ($parts[0] == $id) {
		        		$change = true;
					array_splice($adds[$cmd], $i, 1);
					break;
		   		}
			}		

                	if (count($adds[$cmd]) == 0) {
                    		unset($adds[$cmd]);
                	}
		}

		if ($change && $set) {
		    setVar($adds);
		}		

		return !isset($adds[$cmd]);
	}

	function getVar($index = ENTRY_INDEX) {	
		global $mem;
		return unserialize(shmop_read($mem[$index], 0, shmop_size($mem[$index])));
	}

	function setVar($var, $index = ENTRY_INDEX) {
		global $mem;
		$str = serialize($var);
		shmop_write($mem[$index], $str, 0);
	}

	function lock($m = ENTRY_INDEX) {
		global $mutex, $mutex2;
		sem_acquire($m == ENTRY_INDEX ? $mutex : $mutex2);
	}

	function unlock($m = ENTRY_INDEX) {
		global $mutex, $mutex2;
		sem_release($m == ENTRY_INDEX ? $mutex : $mutex2);
	}

	function listen($address) {
                $socket = stream_socket_server($address, $errno, $errstr);		
		stream_set_blocking($socket, 0);
		setVar(array());
		setVar(array(), DNS_INDEX);

                if (!$socket) {
                    die("Error occurred while starting fw server: $errstr ($errno)");
                } else {
		    debug("listening for connections");
		    for ($i = 0; $i < THREAD_COUNT; $i++) {
		    	$pid = pcntl_fork();
			if ($pid == -1) {
				 die('fail');
			} else if ($pid) {
				debug("pid $pid launched");
			} else {							
		    		accept_connection($socket);
		    	}
		    }
		}

		$evict = time();
        	$exits = 0;
        	do {
			if (time() - $evict > EVICT_CHECK_INTERVAL) {
				// evict old entries, this must be an atomic operation
				lock();
				$adds = getVar();
				foreach ($adds as $k => $v) {
					for ($i = count($adds[$k]) - 1; $i >= 0; $i--) {
						$parts = explode(":", $adds[$k][$i]);
						if (((int)$parts[1]) + EVICT_INTERVAL < time()) {
							debug("evicted expired entry $k for id ".$parts[0]." (added at " . date("d-M-Y H:i:s", $parts[1]) . ")");
							array_splice($adds[$k], $i, 1);
						}						
					}

					// if no remaining ids, delete the rule
					if (count($adds[$k]) == 0) {
						debug("evicted entry $k");
						iptables($k, "-D");
						unset($adds[$k]);
					}
				}
				setVar($adds);
				unlock();
				$evict = time();
			}
            		$res = pcntl_waitpid(-1, $status, WNOHANG);
			usleep(500000);
        	} while (!$res || ++$exits < THREAD_COUNT);
	}

	function my_array_search($needle, $stack) {
		foreach ($stack as $i) {
			$parts = explode(":", $i);
			if ($parts[0]== $needle) {
				return true;
			}
		}
		return false;
	}

	function accept_connection($socket) {
		    while (1) {
		        try {
                        	$conn = stream_socket_accept($socket, 0);
				stream_set_blocking($conn, 0);
				stream_set_timeout($conn, 120);
				debug("new connection");			
				$start = time();
				while (!feof($conn)) {
				        if (time() - $start > 10) {
					    break;
					}

                            		$req = trim(stream_socket_recvfrom($conn, 1024));
					if (strlen($req) == 0) {
						continue;
					}

			    		debug("received $req");					
					$eof = false;
					if (strpos($req, "EOF") !== false) {
					    $eof = true;
					    $req = str_replace("EOF", "", $req);
					}
			    			
				        $cmd = explode("|", $req);
					debug("command count: ".(count($cmd) - 1));
					for ($i = 0; $i < count($cmd) - 1; $i++) {
					    $c = $cmd[$i];
					    debug("command #$i: $c");
					    $c = preg_replace('/ 0+(\d+)$/', ' $1', $c);
					    $id = substr($c, 0, strpos($c, " "));
					    $c = substr($c, strpos($c, " ") + 1);					    
					    if ((!preg_match(SYNTAX, $c) && !preg_match(SYNTAX_IP, $c) && !preg_match(SYNTAX_PORT, $c)) || (!is_numeric($id) && $id != '*')) {
					    	debug("invalid entry: " . $cmd[$i]);
					    	continue;
					    }					    

					    
					    $host = substr($c, strpos($c, "-s ") + strlen("-s "), strpos($c, "/32") - strpos($c, "-s ") - strlen("-s "));
					    $cmds = array();
					    $type = substr($c, 0, strpos($c, " "));
					    debug("type = $type");

					    if ((preg_match(SYNTAX, $c) || preg_match(SYNTAX_IP, $c)) && preg_match('/[^0-9\.]/', $host)) {
					    	debug("getting ip");						
					        $ips = getips($id, $host, $type);
						//debug("lock block 1 took " .(microtime(true) - $start));
						debug("got ip");

						// for these hosts, let the rules be evicted rather than remove them, since it will be overkill
						// as they are used very often
						foreach ($ips as $ip) {
						    $cmds[] = str_replace($host, $ip, $c);
						}						
					    } else {
					        $cmds[] = $c;
					    }
					    
					    foreach ($cmds as $c) {
						$c = substr($c, strpos($c, " ") + 1);					
						lock();						
						//$start = microtime(true);						
						$adds = getVar();
						if ($type == '-A') {
						    $exists = $adds[$c];
					            if ($exists && my_array_search($id, $adds[$c]) !== false) {
						        unlock();
						        continue;
						    }
						    $adds[$c][] = $id . ":" . time();						    
						    setVar($adds);						    
						    if (!$exists) {
			        		        debug("adding entry $c");
						        iptables($c, "-A");
						    }
						} else if ($type == '-D') {
						    if (checkDeleted($adds, $id, $c)) {
						        debug("deleting entry $c");
							iptables($c, "-D");
						    }
						}
						//debug("lock block 2 took " .(microtime(true) - $start));
						unlock();
					    }
					}
					
					if ($eof) {
					    debug("next..");
					    break;
					}

					usleep(50000);
				}

				stream_socket_sendto($conn, "BYE");
				@fclose($conn);
			} catch (Exception $e) {
			    try {
			        unlock(); // in case it was locked
				unlock(DNS_INDEX);
			    } catch (Exception $ex) {
			        // ignore
			    }
			}

			usleep(50000);		    
                }
	}       


	$keys = array('0xabc', '0xabe');
	$mutex = sem_get($keys[ENTRY_INDEX]);	
	$mutex2 = sem_get($keys[DNS_INDEX]);	
	$mem = array(shmop_open($keys[ENTRY_INDEX], "c", 0644, 512*1024), shmop_open($keys[DNS_INDEX], "c", 0644, 512*1024));	

	system("/etc/init.d/iptables reload"); // clean up
	$ns = getns();
	listen("tcp://127.0.0.1:1338");

	foreach ($keys as $key) {
		shmop_delete($key);
		shmop_close($key);
	}
?>
