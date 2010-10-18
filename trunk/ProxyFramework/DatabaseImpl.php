<?php
require_once('Database.php');
require_once('LoggingImpl.php');

// search options
define('SEARCH_DEFAULT', 0);
define('SEARCH_FIELD_OFFSET', 1);
define('SEARCH_COMPARE', 2);
define('SEARCH_ARRAY', 4);
define('SEARCH_REAL_ID', 8);
define('SEARCH_REFERENCED_ENTRIES', 16);
define('SEARCH_RESULTS_MAPPED', 32);

/**
 * This class implements the Database interface, adding core functionality to it.
 */
class DatabaseImpl implements Database {

	/**
	 * Filename to save the entries to. It is initially set to 'db.txt' and can be set via its setter.
	 */
	private $file = 'db.txt';

	/**
	 * Separator for entry fields. Can be changed via its setter.
	 */
	private $entryFieldSeparator = null;

	/**
	 * Separator for entries. Can be changed via its setter.
	 */
	private $entrySeparator = null;

	/**
	 * Separator for header sections. Can be changed via its setter.
	 **/
	private $headerSeparator = null;

	/**
	 * Separator for header fields. Can be changed via its setter.
	 **/
	private $headerFieldSeparator = null;

	/**
	 * Separator for header entries. Can be changed via its setter.
	 **/
	private $headerEntrySeparator = null;

        /**
         * Character to pad fields with until they reach maximum length. Can be changed via its setter.
         */
        private $padCharacter = null;

	/**
	 * Logging object used to log events. Initialized upon construction and never changed.
	 */
	private $logger = null;

	/**
	 * Keeps track of entries so we don't have to load them every time.
	 */
	private $entries = null;

        /**
	 * An array of maximum field lengths so we know how much to pad each field when it doesn't reach its full length.
	 */
	private $structure = null;

	/**
         * DEPRECATED (will be removed soon): The cached buffer. It's stored in memory so we don't have to read the database file each time.
         */
//	private $cachedBuffer = null;

        /**
	 * Array of deleted offsets. Whenever a new entry is added, the first entry is removed.
	 * When an entry is deleted a new offset is added here (the offset of the start of the entry).
	 */
	private $deletedOffsets = array();

        /**
	 * The length of one entry in the file.
	 */
	private $entryLength;

        /**
	 * Whether to log messages at debug level or ignore them.
	 */
	private $debug;	

	/**
	 * Represents the length of the header. This helps us determine the offset we write entries at.
	 */
	private $headerLength;

	/**
	 * Represents the array of foreign keys.
	 */
	private $fks = array();

	/**
	 * Represents the array of referenced databases. Used for fetching 
	 * referenced entries.
	 */
	private $referencedDbs = array();

	/**
	 * Initializes some of the fields to default values.
	 *
	 * @param string $file The file the database will use
	 * @param array $structure The field structure array
	 * @param array $fks The foreign key array
	 * @param bool $debug Whether to log debug information or not
	 */
	function DatabaseImpl($file, $structure = null, $fks = null, $debug = false) {
		$this->logger = new LoggingImpl();
		Helper::checkString("file", $file, $this->logger);

		$this->file = $file;
		$this->entryFieldSeparator = chr(1);
		$this->entrySeparator = chr(2);
		$this->padCharacter = chr(3);

		$this->headerSeparator = chr(4);
		$this->headerEntrySeparator = ";";
		$this->headerFieldSeparator = ",";

		if ($structure) {
			Helper::checkArray("structure", $structure, $this->logger);
			$this->structure = $structure;
			$this->entryLength = array_sum($this->structure) + (count($this->structure) - 1) * strlen($this->entryFieldSeparator) + strlen($this->entrySeparator);
		}

		if ($fks) {
			Helper::checkArray("fks", $fks, $this->logger);
			$this->fks = $fks;			
		}

		$this->debug = $debug;
		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Initialized database object');
	}

        private function error($msg) {
            $fp = fopen("php://stderr", "a");
            fwrite($fp, $msg . "\n");
            fclose($fp);
        }

	private function getStructureHeader() {		
		return implode($this->entryFieldSeparator, $this->structure);
	}

	private function getFKHeader() {
		$header = '';
		if (count($this->fks) == 0) {
			return $header;
		}

		for ($i = 0; $i < count($this->fks); $i++) {
			$header .= ($i > 0 ? $this->headerEntrySeparator : '') . implode($this->headerFieldSeparator, $this->fks[$i]);
		}

		return $this->headerSeparator . $header;
	}

