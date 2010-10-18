<?php
require_once('LoggingImpl.php');
require_once('Helper.php');

abstract class ProxyModule {

    private $logger;

    /**
     * Default constructor.
     */
    function ProxyModule() {
        $this->logger = new LoggingImpl();
    }
      
    public function getProxies($page) {
        $url = parse_url($page);
	if (!$url['port']) {
	    $url['port'] = 80; // default
	}
	// now with paranoid fw support
        try {	    
	    fw($url['host'], $url['port'], '-A');
            $content = file_get_contents($page) or $err = 1;	    
	    //fw($url['host'], $url['port'], '-D');
	} catch (Exception $e) {
	    $err = 1;
	    // ignore since it's logged below
	}
	while ($err) {	    
	    $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Could not fetch page $page.");
	    try {
	        fw($url['host'], $url['port'], '-A');
	        $content = file_get_contents($page) or $err = 1;		
		//fw($url['host'], $url['port'], '-D');
		$err = 0;
	    } catch (Exception $e) {
	        $err = 1;
	        // ignore since it's logged anyway
	    }
	}
        $entries = $this->parseContent($content);
        if (count($entries) == 0) {
	    $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "No proxies were found on page $page");
	    return $entries;
	} else {
	    $newentries = array();
	    foreach ($entries as $entry) {
		if (!preg_match('/^([A-Za-z0-9\\-.]+?):(\d+?)$/', $entry)) {
		    $this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Proxy '$entry' from page $page is badly formatted");
		} else {
		    // strip leading zeros for the port
		    $newentries[] = preg_replace('/:0+(\d+)$/', ':$1', $entry);
		}
	    }
	    return $newentries;
	}	
    }    
    
    // implementation for each module, will extract proxies from given content -- internal
    abstract protected function parseContent($content);    
    
    // implementation for each module -- caled by framework
    abstract public function getPages();
    
    // implementation for each module -- called by framework
    abstract public function getUpdateInterval();
    
    // implementation for each module -- called by framework     
    abstract public function getPageDelay();
    
    // implementation for each module -- called by framework     
    abstract public function getName();
    
    // implementation for each module  -- caled by framework   
    abstract public function getThreadNumber();

    protected function getLogger() {
        return $this->logger;
    }
}  
  
?>
