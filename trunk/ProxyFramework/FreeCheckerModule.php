<?php
require_once('ProxyModule.php');

class FreeCheckerModule extends ProxyModule {

    function FreeCheckerModule() {    
        ProxyModule::ProxyModule();
    }
        
    protected function parseContent($content) {
	// extract proxies from given content
        $patt1 = '/<script> <!\-\-(.*?)\/\/ \-\-><\/script>/s';
	preg_match_all($patt1, $content, $matches);
	$proxies = array();

        if (count($matches) >= 2) {
	    for ($i = 0; $i < count($matches[1]); $i++) {
	        $ecs = explode(";", $matches[1][$i]);
	        $vars = array();
	        $rand = "";

                for ($j = 0; $j < count($ecs) - 2; $j++) {
	            list($name, $var) = explode("=", $ecs[$j]);
		    $name = trim($name);
		    $var = trim($var);

		    if (strpos($var, 'port') !== false) {
		        $var = preg_replace('/(\w+)(\d+)/', '\$vars[$1$2]', $var, 1);
		        $rand = $name;
		    }

		    $vars[$name] = $var;	            	
	        }    

                foreach ($vars as $name => $var) {
	            eval('$vars['.$name.'] = '.$var.';');
	        }

	        $proxies[] = $vars[name].':'.$vars[$rand];
	    }	  

	    return $proxies;
	}

        return array();
    }
    
    public function getPages() {
    	return array("http://www.checker.freeproxy.ru/checker/last_checked_proxies.php");
    }
    
    public function getUpdateInterval() {
        // 4 hours
        return ceil(3600*(rand(13, 17)/4));
    }
      
    public function getPageDelay() {
        // only one page so this doesnt matter anyway
	return 1;
    }
       
    public function getName() {
        return "FreeCheckerModule";
    }
    
    public function getThreadNumber() {
        // single thread
        return 1;
    }    
}
?>
