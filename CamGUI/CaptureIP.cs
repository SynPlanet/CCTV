using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Runtime.ExceptionServices;

namespace Cam
{
    delegate void GetTimeMovie([MarshalAs(UnmanagedType.U4)]uint time, uint totalTime, [MarshalAs(UnmanagedType.U4)]PlayerState status);
    delegate void PlayerStatus([MarshalAs(UnmanagedType.U4)]uint id, [MarshalAs(UnmanagedType.U4)]PlayerState status);
    delegate void Joy_IsConnect([MarshalAs(UnmanagedType.U1)]bool hasJoy);
    delegate void Joy_Control([MarshalAs(UnmanagedType.U4)]uint id, [MarshalAs(UnmanagedType.U4)]JoystickButtonState status);
    delegate void SetMovieFolder(IntPtr dirPtr, int size);

    static class CaptureIP
    {
        static bool? IsExist_SetPlayerManualControl = null;
        static bool? IsExist_SetTimeRecMinutes = null;
        static bool? IsExist_SetPort_VideoControl = null;
        static bool? IsExist_SetCurrentCamera = null;
        static bool? IsExist_Initialize_ALL = null;
        static bool? IsExist_Release_ALL = null;
        static bool? IsExist_SetTimePlay = null;
        static bool? IsExist_GetTotalTimePlay = null;
        static bool? IsExist_CreateStreamIP = null;
        static bool? IsExist_CreateStreamFile = null;
        static bool? IsExist_GetBMP_NmbCamera = null;
        static bool? IsExist_ReleaseCameraIP_Nmb = null;
        static bool? IsExist_GetCamera_360 = null;
        static bool? IsExist_SetCamera_360 = null;
        static bool? IsExist_GetParamsCamera = null;
        static bool? IsExist_WriteVideoStream = null;
        static bool? IsExist_Joystick_Control = null;
        static bool? IsExist_CallBackFunc_PlayerState = null;
        static bool? IsExist_CallBackFunc_GetTimePlayer = null;
        static bool? IsExist_CallBackFunc_SetMovieFolder = null;
        static bool? IsExist_CallBackFunc_JoyControl = null;
        static bool? IsExist_SetPathRec = null;
        static bool? IsExist_SetPathsVideo = null;
        static bool? IsExist_SetPathsAudio = null;
        static bool? IsExist_SetAudioDevice = null;
        static bool? IsExist_SetTypeInterface = null;

        public static void SetPlayerManualControl(bool isManual)
        {
            if (IsExist_SetPlayerManualControl == null)
            {
                try
                {
                    CaptureIPApi.SetPlayerManualControl(isManual);
                    IsExist_SetPlayerManualControl = true;
                }
                catch { IsExist_SetPlayerManualControl = false; }
            }
            else
            {
                if (IsExist_SetPlayerManualControl.Value == false) return;
                CaptureIPApi.SetPlayerManualControl(isManual);
            }
        }

        public static bool SetTimeRecMinutes(int minRec)
        {
            if (IsExist_SetTimeRecMinutes == null)
            {
                try
                {
                    bool value = CaptureIPApi.SetTimeRecMinutes(minRec);
                    IsExist_SetTimeRecMinutes = true;
                    return value;
                }
                catch { IsExist_SetTimeRecMinutes = false; }
            }
            else
            {
                if (IsExist_SetTimeRecMinutes.Value) return CaptureIPApi.SetTimeRecMinutes(minRec);
            }
            return false;
        }

        public static bool SetPort_VideoControl(int port)
        {
            if (IsExist_SetPort_VideoControl == null)
            {
                try
                {
                    bool value = CaptureIPApi.SetPort_VideoControl(port);
                    IsExist_SetPort_VideoControl = true;
                    return value;
                }
                catch { IsExist_SetPort_VideoControl = false; }
            }
            else
            {
                if (IsExist_SetPort_VideoControl.Value) return CaptureIPApi.SetPort_VideoControl(port);
            }
            return false;
        }

        public static bool SetCurrentCamera(int nmb_cur_Cmr_in)
        {
            if (IsExist_SetCurrentCamera == null)
            {
                try
                {
                    bool value = CaptureIPApi.SetCurrentCamera(nmb_cur_Cmr_in);
                    IsExist_SetCurrentCamera = true;
                    return value;
                }
                catch { IsExist_SetCurrentCamera = false; }
            }
            else
            {
                if (IsExist_SetCurrentCamera.Value) return CaptureIPApi.SetCurrentCamera(nmb_cur_Cmr_in);
            }
            return false;
        }

