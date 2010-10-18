using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Xml;
using System.Runtime.InteropServices;
using System.IO;
using Microsoft.Win32;

namespace Plugin
{
    /// <summary>
    /// Helper class.
    /// </summary>
    class Helper
    {
        /// <summary>
        /// Whether the connection thread is currently idle.
        /// </summary>
        private static bool CurrentlyIdle = true;

        /// <summary>
        /// Whether the current feed is a valid feed.
        /// </summary>
        private static bool IsFeed = false;

        /// <summary>
        /// The host for the underlying connection.
        /// </summary>
        private static string CurrentHost;

        /// <summary>
        /// The port for the underlying connection.
        /// </summary>
        private static int CurrentPort;

        /// <summary>
        /// The server path for the underlying connection.
        /// </summary>
        private static string CurrentPath;

        /// <summary>
        /// The type of the feed.
        /// </summary>
        private static string FeedType;

        /// <summary>
        /// The socket used when retrieving feeds.
        /// </summary>
        private static Socket Client;

        /// <summary>
        /// Data read from the socket.
        /// </summary>
        private static byte[] Data = new byte[1024];

        /// <summary>
        /// Size of the data read from the socket.
        /// </summary>
        private static readonly int Size = 1024;

        /// <summary>
        /// Data read from each feed, reset every update.
        /// </summary>
        private static StringBuilder EntireData = new StringBuilder();

        /// <summary>
        /// The link to use when retrieving the feed (only used when using a proxy).
        /// </summary>
        private static string Link;

        /// <summary>
        /// Whether to use a proxy or not when connecting to the feed.
        /// </summary>
        private static bool UseProxy = false;

        /// <summary>
        /// Minimum length of the buffer to check if feed is valid.
        /// </summary>
        private static readonly int MINIMUM_LENGTH_FEED_VALID = 1400;

        /// <summary>
        /// Checks given object for validity.
        /// </summary>
        /// <param name="value">The value of the parameter.</param>
        /// <param name="name">The name of the parameter.</param>
        /// <exception cref="ArgumentNullException">If argument is null.</exception>
        internal static void CheckObject(object value, string name)
        {
            if (value == null)
            {
                throw new ArgumentNullException("[" + name + "] cannot be null.");
            }
        }

        /// <summary>
        /// Checks given string for validity.
        /// </summary>
        /// <param name="value">The value of the parameter.</param>
        /// <param name="name">The name of the parameter.</param>
        /// <exception cref="ArgumentException">If argument is empty.</exception>
        /// <exception cref="ArgumentNullException">If argument is null.</exception>
        internal static void CheckString(string value, string name)
        {
            CheckObject(value, name);
            if (value.Trim().Length == 0)
            {
                throw new ArgumentException("[" + name + "] cannot be empty.");
            }
        }

        /// <summary>
        /// Checks given int for validity.
        /// </summary>
        /// <param name="value">The value of the parameter.</param>
        /// <param name="name">The name of the parameter.</param>
        /// <param name="min">The minimum value for the parameter.</param>
        /// <exception cref="ArgumentException">If argument is empty or cannot be parsed or smaller
        /// than min value.</exception>
        /// <exception cref="ArgumentNullException">If argument is null.</exception>
        internal static void CheckInt(string value, string name, int min)
        {
            CheckString(value, name);
            try
            {
                Int32.Parse(value);
            }
            catch (Exception)
            {
                throw new ArgumentException("[" + name + "] cannot be parsed to an integer.");
            }

            if (Int32.Parse(value) < min)
            {
                throw new ArgumentException("[" + name + "] is smaller than the minimum value (" + min + ")");
            }
        }

        /// <summary>
        /// Checks given int for validity.
        /// </summary>
        /// <param name="value">The value of the parameter.</param>
        /// <param name="name">The name of the parameter.</param>
        /// <param name="min">The minimum value for the parameter.</param>
        /// <exception cref="ArgumentException">If argument is empty or cannot be parsed or smaller
        /// than min value.</exception>
        /// <exception cref="ArgumentNullException">If argument is null.</exception>
        internal static void CheckInt(string value, string name, int min, int max)
        {
            CheckInt(value, name, min);
            if (Int32.Parse(value) > max)
            {
                throw new ArgumentException("[" + name + "] is bigger than the maximum value (" + max + ")");
            }
        }

