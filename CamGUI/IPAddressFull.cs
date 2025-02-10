using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;

namespace Cam
{
    class IPAddressFull : IPAddress
    {
        public new static IPAddressFull Parse(string str)
        {
            IPAddressFull ip;
            int index = str.IndexOf(":");
            if (index == -1) ip = new IPAddressFull(IPAddress.Parse(str).GetAddressBytes());
            else ip = new IPAddressFull(IPAddress.Parse(str.Substring(0, index)).GetAddressBytes());
            if (index != -1) ip.Port = int.Parse(str.Substring(index + 1));
            return ip;
           
        }

        private IPAddressFull(byte[] adresses)
            : base(adresses) 
        {
        }

        public int Port { get; private set; }
    }
}
