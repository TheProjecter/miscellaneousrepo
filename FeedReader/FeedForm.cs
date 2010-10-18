using System;
using System.Drawing;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using Microsoft.Win32;
using System.Text;
using AccCoreLib;

namespace Plugin
{
    /// <summary>
    /// Summary description for FeedReader.
    /// </summary>
    public class FeedForm : System.Windows.Forms.Form
    {
        public System.Windows.Forms.TreeView TreeView;
        private IContainer components;
        public GroupBox Panel;
        public LinkLabel LinkAnchor;
        public Label Link;
        public Label Description;
        public Label FeedType;
        public WebBrowser WebBrowser;
        private Button DeleteFeed;
        private Label AddFeedNameLabel;
        private TextBox AddFeedName;
        private TextBox AddFeedLink;
        private Label AddFeedLinkLabel;
        private Label AddFeedError;
        private Button AddFeed;
        public Label LastUpdate;
        private ImageList Images;
        public string SelectedFeed = null;
        private Button DetachBrowser;
        private bool Clicked = false;
        private Label DateRetrieved;
        public ComboBox Filter;
        private Label FilterLabel;
        private Label SearchLabel;
        public TextBox Search;
        private Button Export;
        private Label ExportLabel;
        private Button PickExportFile;
        private TextBox ExportBox;
        private TextBox ImportBox;
        private Button Import;
        private Label ImportLabel;
        private Button PickImportFile;
        private SaveFileDialog ExportFile;
        private OpenFileDialog ImportFile;
        private BrowserForm browserForm = null;
        private bool Importing = false;
        private bool Exporting = false;
        private Label ImportProgress;
        private Label ExportProgress;
        private CheckBox UpdateStatus;
        private Button EditNotifyList;
        private bool DoNotTriggerAfterSelect = false;
        private NotifyListForm NotifyForm = null;

