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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

using AccCoreLib;

namespace acshbuddy08
{
    public class PrefsHook : IAccPreferencesHook
    {
        public static PrefsHook m_prefsHook;
        private Dictionary<string, string> prefDict;
        public PrefsHook()
        {
            // preference storage can be done in many different
            // ways.  We could use the Windows Registry, or we
            // can write an XML file, in this case we will use
            // a .net Dictionary object using string pairs.
            m_prefsHook = this;
            prefDict = new Dictionary<string, string>();
        }

        #region IAccPreferencesHook Members

        public object GetChildSpecifiers(string specifier)
        {
            // count the child specifiers
            // for example count the number of
            // child prefs in 'aimcc.connect'
            // see PrefsSpecifierSimplified.txt
            // in the 'docs' folder of the SDK
            /*
            string[] childSpec = null;
            Dictionary<string, string>.KeyCollection keyColl =
                prefDict.Keys;
            foreach (string s in keyColl)
            {
                if (s.Contains(specifier))
                {
                    int i = childSpec.Length;
                    childSpec[i] = s;
                }
            }
            return childSpec;
            */
            
            IList<string> childList = new List<string>();
            foreach (KeyValuePair<string, string> kvp in prefDict)
            {
                if (kvp.Value.Contains(specifier))
                {
                    childList.Add(kvp.Value);
                }
            }

            string[] childs = new string[childList.Count];
            int i = 0;
            foreach (string value in childList)
            {
                childs[i++] = value;
            }
            return childs;
            
        }

        public object GetDefaultValue(string specifier)
        {
            // we have no default values, so we will use ones
            // aimcc provides
            object o = null;
            Marshal.ThrowExceptionForHR((int)AccResult.ACC_E_NO_DATA);
            return o;
        }

        public object GetValue(string specifier)
        {
            string val = "";
            prefDict.TryGetValue(specifier, out val);
            if (prefDict.TryGetValue(specifier, out val))
                return val;
            else
                Marshal.ThrowExceptionForHR((int)AccResult.ACC_E_NO_DATA);

            return val;
        }

        public void Reset(string specifier)
        {
            try
            {
                prefDict.Remove(specifier);
            }
            catch
            {
                Marshal.ThrowExceptionForHR((int)AccResult.ACC_E_NO_DATA);
            }
        }

        public void SetValue(string specifier, object value)
        {
            try
            {
                prefDict[specifier] = (string)value;
            }
            catch (KeyNotFoundException)
            {
                prefDict.Add(specifier, (string)value);
            }
        }

        #endregion
    }
}