        public static int Initialize_ALL(int flag)
        {
            if (IsExist_Initialize_ALL == null)
            {
                try
                {
                    int value = CaptureIPApi.Initialize_ALL(flag);
                    IsExist_Initialize_ALL = true;
                    return value;
                }
                catch { IsExist_Initialize_ALL = false; }
            }
            else
            {
                if (IsExist_Initialize_ALL.Value) return CaptureIPApi.Initialize_ALL(flag);
            }
            return -1;
        }

        public static int Release_ALL()
        {
            if (IsExist_Release_ALL == null)
            {
                try
                {
                    int value = CaptureIPApi.Release_ALL();
                    IsExist_Release_ALL = true;
                    return value;
                }
                catch { IsExist_Release_ALL = false; }
            }
            else
            {
                if (IsExist_Release_ALL.Value) return CaptureIPApi.Release_ALL();
            }
            return -1;
        }

        public static int SetTimePlay(int time)
        {
            if (IsExist_SetTimePlay == null)
            {
                try
                {
                    int value = CaptureIPApi.SetTimePlay(time);
                    IsExist_SetTimePlay = true;
                    return value;
                }
                catch { IsExist_SetTimePlay = false; }
            }
            else
            {
                if (IsExist_SetTimePlay.Value) return CaptureIPApi.SetTimePlay(time);
            }
            return -1;
        }

        public static int GetTotalTimePlay(IntPtr totalTimePtr)
        {
            if (IsExist_GetTotalTimePlay == null)
            {
                try
                {
                    int value = CaptureIPApi.GetTotalTimePlay(totalTimePtr);
                    IsExist_GetTotalTimePlay = true;
                    return value;
                }
                catch { IsExist_GetTotalTimePlay = false; }
            }
            else
            {
                if (IsExist_GetTotalTimePlay.Value) return CaptureIPApi.GetTotalTimePlay(totalTimePtr);
            }
            return 0;
        }

        public static int CreateStreamIP(int id, IntPtr net_addr, IntPtr content)
        {
            if (IsExist_CreateStreamIP == null)
            {
                try
                {
                    int value = CaptureIPApi.CreateStreamIP(id, net_addr, content);
                    IsExist_CreateStreamIP = true;
                    return value;
                }
                catch { IsExist_CreateStreamIP = false; }
            }
            else
            {
                if (IsExist_CreateStreamIP.Value) return CaptureIPApi.CreateStreamIP(id, net_addr, content);
            }
            return -1;
        }

        public static int CreateStreamFile(int id, int numberFile, IntPtr fileName)
        {
            if (IsExist_CreateStreamFile == null)
            {
                try
                {
                    int value = CaptureIPApi.CreateStreamFile(id, numberFile, fileName);
                    IsExist_CreateStreamFile = true;
                    return value;
                }
                catch { IsExist_CreateStreamFile = false; }
            }
            else
            {
                if (IsExist_CreateStreamFile.Value) return CaptureIPApi.CreateStreamFile(id, numberFile, fileName);
            }
            return -1;
        }

        public static int GetBMP_NmbCamera(int id, out int width, out int height, out int bitPerPixel, out IntPtr ptr)
        {
            if (IsExist_GetBMP_NmbCamera == null)
            {
                try
                {
                    int value = CaptureIPApi.GetBMP_NmbCamera(id, out width, out height, out bitPerPixel, out ptr);
                    IsExist_GetBMP_NmbCamera = true;
                    return value;
                }
                catch { IsExist_GetBMP_NmbCamera = false; }
            }
            else
            {
                if (IsExist_GetBMP_NmbCamera.Value) return CaptureIPApi.GetBMP_NmbCamera(id, out width, out height, out bitPerPixel, out ptr);
            }
            width = 0;
            height = 0;
            bitPerPixel = 0;
            ptr = IntPtr.Zero;
            return -1;
        }

        public static int ReleaseCameraIP_Nmb(int nmb_Cmr_del)
        {
            if (IsExist_ReleaseCameraIP_Nmb == null)
            {
                try
                {
                    int value = CaptureIPApi.ReleaseCameraIP_Nmb(nmb_Cmr_del);
                    IsExist_ReleaseCameraIP_Nmb = true;
                    return value;
                }
                catch { IsExist_ReleaseCameraIP_Nmb = false; }
            }
            else
            {
                if (IsExist_ReleaseCameraIP_Nmb.Value) return CaptureIPApi.ReleaseCameraIP_Nmb(nmb_Cmr_del);
            }
            return -1;
        }

        public static bool GetCamera_360(int id)
        {
            if (IsExist_GetCamera_360 == null)
            {
                try
                {
                    bool value = CaptureIPApi.GetCamera_360(id);
                    IsExist_GetCamera_360 = true;
                    return value;
                }
                catch { IsExist_GetCamera_360 = false; }
            }
            else
            {
                if (IsExist_GetCamera_360.Value) return CaptureIPApi.GetCamera_360(id);
            }
            return false;
        }