	private function parseStructure($str) {	
		$this->headerLength = strlen($str) + strlen($this->entrySeparator); // count separator too
		$this->structure = explode($this->entryFieldSeparator, $str);
		$this->entryLength = array_sum($this->structure) + (count($this->structure) - 1) * strlen($this->entryFieldSeparator) + strlen($this->entrySeparator);
	}

	private function initializeReferencedDbs($fks, $initFks = false) {		
		foreach ($fks as $fk) {
			if ($initFks) {
			    $fk = explode($this->headerFieldSeparator, $fk);
			    $this->fks[] = $fk;
			}

			// initialize referenced db		
			$db = new DatabaseImpl($fk[0]);		
			$db->cacheEntries();		
			$this->referencedDbs[] = $db;
		}
	}

	private function parseFK($str) {
		$this->headerLength += strlen($this->headerSeparator) + strlen($str);
		$fks = explode($this->headerEntrySeparator, $str);
		$this->fks = array();
		
		$this->initializeReferencedDbs($fks, true);
	}

	/**
	 * Loads the entries internally and caches them.
	 */
	public function cacheEntries() {
		if (!file_exists($this->file)) {
			if (!$this->structure) {
				$this->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Structure must be specified for file: " . $this->file);
			}

			touch($this->file);
			
			// write structure header, fk header
			$fp = fopen($this->file, "wb");
			$header = $this->getStructureHeader() . $this->getFKHeader();

			if ($this->fks) {
				$this->initializeReferencedDbs($this->fks);				
			}

			flock($fp, LOCK_EX);
			fwrite($fp, $header);			
			fclose($fp);

			$this->headerLength = strlen($header) + strlen($this->entrySeparator); // count separator too

			$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Created database file');
			$this->entries = array();			

			return;
		}

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loading entries from file..');		
		$fail = false;
		$fp = fopen($this->file, "rb") or $fail = true;
		if ($fail) {
		    $this->logEntry(LoggingImpl::LEVEL_ERROR, "Could not open file: " . $this->file);
		    return;
		}
		while (!flock($fp, LOCK_SH | LOCK_NB)) { $this->logEntry(LoggingImpl::LEVEL_ERROR, "Could not obtain lock on file: " . $this->file); sleep(10); }
		$buf = '';
		while (!feof($fp)) {
			$buf .= fgets($fp, 128);
		}
		fclose($fp);

		// the first few wntries are the headers storing information about the database
		// for now only one header: structure

		// parse structure header
		$ent = explode($this->entrySeparator, $buf);

		$headers = explode($this->headerSeparator, array_shift($ent));
		$this->parseStructure($headers[0]);
		if (count($headers) > 1) {
			$this->parseFK($headers[1]);
		}

		$entries = array();		

		$blank = str_repeat($this->entryFieldSeparator, count($this->structure) - 1);
		/*
		if (strpos($buf, $this->entrySeparator) !== false) {
			$ent = explode($this->entrySeparator, $buf);
		*/	
		$i = 0;
		$atOffset = 0;
		foreach ($ent as $e) {			
			$e = str_replace($this->padCharacter, '', $e);

			// if the entry is made of padded chars, the entry was deleted				
			if ($e != $blank) {
				$entries[$i] = explode($this->entryFieldSeparator, $e);
				$i++;
			} else {
				$this->deletedOffsets[] = $atOffset;
			}

			$atOffset += $this->entryLength;				
		}
		/*	
		} else if ($buf != '') {
			$buf = str_replace($this->padCharacter, '', $buf);
			if (strpos($buf, $this->entryFieldSeparator) !== false) { // just 1 entry with multiple fields
				// if the entry is made of padded chars, the entry was deleted				
				if ($buf != $blank) {
					$entries[0] = explode($this->entryFieldSeparator, $buf);
				} else {
					$this->deletedOffsets[] = 0;
				}
			} else { // just 1 entry with 1 field
				// if the entry is made of padded chars, the entry was deleted				
				if ($buf != $blank) {
					$entries[0] = array($buf);
				} else {
					$this->deletedOffsets[] = 0;
				}
			}
		}
		*/

		$this->entries = $entries;
	}

