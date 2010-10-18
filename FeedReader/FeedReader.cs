///   Copyright (c) 2007-2008, AOL LLC 

///   All rights reserved.

///   Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

///   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
///   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
///   Neither the name of the AOL LLC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 
///   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
///   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
///   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
///   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
///   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
///   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
///   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
///   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
///   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
///   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Microsoft.Win32;
using AccCoreLib;

using System.Net;
using System.Net.Sockets;
using System.Xml;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using TimerClass = System.Threading.Timer;
using System.IO;
using System.Text.RegularExpressions;

namespace Plugin
{
    #region Db
    public class Db
    {
        /// <summary>
        /// List of feeds.
        /// </summary>
        public static IDictionary<string, IDictionary<string, FeedItem>> Feeds = new Dictionary<string, IDictionary<string, FeedItem>>();

        /// <summary>
        /// List of feeds and their corresponding list of feed items.
        /// </summary>
        public static IDictionary<string, Feed> FeedData = new Dictionary<string, Feed>();

        /// <summary>
        /// List of feeds added during the update phase.
        /// </summary>
        public static IDictionary<string, Feed> NewFeeds = new Dictionary<string, Feed>();

        /// <summary>
        /// List of feeds scheduled to be deleted after updating process is over.
        /// </summary>
        public static IList<string> ToDelete = new List<string>();
    }
    #endregion

    #region FeedItem Comparer
    /// <summary>
    /// Used for sorting FeedItems by date.
    /// </summary>
    public class FeedItemComparer : IComparer<FeedItem>
    {
        /// <summary>
        /// Compare two <c>FeedItem</c> objects' dates.
        /// </summary>
        /// <param name="x">The first object</param>
        /// <param name="y">The second object</param>
        /// <returns>Returns less than 0 if first object's date is greater than second's; 0 if equal,
        /// and greater than 0 if greater</returns>
        public int Compare(FeedItem x, FeedItem y)
        {
            return x.Date.CompareTo(y.Date);
        }
    }
    #endregion

