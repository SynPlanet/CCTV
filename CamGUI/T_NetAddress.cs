using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Cam
{
    [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet= CharSet.Ansi)]
    struct T_NetAddress
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65)]
        public string Protocol;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65)]
        public string Tile;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65)]
        public string Login;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65)]
        public string Password;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] IP;
        public int Port;
    }
}