    /**
     * Saves the cached entries to the db file. Note that this doesn't save deleted offsets, the db is cleanly rewritten.
     */        
	public function saveFile() {		
                if (!is_array($this->entries)) {
		    $this->cacheEntries();
                } else {
                    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
                }		

		$fp = fopen($this->file, "wb+") or $fail = true;
		if ($fail) {
		    $this->logEntry(LoggingImpl::LEVEL_ERROR, "Could not open file: " . $this->file);
		    return;
		}
		while(!flock($fp, LOCK_EX | LOCK_NB)) { $this->logEntry(LoggingImpl::LEVEL_ERROR, "Could not obtain lock on file: " . $this->file); sleep(10); }
		$str = '';

		// write headers
		fwrite($fp, $this->getStructureHeader() . $this->getFKHeader());

		// separator after the structure header, before entries start
		if (count($this->entries) > 0) {
			fwrite($fp, $this->entrySeparator);
		}

		for ($i = 0; $i < count($this->entries); $i++) {
			$entry = $this->getEntry($this->entries[$i]);
			//$str .= 
			fwrite($fp, $entry . ($i < count($this->entries) - 1 ? $this->entrySeparator : ''));
		}
//		$this->cachedBuffer = $str;

//		fwrite($fp, $str);
		fclose($fp);
	}	

	private function getFilePointer() {
		$fp = fopen($this->file, "rb+") or $this->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not open file: " . $this->file);
		   
	        while(!flock($fp, LOCK_EX | LOCK_NB)) { $this->logEntry(LoggingImpl::LEVEL_ERROR, "Could not obtain lock on file: " . $this->file); sleep(10); }
		return $fp;
	}

	private function getEntryOffset($id = 0)
	{	
		if ($id < 0) {
			return false;
		}

		$real_id = $id - count($this->deletedOffsets);
		$max_id = $this->entryCount() - 1;

		if ($real_id > $max_id) {			
			return false;
		}
		return $id * $this->entryLength;
	}

	private function padField($field, $i) {
		$len = $this->structure[$i];
		if (strlen($field) > $len) {
		    return substr($field, 0, $len);
		}
		return str_pad($field, $len, $this->padCharacter);
	}

	private function getEntry($entry) {		
		$buf = '';
		for ($i = 0; $i < count($entry); $i++) {
			$buf .= $this->padField($entry[$i], $i) . ($i < count($entry) - 1 ? $this->entryFieldSeparator : '');
		}		
		return $buf;
	}

	private function getZeroPaddedEntry() {		
		$buf = '';
		for ($i = 0; $i < count($this->structure); $i++) {
			$buf .= str_repeat($this->padCharacter, $this->structure[$i]) . ($i < count($this->structure) - 1 ? $this->entryFieldSeparator : '');
		}
		return $buf;
	}

	public function getRealId($id) {
		$count = 0;
		foreach ($this->deletedOffsets as $offset) {
			$i = $offset / $this->entryLength;
			if ($i < $id) {
				$count++;
			} else if ($i == $id) {
				return -1; // deleted entries do not have a real id
			} else {
				break;
			}			
		}
		return $id - $count;
	}

	public function getPersistentId($id) {		
		foreach ($this->deletedOffsets as $offset) {
			$did = $offset / $this->entryLength;
			if ($did <= $id) {
				$id++;
			}
		}

		return $id;	
	}

	private function addReferencedEntries($entry, &$entries) {
		for ($i = 0; $i < count($this->fks); $i++) {
			$db = $this->referencedDbs[$i];
			$referenceField = $this->fks[$i][1];
			$referencedField = $this->fks[$i][2];

			$ref = null;
			if ($referencedField == -1) {
				// load referenced entry by id
				$id = $entry[$referenceField];				
				$db->loadEntries($ref, $id);				
			} else {
				// load referenced entry by custom field value
				$id = $db->searchEntries(SEARCH_FIELD_OFFSET, $referencedField, $entry[$referenceField]);
				if (is_array($id)) {
					$this->logEntry(LoggingImpl::LEVEL_DEBUG, "Referenced field " . $entry[$referenceField] . " from db " . $db->getFile() . " matched more than 1 entry. Using first.");
					$id = $id[0];
				}

				if ($id != -1) {
					$db->loadEntries($ref, $id);
				}
			}

			if (!is_array($entries["r$i"])) {
				$entries["r$i"] = array();
			}

			if ($ref) {
				// prepend the referenced entry's persistent id
				array_unshift($ref, $id);
			}

			$entries["r$i"][] = $ref;
		}
	}