        public static bool SetCamera_360(int id, bool has360)
        {
            if (IsExist_SetCamera_360 == null)
            {
                try
                {
                    bool value = CaptureIPApi.SetCamera_360(id, has360);
                    IsExist_SetCamera_360 = true;
                    return value;
                }
                catch { IsExist_SetCamera_360 = false; }
            }
            else
            {
                if (IsExist_SetCamera_360.Value) return CaptureIPApi.SetCamera_360(id, has360);
            }
            return false;
        }

        public static int GetParamsCamera(int id, out int frequency, out int Width, out int Height)
        {
            if (IsExist_GetParamsCamera == null)
            {
                try
                {
                    int value = CaptureIPApi.GetParamsCamera(id, out frequency, out Width, out Height);
                    IsExist_GetParamsCamera = true;
                    return value;
                }
                catch { IsExist_GetParamsCamera = false; }
            }
            else
            {
                if (IsExist_GetParamsCamera.Value) return CaptureIPApi.GetParamsCamera(id, out frequency, out Width, out Height);
            }
            frequency = 0;
            Width = 0;
            Height = 0;
            return -1;
        }

        public static int WriteVideoStream(int id, PlayerState status)
        {
            if (IsExist_WriteVideoStream == null)
            {
                try
                {
                    int value = CaptureIPApi.WriteVideoStream(id, status);
                    IsExist_WriteVideoStream = true;
                    return value;
                }
                catch { IsExist_WriteVideoStream = false; }
            }
            else
            {
                if (IsExist_WriteVideoStream.Value) return CaptureIPApi.WriteVideoStream(id, status);
            }
            return -1;
        }

        public static int Joystick_Control(int id, JoystickButtonState status)
        {
            if (IsExist_Joystick_Control == null)
            {
                try
                {
                    int value = CaptureIPApi.Joystick_Control(id, status);
                    IsExist_Joystick_Control = true;
                    return value;
                }
                catch { IsExist_Joystick_Control = false; }
            }
            else
            {
                if (IsExist_Joystick_Control.Value) return CaptureIPApi.Joystick_Control(id, status);
            }
            return -1;
        }

        public static void CallBackFunc_PlayerState(PlayerStatus callback)
        {
            if (IsExist_CallBackFunc_PlayerState == null)
            {
                try
                {
                    CaptureIPApi.CallBackFunc_PlayerState(callback);
                    IsExist_CallBackFunc_PlayerState = true;
                }
                catch { IsExist_CallBackFunc_PlayerState = false; }
            }
            else
            {
                if (IsExist_CallBackFunc_PlayerState.Value) CaptureIPApi.CallBackFunc_PlayerState(callback);
            }
        }

        public static void CallBackFunc_GetTimePlayer(GetTimeMovie callback)
        {
            if (IsExist_CallBackFunc_GetTimePlayer == null)
            {
                try
                {
                    CaptureIPApi.CallBackFunc_GetTimePlayer(callback);
                    IsExist_CallBackFunc_GetTimePlayer = true;
                }
                catch { IsExist_CallBackFunc_GetTimePlayer = false; }
            }
            else
            {
                if (IsExist_CallBackFunc_GetTimePlayer.Value) CaptureIPApi.CallBackFunc_GetTimePlayer(callback);
            }
        }

        public static void CallBackFunc_SetMovieFolder(SetMovieFolder callback)
        {
            if (IsExist_CallBackFunc_SetMovieFolder == null)
            {
                try
                {
                    CaptureIPApi.CallBackFunc_SetMovieFolder(callback);
                    IsExist_CallBackFunc_SetMovieFolder = true;
                }
                catch { IsExist_CallBackFunc_SetMovieFolder = false; }
            }
            else
            {
                if (IsExist_CallBackFunc_SetMovieFolder.Value) CaptureIPApi.CallBackFunc_SetMovieFolder(callback);
            }
        }

        public static void CallBackFunc_JoyControl(Joy_IsConnect joy_IsConnect, Joy_Control joy_Control)
        {
            if (IsExist_CallBackFunc_JoyControl == null)
            {
                try
                {
                    CaptureIPApi.CallBackFunc_JoyControl(joy_IsConnect, joy_Control);
                    IsExist_CallBackFunc_JoyControl = true;
                }
                catch { IsExist_CallBackFunc_JoyControl = false; }
            }
            else
            {
                if (IsExist_CallBackFunc_JoyControl.Value) CaptureIPApi.CallBackFunc_JoyControl(joy_IsConnect, joy_Control);
            }
        }

