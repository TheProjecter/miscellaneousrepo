using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;

namespace Plugin
{
    public partial class NotifyListForm : Form
    {
        public NotifyListForm()
        {
            InitializeComponent();
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            // move the selected items over to the first list box
            IList<string> items = new List<string>();

            for (int i = 0; i < AvailableBuddies.SelectedItems.Count; i++)
            {
                items.Add(AvailableBuddies.SelectedItems[i].ToString());
            }

            foreach (string item in items)
            {
                AvailableBuddies.Items.Remove(item);
                NotifyBuddies.Items.Add(item);
            }
        }

        private void DeleteButton_Click(object sender, EventArgs e)
        {
            // move the selected items back to the second list box
            IList<string> items = new List<string>();

            for (int i = 0; i < NotifyBuddies.SelectedItems.Count; i++)
            {
                items.Add(NotifyBuddies.SelectedItems[i].ToString());
            }

            foreach (string item in items)
            {
                NotifyBuddies.Items.Remove(item);
                AvailableBuddies.Items.Add(item);
            }
        }

        private void SaveList_Click(object sender, EventArgs e)
        {
            if (Db.NewFeeds.ContainsKey(EditingFeed) || Db.FeedData.ContainsKey(EditingFeed))
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + EditingFeed + "\\NotifyList", true);

                // check first listbox
                for (int i = 0; i < NotifyBuddies.Items.Count; i++)
                {
                    string item = NotifyBuddies.Items[i].ToString();
                    // check if this item is already in the notify list, if not add it (and to registry as well)
                    if (!NotifyList.Contains(item))
                    {
                        NotifyList.Add(item);
                        key.CreateSubKey(item);
                    }
                }

                // now check second listbox
                for (int i = 0; i < AvailableBuddies.Items.Count; i++)
                {
                    string item = AvailableBuddies.Items[i].ToString();
                    // check if this item is in the notify list, if it is delete it (and from registry as well)
                    if (NotifyList.Contains(item))
                    {
                        NotifyList.Remove(item);
                        key.DeleteSubKey(item);
                    }
                }

                // update notify enabled and notify list
                RegistryKey feedKey = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + EditingFeed, true);
                feedKey.SetValue("NotifyEnabled", NotifyEnabled.Checked ? "1" : "0");

                if (Db.FeedData.ContainsKey(EditingFeed))
                {
                    Db.FeedData[EditingFeed].NotifyList = NotifyList;
                    Db.FeedData[EditingFeed].NotifyEnabled = NotifyEnabled.Checked;
                }
                else
                {
                    Db.NewFeeds[EditingFeed].NotifyList = NotifyList;
                    Db.NewFeeds[EditingFeed].NotifyEnabled = NotifyEnabled.Checked;
                }
            }
        }
    }
}