    /**
     * Reads all the entries from a file onto an array. If id is not null it only loads that specific entry.
     *
     * @param array $entries The array of entries
     * @param int $id The optional id of the entry to fetch. This is the persistent id, not the real id of the entry.
     * @param int $offset The offset to get entries at
     * @param int $size The maximum size of the array; only used when id is null
     */
	public function loadEntries(&$entries, $id = null, $offset = null, $size = null, $sortField = null, $sortOrder = SORT_ASC) {
		if (!is_array($this->entries)) {
			$this->cacheEntries();
		} else {
			$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
		}
	
		if (!is_array($id)) {
			$id = (string)$id; // needed due to comparison to null below
		}
		
		if ($id != null) {
			if (is_array($id)) {				
				$entries = array();
				
				foreach ($id as $i) {
				    // find out how many deleted entries are before the given id, and decrease that number from the id
				    // (using entryLength and deletedOfffsets elements)
				    $real_id = $this->getRealId($i);

                                    if (isset($this->entries[$real_id])) {
                                        $entries[] = $this->entries[$real_id];
					$this->addReferencedEntries($this->entries[$real_id], $entries);					
                                    } else {
                                        $this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to load entry #' . $i . ' , but it did not exist');
                                    }				   
				}
			} else {
			    if (!is_numeric($id)) {
			    	$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to load entry #' . $id . ' , but it did not exist');
				return;
			    }

			    if (is_numeric($offset)) {
				$id += $offset;
			    }

			    // find out how many deleted entries are before the given id, and decrease that number from the id
			    $real_id = $this->getRealId($id);

			    if (isset($this->entries[$real_id])) {
				$entries = $this->entries[$real_id];				
				$this->addReferencedEntries($entries, $entries);				
			    } else {
				$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to load entry #' . $id . ' , but it did not exist');
			    }
			}
		} else {
			if (is_numeric($offset)) {
				if (is_numeric($size)) {
					$entries = array_slice($this->entries, $offset, $size);
				} else {
					$entries = array_slice($this->entries, $offset);
				}
			} else {
				$entries = $this->entries;
			}

			$oldentries = $entries;
			foreach ($oldentries as $entry) {
				$this->addReferencedEntries($entry, $entries);
			}		
		}

		if ($sortField != null) {
		    usort($entries, create_function('$a,$b', 'return !is_numeric($a[' . $sortField . ']) ? ' . ($sortOrder == SORT_ASC ? 1 : -1) . ' * strcmp($a[' . $sortField . '], $b[' . $sortField . ']) : ' . ($sortOrder == SORT_ASC ? 1 : -1) . ' * ($a[' . $sortField . '] == $b[' . $sortField . '] ? 0 : ($a[' . $sortField . '] > $b[' . $sortField . '] ? 1 : -1));'));
		}
	}

	private function insertEntry($i, $entry) {
		// array_splice will treat arrays differently than single elements
		// which is not our intention
		array_splice($this->entries, $i, 0, '');
		$this->entries[$i] = $entry;
	}

	private function addDeletedOffset($pos) {
		for ($i = 0; $i < count($this->deletedOffsets); $i++) {
			if ($pos < $this->deletedOffsets[$i]) {
				array_splice($this->deletedOffsets, $i, 0, $pos);				
				return;
			}
		}

		$this->deletedOffsets[] = $pos;		
	}

	private function checkReference($entry) {
                for ($i = 0; $i < count($this->fks); $i++) {
                        $db = $this->referencedDbs[$i];
                        $referenceField = $this->fks[$i][1];
                        $referencedField = $this->fks[$i][2];

                        $ref = null;
			$id = -1;
                        if ($referencedField == -1) {
                                // load referenced entry by id
                                $id = $entry[$referenceField];
                                $db->loadEntries($ref, $id);
                        } else {
                                // load referenced entry by custom field value
                                $id = $db->searchEntries(SEARCH_FIELD_OFFSET, $referencedField, $entry[$referenceField]);
                                if (is_array($id)) {
                                        $this->logEntry(LoggingImpl::LEVEL_DEBUG, "Referenced field " . $entry[$referenceField] . " from db " . $db->getFile() . " matched more than 1 entry. Using first.");
                                        $id = $id[0];
                                }                               
                        }

                        if (!$ref && $id == -1) {
                        	$this->logEntry(LoggingImpl::LEVERL_WARN, "Referenced field " . $entry[$referenceField] . " from db " . $db->getFile() . " did not match any entries.");        
                        }
                }
	}
	
