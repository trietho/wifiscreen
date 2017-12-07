using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace WifiScreenConnector
{
    public class WifiScreenWrapper
    {

        private static string wifiScreenProcessName = "WifiScreenServer";
        private string wifiScreenAppPath = "";


        private Process WifiScreenServer
        {
            get
            {
                Process wifiScreen = Utils.GetProcessByName(wifiScreenProcessName);
                if (wifiScreen == null)
                {
                    wifiScreen = new Process();
                    wifiScreen.StartInfo.FileName = wifiScreenAppPath;
                    wifiScreen.StartInfo.WorkingDirectory = Utils.GetExecutingDirectoryName();
                    wifiScreen.Start();
                    Thread.Sleep(1000);
                }
                return wifiScreen;
            }
        }

        public WifiScreenWrapper()
        {
            wifiScreenAppPath = Utils.GetExecutingDirectoryName() + "\\WifiScreenServer.exe";
        }

        public bool Start()
        {
            if (!WifiScreenServer.HasExited)
            {
                return true;
            }
            return false;
        }

        public void Config()
        {
            executeControlArgs("-configapp");
        }

        public void DisconnectAll()
        {
            executeControlArgs("-controlapp -disconnectall");
        }

        public void Stop()
        {
            executeControlArgs("-controlapp -shutdown");
        }

        public bool Restart()
        {
            Stop();
            Thread.Sleep(1000);
            return Start();
        }

        public void ShareDisplay(string displayNumber)
        {
            string arg = (displayNumber == "0") ? "-controlapp -sharefull" : "-controlapp -sharedisplay " + displayNumber;
            executeControlArgs(arg);
        }

        public void ShareWindows(string caption)
        {
            string arg = "-controlapp -sharewindow " + caption;
            executeControlArgs(arg);
        }

        public void ShareApplication(string appId)
        {
            string arg = "-controlapp -shareapp " + appId;
            executeControlArgs(arg);
        }

        public void Reload()
        {
            executeControlArgs("-controlapp -reload");
      
        }

        public static void Shutdown()
        {
            Process p = Utils.GetProcessByName(wifiScreenProcessName);
            if (p!= null)
            {
                p.Kill();
             
            }
        }

        private void executeControlArgs(string arg)
        {
            Process tmpProcess = new Process();
            tmpProcess.StartInfo.FileName = wifiScreenAppPath;
            tmpProcess.StartInfo.Arguments = arg;
            tmpProcess.Start();
        }

    }
}
