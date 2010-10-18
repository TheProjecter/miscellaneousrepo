using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;

namespace Recursor
{
    /// <summary>
    /// Recursor app.
    /// </summary>
    [Guid("BB08EC17-FF78-4C64-AF58-90930B0AE11B")]
    public partial class Recursor : Form
    {
        /// <summary>
        /// 
        /// </summary>
        public static readonly char FILE_WILDCARD_SEPARATOR = '|';

        /// <summary>
        /// 
        /// </summary>
        public static readonly string REPLACE_STRING_FORMAT = "Replace {0} with {1} for files {2}";

        /// <summary>
        /// 
        /// </summary>
        public static readonly int MAX_LINE_LENGTH = 120;


        private static IDictionary<string, string> regexes = new Dictionary<string, string>();

        private readonly string CODE_CHECK_PATTERN = Regexify("*.js|*.html|*.cs|*.java|*.sql|*.pkg|*.pkb|*.py|*.rb|*.pl|*.cpp|*.h|*.c|*.hpp");

        /// <summary>
        /// 
        /// </summary>
        private RecursorSettings settings;

        /// <summary>
        /// 
        /// </summary>
        private bool IsStopped;

        /// <summary>
        /// Initializes a new instance of the <see cref="Recursor"/> class.
        /// </summary>
        public Recursor()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Gets the name of the app key.
        /// </summary>
        /// <value>The name of the app key.</value>
        public string AppKeyName
        {
            get
            {
                return GetType().GUID.ToString();
            }
        }

        /// <summary>
        /// Wildcards the match.
        /// </summary>
        /// <param name="strWithWildCards">The STR with wild cards.</param>
        /// <param name="myString">My string.</param>
        /// <returns></returns>
        public static bool WildcardMatch(string strWithWildCards, string myString)
        {
            if (strWithWildCards.Length == 0)
            {
                return myString.Length == 0;
            }

            if (myString.Length == 0)
            {
                return false;
            }

            return Regex.IsMatch(myString, Regexify(strWithWildCards));
        }

        private static string Regexify(string strWithWildCards)
        {
            if (regexes.ContainsKey(strWithWildCards))
            {
                return regexes[strWithWildCards];
            }

            // support a simplified variant of regexes
            // wildcards like A*le (matches Apple), Oran?e? (matches Oranges), or even name1|name2|name3 (matches any of those)
            return (regexes[strWithWildCards] = "^(" + Regex.Escape(strWithWildCards).
                                                   Replace(@"\?", ".{1}").Replace(@"\*", ".*").
                                                   Replace(@"\|", "|") + ")$");
        }

        /// <summary>
        /// Handles the Click event of the browseButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void browseButton_Click(object sender, EventArgs e)
        {
            try
            {
                if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
                {
                    folder.Text = folderBrowserDialog1.SelectedPath;
                }
            }
            catch (Exception)
            {
                // bad directory, clear it
                folderBrowserDialog1.SelectedPath = "";
                folder.Text = "";
                if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
                {
                    folder.Text = folderBrowserDialog1.SelectedPath;
                }
            }
        }

        /// <summary>
        /// Handles the Click event of the addRuleButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void addRuleButton_Click(object sender, EventArgs e)
        {
            // for trimming trailing whitespace use: [ \t]+$

            // add a new replacement rule
            IList<ReplaceRule> rules = new List<ReplaceRule>(settings.ReplaceRules) { new ReplaceRule(toReplace.Text,
                replaceWith.Text.Replace("\\r", "\r").Replace("\\t", "\t").Replace("\\n", "\n"), replaceFiles.Text) };
            settings.ReplaceRules = rules;
            // add to rules listbox
            listRules.Items.Add(string.Format(REPLACE_STRING_FORMAT,
                                              toReplace.Text, replaceWith.Text,
                                              replaceFiles.Text));
        }

