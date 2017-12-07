using Microsoft.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WifiScreenConnector
{
    public enum ShareType
    {
        Screen,
        Windows,
        Application
    }

    public static class Utils
    {
        public const string WIFISCREEN_REG_ROOT = "Software\\WifiScreen\\Server\\";

        public static Collection<ComboBoxItem> GetShareables(ShareType sharetype)
        {
            switch (sharetype)
            {
                case ShareType.Screen:
                    return GetScreens();
                case ShareType.Windows:
                    return GetDesktopApplications();
                default:
                    return new Collection<ComboBoxItem>();
            }
        }

        public static Collection<ComboBoxItem> GetScreens()
        {
            Collection<ComboBoxItem> ret = new Collection<ComboBoxItem>();
            ret.Add(new ComboBoxItem() { Text = "All Screens", Value = "0" });
            for (int i = 0; i < Screen.AllScreens.Length; i++)
            {
                ret.Add(new ComboBoxItem() { Text = (i + 1) + "" + ((Screen.AllScreens[i].Primary) ? " - Primary" : ""), Value = (i + 1).ToString() });
            }
            ret.Add(new ComboBoxItem() { Text = "Refresh this list", Value = "" });
            return ret;
        }

        public static Collection<ComboBoxItem> GetDesktopApplications()
        {
            Process[] runningProcs = Process.GetProcesses();
            Collection<ComboBoxItem> ret = new Collection<ComboBoxItem>();

            foreach (Process p in runningProcs)
            {
                if (p.MainWindowTitle != "")
                    ret.Add(new ComboBoxItem() { Text = p.ProcessName + "-" + p.Id + " - " + p.MainWindowTitle, Value = p.Id.ToString() });
            }
            ret.Add(new ComboBoxItem() { Text = "Refresh this list", Value = "" });
            return ret;
        }


        public static string GetExecutingDirectoryName()
        {
            return new FileInfo(Application.ExecutablePath).Directory.FullName;
        }

        public static bool IsProcessRunning(string name)
        {
            Process[] runningProcs = Process.GetProcessesByName(name);
            return (runningProcs.Length > 0);
        }

        public static Process GetProcessByName(string name)
        {
            Process[] runningProcs = Process.GetProcessesByName(name);
            return runningProcs.FirstOrDefault<Process>(p => p.ProcessName == name);
        }

        public static string getIPString(int port)
        {
            IPAddress[] ips = Dns.GetHostAddresses(Dns.GetHostName());
            string ret = "";
            foreach (IPAddress ip in ips)
            {
                if (ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                    ret += ip.ToString() + ":" + port + ", ";
            }
            return ret.TrimEnd(", ".ToCharArray());
        }

        public static ConfigInfo ReadConfigFromRegistry()
        {
            ConfigInfo ret = new ConfigInfo();
            RegistryKey key = Registry.CurrentUser.OpenSubKey(WIFISCREEN_REG_ROOT);

            if (key != null)
            {
                ret.AuthenRequired = ((int)key.GetValue("UseVncAuthentication", 0) == 1);
                ret.BlockRemoteInput = ((int)key.GetValue("BlockRemoteInput", 1) == 1);
                ret.Port = (int)key.GetValue("RfbPort", 8989);
                ret.IsShare = ((int)key.GetValue("AlwaysShared", 0) == 1);
                ret.TransferWallpaper = ((int)key.GetValue("RemoveWallpaper", 1) == 0);

                key.Close();
            }

            return ret;
        }

    }
}
