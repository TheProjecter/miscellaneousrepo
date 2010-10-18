<?php
require_once('ProxyChecker.php');
require_once('ProxyData.php');
require_once('Curl.php');
require_once('LoggingImpl.php');
require_once('SimpleCodec.php');
require_once('Helper.php');

// ip to ping to check for connectivity
define("PING_IP", "209.85.135.104");
// check script location (must support both HTTP and HTTPS protocols)
define("CHECK_SCRIPT", "xyz.com/proxy_framework/check.php");

class ParanoidProxyCheckerImpl implements ProxyChecker {

      private $timeout = 45;     

      private $logger;

      private $curl;

      private $codec;
     
      function ParanoidProxyCheckerImpl($timeout = 45) {
          $this->timeout = $timeout;        
	  $this->curl = new Curl($timeout);
	  $this->logger = new LoggingImpl();
	  $this->codec = new SimpleCodec();
      }
      
      public function getCheckTimeout() {
          return $this->timeout;
      }
      
      public function setCheckTimeout($timeout) {
          $this->curl = new Curl($timeout);
          $this->timeout = $timeout;
      }

      private function runProcess($cmd, $descriptorspec, &$buf) {
          $process = proc_open($cmd, $descriptorspec, $pipes);
	  if (is_resource($process)) {
	      $buf = stream_get_contents($pipes[1]);
	      fclose($pipes[1]);

	      proc_close($process);
	  }
      }
	  
      private function parseOutput($url, &$proxyData, &$success, $host, $port, $type, $ssl = false, $timeout) {
	  if ($timeout != null) {
	  	$this->curl->connectTimeout = $timeout;
	  }

	  // first check that my connection didn't drop
	  // if it did, wait until it comes back
          $descriptorspec = array(
	      0 => array("pipe", "r"),
	      1 => array("pipe", "w"),
	      2 => array("file", "/dev/null", "a"));

	  do {
	      $this->runProcess("ping -c 3 ".PING_IP, $descriptorspec, $buf);

	      if (strpos($buf, "64 bytes from") !== false) {
	          break;
	      }
	      $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Connection down.");

	      sleep(120); // 2 minutes should be appropriate if connection goes down
	  } while (1);
                    
          $start = microtime(true); // this is only measured for http	           
	  $buf = $this->curl->get($url, $host . ":". $port, $type);
	  $elapsed = microtime(true) - $start;
	  $buf = $this->codec->decode(trim($buf));
          
          if (strpos($buf, "HANK") !== false)
          {
              list($junk, $ip, $anonLvl, $usingSSL, $country) = explode("<hr>", $buf);
	      $ip = trim($ip);
	      $country = trim($country);
              
              // we don't like transparent proxies nor proxies that can't read this page properly
              if (trim($anonLvl) == '0' || !preg_match("/^([A-Z]{2}|(N\/A))$/", $country))
              {
                  $success = false;		 
                  return;
              }
              
	      $success = true;

              // set the fields of the proxyData object              
              if ($ssl) {
                  $proxyData->sslSupport = 1;
              } else {
                  $proxyData->host = $host;
                  $proxyData->port = $port;
                  $proxyData->shownHost = $ip;
		  // remove junk added by proxy
		  if ($proxyData->shownHost != $host && strpos($proxyData->shownHost, $host) !== false) {
		      $proxyData->shownHost = $host;
		  }

                  $proxyData->country = trim($country);
		  // remove junk added by proxy
		  if ($proxyData->country != "N/A" && strlen($proxyData->country) > 2) {
		      $proxyData->country = substr($proxyData->country, 0, 2);
		  }

                  $proxyData->responseTime = $elapsed;
                  $proxyData->anonimityLevel = trim($anonLvl);
                  $proxyData->type = $type;
		  $proxyData->sslSupport = 0;
              }
              	      
              return;
          }
          
          $success = false;	  
      }
    
      public function check($host, $port, $type = null, $timeout = null) {	  
	  $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Check parameters: $host $port $type");

          $success = false;          
          $proxyData = new ProxyData();          
          
          // if type is specified, try that type, otherwise:
	  $url = "http://" . CHECK_SCRIPT;	  
	  fw(null, null, null,
	  	array("-A INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --sports $port",
			"-A INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --dports 80,443"), $this->logger);
/*			
          if (!$this->checkFwEntry($host, $port, $type)) {
                $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Fw entry $host:$port was not properly added (check 1), trying again..");
                return $this->check($host, $port, $type);
          }	 
*/

          if ($type) {            
            $this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);	   

	    // if failed, try again with the other 2 possibilities
	    if (!$success) {
                $types = array(PROXY, SOCKS4, SOCKS5);
                array_splice($types, array_search($type, $types), 1);

	    	$type = $types[0];
		$this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);

		if (!$success) {
			$type = $types[1];
			$this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);
		}
	    }
          } else {          
              // try proxy first
              $type = PROXY;              
              $this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);              
          
              // failed, try socks4
              if (!$success) {
                  $type = SOCKS4;                  
                  $this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);
                  
                  // failed, try socks5
                  if (!$success) {
                      $type = SOCKS5;                      
                      $this->parseOutput($url, $proxyData, $success, $host, $port, $type, false, $timeout);                                        
                  }
              }
          }
          
          // if first check (http) succeeded try https too
          if ($success) {	      
	      $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "HTTP check passed.");
	      $url = "https://" . CHECK_SCRIPT;	                                
              $this->parseOutput($url, $proxyData, $success, $host, $port, $type, true, $timeout);              

	      if ($success) {
	          $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "HTTPS check passed.");
	      }
/*              
              if (!$this->checkFwEntry($host, $port, $type)) {
                  $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Fw entry $host:$port was not properly added (check 2), trying again..");
                  return $this->check($host, $port, $type);
              }	      */
              fw(null, null, null,
                  array("-D INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --sports $port",
	                  "-D INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --dports 80,443"), $this->logger);

              // finally, return the ProxyData object
              return $proxyData;
          } else {
/*
              if (!$this->checkFwEntry($host, $port, $type)) {
                  $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Fw entry $host:$port was not properly added (check 3), trying again..");
                  return $this->check($host, $port, $type);
              }	      */
	      fw(null, null, null,
	          array("-D INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --sports $port",
	              "-D INPUT -p tcp -m tcp -m multiport -s " . $host . "/32 -j ACCEPT --dports 80,443"), $this->logger);
	      
              return null; // unsuccessfull
	  }
      }  
}
?>
