using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading.Tasks;

namespace WifiScreenConnector
{
    public class ConfigurationsWatcher
    {
        public event EventHandler<ConfigChangedEventArgs> ConfigChanged;

        protected ConfigInfo config, newConfig;

        private Timer timer;

        public ConfigurationsWatcher()
        {
            config = Utils.ReadConfigFromRegistry();
            timer = new Timer();
            timer.Tick += Timer_Tick;
            timer.Interval = 2000;
            timer.Start();

            
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            newConfig = Utils.ReadConfigFromRegistry();
            if (config.IsDiff(newConfig) && ConfigChanged != null)
            {
                bool needRestart = (config.Port != newConfig.Port);
                config = newConfig;
                ConfigChanged(this, new ConfigChangedEventArgs() { NewConfig = newConfig, NeedRestart = needRestart });
            }
        }

        public class ConfigChangedEventArgs : EventArgs
        {
            public ConfigInfo NewConfig { get; set; }
            public bool NeedRestart { get; set; }
        }

    }


}
