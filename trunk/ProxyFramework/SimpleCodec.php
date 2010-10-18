<?php

class SimpleCodec {
	const SALT = 'fdsf34f34y45hg45h26ui';

	private $values = array(
		"<",
		"h",
		"r",
		">",
		".",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
                "N",
		"/",
		"\n",
		"\r",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
                "L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z"
	);

	private $encodeMap = array();

	private $decodeMap = array();

	public function SimpleCodec() {
	   	foreach ($this->values as $v) {
	        	$md5 = md5($v . SimpleCodec::SALT);
	        	$this->decodeMap[$md5] = $v;
			$this->encodeMap[$v] = $md5;		
	  	}	    
	}

	public function encode($buf) {
		$newbuf = '';
		for ($i = 0; $i < strlen($buf); $i++) {
			$newbuf .= $this->encodeMap[$buf[$i]];
		}

		return $newbuf;
	}

	public function decode($buf) {
		$newbuf = '';
		for ($i = 0; $i < strlen($buf); $i += 32) {
			$code = substr($buf, $i, 32);
			$newbuf .= $this->decodeMap[$code];
		}

		return $newbuf;
	}
}
