///----------------------------------------------------------------------------
///
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
///
///----------------------------------------------------------------------------

using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using System.Windows.Forms;
using System.Threading;
using TimerClass = System.Threading.Timer;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Security;

// added the Open AIM Library via
// right click on References and choose
// 'Add References.'  Select the COM tab
// then scroll until you find AccCore 1.0
// type library.
using AccCoreLib;	

namespace acshbuddy08
{
    class Upload
    {
        /// <summary>
        /// The file name.
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
        /// The date the file was uploaded.
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
        /// Initializes the fields.
        /// </summary>
        /// <param name="pname"></param>
        /// <param name="pdate"></param>
        public Upload(string pname, DateTime pdate)
        {
            Name = pname;
            Date = pdate;
        }
    }

    class RunProcess
    {
        private Process process = null;
        public Process Process
        {
            get
            {
                return process;
            }
            set
            {
                process = value;
            }
        }

        private bool killedOff = false;
        public bool KilledOff
        {
            get
            {
                return killedOff;
            }
            set
            {
                killedOff = value;
            }
        }

        public RunProcess(Process proc)
        {
            Process = proc;
        }

        public RunProcess()
        {
            Process = new Process();
        }
    }

    class acshbuddy : Control
    {
        private const string DEFAULT_DOWNLOAD_PATH = "E:\\CodeRemote\\temp";

        private const string BASE_PATH = "E:\\CodeRemote\\";

        private const string TUTORIAL_PATH = "E:\\CodeRemoteTutorial\\";

        private const string SOURCE_MSG = "To view the source code for this example type source";

        private const string NEXT_MSG = "To move on to the next part of the tutorial type next";

        private const string QUIT_TUTORIAL_MSG = "To quit the tutorial at any time type quit";

        private readonly string[] COMPILE_RUN_MSG = {
            "To compile and run this code type run csh Main.cs Function.cs",
            "To compile and run this code type run java tutorial/Main.java tutorial/Function.java",
            "To compile and run this code type run cpp main.cpp func.cpp",
            "To run this script type run php script.php",
            "To run this script type run perl script.pl",
            "To run this script type run python script.py"
        };

        private readonly string[] TUTORIAL_MSG = {
            "\r\nCodeRemote Tutorial Part 1/6\r\n\r\n" +
                        "Hi. This is part 1 of the tutorial. You will learn how to compile and run C# code.\r\n" +
                        "Two files were provided for this purpose in your workspace: Main.cs and Function.cs\r\n\r\n",
            "\r\nCodeRemote Tutorial Part 2/6\r\n\r\n" +
                        "Hi. This is part 2 of the tutorial. You will learn how to compile and run JAVA code.\r\n" +
                        "Two files were provided for this purpose in your workspace: tutorial/Main.java and tutorial/Function.java\r\n\r\n",
            "\r\nCodeRemote Tutorial Part 3/6\r\n\r\n" +
                        "Hi. This is part 3 of the tutorial. You will learn how to compile and run C++ code.\r\n" +
                        "Three files were provided for this purpose in your workspace: main.cpp, func.cpp and header.h\r\n\r\n",
            "\r\nCodeRemote Tutorial Part 4/6\r\n\r\n" +
                        "Hi. This is part 4 of the tutorial. You will learn how to run PHP code.\r\n" +
                        "One file was provided for this purpose in your workspace: script.php\r\n\r\n",
            "\r\nCodeRemote Tutorial Part 5/6\r\n\r\n" +
                        "Hi. This is part 5 of the tutorial. You will learn how to run PERL code.\r\n" +
                        "One file was provided for this purpose in your workspace: script.pl\r\n\r\n",
            "\r\nCodeRemote Tutorial Part 6/6\r\n\r\n" +
                        "Hi. This is part 6, which will conclude our tutorial. You will learn how to run Python code.\r\n" +
                        "One file was provided for this purpose in your workspace: script.py\r\n\r\n"
        };

        private readonly string[][] TUTORIAL_SOURCE_CODE = {
            new string[] { "Main.cs", "Function.cs" },
            new string[] { "tutorial" },
            new string[] { "main.cpp", "func.cpp", "header.h" },
            new string[] { "script.php" },
            new string[] { "script.pl" },
            new string[] { "script.py" }
        };

        private const string PHP_VERSION = "PHP 5.2.4";

        private const string PYTHON_VERSION = "Python 2.5.2";

        private const string PERL_VERSION = "Perl 5.10.0";

        private const string JAVA_VERSION = "JVM 1.5.0_12-b04";

        private const string CPP_VERSION = "Microsoft Visual C++ 2008 Compiler (v15.00.21022.08)";

        private const string CSH_VERSION = "Microsoft Visual C# 2008 Compiler (v3.5.21022.08), .NET Framework v3.5";

        private const string CODEREMOTE_VERSION = "CodeRemote v1.21";

        private const string UPDATES = "Added version command and updated VS to 2008 version (4/29/2008 9:16 PM EEST)";

        // max upload / file
        private const int MAX_UPLOAD_SIZE = 204800;

        // max upload size every 15 minutes
        private const int MAX_TOTAL_UPLOAD_SIZE = 1048576;

        /// <summary>
        /// The timer we will use for clean up and various other things..
        /// </summary>
        private TimerClass timer = null;

        /// <summary>
        /// How often to perform clean up of uploaded files.
        /// </summary>
        private const int CLEANUP_INTERVAL = 900;

        /// <summary>
        /// How often to trigger the timer.
        /// </summary>
        private const int TIMER_INTERVAL = 10;

        /// <summary>
        /// How long to let the process live before we kill it off.
        /// </summary>
        private const int MAX_PROCESS_TIME = 3;

        /// <summary>
        /// Max message length; if it exceeds this, it will be trimmed down, onto multiple messages.
        /// </summary>
        private const int MAX_MESSAGE_BREAK_LENGTH = 512;

        /// <summary>
        /// Max message length; if it exceeds this, it will be trimmed down.
        /// </summary>
        private const int MAX_MESSAGE_LENGTH = 1400;

        /// <summary>
        /// What user to run the process as.
        /// </summary>
        private const string PROCESS_USERNAME = "coderemote";

        private bool useLimitedUser = false;

        /// <summary>
        /// The password for the above user.
        /// </summary>
        private readonly char[] PROCESS_PASSWORD = { '1', '2', '3', '4', '5', '6', 'g', 'g', 'g' };

        private Thread killProcessesThread = null;