        public FeedForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if (components != null) 
                {
                    components.Dispose();
                }
            }
            base.Dispose( disposing );
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.TreeNode treeNode1 = new System.Windows.Forms.TreeNode("Feeds");
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FeedForm));
            this.TreeView = new System.Windows.Forms.TreeView();
            this.Images = new System.Windows.Forms.ImageList(this.components);
            this.Panel = new System.Windows.Forms.GroupBox();
            this.UpdateStatus = new System.Windows.Forms.CheckBox();
            this.ImportProgress = new System.Windows.Forms.Label();
            this.ExportProgress = new System.Windows.Forms.Label();
            this.PickImportFile = new System.Windows.Forms.Button();
            this.ImportBox = new System.Windows.Forms.TextBox();
            this.Import = new System.Windows.Forms.Button();
            this.ImportLabel = new System.Windows.Forms.Label();
            this.PickExportFile = new System.Windows.Forms.Button();
            this.ExportBox = new System.Windows.Forms.TextBox();
            this.ExportLabel = new System.Windows.Forms.Label();
            this.Export = new System.Windows.Forms.Button();
            this.DateRetrieved = new System.Windows.Forms.Label();
            this.DetachBrowser = new System.Windows.Forms.Button();
            this.LastUpdate = new System.Windows.Forms.Label();
            this.AddFeedError = new System.Windows.Forms.Label();
            this.AddFeed = new System.Windows.Forms.Button();
            this.AddFeedLink = new System.Windows.Forms.TextBox();
            this.AddFeedLinkLabel = new System.Windows.Forms.Label();
            this.AddFeedNameLabel = new System.Windows.Forms.Label();
            this.AddFeedName = new System.Windows.Forms.TextBox();
            this.DeleteFeed = new System.Windows.Forms.Button();
            this.WebBrowser = new System.Windows.Forms.WebBrowser();
            this.FeedType = new System.Windows.Forms.Label();
            this.Description = new System.Windows.Forms.Label();
            this.LinkAnchor = new System.Windows.Forms.LinkLabel();
            this.Link = new System.Windows.Forms.Label();
            this.Filter = new System.Windows.Forms.ComboBox();
            this.FilterLabel = new System.Windows.Forms.Label();
            this.SearchLabel = new System.Windows.Forms.Label();
            this.Search = new System.Windows.Forms.TextBox();
            this.ExportFile = new System.Windows.Forms.SaveFileDialog();
            this.ImportFile = new System.Windows.Forms.OpenFileDialog();
            this.EditNotifyList = new System.Windows.Forms.Button();
            this.Panel.SuspendLayout();
            this.SuspendLayout();
            // 
            // TreeView
            // 
            this.TreeView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.TreeView.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TreeView.HotTracking = true;
            this.TreeView.ImageIndex = 0;
            this.TreeView.ImageList = this.Images;
            this.TreeView.Indent = 15;
            this.TreeView.ItemHeight = 18;
            this.TreeView.Location = new System.Drawing.Point(8, 54);
            this.TreeView.Name = "TreeView";
            treeNode1.Name = "";
            treeNode1.Text = "Feeds";
            this.TreeView.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
            treeNode1});
            this.TreeView.SelectedImageIndex = 0;
            this.TreeView.Size = new System.Drawing.Size(448, 378);
            this.TreeView.TabIndex = 0;
            this.TreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.TreeView_AfterSelect);
            // 
            // Images
            // 
            this.Images.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("Images.ImageStream")));
            this.Images.TransparentColor = System.Drawing.Color.Transparent;
            this.Images.Images.SetKeyName(0, "check.ico");
            this.Images.Images.SetKeyName(1, "new.ico");
            // 
            // Panel
            // 
            this.Panel.Controls.Add(this.EditNotifyList);
            this.Panel.Controls.Add(this.UpdateStatus);
            this.Panel.Controls.Add(this.ImportProgress);
            this.Panel.Controls.Add(this.ExportProgress);
            this.Panel.Controls.Add(this.PickImportFile);
            this.Panel.Controls.Add(this.ImportBox);
            this.Panel.Controls.Add(this.Import);
            this.Panel.Controls.Add(this.ImportLabel);
            this.Panel.Controls.Add(this.PickExportFile);
            this.Panel.Controls.Add(this.ExportBox);
            this.Panel.Controls.Add(this.ExportLabel);
            this.Panel.Controls.Add(this.Export);
            this.Panel.Controls.Add(this.DateRetrieved);
            this.Panel.Controls.Add(this.DetachBrowser);
            this.Panel.Controls.Add(this.LastUpdate);
            this.Panel.Controls.Add(this.AddFeedError);
            this.Panel.Controls.Add(this.AddFeed);
            this.Panel.Controls.Add(this.AddFeedLink);
            this.Panel.Controls.Add(this.AddFeedLinkLabel);
            this.Panel.Controls.Add(this.AddFeedNameLabel);
            this.Panel.Controls.Add(this.AddFeedName);
            this.Panel.Controls.Add(this.DeleteFeed);
            this.Panel.Controls.Add(this.WebBrowser);
            this.Panel.Controls.Add(this.FeedType);
            this.Panel.Controls.Add(this.Description);
            this.Panel.Controls.Add(this.LinkAnchor);
            this.Panel.Controls.Add(this.Link);
            this.Panel.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Panel.Location = new System.Drawing.Point(462, 54);
            this.Panel.Name = "Panel";
            this.Panel.Size = new System.Drawing.Size(353, 378);
            this.Panel.TabIndex = 1;
            this.Panel.TabStop = false;
            this.Panel.Text = "Feeds";
            // 
            // UpdateStatus
            // 
            this.UpdateStatus.AutoSize = true;
            this.UpdateStatus.Location = new System.Drawing.Point(9, 323);
            this.UpdateStatus.Name = "UpdateStatus";
            this.UpdateStatus.Size = new System.Drawing.Size(280, 17);
            this.UpdateStatus.TabIndex = 26;
            this.UpdateStatus.Text = "Change my status when this feed is updated";
            this.UpdateStatus.UseVisualStyleBackColor = true;
            this.UpdateStatus.Visible = false;
            this.UpdateStatus.CheckedChanged += new System.EventHandler(this.UpdateStatus_CheckedChanged);
            // 
            // ImportProgress
            // 
            this.ImportProgress.AutoSize = true;
            this.ImportProgress.Location = new System.Drawing.Point(154, 220);
            this.ImportProgress.Name = "ImportProgress";
            this.ImportProgress.Size = new System.Drawing.Size(122, 13);
            this.ImportProgress.TabIndex = 25;
            this.ImportProgress.Text = "Import in progress..";
            this.ImportProgress.Visible = false;
            // 
            // ExportProgress
            // 
            this.ExportProgress.AutoSize = true;
            this.ExportProgress.Location = new System.Drawing.Point(154, 162);
            this.ExportProgress.Name = "ExportProgress";
            this.ExportProgress.Size = new System.Drawing.Size(120, 13);
            this.ExportProgress.TabIndex = 24;
            this.ExportProgress.Text = "Export in progress..";
            this.ExportProgress.Visible = false;
            // 
            // PickImportFile
            // 
            this.PickImportFile.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.PickImportFile.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.PickImportFile.Location = new System.Drawing.Point(213, 242);
            this.PickImportFile.Name = "PickImportFile";
            this.PickImportFile.Size = new System.Drawing.Size(32, 21);
            this.PickImportFile.TabIndex = 23;
            this.PickImportFile.Text = "...";
            this.PickImportFile.UseVisualStyleBackColor = false;
            this.PickImportFile.Click += new System.EventHandler(this.PickImportFile_Click);
            // 
            // ImportBox
            // 
            this.ImportBox.Location = new System.Drawing.Point(57, 242);
            this.ImportBox.Name = "ImportBox";
            this.ImportBox.Size = new System.Drawing.Size(150, 21);
            this.ImportBox.TabIndex = 22;
            // 
            // Import
            // 
            this.Import.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.Import.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.Import.Location = new System.Drawing.Point(13, 215);
            this.Import.Name = "Import";
            this.Import.Size = new System.Drawing.Size(117, 23);
            this.Import.TabIndex = 21;
            this.Import.Text = "Import feed list";
            this.Import.UseVisualStyleBackColor = false;
            this.Import.Click += new System.EventHandler(this.Import_Click);
            // 
            // ImportLabel
            // 
            this.ImportLabel.AutoSize = true;
            this.ImportLabel.Location = new System.Drawing.Point(10, 245);
            this.ImportLabel.Name = "ImportLabel";
            this.ImportLabel.Size = new System.Drawing.Size(41, 13);
            this.ImportLabel.TabIndex = 20;
            this.ImportLabel.Text = "From:";
            // 
            // PickExportFile
            // 
            this.PickExportFile.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.PickExportFile.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.PickExportFile.Location = new System.Drawing.Point(213, 188);
            this.PickExportFile.Name = "PickExportFile";
            this.PickExportFile.Size = new System.Drawing.Size(32, 21);
            this.PickExportFile.TabIndex = 19;
            this.PickExportFile.Text = "...";
            this.PickExportFile.UseVisualStyleBackColor = false;
            this.PickExportFile.Click += new System.EventHandler(this.PickExportFile_Click);
            // 
            // ExportBox
            // 
            this.ExportBox.Location = new System.Drawing.Point(57, 188);
            this.ExportBox.Name = "ExportBox";
            this.ExportBox.Size = new System.Drawing.Size(150, 21);
            this.ExportBox.TabIndex = 18;
            // 
            // ExportLabel
            // 
            this.ExportLabel.AutoSize = true;
            this.ExportLabel.Location = new System.Drawing.Point(10, 191);
            this.ExportLabel.Name = "ExportLabel";
            this.ExportLabel.Size = new System.Drawing.Size(26, 13);
            this.ExportLabel.TabIndex = 17;
            this.ExportLabel.Text = "To:";
            // 
            // Export
            // 
            this.Export.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.Export.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.Export.Location = new System.Drawing.Point(9, 161);
            this.Export.Name = "Export";
            this.Export.Size = new System.Drawing.Size(121, 23);
            this.Export.TabIndex = 16;
            this.Export.Text = "Export feed list";
            this.Export.UseVisualStyleBackColor = false;
            this.Export.Click += new System.EventHandler(this.Export_Click);
            // 
            // DateRetrieved
            // 
            this.DateRetrieved.AutoSize = true;
            this.DateRetrieved.Location = new System.Drawing.Point(6, 261);
            this.DateRetrieved.Name = "DateRetrieved";
            this.DateRetrieved.Size = new System.Drawing.Size(95, 13);
            this.DateRetrieved.TabIndex = 15;
            this.DateRetrieved.Text = "Date retrieved:";
            this.DateRetrieved.Visible = false;
            // 
            // DetachBrowser
            // 
            this.DetachBrowser.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.DetachBrowser.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.DetachBrowser.Location = new System.Drawing.Point(94, 178);
            this.DetachBrowser.Name = "DetachBrowser";
            this.DetachBrowser.Size = new System.Drawing.Size(144, 20);
            this.DetachBrowser.TabIndex = 14;
            this.DetachBrowser.Text = "Detach browser";
            this.DetachBrowser.UseVisualStyleBackColor = false;
            this.DetachBrowser.Visible = false;
            this.DetachBrowser.Click += new System.EventHandler(this.DetachBrowser_Click);
            // 
            // LastUpdate
            // 
            this.LastUpdate.AutoSize = true;
            this.LastUpdate.Location = new System.Drawing.Point(6, 296);
            this.LastUpdate.Name = "LastUpdate";
            this.LastUpdate.Size = new System.Drawing.Size(116, 13);
            this.LastUpdate.TabIndex = 13;
            this.LastUpdate.Text = "Last update: Never";
            this.LastUpdate.Visible = false;
            // 
            // AddFeedError
            // 
            this.AddFeedError.AutoSize = true;
            this.AddFeedError.Location = new System.Drawing.Point(9, 128);
            this.AddFeedError.MaximumSize = new System.Drawing.Size(250, 0);
            this.AddFeedError.Name = "AddFeedError";
            this.AddFeedError.Size = new System.Drawing.Size(42, 13);
            this.AddFeedError.TabIndex = 12;
            this.AddFeedError.Text = "errors";
            this.AddFeedError.Visible = false;
            // 
            // AddFeed
            // 
            this.AddFeed.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.AddFeed.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.AddFeed.Location = new System.Drawing.Point(94, 93);
            this.AddFeed.Name = "AddFeed";
            this.AddFeed.Size = new System.Drawing.Size(75, 23);
            this.AddFeed.TabIndex = 11;
            this.AddFeed.Text = "Add Feed";
            this.AddFeed.UseVisualStyleBackColor = false;
            this.AddFeed.Click += new System.EventHandler(this.AddFeed_Click);
            // 
            // AddFeedLink
            // 
            this.AddFeedLink.Location = new System.Drawing.Point(57, 66);
            this.AddFeedLink.Name = "AddFeedLink";
            this.AddFeedLink.Size = new System.Drawing.Size(290, 21);
            this.AddFeedLink.TabIndex = 10;
            // 
            // AddFeedLinkLabel
            // 
            this.AddFeedLinkLabel.AutoSize = true;
            this.AddFeedLinkLabel.Location = new System.Drawing.Point(6, 64);
            this.AddFeedLinkLabel.Name = "AddFeedLinkLabel";
            this.AddFeedLinkLabel.Size = new System.Drawing.Size(35, 13);
            this.AddFeedLinkLabel.TabIndex = 9;
            this.AddFeedLinkLabel.Text = "Link:";
            // 
            // AddFeedNameLabel
            // 
            this.AddFeedNameLabel.AutoSize = true;
            this.AddFeedNameLabel.Location = new System.Drawing.Point(6, 35);
            this.AddFeedNameLabel.Name = "AddFeedNameLabel";
            this.AddFeedNameLabel.Size = new System.Drawing.Size(45, 13);
            this.AddFeedNameLabel.TabIndex = 8;
            this.AddFeedNameLabel.Text = "Name:";
            // 
            // AddFeedName
            // 
            this.AddFeedName.Location = new System.Drawing.Point(57, 35);
            this.AddFeedName.Name = "AddFeedName";
            this.AddFeedName.Size = new System.Drawing.Size(290, 21);
            this.AddFeedName.TabIndex = 7;
            // 
            // DeleteFeed
            // 
            this.DeleteFeed.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.DeleteFeed.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.DeleteFeed.Location = new System.Drawing.Point(180, 256);
            this.DeleteFeed.Name = "DeleteFeed";
            this.DeleteFeed.Size = new System.Drawing.Size(127, 23);
            this.DeleteFeed.TabIndex = 6;
            this.DeleteFeed.Text = "Delete Feed";
            this.DeleteFeed.UseVisualStyleBackColor = false;
            this.DeleteFeed.Visible = false;
            this.DeleteFeed.Click += new System.EventHandler(this.DeleteFeed_Click);
            // 
            // WebBrowser
            // 
            this.WebBrowser.Location = new System.Drawing.Point(9, 54);
            this.WebBrowser.MinimumSize = new System.Drawing.Size(20, 20);
            this.WebBrowser.Name = "WebBrowser";
            this.WebBrowser.Size = new System.Drawing.Size(338, 118);
            this.WebBrowser.TabIndex = 2;
            this.WebBrowser.Url = new System.Uri("", System.UriKind.Relative);
            this.WebBrowser.Visible = false;
            // 
            // FeedType
            // 
            this.FeedType.AutoSize = true;
            this.FeedType.Location = new System.Drawing.Point(6, 277);
            this.FeedType.Name = "FeedType";
            this.FeedType.Size = new System.Drawing.Size(96, 13);
            this.FeedType.TabIndex = 5;
            this.FeedType.Text = "Feed type: RSS";
            this.FeedType.Visible = false;
            // 
            // Description
            // 
            this.Description.AutoSize = true;
            this.Description.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Description.Location = new System.Drawing.Point(6, 38);
            this.Description.Name = "Description";
            this.Description.Size = new System.Drawing.Size(76, 13);
            this.Description.TabIndex = 4;
            this.Description.Text = "Description:";
            this.Description.Visible = false;
            // 
            // LinkAnchor
            // 
            this.LinkAnchor.AutoSize = true;
            this.LinkAnchor.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LinkAnchor.Location = new System.Drawing.Point(54, 204);
            this.LinkAnchor.MaximumSize = new System.Drawing.Size(250, 40);
            this.LinkAnchor.Name = "LinkAnchor";
            this.LinkAnchor.Size = new System.Drawing.Size(153, 13);
            this.LinkAnchor.TabIndex = 3;
            this.LinkAnchor.TabStop = true;
            this.LinkAnchor.Text = "http://www.topcoder.com";
            this.LinkAnchor.Visible = false;
            this.LinkAnchor.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.LinkAnchor_LinkClicked);
            // 
            // Link
            // 
            this.Link.AutoSize = true;
            this.Link.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Link.Location = new System.Drawing.Point(6, 204);
            this.Link.Name = "Link";
            this.Link.Size = new System.Drawing.Size(35, 13);
            this.Link.TabIndex = 2;
            this.Link.Text = "Link:";
            this.Link.Visible = false;
            // 
            // Filter
            // 
            this.Filter.FormattingEnabled = true;
            this.Filter.Items.AddRange(new object[] {
            "All",
            "From last hour",
            "From last 12 hours",
            "From today",
            "From yesterday",
            "From last week",
            "From last month"});
            this.Filter.Location = new System.Drawing.Point(138, 17);
            this.Filter.Name = "Filter";
            this.Filter.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.Filter.Size = new System.Drawing.Size(121, 21);
            this.Filter.TabIndex = 2;
            this.Filter.Text = "All";
            this.Filter.SelectedIndexChanged += new System.EventHandler(this.Filter_SelectedIndexChanged);
            // 
            // FilterLabel
            // 
            this.FilterLabel.AutoSize = true;
            this.FilterLabel.Location = new System.Drawing.Point(21, 21);
            this.FilterLabel.Name = "FilterLabel";
            this.FilterLabel.Size = new System.Drawing.Size(97, 13);
            this.FilterLabel.TabIndex = 3;
            this.FilterLabel.Text = "Filter items by date:";
            // 
            // SearchLabel
            // 
            this.SearchLabel.AutoSize = true;
            this.SearchLabel.Location = new System.Drawing.Point(276, 21);
            this.SearchLabel.Name = "SearchLabel";
            this.SearchLabel.Size = new System.Drawing.Size(59, 13);
            this.SearchLabel.TabIndex = 4;
            this.SearchLabel.Text = "Search for:";
            // 
            // Search
            // 
            this.Search.Location = new System.Drawing.Point(341, 17);
            this.Search.Name = "Search";
            this.Search.Size = new System.Drawing.Size(140, 20);
            this.Search.TabIndex = 5;
            this.Search.TextChanged += new System.EventHandler(this.Search_TextChanged);
            // 
            // ExportFile
            // 
            this.ExportFile.CreatePrompt = true;
            this.ExportFile.Filter = "Text files|*.txt";
            // 
            // ImportFile
            // 
            this.ImportFile.Filter = "Text files|*.txt";
            // 
            // EditNotifyList
            // 
            this.EditNotifyList.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.EditNotifyList.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.EditNotifyList.Location = new System.Drawing.Point(12, 349);
            this.EditNotifyList.Name = "EditNotifyList";
            this.EditNotifyList.Size = new System.Drawing.Size(128, 23);
            this.EditNotifyList.TabIndex = 27;
            this.EditNotifyList.Text = "Edit notify list";
            this.EditNotifyList.UseVisualStyleBackColor = false;
            this.EditNotifyList.Click += new System.EventHandler(this.EditNotifyList_Click);
            // 
            // FeedForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(827, 445);
            this.Controls.Add(this.Search);
            this.Controls.Add(this.SearchLabel);
            this.Controls.Add(this.FilterLabel);
            this.Controls.Add(this.Filter);
            this.Controls.Add(this.Panel);
            this.Controls.Add(this.TreeView);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(835, 800);
            this.MinimumSize = new System.Drawing.Size(835, 479);
            this.Name = "FeedForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Feeds";
            this.Load += new System.EventHandler(this.FeedForm_Load);
            this.Panel.ResumeLayout(false);
            this.Panel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

      }
        #endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new FeedForm());
        }

        private void LinkAnchor_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            // open a browser with this link
            System.Diagnostics.Process.Start(LinkAnchor.Text);
        }

        private void TreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (DoNotTriggerAfterSelect) // make sure we don't enter a never ending loop (UpdateTreeView -> UpdateTreeView)
            {
                DoNotTriggerAfterSelect = false;
                return;
            }

            // hide the notify list form if visible
            if (NotifyForm != null && NotifyForm.Visible)
            {
                NotifyForm.Hide();
            }

            try
            {
                string[] path = e.Node.FullPath.Split(new string[] { TreeView.PathSeparator }, StringSplitOptions.None);
                SelectedFeed = null;

                if (path.Length == 1) // Root
                {
                    Panel.Text = "Feeds";
                    WebBrowser.Visible = false;
                    FeedType.Visible = false;
                    LinkAnchor.Visible = false;
                    Link.Visible = false;
                    Description.Visible = false;
                    DeleteFeed.Visible = false;
                    LastUpdate.Visible = false;
                    DetachBrowser.Visible = false;
                    DateRetrieved.Visible = false;

                    AddFeedError.Visible = false;
                    AddFeed.Visible = true;
                    AddFeedLink.Visible = true;
                    AddFeedLinkLabel.Visible = true;
                    AddFeedName.Visible = true;
                    AddFeedNameLabel.Visible = true;

                    Export.Visible = true;
                    ExportLabel.Visible = true;
                    ExportBox.Visible = true;
                    PickExportFile.Visible = true;
                    Import.Visible = true;
                    ImportLabel.Visible = true;
                    ImportBox.Visible = true;
                    PickImportFile.Visible = true;

                    ExportProgress.Visible = false;
                    ImportProgress.Visible = false;
                    this.UpdateStatus.Visible = false;
                    EditNotifyList.Visible = false;
                }
                else if (path.Length == 2)
                { // feed
                    Panel.Text = path[1];
                    SelectedFeed = path[1];

                    WebBrowser.Visible = true;
                    FeedType.Visible = true;
                    LinkAnchor.Visible = true;
                    Link.Visible = true;
                    Description.Visible = true;
                    DeleteFeed.Visible = true;
                    LastUpdate.Visible = true;
                    DetachBrowser.Visible = true;
                    DateRetrieved.Visible = false;

                    AddFeedError.Visible = false;
                    AddFeed.Visible = false;
                    AddFeedLink.Visible = false;
                    AddFeedLinkLabel.Visible = false;
                    AddFeedName.Visible = false;
                    AddFeedNameLabel.Visible = false;

                    Export.Visible = false;
                    ExportLabel.Visible = false;
                    ExportBox.Visible = false;
                    PickExportFile.Visible = false;
                    Import.Visible = false;
                    ImportLabel.Visible = false;
                    ImportBox.Visible = false;
                    PickImportFile.Visible = false;

                    ExportProgress.Visible = false;
                    ImportProgress.Visible = false;

                    string type = null;
                    string description = null;
                    string link = null;

                    // not in the db, then it must be in the NewFeeds
                    if (Db.FeedData.ContainsKey(path[1]))
                    {
                        type = Db.FeedData[path[1]].Type;
                        description = Db.FeedData[path[1]].Description;
                        link = Db.FeedData[path[1]].Link;
                        LastUpdate.Text = "Last update: " + Db.FeedData[path[1]].LastUpdate;
                        this.UpdateStatus.Checked = Db.FeedData[path[1]].UpdateStatus;
                    }
                    else
                    {
                        type = Db.NewFeeds[path[1]].Type;
                        description = Db.NewFeeds[path[1]].Description;
                        link = Db.NewFeeds[path[1]].Link;
                        LastUpdate.Text = "Last update: Never";
                        this.UpdateStatus.Checked = Db.NewFeeds[path[1]].UpdateStatus;
                    }
                    this.UpdateStatus.Visible = true;
                    EditNotifyList.Visible = true;

                    // create a writer and open the file
                    TextWriter tw = new StreamWriter(new FileStream (FeedReader.PATH_TO_DESCRIPTION,
                        FileMode.Create),
                        Encoding.UTF8);
                    // write a line of text to the file
                    tw.WriteLine("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>" + Helper.DecodeSpecialChars(description) + "</body></html>");
                    // close the stream
                    tw.Close();

                    WebBrowser.Navigate("file:///" + Helper.EncodeURI(FeedReader.PATH_TO_DESCRIPTION));

                    FeedType.Text = "Feed type: " + type;
                    FeedType.Visible = true;
                    LinkAnchor.Text = link;
                }
                else if (path.Length == 3)
                {
                    Panel.Text = path[2];
                    Db.Feeds[path[1]][path[2]].IsRead = true;
                    UpdateTreeView(Filter.Text, Search.Text);

                    // mark the item as read in the registry too
                    RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + path[1] + "\\Items\\" + path[2], true);
                    key.SetValue("IsRead", "1");

                    DateRetrieved.Text = String.Format("Date retrieved: {0:" + FeedReader.DATE_FORMAT + "}", Db.Feeds[path[1]][path[2]].Date);

                    WebBrowser.Visible = true;
                    FeedType.Visible = false;
                    LinkAnchor.Visible = true;
                    Link.Visible = true;
                    Description.Visible = true;
                    DeleteFeed.Visible = false;
                    LastUpdate.Visible = false;
                    DetachBrowser.Visible = true;
                    DateRetrieved.Visible = true;

                    AddFeedError.Visible = false;
                    AddFeed.Visible = false;
                    AddFeedLink.Visible = false;
                    AddFeedLinkLabel.Visible = false;
                    AddFeedName.Visible = false;
                    AddFeedNameLabel.Visible = false;

                    Export.Visible = false;
                    ExportLabel.Visible = false;
                    ExportBox.Visible = false;
                    PickExportFile.Visible = false;
                    Import.Visible = false;
                    ImportLabel.Visible = false;
                    ImportBox.Visible = false;
                    PickImportFile.Visible = false;

                    ExportProgress.Visible = false;
                    ImportProgress.Visible = false;
                    this.UpdateStatus.Visible = false;
                    EditNotifyList.Visible = false;

                    // create a writer and open the file
                    TextWriter tw = new StreamWriter(new FileStream(FeedReader.PATH_TO_DESCRIPTION,
                        FileMode.Create),
                        Encoding.UTF8);
                    // write a line of text to the file
                    tw.WriteLine("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>" + Helper.DecodeSpecialChars(Db.Feeds[path[1]][path[2]].Description) + "</body></html>");
                    // close the stream
                    tw.Close();

                    WebBrowser.Navigate("file:///" + Helper.EncodeURI(FeedReader.PATH_TO_DESCRIPTION));

                    LinkAnchor.Text = Db.Feeds[path[1]][path[2]].Link;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("An error was encountered while processing this operation: " + ex.Message + ex.StackTrace);
            }
        }

        private void AddFeed_Click(object sender, EventArgs e)
        {
            try
            {
                if (Clicked)
                {
                    return;
                }

                Clicked = true;

                AddFeedError.Visible = true;
                AddFeedError.Text = "Checking in progress, please hold...";

                AddFeedName.Text = AddFeedName.Text.Trim();
                AddFeedLink.Text = AddFeedLink.Text.Trim();

                // check validity of arguments and uniqueness of link/name
                Feed feed = null;
                try
                {
                    feed = new Feed(AddFeedName.Text, "None specified", AddFeedLink.Text, "NONE");
                }
                catch (Exception ex)
                {
                    AddFeedError.Text = ex.Message;
                }

                if (Db.FeedData.ContainsKey(AddFeedName.Text) && !Db.ToDelete.Contains(AddFeedName.Text))
                {
                    AddFeedError.Text = "Feed name must be unique.";
                }

                if (Helper.LinkExists(AddFeedLink.Text, Db.FeedData, true) ||
                    Helper.LinkExists(AddFeedLink.Text, Db.NewFeeds, false))
                {
                    AddFeedError.Text = "Feed link must be unique.";
                }

                if (AddFeedError.Text.Equals("Checking in progress, please hold..."))
                {
                    string type = feed.Type;

                    // check for proxy support
                    RegistryKey settingsKey = Registry.LocalMachine.OpenSubKey(
                        FeedReader.PluginKey + "\\Settings");
                    bool useProxy = (settingsKey.GetValue("UseProxy").ToString().Equals("1") ? true : false);
                    string proxyHost = null;
                    int proxyPort = 0;
                    if (useProxy)
                    {
                        proxyHost = settingsKey.GetValue("ProxyHost").ToString();
                        proxyPort = Int32.Parse(settingsKey.GetValue("ProxyPort").ToString());
                    }

                    if (!Helper.IsGoodFeed(feed, Db.FeedData, Db.NewFeeds, out type, useProxy, proxyHost, proxyPort))
                    {
                        AddFeedError.Text = "Feed wasn't in a supported format (RSS 2.0 or ATOM 1.0).";
                        AddFeedError.Visible = true;
                    }
                    else
                    {
                        AddFeedError.Text = "Feed successfully added.";

                        feed.Type = type;

                        // passed checks, add feed
                        AddFeedLink.Text = "";
                        AddFeedName.Text = "";

                        // update registry
                        Helper.CreateFeedKey(feed);

                        // check if update is in progress, if it is add the feed to a temp list
                        if (FeedReader.UpdatingFeed != null)
                        {
                            Db.NewFeeds.Add(feed.Name, feed);
                        }
                        else
                        {
                            Db.FeedData.Add(feed.Name,
                                new Feed(feed.Name, feed.Description, feed.Link, feed.Type));
                            Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
                        }

                        // add the node too
                        UpdateTreeView(Filter.Text, Search.Text);
                    }
                }
            }
            catch (Exception ex)
            {
                AddFeedError.Visible = false;
                MessageBox.Show("An error was encountered while processing this operation: " + ex.Message + ex.StackTrace);
            }
            finally
            {
                // clean up
                Clicked = false;
            }
        }

        private void DeleteFeed_Click(object sender, EventArgs e)
        {
            try
            {
                if (Clicked)
                {
                    return;
                }

                Clicked = true;

                // delete from registry too
                RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds", true);
                key.DeleteSubKeyTree(SelectedFeed);

                string nodeName = SelectedFeed.ToString();

                if (SelectedFeed != null)
                {
                    // hide the notify list form if visible and it has the current feed open
                    if (NotifyForm != null && NotifyForm.Visible && NotifyForm.EditingFeed.Equals(SelectedFeed))
                    {
                        NotifyForm.Hide();
                    }

                    if (FeedReader.UpdatingFeed != null)
                    {
                        // delete it from new feeds if it was added during the updating process
                        if (Db.NewFeeds.ContainsKey(SelectedFeed))
                        {
                            Db.NewFeeds.Remove(SelectedFeed);
                        }
                        else
                        {
                            Db.ToDelete.Add(SelectedFeed);
                        }
                    }
                    else
                    {
                        Db.Feeds.Remove(SelectedFeed);
                        Db.FeedData.Remove(SelectedFeed);
                    }
                }

                // the root node will be selected
                TreeView.SelectedNode = TreeView.Nodes[0];

                // delete the node too
                UpdateTreeView(Filter.Text, Search.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show("An error was encountered while processing this operation: " + ex.Message + ex.StackTrace);
            }
            finally
            {
                // clean up
                Clicked = false;
            }
        }

        private void FeedForm_Load(object sender, EventArgs e)
        {
            TreeView.PathSeparator = ((char)1).ToString() + ((char)3).ToString();
        }

        private void DetachBrowser_Click(object sender, EventArgs e)
        {
            if (browserForm != null && browserForm.Visible)
            {
                // close the previous window
                browserForm.Hide();
            }

            // open a new form containing a bigger browser window
            if (browserForm == null || !browserForm.Visible)
            {
                browserForm = new BrowserForm();
                browserForm.Text = Panel.Text;
            }

            browserForm.WebBrowser.Navigate("file:///" + Helper.EncodeURI(FeedReader.PATH_TO_DESCRIPTION));
            browserForm.Show();
        }

        private bool IsInDateRange(string filter, FeedItem item)
        {
            bool isGood = false;
            if (filter.Equals("All")) // all
            {
                isGood = true;
            }
            else if (filter.Equals("From last hour")) // from last hour
            {
                DateTime threshold = DateTime.Now;
                threshold = threshold.AddHours(-1);
                isGood = (item.Date.CompareTo(threshold) >= 0);
            }
            else if (filter.Equals("From last 12 hours")) // from last 12 hours
            {
                DateTime threshold = DateTime.Now;
                threshold = threshold.AddHours(-12);
                isGood = (item.Date.CompareTo(threshold) >= 0);
            }
            else if (filter.Equals("From today")) // from today
            {
                DateTime threshold = DateTime.Now;
                isGood = (threshold.DayOfYear == item.Date.DayOfYear &&
                    threshold.Year == item.Date.Year);
            }
            else if (filter.Equals("From yesterday")) // from yesterday
            {
                DateTime threshold = DateTime.Now;
                threshold = threshold.AddDays(-1);
                isGood = (item.Date.CompareTo(threshold) >= 0);
            }
            else if (filter.Equals("From last week")) // from last week
            {
                DateTime threshold = DateTime.Now;
                threshold = threshold.AddDays(-7);
                isGood = (item.Date.CompareTo(threshold) >= 0);
            }
            else if (filter.Equals("From last month")) // from last month
            {
                DateTime threshold = DateTime.Now;
                threshold = threshold.AddMonths(-1);
                isGood = (item.Date.CompareTo(threshold) >= 0);
            }

            return isGood;
        }

        public void UpdateTreeView(string filter, string search)
        {
            UpdateTreeView(filter, search, true);
        }

        public void UpdateTreeView(string filter, string search, bool keepSelection)
        {
            DoNotTriggerAfterSelect = false;
            // first save the previous state - i.e. collapsed nodes
            bool rootExpanded = TreeView.Nodes[0].IsExpanded;
            IDictionary<string, bool> feedsExpanded = new Dictionary<string, bool>();
            foreach (TreeNode node in TreeView.Nodes[0].Nodes)
            {
                feedsExpanded.Add(node.Text, node.IsExpanded);
            }
            string selectedNodePath = (TreeView.SelectedNode != null ? TreeView.SelectedNode.FullPath : null);

            // clear all of the nodes
            TreeView.Nodes.Clear();
            TreeView.Nodes.Add("Feeds"); // add root node
            bool isRootNodeRead = true;

            // cycle through all the feed items
            foreach (KeyValuePair<string, IDictionary<string, FeedItem>> kvp in Db.Feeds)
            {
                // make sure this feed wasn't scheduled to be deleted
                if (!Db.ToDelete.Contains(kvp.Key))
                {
                    IList<FeedItem> items = new List<FeedItem>();
                    foreach (KeyValuePair<string, FeedItem> fikvp in kvp.Value)
                    {
                        bool isGood = true;

                        // search is empty
                        if (search.Trim().Length == 0)
                        {
                            isGood = IsInDateRange(filter, fikvp.Value);
                        }
                        // search is not empty
                        else
                        {
                            isGood = IsInDateRange(filter, fikvp.Value);
                            isGood = isGood && (fikvp.Value.Name.ToLower().IndexOf(search.ToLower()) != -1 ||
                                fikvp.Value.Description.ToLower().IndexOf(search.ToLower()) != -1);
                        }

                        // if it was good, add it to the treeview
                        if (isGood)
                        {
                            items.Add(fikvp.Value);
                        }
                    }

                    TreeNode node = TreeView.Nodes[0].Nodes.Add(kvp.Key);

                    bool isNodeRead = true;
                    if (items.Count > 0)
                    {
                        foreach (FeedItem item in items)
                        {
                            TreeNode subNode = node.Nodes.Add(item.Name);
                            // update feed item node read status
                            subNode.ImageIndex = (item.IsRead ? FeedReader.IMAGE_INDEX_READ :
                                FeedReader.IMAGE_INDEX_UNREAD);
                            subNode.SelectedImageIndex = (item.IsRead ? FeedReader.IMAGE_INDEX_READ :
                                FeedReader.IMAGE_INDEX_UNREAD);

                            if (!item.IsRead)
                            {
                                isNodeRead = false;
                                isRootNodeRead = false;
                            }
                        }
                    }

                    // update feed node read status
                    node.ImageIndex = (isNodeRead ? FeedReader.IMAGE_INDEX_READ :
                        FeedReader.IMAGE_INDEX_UNREAD);
                    node.SelectedImageIndex = (isNodeRead ? FeedReader.IMAGE_INDEX_READ :
                        FeedReader.IMAGE_INDEX_UNREAD);
                    if (feedsExpanded.ContainsKey(kvp.Key) && feedsExpanded[kvp.Key])
                    {
                        node.Expand();
                    }
                }
            }

            // new feed nodes are by default read
            foreach (KeyValuePair<string, Feed> kvp in Db.NewFeeds)
            {
                TreeNode node = TreeView.Nodes[0].Nodes.Add(kvp.Key);
                // update feed node read status
                node.ImageIndex = FeedReader.IMAGE_INDEX_READ;
                node.SelectedImageIndex = FeedReader.IMAGE_INDEX_READ;
            }

            // update root node read status
            TreeView.Nodes[0].ImageIndex = (isRootNodeRead ? FeedReader.IMAGE_INDEX_READ :
                FeedReader.IMAGE_INDEX_UNREAD);
            TreeView.Nodes[0].SelectedImageIndex = (isRootNodeRead ? FeedReader.IMAGE_INDEX_READ :
                FeedReader.IMAGE_INDEX_UNREAD);
            if (rootExpanded)
            {
                TreeView.Nodes[0].Expand();
            }

            if (keepSelection && selectedNodePath != null && selectedNodePath.Trim().Length > 0)
            {
                string[] path = selectedNodePath.Split(new string[] { TreeView.PathSeparator }, StringSplitOptions.None);
                if (path.Length == 1) // root node was selected
                {
                    DoNotTriggerAfterSelect = true;
                    TreeView.SelectedNode = TreeView.Nodes[0];
                }
                else if (path.Length == 2) // a feed node was selected
                {
                    DoNotTriggerAfterSelect = true;
                    TreeView.SelectedNode = Helper.GetNodeByText(TreeView.Nodes[0], path[1]);
                }
                else if (path.Length == 3) // a feed item was selected
                {
                    TreeNode feedNode = Helper.GetNodeByText(TreeView.Nodes[0], path[1]);
                    DoNotTriggerAfterSelect = true;
                    TreeView.SelectedNode = Helper.GetNodeByText(feedNode, path[2]);
                }
            }
        }

        private void Filter_SelectedIndexChanged(object sender, EventArgs e)
        {
            // update the treeview to display the appropriate items based on the filters
            UpdateTreeView(Filter.Text, Search.Text, false);
        }

        private void Search_TextChanged(object sender, EventArgs e)
        {
            // update the treeview to display the appropriate items based on the filters
            UpdateTreeView(Filter.Text, Search.Text, false);
        }

        private void PickExportFile_Click(object sender, EventArgs e)
        {
            ExportFile.InitialDirectory = Helper.ParseDirectory(ExportBox.Text);
            // pick a file
            if (ExportFile.ShowDialog() == DialogResult.OK)
            {
                ExportBox.Text = ExportFile.FileName;
            }
        }

        private void PickImportFile_Click(object sender, EventArgs e)
        {
            ImportFile.InitialDirectory = Helper.ParseDirectory(ExportBox.Text);
            // pick a file
            if (ImportFile.ShowDialog() == DialogResult.OK)
            {
                ImportBox.Text = ImportFile.FileName;
            }
        }

        private void Export_Click(object sender, EventArgs e)
        {
            if (Exporting || ExportBox.Text.Trim().Length == 0)
            {
                return;
            }
            Exporting = true;
            ExportProgress.Visible = true;

            try
            {
                string outFile = ExportBox.Text;
                TextWriter tw = new StreamWriter(new FileStream(outFile,
                    FileMode.Create),
                    Encoding.UTF8);
                foreach (KeyValuePair<string, Feed> kvp in Db.FeedData)
                {
                    // write a line of text to the file
                    if (!Db.ToDelete.Contains(kvp.Key))
                    {
                        tw.WriteLine(kvp.Value.Link);
                    }
                }

                foreach (KeyValuePair<string, Feed> kvp in Db.NewFeeds)
                {
                    // write a line of text to the file
                    tw.WriteLine(kvp.Value.Link);
                }
                // close the stream
                tw.Close();

                MessageBox.Show("Feed list successfully exported to " + outFile);
                ExportBox.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error occurred while writing to file: " + ex.Message + ex.StackTrace);
            }
            finally
            {
                Exporting = false;
                ExportProgress.Visible = false;
            }
        }

        private void Import_Click(object sender, EventArgs e)
        {
            if (Importing || ImportBox.Text.Trim().Length == 0)
            {
                return;
            }
            Importing = true;
            ImportProgress.Visible = true;

            try
            {
                string inFile = ImportBox.Text;
                TextReader tr = new StreamReader(new FileStream(inFile,
                    FileMode.Open),
                    Encoding.UTF8);

                string line = null;

                // check for proxy support
                RegistryKey settingsKey = Registry.LocalMachine.OpenSubKey(
                    FeedReader.PluginKey + "\\Settings");
                bool useProxy = (settingsKey.GetValue("UseProxy").ToString().Equals("1") ? true : false);
                string proxyHost = null;
                int proxyPort = 0;
                if (useProxy)
                {
                    proxyHost = settingsKey.GetValue("ProxyHost").ToString();
                    proxyPort = Int32.Parse(settingsKey.GetValue("ProxyPort").ToString());
                }
                bool anyGood = false;

                while ((line = tr.ReadLine()) != null)
                {
                    string link = line.Trim();
                    // we assume one link / line

                    Feed feed = null;
                    try
                    {
                        feed = new Feed(
                                link,
                                "None specified",
                                link,
                                "NONE");
                    }
                    catch (Exception)
                    {
                        // feed invalid, skip
                        continue;
                    }

                    // the link will not be added if a feed with that name/link already exists
                    // so check this first
                    // also check if the link is indeed a feed
                    string type = feed.Type;

                    if (!Helper.IsGoodFeed(feed, Db.FeedData, Db.NewFeeds, out type, useProxy, proxyHost, proxyPort))
                    {
                        continue;
                    }

                    anyGood = true;
                    feed.Type = type;

                    // update registry
                    Helper.CreateFeedKey(feed);

                    // check if update is in progress, if it is add the feed to a temp list
                    if (FeedReader.UpdatingFeed != null)
                    {
                        Db.NewFeeds.Add(feed.Name, feed);
                    }
                    else
                    {
                        Db.FeedData.Add(feed.Name,
                            new Feed(feed.Name, feed.Description, feed.Link, feed.Type));
                        Db.Feeds.Add(feed.Name, new Dictionary<string, FeedItem>());
                    }
                }
                // close the stream
                tr.Close();

                // add the node too
                if (anyGood && this.Visible)
                {
                    UpdateTreeView(Filter.Text, Search.Text);
                }

                MessageBox.Show("Feed list successfully imported from " + inFile);
                ImportBox.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error occurred while reading from file: " + ex.Message + ex.StackTrace);
            }
            finally
            {
                Importing = false;
                ImportProgress.Visible = false;
            }
        }

        private void UpdateStatus_CheckedChanged(object sender, EventArgs e)
        {
            // not in the db, then it must be in the NewFeeds
            if (Db.FeedData.ContainsKey(Panel.Text))
            {
                Db.FeedData[Panel.Text].UpdateStatus = UpdateStatus.Checked;
            }
            else
            {
                Db.NewFeeds[Panel.Text].UpdateStatus = UpdateStatus.Checked;
            }

            // update registry too
            RegistryKey key = Registry.LocalMachine.OpenSubKey(FeedReader.PluginKey + "\\Feeds\\" + Panel.Text, true);
            key.SetValue("UpdateStatus", UpdateStatus.Checked ? "1" : "0");
        }

        private void EditNotifyList_Click(object sender, EventArgs e)
        {
            if (NotifyForm != null && NotifyForm.Visible)
            {
                // close the previous window
                NotifyForm.Hide();
            }

            // open a new form containing a bigger browser window
            if (NotifyForm == null || !NotifyForm.Visible)
            {
                NotifyForm = new NotifyListForm();
                NotifyForm.EditingFeed = Panel.Text;

                // get the existing notify list
                // first check if this feed is newly added
                if (Db.FeedData.ContainsKey(Panel.Text))
                {
                    NotifyForm.NotifyList = Db.FeedData[Panel.Text].NotifyList;
                    NotifyForm.NotifyEnabled.Checked = Db.FeedData[Panel.Text].NotifyEnabled;
                }
                else
                { // newly added
                    NotifyForm.NotifyList = Db.NewFeeds[Panel.Text].NotifyList;
                    NotifyForm.NotifyEnabled.Checked = Db.NewFeeds[Panel.Text].NotifyEnabled;
                }

                // iterate through the groups
                for (int i = 0; i < FeedReader.m_session.BuddyList.GroupCount; i++)
                {
                    IAccGroup grp = FeedReader.m_session.BuddyList.GetGroupByIndex(i);
                    // for each group iterate through its users
                    for (int j = 0; j < grp.BuddyCount; j++)
                    {
                        IAccUser user = grp.GetBuddyByIndex(j);
                        if (!NotifyForm.NotifyList.Contains(user.Name))
                        {
                            NotifyForm.AvailableBuddies.Items.Add(user.Name);
                        }
                    }
                }

                // now add the notify list buddies
                foreach (string item in NotifyForm.NotifyList)
                {
                    NotifyForm.NotifyBuddies.Items.Add(item);
                }

                NotifyForm.Text = "Editing notify list for [" + Panel.Text + "]";
            }

            NotifyForm.Show();
        }
    }
}