    /**
     * Saves the entry to a file. If id is not null, it saves that specific entry.
     *
     * @param array $entries The entries to save (an array of field values); if it is a one level array then it's just one entry
     * @param int $id The optional id of the entry. This is the persistent id, not the real id of the entry.
     * @param int $offset The offset to save entries at
     */
	public function saveEntries($entries, $id = null/*, $offset = null*/) {
		Helper::checkArray("entries", $entries, $this->logger);

		if (!is_array($this->entries)) {			
			$this->cacheEntries();
		} else {			
			$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
		}

		$id = (string)$id;
		if ($id != null) {
                        if (!is_numeric($id)) {
                            $this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to save entry #' . $id . ' , but it did not exist');
                            return;
                        }

			/*
			if (is_numeric($offset)) {
				$id += $offset;
			}
			*/

			// find out how many deleted entries are before the given id, and decrease that number from the id
			$real_id = $this->getRealId($id);

			if (isset($this->entries[$real_id])) {				
				$this->checkReference($entries);
				$this->entries[$real_id] = $entries;				
				$pos = $this->getEntryOffset(/*$this->cachedBuffer, */$id);

				if ($pos !== false) {  
					$fp = $this->getFilePointer();
					fseek($fp, $pos + $this->headerLength);
					$entry = $this->getEntry($entries);
					// rwrite the entry to the file
					fwrite($fp, $entry);
					// modify the cached buffer as well
					//$this->cachedBuffer = substr_replace($this->cachedBuffer, $entry, $pos, strlen($entry));

					fclose($fp);
				} else {
					$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to save entry #' . $id . ' , but could not find the separator before it');
					return;
				}				
			} else {
				$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to save entry #' . $id . '(' . $real_id . ') , but it did not exist');
				return;
			}
		} else {
			// create an array containing one entry
			if (!is_array($entries[0])) {
				$entries = array($entries);
			}

			// add a new entry at the end
			// if there is a deleted entry, use that instead
			$fp = $this->getFilePointer();
			
			foreach ($entries as $entry) {
				$this->checkReference($entry);
				$ent = $this->getEntry($entry);
				if (count($this->deletedOffsets) > 0) {
					// replace a deleted entry
					$offset = array_shift($this->deletedOffsets);
					$id = $offset / $this->entryLength;
					$id = floor($id);
					// NOTE: this id is correct, because the deletedOffsets are sorted in ascending order
					// so there will be no deleted entries before this one

					$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Replacing deleted entry with id #' . $id);

					fseek($fp, $offset + $this->headerLength);
					fwrite($fp, $ent);
					$this->insertEntry($id, $entry);
				} else {
					// add at the end
					fseek($fp, 0, SEEK_END);
					fwrite($fp, $this->entrySeparator . $ent); // entry separator is always written because of headers
					$this->entries[count($this->entries)] = $entry;					
				}			
			}

			fclose($fp);
		}

		//$this->saveFile(); // save changes to db file
	}

    /**
     * Deletes an entry with the specified id.
     *
     * @param int $id The optional id of the entry to delete. This is the persistent id, not the real id of the entry.
     */
	public function deleteEntry($id = null) {
		if (!file_exists($this->file)) {
			$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to delete an entry, but db file ' . $this->file . ' did not exist');
			return;
		}

		if (!is_array($this->entries)) {
			$this->cacheEntries();
		} else {
			$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
		}

		$id = (string)$id;
		if ($id == null) {
			$fp = $this->getFilePointer();
			ftruncate($fp, $this->headerLength - strlen($this->entrySeparator)); // subtract entry separator length, since we cleared all entries
			fclose($fp);
			// clear all entries and deleted offsets
			$this->entries = array();
			$this->deletedOffsets = array();
		} else {
                        if (!is_numeric($id)) {
                            $this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to delete entry #' . $id . ' , but it did not exist');
                            return;
                        }

			$real_id = $this->getRealId($id); // decrease id by number of deleted entries before it
			if (isset($this->entries[$real_id])) {
				$this->checkReference($this->entries[$real_id]);
				// remove the entry with the specified id
				$pos = $this->getEntryOffset(/*$this->cachedBuffer, */$id);
				if ($pos !== false) {
					$fp = $this->getFilePointer();
					fseek($fp, $pos + $this->headerLength);

					for ($i = $real_id; $i < count($this->entries) - 1; $i++) {
						$this->entries[$i] = $this->entries[$i + 1];
					}
				
					array_splice($this->entries, count($this->entries) - 1, 1); // remove last entry				

					$entry = $this->getZeroPaddedEntry();
					fwrite($fp, $entry);
//					$this->cachedBuffer = substr_replace($this->cachedBuffer, $entry, $pos, strlen($entry));			
					$this->addDeletedOffset($pos);

					fclose($fp);
				} else {
					$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to delete entry #' . $id . ' , but could not find separator before it');
					return;
				}
			} else {
				$this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to delete entry #' . $id . '(' . $real_id . ') , but it did not exist');
				return;
			}
		}

		//$this->saveFile(); // save changes to db file
		if ($id == null) {
		    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Deleted all entries');
		} else { 
		    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Deleted entry #' . $id);
		}
	}


