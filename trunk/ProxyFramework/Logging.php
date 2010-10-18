<?php
/**
 * This interface defines the bare minimum for a logging structure.
 */
interface Logging {

	/**
	 * Logs a message entry to the file, and depending on its level may or may not display it to stdout.
	 *
	 * @param int $level Log message level
	 * @param string $message The actual message to log
	 */
	public function logEntry($level, $message);

	/**
	 * Sets the name of the log file.
	 *
	 * @param string $file The name of the log file
	 */
	public function setFile($file);

	/**
	 * Sets the log entry format.
	 *
	 * @param string $format The log entry format; cannot be null or empty
	 */
	public function setLogFormat($format);

	/**
	 * Sets the time format.
	 *
	 * @param string $format The time format; cannot be null or empty
	 */
	public function setTimeFormat($format);
}

?>