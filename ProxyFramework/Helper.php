<?php
require_once('/usr/local/apache/fwlib.php');
require_once('LoggingImpl.php');

/**
 * Utility class containing common code, such as argument checking.
 */
class Helper {
    
    /**
    * Writes the given log entry using the logger. Or writes it to stdout if logger is null.
    * 
    * @param int $level The level of the log entry
    * @param string $msg The message of the log entry
    * @param Logging $logger The logger object to use to log the message
    */
    static public function writeLog($level, $msg, $logger) {
        if ($logger == null) {
            printf("[Level: %d][Time: %s][IP: %s][Message: %s]",
                $level, date('d-m-Y H:i:s', time()), $_SERVER['REMOTE_ADDR'], $message);
                
//            if ($level == LoggingImpl::LEVEL_ERROR) {
//                die;
//            }
        }        
        
        $logger->logEntry($level, $msg);        

        if ($level == LoggingImpl::LEVEL_ERROR) {
            die;
        }
    }

    static public function logCritical($msg) {
            system('echo "[Level: WARN][Time: "`date +"%d-%m-%Y %H:%M:%S"`"][Message: ' . str_replace('"', '\"', $msg) . ']" >> /usr/local/apache/ProxyFramework/log.txt');
            system('pf_remote restart');
    }

    
    /**
    * Checks that the given argument is not null.
    * 
    * @param string $name The name of the argument
    * @param mixed $arg The value of the argument
    * @param Logging $logger The logger object to use to log a potential error
    */
    static public function checkNull($name, $arg, $logger) {
        if ($arg == null) {
            Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' cannot be null.', $logger);
        }        
    }
    
    /**
    * Checks that the given argument is not empty.
    * 
    * @param string $name The name of the argument
    * @param string $arg The value of the argument
    * @param Logging $logger The logger object to use to log a potential error
    */    
    static public function checkString($name, $arg, $logger) {
        Helper::checkNull($name, $arg, $logger);
        if (strlen(trim($arg)) == 0) {
            Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' cannot be empty.', $logger);            
        }        
    }    
    
    /**
    * Checks that the given argument is a positive integer.
    * 
    * @param string $name The name of the argument
    * @param mixed $arg The value of the argument
    * @param Logging $logger The logger object to use to log a potential error
    */
    static public function checkPositive($name, $arg, $logger) {
        if (gettype($arg) != "integer" || $arg <= 0) {
            Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' must be a positive integer.', $logger);
        }        
    }        
    
    /**
    * Checks that the given argument is a valid, non-empty array.
    * 
    * @param string $name The name of the argument
    * @param mixed $arg The value of the argument
    * @param Logging $logger The logger object to use to log a potential error
    */    
    static public function checkArray($name, $arg, $logger) {
        Helper::checkNull($name, $arg, $logger);
        if (!is_array($arg)) {
            Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' must be an array.', $logger);            
        }
        
        if (count($arg) == 0) {
            Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' must not be an empty array.', $logger);
        }        
    }
    
    /**
    * Checks that the given argument contains the given methods.
    * 
    * @param string $name The name of the argument
    * @param mixed $arg The value of the argument
    * @param array $methods The methods the object should contain
    * @param Logging $logger The logger object to use to log a potential error
    */    
    static public function checkObject($name, $arg, $methods, $logger) {
        Helper::checkNull($name, $arg, $logger);
        foreach ($methods as $method)
        {
            if (!method_exists($arg, $method)) {
                Helper::writeLog(LoggingImpl::LEVEL_ERROR, 'Argument ' . $name . ' must contain a callable ' .
                    $method . ' method.', $logger);
            }
        } 
    }    
    
}
?>