        /// <summary>
        /// Handles the Click event of the saveRuleButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void saveRuleButton_Click(object sender, EventArgs e)
        {
            // save an existing replacement rule
            IList<ReplaceRule> rules = new List<ReplaceRule>(settings.ReplaceRules);
            if (listRules.SelectedItem != null)
            {
                rules[listRules.SelectedIndex] = new ReplaceRule(toReplace.Text,
                    replaceWith.Text.Replace("\\r", "\r").Replace("\\t", "\t").Replace("\\n", "\n"), replaceFiles.Text);
                settings.ReplaceRules = rules;
                listRules.Items[listRules.SelectedIndex] = string.Format(REPLACE_STRING_FORMAT,
                                                                         toReplace.Text, replaceWith.Text,
                                                                         replaceFiles.Text);
            }
        }

        /// <summary>
        /// Handles the Click event of the deleteRuleButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void deleteRuleButton_Click(object sender, EventArgs e)
        {
            // delete a replacement rule
            IList<ReplaceRule> rules = new List<ReplaceRule>(settings.ReplaceRules);
            if (listRules.SelectedItem != null)
            {
                rules.RemoveAt(listRules.SelectedIndex);
                settings.ReplaceRules = rules;
                // delete from rules listbox
                listRules.Items.RemoveAt(listRules.SelectedIndex);
            }
        }

        /// <summary>
        /// Handles the Click event of the saveFilterButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void saveFilterButton_Click(object sender, EventArgs e)
        {
            // save the file removal filter
            settings.RemoveFiles = fileFilter.Text;
        }

        /// <summary>
        /// Delete extra files/directories recursively.
        /// </summary>
        /// <param name="path">Current path</param>
        private void GetFiles(string path, IList<string> fileList)
        {
            string[] dirs = Directory.GetDirectories(path);
            string[] files = Directory.GetFiles(path);

            foreach (string dir in dirs)
            {
                fileList.Add(dir);
                GetFiles(dir, fileList);
            }

            foreach (string file in files)
            {
                fileList.Add(file);
            }
        }

        /// <summary>
        /// Handles the Click event of the goButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void goButton_Click(object sender, EventArgs e)
        {
            IsStopped = false;
            goButton.Enabled = false;
            stopButton.Enabled = true;
            new Thread(Process).Start();
        }

