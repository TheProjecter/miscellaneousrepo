<?php
require_once('Helper.php');
set_time_limit(0);
error_reporting(E_ERROR | E_WARNING);
function exception_error_handler($errno, $errstr, $errfile, $errline ) {
    throw new ErrorException($errstr, 0, $errno, $errfile, $errline);
}
set_error_handler('exception_error_handler', error_reporting());

try {
	include('ProxyFramework.php');
	//include('ProxyCheckerImpl.php');
	include('ParanoidProxyCheckerImpl.php');
	include('AtomintersoftModule.php');
	include('SamairModule.php');
	include('FreeCheckerModule.php');
	include('NNTimeModule.php');
	
	// set proper permissions
	if (posix_getgid() != GID) {
		posix_setgid(GID);
	}

        if (posix_getuid() != UID) {
                posix_setuid(UID);
        }

        if (posix_getegid() != GID) {
                posix_setegid(GID);
        }

        if (posix_geteuid() != UID) {
                posix_seteuid(UID);
        }


	// first check that an instance is not already running
	if (file_exists(PIDFILE)) {
		$line = file(PIDFILE);
		$pid = trim($line[0]);
		if (count(explode("\n", shell_exec("ps --pid ".$pid))) > 2) {
			die("An instance of the daemon is already running with PID $pid\n");
		} else {
			// no process with that PID, can safely remove the existing PID file
			print "Found dangling PID file, removing...\n";
			unlink(PIDFILE);
		}
	}       

        // fork the main process
        $pid = pcntl_fork();

	if ($pid == -1) {
	    die("Could not fork!");
        } else if ($pid) {
            // store the pid
            $fp = fopen(PIDFILE, "w+");
            fwrite($fp, $pid."\n".time());
            fclose($fp);

	    while (file_exists(PIDFILE)) {
	        usleep(50000);
	    }
        } else {	
            //$checker = new ProxyCheckerImpl();
	    $checker = new ParanoidProxyCheckerImpl();
	    $framework = new ProxyFramework("framework.db", $checker, 7200, 5400, 1, 30, 20, PIDFILE);

            $atom = new AtomintersoftModule();
	    $samair = new SamairModule();
	    $freechecker = new FreeCheckerModule();
	    $nntime = new NNTimeModule();

	    $framework->addModule($atom);

	    // since samair.ru is down most of the time, perform this check before adding the samair module to PF	    	    
	    print "Checking Samair\n";
	    fw('samair.ru', 80, '-A');
            $test = shell_exec('curl -f samair.ru -m 20 --connect-timeout 20');	    
	    //fw('samair.ru', 80, '-D');
	    if (trim($test) != '' && strpos($test, "connect() timed out") === false) {
                $framework->addModule($samair);
	    }

            $framework->addModule($freechecker);
            
            print "Checking NNTime\n";
	    fw('nntime.com', 80, '-A');
	    $test = shell_exec('curl -f nntime.com -m 20 --connect-timeout 20');	   
	    //fw('nntime.com', 80, '-D'); 
	    if (trim($test) != '' && strpos($test, "connect() timed out") === false) {
	        $framework->addModule($nntime);
	    }	    

            print "Starting.\n";
	    $framework->start();
	    $framework->stop();
	}
} catch (Exception $e) {
	Helper::logCritical("Exception occurred: " . $e);
}
?>
