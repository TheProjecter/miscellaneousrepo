namespace Recursor
{
    partial class Recursor
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
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.folder = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.browseButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.listRules = new System.Windows.Forms.ListBox();
            this.toReplace = new System.Windows.Forms.TextBox();
            this.saveRuleButton = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.replaceWith = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.replaceFiles = new System.Windows.Forms.TextBox();
            this.deleteRuleButton = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.fileFilter = new System.Windows.Forms.TextBox();
            this.saveFilterButton = new System.Windows.Forms.Button();
            this.goButton = new System.Windows.Forms.Button();
            this.addRuleButton = new System.Windows.Forms.Button();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.actionsLog = new System.Windows.Forms.TextBox();
            this.longLinesLog = new System.Windows.Forms.TextBox();
            this.stopButton = new System.Windows.Forms.Button();
            this.parseCheck = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // folder
            // 
            this.folder.Location = new System.Drawing.Point(67, 36);
            this.folder.Name = "folder";
            this.folder.Size = new System.Drawing.Size(341, 20);
            this.folder.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(22, 39);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(39, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Folder:";
            // 
            // browseButton
            // 
            this.browseButton.Location = new System.Drawing.Point(414, 34);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(51, 23);
            this.browseButton.TabIndex = 2;
            this.browseButton.Text = "...";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(26, 87);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Replace rules:";
            // 
            // listRules
            // 
            this.listRules.FormattingEnabled = true;
            this.listRules.Location = new System.Drawing.Point(29, 103);
            this.listRules.Name = "listRules";
            this.listRules.Size = new System.Drawing.Size(419, 95);
            this.listRules.TabIndex = 4;
            this.listRules.SelectedIndexChanged += new System.EventHandler(this.listRules_SelectedIndexChanged);
            // 
            // toReplace
            // 
            this.toReplace.Location = new System.Drawing.Point(79, 206);
            this.toReplace.Name = "toReplace";
            this.toReplace.Size = new System.Drawing.Size(101, 20);
            this.toReplace.TabIndex = 5;
            // 
            // saveRuleButton
            // 
            this.saveRuleButton.Location = new System.Drawing.Point(201, 233);
            this.saveRuleButton.Name = "saveRuleButton";
            this.saveRuleButton.Size = new System.Drawing.Size(75, 23);
            this.saveRuleButton.TabIndex = 6;
            this.saveRuleButton.Text = "Save";
            this.saveRuleButton.UseVisualStyleBackColor = true;
            this.saveRuleButton.Click += new System.EventHandler(this.saveRuleButton_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(26, 207);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Replace";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(32, 236);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "With";
            // 
            // replaceWith
            // 
            this.replaceWith.Location = new System.Drawing.Point(79, 233);
            this.replaceWith.Name = "replaceWith";
            this.replaceWith.Size = new System.Drawing.Size(100, 20);
            this.replaceWith.TabIndex = 9;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(30, 263);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(43, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "For files";
            // 
            // replaceFiles
            // 
            this.replaceFiles.Location = new System.Drawing.Point(79, 263);
            this.replaceFiles.Name = "replaceFiles";
            this.replaceFiles.Size = new System.Drawing.Size(101, 20);
            this.replaceFiles.TabIndex = 11;
            this.replaceFiles.Text = "*.*";
            // 
            // deleteRuleButton
            // 
            this.deleteRuleButton.Location = new System.Drawing.Point(201, 261);
            this.deleteRuleButton.Name = "deleteRuleButton";
            this.deleteRuleButton.Size = new System.Drawing.Size(75, 23);
            this.deleteRuleButton.TabIndex = 12;
            this.deleteRuleButton.Text = "Delete";
            this.deleteRuleButton.UseVisualStyleBackColor = true;
            this.deleteRuleButton.Click += new System.EventHandler(this.deleteRuleButton_Click);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(22, 305);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(130, 13);
            this.label6.TabIndex = 13;
            this.label6.Text = "Remove all files matching:";
            // 
            // fileFilter
            // 
            this.fileFilter.Location = new System.Drawing.Point(158, 302);
            this.fileFilter.Name = "fileFilter";
            this.fileFilter.Size = new System.Drawing.Size(151, 20);
            this.fileFilter.TabIndex = 14;
            // 
            // saveFilterButton
            // 
            this.saveFilterButton.Location = new System.Drawing.Point(315, 299);
            this.saveFilterButton.Name = "saveFilterButton";
            this.saveFilterButton.Size = new System.Drawing.Size(75, 23);
            this.saveFilterButton.TabIndex = 15;
            this.saveFilterButton.Text = "Save";
            this.saveFilterButton.UseVisualStyleBackColor = true;
            this.saveFilterButton.Click += new System.EventHandler(this.saveFilterButton_Click);
            // 
            // goButton
            // 
            this.goButton.Location = new System.Drawing.Point(118, 353);
            this.goButton.Name = "goButton";
            this.goButton.Size = new System.Drawing.Size(75, 23);
            this.goButton.TabIndex = 16;
            this.goButton.Text = "GO";
            this.goButton.UseVisualStyleBackColor = true;
            this.goButton.Click += new System.EventHandler(this.goButton_Click);
            // 
            // addRuleButton
            // 
            this.addRuleButton.Location = new System.Drawing.Point(201, 202);
            this.addRuleButton.Name = "addRuleButton";
            this.addRuleButton.Size = new System.Drawing.Size(75, 23);
            this.addRuleButton.TabIndex = 18;
            this.addRuleButton.Text = "Add";
            this.addRuleButton.UseVisualStyleBackColor = true;
            this.addRuleButton.Click += new System.EventHandler(this.addRuleButton_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(209, 353);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(239, 23);
            this.progressBar1.TabIndex = 19;
            // 
            // actionsLog
            // 
            this.actionsLog.Location = new System.Drawing.Point(12, 382);
            this.actionsLog.Multiline = true;
            this.actionsLog.Name = "actionsLog";
            this.actionsLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.actionsLog.Size = new System.Drawing.Size(477, 111);
            this.actionsLog.TabIndex = 20;
            // 
            // longLinesLog
            // 
            this.longLinesLog.Location = new System.Drawing.Point(12, 499);
            this.longLinesLog.Multiline = true;
            this.longLinesLog.Name = "longLinesLog";
            this.longLinesLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.longLinesLog.Size = new System.Drawing.Size(477, 133);
            this.longLinesLog.TabIndex = 21;
            // 
            // stopButton
            // 
            this.stopButton.Enabled = false;
            this.stopButton.Location = new System.Drawing.Point(29, 353);
            this.stopButton.Name = "stopButton";
            this.stopButton.Size = new System.Drawing.Size(75, 23);
            this.stopButton.TabIndex = 22;
            this.stopButton.Text = "Stop";
            this.stopButton.UseVisualStyleBackColor = true;
            this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // parseCheck
            // 
            this.parseCheck.AutoSize = true;
            this.parseCheck.Checked = true;
            this.parseCheck.CheckState = System.Windows.Forms.CheckState.Checked;
            this.parseCheck.Location = new System.Drawing.Point(29, 67);
            this.parseCheck.Name = "parseCheck";
            this.parseCheck.Size = new System.Drawing.Size(169, 17);
            this.parseCheck.TabIndex = 23;
            this.parseCheck.Text = "Parse files for line replacement";
            this.parseCheck.UseVisualStyleBackColor = true;
            // 
            // Recursor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(501, 644);
            this.Controls.Add(this.parseCheck);
            this.Controls.Add(this.stopButton);
            this.Controls.Add(this.longLinesLog);
            this.Controls.Add(this.actionsLog);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.addRuleButton);
            this.Controls.Add(this.goButton);
            this.Controls.Add(this.saveFilterButton);
            this.Controls.Add(this.fileFilter);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.deleteRuleButton);
            this.Controls.Add(this.replaceFiles);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.replaceWith);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.saveRuleButton);
            this.Controls.Add(this.toReplace);
            this.Controls.Add(this.listRules);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.browseButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.folder);
            this.Name = "Recursor";
            this.Text = "Recursor";
            this.Load += new System.EventHandler(this.Recursor_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.TextBox folder;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListBox listRules;
        private System.Windows.Forms.TextBox toReplace;
        private System.Windows.Forms.Button saveRuleButton;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox replaceWith;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox replaceFiles;
        private System.Windows.Forms.Button deleteRuleButton;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox fileFilter;
        private System.Windows.Forms.Button saveFilterButton;
        private System.Windows.Forms.Button goButton;
        private System.Windows.Forms.Button addRuleButton;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.TextBox actionsLog;
        private System.Windows.Forms.TextBox longLinesLog;
        private System.Windows.Forms.Button stopButton;
        private System.Windows.Forms.CheckBox parseCheck;
    }
}

