using System.Collections.Generic;

namespace Plugin
{
    partial class NotifyListForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(NotifyListForm));
            this.NotifyBuddies = new System.Windows.Forms.ListBox();
            this.AvailableBuddies = new System.Windows.Forms.ListBox();
            this.NotifyEnabled = new System.Windows.Forms.CheckBox();
            this.SaveList = new System.Windows.Forms.Button();
            this.AddButton = new System.Windows.Forms.Button();
            this.DeleteButton = new System.Windows.Forms.Button();
            this.BuddyLabel1 = new System.Windows.Forms.Label();
            this.BuddyLabel2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // NotifyBuddies
            // 
            this.NotifyBuddies.FormattingEnabled = true;
            this.NotifyBuddies.Location = new System.Drawing.Point(12, 42);
            this.NotifyBuddies.Name = "NotifyBuddies";
            this.NotifyBuddies.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.NotifyBuddies.Size = new System.Drawing.Size(159, 121);
            this.NotifyBuddies.TabIndex = 0;
            // 
            // AvailableBuddies
            // 
            this.AvailableBuddies.FormattingEnabled = true;
            this.AvailableBuddies.Location = new System.Drawing.Point(216, 42);
            this.AvailableBuddies.Name = "AvailableBuddies";
            this.AvailableBuddies.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.AvailableBuddies.Size = new System.Drawing.Size(177, 121);
            this.AvailableBuddies.TabIndex = 1;
            // 
            // NotifyEnabled
            // 
            this.NotifyEnabled.AutoSize = true;
            this.NotifyEnabled.Location = new System.Drawing.Point(112, 179);
            this.NotifyEnabled.Name = "NotifyEnabled";
            this.NotifyEnabled.Size = new System.Drawing.Size(145, 17);
            this.NotifyEnabled.TabIndex = 2;
            this.NotifyEnabled.Text = "Enable buddy notification";
            this.NotifyEnabled.UseVisualStyleBackColor = true;
            // 
            // SaveList
            // 
            this.SaveList.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.SaveList.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.SaveList.Location = new System.Drawing.Point(138, 202);
            this.SaveList.Name = "SaveList";
            this.SaveList.Size = new System.Drawing.Size(103, 23);
            this.SaveList.TabIndex = 3;
            this.SaveList.Text = "Save";
            this.SaveList.UseVisualStyleBackColor = false;
            this.SaveList.Click += new System.EventHandler(this.SaveList_Click);
            // 
            // AddButton
            // 
            this.AddButton.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.AddButton.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.AddButton.Location = new System.Drawing.Point(177, 68);
            this.AddButton.Name = "AddButton";
            this.AddButton.Size = new System.Drawing.Size(33, 23);
            this.AddButton.TabIndex = 4;
            this.AddButton.Text = "<<";
            this.AddButton.UseVisualStyleBackColor = false;
            this.AddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // DeleteButton
            // 
            this.DeleteButton.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.DeleteButton.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.DeleteButton.Location = new System.Drawing.Point(177, 115);
            this.DeleteButton.Name = "DeleteButton";
            this.DeleteButton.Size = new System.Drawing.Size(33, 23);
            this.DeleteButton.TabIndex = 5;
            this.DeleteButton.Text = ">>";
            this.DeleteButton.UseVisualStyleBackColor = false;
            this.DeleteButton.Click += new System.EventHandler(this.DeleteButton_Click);
            // 
            // BuddyLabel1
            // 
            this.BuddyLabel1.AutoSize = true;
            this.BuddyLabel1.Location = new System.Drawing.Point(40, 17);
            this.BuddyLabel1.Name = "BuddyLabel1";
            this.BuddyLabel1.Size = new System.Drawing.Size(88, 13);
            this.BuddyLabel1.TabIndex = 6;
            this.BuddyLabel1.Text = "Buddies to notify:";
            // 
            // BuddyLabel2
            // 
            this.BuddyLabel2.AutoSize = true;
            this.BuddyLabel2.Location = new System.Drawing.Point(242, 17);
            this.BuddyLabel2.Name = "BuddyLabel2";
            this.BuddyLabel2.Size = new System.Drawing.Size(93, 13);
            this.BuddyLabel2.TabIndex = 7;
            this.BuddyLabel2.Text = "Available buddies:";
            // 
            // NotifyListForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(409, 235);
            this.Controls.Add(this.BuddyLabel2);
            this.Controls.Add(this.BuddyLabel1);
            this.Controls.Add(this.DeleteButton);
            this.Controls.Add(this.AddButton);
            this.Controls.Add(this.SaveList);
            this.Controls.Add(this.NotifyEnabled);
            this.Controls.Add(this.AvailableBuddies);
            this.Controls.Add(this.NotifyBuddies);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximumSize = new System.Drawing.Size(417, 269);
            this.MinimumSize = new System.Drawing.Size(417, 269);
            this.Name = "NotifyListForm";
            this.Text = "Notify list for";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        public System.Windows.Forms.ListBox NotifyBuddies;
        public System.Windows.Forms.ListBox AvailableBuddies;
        public System.Windows.Forms.CheckBox NotifyEnabled;
        private System.Windows.Forms.Button SaveList;
        private System.Windows.Forms.Button AddButton;
        private System.Windows.Forms.Button DeleteButton;
        private System.Windows.Forms.Label BuddyLabel1;
        private System.Windows.Forms.Label BuddyLabel2;
        public string EditingFeed = "";
        public IList<string> NotifyList = null;
    }
}