        public static int SetPathsVideo(string pathRecord, string pathPlay)
        {
            if (IsExist_SetPathsVideo == null)
            {
                try
                {
                    int value = CaptureIPApi.SetPathsVideo(pathRecord, pathPlay);
                    IsExist_SetPathRec = true;
                    return value;
                }
                catch { IsExist_SetPathRec = false; return -1; }
            }
            else
            {
                if (IsExist_SetPathRec.Value) CaptureIPApi.SetPathsVideo(pathRecord, pathPlay);
            }
            return -1;
        }

        public static int SetPathsAudio(string pathRecord, string pathPlay)
        {
            if (IsExist_SetPathsAudio == null)
            {
                try
                {
                    int value = CaptureIPApi.SetPathsAudio(pathRecord, pathPlay);
                    IsExist_SetPathsAudio = true;
                    return value;
                }
                catch { IsExist_SetPathsAudio = false; return -1; }
            }
            else
            {
                if (IsExist_SetPathsAudio.Value) CaptureIPApi.SetPathsAudio(pathRecord, pathPlay);
            }
            return -1;
        }

        public static int SetAudioDevice(int deviceNumber, bool isEnable)
        {
            if (IsExist_SetAudioDevice == null)
            {
                try
                {
                    int value = CaptureIPApi.SetAudioDevice(deviceNumber, isEnable);
                    IsExist_SetAudioDevice = true;
                    return value;
                }
                catch { IsExist_SetAudioDevice = false; return -2; }
            }
            else
            {
                if (IsExist_SetAudioDevice.Value) CaptureIPApi.SetAudioDevice(deviceNumber, isEnable);
            }
            return -2;
        }

        public static int SetTypeInterface(PlayerState type)
        {
            if (IsExist_SetTypeInterface == null)
            {
                try
                {
                    int value = CaptureIPApi.SetTypeInterface(type);
                    IsExist_SetTypeInterface = true;
                    return value;
                }
                catch { IsExist_SetTypeInterface = false; return -1; }
            }
            else
            {
                if (IsExist_SetTypeInterface.Value) CaptureIPApi.SetTypeInterface(type);
            }
            return -1;
        }

        private static class CaptureIPApi
        {

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void SetPlayerManualControl([MarshalAs(UnmanagedType.U1)] bool isManual);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static bool SetTimeRecMinutes(int minRec);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static bool SetPort_VideoControl(int port);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static bool SetCurrentCamera(int nmb_cur_Cmr_in);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int Initialize_ALL(int flag);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int Release_ALL();

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetPathPlay(string path);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetTimePlay(int time);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int GetTotalTimePlay(IntPtr totalTimePtr);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int CreateStreamIP(int id, IntPtr net_addr, IntPtr content);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int CreateStreamFile(int id, int numberFile, IntPtr fileName);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int GetBMP_NmbCamera(int id, out int width, out int height, out int bitPerPixel, out IntPtr ptr);

            [DllImport("CaptureIP.dll", SetLastError = false, CallingConvention = CallingConvention.Cdecl)]
            public static extern int ReleaseCameraIP_Nmb(int nmb_Cmr_del);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl)]
            public extern static bool GetCamera_360(int id);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl)]
            public extern static bool SetCamera_360(int id, [MarshalAs(UnmanagedType.U1)] bool has360);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl)]
            public extern static int GetParamsCamera(int id, out int frequency, out int Width, out int Height);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl)]
            public extern static int WriteVideoStream(int id, [MarshalAs(UnmanagedType.U4)]PlayerState status);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl)]
            public extern static int Joystick_Control(int id, [MarshalAs(UnmanagedType.U4)]JoystickButtonState status);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void CallBackFunc_PlayerState([MarshalAs(UnmanagedType.FunctionPtr)] PlayerStatus callback);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void CallBackFunc_GetTimePlayer([MarshalAs(UnmanagedType.FunctionPtr)] GetTimeMovie callback);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void CallBackFunc_SetMovieFolder([MarshalAs(UnmanagedType.FunctionPtr)] SetMovieFolder callback);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void CallBackFunc_JoyControl([MarshalAs(UnmanagedType.FunctionPtr)] Joy_IsConnect joy_IsConnect, [MarshalAs(UnmanagedType.FunctionPtr)] Joy_Control joy_Control);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static void SetPathRec(string path);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetPathsVideo(string pathRecord, string pathPlay);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetPathsAudio(string pathRecord, string pathPlay);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetAudioDevice(int deviceNumber, [MarshalAs(UnmanagedType.U1)] bool isEnable);

            [DllImport("CaptureIP.dll", CallingConvention = CallingConvention.Cdecl), HandleProcessCorruptedStateExceptions]
            public extern static int SetTypeInterface(PlayerState type);
        }
    }
}