    public function mergeEntries($canBeReplaced = array(), $primaryKeyIndex = 0, &$entries, $commit = false) {
        Helper::checkArray("entries", $entries, $this->logger);

        if (!is_array($this->entries)) {
	    $this->cacheEntries();
	} else {
	    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
	}

	if (!is_array($canBeReplaced)) {
	    $canBeReplaced = array();
	}

        // we don't actually modify the entries, it's up to the external caller to whether they want to save the entries or not
        $entriesCopy = $this->entries;
	
        foreach ($entries as $entry) {
	    $found = false;

	    // see if the entry already exists, if so replace its fields that can be replaced
	    for ($i = 0; $i < count($entriesCopy); $i++) {
	        if ($entriesCopy[$i][$primaryKeyIndex] == $entry[$primaryKeyIndex]) {
		    // modify entry
                    foreach ($canBeReplaced as $replace) {
                        $entriesCopy[$i][$replace] = $entry[$replace];
                    }
		    $found = true;
		    $this->checkReference($entriesCopy[$i]);

		    $id_persistent = $this->getPersistentId($i); // get the persistent id from the real id
		    if ($commit) {		        
  		        $pos = $this->getEntryOffset(/*$this->cachedBuffer, */$id_persistent);
                        if ($pos !== false) {			    
                            if (!$fp) $fp = $this->getFilePointer();
                            fseek($fp, $pos + $this->headerLength);

                            $ent = $this->getEntry($entriesCopy[$i]);
                            // rwrite the entry to the file			   
                            fwrite($fp, $ent);
                            // modify the cached buffer as well
//                            $this->cachedBuffer = substr_replace($this->cachedBuffer, $ent, $pos, strlen($ent));
                                       
                            $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Merge resulted in entry #' . $id_persistent . ' being modified');                        
                        } else {
                            $this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to save entry #' . $id_persistent . ' , but could not find the separator before it');
                        }
		    } else {
		        $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Merge resulted in entry #' . $id_persistent . ' being modified');
		    }

		    break;
		}
	    }

            // not found, add a new entry then
	    if (!$found) {
	        $entriesCopy[] = $entry;
		$this->checkReference($entry);
		if ($commit) {		
		    if (!$fp) $fp = $this->getFilePointer();

                    $ent = $this->getEntry($entry);
                    if (count($this->deletedOffsets) > 0) {
                        $offset = array_shift($this->deletedOffsets);
                        fseek($fp, $offset + $this->headerLength);
                        fwrite($fp, $ent);
//                        $this->cachedBuffer = substr_replace($this->cachedBuffer, $ent, $offset, strlen($ent));
                    } else {
                        fseek($fp, 0, SEEK_END);
                        fwrite($fp, $this->entrySeparator . $ent);
//                        $this->cachedBuffer .= ($this->cachedBuffer != '' ? $this->entrySeparator : '') . $ent;
                    }
		}
                
		// add new entry
		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Merge resulted in new entry being added');
	    }
	}

	// now delete any old entries
	$newEntries = array();	
	for ($i = 0; $i < count($entriesCopy); $i++) {
	    $found = false;
	    foreach ($entries as $entry) {
	        if ($entriesCopy[$i][$primaryKeyIndex] == $entry[$primaryKeyIndex]) {
		    $found = true;
		    break;
		}
	    }

            // only delete an entry if it is not found in the new array
	    if ($found) {
	        $newEntries[] = $entriesCopy[$i];
	    } else {	    
	    	// delete entry
		$this->checkReference($entriesCopy[$i]);
		$id_persistent = $this->getPersistentId($i); // get persistent id from real id
		if ($commit) {		    
                    $pos = $this->getEntryOffset(/*$this->cachedBuffer, */$id_persistent);
                    if ($pos !== false) {
		        if (!$fp) $fp = $this->getFilePointer();

                        $entry = $this->getZeroPaddedEntry();		    

		        fseek($fp, $pos + $this->headerLength);
                        fwrite($fp, $entry);
//                        $this->cachedBuffer = substr_replace($this->cachedBuffer, $entry, $pos, strlen($entry));
			
                        $this->addDeletedOffset($pos);
			
 	                $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Merge resulted in entry #' . $id_persistent . ' being deleted');			
		    } else {
		        $this->logEntry(LoggingImpl::LEVEL_WARN, 'Tried to delete entry #' . $id_persistent . ' , but could not find separator before it');		
		        $newEntries[] = $entriesCopy[$i];
		    }
		} else {
  		    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Merge resulted in entry #' . $id_persistent . ' being deleted');
		}
	    }
	}

	if ($fp) {
		fclose($fp);
	}

	$entries = $newEntries;
	if ($commit) {
		$this->entries = $newEntries;			
	}	
    }