        private void Process()
        {
            bool parseFiles = parseCheck.Checked;
            // organize the file wildcards from settings.ReplaceRules and settings.RemoveFiles
            // first check against the remove wildcard, then against the replace rules
            string removeWildcards = Regexify(fileFilter.Text);
            DateTime start = DateTime.Now;

            IList<string> files = new List<string>();
            try
            {
                GetFiles(folder.Text, files);
            }
            catch
            {
                Invoke(new MethodInvoker(delegate
                {
                    MessageBox.Show("Path is incorrect.");
                    goButton.Enabled = true;
                    stopButton.Enabled = false;
                }));
                return;
            }

            int processedFiles = 0;
            try
            {
                foreach (string file in files)
                {
                    if (IsStopped)
                        goto finish;

                    Invoke(new MethodInvoker(delegate
                                                 {
                                                     progressBar1.Value = 100 * processedFiles / files.Count;
                                                 }));
                    string smallFile = file.Substring(file.LastIndexOf("\\") + 1);

                    bool deleted = false;
                    // remove files wildcards

                    if (IsStopped)
                        goto finish;

                    // if the file matches, delete it and skip to next one
                    if (Regex.IsMatch(smallFile, removeWildcards))
                    {
                        // log the action
                        Invoke(new MethodInvoker(delegate
                                                     {
                                                         actionsLog.Text += "Deleting " + smallFile + "\r\n";
                                                     }));
                        processedFiles++;
                        if (Directory.Exists(file))
                        {
                            Directory.Delete(file, true);
                            deleted = true;
                        }
                        if (File.Exists(file))
                        {
                            File.Delete(file);
                            deleted = true;
                        }
                    }

                    if (deleted || !parseFiles)
                    {
                        /*
                        if (processedFiles % 50 == 0)
                        {
                            Thread.Sleep(50); // rest a bit
                        }*/
                        continue;
                    }

                    string content = null;
                    string[] lines = null;
                    // line checking is currently hardcoded for cs, java, pkg, sql and pkb files
                    if (Regex.IsMatch(smallFile, CODE_CHECK_PATTERN))
                    {
                        content = File.ReadAllText(file);
                        lines = content.Split(new[] { "\r\n" }, StringSplitOptions.None);

                        // check length of each line first
                        for (int i = 0; i < lines.Length; i++)
                        {
                            if (lines[i].Length > MAX_LINE_LENGTH)
                            {
                                Invoke(new MethodInvoker(delegate
                                                             {
                                                                 longLinesLog.Text += smallFile + " line " + (i + 1) +
                                                                                      " over " +
                                                                                      MAX_LINE_LENGTH +
                                                                                      " chars" + "\r\n";
                                                             }));
                            }
                        }
                    }


                    // for each replace rule
                    foreach (ReplaceRule rule in settings.ReplaceRules)
                    {
                        // get replace files wildcards for the rule and match each
                        foreach (string wildcard in rule.ReplaceFilesList)
                        {
                            // if the file matches, replace stuff
                            if (WildcardMatch(wildcard, smallFile) && File.Exists(file))
                            {
                                if (content == null) content = File.ReadAllText(file);

                                string oldc = content;
                                content = Regex.Replace(content, rule.ToReplace, rule.ReplaceWith);
                                if (oldc != content)
                                {
                                    // log the action
                                    Invoke(new MethodInvoker(delegate
                                    {
                                        actionsLog.Text += "Replaced " + rule.ToReplace +
                                                           " with " +
                                                           rule.ReplaceWith +
                                                           " in " + smallFile + "\r\n";
                                    }));
                                }
                                File.WriteAllText(file, content);
                                break;
                            }
                        }
                    }
                    processedFiles++;
                    /*if (processedFiles % 50 == 0)
                    {
                        Thread.Sleep(50); // rest a bit
                    }*/
                }
            }
            catch (Exception e)
            {
                Invoke(new MethodInvoker(delegate
                {
                    progressBar1.Value = 100;
                    MessageBox.Show("An unexpected error occurred: " + e);
                    goButton.Enabled = true;
                    stopButton.Enabled = false;
                }));
                return;
            }

        finish:
            Invoke(new MethodInvoker(delegate
                                         {
                                             progressBar1.Value = 100;
                                             MessageBox.Show("Done, processed " + processedFiles + " files in " + (DateTime.Now - start).TotalMilliseconds + "ms.");
                                             goButton.Enabled = true;
                                             stopButton.Enabled = false;
                                         }));
        }

        /// <summary>
        /// Handles the Load event of the Recursor control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void Recursor_Load(object sender, EventArgs e)
        {
            // settings:
            // ReplaceRules (key)
            //     1 (key)
            //         ToReplace (value)
            //         ReplaceWith (value)
            //         ReplaceFiles (value)
            // RemoveFiles (value)

            settings = new RecursorSettings(AppKeyName);

            // load stuff into listbox also
            foreach (ReplaceRule rule in settings.ReplaceRules)
            {
                listRules.Items.Add(string.Format(REPLACE_STRING_FORMAT,
                                                  rule.ToReplace, rule.ReplaceWith.Replace("\r", "\\r").Replace("\t", "\\t").Replace("\n", "\\n"), rule.ReplaceFiles));
            }

            // load remove files setting
            fileFilter.Text = settings.RemoveFiles;
        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the listRules control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void listRules_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listRules.SelectedIndex != -1)
            {
                toReplace.Text = settings.ReplaceRules[listRules.SelectedIndex].ToReplace;
                replaceWith.Text = settings.ReplaceRules[listRules.SelectedIndex].ReplaceWith.Replace("\r", "\\r").Replace("\t", "\\t").Replace("\n", "\\n");
                replaceFiles.Text = settings.ReplaceRules[listRules.SelectedIndex].ReplaceFiles;
            }
        }

        /// <summary>
        /// Handles the Click event of the stopButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void stopButton_Click(object sender, EventArgs e)
        {
            IsStopped = true;
        }
    }
}