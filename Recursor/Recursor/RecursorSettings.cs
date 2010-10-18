using System.Collections.Generic;
using Microsoft.Win32;

namespace Recursor
{
    /// <summary>
    /// 
    /// </summary>
    internal class ReplaceRule
    {
        /// <summary>
        /// 
        /// </summary>
        private string replaceFiles;

        /// <summary>
        /// 
        /// </summary>
        private IList<string> replaceFilesList;

        /// <summary>
        /// Initializes a new instance of the <see cref="ReplaceRule"/> class.
        /// </summary>
        /// <param name="toReplace">To replace.</param>
        /// <param name="replaceWith">The replace with.</param>
        /// <param name="replaceFiles">The replace files.</param>
        public ReplaceRule(string toReplace, string replaceWith, string replaceFiles)
        {
            ToReplace = toReplace;
            ReplaceWith = replaceWith;
            ReplaceFiles = replaceFiles;
            replaceFilesList = replaceFiles.Split(new[] {Recursor.FILE_WILDCARD_SEPARATOR});
        }

        /// <summary>
        /// Gets or sets to replace.
        /// </summary>
        /// <value>To replace.</value>
        public string ToReplace
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the replace with.
        /// </summary>
        /// <value>The replace with.</value>
        public string ReplaceWith
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the replace files.
        /// </summary>
        /// <value>The replace files.</value>
        public string ReplaceFiles
        {
            get
            {
                return replaceFiles;
            }
            set
            {
                replaceFiles = value;
                replaceFilesList = replaceFiles.Split(new[] {Recursor.FILE_WILDCARD_SEPARATOR});
            }
        }

        /// <summary>
        /// Gets the replace files list.
        /// </summary>
        /// <value>The replace files list.</value>
        public IList<string> ReplaceFilesList
        {
            get
            {
                return replaceFilesList;
            }
        }
    }

    /// <summary>
    /// 
    /// </summary>
    internal class RecursorSettings
    {
        /// <summary>
        /// 
        /// </summary>
        public static readonly string REMOVE_FILES = "RemoveFiles";

        /// <summary>
        /// 
        /// </summary>
        public static readonly string REPLACE_FILES = "ReplaceFiles";

        /// <summary>
        /// 
        /// </summary>
        public static readonly string REPLACE_RULES = "ReplaceRules";

        /// <summary>
        /// 
        /// </summary>
        public static readonly string REPLACE_WITH = "ReplaceWith";

        /// <summary>
        /// 
        /// </summary>
        public static readonly string TO_REPLACE = "ToReplace";

        /// <summary>
        /// 
        /// </summary>
        private readonly string absoluteKeyName;

        /// <summary>
        /// 
        /// </summary>
        private readonly string appKeyName;

        /// <summary>
        /// 
        /// </summary>
        private string removeFiles;

        /// <summary>
        /// 
        /// </summary>
        private IList<ReplaceRule> replaceRules = new List<ReplaceRule>();

        /// <summary>
        /// Initializes a new instance of the <see cref="RecursorSettings"/> class.
        /// </summary>
        /// <param name="keyName">Name of the key.</param>
        public RecursorSettings(string keyName)
        {
            // loading  stuff from registry / creating it if it's missing
            appKeyName = keyName;
            absoluteKeyName = @"Software\" + keyName;

            RegistryKey[] keys = GetKeys(REPLACE_RULES);
            foreach (RegistryKey key in keys)
            {
                // parse the replace rules
                replaceRules.Add(new ReplaceRule(
                                     key.GetValue(TO_REPLACE).ToString(),
                                     key.GetValue(REPLACE_WITH).ToString(),
                                     key.GetValue(REPLACE_FILES).ToString()
                                     ));
            }
            // get the remove files filter
            removeFiles = ReadValue(REMOVE_FILES);
        }

        /// <summary>
        /// Gets or sets the remove files.
        /// </summary>
        /// <value>The remove files.</value>
        public string RemoveFiles
        {
            get
            {
                return removeFiles;
            }
            set
            {
                // persisting to registry
                WriteValue(REMOVE_FILES, value);
                removeFiles = value;
            }
        }