    private function addReferencedFields(&$entry) {
    	// add fields from referenced entries
        for ($i = 0; $i < count($this->fks); $i++) {
 	    $db = $this->referencedDbs[$i];
            $referenceField = $this->fks[$i][1];
            $referencedField = $this->fks[$i][2];

            $ref = null;
            if ($referencedField == -1) {
                // load referenced entry by id
                $id = $entry[$referenceField];
                $db->loadEntries($ref, $id);
            } else {
                // load referenced entry by custom field value
                $id = $db->searchEntries(SEARCH_FIELD_OFFSET, $referencedField, $entry[$referenceField]);
                if (is_array($id)) {
                    $this->logEntry(LoggingImpl::LEVEL_DEBUG, "Referenced field " . $entry[$referenceField] . " from db " . $db->getFile() . " matched more than 1 entry. Using first.");
                    $id = $id[0];
                }

                if ($id != -1) {
                    $db->loadEntries($ref, $id);
                }
            }          

            if ($ref) {
                // append the referenced entry fields at the end of the parent entry
		foreach ($ref as $field) {
			array_push($entry, $field);
		}
            }
        }
    }
    
    /**
     * Searches, using given options, for entries that contain the given fields, and returns their ids.
     */
    public function searchEntries() {
        $args = func_get_args();
        Helper::checkArray("args", $args, $this->logger);

        if (!is_array($this->entries)) {
	    $this->cacheEntries();
	} else {
	    $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
	}

	$options = $args[0];
	if (!is_array($this->entries) || count($this->entries) == 0) {
	    return $options & SEARCH_ARRAY ? array() : -1;
	}
	
	$argIndex = 1;
	$offset = 0;
	// parse options
	if ($options & SEARCH_FIELD_OFFSET) {
	    $argIndex = 2;
	    $offset = $args[1];
	}
        
        $matches = array();
        $index = 0;

        // test each entry against the search arguments
	$bkup = $args; // backup args
        foreach ($this->entries as $entry) {
	    $args = $bkup; // restore args
	    if ($options & SEARCH_REFERENCED_ENTRIES) {
	    	$this->addReferencedFields($entry);
	    }

            if ( (($options & SEARCH_FIELD_OFFSET) && (count($entry) < count($args) - 2)) ||
	        count($entry) < count($args) - 1) {
                return $options & SEARCH_ARRAY ? array() : -1;
            }
            
            $match = true;
            for ($i = $argIndex; $i < count($args); $i++) {
		if (is_array($args[$i])) {
			// if field is an array, then we treat its values as possible matches, so we try
			// all of them until we get a good match
			$array = $args[$i];
		} else {
			$array = array($args[$i]);
		}

		foreach ($array as $args[$i]) {		
	            // compare the argument against the entry field
		    // if argument is null, we don't have to compare
		    if ($args[$i] == null && $args[$i] != '0') {
		        continue;
	 	    }

	    	    if ($options & SEARCH_COMPARE) {		
		        if ($args[$i][0] == '<') {
		            if ($args[$i][1] == '=') {
			        $num = substr($args[$i], 2);
                                $match = $entry[$offset + $i - $argIndex] <= $num;
				if ($match) {
				    break;
				}
			    } else {
		                $num = substr($args[$i], 1);
		                $match = $entry[$offset + $i - $argIndex] < $num;
				if ($match) {
				    break;
				}
			    }
		        } else if ($args[$i][0] == '>') {
		            if ($args[$i][1] == '=') {
                                $num = substr($args[$i], 2);
			        $match = $entry[$offset + $i - $argIndex] >= $num;
				if ($match) {
				    break;
				}
			    } else {
		                $num = substr($args[$i], 1);
                                $match = $entry[$offset + $i - $argIndex] > $num;
				if ($match) {
				    break;
				}
			    }
		        } else {		      		        
		            $match = $args[$i] == $entry[$offset + $i - $argIndex];				
			    if ($match) {
			        break;
			    }
		        }		    
		    } else {                   
                        $match = $args[$i] == $entry[$offset + $i - $argIndex];                   
		        if ($match) {
		            break;
		        }
	            }		    
		}

		if (!$match) {
		    break;
		}
            }
            
            if ($match) {		
	    	// get persistent id from real id
		if (!($options & SEARCH_REAL_ID)) {			
			$id_persistent = $this->getPersistentId($index);
			if ($options & SEARCH_RESULTS_MAPPED) {
			    // map result depending on the value of the argument			    
			    $matches[$entry[$offset]][] = $id_persistent;
			} else {
                	    $matches[] = $id_persistent;
			}
		} else {
			if ($options & SEARCH_RESULTS_MAPPED) {
				// map result depending on the value of the argument
				$matches[$entry[$offset]][] = $index;
			} else {
				$matches[] = $index;
			}
		}
            }
            $index++;
        }

	if ($options & SEARCH_RESULTS_MAPPED) {
		return $matches;
	}
        
        // return the results
        if (count($matches) == 0) {
            return $options & SEARCH_ARRAY ? array() : -1;
        } else if (count($matches) == 1) {
            return $options & SEARCH_ARRAY ? $matches : $matches[0];
        } else if (count($matches) > 1) {
            return $matches;   
        }
    }        

