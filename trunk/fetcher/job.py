#!/usr/local/bin/python
import re
import sys

class Comparer:
        def __init__(self, pattern, name, string1 = None, string2 = None):
		try:
                    self.prog = re.compile(pattern, re.MULTILINE | re.DOTALL)
		except Exception:
		    self.prog = None
		    sys.stderr.write('Regex is bad for ' + name)

                self.string1 = string1
                self.string2 = string2
		self.name = name

	def setStrings(self, string1, string2):
		self.string1 = string1
		self.string2 = string2

        def areDifferent(self):
		if (len(self.string1) == 0 and len(self.string2) != 0) or (len(self.string1) != 0 and len(self.string2) == 0):
			return True

		if self.prog != None:
                    results1 = self.prog.findall(self.string1)
                    results2 = self.prog.findall(self.string2)		

                    if len(results1) != len(results2):
                        return True

                    for i in xrange(len(results1)):
                        if len(results1[i]) != len(results2[i]):
                            return True
                        for j in xrange(len(results1[i])):
                            if results1[i][j] != results2[i][j]:
                                return True

                return False

def createhtmlmail (html, subject, sender, recipient):
      	"""Create a mime-message that will render HTML in popular
	   MUAs, text in better ones"""
	import MimeWriter
	import mimetools
	import cStringIO
	
	out = cStringIO.StringIO() # output buffer for our message 
	htmlin = cStringIO.StringIO(html) 	
	
	writer = MimeWriter.MimeWriter(out)
	#
	# set up some basic headers... we put subject here
	# because smtplib.sendmail expects it to be in the
	# message body
	#
	writer.addheader("From", sender)
	writer.addheader("To", recipient)

	writer.addheader("Subject", subject)
	writer.addheader("MIME-Version", "1.0")
	#
	# start the multipart section of the message
	# multipart/alternative seems to work better
	# on some MUAs than multipart/mixed
	#
	writer.startmultipartbody("alternative")
	writer.flushheaders()		
	#
	# start the html subpart of the message
	#
	subpart = writer.nextpart()
	subpart.addheader("Content-Transfer-Encoding", "quoted-printable")
	#
	# returns us a file-ish object we can write to
	#
	pout = subpart.startbody("text/html", [("charset", 'us-ascii')])
	mimetools.encode(htmlin, pout, 'quoted-printable')
	htmlin.close()
	#
	# Now that we're done, close our writer and
	# return the message body
	#
	writer.lastpart()
	msg = out.getvalue()
	out.close()
	print msg
	return msg

def sendmail(sender, recipient, subj, htmlfile, smtp_host, smtp_port,
	smtp_user, smtp_pass):
	import smtplib
	f = open(htmlfile, 'r')
	html = f.read()
	f.close()	
	message = createhtmlmail(html, subj, sender, recipient)
	fw(smtp_host, smtp_port, '-A')
	server = smtplib.SMTP(smtp_host, smtp_port)
	server.ehlo()
	server.starttls()
	server.ehlo()
	server.login(smtp_user, smtp_pass)
	server.sendmail(sender, recipient, message)
	server.quit()
#	fw(smtp_host, smtp_port, '-D')

def lockfile(path):
	import fcntl	
	import time
        done = False
	f = open(path, 'r')
        while done == False:
                done = True
                try:
                        fcntl.lockf(f, fcntl.LOCK_SH | fcntl.LOCK_NB)
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
                s.connect(('localhost', 1338))
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

def execute(name):
	import os
	import smtplib	
	BASE_PATH = '/usr/local/apache/python_ports/fetch/'
	FROM_EMAIL = 'xxx@abc.net'
	TO_EMAIL = 'xxx@abc.net'
	SMTP_HOST = 'smtp.abc.net'
	SMTP_PORT = 25
	SMTP_USER = 'xxx'
	SMTP_PASS = 'yyy'

	l = os.listdir(BASE_PATH + name)
	l.sort()

	comp = None        

	if os.path.isfile(BASE_PATH + name + '.regex'):
		# if regex file exists, read its contents and use the regex to compare files instead
		f = open(BASE_PATH + name + '.regex', 'r')
		comp = Comparer(f.read().strip(), name)
		f.close()

	no_notice_regexes = []
	i = 0
	while True:
		if os.path.isfile(BASE_PATH + name + '.' + str(i) + '.nonotice.regex'):		    
		    f = open(BASE_PATH + name + '.' + str(i) + '.nonotice.regex', 'r')
		    try:
		    	no_notice_regexes.append(re.compile(f.read().strip(), re.MULTILINE | re.DOTALL))
		    except Exception:
		    	sys.stderr.write("No notice regex #" + str(i) + " is bad for " + name)
		    	# do nothing

		    f.close()

		    # read next file
		    i = i + 1
		else:
		    break       

	# we'll always have at least 2 files (we'll create a dummy 0.html file in the beginning)	
	i = 0
	while i < len(l) - 1:
		path1 = BASE_PATH + name + '/' + l[i]
		if os.path.isfile(path1) == False:
			# if file has been deleted skip to next
			continue

		path2 = BASE_PATH + name + '/' + l[i+1]
                if os.path.isfile(path2) == False:
		        # if file has been deleted skip to next
                        continue

		# first one is .xml and second one is .html, then it's a feed, if it's .html it means the feed
		# could not be parsed properly so no update should be sent and the file should just be deleted
		if (path1[-4:] != path2[-4:]) and (path1[-4:] == '.xml'):
			os.remove(path2)
			l.pop(i + 1)
			continue				

		# ensure thread safe access to the files
		f1 = lockfile(path1)
		f2 = lockfile(path2)
		stop = False		

		str1 = f1.read()
		str2 = f2.read()	

		# preliminary checking (first ensure the content of the new file does not match any of the no notice regexes		
		for regex in no_notice_regexes:						
			if regex.match(str2):
				# closing the files will release the lock on them
				f1.close()
				f2.close()
				# remove the new file
				os.remove(path2)
				l.pop(i + 1)
				# try again with the new order
				continue		
		
		if comp != None:
			# advanced comparison using regex			
			comp.setStrings(str1, str2)			

			if comp.areDifferent():
				try:
					sendmail(FROM_EMAIL, TO_EMAIL, name + ' UPDATE ' + l[i+1], path2, SMTP_HOST, SMTP_PORT, SMTP_USER, SMTP_PASS)
				except Exception:
#				        fw(SMTP_HOST, SMTP_PORT, '-D')
					sys.stderr.write("error sending e-mail")
					stop = True			
		else:
			# simple size comparison
			if os.stat(path1).st_size != os.stat(path2).st_size:
				# size is different, e-mail the new version
				try:
					sendmail(FROM_EMAIL, TO_EMAIL, name + ' UPDATE ' + l[i+1], path2, SMTP_HOST, SMTP_PORT, SMTP_USER, SMTP_PASS)
				except Exception:
#				        fw(SMTP_HOST, SMTP_PORT, '-D')
					sys.stderr.write("error sending e-mail")
					# if we can't send the e-mail stop here (we can't go on because I want to keep the updates in chronological order)
					stop = True		

		# closing the files will release the lock on them
		f1.close()
		f2.close()		

		if stop == True:
			break

		# remove this file to mark the fact that it has been processed and is thus no longer needed
		os.remove(path1)
		# NOTE that the last file will always remain, since we always need something to compare to
		i = i + 1

execute(sys.argv[1])
