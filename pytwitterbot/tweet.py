import twitter
import simplejson
import random
import logging
from google.appengine.api import urlfetch
 
TWITTER_USER = "user"
TWITTER_PASSWORD = "password" 
 
def tweet():
      if random.randint(1, 100) > 30:
          logging.info("do nothing")
          return   

      api = twitter.Api(username=TWITTER_USER, password=TWITTER_PASSWORD)
      statuses = api.GetPublicTimeline()
      random.shuffle(statuses)

      f = open("quotes.json", "r")
      text = f.read()
      f.close()

      twt = False
      q = simplejson.loads(text) 
      for s in statuses:
          if s.text.lower().find('sopranos') != -1:
              random.shuffle(q)
              ind = 0
              while len(q[ind]) + len(s.user.screen_name) + 2 > 140:
                  ind = ind + 1

              if ind < len(q):
                  status = api.PostUpdate("@%s %s" % (s.user.screen_name, q[ind]))
                  logging.info("@%s %s" % (s.user.screen_name, q[ind]))
              else:
                  logging.info("username too long")

      if random.randint(1, 100) > 40:
          twt = True
          fr = api.GetFriends() 	    
          if (len(fr) > 0) and (random.randint(1, 100) > 60):
              logging.info("harassing friend")
              random.shuffle(fr)
              name = fr[0].screen_name
          else:
              logging.info("harassing stranger")
              name = statuses[0].user.screen_name              

          logging.info("tweet")

          random.shuffle(q)      
          ind = 0
          while len(q[ind]) + len(name) + 2 > 140:
              ind = ind + 1

          if ind < len(q):
              status = api.PostUpdate("@%s %s" % (name, q[ind]))
              logging.info("@%s %s" % (name, q[ind]))
          else:
              logging.info("username too long")
      
      if random.randint(1, 100) > 93:
          logging.info("follow @%s" % statuses[0].user.screen_name)
          api.CreateFriendship(statuses[0].user.screen_name)
 
if __name__ == '__main__':
  tweet()