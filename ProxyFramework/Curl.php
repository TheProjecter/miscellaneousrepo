<?php
require_once('CurlResponse.php');

class Curl {
   
    public $headers = array();
    public $options = array();
    public $referer = '';
    public $userAgent = '';   

    protected $error = '';

    private $proxyTypes = array(CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS5);
    private $timeout = 45;
    public $connectTimeout = null;

    public function __construct($timeout = 45) {
        $this->userAgent = $_SERVER['HTTP_USER_AGENT'];
	$this->timeout = $timeout;
    }

    public function delete($url, $vars = array()) {
        return $this->request('DELETE', $url, $vars);
    }

    public function error() {
        return $this->error;
    }

    public function get($url, $proxy = null, $proxyType = null, $vars = array(), $writeFunc = null) {           
        if (!empty($vars)) {
            $url .= (stripos($url, '?') !== false) ? '&' : '?';
            $url .= http_build_query($vars);
        }
        return $this->request('GET', $proxy, $proxyType, $url, $vars, $writeFunc);
    }

    public function post($url, $proxy = null, $proxyType = null, $vars = array(), $writeFunc = null) {
        return $this->request('POST', $proxy, $proxyType, $url, $vars, $writeFunc);
    }

    public function put($url, $proxy = null, $proxyType = null, $vars = array(), $writeFunc = null) {
        return $this->request('PUT', $proxy, $proxyType, $url, $vars, $writeFunc);
    }

    protected function request($method, $proxy = null, $proxyType = null, $url, $vars = array(), $writeFunc = null) {
        $handle = curl_init();
       
        # Set some default CURL options
	if ($proxy != null) {
	    curl_setopt($handle, CURLOPT_PROXY, $proxy);	    
	    curl_setopt($handle, CURLOPT_PROXYTYPE, $this->proxyTypes[$proxyType]);
	} else {
            die('I refuse to send the request without using a proxy.');
	}

	curl_setopt($handle, CURLOPT_SSL_VERIFYPEER, false);	
        curl_setopt($handle, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($handle, CURLOPT_HEADER, true);
        curl_setopt($handle, CURLOPT_POSTFIELDS, http_build_query($vars));
        curl_setopt($handle, CURLOPT_REFERER, $this->referer);
        curl_setopt($handle, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($handle, CURLOPT_URL, $url);
        curl_setopt($handle, CURLOPT_USERAGENT, $this->userAgent);
	curl_setopt($handle, CURLOPT_CONNECTTIMEOUT, $this->connectTimeout ? $this->connectTimeout : $this->timeout);	
	curl_setopt($handle, CURLOPT_TIMEOUT, $this->timeout);
	
	if ($writeFunc != null) {	
            curl_setopt($handle, CURLOPT_WRITEFUNCTION, $writeFunc);
	}               

        # Format custom headers for this request and set CURL option
        $headers = array();
        foreach ($this->headers as $key => $value) {
            $headers[] = $key.': '.$value;
        }
        curl_setopt($handle, CURLOPT_HTTPHEADER, $headers);
       
        # Determine the request method and set the correct CURL option
        switch ($method) {
            case 'GET':
                curl_setopt($handle, CURLOPT_HTTPGET, true);
                break;
            case 'POST':
                curl_setopt($handle, CURLOPT_POST, true);
                break;
            default:
                curl_setopt($handle, CURLOPT_CUSTOMREQUEST, $method);
        }
       
        # Set any custom CURL options
        foreach ($this->options as $option => $value) {
            curl_setopt($handle, constant('CURLOPT_'.str_replace('CURLOPT_', '', strtoupper($option))), $value);
        }
       
        $response = curl_exec($handle);
        if ($response) {
            $response = new CurlResponse($response);
        } else {
            $this->error = curl_errno($handle).' - '.curl_error($handle);
        }
        curl_close($handle);
        return $response;
    }
}
?>
