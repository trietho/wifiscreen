using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WifiScreenConnector
{
    public class ConfigInfo
    {
      

        public int Port { get; set; }
        public bool AuthenRequired { get; set; }
        public bool TransferWallpaper { get; set; }
        public bool BlockRemoteInput { get; set; }
        public bool IsShare { get; set; }
        //public bool PasswordExisted { get; set; }

        public bool IsDiff(ConfigInfo dest)
        {
            return (this.Port != dest.Port ||
                this.AuthenRequired != dest.AuthenRequired ||
                this.IsShare != dest.IsShare ||
                this.TransferWallpaper != dest.TransferWallpaper ||
                this.BlockRemoteInput != dest.BlockRemoteInput);
        }
    }
}