    public function getReferencedDbs() {
    	return $this->referencedDbs;
    }

    /**
     * Returns the number of entries in the file.
     *
     * @return int The number of entries in the file.
     */
        public function entryCount() {
		if (!is_array($this->entries)) {
			$this->cacheEntries();
		} else {
			$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Loaded entries from cache');
		}

		if (!is_array($this->entries)) return 0;
		return count($this->entries);
	}

    /**
     * Gets the name of the database file.
     *
     * @return string The name of the database file
     */
	public function getFile() {
		return $this->file;
	}

    /**
     * Sets the name of the database file.
     *
     * @param string $file The name of the database file
     */
	public function setFile($file) {
        	Helper::checkString("file", $file, $this->logger);

		$this->file = $file;
		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set file name to ' . $file);
	}

	private function recalculateHeaderLength() {
		$header = $this->getStructureHeader() . $this->getFKHeader();
		$this->headerLength = strlen($header) + strlen($this->entrySeparator);
	}

    /**
     * Sets the entry field separator.
     *
     * @param string $separator The entry field separator
     */
	public function setEntryFieldSeparator($separator) {
        	Helper::checkString("separator", $separator, $this->logger);

		$this->entryFieldSeparator = $separator;
	        $this->entryLength = array_sum($this->structure) + (count($this->structure) - 1) * strlen($this->entryFieldSeparator) + strlen($this->entrySeparator);
		$this->recalculateHeaderLength();

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set entry field separator to ' . $separator);
	}

    /**
     * Sets the entry separator.
     *
     * @param string $separator The entry separator
     */
	public function setEntrySeparator($separator) {
		Helper::checkString("separator", $separator, $this->logger);

		$this->entrySeparator = $separator;
       	        $this->entryLength = array_sum($this->structure) + (count($this->structure) - 1) * strlen($this->entryFieldSeparator) + strlen($this->entrySeparator);
		$this->recalculateHeaderLength();

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set entry separator to ' . $separator);
	}

	public function setHeaderSeparator($separator) {
		Helper::checkString("separator", $separator, $this->logger);

		$this->headerSeparator = $separator;
		$this->recalculateHeaderLength();

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set header separator to ' . $separator);		
	}

	public function setHeaderEntrySeparator($separator) {
		Helper::checkString("separator", $separator, $this->logger);

		$this->headerEntrySeparator = $separator;
		$this->recalculateHeaderLength();

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set header entry separator to ' . $separator);		
	}

	public function setHeaderFieldSeparator($separator) {
		Helper::checkString("separator", $separator, $this->logger);

		$this->headerFieldSeparator = $separator;
		$this->recalculateHeaderLength();

		$this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set header field separator to ' . $separator);
	}

    /**
     * Sets the pad character.
     *
     * @param string $character The pad character
     */
        public function setPadCharacter($character) {
                Helper::checkString("character", $character, $this->logger);

                $this->padCharacter = $character;
                $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Set pad character to ' . $character);
        }

    /**
     * Gets the logger object.
     *
     * @return Logging The logger object
     */ 
	public function getLogger() {
		return $this->logger;
	}

    /**
     * Sets the logger object to use.
     *
     * @param Logging $logger The logger object to use
     */
	public function setLogger($logger) {
		Helper::checkNull("logger", $logger, $this->logger);

		$this->logger = $logger;
	}
    
    /**
     * Generates a snapshot of the database with the given name.
     * 
     * @param string $name The name of the snapshot
     */
    public function snapshot($name) {
	if (!file_exists($this->file)) {
            $this->logEntry(LoggingImpl::LEVEL_DEBUG, 'Tried to make a snapshot but database file was missing');
	    return;
	}

        copy($this->file, $name);
    }    

    private function logEntry($lvl, $msg) {
        if ($this->debug || !($lvl & LoggingImpl::LEVEL_DEBUG)) {
            $this->logger->logEntry($lvl, "(" . $this->file .") " . $msg);
	}
    }
}

?>