        public IList<ReplaceRule> ReplaceRules
        {
            // TODO: add a custom IList to make things more efficient, and thus to know exactly the item that was removed/changed
            get
            {
                return replaceRules;
            }
            set
            {
                // persisting to registry

                // check if an entry was added
                if (value.Count > replaceRules.Count)
                {
                    RegistryKey key = CreateRuleKey((value.Count - 1).ToString());
                    key.SetValue(TO_REPLACE, value[value.Count - 1].ToReplace);
                    key.SetValue(REPLACE_WITH, value[value.Count - 1].ReplaceWith);
                    key.SetValue(REPLACE_FILES, value[value.Count - 1].ReplaceFiles);
                }

                // check if an entry was deleted
                if (value.Count < replaceRules.Count)
                {
                    int deletedIndex = replaceRules.Count - 1;
                    for (int i = 0; i < value.Count; i++)
                    {
                        if (value[i].ReplaceFiles != replaceRules[i].ReplaceFiles ||
                            value[i].ToReplace != replaceRules[i].ToReplace ||
                            value[i].ReplaceWith != replaceRules[i].ReplaceWith)
                        {
                            deletedIndex = i;
                            break;
                        }
                    }
                    // delete the rule from registry
                    DeleteRuleKey(deletedIndex);
                }

                // change the ones that were modified in the registry
                if (value.Count == replaceRules.Count)
                {
                    for (int i = 0; i < value.Count; i++)
                    {
                        if (value[i].ReplaceFiles != replaceRules[i].ReplaceFiles ||
                            value[i].ToReplace != replaceRules[i].ToReplace ||
                            value[i].ReplaceWith != replaceRules[i].ReplaceWith)
                        {
                            RegistryKey key = OpenRuleKey(i.ToString());
                            if (key != null)
                            {
                                key.SetValue(REPLACE_FILES, value[i].ReplaceFiles);
                                key.SetValue(TO_REPLACE, value[i].ToReplace);
                                key.SetValue(REPLACE_WITH, value[i].ReplaceWith);
                            }
                        }
                    }
                }

                replaceRules = value;
            }
        }

        /// <summary>
        /// Reads the value with the given name from the registry.
        /// </summary>
        /// <param name="name">the setting name</param>
        /// <returns>the setting value</returns>
        public string ReadValue(string name)
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(absoluteKeyName);
            if (key != null)
            {
                return (key.GetValue(name) != null ? key.GetValue(name).ToString() : null);
            }
            else
            {
                // create the parent key if it wasn't present
                CreateKey(null);
                return null;
            }
        }

        /// <summary>
        /// Writes the value to registry.
        /// </summary>
        /// <param name="name">setting name</param>
        /// <param name="value">setting value</param>
        public void WriteValue(string name, string value)
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(absoluteKeyName, true);
            if (key == null)
            {
                // create the parent key if it wasn't present
                key = CreateKey(null);
            }
            key.SetValue(name, value);
        }

        /// <summary>
        /// Creates a key with the given name.
        /// </summary>
        /// <param name="name">the registry key name</param>
        /// <returns></returns>
        public RegistryKey CreateKey(string name)
        {
            // create the root key if it wasn't present
            RegistryKey key = Registry.LocalMachine.OpenSubKey(absoluteKeyName, true);
            if (key == null)
            {
                key = Registry.LocalMachine.OpenSubKey("Software", true).CreateSubKey(appKeyName);
            }

            // parent key if name is null
            if (name == null)
            {
                return null;
            }

            return key.CreateSubKey(name);
        }

        /// <summary>
        /// Gets the rules key.
        /// </summary>
        /// <returns></returns>
        public RegistryKey GetRulesKey()
        {
            // create the root key if it wasn't present
            RegistryKey key = Registry.LocalMachine.OpenSubKey(absoluteKeyName, true);
            if (key == null)
            {
                // create the parent key if it wasn't present
                key = CreateKey(null);
            }

            RegistryKey rulesKey = key.OpenSubKey(REPLACE_RULES, true);
            if (rulesKey == null)
            {
                rulesKey = key.CreateSubKey(REPLACE_RULES);
            }

            return rulesKey;
        }

        /// <summary>
        /// Creates a rule key with the given name.
        /// </summary>
        /// <param name="name">the registry key name</param>
        /// <returns></returns>
        public RegistryKey CreateRuleKey(string name)
        {
            RegistryKey rulesKey = GetRulesKey();

            return rulesKey.CreateSubKey(name);
        }

        /// <summary>
        /// Opens a rule key with the given name.
        /// </summary>
        /// <param name="name">the registry key name</param>
        /// <returns></returns>
        public RegistryKey OpenRuleKey(string name)
        {
            RegistryKey rulesKey = GetRulesKey();

            return rulesKey.OpenSubKey(name, true);
        }

        /// <summary>
        /// Deletes a rule key with the given name.
        /// </summary>
        /// <param name="name">the registry key name</param>
        /// <returns></returns>
        public void DeleteRuleKey(int index)
        {
            RegistryKey rulesKey = GetRulesKey();
            string name = rulesKey.GetSubKeyNames()[index];
            rulesKey.DeleteSubKey(name, false);
        }

        /// <summary>
        /// Reads the sub keys of the key with the given name from the registry.
        /// </summary>
        /// <param name="name">the setting name</param>
        /// <returns>the setting value</returns>
        public RegistryKey[] GetKeys(string name)
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(absoluteKeyName + "\\" + name);
            if (key != null)
            {
                string[] keyNames = key.GetSubKeyNames();
                var keys = new RegistryKey[keyNames.Length];
                int i = 0;
                foreach (string keyName in keyNames)
                {
                    keys[i++] = key.OpenSubKey(keyName, true);
                }
                return keys;
            }
            else
            {
                // create the key if it wasn't present
                CreateKey(name);
                return new RegistryKey[0];
            }
        }
    }
}