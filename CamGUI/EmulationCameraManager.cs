using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ExceptionServices;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.ComponentModel;
using System.Net;

namespace Cam
{
    class EmulationCameraManager
    {
        static object locker = new object();
        List<ICamera> array;
        int isRun = 0;

        public EmulationCameraManager()
        {
            array = new List<ICamera>();
        }

       

        public void AddCamera(CameraInfo info)
        {
            if (info == null || info.Path == null) return;
            lock (locker)
            {
                if (array.Find(x => x.PictureControl == info.Image) != null) return;
                array.Add(new IPCamera(info));
            }
            info.Camera = array[array.Count - 1];
            info.hasContent = false;
        }

        public void Clear()
        {
            lock (locker)
            {
                foreach (var item in array) ((IDisposable)item).Dispose();
                array.Clear();
                array.TrimExcess();
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        public void Remove(ICamera camera)
        {
            lock (locker)
            {
                array.Remove(camera);
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        public void UpdateBuffer()
        {
            if (Interlocked.CompareExchange(ref isRun, 1, 0) == 0)
            {
                var array = this.array.ToArray(); // копирую, т.к. в другом потоке массив может быть изменен
                Task.Factory.StartNew(new Action(delegate()
                {
                    foreach (var item in array) item.Update();
                    Interlocked.Exchange(ref isRun, 0);
                }));
            }
        }

        public void Update()
        {
            int a, b, c;


            foreach (var item in array)
            {
                item.UpdateImage();
                if (CaptureIP.GetParamsCamera(item.ID, out a, out b, out c) < 0)
                {
                    ((IPCamera)item).Frequency = 0;
                    ((IPCamera)item).Width = 0;
                    ((IPCamera)item).Height = 0;
                    ((IPCamera)item).TextInfo = Local.Message.EmulationCameraManager_Message0;
                }
                else
                {
                    ((IPCamera)item).Frequency = a;
                    ((IPCamera)item).Width = b;
                    ((IPCamera)item).Height = c;
                    ((IPCamera)item).TextInfo = string.Format("{0}: {1}x{2}  {3}{4}", ((IPCamera)item).CameraInfo.Name, b, c, a, Local.Message.EmulationCameraManager_Message1);
                }
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        private void GetSizeCapture(out int maxPixelWidth, out int maxPixelHeight)
        {
            maxPixelWidth = 1;
            maxPixelHeight = 1;
            for (int i = 0; i < array.Count; i++)
            {
                if (array[i].PictureControl == null || array[i].PictureControl.Parent == null) continue;
                if (maxPixelWidth < (int)((FrameworkElement)array[i].PictureControl.Parent).ActualWidth) maxPixelWidth = (int)((FrameworkElement)array[i].PictureControl.Parent).ActualWidth;
                maxPixelHeight += (int)((FrameworkElement)array[i].PictureControl.Parent).ActualHeight;
            }
        }

        class IPCamera : ICamera , IDisposable
        {
            static int nexrID = 0;
            bool isDispose = false;
            int width, height, bitPerPixel;
            IntPtr buffer = IntPtr.Zero;
            bool hasPtr;
            WriteableBitmap writeable = null;
            int frequency, widthInfo, heightInfo;
            string textInfo = string.Empty;
            bool hasStream = false;

            public IPCamera(CameraInfo info)
            {
                CameraInfo = info;
                PictureControl = info.Image;
                ID = nexrID;
                nexrID++;
                writeable = new WriteableBitmap(1, 1, 96, 96, PixelFormats.Bgr32, null);
                PictureControl.Source = writeable;
                lock (locker)
                {
                    //buffer = Marshal.AllocHGlobal(1920 * 1080 * 4);
                    hasPtr = false;
                    hasStream = false;
                }
            }

            public void UpdateImage()
            {
                if (!hasPtr) return;

                lock (locker)
                {
                    if (isDispose) return;
                    if (width != writeable.PixelWidth || height != writeable.PixelHeight || bitPerPixel != writeable.Format.BitsPerPixel)
                    {
                        writeable = new WriteableBitmap(width, height, 96, 96, bitPerPixel == 32 ? PixelFormats.Bgr32 : PixelFormats.Bgr24, null);
                        PictureControl.Source = writeable;
                    }
                    if (buffer != IntPtr.Zero)
                        writeable.WritePixels(new Int32Rect(0, 0, width, height), buffer, width * height * bitPerPixel / 8, width * bitPerPixel / 8);
                }
            }

            public Image PictureControl { get; private set; }

            public bool Update(string header, string tile)
            {
                if (isDispose) return false;
                IntPtr contentPtr = IntPtr.Zero;
                IntPtr infoPtr = IntPtr.Zero;
                IntPtr fileNamePtr = IntPtr.Zero;
                try
                {
                  //  IPAddressFull ip = IPAddressFull.Parse(CameraInfo.Address);
                  //  int size= Marshal.SizeOf(typeof(T_NetAddress));
                  //  contentPtr = Marshal.AllocHGlobal(260);
                    fileNamePtr = Marshal.AllocHGlobal(260);
              /*      var info = new T_NetAddress()
                    {
                        IP = ip.GetAddressBytes(),
                        Login = CameraInfo.UserName,
                        Password = CameraInfo.Password,
                        Port = ip.Port,
                        Protocol = header,
                        Tile = tile
                    };*/
                    if (CameraInfo.IsPlayer)
                    {
                        hasStream = CaptureIP.CreateStreamFile(ID, CameraInfo.PlayerIndex, fileNamePtr) >= 0;
                        CameraInfo.Name = Marshal.PtrToStringAnsi(fileNamePtr);
                    }
                    else
                    {
                        IPAddressFull ip = IPAddressFull.Parse(CameraInfo.Address);
                        contentPtr = Marshal.AllocHGlobal(260);
                        var info = new T_NetAddress()
                        {
                            IP = ip.GetAddressBytes(),
                            Login = CameraInfo.UserName,
                            Password = CameraInfo.Password,
                            Port = ip.Port,
                            Protocol = header,
                            Tile = tile
                        }; 
                        infoPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(T_NetAddress)));
                        Marshal.StructureToPtr(info, infoPtr, false);
                        if (Memory.Instance.IsRelease) return false;
                        hasStream = CaptureIP.CreateStreamIP(ID, infoPtr, contentPtr) >= 0;
                        if (hasStream) CameraInfo.Content = Marshal.PtrToStringAnsi(contentPtr);
                    }
                 /*   infoPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(T_NetAddress)));
                    Marshal.StructureToPtr(info, infoPtr, false);
                    if (Memory.Instance.IsRelease) return false;
                    hasStream = CaptureIP.CreateStreamIP(ID, infoPtr, contentPtr) >= 0;
                    if (hasStream) CameraInfo.Content = Marshal.PtrToStringAnsi(contentPtr);*/
                }
                catch
                {
                    hasStream = false;
                }
                finally
                {
                    if (contentPtr != IntPtr.Zero) Marshal.FreeHGlobal(contentPtr);
                    if (infoPtr != IntPtr.Zero) Marshal.FreeHGlobal(infoPtr);
                    if (fileNamePtr != IntPtr.Zero) Marshal.FreeHGlobal(fileNamePtr);
                }
                if (hasStream && isDispose)
                {
                    CaptureIP.ReleaseCameraIP_Nmb(ID);
                    hasStream = false;
                }
                return hasStream;
            }

            [HandleProcessCorruptedStateExceptions]
            public void Update()
            {
                lock (locker)
                {
                    if (isDispose || hasStream == false) return;
                    int reurnValue = CaptureIP.GetBMP_NmbCamera(ID, out width, out height, out bitPerPixel, out buffer);
                    if (buffer == IntPtr.Zero) return;
                    hasPtr = true;
                    if (reurnValue != 0) hasPtr = false;
                }
            }

            /*     [HandleProcessCorruptedStateExceptions]
                 public void Update()
                 {
                     IntPtr ptr;
                     // int reurnValue = GetBMP_NmbCamera(ID, out width, out height, out bitPerPixel, out ptr);
                     lock (locker)
                     {
                         int reurnValue = CaptureIP.GetBMP_NmbCamera(ID, out width, out height, out bitPerPixel, out ptr);
                         if (buffer == IntPtr.Zero) return;
                         hasPtr = true;
                         if (reurnValue == 0 && ptr != IntPtr.Zero)
                         {
                             Kernel32.CopyMemory(buffer, ptr, width * height * bitPerPixel / 8);
                         }
                         else hasPtr = false;
                     }
                 }
                 */

            public int ID {
                            get {
                                return ID_vol;
                            }

                private set {
                    Interlocked.Exchange(ref ID_vol, value);
                }
            }

            volatile int ID_vol = 0;


            public int Height
            {
                get
                {
                    return heightInfo;
                }
                set
                {
                    if (heightInfo == value) return;
                    heightInfo = value;
                    if (PropertyChanged != null)
                        PropertyChanged(this, new PropertyChangedEventArgs("Height"));
                }
            }

            public int Width
            {
                get
                {
                    return widthInfo;
                }
                set
                {
                    if (widthInfo == value) return;
                    widthInfo = value;
                    if (PropertyChanged != null)
                        PropertyChanged(this, new PropertyChangedEventArgs("Width"));
                }
            }

            public int Frequency
            {
                get
                {
                    return frequency;
                }
                set
                {
                    if (frequency == value) return;
                    frequency = value;
                    if (PropertyChanged != null)
                        PropertyChanged(this, new PropertyChangedEventArgs("Frequency"));
                }
            }

            public string TextInfo
            {
                get { return textInfo; }
                set
                {
                    if (textInfo == value) return;
                    textInfo = value;
                    if (PropertyChanged != null)
                        PropertyChanged(this, new PropertyChangedEventArgs("TextInfo"));
                }
            }

            public CameraInfo CameraInfo { get; private set; }

            public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;

            public void Dispose()
            {

                    if (hasStream) MainWindow.Writer.WriteLine(string.Format("{0}:{1}:{2}>Begin ReleaseCameraIP_Nmb({3})", DateTime.Now.Hour, DateTime.Now.Minute, DateTime.Now.Second, ID));
                    lock (locker)
                    {
                        isDispose = true;
                        try
                        {
                            if (hasStream)
                            {
                            MainWindow.Writer.WriteLine(string.Format("{0}:{1}:{2}>Now ReleaseCameraIP_Nmb({3})", DateTime.Now.Hour, DateTime.Now.Minute, DateTime.Now.Second, ID));
                                CaptureIP.ReleaseCameraIP_Nmb(ID);
                            }
                        }
                        catch { }
                    }
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                    if (hasStream) MainWindow.Writer.WriteLine(string.Format("{0}:{1}:{2}>End ReleaseCameraIP_Nmb({3})", DateTime.Now.Hour, DateTime.Now.Minute, DateTime.Now.Second, ID));
            }

        /*    public void Dispose()
            {
                lock (locker)
                {
                    if (buffer != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(buffer);
                        buffer = IntPtr.Zero;
                    }
                    try { CaptureIP.ReleaseCameraIP_Nmb(ID); }
                    catch { }
                }
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }*/
        }
    }
}
