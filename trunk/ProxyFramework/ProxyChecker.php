<?php
// define proxy types
define("PROXY", 0);
define("SOCKS4", 1) ;
define("SOCKS5", 2);

interface ProxyChecker {

    public function check($host, $port, $type = null, $timeout = null);
      
    public function setCheckTimeout($timeout);
      
    public function getCheckTimeout();
}
?>
