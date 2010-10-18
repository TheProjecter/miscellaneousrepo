<?php
require_once('ProxyModule.php');

class NNTimeModule extends ProxyModule {

    function NNTimeModule() {    
        ProxyModule::ProxyModule();
    }
        
    protected function parseContent($content) {
	// extract proxies from given content
	if (preg_match("/([a-zA-Z0-9=;]+?)<\/script><link/", $content, $match)) {
	    $exps = explode(";", $match[1]);
	    if (count($exps) < 2) {
	       return array();
	    }
	    array_pop($exps);

	    foreach ($exps as $e) {
	        list($key, $value) = explode('=', $e);
		$key = trim($key);
		$value = trim($value);
                if ($key == '' || $value == '') {
		    return array();
		}

	        $values[$key] = $value;
	    }
	} else {
	    return array();
	}

	// changed 10/28/09        
        $patt = '/onclick="choice\(\)" \/><\/td><td>([^>]+?)<script type="text\/javascript">document\.write\("\:"\+([\+a-zA-Z]+?)\)<\/script>/';

        preg_match_all($patt, $content, $matches);
        if (count($matches) >= 3) {
            $newmatches = array();            
	    if (count($matches[1]) > 0 && count($matches[2]) > 0) {
	        for ($i = 0; $i < count($matches[1]); $i++) {		    
		    $port = $matches[2][$i];
		    // replace all js variables
		    foreach ($values as $k => $v) {
		        $port = str_replace($k, $v, $port);
		    }
		    // trim the concatenators
		    $port = str_replace("+", "", $port);		    
		    $newmatches[] = $matches[1][$i] . ":" . $port;
                }
	    }

	    return $newmatches;                        
        }

        return array();
    }
    
    public function getPages() {
    	$format = "http://nntime.com/proxy-list-%02d.htm";
	$arr = array();
	for ($i = 1; $i <= 15; $i++) {
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
        return "NNTimeModule";
    }
    
    public function getThreadNumber() {
        return 1;
    }    
}
?>