        /// <summary>
        /// Checks given parameters for validity.
        /// </summary>
        /// <param name="paramValues">The parameters to check for validity.</param>
        /// <param name="paramNames">The names of the parameters to check for validity.</param>
        /// <exception cref="ArgumentException">If an argument is empty.</exception>
        /// <exception cref="ArgumentNullException">If an argument is null.</exception>
        internal static void CheckParameters(object[] paramValues, string[] paramNames)
        {
            for (int i = 0; i < paramValues.Length; i++)
            {
                if (paramValues[i] is string)
                {
                    CheckString(paramValues[i].ToString(), paramNames[i]);
                }
                else
                {
                    CheckObject(paramValues[i], paramNames[i]);
                }
            }
        }

        /// <summary>
        /// Decodes the string, converting special html strings, such as `&lt;' and `&gt'.
        /// </summary>
        /// <param name="str">The string to decode</param>
        /// <returns>The decoded string</returns>
        internal static string DecodeSpecialChars(string str)
        {
            return str.Replace("&lt;", "<").Replace("&gt;", ">");
        }

        /// <summary>
        /// Replace the characters in this string with URI specific characters.
        /// </summary>
        /// <param name="str">The string to encode</param>
        /// <returns>The encoded string</returns>
        internal static string EncodeURI(string str)
        {
            return str.Replace(" ", "%20");
        }

        /// <summary>
        /// Whether the given character is a hex code character.
        /// </summary>
        /// <param name="str">The character to check</param>
        /// <returns>True if it is a hex character, false otherwise</returns>
        internal static bool IsHex(char str)
        {
            string hexChars = "abcdef0123456789";
            return (hexChars.IndexOf(str) != -1);
        }

        /// <summary>
        /// Assemble the chunks of data from the buffer into a readable feed response.
        /// </summary>
        /// <param name="buffer">The buffer to assemble the chunks from</param>
        /// <param name="eoh">The string that indicates the end of the header</param>
        /// <param name="eol">The string that indicates the end of a line</param>
        /// <returns>The assembled chunks of data</returns>
        internal static string ProcessChunks(string buffer, string eoh, string eol)
        {
            int content = buffer.IndexOf(eoh) + eoh.Length; // marks the end of the header and beginning of actual data
            buffer = buffer.Substring(content);

            // now retrieve the chunks from the content
            StringBuilder sb = new StringBuilder();

            // get the length of the chunk
            while (buffer.Length > 0)
            {
                string strlength = "";
                string data = buffer.Substring(0, buffer.IndexOf(eol));
                int i = 0;

                while (i < data.Length && IsHex(data[i]))
                {
                    strlength += data[i++];
                }

                uint length = Convert.ToUInt32(strlength, 16);

                if (length > 0)
                {
                    // get the actual chunk
                    sb.Append(buffer.Substring(buffer.IndexOf(eol) + eol.Length, (int)length));
                    // advance the position, there is another CRLF after the content data
                    buffer = buffer.Substring(buffer.IndexOf(eol) + eol.Length + (int)length + eol.Length);
                }
                else
                {
                    break;
                }

                // and repeat till length is 0 or end of string is reached
            }

            return sb.ToString();
        }

