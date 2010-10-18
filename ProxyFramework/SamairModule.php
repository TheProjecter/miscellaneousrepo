<?php
require_once('ProxyModule.php');

class SamairModule extends ProxyModule {

    function SamairModule() {    
        ProxyModule::ProxyModule();
    }
        
    protected function parseContent($content) {
	// extract proxies from given content

	// changed 6/28/09        
        $patt = '/<td>([^>]+?):(\d+?)<\/td>/';

        preg_match_all($patt, $content, $matches);
        if (count($matches) >= 3) {
            $newmatches = array();            
	    if (count($matches[1]) > 0 && count($matches[2]) > 0) {
	        for ($i = 0; $i < count($matches[1]); $i++) {
		    $newmatches[] = $matches[1][$i] . ":" . $matches[2][$i];
                }
	    }

	    return $newmatches;                        
        }	

	/*
        $patt1 = '/<script type="text\/javascript">(.*?)<\/script><\/head>/s';
	$patt2 = '/<table class="tablelist" style="font-size:1.1em" border="0" align="center" cellpadding="0" cellspacing="0" width="97%">(.*?)<\/table>/s';
	$patt3 = '/<td>((\d|\.)*?)<script type="text\/javascript">document\.write\("\:"\+(.*?)\)<\/script>/';
	preg_match($patt1, $content, $matches);

	if (count($matches) >= 2) {
	    // variable ecuations	
	    $match = trim($matches[1]);
	    $this->getLogger()->logEntry(LoggingImpl::LEVEL_INFO, $match);
	    $ecs = explode(";", substr($match, 0, strlen($match) - 1));	    
	    $vars = array();
	    
	    if (is_array($ecs)) {
	        foreach ($ecs as $ec) {
	            list($var, $value) = explode("=", $ec);
		    $vars[$var] = $value;
	        }
            } else {
	        return array();
	    }
	    
            // proxy ips and ports
	    preg_match($patt2, $content, $matches);

	    if (count($matches) >= 2) {
	        preg_match_all($patt3, $matches[1], $matches);

		if (count($matches) >= 4) {
		    $proxies = array();
		    $count = count($matches[1]) > count($matches[3]) ? count($matches[3]) : count($matches[1]);
		
		    for ($i = 0; $i < $count; $i++) {
		        $port = "";
			$names = explode("+", $matches[3][$i]);
			
			foreach ($names as $name) {
			    $port .= $vars[$name];
			}

			$proxies[] = $matches[1][$i].":".$port;
		    }

		    return $proxies;
		}
            }
	}
	*/

        return array();
    }
    
    public function getPages() {
    	$format = "http://samair.ru/proxy/proxy-%02d.htm";
	$arr = array();
	for ($i = 1; $i <= 23; $i++) {
	    $arr[] = sprintf($format, $i);
	}

        shuffle($arr);
	return $arr;
    }
    
    public function getUpdateInterval() {
        // 5 hours
        return ceil(3600*(rand(20, 30)/5));
    }
      
    public function getPageDelay() {
        // 30 - 50 seconds wait between pages
	return pow(10, 6)*rand(30, 50);
    }
       
    public function getName() {
        return "SamairModule";
    }
    
    public function getThreadNumber() {
        return 1;
    }    
}
?>
