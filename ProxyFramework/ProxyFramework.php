<?php
require_once('DatabaseImpl.php');
require_once('ProxyModule.php');
require_once('UniqueIPDatabase.php');

/**
 * Needed so that the signal handler works properly.
 */
declare(ticks = 1);

define("__STARTER_MEM", 1024*1024);
define("__ALLOCATE_EXTRA", 1024*1024);

define("__INDEX_DATA", 0);
define("__INDEX_PIDS", 1);
define("__INDEX_CHECKING_THREAD_COUNT", 2);
define("__INDEX_UDATA", 3);

define("PIDFILE", "framework.pid");
define("UID", 65534);
define("GID", 65534);

// lock block states
define("STATE_NOTLOCKED", 0);
define("STATE_LOCKED", 1);
define("STATE_SHUTDOWNINLOCK", 2);
define("STATE_LOCKED_CT", 4);
define("STATE_LOCKED_PID", 8);

class ProxyFramework {

    private $modules = array();
    
    private $logger;
    
    private $proxyChecker;
    
    private $snapshotInterval;          

    /* begin shared memory variables */	

    private $keys;

    private $ids;
    
    private $mutexes;

    /* end shared memory variables */
    
    private $lastSnapshot = 0;
    
    private $proxyCheckInterval;
    
    private $quarantineCheckInterval;

    private $checkingThreadCap;

    private $maxQuarantineRetries;

    static private $pidFile;
    
    static private $instance;

    static private $mainPid;

    private $lockBlockState = STATE_NOTLOCKED;
    
    /**
     * Initializes logger and database.
     */
    function ProxyFramework($dbFile, $proxyChecker, $snapshotInterval, $proxyCheckInterval,
        $quarantineCheckInterval, $checkingThreadCap, $maxQuarantineRetries, $pidFile) {
        $this->logger = new LoggingImpl();
        Helper::checkString("dbFile", $dbFile, $this->logger);        
        Helper::checkNull("proxyChecker", $proxyChecker, $this->logger);        
        
        $db = new DatabaseImpl($dbFile, array(100, 5, 15, 3, 13, 1, 1, 1, 1, 10, 6, 6, 4));
	$db->cacheEntries(); //preemptively cache entries to make sure the db state is the same across all threads

	$uniqueIP = new UniqueIPDatabase();

	// set up mutexes and shared data, as well as the signal handler
	$this->init();
	
        $this->setVar(__INDEX_DATA, $db);		
        $this->setVar(__INDEX_PIDS, array());
	$this->setVar(__INDEX_CHECKING_THREAD_COUNT, 0);
	$this->setVar(__INDEX_UDATA, $uniqueIP);

        $this->proxyChecker = $proxyChecker;
        $this->snapshotInterval = $snapshotInterval;
        $this->proxyCheckInterval = $proxyCheckInterval;
        $this->quarantineCheckInterval = $quarantineCheckInterval;
	$this->checkingThreadCap = $checkingThreadCap;
	$this->maxQuarantineRetries = $maxQuarantineRetries;	

	ProxyFramework::$pidFile = $pidFile;
        ProxyFramework::$instance = $this;
	ProxyFramework::$mainPid = posix_getpid();
    }
    
    public function addModule($module) {
        Helper::checkObject("module", $module, array("getName"), $this->logger);
        
        $this->modules[$module->getName()] = $module;
    }
    
    public function removeModule($module) {
        Helper::checkObject("module", $module, array("getName"), $this->logger);
               
        unset($this->modules[$module->getName()]);
    }
    
    public function getLoadedModules() {
        $names = array();
        
        foreach ($this->modules as $key => $module) {
            if ($modules != null) {
                $names[] = $key;
            }
        }
        
        return $names;
    }
    
