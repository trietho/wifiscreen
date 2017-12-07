using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WifiScreenConnector
{
    public partial class MainForm : Form
    {
        public WifiScreenWrapper wifiScreen;
        private bool serverRunning = false;
        //private ShareType shareType;
        private Collection<ComboBoxItem> Shareables;
        private ConfigurationsWatcher configWatcher;
        private ConfigInfo configures;

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            configWatcher = new ConfigurationsWatcher();
            configWatcher.ConfigChanged += OnConfigChanged;
            //shareType = ShareType.Screen;
            loadScreensList();

            wifiScreen = new WifiScreenWrapper();
            comboScreens.DisplayMember = "Text";
            comboScreens.ValueMember = "Value";

            configures = Utils.ReadConfigFromRegistry();
            showConfigs();

            StartServer();
        }

        public void OnConfigChanged(object sender, ConfigurationsWatcher.ConfigChangedEventArgs configChangedEventArgs)
        {
            configures = configChangedEventArgs.NewConfig;
            showConfigs();
            if (configChangedEventArgs.NeedRestart)
            {
                wifiScreen.Restart();
            }
            else
            {
                wifiScreen.Reload();
            }
            wifiScreen.ShareDisplay((string)comboScreens.SelectedValue);
        }

        private void showConfigs()
        {
            labelIP.Text = Utils.getIPString(configures.Port);
            labelMultiConnection.Text = (configures.IsShare) ? "YES" : "NO";
            labelRequirePass.Text = (configures.AuthenRequired) ? "YES" : "NO";
            labelRemoteInput.Text = (configures.BlockRemoteInput) ? "NO" : "YES";
        }


        private void loadScreensList()
        {
            //Shareables = Utils.GetShareables(shareType);
            Shareables = Utils.GetShareables(ShareType.Screen);
            comboScreens.DataSource = Shareables;

            if (Shareables.Count > 0)
                comboScreens.SelectedIndex = 0;
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            StartServer();

        }

        private void StartServer()
        {
            if (!serverRunning)
            {
                serverRunning = wifiScreen.Start();
                if (serverRunning)
                {
                    buttonStart.Text = "Stop";
                    labelStatus.Text = "Running";
                    wifiScreen.ShareDisplay((string)comboScreens.SelectedValue);
                }
            }
            else
            {
                if (MessageBox.Show("Are you sure?", "Stop sharing", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {
                    wifiScreen.Stop();
                    buttonStart.Text = "Start";
                    labelStatus.Text = "Stop";
                    serverRunning = false;
                }
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                this.Hide();
                e.Cancel = true;
            }
        }


        private void comboScreens_SelectionChangeCommitted(object sender, EventArgs e)
        {
            string selectedValue = (string)comboScreens.SelectedValue;

            if (selectedValue == "")
            {
                loadScreensList();
            }
            else
            {
                if (serverRunning)
                {
                    wifiScreen.ShareDisplay(selectedValue);
                }

            }


        }

        private void buttonWifiScreenConfig_Click(object sender, EventArgs e)
        {
            wifiScreen.Config();
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Are you sure?", "Exit Application", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {

                wifiScreen.Stop();
                Application.Exit();
            }

        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            this.Show();
            Activate();
        }

        private void buttonDisconnectAll_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Are you sure?", "Disconnect all", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                wifiScreen.DisconnectAll();
            }
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Hide();
        }
    }
}