        /// <summary>
        /// Checks whether a feed with this link already exists in the feed list.
        /// </summary>
        /// <param name="link">The link of the feed</param>
        /// <param name="data">The list of feeds in which to check</param>
        /// <param name="checkDeleted">Whether to check if the feed was scheduled for deletion</param>
        /// <returns>True if it already exists, false otherwise</returns>
        internal static bool LinkExists(string link, IDictionary<string, Feed> data, bool checkDeleted)
        {
            foreach (KeyValuePair<string, Feed> kvp in data)
            {
                if (kvp.Value.Link.ToLower().Equals(link.ToLower()) &&
                    ((checkDeleted && !Db.ToDelete.Contains(kvp.Key)) || !checkDeleted))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Removes the CDATA tags in the given string.
        /// </summary>
        /// <param name="str">The string to use</param>
        /// <param name="start">The start CDATA tag</param>
        /// <param name="end">The end CDATA tag</param>
        /// <returns>The string with the removed CDATA tags</returns>
        internal static string RemoveCDATA(string str, string start, string end)
        {
            return str.Replace(start, "").Replace(end, "");
        }

        /// <summary>
        /// Checks whether a given feed can be added to the feed list.
        /// </summary>
        /// <param name="feed">The actual feed</param>
        /// <param name="data">The list of existing feeds</param>
        /// <param name="type">The type of the feed will be stored in here</param>
        /// <param name="useProxy">Whether to use a proxy or not</param>
        /// <param name="proxyHost">The proxy host to connect to</param>
        /// <param name="proxyPort">The proxy port to connect to</param>
        /// <returns>True if the link can be added, false otherwise</returns>
        internal static bool IsGoodFeed(Feed feed, IDictionary<string, Feed> data,
            IDictionary<string, Feed> newFeeds, out string type, bool useProxy, string proxyHost,
            int proxyPort)
        {
            type = null;

            // check if a feed with this name already exists
            if (data.ContainsKey(feed.Name) && !Db.ToDelete.Contains(feed.Name))
            {
                return false;
            }

            // check if a feed with this link already exists
            if (LinkExists(feed.Link, data, true))
            {
                return false;
            }

            // check if a feed with this link already exists in the new feeds
            if (LinkExists(feed.Link, newFeeds, false))
            {
                return false;
            }

            // now check if the link is a valid feed (RSS or ATOM)
            // client part
            Socket newSock = new Socket(AddressFamily.InterNetwork,
                                  SocketType.Stream, ProtocolType.Tcp);

            UseProxy = useProxy;
            if (useProxy)
            {
                CurrentHost = proxyHost;
                CurrentPort = proxyPort;
                Link = feed.Link;
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

            newSock.BeginConnect(iep, new AsyncCallback(Helper.Connected), newSock);

            // wait for the connect thread to finish
            while (!CurrentlyIdle)
            {
                Application.DoEvents();
                Thread.Sleep(100);
            }

            type = FeedType;

            return IsFeed;
        }

        /// <summary>
        /// PlaySound method, imported from winmm.dll. Supports WAV files.
        /// </summary>
        /// <param name="lpszName">file name</param>
        /// <param name="hModule">module</param>
        /// <param name="dwFlags">flags</param>
        /// <returns>True if succeeded, false otherwise</returns>
        [DllImport("winmm.dll")]
        public static extern bool PlaySound(string lpszName, int hModule, int dwFlags);

        /// <summary>
        /// Gets the directory out of a given path.
        /// </summary>
        /// <param name="path">The path to parse</param>
        /// <returns>The directory</returns>
        internal static string ParseDirectory(string path)
        {
            if (path.LastIndexOf('/') != -1)
            {
                return path.Substring(0, path.LastIndexOf('/'));
            }
            else
            {
                return "";
            }
        }

        /// <summary>
        /// Helper method that parses the given url.
        /// </summary>
        /// <param name="url">The url to parse</param>
        /// <exception cref="ArgumentExcpetion">If there are any problems parsing the url.</exception>
        /// <returns>Returns an array consisting of the server, port and path.</returns>
        internal static string[] ParseUrl(string url)
        {
            // url needs to be in the form http://host:port/path or http://host/path
            // if it's not throw ArgumentException

            if (!url.ToLower().StartsWith("http://"))
            {
                throw new ArgumentException("Invalid url: must start with http://");
            }

            // cut off first part
            url = url.Substring(7);

            if (url.IndexOf('/') == -1)
            {
                throw new ArgumentException("Invalid url: must contain at least one `/' after http://");
            }

            string hostAndPort = url.Substring(0, url.IndexOf('/'));
            if (hostAndPort.Trim().Length == 0)
            {
                throw new ArgumentException("Invalid url: host and port section must not be null");
            }

            string parsedPath = url.Substring(url.IndexOf('/'));

            string parsedPort = "80"; // default port
            string[] parts = hostAndPort.Split(':');
            string parsedServer = parts[0];

            if (parts.Length == 2)
            {
                parsedPort = parts[1];
            }
            else if (parts.Length > 2)
            {
                throw new ArgumentException("Invalid url: only one `:' is allowed in the string");
            }

            return new string[] { parsedServer, parsedPort, parsedPath };
        }

        /// <summary>
        /// Gets the node with this text.
        /// </summary>
        /// <param name="node">The root node of the node we are looking for</param>
        /// <param name="text">The text in the node</param>
        /// <returns>The node or null if none found</returns>
        internal static TreeNode GetNodeByText(TreeNode node, string text)
        {
            foreach (TreeNode subNode in node.Nodes)
            {
                if (subNode.Text.Equals(text))
                {
                    return subNode;
                }
            }

            return null;
        }

        /// <summary>
        /// Removes a node with this text.
        /// </summary>
        /// <param name="form">The form the node resides in</param>
        /// <param name="text">The text in the node</param>
        internal static void RemoveNodeByText(FeedForm form, string text)
        {
            foreach (TreeNode node in form.TreeView.Nodes[0].Nodes)
            {
                if (node.Text.Equals(text))
                {
                    node.Remove();
                    return;
                }
            }
        }

        /// <summary>
        /// If an item with that name already exists it appends a # and a number.
        /// </summary>
        /// <param name="name">The item name</param>
        /// <param name="dict">The dictionary containing item names</param>
        /// <returns>The new item name</returns>
        public static string GetNewItemName(string name, IDictionary<string, FeedItem> dict)
        {
            if (!dict.ContainsKey(name))
            {
                return name;
            }

            int i = 2;
            for (; dict.ContainsKey(name + " #" + i); i++)
            {
                // loop till it's over
            }

            return name + " #" + i;
        }

        /// <summary>
        /// Checks whether the given item exists in the given dictionary.
        /// </summary>
        /// <param name="dict">The dictionary</param>
        /// <param name="item">The item</param>
        /// <returns>True if it exists, false otherwise</returns>
        public static bool ItemExists(IDictionary<string, FeedItem> dict, FeedItem item)
        {
            foreach (KeyValuePair<string, FeedItem> kvp in dict)
            {
                if (kvp.Value.Link.Equals(item.Link) &&
                    kvp.Value.Description.Equals(item.Description))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Replaces html codes such as &nbsp;, &amp;, &quot; from a given string.
        /// </summary>
        /// <param name="input">The string to replace html codes for</param>
        /// <returns>The new string</returns>
        public static string ReplaceHtmlCodes(string input)
        {
            input = input.Replace("&amp;", "&");
            return input.Replace("&nbsp;", " ").Replace("&amp;", "&").Replace("&quot;", "\"").
                Replace("&lt;", "<").Replace("&gt;", ">").Replace("&#160;", " ").Replace("&#38;", "&").
                Replace("&#34;", "\"").Replace("&#60;", "<").Replace("&#62;", ">").Replace("&copy", "©").
                Replace("&#169;", "©").Replace("&#39;", "'");
        }

        /// <summary>
        /// Creates the feed key in the registry.
        /// </summary>
        /// <param name="feed">The feed</param>
        public static void CreateFeedKey(Feed feed)
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds", true);
            RegistryKey newKey = key.CreateSubKey(feed.Name);
            newKey.SetValue("Name", feed.Name);
            newKey.SetValue("Description", feed.Description);
            newKey.SetValue("Link", feed.Link);
            newKey.SetValue("Type", feed.Type);
            newKey.SetValue("LastUpdate", feed.LastUpdate);
            newKey.SetValue("UpdateStatus", feed.UpdateStatus ? "1" : "0");
            newKey.SetValue("NotifyEnabled", feed.NotifyEnabled ? "1" : "0");
            newKey.CreateSubKey("Items");
            newKey.CreateSubKey("NotifyList");
        }

        #region Check if feed is valid
        /// <summary>
        /// Callback method called once connected to the feed server.
        /// </summary>
        /// <param name="iar">The result</param>
        private static void Connected(IAsyncResult iar)
        {
            try
            {
                Client = (Socket)iar.AsyncState;
                Client.EndConnect(iar);

                string toSend = null;
                // if connection was successful begin sending data
                if (UseProxy)
                {
                    toSend = "GET " + Link + " HTTP/1.1\r\n\r\n";
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
                             new AsyncCallback(Helper.SendData), Client);
            }
            catch (Exception)
            {
                IsFeed = false;
                CurrentlyIdle = true;
            }
        }

        /// <summary>
        /// Callback method called after sending data. Triggers the receving data callback method.
        /// </summary>
        /// <param name="iar">The result</param>
        private static void SendData(IAsyncResult iar)
        {
            try
            {
                Socket remote = (Socket)iar.AsyncState;
                if (remote.Connected)
                {
                    int sent = remote.EndSend(iar);
                    remote.BeginReceive(Data, 0, Size, SocketFlags.None,
                                  new AsyncCallback(Helper.ReceiveData), remote);
                }
            }
            catch (Exception)
            {
                IsFeed = false;
                CurrentlyIdle = true;
            }
        }

        /// <summary>
        /// The callback method that actually handles received data and parses it.
        /// </summary>
        /// <param name="iar">The result</param>
        private static void ReceiveData(IAsyncResult iar)
        {
            try
            {
                Socket remote = (Socket)iar.AsyncState;
                if (remote.Connected)
                {
                    int recv = remote.EndReceive(iar);
                    string stringData = Encoding.ASCII.GetString(Data, 0, recv);

                    if (stringData.Length > 0)
                    {
                        EntireData.Append(stringData);
                        string buf = EntireData.ToString();

                        if (buf.IndexOf(FeedReader.ATOM_FEED) != -1 || buf.IndexOf(FeedReader.RSS_FEED) != -1)
                        {
                            // link is good
                            if (buf.IndexOf(FeedReader.ATOM_FEED) != -1)
                            {
                                FeedType = "ATOM";
                            }
                            else if (buf.IndexOf(FeedReader.RSS_FEED) != -1)
                            {
                                FeedType = "RSS";
                            }

                            IsFeed = true;
                            EntireData.Remove(0, EntireData.Length);
                            Client.Close();
                            CurrentlyIdle = true;
                            return;
                        }

                        if (EntireData.Length > MINIMUM_LENGTH_FEED_VALID)
                        {
                            // no RSS and no ATOM tag at this point? then the link is bad
                            if (buf.IndexOf(FeedReader.ATOM_FEED) == -1 && buf.IndexOf(FeedReader.RSS_FEED) == -1)
                            {
                                throw new ArgumentException("Link is bad (1).");
                            }
                            else
                            {
                                // link is good
                                if (buf.IndexOf(FeedReader.ATOM_FEED) != -1)
                                {
                                    FeedType = "ATOM";
                                }
                                else if (buf.IndexOf(FeedReader.RSS_FEED) != -1)
                                {
                                    FeedType = "RSS";
                                }

                                IsFeed = true;
                                EntireData.Remove(0, EntireData.Length);
                                Client.Close();
                                CurrentlyIdle = true;
                                return;
                            }
                        }


                        if (Client.Connected)
                        {
                            Client.BeginReceive(Data, 0, Size, SocketFlags.None,
                                                          new AsyncCallback(Helper.ReceiveData), Client);
                        }
                    }
                    else
                    {
                        throw new ArgumentException("Link is bad (2).");
                    }
                }
            }
            catch (Exception)
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

                IsFeed = false;
                CurrentlyIdle = true;
            }
        }
        #endregion
    }
}