    protected function fireProxyRetrievalDoneEvent($proxies) {
        // 1. for these proxies, filter out the duplicate ones (based on db)        
        // 2. then for the remaining, unique ones, perform checking
        // using the proxy checker and fire the proxy checking done event when done
                	
        // 1 
        $data = $this->getVar(__INDEX_DATA);
	$data->loadEntries($dbProxies);
	if ($dbProxies == null) {
	    $dbProxies = array();
	}

	$pids = array();
        foreach ($proxies as $proxy) {
            if (strpos($proxy, ":") !== false) {
                list($ip, $port) = explode(":", $proxy);
                $exists = false;
                foreach ($dbProxies as $existingProxy) {
                    if ($existingProxy[0] == $ip && $existingProxy[1] == $port) {
                        $exists = true;
                        $break;
                    }
                }
            	    
                if (!$exists) {
                    // 2

		    // check thread count; if cap been reached sleep and check again till it's good
		    // must lock here, and unlock also here if count was bad; or unlock after launching thread
		    // if count was good		    
		    $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
		    $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);		    	
		    if ($count >= $this->checkingThreadCap) {		       
			do {
			    $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]); // release previous lock
			    usleep(50000);  // free cpu cycles

			    // grab lock again, and check again
			    $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
			    $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
			} while ($count >= $this->checkingThreadCap);
		    }

                    // increase thread count, unlock
	            $this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count + 1, false);		    
		    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "INCREASING THREAD COUNT TO ".($count + 1));
		    $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);

		    // thread for each
		    $pid = pcntl_fork();
		    if ($pid == -1) {
		        $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
		    } else if (!$pid) {		   
	                $this->fireProxyCheckingDoneEvent($this->proxyChecker->check($ip, $port));

			// checking is done when it gets to this point
		        // decrease checking thread count (lock/unlock)
			$this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
			$count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
			$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "DECREASING THREAD COUNT TO ".($count - 1));
                        $this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count - 1, false);
			$this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);

			die; // end thread
		    } else {
		        $pids[] = $pid;
		    	$this->addPid($pid);			
		    }		    
                }
            }
        }

        $this->waitPids($pids);
    }

    protected function waitPids(&$pids, $main_thread = false) {
        while (count($pids) > 0) {
	    $pid = pcntl_wait($status);
            // remove it from existing pids
	    $newpids = array();
	    
	    foreach ($pids as $p) {
	        if ($p != $pid) {
		    $newpids[] = $p;
		}

	    }
            
	    $pids = $newpids;
            // remove the pid from the mem segment too
	    $this->removePid($pid, $main_thread);	    
        }
    }
    
    protected function fireProxyCheckingDoneEvent($proxyData) {
        $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Checking done.");

        // if proxyData != null it means the proxy passed checking
        if ($proxyData == null) {
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Checking result was negative.");
            return;
        }
        
        // we must also check if shownHost is duplicate in the db
        // lock this block, because this is where we commit to the db
        $this->lock();

	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Checking result was positive.");
        
        $data = $this->getVar(__INDEX_DATA, false);        
	$udata = $this->getVar(__INDEX_UDATA, false);	
	$data->loadEntries($dbProxies);
        if ($dbProxies == null) {
	    $dbProxies = array();
	}
        foreach ($dbProxies as $existingProxy) {
            if (($proxyData->host == $existingProxy[0] && $proxyData->port == $existingProxy[1]) ||
                $proxyData->shownHost == $existingProxy[2])
            {
                $this->unlock();
                return;
            }    
        }

	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Found proxy is NEW!");
        
        // if it's not add it to db, using proxyData->host, port, shownHost, country, responseTime,
        // sslSupport, anonimityLevel, type(proxy/socks4/socks5) quarantined,
        // last checked ts, totalChecks, totalPassed, quarantineRetries
        $proxy = array($proxyData->host, $proxyData->port,
            $proxyData->shownHost, $proxyData->country, $proxyData->responseTime,
            $proxyData->sslSupport, $proxyData->anonimityLevel,
            $proxyData->type, 0, time(), 1, 1, 0);

	$udata->addProxy($proxyData->host);	    
        $this->setVar(__INDEX_UDATA, $udata, false);

	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, print_r($proxy, true));
	
        $data->saveEntries($proxy);
       
        $this->setVar(__INDEX_DATA, $data, false);        
        $this->unlock();	
    }   
    
    protected function init() {        
	$this->keys = array(
	    __INDEX_DATA => 0xffa,
	    __INDEX_PIDS => 0xffb,
	    __INDEX_CHECKING_THREAD_COUNT => 0xffc,
	    __INDEX_UDATA => 0xffd
	);
	
        $this->mutexes = array(
            __INDEX_DATA => sem_get($this->keys[__INDEX_DATA]),
            __INDEX_PIDS => sem_get($this->keys[__INDEX_PIDS]),
            __INDEX_CHECKING_THREAD_COUNT => sem_get($this->keys[__INDEX_CHECKING_THREAD_COUNT])	    
	);      

        $this->ids = array(
	    __INDEX_DATA => shmop_open($this->keys[__INDEX_DATA], "c", 0644, __STARTER_MEM),
	    __INDEX_PIDS => shmop_open($this->keys[__INDEX_PIDS], "c", 0644, __STARTER_MEM),
	    __INDEX_CHECKING_THREAD_COUNT => shmop_open($this->keys[__INDEX_CHECKING_THREAD_COUNT], "c", 0644, 1024),
	    __INDEX_UDATA => shmop_open($this->keys[__INDEX_UDATA], "c", 0644, __STARTER_MEM)
	);
       
	// register signal handler
        pcntl_signal(SIGTERM, "ProxyFramework::handler");
        pcntl_signal(SIGINT, "ProxyFramework::handler");	
    }    
    
    protected function lock($mutex = null) {
        if ($mutex == null) {
            sem_acquire($this->mutexes[__INDEX_DATA]);
        } else {
            sem_acquire($mutex);
        }
    }
    
    protected function unlock($mutex = null) {
        if ($mutex == null) {
            sem_release($this->mutexes[__INDEX_DATA]);     
        } else {
            sem_release($mutex);
        }
    }
    
    protected function getVar($index, $lock = true) {

        if ($lock) {
            $this->lock($this->mutexes[$index]);
        }
        
	$data = shmop_read($this->ids[$index], 0, shmop_size($this->ids[$index]));	
        
        if ($lock) {
            $this->unlock($this->mutexes[$index]);
        }
        
        return unserialize($data);
    }

    protected function memSize($object) {
        return (strlen($object) + 44) * 2;
    }
    
    protected function setVar($index, $variable, $lock = true) {	
        if ($lock) {        
            $this->lock($this->mutexes[$index]);
        }                

	$str = serialize($variable);
       
        if (strlen($str) != shmop_write($this->ids[$index], $str, 0)) {
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Ran out of shared memory. Allocating some more...");

	    // allocate more memory (+1mb), then try again - no reason for it not to work this time
	    // we will never have a 1mb or more change		          

	    // deallocate previous
	    shmop_delete($this->ids[$index]);
	    shmop_close($this->ids[$index]);

	    // calculate new size
	    $size = $this->memSize($str);
	    $size += __ALLOCATE_EXTRA;

	    // alocate new
	    $this->ids[$index] = shmop_open($this->keys[$index], "c", "0644", $size);
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Allocated an additional " . __ALLOCATE_EXTRA . " bytes of memory.");

            // now write the data again
	    shmop_write($this->ids[$index], $str, 0);	
	}
		
	if ($lock) {
	    $this->unlock($this->mutexes[$index]);
	}
    }
	    
    protected function initModules() {
	foreach ($this->modules as $key => $module) {
	    $pid = pcntl_fork();
	    if ($pid == -1) {
	        $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
	    } else if (!$pid) {
		// each managing thread starts the retrieval threads every time
		// the list needs to be refreshed                
		while(1)
		{ 
		    $pages = $module->getPages();		    
		    $pids = array();

		    // if delay is greater than 0, we will do the retrieval in
		    // a single thread (the current one)
		    if ($module->getPageDelay() > 0) {		
			foreach ($pages as $page) {
			    // after each page is done retrieving, fire the event
			    // that will launch the proxy checking threads
			    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Fetching page: $page");

			    $ppid = pcntl_fork();
			    if ($ppid == -1) {
			        $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
			    } else if (!$ppid) {	
				$this->fireProxyRetrievalDoneEvent($module->getProxies($page));
				die;
			    } else {
				$pids[] = $ppid;
				$this->addPid($ppid);
			    }

			    // sleep for page delay
			    usleep($module->getPageDelay());
			}
		    } else {
			// figure out what the thread count will be
			// (if it is higher than the number of pages, adjust it)
			$threadCount = $module->getThreadNumber();
				
			if ($threadCount > count($pages)) {
			    $threadCount = count($pages);
			}
				
			// now depending on the thread:page ratio,
			// assign 1 or more pages to each thread
			$ratio = count($pages) / $threadCount;
			$lowPageCount = ceil($ratio) - 1;
			// we use floor because there need to be as few lazy threads as possible
			$lazyThreads = floor((ceil($ratio) - $ratio)*$threadCount);                    
			// $lazyThreads indicates the number of threads that will use
			// the lower number of pages; the rest of the threads will
			// use that number + 1
				  
			// start the lazy threads                      
			for ($i = 0; $i < $lazyThreads; $i++) {
			    // fork each thread (assign pages to each)
			    $lazyPid = pcntl_fork();
			    if ($lazyPid == -1) {
			        $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
			    } else if (!$lazyPid) {
				for ($j = 0; $j < $lowPageCount; $j++) {
				    $this->fireProxyRetrievalDoneEvent($module->getProxies($pages[$i*$lowPageCount + $j]));
				}
				die; // dispose of the thread after done                                 
			    } else {
				// add pid to mem segment
				$pids[] = $lazyPid;
				$this->addPid($lazyPid);
			    }
			}                       

			// start the non lazy threads - page count + 1
			for ($i = 0; $i < $threadCount - $lazyThreads; $i++) {
			    // fork each thread (assign pages to each)
			    $nonLazyPid = pcntl_fork();
			    if ($nonLazyPid == -1) {
			        $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
			    } else if (!$nonLazyPid) {
				for ($j = 0; $j < $lowPageCount + 1; $j++) {
				    $this->fireProxyRetrievalDoneEvent($module->getProxies(
				        $pages[$lowPageCount*$lazyThreads + $i*($lowPageCount + 1) + $j]));
				}		 
				die; // dispose of the thread after done                              
			    } else {
				// add pid to mem segment
				$pids[] = $nonLazyPid;
				$this->addPid($nonLazyPid);
			    }                              
			}                
		    }
			
		    // don't leave zombies around
		    $this->waitPids($pids);

		    // pause for a while, then start over (update)
		    sleep($module->getUpdateInterval());
		} // end while (managing thread)
		die; // end the thread if any problems occur
	    } else { // end managing thread logic            
		// add pid to mem segment
		$this->addPid($pid, true);                
	    }
	} // end managing thread launcher    
    }
	    
    protected function startWatching() {
	while (1) {
	    if (time() > $this->lastSnapshot + $this->snapshotInterval) {	        
		$data = $this->getVar(__INDEX_DATA);
		$data->loadEntries($dbProxies);
		if ($dbProxies == null) {
		    $dbProxies = array();
		}
		// local variable holding the pids of the threads
		// that do the checking
		$pids = array();
			
		// prepare the checked data					
		$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Performing scheduled proxy checking.");
		foreach ($dbProxies as $proxy)
		{
		    if ($proxy[8] == 0)
		    {
			// check previously working proxies' state                    
			if (time() > $proxy[9] + $this->proxyCheckInterval)
			{                                    
			    $this->launchCheckThread($proxy, $pids);
			}
		    } else {                            
			// check quarantined proxies' state (less often)
			if (time() > $proxy[9] + $this->quarantineCheckInterval)
			{
			    $this->launchCheckThread($proxy, $pids);
			}                        
		    }	    
		}       

		$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Waiting on proxy checking threads.");

		// after all threads are done checking (use pcntl_wait)
		$this->waitPids($pids, true);	

		$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Done checking proxies.");				

		// first, get an updated copy of the proxy list		
		$this->lockBlockState |= STATE_LOCKED;
		$this->lock();		

		$data = $this->getVar(__INDEX_DATA, false);

		// generate snapshot of the current db                
		$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Generating snapshot of db.");		
		$data->snapshot("snapshot-".date("Y.m.d.H.i.s"));		
		
		$this->lastSnapshot = time();                
				
		if ($this->lockBlockState & STATE_SHUTDOWNINLOCK) {
		    // shutdown
		    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Emergency shutdown was needed.");		    
                    $this->performShutdownTasks();

		    die; // so that child threads exit
		}

		$this->unlock();
		$this->lockBlockState &= ~STATE_LOCKED;
	    }
	    // sleep for a bit to save cpu time
	    usleep(1000);
	}    
    }
	    
    public function start() {                      
	// start 1 managing thread for each module
	$this->initModules();
		
	// finally, start the watching process: depending on the last checked ts,
	// we will launch threads to check the state of each existing proxy        
	// loop until stopped
	$this->startWatching();
    }
	    
    protected function launchCheckThread($proxy, &$pids) {
	// check thread count; if cap been reached sleep and check again till it's good
	// must lock here, and unlock also here if count was bad; or unlock after launching thread
	// if count was good
	
	$this->lockBlockState |= STATE_LOCKED_CT;
	$this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
	$count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);

	if ($count >= $this->checkingThreadCap) {
	    do {
	        $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]); // release previous lock
		$this->lockBlockState &= ~STATE_LOCKED_CT;

		usleep(50000);  // free cpu cycles
		
		// grab lock again, and check again
		$this->lockBlockState |= STATE_LOCKED_CT;
		$this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);				
		
		$count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
	    } while ($count >= $this->checkingThreadCap);
	}
     
        // increase thread count, unlock
	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "INCREASING THREAD COUNT TO ".($count + 1));
	$this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count + 1, false);
	$this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
	$this->lockBlockState &= ~STATE_LOCKED_CT;

        $pid = pcntl_fork();
	if ($pid == -1) {
	    $this->logger->logEntry(LoggingImpl::LEVEL_ERROR | LoggingImpl::LEVEL_DIE, "Could not fork!");
        } else if ($pid) {
            // add it to pids array
            $pids[] = $pid;
            // add pid to mem segment
            $this->addPid($pid, true);            
        } else {
            $proxyData = $this->proxyChecker->check($proxy[0], $proxy[1], $proxy[7]);
            
            // we use a separate mutex for this; no reason blocking threads that do 
            // retrieval

	    // 3 new fields added: totalChecks (increased every check),
	    // totalPassed (increased only when passing), quarantineRetries (increased when already in quarantine,
	    // reset when out of quarantine)

	    $this->lock();
            
            // we need to save the data to both the db and the shared memory
            // but not now (when generating the snapshot)
            // for now we will just save it in a temporary memory segment
            // fields that can change:
            // shownHost, country, responseTime,
            // sslSupport, anonimityLevel, type(proxy/socks4/socks5) quarantined,
            // last checked ts
            // field used to identify the record: host and port

	    // check whether shown host is duplicate (if yes, we'll quarantine the proxy,
            // the good proxy list must only contain unique, anonymous, working proxies)	    

            $data = $this->getVar(__INDEX_DATA, false);
	    $data->loadEntries($dbProxies);
	    if ($dbProxies != null) {
	        if ($proxyData != null) {
	            foreach ($dbProxies as $existingProxy) {
	                if ($proxyData->shownHost == $existingProxy[2] && ($proxyData->host != $existingProxy[0] || $proxyData->port != $existingProxy[1])
		            && $existingProxy[8] == 0) {
		            $shownHostDuplicate = true;
	                    break;
	                }
		    }
                }		

	        $id = $data->searchEntries(SEARCH_DEFAULT | SEARCH_REAL_ID, $proxy[0], $proxy[1]);
		if ($id == -1) {
			$this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Problem in checking thread: search result cannot be empty");
			$this->unlock();

                        $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
                        $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
	                $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "DECREASING THREAD COUNT TO ".($count - 1));
			$this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count - 1, false);
			$this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
			die;
		} else if (is_array($id)) {
			$this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Problem in checking thread: search result cannot be array");
			$this->unlock();

                        $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
                        $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
	                $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "DECREASING THREAD COUNT TO ".($count - 1));
			$this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count - 1, false);
			$this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
			die;
		}

		$host = $proxy[0];
		$port = $proxy[1];
	        $proxy = $dbProxies[$id];

		// persistent id is used for remaining operations
		$id = $data->getPersistentId($id);

		if ($host != $proxy[0] || $port != $proxy[1]) {
			$this->logger->logEntry(LoggingImpl::LEVEL_WARN, "Problem in checking thread: ".$proxy[0].":".$proxy[1]." has id meant for ".$host.":".$port);
			$this->unlock();

                        $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
                        $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
	                $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "DECREASING THREAD COUNT TO ".($count - 1));
 	                $this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count - 1, false);
	                $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
			die;
		}

		$deleted = false;

                if ($proxyData != null) {
                    // not quarantined, more fields to change
		    $quarantine = $proxyData->anonimityLevel == 0 || $shownHostDuplicate;                   

		    if ($quarantine && $proxy[8] == 1 && $proxy[12] + 1 >= $this->maxQuarantineRetries) {
		        $data->deleteEntry($id);
			$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Removing defunct proxy ".$proxy[0].":".$proxy[1]);
			$deleted = true;
   		    } else {
                        $proxy = array($proxy[0], $proxy[1], $proxyData->shownHost,
		            $proxyData->country, $proxyData->responseTime, $proxyData->sslSupport,
			    $proxyData->anonimityLevel, $proxyData->type, $quarantine ? 1 : 0, time(),
			    $proxy[10] + 1, $proxy[11] + 1, $quarantine ? $proxy[12] + 1 : 0);			
		    }
		    // if anonimityLevel is 0, the proxy is quarantined (we don't like transparent proxies)    
                } else {
                    // only change quarantine and lastcheckedts fields when saving
                    // field used to identify the record: host and port                

                    // if max quarantine retries has been reached, remove this proxy
                    if ($proxy[8] == 1 && $proxy[12] + 1 >= $this->maxQuarantineRetries) {
                        $data->deleteEntry($id);
                        $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Removing defunct proxy ".$proxy[0].":".$proxy[1]);
			$deleted = true;
                    } else {
                    	// if quarantined, only 2 fields have changed
		    	$proxy = array($proxy[0], $proxy[1], $proxy[2], $proxy[3], $proxy[4],
		        	$proxy[5], $proxy[6], $proxy[7], 1, time(), $proxy[10] + 1,
		        	$proxy[11], $proxy[8] == 0 ? 0 : $proxy[12] + 1);
		    }
		    // last 3 are totalChecks, totalPassed, quarantineRetries                                       
                }            

                // save entry to db
		if (!$deleted) {
                    $data->saveEntries($proxy, $id);
		}
		$this->setVar(__INDEX_DATA, $data, false);
	    }

	    $this->unlock();        
            
            // checking is done when it gets to this point
	    // decrease checking thread count (lock/unlock)
	    $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
	    $count = $this->getVar(__INDEX_CHECKING_THREAD_COUNT, false);
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "DECREASING THREAD COUNT TO ".($count - 1));
	    $this->setVar(__INDEX_CHECKING_THREAD_COUNT, $count - 1, false); 
	    $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);

            // end the thread here
            die;
        }
    }
    
    public function addPid($pid, $main_thread = false) {
        $this->lock($this->mutexes[__INDEX_PIDS]);      
	if ($main_thread) $this->lockBlockState |= STATE_LOCKED_PID;
        $gpids = $this->getVar(__INDEX_PIDS, false);
        $gpids[] = $pid;
        $this->setVar(__INDEX_PIDS, $gpids, false);
        $this->unlock($this->mutexes[__INDEX_PIDS]);    
	if ($main_thread) $this->lockBlockState &= ~STATE_LOCKED_PID;
    }
    
    protected function removePid($pid, $main_thread = false) {
        $this->lock($this->mutexes[__INDEX_PIDS]);      
	if ($main_thread) $this->lockBlockState |= STATE_LOCKED_PID;
        $gpids = $this->getVar(__INDEX_PIDS, false);
        $newpids = array();
        foreach ($gpids as $p) {
            if ($p != $pid) {
                $newpids[] = $p;
            }
        }
        $this->setVar(__INDEX_PIDS, $newpids, false);	
        $this->unlock($this->mutexes[__INDEX_PIDS]);
	if ($main_thread) $this->lockBlockState &= ~STATE_LOCKED_PID;
    }

    private function performShutdownTasks() {
        // clean up memory
        foreach ($this->ids as $index => $id) {
            shmop_delete($id);
            shmop_close($id);
        }

        $this->unlock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);        
        $this->unlock();

	if (!($this->lockBlockState & STATE_LOCKED_PID)) {
            $this->unlock($this->mutexes[__INDEX_PIDS]);
	}

        @unlink(ProxyFramework::$pidFile);
    }
    
    public function stop() {
        if ($this->lockBlockState & STATE_LOCKED) {
	    $this->lockBlockState |= STATE_SHUTDOWNINLOCK;
	}

        // this should kill all the running threads and stop the framework completely
	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCKS");
	if (!($this->lockBlockState & STATE_LOCKED_PID)) {
	    // only lock if not already locked in the main thread
            $this->lock($this->mutexes[__INDEX_PIDS]);
            $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "PIDS LOCK");
        }

	if (!($this->lockBlockState & STATE_LOCKED)) {
	    // only lock if not already locked in the main thread	
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK 1");
            $this->lock();	    
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK 2");
	}
	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK 3");
        
	if (!($this->lockBlockState & STATE_LOCKED_CT)) {
	    // only lock if not already locked in the main thread
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK CT 1");
	    $this->lock($this->mutexes[__INDEX_CHECKING_THREAD_COUNT]);
	    $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK CT 2");
	}
	$this->logger->logEntry(LoggingImpl::LEVEL_INFO, "LOCK CT 3");

        $gpids = $this->getVar(__INDEX_PIDS, false);                

        // kill threads
        foreach ($gpids as $pid) {
            $this->logger->logEntry(LoggingImpl::LEVEL_INFO, "Killing PID $pid");
            posix_kill($pid, SIGKILL);
        }        

	if (!($this->lockBlockState & STATE_LOCKED)) {
            $this->performShutdownTasks();
	}
    }

    public function getLogger() {
        return $this->logger;
    }

    public function getLockBlockState() {
        return $this->lockBlockState;
    }
    
    static public function handler($sig) {
    	ProxyFramework::$instance->getLogger()->logEntry(LoggingImpl::LEVEL_DEBUG, "Handler called for PID " . posix_getpid() . "." . (ProxyFramework::$mainPid != null ? " Main PID is " . ProxyFramework::$mainPid . "." : ""));
	
        // the main process will call the shots
	$cond = posix_getpid() == ProxyFramework::$mainPid;
    	if ($cond) {	  	    	    
            ProxyFramework::$instance->stop();
	    	    
	    ProxyFramework::$instance->getLogger()->logEntry(LoggingImpl::LEVEL_INFO, "Shutting down main process (" .
	        ProxyFramework::$mainPid . ").");    
	    ProxyFramework::$mainPid = null;
	}

        // if shutdown trigger occurred out of lock block we can safely end the process here,
	// otherwise it should wait for the lock block to finish and end it there
	if (!$cond || !(ProxyFramework::$instance->getLockBlockState() & STATE_LOCKED)) {
	    if ($cond) {
	        ProxyFramework::$instance->getLogger()->logEntry(LoggingImpl::LEVEL_INFO, ProxyFramework::$instance->getLockBlockState() & STATE_LOCKED_CT ? "Shutdown occurred in checking thread lock block" : "Shutdown occurred out of lock block.");
	    }
	    die; // so that all threads exit
	}
    }
}  
?>
