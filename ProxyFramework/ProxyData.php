<?php
  class ProxyData {
      public $host;
      public $port;
      public $shownHost;
      public $country;
      public $responseTime;
      public $sslSupport = false;
      public $anonimityLevel;
      public $type;

      public function __toString() {
      	return "host: ".$this->host.", port: ".$this->port.", shown: ".$this->shownHost.", country: ".$this->country.
		", response: ".$this->responseTime.
		", ssl: " . ($this->sslSupport ? "1" : "0") . ", anonLvl: ".$this->anonimityLevel.", type: ".$this->type;
      }
  }
?>
