<?php	
	if (!defined("PF_DIR")) {
	    define("PF_DIR", "/usr/local/apache/ProxyFramework");
	}

	require_once(PF_DIR . '/DatabaseImpl.php');

	class Stats {
		const BAD = 1;
		const FAST = 2;
		const SSL = 3;
		const GOOD = 4;		

		private $db;

		public function Stats() {
			$this->db = new DatabaseImpl("/usr/local/apache/htdocs/proxy_framework/stats.db", array(20, 1, 6));
			print "serving " . $this->db->entryCount() . " entries\n";
		}

		public function addStats($date, $type, $count) {
			$this->db->saveEntries(array($date, $type, $count));
		}

		public function getLastUpdate() {
			$this->db->loadEntries($entries);
			if (is_array($entries) && count($entries) > 0) {
				$date = $entries[count($entries) - 1][0];
				// fmt: YYYY.mm.dd.HH.ii.ss
				sscanf($date, "%d.%d.%d.%d.%d.%d", $y, $m, $d, $h, $i, $s);
				return mktime($h, $i, $s, $m, $d, $y);
			}

			return 0;
		}

		public function getStats($type) {
			$ret = array();
			print "getting stats..\n";
			$start = microtime(true);
			$ids = $this->db->searchEntries(SEARCH_FIELD_OFFSET, 1, $type);
			print "searching: " . (microtime(true) - $start) . "\n";
			if ($ids == -1) {			
			    return $ret;
			}

			if (!is_array($ids)) {
			    $ids = array($ids);
			}

			$start = microtime(true);
			$this->db->loadEntries($entries, $ids);
			print "loading: " . (microtime(true) - $start) . "\n";

			$start = microtime(true);
			foreach ($entries as $e) {
				$ret[] = $e[2];
			}
			print "adding entries to array: " . (microtime(true) - $start) . "\n";

			return $ret;
		}	

		public function getDates($fmt) {
			$this->db->loadEntries($entries);
			$ret = array();
			$dates = array();
			if (is_array($entries) && count($entries) > 0) {
				foreach ($entries as $e) {
					if (!in_array($e[0], $dates)) {
					    $dates[] = $e[0];
					    sscanf($e[0], $fmt, $y, $m, $d);
					    $ret[] = sprintf($fmt, $y, $m, $d);
					}
				}
			}			

			return $ret;
		}
	}
?>
