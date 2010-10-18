<?php
require_once('Logging.php');
require_once('Helper.php');

/**
 * This class implements the Logging interface.
 */
class LoggingImpl implements Logging {
    
   /**
    * Info level.
    */
    const LEVEL_INFO = 1;

   /**
    * Warn level.
    */
    const LEVEL_WARN = 2;

   /**
    * Error level.
    */
    const LEVEL_ERROR = 4;

   /**
    * Debug level.
    */
    const LEVEL_DEBUG = 8;    

    const LEVEL_DIE = 16;
       
	/**
	 * The file to log to. It is initially set to 'log.txt' and can be changed via its setter.
	 */
	private $file = 'log.txt';

	/**
	 * The format of each log entry. Can be changed via its setter.
	 */
	private $logFormat = '[Level: %s][Time: %s][Message: %s]';

	/**
	 * The format of the timestamp used in each log entry. Can be changed via its setter.
	 */
	private $timeFormat = 'd-m-Y H:i:s';

	/**
	 * Default constructor.
	 */
	function LoggingImpl($file = 'log.txt') {
	    $this->file = $file;	
	}
	
	private function error($msg) {
		$fp = fopen("php://stderr", "a");
		fwrite($fp, $msg . "\n");
		fclose($fp);
	}

	/**
	 * Logs a message entry to the file, and depending on its level may or may not display it to stdout or stop execution.
	 *
	 * @param int $level Log message level
	 * @param string $message The actual message to log
	 */
	public function logEntry($level, $message) {
		$fileCreate = false;
		if (!file_exists($this->file)) {
			$fileCreate = true;
		}
		$fp = fopen($this->file, $fileCreate ? "w+" : "a") or $fail = true;
		//die('Could not open file: ' . $this->file);
		if ($fail) {
			$this->error("could not open log file " . $this->file . "\n");
			return;
		}
		while (!flock($fp, LOCK_EX | LOCK_NB)) { sleep(10); $this->error("waiting for lock on " . $this->file . "..."); } //or die("Could not obtain lock on file: " . $this->file);

		if ($level & self::LEVEL_INFO) {
			// info level does not print anything
			fprintf($fp, $this->logFormat."\r\n", 'INFO', date($this->timeFormat, time()), $message);
                        printf($this->logFormat."\n", 'INFO', date($this->timeFormat, time()), $message);
		} else if ($level & self::LEVEL_DEBUG) {
			// debug level does not print anything
			fprintf($fp, $this->logFormat."\r\n", 'DEBUG', date($this->timeFormat, time()), $message);
		} else if ($level & self::LEVEL_WARN) {
			// warn level prints to stdout
			fprintf($fp, $this->logFormat."\r\n", 'WARN', date($this->timeFormat, time()), $message);
			printf($this->logFormat."\n", 'WARN', date($this->timeFormat, time()), $message);
		} else if ($level & self::LEVEL_ERROR) {
			// error level prints to stdout in bold and stops execution of the script
			fprintf($fp, $this->logFormat."\r\n", 'ERROR', date($this->timeFormat, time()), $message);
			printf($this->logFormat."\n", 'ERROR', date($this->timeFormat, time()), $message);
		}

		fclose($fp);

		if ($level & self::LEVEL_DIE) {
			die;
		}
	}

	/**
	 * <p>
	 * Sets the name of the log file.
	 * </p>
	 * @param string $file The name of the log file; cannot be null or empty
	 */
	public function setFile($file) {
		Helper::checkString("file", $file, null);

		$this->file = $file;
	}

	/**
	 * Sets the log entry format.
	 *
	 * @param string $format The log entry format; cannot be null or empty
	 */
	public function setLogFormat($format) {
		Helper::checkString("format", $format, null);

		$this->logFormat = $format;
	}

	/**
	 * Sets the time format.
	 *
	 * @param string $format The time format; cannot be null or empty
	 */
	public function setTimeFormat($format) {
		Helper::checkString("format", $format, null);

		$this->timeFormat = $format;
	}
}

?>