    #region FeedItem
    /// <summary>
    /// FeedItem class.
    /// </summary>
    public class FeedItem
    {
        /// <summary>
        /// The name of the feed item (it is extracted from the feed xml, must be unique for each item in a feed).
        /// </summary>
        private string name = null;
        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
            }
        }

        /// <summary>
        /// The description of the feed item (it is extracted from the feed xml).
        /// </summary>
        private string description = null;
        public string Description
        {
            get
            {
                return description;
            }
            set
            {
                description = value;
            }
        }

        /// <summary>
        /// The link of the feed item (it is extracted from the feed xml).
        /// </summary>
        private string link = null; 
        public string Link
        {
            get
            {
                return link;
            }
            set
            {
                link = value;
            }
        }

        /// <summary>
        /// Whether this feed item has been read yet.
        /// </summary>
        private bool isRead = false;
        public bool IsRead
        {
            get
            {
                return isRead;
            }
            set
            {
                isRead = value;
            }
        }

        /// <summary>
        /// The date the feed item was added.
        /// </summary>
        private DateTime date = DateTime.MinValue;
        public DateTime Date
        {
            get
            {
                return date;
            }
            set
            {
                date = value;
            }
        }

        /// <summary>
        /// Initializes the name, description and link fields.
        /// </summary>
        /// <param name="name">The name</param>
        /// <param name="description">The description</param>
        /// <param name="link">The link</param>
        public FeedItem(string name, string description, string link)
        {
            Name = name;
            Description = description;
            Link = link;
            IsRead = false;
            Date = DateTime.Now;
        }
    }
    #endregion

    #region Feed
    public class Feed
    {
        /// <summary>
        /// The name of the feed (it is chosen by the user and must be unique for each feed).
        /// </summary>
        private string name = null;
        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
            }
        }

        /// <summary>
        /// The description of the feed (it is extracted from the feed xml).
        /// </summary>
        private string description = null;
        public string Description
        {
            get
            {
                return description;
            }
            set
            {
                description = value;
            }
        }

        /// <summary>
        /// The link of the feed (it is chosen by the user and must be unique for each feed).
        /// </summary>
        private string link = null; 
        public string Link
        {
            get
            {
                return link;
            }
            set
            {
                link = value;
            }
        }

        /// <summary>
        /// The type of the feed (depending on the format, it can be RSS or ATOM).
        /// </summary>
        private string type = null;
        public string Type
        {
            get
            {
                return type;
            }
            set
            {
                type = value;
            }
        }

        /// <summary>
        /// The feed server to connect to. (parsed from the feed link)
        /// </summary>
        private string server = null;
        public string Server
        {
            get
            {
                return server;
            }
            set
            {
                server = value;
            }
        }

        /// <summary>
        /// The feed port to connect to. (parsed from the feed link)
        /// </summary>
        private int port;
        public int Port
        {
            get
            {
                return port;
            }
            set
            {
                port = value;
            }
        }

        /// <summary>
        /// The path to the feed on the server. (parsed from the feed link)
        /// </summary>
        private string path = null;
        public string Path
        {
            get
            {
                return path;
            }
            set
            {
                path = value;
            }
        }

        /// <summary>
        /// The time of last update of the feed.
        /// </summary>
        private string lastUpdate = null;
        public string LastUpdate
        {
            get
            {
                return lastUpdate;
            }
            set
            {
                lastUpdate = value;
            }
        }

        /// <summary>
        /// Whether status text should be updated every time a new feed item is found for this feed.
        /// </summary>
        private bool updateStatus = false;
        public bool UpdateStatus
        {
            get
            {
                return updateStatus;
            }
            set
            {
                updateStatus = value;
            }
        }

        /// <summary>
        /// Contains a list of buddies to notify when a new item is found.
        /// </summary>
        private IList<string> notifyList = new List<string>();
        public IList<string> NotifyList
        {
            get
            {
                return notifyList;
            }
            set
            {
                notifyList = value;
            }
        }

        /// <summary>
        /// Whether buddy notification is enabled.
        /// </summary>
        private bool notifyEnabled = false;
        public bool NotifyEnabled
        {
            get
            {
                return notifyEnabled;
            }
            set
            {
                notifyEnabled = value;
            }
        }

        /// <summary>
        /// Initializes the name, description, link and type fields.
        /// </summary>
        /// <param name="name">The name</param>
        /// <param name="description">The description</param>
        /// <param name="link">The link</param>
        /// <param name="type">The feed type</param>
        /// <exception cref="ArgumentException">If any of the arguments is empty.</exception>
        /// <exception cref="ArgumentNullException">If any of the arguments is null.</exception>
        public Feed(string name, string description, string link, string type)
        {
            // do not check for these things on initialization
            if (!FeedReader.Initializing)
            {
                Helper.CheckParameters(new object[] { name, description, link, type },
                                       new string[] { "name", "description", "link", "type" });
            }

            Name = name;
            Description = description;
            Link = link;
            Type = type;
            LastUpdate = "Never";

            string[] parts = Helper.ParseUrl(link);

            Server = parts[0];
            Port = Int32.Parse(parts[1]);
            Path = parts[2];
        }
    }
    #endregion

    /// <summary>
    /// Plugin that reads, manages syndicate feeds.
    /// </summary>
    #if DEBUG
    [GuidAttribute("7665316D-7449-4238-3264-4C594D754A52")]
    #else
    [GuidAttribute("7665316D-7449-4238-3264-4C594D754A52")]
    #endif
    public class FeedReader : IAccPlugin, IAccCommandTarget
    {
        /// <summary>
        /// The id of the command that opens the feeds form.
        /// </summary>
        private const int kMyFeedsId = 0;

        /// <summary>
        /// The id of the command that shares one's feed list with one's contact.
        /// </summary>
        private const int kShareFeedsId = 1;

        /// <summary>
        /// The AIM session.
        /// </summary>
        public static AccSession m_session;

        /// <summary>
        /// Empty constructor.
        /// </summary>
        public FeedReader()
        {
        }

        #region Feed retrieval

        /// <summary>
        /// The socket used when retrieving feeds.
        /// </summary>
        private Socket Client;

        /// <summary>
        /// Data read from the socket.
        /// </summary>
        private byte[] Data = new byte[1024];

        /// <summary>
        /// Size of the data read from the socket.
        /// </summary>
        private const int Size = 1024;

        /// <summary>
        /// Whether the plug-in is currently initializing.
        /// </summary>
        public static bool Initializing = false;

        /// <summary>
        /// Data read from each feed, reset every update.
        /// </summary>
        private StringBuilder EntireData = new StringBuilder();

        /// <summary>
        /// RSS feed signature.
        /// </summary>
        public static readonly string RSS_FEED = "<rss ";

        /// <summary>
        /// ATOM feed signature.
        /// </summary>
        public static readonly string ATOM_FEED = "<feed ";

        /// <summary>
        /// String to indicate the end of the HTTP header.
        /// </summary>
        public static readonly string END_OF_HEADER = "\r\n\r\n";

        /// <summary>
        /// String to indicate the end of a line.
        /// </summary>
        public static readonly string END_OF_LINE = "\r\n";

        /// <summary>
        /// String to indicate that transfer encoding will be chunked.
        /// </summary>
        public static readonly string TRANSFER_CHUNKED = "Transfer-Encoding: chunked";

        /// <summary>
        /// Whether transfer encoding is chunked for this feed.
        /// </summary>
        private bool TransferChunked = false;

        /// <summary>
        /// If a field contains this string at the start, it must be removed.
        /// </summary>
        private const string CDATA_START = "<![CDATA[";

        /// <summary>
        /// If a field contains this string at the end, it must be removed.
        /// </summary>
        private const string CDATA_END = "]]>";

        /// <summary>
        /// Number of maximum update lines show in a showbox.
        /// </summary>
        private const int MAX_UPDATES_PER_SHOWBOX = 10;

        /// <summary>
        /// Default feed retrieval interval in seconds.
        /// </summary>
        private const int DEFAULT_INTERVAL = 15000;

        /// <summary>
        /// The image index for the read icon.
        /// </summary>
        public static readonly int IMAGE_INDEX_READ = 0;

        /// <summary>
        /// The image index for the unread icon.
        /// </summary>
        public static readonly int IMAGE_INDEX_UNREAD = 1;

        /// <summary>
        /// Default sound to play when new feed items are found.
        /// </summary>
        private const string DEFAULT_SOUND = @"C:\aim plugin\update.wav";

        /// <summary>
        /// Minimum feed retrieval interval in seconds.
        /// </summary>
        public static readonly int MIN_INTERVAL = 15;

        /// <summary>
        /// The date format to use everywhere.
        /// </summary>
        public static readonly string DATE_FORMAT = "MM-dd-yyyy HH:mm:ss";

        /// <summary>
        /// The path to where the description of each feed/feed item is saved on the hdd.
        /// </summary>
        public static readonly string PATH_TO_DESCRIPTION = "C:/aim plugin/description.html";

        /// <summary>
        /// String to indicate that there is a feed link following.
        /// </summary>
        private const string SHARE_FEEDS_STRING = "FRSHARE";

        /// <summary>
        /// Start throttling messages after exceeding this threshold.
        /// </summary>
        private const int THROTTLE_THRESHOLD = 50;

        /// <summary>
        /// A dictionary of feeds and their recently added feed items, reset before each update.
        /// </summary>
        private IDictionary<string, IList<FeedItem>> RecentFeeds = new Dictionary<string, IList<FeedItem>>();

        /// <summary>
        /// Messages about new feed items being found, reset before each update.
        /// </summary>
        private IList<string> FeedUpdates = new List<string>();

        /// <summary>
        /// Total number of feed items that were updated.
        /// </summary>
        private int FeedUpdatesCount = 0;

        /// <summary>
        /// Whether the connection thread is currently idle.
        /// </summary>
        private bool CurrentlyIdle = true;

        /// <summary>
        /// The host for the underlying connection.
        /// </summary>
        private string CurrentHost;

        /// <summary>
        /// The port for the underlying connection.
        /// </summary>
        private int CurrentPort;

        /// <summary>
        /// The server path for the underlying connection.
        /// </summary>
        private string CurrentPath;

        /// <summary>
        /// The form listing all the feeds and their items. Can add/delete/edit existing feeds.
        /// </summary>
        private FeedForm FeedsForm = null;

        /// <summary>
        /// The settings form.
        /// </summary>
        private SettingsForm Settings = null;

        /// <summary>
        /// The name of the feed that is currently being updated. If null no feed is currently being updated.
        /// </summary>
        public static string UpdatingFeed = null;

        /// <summary>
        /// The timer used to retrieve the feeds.
        /// </summary>
        public static TimerClass TimerEntity = null;

        /// <summary>
        /// Indicates whether the feed we are currently updating is the last one in the list.
        /// </summary>
        private bool IsLastFeed = false;

        /// <summary>
        /// The plugin key name.
        /// </summary>
        public static string PluginKey = null;

        /// <summary>
        /// Whether we are using a proxy to connect to the feeds.
        /// </summary>
        private bool UseProxy = false;

        /// <summary>
        /// Timer for retrieving feed content.
        /// </summary>
        /// <param name="state">The state of the timer</param>
        private void FeedTimer(Object state)
        {
            try
            {
                if (UpdatingFeed != null)
                {
                    // if the previous feed timer is still updating, quit
                    return;
                }

                // mark that we've begun updating, adding/deleting/editing feeds will be disabled until done
                UpdatingFeed = "";
                FeedUpdatesCount = 0;
                RecentFeeds.Clear();
                FeedUpdates.Clear();
                IsLastFeed = false;
                int count = 0;

                if (Db.FeedData.Count == 0)
                {
                    UpdatingFeed = null;
                }

                // it will go through each added feed and retrieve content for it
                foreach (KeyValuePair<string, Feed> kvp in Db.FeedData)
                {
                    while (!CurrentlyIdle)
                    {
                        // wait for the previous update to finish
                        Thread.Sleep(1000);
                    }

                    // if finished previous connection go get next
                    if (CurrentlyIdle)
                    {
                        TransferChunked = false;
                        if (count == Db.FeedData.Count - 1)
                        {
                            IsLastFeed = true;
                        }

                        Feed feed = kvp.Value;
                        UpdatingFeed = feed.Name;

                        // client part
                        Socket newSock = new Socket(AddressFamily.InterNetwork,
                                              SocketType.Stream, ProtocolType.Tcp);

                        RegistryKey key = Registry.LocalMachine.OpenSubKey(
                            FeedReader.PluginKey + "\\Settings");
                        bool useProxy = (key.GetValue("UseProxy").ToString().Equals("1") ? true : false);
                        UseProxy = useProxy;
                        if (useProxy)
                        {
                            CurrentHost = key.GetValue("ProxyHost").ToString();
                            CurrentPort = Int32.Parse(key.GetValue("ProxyPort").ToString());
                        }
                        else
                        {
                            CurrentHost = feed.Server;
                            CurrentPort = feed.Port;
                            CurrentPath = feed.Path;
                        }

                        IPHostEntry host = Dns.GetHostEntry(CurrentHost);
                        IPEndPoint iep = new IPEndPoint(host.AddressList[0], CurrentPort);
                        CurrentlyIdle = false;
                        newSock.BeginConnect(iep, new AsyncCallback(Connected), newSock);
                        count++;
                    }
                }
            }
            catch (Exception)
            {
                // fail silently
                UpdateState(false);
            }
        }

        /// <summary>
        /// Updates the state of the application when done updating feeds.
        /// </summary>
        /// <param name="success">Whether updating was a success or not</param>
        private void UpdateState(bool success)
        {
            if (success)
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(
                    FeedReader.PluginKey + "\\Settings");

                if (FeedUpdates.Count > 0)
                {
                    // play a sound if there were new items found
                    try
                    {
                        bool soundEnabled = (key.GetValue("SoundEnabled").ToString().Equals("1") ? true : false);
                        if (soundEnabled)
                        {
                            Helper.PlaySound(key.GetValue("Sound").ToString(), 0, 1);
                        }
                    }
                    catch (Exception)
                    {
                        // do nothing
                    }
                }

                // now show update messages if this setting is enabled
                bool alerts = (key.GetValue("Alerts").ToString().Equals("1") ? true : false);
                if (alerts)
                {
                    StringBuilder buffer = new StringBuilder();
                    for (int i = 0; i < FeedUpdates.Count; i++)
                    {
                        if (i > 0 && i % MAX_UPDATES_PER_SHOWBOX == 0)
                        {
                            // show messagebox and clear buffer
                            MessageBox.Show(buffer.ToString(), FeedUpdatesCount + " new feed " + (FeedUpdatesCount == 1 ? "item" : "items") + " found");
                            buffer.Remove(0, buffer.Length);
                        }

                        buffer.Append(FeedUpdates[i]);
                    }

                    if (buffer.Length > 0)
                    {
                        MessageBox.Show(buffer.ToString(), FeedUpdatesCount + " new feed " + (FeedUpdatesCount == 1 ? "item" : "items") + " found");
                    }

                    if (FeedsForm != null && FeedsForm.Visible && FeedUpdates.Count > 0)
                    {
                        FeedsForm.BringToFront();
                        FeedsForm.Focus();
                    }
                }
            }

            // delete feeds that were scheduled to be deleted
            foreach (string feed in Db.ToDelete)
            {
                Db.FeedData.Remove(feed);
                Db.Feeds.Remove(feed);
            }
            // reset "to be deleted" feeds list
            Db.ToDelete.Clear();

            // add newly added feeds to the db
            foreach (KeyValuePair<string, Feed> kvp in Db.NewFeeds)
            {
                Feed feed = kvp.Value;

                Db.FeedData.Add(feed.Name,
                    new Feed(feed.Name, feed.Description, feed.Link, feed.Type));
                Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
            }
            // reset newly added feeds list
            Db.NewFeeds.Clear();

            UpdatingFeed = null;
        }

        /// <summary>
        /// Callback method called once connected to the feed server.
        /// </summary>
        /// <param name="iar">The result</param>
        private void Connected(IAsyncResult iar)
        {
            try
            {
                Client = (Socket)iar.AsyncState;
                Client.EndConnect(iar);

                // if connection was successful begin sending data
                string toSend = null;
                if (UseProxy)
                {
                    toSend = "GET " + Db.FeedData[UpdatingFeed].Link + " HTTP/1.1\r\n\r\n";
                }
                else
                {
                    toSend = "GET " + CurrentPath + " HTTP/1.1\r\n" +
                                    "Host: " + CurrentHost + "\r\n" +
                                    "Port: " + CurrentPort + "\r\n" +
                                    "Connection: Close\r\n\r\n";
                }

                byte[] message = Encoding.ASCII.GetBytes(toSend);
                Client.BeginSend(message, 0, message.Length, SocketFlags.None,
                             new AsyncCallback(SendData), Client);
            }
            catch (Exception)
            {
                CurrentlyIdle = true;
                if (IsLastFeed)
                {
                    UpdateState(false);
                }
            }
        }

        /// <summary>
        /// Callback method called after sending data. Triggers the receving data callback method.
        /// </summary>
        /// <param name="iar">The result</param>
        private void SendData(IAsyncResult iar)
        {
            try
            {
                Socket remote = (Socket)iar.AsyncState;
                if (remote != null && remote.Connected)
                {
                    int sent = remote.EndSend(iar);
                    remote.BeginReceive(Data, 0, Size, SocketFlags.None,
                                  new AsyncCallback(ReceiveData), remote);
                }
            }
            catch (Exception)
            {
                CurrentlyIdle = true;
                if (IsLastFeed)
                {
                    UpdateState(false);
                }
            }
        }

        /// <summary>
        /// Writes the item off to an xm file.
        /// </summary>
        /// <param name="item">The item to write</param>
        private void UpdateXml(FeedItem item, string category)
        {
            TextWriter tw = new StreamWriter(@"E:/apache/htdocs/wap.xml", true);
            tw.WriteLine("<category>" + category + "</category><name>" + item.Name + "</name><desc>" + item.Description + "</desc><date>" + DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1, 0, 0, 0)).TotalSeconds + "</date>");
            tw.Close();
        }

        /// <summary>
        /// The callback method that actually handles received data and parses it.
        /// </summary>
        /// <param name="iar">The result</param>
        private void ReceiveData(IAsyncResult iar)
        {
            try
            {
                // it it was scheduled to be deleted exit the method right here
                if (Db.ToDelete.Contains(UpdatingFeed))
                {
                    // clean up
                    if (EntireData.Length > 0)
                    {
                        EntireData.Remove(0, EntireData.Length);
                    }
                    if (Client != null && Client.Connected)
                    {
                        Client.Close();
                    }

                    CurrentlyIdle = true;
                    if (IsLastFeed)
                    {
                        UpdateState(true);
                    }

                    return;
                }

                Socket remote = (Socket)iar.AsyncState;
                if (remote != null && remote.Connected)
                {
                    int recv = remote.EndReceive(iar);
                    string stringData = null;
                    // use ASCII charset when transfer encoding is chunked because there will be offsets
                    // making the data very hard to parse as UTF8
                    if (TransferChunked)
                    {
                        stringData = Encoding.ASCII.GetString(Data, 0, recv);
                    }
                    else
                    {
                        stringData = Encoding.UTF8.GetString(Data, 0, recv);
                    }

                    if (stringData.IndexOf(TRANSFER_CHUNKED) != -1
                        && stringData.IndexOf(END_OF_HEADER) > stringData.IndexOf(TRANSFER_CHUNKED) &&
                            stringData.IndexOf(END_OF_HEADER) != -1)
                    {
                        TransferChunked = true;
                    }

                    if (stringData.Length > 0)
                    {
                        EntireData.Append(stringData);
                        Client.BeginReceive(Data, 0, Size, SocketFlags.None,
                                                      new AsyncCallback(ReceiveData), Client);
                    }
                    else
                    {
                        string buffer = EntireData.ToString();
                        string xml = null;
                        if (buffer.IndexOf(TRANSFER_CHUNKED) != -1 &&
                            buffer.IndexOf(END_OF_HEADER) > buffer.IndexOf(TRANSFER_CHUNKED) &&
                            buffer.IndexOf(END_OF_HEADER) != -1)
                        {
                            xml = Helper.ProcessChunks(buffer, END_OF_HEADER, END_OF_LINE);
                        }
                        else
                        {
                            xml = buffer;
                        }

                        byte[] bytes = Encoding.UTF8.GetBytes(xml);
                        xml = Encoding.UTF8.GetString(bytes);

                        string title = null;
                        string description = null;
                        string link = null;
                        RecentFeeds.Add(UpdatingFeed, new List<FeedItem>());

                        if (xml.IndexOf(RSS_FEED) != -1) // feed is rss
                        {
                            xml = xml.Substring(xml.IndexOf(RSS_FEED));

                            // parse the xml
                            XmlDocument xDoc = new XmlDocument();
                            xDoc.LoadXml(xml);
                            XmlNodeList nodes = xDoc.SelectNodes("rss/channel/item/*");

                            for (int i = 0; i < nodes.Count; i++)
                            {
                                // property nodes: title, link, description/content:encoded
                                if (title == null &&
                                    nodes[i].Prefix.Trim().Length == 0 &&
                                    nodes[i].LocalName.ToLower().Equals("title"))
                                {
                                    title = Helper.ReplaceHtmlCodes(Helper.RemoveCDATA(nodes[i].InnerXml.Trim(), CDATA_START, CDATA_END));
                                    if (title.Trim().Length == 0)
                                    {
                                        title = "None specified";
                                    }
                                }

                                if (link == null &&
                                    nodes[i].Prefix.Trim().Length == 0 &&
                                    nodes[i].LocalName.ToLower().Equals("link"))
                                {
                                    link = Helper.ReplaceHtmlCodes(Helper.RemoveCDATA(nodes[i].InnerXml.Trim(), CDATA_START, CDATA_END));
                                    if (link.Trim().Length == 0)
                                    {
                                        link = "None specified";
                                    }
                                }

                                if (description == null &&
                                    (nodes[i].Prefix.Trim().Length == 0 &&
                                        (nodes[i].LocalName.ToLower().Equals("description") ||
                                        nodes[i].LocalName.ToLower().Equals("content"))) ||
                                    (nodes[i].Prefix.ToLower().Equals("content") &&
                                        nodes[i].LocalName.ToLower().Equals("encoded")))
                                {
                                    description = Helper.RemoveCDATA(nodes[i].InnerXml.Trim(), CDATA_START, CDATA_END).Replace("&amp;", "&");
                                    if (description.Trim().Length == 0)
                                    {
                                        description = "None specified";
                                    }
                                }

                                // got all the data, add the item
                                if (title != null && link != null && description != null)
                                {
                                    FeedItem item = new FeedItem(title, description, link);
                                    item.Date = DateTime.Now;
                                    RecentFeeds[UpdatingFeed].Add(item);

                                    title = null;
                                    link = null;
                                    description = null;

                                    // go on to next item node
                                    continue;
                                }
                            }

                            nodes = xDoc.SelectNodes("rss/channel/description");
                            if (nodes.Count == 1)
                            {
                                Db.FeedData[UpdatingFeed].Description = Helper.RemoveCDATA(nodes[0].InnerXml.Trim(), CDATA_START, CDATA_END);
                            }
                            else
                            {
                                Db.FeedData[UpdatingFeed].Description = "None specified";
                            }

                            Db.FeedData[UpdatingFeed].Type = "RSS";
                        }
                        else if (xml.IndexOf(ATOM_FEED) != -1)
                        {
                            xml = xml.Substring(xml.IndexOf(ATOM_FEED));

                            // parse the xml
                            XmlDocument xDoc = new XmlDocument();
                            xDoc.LoadXml(xml);
                            XmlNodeList nodes = xDoc.FirstChild.ChildNodes;
                            Db.FeedData[UpdatingFeed].Description = "None specified";

                            for (int i = 0; i < nodes.Count; i++)
                            {
                                if (nodes[i].Prefix.Trim().Length == 0 &&
                                    nodes[i].LocalName.ToLower().Equals("entry"))
                                {
                                    foreach (XmlNode subNode in nodes[i].ChildNodes)
                                    {
                                        // property nodes: title, link, content
                                        if (subNode.Prefix.Trim().Length == 0 &&
                                            subNode.LocalName.ToLower().Equals("title"))
                                        {
                                            title = Helper.ReplaceHtmlCodes(Helper.RemoveCDATA(subNode.InnerXml.Trim(), CDATA_START, CDATA_END));
                                            if (title.Trim().Length == 0)
                                            {
                                                title = "None specified";
                                            }
                                        }

                                        if (subNode.Prefix.Trim().Length == 0 &&
                                            subNode.LocalName.ToLower().Equals("link") &&
                                            subNode.Attributes["href"] != null)
                                        {
                                            link = Helper.ReplaceHtmlCodes(Helper.RemoveCDATA(subNode.Attributes["href"].InnerText.Trim(),
                                                CDATA_START, CDATA_END));
                                            if (link.Trim().Length == 0)
                                            {
                                                link = "None specified";
                                            }
                                        }

                                        if ((subNode.Prefix.Trim().Length == 0 &&
                                            subNode.LocalName.ToLower().Equals("content")) ||
                                            (subNode.Prefix.ToLower().Equals("content") &&
                                                subNode.LocalName.ToLower().Equals("encoded")))
                                        {
                                            description = Helper.RemoveCDATA(subNode.InnerXml.Trim(),
                                                CDATA_START, CDATA_END).Replace("&amp;", "&");
                                            if (description.Trim().Length == 0)
                                            {
                                                description = "None specified";
                                            }
                                        }

                                        // got all the data, add the item
                                        if (title != null && link != null && description != null)
                                        {
                                            FeedItem item = new FeedItem(title, description, link);
                                            item.Date = DateTime.Now;
                                            RecentFeeds[UpdatingFeed].Add(item);

                                            title = null;
                                            link = null;
                                            description = null;

                                            break;
                                        }
                                    }
                                }
                                else if (nodes[i].Prefix.Trim().Length == 0 &&
                                    nodes[i].LocalName.ToLower().Equals("subtitle"))
                                {
                                    Db.FeedData[UpdatingFeed].Description = Helper.RemoveCDATA(nodes[i].InnerText.Trim(), CDATA_START, CDATA_END);
                                }
                            }

                            Db.FeedData[UpdatingFeed].Type = "ATOM";
                        }

                        if (Db.FeedData[UpdatingFeed].Description.Trim().Length == 0)
                        {
                            Db.FeedData[UpdatingFeed].Description = "None specified";
                        }

                        // update type and description in registry too
                        RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + UpdatingFeed, true);
                        key.SetValue("Type", Db.FeedData[UpdatingFeed].Type);
                        key.SetValue("Description", Db.FeedData[UpdatingFeed].Description);
                        // update last update
                        Db.FeedData[UpdatingFeed].LastUpdate = String.Format("{0:" + DATE_FORMAT + "}", DateTime.Now);
                        key.SetValue("LastUpdate", Db.FeedData[UpdatingFeed].LastUpdate);

                        if (FeedsForm != null && FeedsForm.Visible && FeedsForm.SelectedFeed != null
                            && FeedsForm.SelectedFeed.Equals(UpdatingFeed))
                        {
                            // update description and feed type in the form as well
                            // create a writer and open the file
                            TextWriter tw = new StreamWriter(new FileStream(FeedReader.PATH_TO_DESCRIPTION,
                                FileMode.Create),
                                Encoding.UTF8);
                            // write a line of text to the file
                            tw.WriteLine("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>" + Helper.DecodeSpecialChars(Db.FeedData[UpdatingFeed].Description) + "</body></html>");
                            // close the stream
                            tw.Close();

                            FeedsForm.WebBrowser.Navigate("file:///" + Helper.EncodeURI(PATH_TO_DESCRIPTION));
                            FeedsForm.FeedType.Text = "Feed type: " + Db.FeedData[UpdatingFeed].Type;
                            FeedsForm.LastUpdate.Text = "Last update: " + Db.FeedData[UpdatingFeed].LastUpdate;
                        }

                        int recentCount = 0;
                        IList<FeedItem> toRemove = new List<FeedItem>();

                        foreach (FeedItem item in RecentFeeds[UpdatingFeed])
                        {
                            if (!Helper.ItemExists(Db.Feeds[UpdatingFeed], item))
                            {
                                recentCount++;
                                item.Name = Helper.GetNewItemName(item.Name, Db.Feeds[UpdatingFeed]);
                                Db.Feeds[UpdatingFeed].Add(
                                    item.Name,
                                    new FeedItem(
                                        item.Name, item.Description, item.Link)
                                );
                            }
                            else
                            {
                                toRemove.Add(item);
                            }
                        }

                        EntireData.Remove(0, EntireData.Length);
                        Client.Close();

                        if (recentCount > 0)
                        {
                            foreach (FeedItem item in toRemove)
                            {
                                RecentFeeds[UpdatingFeed].Remove(item);
                            }

                            FeedUpdatesCount += recentCount;
                            FeedUpdates.Add("Found " + recentCount + " new " + (recentCount == 1 ? "item" : "items") +
                                " in feed " + UpdatingFeed + "\r\n");

                            // check if we need to update status or if we need to notify anyone
                            if (RecentFeeds[UpdatingFeed].Count > 0)
                            {
                                if (Db.FeedData[UpdatingFeed].UpdateStatus)
                                {
                                    FeedItem item = RecentFeeds[UpdatingFeed][0];
                                    m_session.set_Property(AccSessionProp.AccSessionProp_StatusText, item.Name + " - " + item.Link + " @ " + item.Date);
                                }

                                if (Db.FeedData[UpdatingFeed].NotifyEnabled)
                                {
                                    foreach (string buddy in Db.FeedData[UpdatingFeed].NotifyList)
                                    {
                                        IAccImSession imSess = m_session.CreateImSession(buddy, AccImSessionType.AccImSessionType_Im);
                                        string msg = "is 1 new item";
                                        if (RecentFeeds[UpdatingFeed].Count > 1)
                                        {
                                            msg = "are " + RecentFeeds[UpdatingFeed].Count + " new items";
                                        }

                                        imSess.SendIm(m_session.CreateIm("Hey there! There " + msg + " in feed [" + UpdatingFeed + "] @ " + Db.FeedData[UpdatingFeed].Link + " published on " + Db.FeedData[UpdatingFeed].LastUpdate + ".", null));
                                        imSess.EndSession();
                                    }
                                }

                                // write everything we found off to an xml file
                                foreach (KeyValuePair<string, IList<FeedItem>> kvp in RecentFeeds)
                                {
                                    foreach (FeedItem item in kvp.Value)
                                    {
                                        UpdateXml(item, UpdatingFeed);
                                    }
                                }
                            }

                            if (FeedsForm != null && FeedsForm.Visible)
                            {
                                // it it was scheduled to be deleted exit the method right here
                                if (Db.ToDelete.Contains(UpdatingFeed))
                                {
                                    // clean up
                                    if (EntireData.Length > 0)
                                    {
                                        EntireData.Remove(0, EntireData.Length);
                                    }
                                    if (Client != null && Client.Connected)
                                    {
                                        Client.Close();
                                    }

                                    CurrentlyIdle = true;
                                    if (IsLastFeed)
                                    {
                                        UpdateState(true);
                                    }

                                    return;
                                }

                                // not idle yet, have to wait for updating form
                                FeedsForm.Invoke(new MethodInvoker(FormUpdate));
                            }
                            else
                            {
                                // it it was scheduled to be deleted exit the method right here
                                if (Db.ToDelete.Contains(UpdatingFeed))
                                {
                                    // clean up
                                    if (EntireData.Length > 0)
                                    {
                                        EntireData.Remove(0, EntireData.Length);
                                    }
                                    if (Client != null && Client.Connected)
                                    {
                                        Client.Close();
                                    }

                                    CurrentlyIdle = true;
                                    if (IsLastFeed)
                                    {
                                        UpdateState(true);
                                    }

                                    return;
                                }

                                // the update was done in the Update method, but since it won't get called in this case
                                // we have to do it here
                                foreach (FeedItem item in RecentFeeds[UpdatingFeed])
                                {
                                    // also add it to registry
                                    RegistryKey feedKey = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + UpdatingFeed + "\\Items", true);
                                    RegistryKey itemKey = feedKey.CreateSubKey(item.Name);
                                    itemKey.SetValue("Name", item.Name);
                                    itemKey.SetValue("Description", item.Description);
                                    itemKey.SetValue("Link", item.Link);
                                    itemKey.SetValue("IsRead", "0");
                                    itemKey.SetValue("Date", String.Format("{0:" + DATE_FORMAT + "}", item.Date));
                                }

                                CurrentlyIdle = true; // no form updating required, idle

                                if (IsLastFeed)
                                {
                                    UpdateState(true);
                                }
                            }
                        }
                        else
                        {
                            CurrentlyIdle = true; // idle now
                            if (IsLastFeed)
                            {
                                UpdateState(true);
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("An error occurred while updating feed '" + UpdatingFeed + "': "
                    + e.Message + e.StackTrace);

                // clean up
                if (EntireData.Length > 0)
                {
                    EntireData.Remove(0, EntireData.Length);
                }
                if (Client != null && Client.Connected)
                {
                    Client.Close();
                }

                CurrentlyIdle = true;
                if (IsLastFeed)
                {
                    UpdateState(false);
                }
            }
        }

        /// <summary>
        /// Callback method for updating the feeds form (nodes) when new items are added.
        /// </summary>
        void FormUpdate()
        {
            if (FeedsForm == null || !FeedsForm.Visible)
            {
                return;
            }

            FeedsForm.UpdateTreeView(FeedsForm.Filter.Text, FeedsForm.Search.Text);

            foreach (FeedItem item in RecentFeeds[UpdatingFeed])
            {
                // also add it to registry
                RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + UpdatingFeed + "\\Items", true);
                RegistryKey itemKey = key.CreateSubKey(item.Name);
                itemKey.SetValue("Name", item.Name);
                itemKey.SetValue("Description", item.Description);
                itemKey.SetValue("Link", item.Link);
                itemKey.SetValue("IsRead", "0");
                itemKey.SetValue("Date", String.Format("{0:" + DATE_FORMAT + "}", item.Date));
            }

            // check if we need to update status or if we need to notify anyone
            if (RecentFeeds[UpdatingFeed].Count > 0)
            {
                if (Db.FeedData[UpdatingFeed].UpdateStatus)
                {
                    FeedItem item = RecentFeeds[UpdatingFeed][0];
                    m_session.set_Property(AccSessionProp.AccSessionProp_StatusText, item.Name + " - " + item.Link + " @ " + item.Date);
                }

                if (Db.FeedData[UpdatingFeed].NotifyEnabled)
                {
                    foreach (string buddy in Db.FeedData[UpdatingFeed].NotifyList)
                    {
                        IAccImSession imSess = m_session.CreateImSession(buddy, AccImSessionType.AccImSessionType_Im);
                        string msg = "is 1 new item";
                        if (RecentFeeds[UpdatingFeed].Count > 1)
                        {
                            msg = "are " + RecentFeeds[UpdatingFeed].Count + " new items";
                        }

                        imSess.SendIm(m_session.CreateIm("Hey there! There " + msg + " in feed [" + UpdatingFeed + "] @ " + Db.FeedData[UpdatingFeed].Link + " published on " + Db.FeedData[UpdatingFeed].LastUpdate + ".", null));
                        imSess.EndSession();
                    }
                }
            }

            CurrentlyIdle = true;
            if (IsLastFeed)
            {
                UpdateState(true);
            }
        }
        #endregion

        #region IAccPlugin Members

        /// <summary>
        /// Used so we can parse the received message.
        /// </summary>
        /// <param name="session">The session</param>
        /// <param name="imSession">The IM session</param>
        /// <param name="user">The user who sent the message</param>
        /// <param name="im">The message, as it was received</param>
        public void m_session_OnImReceived(AccSession session,
            IAccImSession imSession, IAccParticipant user, IAccIm im)
        {
            string matchPattern = "\\<A HREF=\"(.+?)\"\\>";
            Regex re = new Regex(matchPattern, RegexOptions.Multiline | RegexOptions.IgnoreCase);
            MatchCollection theMatches = re.Matches(im.Text);

            // check for proxy support
            RegistryKey settingsKey = Registry.LocalMachine.OpenSubKey(
                FeedReader.PluginKey + "\\Settings");
            bool useProxy = (settingsKey.GetValue("UseProxy").ToString().Equals("1") ? true : false);
            string proxyHost = null;
            int proxyPort = 0;
            if (useProxy)
            {
                proxyHost = settingsKey.GetValue("ProxyHost").ToString();
                proxyPort = Int32.Parse(settingsKey.GetValue("ProxyPort").ToString());
            }

            bool anyGood = false;
            for (int i = 0; i < theMatches.Count; i++)
            {
                string link = theMatches[i].Groups[1].Value;
                Feed feed = null;
                try
                {
                    feed = new Feed(
                            link,
                            "None specified",
                            link,
                            "NONE");
                }
                catch (Exception)
                {
                    // feed invalid, skip
                    continue;
                }

                // the link will not be added if a feed with that name/link already exists
                // so check this first
                // also check if the link is indeed a feed
                string type = feed.Type;

                if (!Helper.IsGoodFeed(feed, Db.FeedData, Db.NewFeeds, out type, useProxy, proxyHost, proxyPort))
                {
                    continue;
                }

                anyGood = true;
                feed.Type = type;

                // this process will be repeated for every valid feed found in the message
                if (MessageBox.Show(
                    "Would you like to add this feed (" + link + ") to your feed list?",
                    "New feed", MessageBoxButtons.YesNo).Equals(DialogResult.Yes))
                {
                    // update registry
                    Helper.CreateFeedKey(feed);

                    // check if update is in progress, if it is add the feed to a temp list
                    if (FeedReader.UpdatingFeed != null)
                    {
                        Db.NewFeeds.Add(feed.Name, feed);
                    }
                    else
                    {
                        Db.FeedData.Add(feed.Name,
                            new Feed(feed.Name, feed.Description, feed.Link, feed.Type));
                        Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
                    }
                }
            }

            // add the node too
            if (anyGood && FeedsForm != null && FeedsForm.Visible)
            {
                FeedsForm.UpdateTreeView(FeedsForm.Filter.Text, FeedsForm.Search.Text);
            }
        }

        /// <summary>
        /// Handles shared feeds.
        /// </summary>
        /// <param name="sess"></param>
        /// <param name="custSess"></param>
        /// <param name="part"></param>
        /// <param name="im"></param>
        private void m_session_BeforeImReceived(AccSession sess, IAccImSession custSess, IAccParticipant part,
            IAccIm im)
        {
            string matchPattern = SHARE_FEEDS_STRING + " (.+)";
            Regex re = new Regex(matchPattern, RegexOptions.Multiline | RegexOptions.IgnoreCase);
            MatchCollection theMatches = re.Matches(im.Text);

            // check for proxy support
            RegistryKey settingsKey = Registry.LocalMachine.OpenSubKey(
                FeedReader.PluginKey + "\\Settings");
            bool useProxy = (settingsKey.GetValue("UseProxy").ToString().Equals("1") ? true : false);
            string proxyHost = null;
            int proxyPort = 0;
            if (useProxy)
            {
                proxyHost = settingsKey.GetValue("ProxyHost").ToString();
                proxyPort = Int32.Parse(settingsKey.GetValue("ProxyPort").ToString());
            }

            bool anyGood = false;
            for (int i = 0; i < theMatches.Count; i++)
            {
                string link = theMatches[i].Groups[1].Value;
                Feed feed = null;
                try
                {
                    feed = new Feed(
                            link,
                            "None specified",
                            link,
                            "NONE");
                }
                catch (Exception)
                {
                    // feed invalid, skip
                    continue;
                }

                // the link will not be added if a feed with that name/link already exists
                // so check this first
                // also check if the link is indeed a feed
                string type = feed.Type;

                if (!Helper.IsGoodFeed(feed, Db.FeedData, Db.NewFeeds, out type, useProxy, proxyHost, proxyPort))
                {
                    continue;
                }

                anyGood = true;
                feed.Type = type;

                // this process will be repeated for every valid feed found in the message
                if (MessageBox.Show(
                    "Would you like to add this feed (" + link + ") to your feed list?",
                    "New feed", MessageBoxButtons.YesNo).Equals(DialogResult.Yes))
                {
                    // update registry
                    Helper.CreateFeedKey(feed);

                    // check if update is in progress, if it is add the feed to a temp list
                    if (FeedReader.UpdatingFeed != null)
                    {
                        Db.NewFeeds.Add(feed.Name, feed);
                    }
                    else
                    {
                        Db.FeedData.Add(feed.Name,
                            new Feed(feed.Name, feed.Description, feed.Link, feed.Type));
                        Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
                    }
                }
            }

            // add the node too
            if (anyGood && FeedsForm != null && FeedsForm.Visible)
            {
                FeedsForm.UpdateTreeView(FeedsForm.Filter.Text, FeedsForm.Search.Text);
            }
        }

        /// <summary>
        /// Called on plugin initialization.
        /// </summary>
        /// <param name="session">The session</param>
        /// <param name="pluginInfo">The plugin info</param>
        public void Init(AccSession session, IAccPluginInfo pluginInfo)
        {
            try
            {
                FeedReader.Initializing = true;

                m_session = session;

                PluginKey = PluginKeyName(this.GetType());

                // register IM event
                m_session.OnImReceived += new DAccEvents_OnImReceivedEventHandler(m_session_OnImReceived);
                m_session.BeforeImReceived += new DAccEvents_BeforeImReceivedEventHandler(m_session_BeforeImReceived);
                // get the feeds from registry
                RegistryKey key = Registry.LocalMachine.OpenSubKey(PluginKey + "\\Feeds");
                IComparer<FeedItem> comparer = new FeedItemComparer();
                foreach (string subKeyName in key.GetSubKeyNames())
                {
                    RegistryKey subKey = key.OpenSubKey(subKeyName);
                    Feed feed = new Feed(subKey.GetValue("Name").ToString(),
                                 subKey.GetValue("Description").ToString(),
                                 subKey.GetValue("Link").ToString(),
                                 subKey.GetValue("Type").ToString());
                    feed.LastUpdate = subKey.GetValue("LastUpdate").ToString();
                    feed.UpdateStatus = subKey.GetValue("UpdateStatus").Equals("1") ? true : false;
                    feed.NotifyEnabled = subKey.GetValue("NotifyEnabled").Equals("1") ? true : false;

                    Db.FeedData.Add(subKey.GetValue("Name").ToString(),
                        feed);

                    // now get feed items for each feed
                    RegistryKey itemsKey = subKey.OpenSubKey("Items");
                    if (itemsKey.SubKeyCount > 0)
                    {
                        string[] keyNames = itemsKey.GetSubKeyNames();
                        FeedItem[] arrayItems = new FeedItem[keyNames.Length];
                        int i = 0;
                        foreach (string feedItem in keyNames)
                        {
                            RegistryKey itemKey = itemsKey.OpenSubKey(feedItem);

                            // name, description, link for each
                            FeedItem item = new FeedItem(
                                   itemKey.GetValue("Name").ToString(),
                                   itemKey.GetValue("Description").ToString(),
                                   itemKey.GetValue("Link").ToString());

                            string dateString = itemKey.GetValue("Date").ToString();
                            string[] parts = dateString.Split(' ');
                            string[] date = parts[0].Split('-');
                            string[] time = parts[1].Split(':');
                            item.Date = new DateTime(
                                Int32.Parse(date[2]),
                                Int32.Parse(date[0]),
                                Int32.Parse(date[1]),
                                Int32.Parse(time[0]),
                                Int32.Parse(time[1]),
                                Int32.Parse(time[2]));

                            item.IsRead = itemKey.GetValue("IsRead").ToString().Equals("1") ? true : false;

                            arrayItems[i++] = item;
                        }

                        // order them by date
                        Array.Sort(arrayItems, comparer);

                        IDictionary<string, FeedItem> items = new Dictionary<string, FeedItem>();
                        Db.Feeds.Add(feed.Name, items);
                        foreach (FeedItem item in arrayItems)
                        {
                            items.Add(item.Name, item);
                        }
                    }
                    else
                    {
                        Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
                    }

                    // get notify list
                    RegistryKey notifyKey = subKey.OpenSubKey("NotifyList");
                    if (notifyKey.SubKeyCount > 0)
                    {
                        string[] keyNames = notifyKey.GetSubKeyNames();
                        foreach (string keyName in keyNames)
                        {
                            feed.NotifyList.Add(keyName);
                        }
                    }
                }

                // add the feeds command
                IAccCommand myFeeds = pluginInfo.AddCommand(kMyFeedsId);
                myFeeds.set_Property(AccCommandProp.AccCommandProp_Text, "My Feeds");
                myFeeds.set_Property(AccCommandProp.AccCommandProp_Flags,
                    AccCommandFlags.AccCommandFlags_BuddyUi |
                    AccCommandFlags.AccCommandFlags_ActionsUi |
                    AccCommandFlags.AccCommandFlags_ImSessionUi);

                // add the share feeds command
                IAccCommand shareFeeds = pluginInfo.AddCommand(kShareFeedsId);
                shareFeeds.set_Property(AccCommandProp.AccCommandProp_Text, "Share feeds with this person");
                shareFeeds.set_Property(AccCommandProp.AccCommandProp_Flags,
                    AccCommandFlags.AccCommandFlags_ImSessionUi);

                // create the delegate that the Timer will call
                TimerCallback tc = new TimerCallback(FeedTimer);

                // create the Timer based on the settings
                RegistryKey settings = Registry.LocalMachine.OpenSubKey(PluginKey + "\\Settings");
                TimerEntity = new TimerClass(tc, null,
                    Int32.Parse(settings.GetValue("Interval").ToString()),
                    Int32.Parse(settings.GetValue("Interval").ToString()));

                FeedReader.Initializing = false;
            }
            catch (Exception e)
            {
                MessageBox.Show("There was an error initializing the Feed Reader plug-in: " + e.Message + e.StackTrace);
            }
        }

        /// <summary>
        /// Called on plugin shutdown.
        /// </summary>
        public void Shutdown()
        {
            while (UpdatingFeed != null)
            {
                Application.DoEvents();
                Thread.Sleep(10);
                Application.DoEvents();
            }

             m_session = null;
             // clean up the resources
             TimerEntity.Dispose();
             TimerEntity = null;
        }

        #endregion

        #region Plugin Registration
        /// <summary>
        /// Registers the plugin with AIM.
        /// </summary>
        /// <param name="t">The plugin class.</param>
        [ComRegisterFunctionAttribute]
        public static void RegisterFunction(Type t)
        {
            RegistryKey key = Registry.LocalMachine.CreateSubKey(PluginKeyName(t));
            key.SetValue("Name", t.Name);

            key.CreateSubKey("Feeds"); // feed data will be stored in here
            RegistryKey settings = key.CreateSubKey("Settings"); // settings will be stored in here
            settings.SetValue("Interval", DEFAULT_INTERVAL); // feed update interval
            settings.SetValue("SoundEnabled", "1"); // sound enabled by default
            settings.SetValue("Alerts", "1"); // alerts enabled by default
            settings.SetValue("Sound", DEFAULT_SOUND); // sound to play when new items are found
            settings.SetValue("UseProxy", "0"); // proxy support, turned off by default
            settings.SetValue("ProxyHost", ""); // proxy host
            settings.SetValue("ProxyPort", ""); // proxy port
        }

        /// <summary>
        /// Unregisters the plugin with AIM.
        /// </summary>
        /// <param name="t">The plugin class.</param>
        [ComUnregisterFunctionAttribute]
        public static void UnregisterFunction(Type t)
        {
            Registry.LocalMachine.DeleteSubKeyTree(PluginKeyName(t));
        }

        /// <summary>
        /// Returns the plugin key name in the registry.
        /// </summary>
        /// <param name="t">The plugin class.</param>
        /// <returns>The plugin key name</returns>
        internal static string PluginKeyName(Type t)
        {
            return "Software\\America Online\\AIM\\Plugins\\" + 
                '{' + t.GUID.ToString() + '}';
        }
        #endregion

        #region IAccCommandTarget Members
        /// <summary>
        /// Executes the command.
        /// </summary>
        /// <param name="command">The command to execute.</param>
        /// <param name="users">The users.</param>
        public void Exec(int command, object users)
        {
            if (command == kMyFeedsId)
            {
                // show the feeds form
                if (FeedsForm == null || !FeedsForm.Visible)
                {
                    FeedsForm = new FeedForm();
                    FeedsForm.UpdateTreeView(FeedsForm.Filter.Text, FeedsForm.Search.Text);
                    FeedsForm.Show();
                }
                else
                {
                    FeedsForm.BringToFront();
                    FeedsForm.Focus();
                }
            }
            else if (command == kShareFeedsId)
            {
                // send the feeds
                int sent = 0;
                IAccImSession imSess = m_session.CreateImSession(((object[])users)[0].ToString(), AccImSessionType.AccImSessionType_Im);
                foreach (KeyValuePair<string, Feed> kvp in Db.FeedData)
                {
                    if (!Db.ToDelete.Contains(kvp.Key))
                    {
                        imSess.SendIm(m_session.CreateIm(SHARE_FEEDS_STRING + " " + kvp.Value.Link, null));
                        sent++;
                        if (sent > 0 && sent % THROTTLE_THRESHOLD == 0)
                        {
                            Thread.Sleep(3000); // pause for 3 seconds before sending more messages
                        }
                    }
                }
                foreach (KeyValuePair<string, Feed> kvp in Db.NewFeeds)
                {
                    imSess.SendIm(m_session.CreateIm(SHARE_FEEDS_STRING + " " + kvp.Value.Link, null));
                    if (sent > 0 && sent % THROTTLE_THRESHOLD == 0)
                    {
                        Thread.Sleep(3000); // pause for 3 seconds before sending more messages
                    }
                }
                imSess.EndSession();
            }
            else if (command == (int)AccCommandId.AccCommandId_Preferences)
            {
                if (Settings == null || !Settings.Visible)
                {
                    Settings = new SettingsForm();
                    RegistryKey key = Registry.LocalMachine.OpenSubKey(
                        FeedReader.PluginKey + "\\Settings");
                    Settings.Interval.Text = (Int32.Parse(key.GetValue("Interval").ToString()) / 1000).ToString();
                    Settings.Sound.Text = key.GetValue("Sound").ToString();
                    bool useProxy = (key.GetValue("UseProxy").ToString().Equals("1") ? true : false);
                    Settings.UseProxy.Checked = useProxy;
                    Settings.ProxyHost.Enabled = useProxy;
                    Settings.ProxyPort.Enabled = useProxy;
                    Settings.ProxyHost.Text = key.GetValue("ProxyHost").ToString();
                    Settings.ProxyPort.Text = key.GetValue("ProxyPort").ToString();
                    bool noAlerts = (key.GetValue("Alerts").ToString().Equals("0") ? true : false);
                    Settings.NoAlerts.Checked = noAlerts;
                    bool soundEnabled = (key.GetValue("SoundEnabled").ToString().Equals("1") ? true : false);
                    Settings.SoundEnabled.Checked = soundEnabled;
                    Settings.Sound.Enabled = soundEnabled;
                    Settings.PickFile.Enabled = soundEnabled;

                    Settings.Show();
                }
                else
                {
                    Settings.BringToFront();
                    Settings.Focus();
                }
            }
        }

        /// <summary>
        /// Queries the status of the command.
        /// </summary>
        /// <param name="command">The command</param>
        /// <param name="users">The users</param>
        /// <returns>True if the command should be executed, false otherwise</returns>
        public bool QueryStatus(int command, object users)
        {
            if (command == (int)AccCommandId.AccCommandId_Preferences ||
                command == kMyFeedsId ||
                command == kShareFeedsId)
                return true;
            else
                return false;
        }
        #endregion
    }
}
