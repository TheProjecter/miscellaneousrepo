<?php

require_once('DatabaseImpl.php');

class UniqueIPDatabase {

	private $db;

	public function UniqueIPDatabase() {
	    $this->db = new DatabaseImpl("/usr/local/apache/ProxyFramework/uniques.db", array(100));
	    $this->db->cacheEntries(); // preemptive load
	}

	public function addProxy($ip) {
	    $ids = $this->db->searchEntries(SEARCH_DEFAULT, $ip);
	    
	    if ($ids == -1) {
	        // add new
		$this->db->saveEntries(array($ip));
	    }
	}

	public function getProxy($ip) {
	    $ids = $this->db->searchEntries(SEARCH_DEFAULT, $ip);

	    if ($ids != -1) {
	        $id = is_array($ids) ? $ids[0] : $ids;
		$this->db->loadEntries($entry, $id);
		return $entry[0];
	    }

	    return null;
	}

	public function getIPsForPort($port) {
		$ids = $this->db->searchEntries(SEARCH_FIELD_OFFSET | SEARCH_ARRAY, 1, $port);
		if (count($ids) == 0) {
			return $ids;
		}
		$this->db->loadEntries($ents, $ids);
		return $ents;
	}

	public function getDatabase() {
	    return $this->db;
	}
}
?>
