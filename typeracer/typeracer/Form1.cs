using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace typeracer
{
    public partial class Form1 : Form
    {
        private const string HASH = "7E4CA4909CEA96A1D2A5B1C8FA9E977A";
        private const int WPS = 3;
        private const string SAVE_PATH = @"C:\typeracer_captcha\";
        private const string LETTER_PATH = SAVE_PATH + @"letters\";
        private const string IMG_NAME = "challenge{0}.jpg";
        private string file;
        private List<Letter> letters = new List<Letter>();
        private Dictionary<string, int> dic = new Dictionary<string, int>();

        public Form1()
        {
            InitializeComponent();
        }

        /// <summary>
        /// method for converting a System.DateTime value to a UNIX Timestamp
        /// </summary>
        /// <param name="value">date to convert</param>
        /// <returns></returns>
        private static long ConvertToTimestamp(DateTime value)
        {
            TimeSpan span = (value - new DateTime(1970, 1, 1, 0, 0, 0, 0).ToLocalTime());
            //return the total seconds (which is a UNIX timestamp)
            return (long)span.TotalMilliseconds;
        }

        private void Run()
        {
            WebClient wc = new WebClient();

            do
            {

                // get session id
                wc.Headers.Add("User-Agent",
                               "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.8) Gecko/20100727 Firefox/3.6.3");
                wc.Headers.Add("Pragma", "no-cache");
                wc.Headers.Add("Cache-Control", "no-cache");
                string buf = wc.DownloadString("http://play.typeracer.com");
                int index = buf.IndexOf("gameserv;jsessionid=") + "gameserv;jsessionid=".Length;
                string session = buf.Substring(index, buf.IndexOf("\"", index) - index);
                index = buf.IndexOf("typeracerServerTime =") + "typeracerServerTime =".Length;
                long serverTime = Int64.Parse(buf.Substring(index, buf.IndexOf(";", index) - index));
                long diff = serverTime - ConvertToTimestamp(DateTime.Now);
                Invoke(new MethodInvoker(delegate
                                             {
                                                 textBox1.Text = "";
                                                 textBox1.Text += "got session id: " + session + "\r\n";
                                                 textBox1.Text += "got server time: " + serverTime + ", diff = " + diff +
                                                                  "\r\n";
                                             }));


                // get initial status
                string data =
                    "5|0|4|http://play.typeracer.com/|" + HASH +
                    "|com.typeracer.main.client.rpc.GameService|getInitialStatus|1|2|3|4|0|";
                wc.Headers.Add("Referer", "http://play.typeracer.com/CC4E0D79367C96C350B73B7CA9687C1F.cache.html");
                wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);

                wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                wc.Headers.Add("Cookie",
                               "prefs=%7B%22ffMsgDismissed%22%3A%20false%2C%20%22dateOfLastVisit%22%3A%20" + serverTime +
                               "%7D");
                data =
                    "5|0|4|http://play.typeracer.com/|" + HASH +
                    "|com.typeracer.main.client.rpc.GameService|joinStandaloneGame|1|2|3|4|0|";
                buf = wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);

                // parse game id and number of words
                string[] parts = buf.Split(new[] { '[' })[1].Split(new[] { ',' });
                string game = parts[parts.Length - 5];

                int words = Int32.Parse(buf.Split(new[] { ',' })[5]);
                Invoke(new MethodInvoker(delegate
                                             {
                                                 textBox1.Text += "got game id: " + game + "\r\n";
                                                 textBox1.Text += "got number of words " + words + "\r\n";
                                             }));

                string state;
                do
                {
                    // send updatePlayerProgress every 1 sec until game starts (state is 2)
                    long curServerTime = ConvertToTimestamp(DateTime.Now) + diff;
                    wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                    data =
                        "5|0|6|http://play.typeracer.com/|" + HASH +
                        "|com.typeracer.main.client.rpc.GameService|updatePlayerProgress|I|D|1|2|3|4|3|5|5|6|" + game +
                        "|0|" + curServerTime + "|";
                    buf = wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);
                    parts = buf.Split(new[] { '[' })[1].Split(new[] { ',' });
                    state = parts[parts.Length - 7];
                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "state = " + state + ", curServerTime = " +
                                                                      curServerTime + "\r\n";
                                                 }));
                    if (state == "2") break;

                    Thread.Sleep(1000);
                } while (state != "2");

                int i = 0;
                // increase word count every 1 sec until reaching word count + 1
                string place;
                bool challenge = false;
                do
                {
                    i += (i + WPS > words ? words - i : WPS);
                    long curServerTime = ConvertToTimestamp(DateTime.Now) + diff;
                    wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                    data =
                        "5|0|6|http://play.typeracer.com/|" + HASH +
                        "|com.typeracer.main.client.rpc.GameService|updatePlayerProgress|I|D|1|2|3|4|3|5|5|6|"
                        + game + "|" + i + "|" + curServerTime + "|";
                    buf = wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);
                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "wc = " + i + ", response = " + buf +
                                                                      "\r\n";
                                                 }));
                    if (buf.IndexOf("TypingChallengeRequiredException") != -1)
                    {
                        challenge = true;
                        break;
                    }
                    // game ends when player position in response has
                    // changed from 0 to something else, display WPM
                    parts = buf.Split(new[] { ',' });
                    place = parts[15];
                    string wpm = parts[14];
                    if (place != "0")
                    {
                        Invoke(new MethodInvoker(delegate
                                                     {
                                                         textBox1.Text += "done. place = " + place + ", wpm = " +
                                                                          wpm + "\r\n";
                                                     }));
                        break;
                    }

                    Thread.Sleep(1000);
                } while (place == "0");

                if (challenge)
                {
                    index = buf.IndexOf("guest:") + "guest:".Length;
                    string user = buf.Substring(index, buf.IndexOf("\"", index) - index);
                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "got user id: " + user + "\r\n";
                                                 }));

                    // get position and wpm
                    long curServerTime = ConvertToTimestamp(DateTime.Now) + diff;
                    wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                    data =
                        "5|0|6|http://play.typeracer.com/|" + HASH +
                        "|com.typeracer.main.client.rpc.GameService|updatePlayerProgress|I|D|1|2|3|4|3|5|5|6|"
                        + game + "|" + i + "|" + curServerTime + "|";
                    buf = wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);
                    parts = buf.Split(new[] { ',' });
                    int at = 8;
                    place = parts[at + 1];
                    string wpm;
                    do
                    {
                        place = parts[at + 1];
                        wpm = parts[at];
                        at += 6;
                    } while (place == "0" && at < parts.Length);
                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "done. place = " + place + ", wpm = " +
                                                                      wpm + "\r\n";
                                                 }));

                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "getting typing challenge\r\n";
                                                 }));
                    wc.Headers.Add("Content-Type", "text/x-gwt-rpc; charset=utf-8");
                    data =
                        "5|0|6|http://play.typeracer.com/|" + HASH +
                        "|com.typeracer.main.client.rpc.GameService|getTypingChallenge|java.lang.String|guest:"
                        + user + "|1|2|3|4|1|5|6|";
                    buf = wc.UploadString("http://play.typeracer.com/gameserv;jsessionid=" + session, data);
                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     textBox1.Text += "challenge url = " + buf +
                                                                      "\r\n";
                                                 }));

                    wc.Headers.Add("Accept", "image/png,image/*;q=0.8,*/*;q=0.5");
                    // save jpeg image into Downloads, if existing add number at end
                    i = 1;
                    do
                    {
                        file = SAVE_PATH + string.Format(IMG_NAME, i + "");
                        i++;
                    } while (File.Exists(file));
                    wc.DownloadFile("http://play.typeracer.com/challenge?id=guest:" + user, file);

                    // parse captcha
                    Captcha();

                    // send result..
                }

                Thread.Sleep(1200);
            } while (true);
        }

        private Bitmap Captcha()
        {
            Bitmap image = new Bitmap(Image.FromFile(file));
            List<Letter> toRemove = new List<Letter>();

            for (int y = 0; y < image.Height; y++)
            {
                for (int x = 0; x < image.Width; x++)
                {
                    Color c = image.GetPixel(x, y);
                    if (c.R < 70 && c.G < 70 && c.B < 70)
                    {
                        image.SetPixel(x, y, Color.FromArgb(255, 255, 0, 0));
                    }
                    else if (c.R < 163 && c.G < 163 && c.B < 163)
                    {
                        //image.SetPixel(x, y, Color.FromArgb(255, 0, 255, 0));
                        // check if point is adjacent to any of the points in the existing letters
                        Point p = new Point(x, y);
                        int letter = -1;
                        List<int> close = new List<int>();
                        for (int i = -1; i <= 1; i++)
                        {
                            for (int j = -1; j <= 1; j++)
                            {
                                if (i == 0 && j == 0) continue;
                                string key = (x + i) + "," + (y + j);
                                if (dic.ContainsKey(key))
                                {
                                    if (letter != -1 && letter != dic[key])
                                    {
                                        close.Add(dic[key]);
                                        continue;
                                    }
                                    letter = dic[key];
                                }
                            }
                        }

                        if (letter == -1)
                        {
                            // add new letter
                            Letter l = new Letter();
                            l.MinX = p.X;
                            l.MaxX = p.X;
                            l.MinY = p.Y;
                            l.MaxY = p.Y;
                            l.Pixels.Add(p);
                            dic[p.X + "," + p.Y] = letters.Count;
                            //l.Color = cols[letters.Count % cols.Length];
                            //image.SetPixel(x, y, l.Color);
                            letters.Add(l);
                            continue;
                        }

                        if (p.X < letters[letter].MinX)
                            letters[letter].MinX = p.X;
                        if (p.Y < letters[letter].MinY)
                            letters[letter].MinY = p.Y;

                        if (p.X > letters[letter].MaxX)
                            letters[letter].MaxX = p.X;
                        if (p.Y > letters[letter].MaxY)
                            letters[letter].MaxY = p.Y;

                        letters[letter].Pixels.Add(p);
                        dic[p.X + "," + p.Y] = letter;

                        // merge all other nearby letters with this one
                        foreach (int l in close)
                        {
                            if (letters[letter].MergePixels(letters[l], image))
                            {
                                toRemove.Add(letters[l]);
                                foreach (Point pp in letters[l].Pixels)
                                {
                                    dic[pp.X + "," + pp.Y] = letter;
                                }
                            }
                        }
                    }
                }
            }

            foreach (Letter l in toRemove)
            {
                letters.Remove(l);
            }

            // complexity: O(n)
            // min: 151ms, max: 245ms, avg: 177.147058823529ms

            Random r = new Random((int)(DateTime.Now.Ticks / 1000));

            // highlight each individual letter by assigning a separate color
            for (int i = 0; i < letters.Count; i++)
            {
                letters[i].Color = Color.FromArgb(255, (r.Next(0, 255) + i * 2) % 255,
                                                  (r.Next(0, 255) + i * 3) % 255,
                                                  (r.Next(0, 255) + i * 5) % 255); //cols[i%cols.Length];
                foreach (Point p in letters[i].Pixels)
                {
                    image.SetPixel(p.X, p.Y, letters[i].Color);
                }
            }

            // draw the whole text image
            Invoke(new MethodInvoker(delegate
            {
                Graphics g = Graphics.FromHwnd(Handle); // real graphics
                g.DrawImage(image, 540, 80);

                g.Dispose();
            }));
            /*
            for (int i = 0; i < letters.Count; i++)
            {
                // create new image containing just this letter, white on black
                Image single = letters[i].CreateImage();

                // prompt user to read letter, if 'bad' is inputted, letter was not properly recognized
                Invoke(new MethodInvoker(delegate
                {
                    Graphics g = Graphics.FromHwnd(Handle); // real graphics
                    g.DrawImage(new Bitmap(100, 40, PixelFormat.Format24bppRgb), 540, 30);
                    g.DrawImage(single, 540, 30);

                    g.Dispose();
                    textBox2.Text = "";
                    textBox2.Enabled = true;
                    textBox2.Focus();
                }));

                while (textBox2.Enabled)
                {
                    Thread.Sleep(100);
                }

                // save individual letter to jpg file
                if (textBox2.Text.ToLower() != "bad")
                {
                    if (!Directory.Exists(LETTER_PATH + textBox2.Text))
                    {
                        Directory.CreateDirectory(LETTER_PATH + textBox2.Text);
                    }
                    for (int j = 0; ; j++)
                    {
                        if (File.Exists(LETTER_PATH + textBox2.Text + "\\" + j +
                                    ".jpg")) continue;

                        single.Save(LETTER_PATH + textBox2.Text + "\\" + j +
                                    ".jpg");
                        break;
                    }
                }
            }
            */
            return image;
        }

        private static double[] GetStats(Bitmap im)
        {
            int minX = -1;
            int maxX = 0;
            int minY = -1;
            int maxY = 0;
            double pix = im.Width * im.Height;
            double covered = 0;
            int mingapx = Int32.MaxValue;
            int maxgapx = 0;
            int mingapy = Int32.MaxValue;
            int maxgapy = 0;

            for (int y = 0; y < im.Height; y++)
            {
                int gap = 0;
                for (int x = 0; x < im.Width; x++)
                {
                    Color c = im.GetPixel(x, y);
                    if (c.R == 255)
                    {
                        // calculate width/height
                        if (minX == -1)
                        {
                            minX = x;
                            minY = y;
                        }

                        if (x > maxX)
                        {
                            maxX = x;
                        }

                        if (y > maxY)
                        {
                            maxY = y;
                        }

                        // percentage of canvas covered
                        covered++;

                        // largest/smallest gap
                        if (gap > maxgapx) maxgapx = gap;
                        if ((gap > 0 || x == im.Width - 1) && gap < mingapx) mingapx = gap;
                        gap = 0;
                    }
                    else
                    {
                        gap++;
                        if (x == im.Width - 1)
                        {
                            if (gap > maxgapx) maxgapx = gap;
                            if (gap < mingapx) mingapx = gap;
                        }
                    }
                }
            }

            for (int x = 0; x < im.Width; x++)
            {
                int gap = 0;
                for (int y = 0; y < im.Height; y++)
                {
                    Color c = im.GetPixel(x, y);
                    if (c.R == 255)
                    {
                        if (gap > maxgapy) maxgapy = gap;
                        if ((gap > 0 || y == im.Height - 1) && gap < mingapy) mingapy = gap;
                        gap = 0;
                    }
                    else
                    {
                        gap++;
                        if (y == im.Height - 1)
                        {
                            if (gap > maxgapy) maxgapy = gap;
                            if (gap < mingapy) mingapy = gap;
                        }
                    }
                }
            }

            return new[] { maxX - minX, maxY - minY, covered / pix, mingapx, maxgapx, mingapy, maxgapy };
        }

        private static double ComputeScore(double[] stats1, double[] stats2, Image im)
        {
            // compute score as a sum of the variance of all the stats

            // TODO: compare percentage of canvas covered, if more check if any extremities can be removed withot affecting above stats
            // then recalculate stats for the image

            double score = Math.Pow(1 - (stats2[0] / stats1[0]), 2) + Math.Pow(1 - (stats2[1] / stats1[1]), 2) +
                Math.Pow(stats2[2] - stats1[2], 2) + Math.Pow(stats2[3] - stats1[3], 2) + Math.Pow(stats2[4] - stats1[4], 2)
                + Math.Pow(stats2[5] - stats1[5], 2) + Math.Pow(stats2[6] - stats1[6], 2);

            return score;
        }

        private static Bitmap RotateImage(Bitmap im, int offset, int line)
        {
            int width = 0;
            int height = 0;
            int offsetX = 0;
            int offsetY = 0;

            // rotate image right/left (above center goes right/left and down, below center goes left/right and up)
            List<Point> pix = new List<Point>();
            for (int y = 0; y < im.Height; y++)
            {
                for (int x = 0; x < im.Width; x++)
                {
                    Color c = im.GetPixel(x, y);
                    if (c.R == 255)
                    {
                        Point p = new Point();

                        if (y < line)
                        {
                            p.X = x + offset;
                            p.Y = y + Math.Abs(offset);
                        }

                        if (y > line)
                        {
                            p.X = x - offset;
                            p.Y = y - Math.Abs(offset);
                        }

                        if (p.X > width) width = p.X;
                        if (p.Y > height) height = p.Y;

                        if (p.X < offsetX) offsetX = p.X;
                        if (p.Y < offsetY) offsetY = p.Y;

                        pix.Add(p);
                    }
                }
            }

            Bitmap bmp = new Bitmap(width + 1 - offsetX, height + 1 - offsetY, PixelFormat.Format24bppRgb);
            foreach (Point p in pix)
            {
                bmp.SetPixel(p.X - offsetX, p.Y - offsetY, Color.FromArgb(255, 255, 255, 255));
            }

            return bmp;
        }

        private static double GetMatchScore(Bitmap img, Bitmap match)
        {
            // compute score by calculating likeness of the two images..
            int line = img.Height / 2;
            double[] stats = GetStats(match);
            double best = Int32.MaxValue;
            for (int i = -3; i < 3; i++)
            {
                Bitmap im = img;//RotateImage(img, i, line);
                double[] statsImg = GetStats(im);
                double score = ComputeScore(stats, statsImg, im);
                if (score < best)
                {
                    best = score;
                }
            }

            return best;
        }

        private static string GetBestMatch(string md5, Bitmap img, List<string> files)
        {
            string best = "";
            double bestScore = Int32.MaxValue;

            foreach (string file in files)
            {
                if (Checksum(file) == md5) return file;

                Bitmap match = new Bitmap(Image.FromFile(file));
                double score = GetMatchScore(img, match);
                if (score < bestScore)
                {
                    bestScore = score;
                    best = file;
                }
            }

            return best;
        }

        private static string Checksum(string fileName)
        {
            bool exc;
            do
            {
                try
                {
                    using (FileStream file = new FileStream(fileName, FileMode.Open))
                    {
                        MD5 md5 = new MD5CryptoServiceProvider();
                        byte[] retVal = md5.ComputeHash(file);


                        StringBuilder sb = new StringBuilder();
                        for (int i = 0; i < retVal.Length; i++)
                        {
                            sb.Append(retVal[i].ToString("x2"));
                        }
                        file.Close();
                        return sb.ToString();
                    }
                }
                catch (Exception e)
                {
                    exc = true;
                }
            } while (exc);

            return null;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            new Thread(new ThreadStart(delegate
                           {
                               // TODO: test exercise, pick one of the more frequent letters, guess which letter it is by comparing it with all the other known ones
                               // take into account: translation on x or y, rotation, right or left
                               // then each matched pixel will add up to a score, letter with the biggest score is the most likely match

                               // afterwards repeat this process for all letters with at least 2 samples, count successes/failures, determine success%

                               // pick random letter with at least 2 samples
                               string[] letters = Directory.GetDirectories(LETTER_PATH);
                               List<string> fs = new List<string>();
                               List<string> allFiles = new List<string>();
                               foreach (string let in letters)
                               {
                                   string[] files = Directory.GetFiles(let);
                                   if (files.Length > 1)
                                   {
                                       fs.AddRange(files);
                                   }
                                   allFiles.AddRange(files);
                               }

                               int correct = 0;
                               Dictionary<string, int> guessesPerLetter = new Dictionary<string, int>();
                               Dictionary<string, int> correctGuessesPerLetter = new Dictionary<string, int>();

                               /*
                               Random r = new Random((int)(DateTime.Now.Ticks / 1000));
                               string file = fs[r.Next() % fs.Count];
                               */

                               foreach (string f in fs)
                               {
                                   string file = f;
                                   List<string> compare = new List<string>(allFiles);
                                   compare.Remove(file);
                                   Invoke(new MethodInvoker(delegate
                                                                {
                                                                    textBox1.Text +=
                                                                        "Picked: " + file +
                                                                        "\r\n";
                                                                }));
                                   string orig = file;
                                   orig = orig.Substring(0, orig.LastIndexOf("\\"));
                                   orig = orig.Substring(orig.LastIndexOf("\\") + 1);

                                   file = GetBestMatch(Checksum(file), new Bitmap(Image.FromFile(file)), compare);
                                   file = file.Substring(0, file.LastIndexOf("\\"));
                                   file = file.Substring(file.LastIndexOf("\\") + 1);
                                   Invoke(new MethodInvoker(delegate
                                                                {
                                                                    textBox1.Text +=
                                                                        "BEST MATCH: " +
                                                                        file + "\r\n";
                                                                }));
                                   if (file == orig)
                                   {
                                       correct++;
                                       if (correctGuessesPerLetter.ContainsKey(file)) correctGuessesPerLetter[file]++;
                                       else
                                           correctGuessesPerLetter[file] = 1;
                                   }
                                   if (guessesPerLetter.ContainsKey(file)) guessesPerLetter[file]++;
                                   else
                                       guessesPerLetter[file] = 1;
                               }
                               Invoke(new MethodInvoker(delegate
                               {
                                   textBox1.Text +=
                                       "correct: " +
                                       correct + "/" + fs.Count + " (" + (((float)correct / fs.Count) * 10000) / 100 + "%)\r\n";

                                   textBox1.Text += "guesses per letter: ";
                                   foreach (string letter in guessesPerLetter.Keys)
                                   {
                                       correct = correctGuessesPerLetter.ContainsKey(letter)
                                                         ? correctGuessesPerLetter[letter]
                                                         : 0;
                                       textBox1.Text += letter + ": " + correct + "/" + guessesPerLetter[letter]
                                           + " (" + (((float)correct / guessesPerLetter[letter]) * 10000) / 100 + "%)\r\n";
                                   }
                               }));
                               /*
correct: 124/251 (49.40239%)
guesses per letter: c: 2/12 (16.66667%)
x: 0/2 (0%)
s: 7/17 (41.17647%)
e: 40/52 (76.92307%)
o: 8/20 (40%)
t: 26/35 (74.28572%)
a: 2/14 (14.28572%)
u: 0/7 (0%)
h: 13/20 (65%)
n: 4/11 (36.36364%)
r: 3/4 (75%)
y: 0/4 (0%)
d: 6/13 (46.15385%)
p: 0/5 (0%)
g: 0/6 (0%)
f: 0/4 (0%)
v: 0/2 (0%)
l: 10/13 (76.92307%)
b: 0/2 (0%)
d2: 0/2 (0%)
w: 3/4 (75%)
m: 0/2 (0%)
*/

                               /*double min = Int32.MaxValue;
                               double max = 0;
                               double sum = 0;
                               for (int i = 1; i <= 102; i++)
                               {
                                   letters.Clear();
                                   dic.Clear();
                                   file = @"C:\Users\johnsmith\Downloads\challenge" + i + ".jpg";
                                   var start = DateTime.Now;
                                   Image image = Captcha();
                                   double took = (DateTime.Now - start).TotalMilliseconds;
                                   if (took > max)
                                   {
                                       max = took;
                                   }

                                   if (took < min)
                                   {
                                       min = took;
                                   }

                                   sum += took;

                                   image.Save(file.Substring(0, file.LastIndexOf(".")) + "b.jpg");
                                   Invoke(new MethodInvoker(delegate
                                   {
                                       textBox1.Text += "Saving " + file.Substring(0, file.LastIndexOf(".")) +
                                                        "b.jpg\r\n";
                                   }));
                               }

                               Invoke(new MethodInvoker(delegate
                               {
                                   textBox1.Text += "min: " + min + "ms, max: " + max + "ms, avg: " + (sum / 102) + "ms\r\n";
                               }));*/
                           })).Start();

            //new Thread(Captcha).Start();
        }

        private void textBox2_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Return)
            {
                textBox2.Enabled = false;
            }
        }
    }

    public class Letter
    {
        private const int RANGE = 1;

        private const int MAX_LETTER_WIDTH = 20;

        private List<Point> pixels = new List<Point>();

        public List<Point> Pixels
        {
            get
            {
                return pixels;
            }
        }

        public Color Color
        {
            get;
            set;
        }

        public bool Merged
        {
            get;
            set;
        }

        public int MinX
        {
            get;
            set;
        }

        public int MaxX
        {
            get;
            set;
        }

        public int MinY
        {
            get;
            set;
        }

        public int MaxY
        {
            get;
            set;
        }

        private int Width
        {
            get
            {
                return MaxX - MinX + 1;
            }
        }

        private int Height
        {
            get
            {
                return MaxY - MinY + 1;
            }
        }

        public Image CreateImage()
        {
            Bitmap bmp = new Bitmap(Width, Height, PixelFormat.Format24bppRgb);
            foreach (Point p in pixels)
            {
                bmp.SetPixel(p.X - MinX, p.Y - MinY, Color.FromArgb(255, 255, 255, 255));
            }

            return bmp;
        }

        public bool Adjacent(Point p)
        {
            foreach (Point c in pixels)
            {
                if (c.X >= p.X - RANGE && c.X <= p.X + RANGE && c.Y >= p.Y - RANGE && c.Y <= p.Y + RANGE)
                {
                    return true;
                }
            }

            return false;
        }

        public bool Adjacent(Letter l)
        {
            foreach (Point p in l.Pixels)
            {
                if (Adjacent(p))
                {
                    return true;
                }
            }

            return false;
        }

        public bool MergePixels(Letter l, Bitmap image)
        {
            // find out resulting width, if too great abort merge
            int min = MinX < l.MinX ? MinX : l.MinX;
            int max = MaxX > l.MaxX ? MaxX : l.MaxX;

            if (max - min + 1 > MAX_LETTER_WIDTH)
            {
                return false;
            }

            MinX = min;
            MaxX = max;
            MinY = MinY < l.MinY ? MinY : l.MinY;
            MaxY = MaxY > l.MaxY ? MaxY : l.MaxY;

            // add all of the pixels of the merged letter
            pixels.AddRange(l.Pixels);
            // change their color
            /*foreach (Point p in l.Pixels)
            {
                image.SetPixel(p.X, p.Y, Color);
            }*/

            return true;
        }
    }
}
