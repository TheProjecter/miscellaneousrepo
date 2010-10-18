namespace Plugin
{
    partial class SettingsForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingsForm));
            this.IntervalLabel = new System.Windows.Forms.Label();
            this.Interval = new System.Windows.Forms.TextBox();
            this.SaveSettings = new System.Windows.Forms.Button();
            this.SoundLabel = new System.Windows.Forms.Label();
            this.OpenFile = new System.Windows.Forms.OpenFileDialog();
            this.PickFile = new System.Windows.Forms.Button();
            this.Sound = new System.Windows.Forms.TextBox();
            this.UseProxy = new System.Windows.Forms.CheckBox();
            this.ProxyHostLabel = new System.Windows.Forms.Label();
            this.ProxyPortLabel = new System.Windows.Forms.Label();
            this.ProxyHost = new System.Windows.Forms.TextBox();
            this.ProxyPort = new System.Windows.Forms.TextBox();
            this.SoundEnabled = new System.Windows.Forms.CheckBox();
            this.NoAlerts = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // IntervalLabel
            // 
            this.IntervalLabel.AutoSize = true;
            this.IntervalLabel.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.IntervalLabel.Location = new System.Drawing.Point(22, 25);
            this.IntervalLabel.Name = "IntervalLabel";
            this.IntervalLabel.Size = new System.Drawing.Size(146, 13);
            this.IntervalLabel.TabIndex = 15;
            this.IntervalLabel.Text = "Update interval (in sec):";
            // 
            // Interval
            // 
            this.Interval.Location = new System.Drawing.Point(174, 22);
            this.Interval.Name = "Interval";
            this.Interval.Size = new System.Drawing.Size(100, 20);
            this.Interval.TabIndex = 16;
            // 
            // SaveSettings
            // 
            this.SaveSettings.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.SaveSettings.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SaveSettings.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.SaveSettings.Location = new System.Drawing.Point(58, 211);
            this.SaveSettings.Name = "SaveSettings";
            this.SaveSettings.Size = new System.Drawing.Size(96, 23);
            this.SaveSettings.TabIndex = 17;
            this.SaveSettings.Text = "Save settings";
            this.SaveSettings.UseVisualStyleBackColor = false;
            this.SaveSettings.Click += new System.EventHandler(this.SaveSettings_Click);
            // 
            // SoundLabel
            // 
            this.SoundLabel.AutoSize = true;
            this.SoundLabel.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SoundLabel.Location = new System.Drawing.Point(21, 96);
            this.SoundLabel.Name = "SoundLabel";
            this.SoundLabel.Size = new System.Drawing.Size(120, 13);
            this.SoundLabel.TabIndex = 18;
            this.SoundLabel.Text = "Update alert sound:";
            // 
            // OpenFile
            // 
            this.OpenFile.Filter = "Wave Sound|*.wav";
            // 
            // PickFile
            // 
            this.PickFile.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.PickFile.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PickFile.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.PickFile.ImageAlign = System.Drawing.ContentAlignment.BottomLeft;
            this.PickFile.Location = new System.Drawing.Point(337, 93);
            this.PickFile.Name = "PickFile";
            this.PickFile.Size = new System.Drawing.Size(27, 20);
            this.PickFile.TabIndex = 19;
            this.PickFile.Text = "...";
            this.PickFile.UseVisualStyleBackColor = false;
            this.PickFile.Click += new System.EventHandler(this.PickFile_Click);
            // 
            // Sound
            // 
            this.Sound.Location = new System.Drawing.Point(147, 93);
            this.Sound.Name = "Sound";
            this.Sound.Size = new System.Drawing.Size(174, 20);
            this.Sound.TabIndex = 20;
            // 
            // UseProxy
            // 
            this.UseProxy.AutoSize = true;
            this.UseProxy.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.UseProxy.Location = new System.Drawing.Point(24, 121);
            this.UseProxy.Name = "UseProxy";
            this.UseProxy.Size = new System.Drawing.Size(84, 17);
            this.UseProxy.TabIndex = 22;
            this.UseProxy.Text = "Use proxy";
            this.UseProxy.UseVisualStyleBackColor = true;
            this.UseProxy.CheckedChanged += new System.EventHandler(this.UseProxy_CheckedChanged);
            // 
            // ProxyHostLabel
            // 
            this.ProxyHostLabel.AutoSize = true;
            this.ProxyHostLabel.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ProxyHostLabel.Location = new System.Drawing.Point(21, 147);
            this.ProxyHostLabel.Name = "ProxyHostLabel";
            this.ProxyHostLabel.Size = new System.Drawing.Size(73, 13);
            this.ProxyHostLabel.TabIndex = 23;
            this.ProxyHostLabel.Text = "Proxy host:";
            // 
            // ProxyPortLabel
            // 
            this.ProxyPortLabel.AutoSize = true;
            this.ProxyPortLabel.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ProxyPortLabel.Location = new System.Drawing.Point(22, 170);
            this.ProxyPortLabel.Name = "ProxyPortLabel";
            this.ProxyPortLabel.Size = new System.Drawing.Size(72, 13);
            this.ProxyPortLabel.TabIndex = 24;
            this.ProxyPortLabel.Text = "Proxy port:";
            // 
            // ProxyHost
            // 
            this.ProxyHost.Location = new System.Drawing.Point(100, 144);
            this.ProxyHost.Name = "ProxyHost";
            this.ProxyHost.Size = new System.Drawing.Size(138, 20);
            this.ProxyHost.TabIndex = 25;
            // 
            // ProxyPort
            // 
            this.ProxyPort.Location = new System.Drawing.Point(100, 170);
            this.ProxyPort.Name = "ProxyPort";
            this.ProxyPort.Size = new System.Drawing.Size(138, 20);
            this.ProxyPort.TabIndex = 26;
            // 
            // SoundEnabled
            // 
            this.SoundEnabled.AutoSize = true;
            this.SoundEnabled.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SoundEnabled.Location = new System.Drawing.Point(24, 70);
            this.SoundEnabled.Name = "SoundEnabled";
            this.SoundEnabled.Size = new System.Drawing.Size(243, 17);
            this.SoundEnabled.TabIndex = 27;
            this.SoundEnabled.Text = "Play sound when new items are found";
            this.SoundEnabled.UseVisualStyleBackColor = true;
            this.SoundEnabled.CheckedChanged += new System.EventHandler(this.SoundEnabled_CheckedChanged);
            // 
            // NoAlerts
            // 
            this.NoAlerts.AutoSize = true;
            this.NoAlerts.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NoAlerts.Location = new System.Drawing.Point(24, 43);
            this.NoAlerts.Name = "NoAlerts";
            this.NoAlerts.Size = new System.Drawing.Size(263, 17);
            this.NoAlerts.TabIndex = 28;
            this.NoAlerts.Text = "Don\'t alert me when new items are found";
            this.NoAlerts.UseVisualStyleBackColor = true;
            this.NoAlerts.CheckedChanged += new System.EventHandler(this.NoAlerts_CheckedChanged);
            // 
            // SettingsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(392, 246);
            this.Controls.Add(this.NoAlerts);
            this.Controls.Add(this.SoundEnabled);
            this.Controls.Add(this.ProxyPort);
            this.Controls.Add(this.ProxyHost);
            this.Controls.Add(this.ProxyPortLabel);
            this.Controls.Add(this.ProxyHostLabel);
            this.Controls.Add(this.UseProxy);
            this.Controls.Add(this.Sound);
            this.Controls.Add(this.PickFile);
            this.Controls.Add(this.SoundLabel);
            this.Controls.Add(this.SaveSettings);
            this.Controls.Add(this.Interval);
            this.Controls.Add(this.IntervalLabel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(400, 280);
            this.MinimumSize = new System.Drawing.Size(400, 280);
            this.Name = "SettingsForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Feed Reader Settings";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label IntervalLabel;
        public System.Windows.Forms.TextBox Interval;
        private System.Windows.Forms.Button SaveSettings;
        private System.Windows.Forms.Label SoundLabel;
        public System.Windows.Forms.OpenFileDialog OpenFile;
        public System.Windows.Forms.Button PickFile;
        public System.Windows.Forms.TextBox Sound;
        public System.Windows.Forms.CheckBox UseProxy;
        private System.Windows.Forms.Label ProxyHostLabel;
        private System.Windows.Forms.Label ProxyPortLabel;
        public System.Windows.Forms.TextBox ProxyHost;
        public System.Windows.Forms.TextBox ProxyPort;
        public System.Windows.Forms.CheckBox SoundEnabled;
        public System.Windows.Forms.CheckBox NoAlerts;
    }
}