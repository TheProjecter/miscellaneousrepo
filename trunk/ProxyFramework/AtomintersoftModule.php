<?php
require_once('ProxyModule.php');

class AtomintersoftModule extends ProxyModule {

    function AtomintersoftModule() {    
        ProxyModule::ProxyModule();
    }
        
    protected function parseContent($content) {
        // extract proxies from given content
        $patt1 = '/<div class="content clear\-block">(.*)<\/div>/s';	
        $patt2 = '/alt="Check (proxy|socks proxy) (.*?) now"/';
        
        preg_match($patt1, $content, $matches);
        if (count($matches) >= 2) {	   
            preg_match_all($patt2, $matches[1], $matches);
	    if (count($matches) >= 3) {
	        return $matches[2];
	    }
        }

        return array();
    }
    
    public function getPages() {
        $arr = array(
	    "http://atomintersoft.com/free_proxy_list",
            "http://atomintersoft.com/transparent_proxy_list",
            "http://atomintersoft.com/anonymous_proxy_list",
            "http://atomintersoft.com/high_anonymity_elite_proxy_list",
	    "http://atomintersoft.com/products/alive-proxy/socks5-list",
	    "http://atomintersoft.com/products/alive-proxy/proxy-list",
	    "http://atomintersoft.com/proxy_list_port_80",
	    "http://atomintersoft.com/proxy_list_port_81",
	    "http://atomintersoft.com/proxy_list_port_3128",
	    "http://atomintersoft.com/proxy_list_port_8000",
	    "http://atomintersoft.com/proxy_list_port_8080"
        );
	shuffle($arr);

	return $arr;
    }
    
    public function getUpdateInterval() {
        // 3 hours
        return ceil(3600*(rand(14, 20)/6));
    }
      
    public function getPageDelay() {
        // 3 seconds wait between pages
        return pow(10, 6)*rand(3, 6);    
    }
       
    public function getName() {
        return "AtomintersoftModule";
    }
    
    public function getThreadNumber() {
        // single thread retrieval
        return 1;
    }    
}
?>
