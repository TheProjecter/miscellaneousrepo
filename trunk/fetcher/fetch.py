#!/usr/local/bin/python
import calendar
import datetime
import feedparser
import fcntl
import httplib
import os
import sys

def lockfile(path):
        import time
        done = False
        f = open(path, 'w+')
        while done == False:
                done = True
                try:
                        fcntl.lockf(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
                except IOError:
                        done = False

                if done == False:
                        time.sleep(10)
                        sys.stderr.write("waiting for lock...")
        return f

def fw(host, port, action):
        import os
        import socket
        import time
        WAIT_LIMIT = 30

        fail = True
        while fail == True:
            fail = False
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.settimeout(120)	    
	        sys.stderr.write("conn...")
                s.connect(('localhost', 1338))		
                sys.stderr.write("connected")
            except Exception:
            	sys.stderr.write("fw server down")
	    	s.close()
            	fail = True

	retry = False	
        try:
	    s.send(str(os.getpid()) + " " + action + " INPUT -p tcp -m tcp -m multiport -s " + host + "/32 -j ACCEPT --sports " + str(port) + "|EOF")
	except Exception:
	    sys.stderr.write("exc occurred while sending")
	    retry = True
	    s.close()	

	s.setblocking(0)    

        start = time.time()
        try:
            while (retry == False) and (len(s.recvfrom(1024)) != 0):
                if time.time() - start >= WAIT_LIMIT:
	            sys.stderr.write("waited for more than " + str(WAIT_LIMIT))
                    retry = True
                    break

                    # wait for server to close connection
                time.sleep(1)
		
        except Exception:
    	    None

        s.close()

        if retry == True:
            fw(host, port, action)


def execute(host, path, name):	
	BASE_PATH = '/usr/local/apache/python_ports/fetch/'
	data = '<html><head><title>none</title><body>'

	# first check if it's a feed
        fw(host, 80, '-A') # assume 80 = default port for now
	try:
		feed = feedparser.parse('http://' + host + path)

		if feed.bozo == 1:
			# not a feed, probably just plain html
			conn = httplib.HTTPConnection(host, 80)
			conn.request('GET', path)
			resp = conn.getresponse()	
			data = resp.read()
			conn.close()
		else:
		        for e in feed.entries:
	        	        data += '<b><a href="' + e.link + '">' + e.title + '</a></b></br><b>' + e.description + '</b></br>'
			data += '</body></html>'
	except Exception:
#	        fw(host, 80, '-D')
		sys.stderr.write('error occurred while fetching content from http://' + host + path)
		return
#        fw(host, 80, '-D')

	if os.path.isdir(BASE_PATH + name) == False:
		os.mkdir(BASE_PATH + name)
		# create dummy revision 0 file
		f = lockfile(BASE_PATH + name + '/0.html')
		f.write('')
		f.close()

	f = lockfile(BASE_PATH + name + '/' + str(calendar.timegm(datetime.datetime.now().timetuple())) + (feed.bozo == 1 and '.html' or '.xml'))
	f.write(data)
	f.close()

execute(sys.argv[1], sys.argv[2], sys.argv[3])	
