<?php
/**
 * This interface defines the bare minimum for a database structure.
 */
interface Database {

	/**
	 * Loads the entries internally and caches them.
	 */
	public function cacheEntries();

	/**
	 * Saves the cached entries to the db file.
	 */
	//public function saveFile();
    
    /**
     * Reads all the entries from a file onto an array. If id is not null it only loads that specific entry.
     * 
     * @param array $entries The array of entries
     * @param int $id The optional id of the entry to fetch
     * @param int $offset The offset to get entries at
     * @param int $size The maximum size of the array; only used when id is null
     */
    public function loadEntries(&$entries, $id = null, $offset = null, $size = null);    

	/**
	 * Saves the entry to a file. If id is not null, it saves that specific entry.
     * 
	 * @param array $entries The entries to save (an array of field values); if it is a one level array then it's just one entry
	 * @param int $id The optional id of the entry	 
	 */
	public function saveEntries($entries, $id = null);

	/**
	 * Deletes an entry with the specified id.
	 *
	 * @param int $id The optional id of the entry to delete	 
	 */
	public function deleteEntry($id = null);
    
    /**
     * Searches for entries that contain the given fields, and returns their ids.
     */
    public function searchEntries();

	/**
	 * Returns the number of entries in the file.
	 *
     * @return int The number of entries in the file.
	 */
	public function entryCount();

	/**
	 * Gets the name of the database file.
	 *
	 * @return string The name of the database file
	 */
	public function getFile();

	/**
	 * Sets the name of the database file.
	 *
	 * @param string $file The name of the database file
	 */
	public function setFile($file);

	/**
	 * Sets the entry field separator.
	 *
	 * @param string $separator The entry field separator
	 */
	public function setEntryFieldSeparator($separator);

	/**
	 * Sets the entry separator.
	 *
	 * @param string $separator The entry separator
	 */
	public function setEntrySeparator($separator);

	/**
	 * Gets the logger object.
	 *
	 * @return Logging The logger object
	 */  
	public function getLogger();

	/**
	 * Sets the logger object to use.
	 *
	 * @param Logging $logger The logger object to use
	 */
	public function setLogger($logger);
    
    /**
     * Generates a snapshot of the database, with the given name.
     * 
     * @param string $name The name of the snapshot
     */
    public function snapshot($name);
}

?>
