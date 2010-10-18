using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Net;
using System.IO;

namespace Plugin
{
    public partial class SettingsForm : Form
    {
        public SettingsForm()
        {
            InitializeComponent();
        }

        private void SaveSettings_Click(object sender, EventArgs e)
        {
            bool error = false;
            try
            {
                Helper.CheckInt(Interval.Text, "interval", FeedReader.MIN_INTERVAL);

                if (UseProxy.Checked)
                {
                    // check host
                    Helper.CheckString(ProxyHost.Text, "proxy host");
                    try
                    {
                        Dns.GetHostEntry(ProxyHost.Text);
                    }
                    catch (Exception)
                    {
                        throw new ArgumentException("Proxy host cannot be resolved.");
                    }

                    // check port
                    Helper.CheckInt(ProxyPort.Text, "proxy port", 1, 65535);
                }

                if (SoundEnabled.Checked)
                {
                    Helper.CheckString(Sound.Text, "sound");
                    if (!File.Exists(Sound.Text))
                    {
                        MessageBox.Show("This sound file does not exist on your hard drive.");
                        error = true;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                error = true;
            }


            if (SoundEnabled.Checked && !error)
            {
                try
                {
                    if (!Helper.PlaySound(Sound.Text, 0, 1))
                    {
                        MessageBox.Show("This sound file is not a valid WAV Sound file.");
                        error = true;
                    }
                }
                catch (Exception)
                {
                    MessageBox.Show("This sound file is not a valid WAV Sound file.");
                    error = true;
                }
            }

            if (!error)
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(
                    FeedReader.PluginKey + "\\Settings", true);
                int interval = Int32.Parse(Interval.Text);
                key.SetValue("Interval", (interval * 1000).ToString());
                key.SetValue("SoundEnabled", (SoundEnabled.Checked ? "1" : "0"));
                key.SetValue("Alerts", (NoAlerts.Checked ? "0" : "1"));
                key.SetValue("Sound", Sound.Text);
                key.SetValue("UseProxy", (UseProxy.Checked ? "1" : "0"));
                key.SetValue("ProxyHost", ProxyHost.Text);
                key.SetValue("ProxyPort", ProxyPort.Text);

                // reschedule the timer
                FeedReader.TimerEntity.Change(interval * 1000, interval * 1000);
            }
        }

        private void PickFile_Click(object sender, EventArgs e)
        {
            // get the initial directory
            OpenFile.FileName = Sound.Text;
            OpenFile.InitialDirectory = Helper.ParseDirectory(Sound.Text);

            // pick a file
            try
            {
                if (OpenFile.ShowDialog() == DialogResult.OK)
                {
                    Sound.Text = OpenFile.FileName;
                }
            }
            catch (Exception)
            {
                // bad file name, clear it
                OpenFile.FileName = "";
                Sound.Text = "";
                if (OpenFile.ShowDialog() == DialogResult.OK)
                {
                    Sound.Text = OpenFile.FileName;
                }
            }
        }

        private void UseProxy_CheckedChanged(object sender, EventArgs e)
        {
            ProxyHost.Enabled = UseProxy.Checked;
            ProxyPort.Enabled = UseProxy.Checked;
        }

        private void SoundEnabled_CheckedChanged(object sender, EventArgs e)
        {
            Sound.Enabled = SoundEnabled.Checked;
            PickFile.Enabled = SoundEnabled.Checked;
        }

        private void NoAlerts_CheckedChanged(object sender, EventArgs e)
        {
            if (NoAlerts.Checked)
            {
                SoundEnabled.Enabled = false;
                SoundEnabled.Checked = false;
            }
            else
            {
                SoundEnabled.Enabled = true;
            }
        }
    }
}