        IDictionary<string, int> onTutorial = new Dictionary<string, int>();

        private string m_userName = null;

        private string m_passWord = null;

        /// <summary>
        /// Keeps track of uploaded files so that they may be cleaned up after.
        /// </summary>
        IDictionary<string, Upload> uploads = new Dictionary<string, Upload>();

        IDictionary<string, RunProcess> processes = new Dictionary<string, RunProcess>();

        IDictionary<string, long> uploadSize = new Dictionary<string, long>();

        DateTime lastResetUploadSize = DateTime.Now;

        // add import dll code so we can call C++ Functions from
        // .net code.
        [DllImport("acccore.dll", EntryPoint = "#111", PreserveSig = false)]
        private static extern void AccCreateSession(
            [MarshalAs(UnmanagedType.LPStruct)] Guid riid,
            [MarshalAs(UnmanagedType.IDispatch)] out object session);
        private static acshbuddy a;
        private AccSession s;

        [STAThread]
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("usage: acshbuddy screenname password [limited]");
                return;
            }

            a = new acshbuddy();
            a.Run(args[0], args[1], (args.Length == 3 ? args[2] : "0"));
        }

        /// <summary>
        /// Delete extra files/directories recursively.
        /// </summary>
        /// <param name="path">Current path</param>
        private void DeleteFiles(string path)
        {
            try {
                string[] dirs = Directory.GetDirectories(path);
                string[] files = Directory.GetFiles(path);

                foreach (string dir in dirs)
                {
                    DeleteFiles(dir);
                }

                foreach (string file in files)
                {
                    // not in uploads -> delete it
                    if (!uploads.ContainsKey(file.Substring(BASE_PATH.Length)))
                    {
                        File.Delete(file);
                    }
                }

                // now after having removed all the useless stuff check if the directory is empty, if so remove it too
                if (Directory.GetFiles(path).Length == 0 && Directory.GetDirectories(path).Length == 0)
                {
                    Directory.Delete(path);
                }
            } catch (Exception) {
                // do nothing
            }
        }


        /// <summary>
        /// This timer will do clean up of uploaded files, among others..
        /// </summary>
        /// <param name="state"></param>
        private void BotTimer(object state)
        {
            try
            {
                try
                {
                    DateTime now = DateTime.Now;
                    IList<string> toRemove = new List<string>();

                    // perform clean up
                    foreach (KeyValuePair<string, Upload> kvp in uploads)
                    {
                        // check if it expired
                        TimeSpan ts = now.Subtract(kvp.Value.Date);
                        if (ts.TotalSeconds >= CLEANUP_INTERVAL)
                        {
                            if (File.Exists(BASE_PATH + kvp.Key))
                            {
                                File.Delete(BASE_PATH + kvp.Key);
                            }

                            // now go through each dir and check if it's empty
                            string[] parts = kvp.Key.Split('\\');

                            string dir = "";
                            for (int i = 0; i < parts.Length - 1; i++)
                            {
                                string currentDir = BASE_PATH + dir + parts[i];
                                if (Directory.GetDirectories(currentDir).Length == 0 &&
                                    Directory.GetFiles(currentDir).Length == 0)
                                {
                                    Directory.Delete(currentDir);
                                }

                                dir += parts[i] + "\\";
                            }

                            toRemove.Add(kvp.Key);
                        }
                    }

                    foreach (string file in toRemove)
                    {
                        uploads.Remove(file);
                    }

                    // now check for files created by processes
                    DeleteFiles(BASE_PATH);
                }
                catch (Exception)
                {
                    // do nothing
                }

                // reset uploaded amount every 15min
                if ((DateTime.Now.Subtract(lastResetUploadSize)).TotalSeconds >= CLEANUP_INTERVAL)
                {
                    foreach (KeyValuePair<string, long> kvp in uploadSize)
                    {
                        uploadSize[kvp.Key] = 0;
                    }
                }
            }
            catch (Exception)
            {
                // do nothing
            }
        }

        /// <summary>
        /// This thread will look for active processes and kill them off if they run for too long.
        /// </summary>
        private void KillProcessesJob()
        {
            try
            {
                for (; ; )
                {
                    DateTime now = DateTime.Now;
                    foreach (KeyValuePair<string, RunProcess> kvp in processes)
                    {
                        try
                        {
                            if (!kvp.Value.Process.HasExited)
                            {
                                TimeSpan ts = now.Subtract(kvp.Value.Process.StartTime);
                                if (ts.TotalSeconds >= MAX_PROCESS_TIME)
                                {
                                    kvp.Value.KilledOff = true;
                                    kvp.Value.Process.Kill();
                                    break;
                                }
                            }
                        }
                        catch (Exception)
                        {
                            // do nothing
                        }
                    }
                    Thread.Sleep(500);
                }
            }
            catch (Exception)
            {
                // do nothing
            }
        }

        private void Run(string username, string password, string limitedUser)
        {
            try
            {
                // create control to allow invokes
                CreateControl();

                useLimitedUser = (limitedUser.Equals("1") ? true : false);

                // create and init session
                object o;
                AccCreateSession(typeof(IAccSession).GUID, out o);
                s = (AccSession)o;

                // create the delegate that the Timer will call
                TimerCallback tc = new TimerCallback(BotTimer);

                timer = new TimerClass(tc, null,
                    TIMER_INTERVAL*1000,
                    TIMER_INTERVAL*1000);

                // start the thread that will terminate processes
                ThreadStart job = new ThreadStart(KillProcessesJob);
                killProcessesThread = new Thread(job);
                killProcessesThread.Start();

                // create our preference hook and then pass it
                // to Open AIM
                PrefsHook prefs = new PrefsHook();
                // the folder where to store all of the downloaded files

                prefs.SetValue("aimcc.connect.disableLocalRateLimits", "-1");
                s.PrefsHook = prefs;

                // set up the listeners to AIM events.  Visual Studio will help you with
                // adding these functions by auto completing the functions.
                s.OnStateChange += new DAccEvents_OnStateChangeEventHandler(s_OnStateChange);
                s.OnSecondarySessionStateChange += new DAccEvents_OnSecondarySessionStateChangeEventHandler(s_OnSecondarySessionStateChange);
                s.OnImReceived += new DAccEvents_OnImReceivedEventHandler(s_OnImReceived);

                // file xfer stuff
                s.OnNewFileXfer += new DAccEvents_OnNewFileXferEventHandler(s_OnNewFileXfer);
                s.OnFileXferProgress += new DAccEvents_OnFileXferProgressEventHandler(s_OnFileXferProgress);
                s.OnFileXferComplete += new DAccEvents_OnFileXferCompleteEventHandler(s_OnFileXferComplete);
                s.OnFileXferCollision += new DAccEvents_OnFileXferCollisionEventHandler(s_OnFileXferCollision);

                // here is where we put the name of our client and developer key.
                // you will replace 'acshbuddy' with your client name and replace
                // 'ju1yztKT86VJ0xj3' with your custom client key
                // get your key at http://dev.aol.com/openaim
                s.ClientInfo.set_Property(AccClientInfoProp.AccClientInfoProp_Description,
                    "CodeRemote (key=co1tf1ToCrIxczUe)");
                Console.WriteLine("trying to sign on..");
                // set the user name on the session
                s.Identity = username;
                // sign on to AIM
                m_userName = username;
                m_passWord = password;
                s.SignOn(password);
                Console.WriteLine("successfully signed on!");

                // start main loop
                Application.Run();
            }
            catch (COMException e)
            {
                Console.WriteLine(e.Message);
            }
        }

        void s_OnFileXferCollision(AccSession session, IAccFileXferSession fileXferSession, IAccFileXfer fileXfer)
        {
            Console.WriteLine("file already existed - replacing..");
            fileXferSession.ResolveCollision(AccFileXferSessionCollisionAction.AccFileXferSessionCollisionAction_ReplaceAll,
                fileXfer.get_Property(AccFileXferProp.AccFileXferProp_LocalPath).ToString());
        }

        void s_OnFileXferComplete(AccSession session, IAccFileXferSession fileXferSession, IAccFileXfer fileXfer, AccResult hr)
        {
            try
            {
                string user = fileXferSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName).ToString();
                Console.WriteLine("File transfer complete! (from user: {0})",
                    user);
                long size = Int64.Parse(fileXferSession.get_Property((int)AccFileXferSessionProp.AccFileXferSessionProp_TotalBytes).ToString());

                if (!uploadSize.ContainsKey(user))
                {
                    uploadSize.Add(user, size);
                }
                else
                {
                    uploadSize[user] += size;
                }

                // move the file to where we want it
                string localPath = fileXfer.get_Property(AccFileXferProp.AccFileXferProp_LocalPath).ToString();
                string fileName = user + "\\" + localPath.Substring(DEFAULT_DOWNLOAD_PATH.Length);

                // we need to create all the necessary directories
                string[] parts = fileName.Split('\\');

                string dir = "";
                for (int i = 0; i < parts.Length - 1; i++)
                {
                    string currentDir = BASE_PATH + dir + parts[i];
                    if (!Directory.Exists(currentDir))
                    {
                        Directory.CreateDirectory(currentDir);
                    }

                    dir += parts[i] + "\\";
                }

                // check if it already existed, if so delete it
                if (File.Exists(BASE_PATH + fileName))
                {
                    File.Delete(BASE_PATH + fileName);
                }

                File.Move(localPath,
                    BASE_PATH + fileName);

                // add it to uploads db
                // if it already existed remove it beforehand
                if (uploads.ContainsKey(fileName))
                {
                    uploads.Remove(fileName);
                }
                uploads.Add(fileName, new Upload(fileName, DateTime.Now));

                // now remove the old directory, if any
                if (parts.Length > 2 && Directory.Exists(DEFAULT_DOWNLOAD_PATH + parts[1]))
                {
                    Directory.Delete(DEFAULT_DOWNLOAD_PATH + parts[1], true);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("exception occurred when completing transfer: " + e.Message + e.StackTrace);
            }
        }

        void s_OnFileXferProgress(AccSession session, IAccFileXferSession fileXferSession, IAccFileXfer fileXfer)
        {
            Console.WriteLine("File progress ({0}): {1}/{2}; localpath: {3}",
                fileXfer.get_Property(AccFileXferProp.AccFileXferProp_Name),
                fileXfer.get_Property(AccFileXferProp.AccFileXferProp_BytesXferred),
                fileXfer.get_Property(AccFileXferProp.AccFileXferProp_Bytes),
                fileXfer.get_Property(AccFileXferProp.AccFileXferProp_LocalPath));
        }

        void s_OnNewFileXfer(AccSession session, IAccFileXferSession fileXferSession, IAccFileXfer fileXfer)
        {
            try
            {
                Console.WriteLine("Got a new file Xfer!");
            }
            catch (Exception e)
            {
                Console.WriteLine("exception encountered when trying to start file xfer: {0}", e.Message + e.StackTrace);
            }
        }

        // this is the code that allows our example
        // application to handle commands from the
        // command window
        class InputEvent : System.EventArgs
        {
            public InputEvent(string incommand) { command = incommand; }
            public string Command { get { return command; } }
            private readonly string command;
        };
        private delegate void InputHandler(object o, InputEvent e);

        private void WaitForCommand()
        {
            MethodInvoker mi = new MethodInvoker(GetCommand);
            mi.BeginInvoke(null, null);
        }

        private void GetCommand()
        {
            string command = Console.ReadLine();
            InputEvent e = new InputEvent(command);
            object[] list = { this, e };
            BeginInvoke(new InputHandler(HandleCommand), list);
        }

        private void HandleCommand(object o, InputEvent e)
        {
            string[] tokens = e.Command.Split(' ');
            string verb = tokens[0];

            switch (verb)
            {
                case ":q":
                    s.SignOff();
                    break;
                case "o:v":
                    Console.WriteLine("Client Version: CodeRemote 1.0");
                    break;
                case ":h":
                    Console.WriteLine("o:v                     Lists client version");
                    Console.WriteLine(":q                      signs off of AIM and quits the test application");
                    break;
                default:
                    Console.WriteLine("Invalid command, use :h for list of commands");
                    break;
            }
            WaitForCommand();
        }

        private void s_OnStateChange(AccSession session, AccSessionState state, AccResult hr)
        {
            Console.WriteLine("STATE CHANGE: {0} {1}", state, hr);

            if (state == AccSessionState.AccSessionState_Online)
            {
                Console.WriteLine("...Welcome to AOL Instant Messenger (SM)...");
                WaitForCommand();
            }
            else if (state == AccSessionState.AccSessionState_Offline)
            {
                Thread.Sleep(5000);
                // automatic reconnection
                s.Identity = m_userName;
                s.SignOn(m_passWord);
                /*
                if (killProcessesThread != null)
                {
                    killProcessesThread.Abort();
                }
                Application.Exit();
                */
            }
        }

        private void s_OnSecondarySessionStateChange(AccSession session, IAccSecondarySession piSecSession, AccSecondarySessionState State, AccResult hr)
        {
            try
            {
                // accept all incoming IMs even if the user is not on the buddy list
                if ((AccSecondarySessionServiceId)piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_ServiceId) ==
                    AccSecondarySessionServiceId.AccSecondarySessionServiceId_Im)
                {
                    if (State == AccSecondarySessionState.AccSecondarySessionState_ReceivedProposal)
                    {
                        Console.WriteLine("** acshbuddy has received an IM from {0}.", (string)
                            piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName));
                        piSecSession.Accept();
                    }
                }
                else if ((AccSecondarySessionServiceId)piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_ServiceId) ==
                    AccSecondarySessionServiceId.AccSecondarySessionServiceId_FileXfer)
                {
                    if (State == AccSecondarySessionState.AccSecondarySessionState_ReceivedProposal)
                    {
                        Console.WriteLine("** acshbuddy has received an file xfer request from {0}.", (string)
                            piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName));

                        string fileName = ((IAccFileXferSession)piSecSession).
                            get_Property((int)AccFileXferSessionProp.AccFileXferSessionProp_Name).ToString();
                        string isDir = ((IAccFileXferSession)piSecSession).get_Property((int)AccFileXferSessionProp.AccFileXferSessionProp_IsDirectory).ToString();
                        string size = ((IAccFileXferSession)piSecSession).get_Property((int)AccFileXferSessionProp.AccFileXferSessionProp_TotalBytes).ToString();
                        string user = piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName).ToString();

                        // check the size
                        if (Int64.Parse(size) > MAX_UPLOAD_SIZE)
                        {
                            piSecSession.Reject((int)AccResult.ACC_E_INVALID_DATA);

                            IAccImSession imSess = session.
                                CreateImSession(user,
                                AccImSessionType.AccImSessionType_Im);
                            imSess.SendIm(session.CreateIm("Rejected transfer because file size was bigger than 200 Kilobytes.", "text/plain"));

                            Console.WriteLine("Rejected transfer because file size was too large: " + size);
                            return;
                        }
                        else if (uploadSize.ContainsKey(user) &&
                            uploadSize[user] + Int64.Parse(size) > MAX_TOTAL_UPLOAD_SIZE)
                        {
                            piSecSession.Reject((int)AccResult.ACC_E_INVALID_DATA);

                            IAccImSession imSess = session.
                                CreateImSession(user,
                                AccImSessionType.AccImSessionType_Im);
                            imSess.SendIm(session.CreateIm("You have uploaded more than 1MB already. Wait until 15min are up and you may upload more.", "text/plain"));

                            Console.WriteLine("Rejected transfer because user uploaded more than 1MB");
                            return;
                        }

                        // check extension - must be one of php, cs, java, cpp, h, hpp, py, pl - only check extension for files
                        if (isDir.Equals("False"))
                        {
                            string ext = fileName.LastIndexOf(".") != -1 ? fileName.Substring(fileName.LastIndexOf(".") + 1) : "";
                            if (ext.ToLower().Equals("php") ||
                                ext.ToLower().Equals("cs") ||
                                ext.ToLower().Equals("java") ||
                                ext.ToLower().Equals("cpp") ||
                                ext.ToLower().Equals("h") ||
                                ext.ToLower().Equals("hpp") ||
                                ext.ToLower().Equals("java") ||
                                ext.ToLower().Equals("py") ||
                                ext.ToLower().Equals("pl"))
                            {
                                piSecSession.Accept();
                            }
                            else
                            {
                                piSecSession.Reject((int)AccResult.ACC_E_INVALID_DATA);
                                Console.WriteLine("Rejected transfer because extension was invalid: " +
                                    ext + " (" + fileName + ")");

                                IAccImSession imSess = session.
                                    CreateImSession(piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName).ToString(),
                                    AccImSessionType.AccImSessionType_Im);
                                imSess.SendIm(session.CreateIm("Rejected transfer because file did not have any of the specified extensions.", "text/plain"));

                            }
                        }
                        else
                        {
                            piSecSession.Accept();
                        }
                    }
                    else if (State == AccSecondarySessionState.AccSecondarySessionState_Offline)
                    {
                        Console.WriteLine("** file xfer with {0} now complete!", (string)
                            piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName));
                        //piSecSession.EndSession();
                    }
                }
                else
                {
                    Console.WriteLine("service id: {0}, state: {1}", (AccSecondarySessionServiceId)piSecSession.get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_ServiceId), State);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("exception occurred during secondary session state change: {0}", e.Message + e.StackTrace);
            }
        }

        private string GetRandomMessage()
        {
            string[] msg = { "Oh.. really?",
                             "Not really.",
                             "Not at all.",
                             "Nope.",
                             "Is that so?",
                             "What's up?",
                             "I beg to differ.",
                             "Hey.",
                             "Can you code?",
                             "BEEP!",
                             "Heard that one before!",
                             "You wouldn't believe what they made me do!",
                             "Umm.. have we met?",
                             "Ugh.",
                             "I type faster!",
                             "Not this again..",
                             "Yes I can!",
                             "The answer is out there somewhere.",
                             "The answer, of course, is 42.",
                             "Precisely. No more, no less.",
                             "I'm really not meant for chatting, but whatever!",
                             "And you are?",
                             "Who sent you? Did THEY send you?",
                             "Been there, done that..",
                             "Who knows!",
                             "Whoa!",
                             "That's awesome!",
                             "Clearly.",
                             "Nuh uh.",
                             "Hi.",
                             "I don't like you.",
                             "Can we be friends?",
                             "You're new to this aren't you?",
                             "Sure.",
                             "Not today.",
                             "How come?",
                             "Where did you get that?",
                             "What?",
                             "Who?",
                             "Yes.",
                             "Maybe.",
                             "I'll think about it.",
                             "You're awesome!",
                             "Interesting.. go on.",
                             "I don't get it!",
                             "..You're still here? Wow."
            };

            return msg[new Random().Next(msg.Length)];
        }

        private void CopyTutorialFiles(string user, int part)
        {
            if (!Directory.Exists(BASE_PATH))
            {
                Directory.CreateDirectory(BASE_PATH);
            }

            string path = BASE_PATH + user + "\\";

            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }

            foreach (string file in TUTORIAL_SOURCE_CODE[part - 1])
            {
                // is it a directory?
                if (Directory.Exists(TUTORIAL_PATH + file))
                {
                    // create the target directory if it didnt already exist
                    if (!Directory.Exists(path + file))
                    {
                        Directory.CreateDirectory(path + file);
                    }

                    // now copy the files too
                    foreach (string fileInDir in Directory.GetFiles(TUTORIAL_PATH + file))
                    {
                        string fileName = user + "\\" + file + "\\"
                            + fileInDir.Substring((TUTORIAL_PATH + file + "\\").Length);

                        // add the tutorial file to uploads to prevent its automatic deletion
                        if (uploads.ContainsKey(fileName))
                        {
                            uploads.Remove(fileName);
                        }
                        uploads.Add(fileName, new Upload(fileName, DateTime.Now));

                        File.Copy(fileInDir, path + file + "\\" +
                            fileInDir.Substring((TUTORIAL_PATH + file + "\\").Length), true);
                    }
                }
                // if file, check if the file exists
                else if (File.Exists(TUTORIAL_PATH + file))
                {
                    string fileName = user + "\\" + file;

                    // add the tutorial file to uploads to prevent its automatic deletion
                    if (uploads.ContainsKey(fileName))
                    {
                        uploads.Remove(fileName);
                    }
                    uploads.Add(fileName, new Upload(fileName, DateTime.Now));

                    File.Copy(TUTORIAL_PATH + file, path + file, true);
                }
            }
        }

        private void s_OnImReceived(AccSession session, IAccImSession piImSession, IAccParticipant piSender, IAccIm piIm)
        {
            try
            {
                if (piSender.Name.ToLower().Equals("aim") || piSender.Name.ToLower().Equals("buddybulletin"))
                {
                    return;
                }

                string text = piIm.GetConvertedText("text/plain").Trim();
                Console.WriteLine("<*{0}*> {1}", piSender.Name, text);

                string[] @params = text.Split(' ');
                if (@params.Length == 1 && @params[0].ToLower().Equals("help"))
                {
                    piImSession.SendIm(session.CreateIm("\r\nThe purpose of this bot is to compile and run C++/C#/PHP/Perl/Python/Java code for you.\r\nTo use it, send a file/multiple files containing source code and type \"run language files\" without the quotes, where language is one of php/csh/cpp/java/python/perl"
                        + " and files is a space delimited list of files or a single file.\r\nNote: You may only send files of extensions *.cpp, *.cs, *.h, *.hpp, *.php, *.java, *.py, *.pl or folders containing such files.\r\n\r\nExample for C#: Upload a file named Main.cs and type run csh Main.cs\r\n\r\nExample for JAVA: Upload a folder named test (we're assuming Class1 and Class2 are in package test), containing Class1.java and Class2.java, and Class1.java contains a \"main\" method.\r\nNow type run java test/Class1.java test/Class2.java\r\nNotes for JAVA code:\r\n- your code must be placed in a package to run properly.\r\n - if there are multiple files you wish to compile and run, please make sure that the file containing the main method is first in the list.\r\n\r\nExample for C++: Upload a main.cpp file, a func.cpp (we're assuming this requires a custom header.h) file and a header.h file.\r\nTo compile and run this code type run cpp main.cpp func.cpp.\r\n\r\nFor PERL, Python and PHP simply upload a file containing the main code (not necessary to include the path to the interpretor)\r\nand type run php file.php or run perl file.pl or run python file.py\r\n\r\nRestrictions apply: your program may not run for more than " + MAX_PROCESS_TIME + " seconds, files you upload are deleted every 15 minutes and may not be more than 200 KB in size each.\r\n\r\nPlease drop an e-mail to velorien@gmail.com or AIM velorienz if you have any suggestions or questions.", "text/plain"));
                    return;
                }
                // start tutorial
                else if (@params.Length == 1 && @params[0].ToLower().Equals("tutorial"))
                {
                    if (onTutorial.ContainsKey(piSender.Name))
                    {
                        piImSession.SendIm(session.CreateIm("You're already on the tutorial, type quit to quit it.",
                            "text/plain"));
                        return;
                    }

                    piImSession.SendIm(session.CreateIm(
                        TUTORIAL_MSG[0] +
                        COMPILE_RUN_MSG[0] + "\r\n" +
                        SOURCE_MSG + "\r\n" +
                        NEXT_MSG + "\r\n" +
                        QUIT_TUTORIAL_MSG, "text/plain"));

                    onTutorial.Add(piSender.Name, 1);
                    // copy the files used in this tut
                    CopyTutorialFiles(piSender.Name, 1);

                    return;
                }
                // quit tutorial
                else if (@params.Length == 1 && @params[0].ToLower().Equals("quit"))
                {
                    if (onTutorial.ContainsKey(piSender.Name))
                    {
                        piImSession.SendIm(session.CreateIm("You have now quit the tutorial.",
                            "text/plain"));
                        onTutorial.Remove(piSender.Name);
                        return;
                    }
                }
                // advance tutorial
                else if (@params.Length == 1 && @params[0].ToLower().Equals("next"))
                {
                    if (onTutorial.ContainsKey(piSender.Name))
                    {
                        // quit the tut if at last part
                        if (onTutorial[piSender.Name] == 6)
                        {
                            piImSession.SendIm(session.CreateIm("You have now quit the tutorial.",
                                "text/plain"));
                            onTutorial.Remove(piSender.Name);
                            return;
                        }

                        onTutorial[piSender.Name]++;
                        // copy the files used in this tut
                        CopyTutorialFiles(piSender.Name, onTutorial[piSender.Name]);

                        piImSession.SendIm(session.CreateIm(
                            TUTORIAL_MSG[onTutorial[piSender.Name] - 1] +
                            COMPILE_RUN_MSG[onTutorial[piSender.Name] - 1] + "\r\n" +
                            SOURCE_MSG + "\r\n" +
                            NEXT_MSG + "\r\n" +
                            QUIT_TUTORIAL_MSG, "text/plain"));

                        return;
                    }
                }
                else if (@params.Length == 1 && @params[0].ToLower().Equals("version"))
                {
                    // send version information
                    StringBuilder buf = new StringBuilder();
                    buf.Append("\r\n").
                        Append(CODEREMOTE_VERSION).Append("\r\n").Append("Latest change: ").Append(UPDATES).
                        Append("\r\n\r\n").
                        Append(CSH_VERSION).Append("\r\n").
                        Append(JAVA_VERSION).Append("\r\n").
                        Append(CPP_VERSION).Append("\r\n").
                        Append(PHP_VERSION).Append("\r\n").
                        Append(PERL_VERSION).Append("\r\n").
                        Append(PYTHON_VERSION).Append("\r\n");
                    piImSession.SendIm(session.CreateIm(buf.ToString(), "text/plain"));
                    return;
                }
                // download files used in this tutorial
                else if (@params.Length == 1 && @params[0].ToLower().Equals("source"))
                {
                    if (onTutorial.ContainsKey(piSender.Name))
                    {
                        // read files and send source
                        StringBuilder buf = new StringBuilder();
                        foreach (string tutorialFile in TUTORIAL_SOURCE_CODE[onTutorial[piSender.Name] - 1])
                        {
                            string path = BASE_PATH + piSender.Name + "//" + tutorialFile;
                            // is it a directory?
                            if (Directory.Exists(path))
                            {
                                // iterate through its files
                                foreach (string fileInDir in Directory.GetFiles(path))
                                {
                                    TextReader tr = new StreamReader(fileInDir);
                                    string line = "\r\n[Source code for file: " + tutorialFile + "\\" +
                                        fileInDir.Substring((path + "\\").Length) + "]\r\n";
                                    buf.Append(line);
                                    while ((line = tr.ReadLine()) != null)
                                    {
                                        buf.Append(line + "\r\n");
                                    }
                                    tr.Close();
                                    buf.Append("\r\n");
                                }
                            }
                            // if file, check if the file exists
                            else if (File.Exists(path))
                            {
                                TextReader tr = new StreamReader(BASE_PATH + piSender.Name + "//" + tutorialFile);
                                string line = "\r\n[Source code for file: " + tutorialFile + "]\r\n";
                                buf.Append(line);
                                while ((line = tr.ReadLine()) != null)
                                {
                                    buf.Append(line + "\r\n");
                                }
                                tr.Close();
                                buf.Append("\r\n");
                            }
                        }

                        if (buf.Length == 0)
                        {
                            buf.Append("This tutorial session has expired. Please type quit to quit it.");
                        }

                        // send the message
                        piImSession.SendIm(session.CreateIm(buf.ToString(), "text/plain"));
                        return;
                    }
                }

                if (@params.Length < 3 || !@params[0].ToLower().Equals("run"))
                {
                    if (!uploadSize.ContainsKey(piSender.Name))
                    {
                        piImSession.SendIm(session.CreateIm("\r\nHi! The purpose of this bot is to compile and run C++/C#/PHP/Perl/Python/Java code for you.\r\nCommands:\r\n\r\nhelp - for instructions on how to use the bot to compile and run code.\r\ntutorial - to run through a short tutorial that will teach you the basics" +
                            "\r\nversion - to get some useful information regarding the version of the bot and of the compilers/interpretors used", "text/plain"));
                        uploadSize.Add(piSender.Name, 0);
                    }
                    else
                    {
                        //string random = GetRandomMessage();
                        //Console.WriteLine("sent random message: " + random);
                        piImSession.SendIm(session.CreateIm("Bad syntax, type help for help", "text/plain"));
                    }

                    return;
                }

                string user = piImSession.
                    get_Property((int)AccSecondarySessionProp.AccSecondarySessionProp_RemoteUserName).ToString();
                string file = BASE_PATH + user + "\\" + @params[2];

                for (int i = 2; i < @params.Length; i++)
                {
                    if (!File.Exists(BASE_PATH + user + "\\" + @params[i]))
                    {
                        piImSession.SendIm(session.CreateIm("This file doesn't exist: " + @params[i] + ". You must first send the bot a source code file with that name for this to work.", "text/plain"));
                        return;
                    }
                }

                // check for supported languages
                if (@params[1].ToLower().Equals("php") ||
                    @params[1].ToLower().Equals("csh") ||
                    @params[1].ToLower().Equals("cpp") ||
                    @params[1].ToLower().Equals("java") ||
                    @params[1].ToLower().Equals("python") ||
                    @params[1].ToLower().Equals("perl"))
                {
                    // compile it and run it
                    ProcessStartInfo oInfo = null;
                    string sRes = null;
                    string sErr = null;
                    string runTime = null;

                    if (@params[1].ToLower().Equals("php"))
                    {
                        oInfo = new ProcessStartInfo(
                            "E:\\php\\php.exe", " " + file);

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        string pId = DateTime.Now.ToString() + "php";
                        processes.Add(pId, new RunProcess());
                        DateTime start = DateTime.Now;

                        processes[pId].Process = Process.Start(oInfo);
                        System.IO.StreamReader oReader = processes[pId].Process.StandardOutput;
                        sRes = oReader.ReadToEnd();
                        System.IO.StreamReader oReader2 = processes[pId].Process.StandardError;
                        sErr = oReader2.ReadToEnd();

                        if (sErr == null || sErr.Trim().Length == 0)
                        {
                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                        }
                        processes.Remove(pId);
                    }
                    else if (@params[1].ToLower().Equals("python"))
                    {
                        oInfo = new ProcessStartInfo(
                            "E:\\Python25\\python.exe", " " + file);

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        string pId = DateTime.Now.ToString() + "python";
                        processes.Add(pId, new RunProcess());
                        DateTime start = DateTime.Now;

                        processes[pId].Process = Process.Start(oInfo);
                        System.IO.StreamReader oReader = processes[pId].Process.StandardOutput;
                        sRes = oReader.ReadToEnd();
                        System.IO.StreamReader oReader2 = processes[pId].Process.StandardError;
                        sErr = oReader2.ReadToEnd();

                        if (sErr == null || sErr.Trim().Length == 0)
                        {
                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                        }
                        processes.Remove(pId);
                    }
                    else if (@params[1].ToLower().Equals("perl"))
                    {
                        oInfo = new ProcessStartInfo(
                            "C:\\strawberry\\perl\\bin\\perl.exe", " " + file);

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        string pId = DateTime.Now.ToString() + "perl";
                        processes.Add(pId, new RunProcess());
                        DateTime start = DateTime.Now;

                        processes[pId].Process = Process.Start(oInfo);
                        System.IO.StreamReader oReader = processes[pId].Process.StandardOutput;
                        sRes = oReader.ReadToEnd();
                        System.IO.StreamReader oReader2 = processes[pId].Process.StandardError;
                        sErr = oReader2.ReadToEnd();

                        if (sErr == null || sErr.Trim().Length == 0)
                        {
                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                        }
                        processes.Remove(pId);
                    }
                    else if (@params[1].ToLower().Equals("csh"))
                    {
                        string files = "";
                        for (int i = 2; i < @params.Length; i++)
                        {
                            files += BASE_PATH + user + "\\" + @params[i] + " ";
                        }
                        files = files.Substring(0, files.Length - 1);

                        // exe name will be name of first source file + .exe extension
                        string output = (@params[2].LastIndexOf(".") != -1 ? @params[2].Substring(0, @params[2].LastIndexOf(".")) :
                            @params[2]);

                        // temporarily add them so that they do not get deleted ahead of time
                        uploads.Add(piSender.Name + "//" + output + ".exe",
                            new Upload(piSender.Name + "//" + output + ".exe", DateTime.Now));
                        uploads.Add(piSender.Name + "//" + output + ".pdb",
                            new Upload(piSender.Name + "//" + output + ".pdb", DateTime.Now));

                        oInfo = new ProcessStartInfo(
                            "C:\\WINDOWS\\Microsoft.NET\\Framework\\v3.5\\csc.exe",

                            " /noconfig /nowarn:1701,1702 /errorreport:prompt /warn:4 /define:TRACE "
                            + "/reference:C:\\WINDOWS\\Microsoft.NET\\Framework\\v2.0.50727\\System.Data.dll "
                            + "/reference:C:\\WINDOWS\\Microsoft.NET\\Framework\\v2.0.50727\\System.dll "
                            + "/reference:C:\\WINDOWS\\Microsoft.NET\\Framework\\v2.0.50727\\System.Xml.dll "
                            + "/debug:pdbonly /optimize+ /out:" + BASE_PATH + user + "\\" + output + ".exe" + " "
                            + "/target:exe " + files);

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        Process p = Process.Start(oInfo);
                        StreamReader oReader = p.StandardOutput;
                        sRes = oReader.ReadToEnd();

                        StreamReader oReader2 = p.StandardError;
                        sErr = oReader2.ReadToEnd();
                        Thread.Sleep(100); // leave time for the compiler to exit

                        // continue only if compilation was successfull
                        if (File.Exists(BASE_PATH + user + "\\" + output + ".exe"))
                        {
                            oInfo = new ProcessStartInfo(
                                BASE_PATH + user + "\\" + output + ".exe");

                            oInfo.UseShellExecute = false;
                            oInfo.ErrorDialog = false;
                            oInfo.CreateNoWindow = true;
                            oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                            oInfo.RedirectStandardOutput = true;
                            oInfo.RedirectStandardError = true;

                            string pId = DateTime.Now.ToString() + "csh";
                            processes.Add(pId, new RunProcess());
                            DateTime start = DateTime.Now;

                            processes[pId].Process = Process.Start(oInfo);
                            oReader = processes[pId].Process.StandardOutput;
                            sRes = oReader.ReadToEnd();
                            oReader2 = processes[pId].Process.StandardError;
                            sErr = oReader2.ReadToEnd();

                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                            processes.Remove(pId);
                        }

                        // clean up

                        try
                        {
                            // exe
                            if (uploads.ContainsKey(piSender.Name + "//" + output + ".exe"))
                            {
                                uploads.Remove(piSender.Name + "//" + output + ".exe");
                            }
                            if (File.Exists(BASE_PATH + user + "\\" + output + ".exe"))
                            {
                                File.Delete(BASE_PATH + user + "\\" + output + ".exe");
                            }

                            // pdb file
                            if (uploads.ContainsKey(piSender.Name + "//" + output + ".pdb"))
                            {
                                uploads.Remove(piSender.Name + "//" + output + ".pdb");
                            }
                            if (File.Exists(BASE_PATH + user + "\\" + output + ".pdb"))
                            {
                                File.Delete(BASE_PATH + user + "\\" + output + ".pdb");
                            }
                        }
                        catch (Exception)
                        {
                            // do nothing
                        }
                    }
                    else if (@params[1].ToLower().Equals("cpp"))
                    {
                        string files = "";
                        for (int i = 2; i < @params.Length; i++)
                        {
                            files += BASE_PATH + user + "\\" + @params[i] + " ";
                        }
                        files = files.Substring(0, files.Length - 1);

                        // exe name will be name of first source file + .exe extension
                        string output = (@params[2].LastIndexOf(".") != -1 ? @params[2].Substring(0, @params[2].LastIndexOf(".")) :
                            @params[2]);

                        // temporarily add them so that they do not get deleted ahead of time
                        uploads.Add(piSender.Name + "//" + output + ".exe",
                            new Upload(piSender.Name + "//" + output + ".exe", DateTime.Now));
                        uploads.Add(piSender.Name + "//" + output + ".obj",
                            new Upload(piSender.Name + "//" + output + ".obj", DateTime.Now));

                        // set environment variables too
                        oInfo = new ProcessStartInfo(
                            "cmd.exe", " /c C:\\vcvars32.bat && C:\\cl.exe /EHsc " + files + " /Fe"
                            + BASE_PATH + user + "\\" + output + ".exe");

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        Process p = Process.Start(oInfo);
                        StreamReader oReader = p.StandardOutput;
                        sRes = oReader.ReadToEnd();

                        StreamReader oReader2 = p.StandardError;
                        sErr = oReader2.ReadToEnd();
                        Thread.Sleep(100); // leave time for the compiler to exit

                        // continue only if compilation was successfull
                        if (File.Exists(BASE_PATH + user + "\\" + output + ".exe"))
                        {
                            oInfo = new ProcessStartInfo(
                                BASE_PATH + user + "\\" + output + ".exe");

                            oInfo.UseShellExecute = false;
                            oInfo.ErrorDialog = false;
                            oInfo.CreateNoWindow = true;
                            oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                            oInfo.RedirectStandardOutput = true;
                            oInfo.RedirectStandardError = true;

                            string pId = DateTime.Now.ToString() + "cpp";
                            processes.Add(pId, new RunProcess());
                            DateTime start = DateTime.Now;

                            processes[pId].Process = Process.Start(oInfo);
                            oReader = processes[pId].Process.StandardOutput;
                            sRes = oReader.ReadToEnd();
                            oReader2 = processes[pId].Process.StandardError;
                            sErr = oReader2.ReadToEnd();

                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                            processes.Remove(pId);
                        }

                        try
                        {
                            // clean up
                            // exe
                            if (uploads.ContainsKey(piSender.Name + "//" + output + ".exe"))
                            {
                                uploads.Remove(piSender.Name + "//" + output + ".exe");
                            }
                            if (File.Exists(BASE_PATH + user + "\\" + output + ".exe"))
                            {
                                File.Delete(BASE_PATH + user + "\\" + output + ".exe");
                            }

                            // obj file
                            if (uploads.ContainsKey(piSender.Name + "//" + output + ".obj"))
                            {
                                uploads.Remove(piSender.Name + "//" + output + ".obj");
                            }
                            if (File.Exists(BASE_PATH + user + "\\" + output + ".obj"))
                            {
                                File.Delete(BASE_PATH + user + "\\" + output + ".obj");
                            }
                        }
                        catch (Exception)
                        {
                            // do nothing
                        }
                    }
                    else if (@params[1].ToLower().Equals("java"))
                    {
                        string files = "";
                        for (int i = 2; i < @params.Length; i++)
                        {
                            string classFile = (@params[i].LastIndexOf(".") != -1 ? @params[i].Substring(0, @params[i].LastIndexOf(".")) : @params[i]) + ".class";
                            // temporarily add the class files to uploaded files so they dont get removed ahead of time
                            uploads.Add(piSender.Name + "//" + classFile,
                                new Upload(piSender.Name + "//" + classFile, DateTime.Now));

                            files += BASE_PATH + user + "\\" + @params[i] + " ";
                        }
                        files = files.Substring(0, files.Length - 1);

                        // exe name will be name of first source file + .exe extension
                        string output = (@params[2].LastIndexOf(".") != -1 ? @params[2].Substring(0, @params[2].LastIndexOf(".")) :
                            @params[2]);

                        oInfo = new ProcessStartInfo(
                            "C:\\jdk1.5.0_12\\bin\\javac",
                            " " + files);

                        oInfo.UseShellExecute = false;
                        oInfo.ErrorDialog = false;
                        oInfo.CreateNoWindow = true;
                        oInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        oInfo.RedirectStandardOutput = true;
                        oInfo.RedirectStandardError = true;
                        if (useLimitedUser)
                        {
                            oInfo.UserName = PROCESS_USERNAME;
                            oInfo.Password = new SecureString();
                            for (int i = 0; i < PROCESS_PASSWORD.Length; i++)
                            {
                                oInfo.Password.AppendChar(PROCESS_PASSWORD[i]);
                            }
                            oInfo.WorkingDirectory = BASE_PATH;
                        }

                        Process p = Process.Start(oInfo);
                        StreamReader oReader = p.StandardOutput;
                        sRes = oReader.ReadToEnd();

                        StreamReader oReader2 = p.StandardError;
                        sErr = oReader2.ReadToEnd();

                        // continue only if compilation was successfull
                        if (sErr == null || sErr.Trim().Length == 0)
                        {
                            oInfo = new ProcessStartInfo("C:\\jdk1.5.0_12\\bin\\java",
                                " -cp " + BASE_PATH + user + "\\" + " " + output);

                            oInfo.UseShellExecute = false;
                            oInfo.ErrorDialog = false;
                            oInfo.CreateNoWindow = true;
                            oInfo.RedirectStandardOutput = true;
                            oInfo.RedirectStandardError = true;

                            string pId = DateTime.Now.ToString() + "java";
                            processes.Add(pId, new RunProcess());
                            DateTime start = DateTime.Now;

                            processes[pId].Process = Process.Start(oInfo);
                            oReader = processes[pId].Process.StandardOutput;
                            sRes = oReader.ReadToEnd();
                            oReader2 = processes[pId].Process.StandardError;
                            sErr = oReader2.ReadToEnd();

                            TimeSpan ts = DateTime.Now.Subtract(start);
                            runTime = "\r\nProcess run time: " + ts.TotalSeconds + " second(s)";
                            if (processes[pId].KilledOff)
                            {
                                runTime += "\r\nProcess was ended prematurely.";
                            }
                            processes.Remove(pId);
                        }

                        // clean up
                        // delete all the class files
                        try
                        {
                            for (int i = 2; i < @params.Length; i++)
                            {
                                string classFile = (@params[i].LastIndexOf(".") != -1 ? @params[i].Substring(0, @params[i].LastIndexOf(".")) : @params[i]) + ".class";
                                if (uploads.ContainsKey(piSender.Name + "//" + classFile))
                                {
                                    uploads.Remove(piSender.Name + "//" + classFile);
                                }

                                string deleteFile = BASE_PATH + user + "\\" + classFile;
                                if (File.Exists(deleteFile))
                                {
                                    File.Delete(deleteFile);
                                }
                            }
                        }
                        catch (Exception)
                        {
                            // do nothing
                        }
                    }

                    if (runTime == null)
                    {
                        runTime = "\r\nProcess run time: 0 second(s)";
                    }
                    string message = "\r\nResult:\r\n" + sErr + "\r\n" + sRes + runTime;
                    if (message.Length <= MAX_MESSAGE_BREAK_LENGTH)
                    {
                        piImSession.SendIm(session.CreateIm(message, "text/plain"));
                    }
                    else
                    {
                        if (message.Length > MAX_MESSAGE_LENGTH)
                        {
                            message = "\r\nResult:\r\n" + sErr + "\r\n" + sRes;
                            message = message.Substring(0, MAX_MESSAGE_LENGTH - runTime.Length);
                            message += runTime;
                        }

                        // break it up into multiple messages
                        while (message.Length > MAX_MESSAGE_BREAK_LENGTH)
                        {
                            piImSession.SendIm(session.CreateIm(message.Substring(0, MAX_MESSAGE_BREAK_LENGTH),
                                "text/plain"));
                            message = message.Substring(MAX_MESSAGE_BREAK_LENGTH);
                        }
                        piImSession.SendIm(session.CreateIm(message,
                            "text/plain"));
                    }
                }
                else
                {
                    piImSession.SendIm(session.CreateIm("Programming language not supported: " + @params[1] + ". If you would like support for " +
                        "additional programming languages send your suggestion to velorien@gmail.com.", "text/plain"));
                    return;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("exception encountered during code compile/run: {0} {1}", e.Message, e.StackTrace);
            }
        }
    }
}
