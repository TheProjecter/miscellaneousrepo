#!/usr/local/bin/python
import sys
import tornado.httpserver
import tornado.ioloop
import tornado.web

class FetchPyHandler(tornado.web.RequestHandler):

    def get_header(self):
        self.write("<html><head><title>fetcher</title></head><body><script type='text/javascript'>" +
	    "function addTextarea() { var n = document.getElementById('no_notice_regex_count').value; n++;" +
	    "document.getElementById('textareas').innerHTML += \"No notice regex #\" + n + \" (optional):<br/> <textarea name='no_notice_regex\" + (n - 1) + \"'></textarea><br/>\"; " +
	    "document.getElementById('no_notice_regex_count').value = n;" +
	    "}" +
	    "</script>");

    def get_footer(self):
        self.write("</body></html>");

    def get_form(self):
        self.get_header()
        self.write("<form action='' method='post'>URL: <input type='textbox' name='url'/><br/>Name: " +
        "<input type='textbox' name='name'/><br/>Regex (optional):<br/>" +
        "<textarea name='regex'></textarea><br/><input type='hidden' name='no_notice_regex_count' id='no_notice_regex_count' value='1'/>No notice regex (optional):<br/>" +
	"<textarea name='no_notice_regex0'></textarea><br/><div id='textareas'></div><br/><a href='#' onclick='addTextarea();'>Add no notice regex</a>" +
	"<br/><input type='submit' name='submit' value='Submit'/></form><br/>")
	
    def get(self):        
        self.get_form()
	self.get_footer()

    def post(self):
        try:
    	    url = self.get_argument('url')
        except Exception:
	    url = ''

        try:
            name = self.get_argument('name').replace(' ', '_')
        except Exception:
            name = ''

        try:
            regex = self.get_argument('regex').strip()
        except Exception:
            regex = ''

	if len(url.strip()) == 0 or len(name.strip()) == 0:
		self.get_form()
		self.write('<b>Name and URL must be valid.</b>');
		return

        parts = url.split('http://')
        if len(parts) < 2:
            parts = url.split('https://')

        if len(parts) < 2:
	    parts = ['', url]

	url = parts[1]
	parts = url.split('/')
	host = parts.pop(0)
	path = '/'.join(parts)
	
	if len(regex) > 0:
		# create regex file if regex was specified
		f = open('/usr/local/apache/python_ports/fetch/' + name + '.regex', 'w+')
		f.write(regex)
		f.close()
	
	j = 0
        for i in xrange(int(self.get_argument('no_notice_regex_count'))):
                try:
                        val = self.get_argument('no_notice_regex' + str(i))

                	# create regex file if regex was specified
                	f = open('/usr/local/apache/python_ports/fetch/' + name + '.' + str(j) + '.nonotice.regex', 'w+')
                	f.write(val)
                	f.close()
			j = j + 1
                except Exception:
                        continue # do nothing

        f = open('/var/spool/cron/crontabs/root', 'a')
        f.write("0,5,10,15,20,25,30,35,40,45,50,55 * * * * /root/fetch.py " + host + " \"/" + path + "\" \"" + name + "\" 2>> /tmp/fetch.err.log\n")
        f.write("1,6,11,16,21,26,31,36,41,46,51,56 * * * * /root/job.py \"" + name + "\" 2>> /tmp/job.err.log\n")
        f.close()

        self.get_form()
        self.write('URL <b>' + url + '</b> scheduled to be fetched under name <b>' + name + '</b>')
	self.get_footer()

application = tornado.web.Application([    
    (r"/fetch", FetchPyHandler)
])

if __name__ == "__main__":
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    tornado.ioloop.IOLoop.instance().start()
