using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Windows.Controls.Primitives;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Windows.Media.Animation;

namespace Cam
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    internal partial class MainWindow : Window
    {
        bool isEven;
        DispatcherTimer timer;
        bool isStop = false;
        object locker = new object();
        bool isLoading = true;

        public static Log Writer;

        private GCHandle SetFolderHandle;

        public MainWindow()
        {
            Writer.WriteLine("Start debug");
            isLoading = true;
            SetMovieFolder method = new SetMovieFolder(SetFolder);
            SetFolderHandle = GCHandle.Alloc(method);

            CaptureIP.CallBackFunc_SetMovieFolder(method);

            Config.Initialize();
            InitializeComponent();
            Config.Load();
            CaptureIP.Initialize_ALL(Memory.Instance.InitPTZFlag);
            if (Memory.Instance.isArchive == null) 
                CaptureIP.SetTypeInterface(PlayerState.WRITE);
            Profile current = Memory.Instance.ProfileCollection.FirstOrDefault(x => x.Name == Memory.Instance.CurrentProfileName);
            if (current == null) Memory.Instance.CurrentProfile = new Profile(string.Empty, true);
            else Memory.Instance.CurrentProfile = current;
           
            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(30);
            timer.Tick += new EventHandler(delegate(object sender, EventArgs e) { if (isEven && Memory.Instance.CurrentProfile != null) Memory.Instance.CurrentProfile.UpdateFrameCameraCollection(); isEven = !isEven; });
            timer.Start();
          
            manual.IsChecked = Memory.Instance.IsManual;


        }

        private void SetFolder(IntPtr ptr, int count)
        {
            this.Dispatcher.Invoke(new Action(delegate()
            {
                Writer.WriteLine("isArchive = {0}", Memory.Instance.isArchive.HasValue == false ? false : Memory.Instance.isArchive.Value);
                if (Memory.Instance.isArchive.HasValue && Memory.Instance.isArchive.Value)
                {
                    ((App)App.Current).Folder = Marshal.PtrToStringAnsi(ptr, count).Replace("\0", "");

                    Writer.WriteLine("Folder open = {0}", ((App)App.Current).Folder);
                    isLoading = true;
                    Button_Click_4(this, null);
                    UpdateDataContex(true);
                    isLoading = false;

                }
            }));
        }

        void Timer_Tick()
        {
            while (true)
            {
                while (isStop) Thread.Sleep(100);
                CameraInfo[] array = new CameraInfo[0];
                lock (locker)
                {
                    if (isStop) continue;
                    if (Memory.Instance.CurrentProfile != null)
                    {
                        lock (Memory.Instance.CurrentProfile.locker) array = Memory.Instance.CurrentProfile.ImageCameraCollection.ToArray();
                    }
                }
                foreach (var item in array)
                    Task.Factory.StartNew(item.Reconnect);
                Thread.Sleep(1000);
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            CaptureIP.SetTimeRecMinutes(Memory.Instance.TimeMinRec);
            if (Memory.Instance.CurrentProfile == null) Memory.Instance.CurrentProfile = new Profile("", true);
            if (Memory.Instance.CurrentProfile != null)
            {
               // Memory.Instance.CurrentProfile.IsArchive = Memory.Instance.CurrentProfile.IsArchiveStartup;
                if (Memory.Instance.CurrentProfile.IsArchiveStartup || ((App)App.Current).Folder != null) Button_Click_4(sender, e);
                UpdateDataContex(true);
             //   this.DataContext = Memory.Instance.CurrentProfile;
             //   monitor.ChangeView(MonitorView.Monitor4Plus);
                Memory.Instance.CurrentProfile.Joystick += new Joy_Control(Instance_Joystick);
            }
            else this.DataContext = null;
            Task.Factory.StartNew(Timer_Tick);
            isLoading = false;
            SetWindow();
            this.WindowState = WindowState.Maximized;
        }

        private void SetWindow()
        {
            if (Memory.Instance.RunToWindow < System.Windows.Forms.Screen.AllScreens.Length)
            {
                Left = System.Windows.Forms.Screen.AllScreens[Memory.Instance.RunToWindow].Bounds.Left + 1;
                Top = System.Windows.Forms.Screen.AllScreens[Memory.Instance.RunToWindow].Bounds.Top + 1;
            }
           
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            monitor.ChangeView(MonitorView.Monitor4);
            Memory.Instance.CurrentProfile.CameraSlot = 1;
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            monitor.ChangeView(MonitorView.MonitorN);
            Memory.Instance.CurrentProfile.CameraSlot = 2;
        }

        private void AddCamera(object sender, RoutedEventArgs e)
        {
          if (Memory.Instance.CurrentProfile == null)
            return;

            for (int i = 1; i < 129; i++)
            {
                if (Memory.Instance.CurrentProfile.ImageCameraCollection.FirstOrDefault(x => x.Name == string.Format("Camera{0}", i)) == null)
                {
                    Memory.Instance.CurrentProfile.ImageCameraCollection.Add(new CameraInfo(false) { Name = string.Format("Camera{0}", i) });
                    Memory.Instance.CurrentProfile.CurrentImageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection[Memory.Instance.CurrentProfile.ImageCameraCollection.Count - 1];
                    UpdateDataContex(false);
                    break;
                }
            }
        }

        private void RemoveCamera(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.CurrentProfile.ImageCameraCollection.Count > 0)
            {
                int index = Memory.Instance.CurrentProfile.ImageCameraCollection.IndexOf(Memory.Instance.CurrentProfile.CurrentImageCamera);
                if (index == -1) return;
                Memory.Instance.CurrentProfile.ImageCameraCollection.RemoveAt(index);
                if (Memory.Instance.CurrentProfile.ImageCameraCollection.Count == 0) Memory.Instance.CurrentProfile.CurrentImageCamera = null;
                else if (Memory.Instance.CurrentProfile.ImageCameraCollection.Count == index) Memory.Instance.CurrentProfile.CurrentImageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection[Memory.Instance.CurrentProfile.ImageCameraCollection.Count - 1];
                else Memory.Instance.CurrentProfile.CurrentImageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection[index];
                UpdateDataContex(false);
                
            }
        }

        private void UpdateDataContex(bool isReload)
        {
            lock (locker) isStop = true;
            this.DataContext = null;
          //  Dispatcher.BeginInvoke(new Action(() => DataContext = Memory.Instance.CurrentProfile));
            if (isReload)
                Dispatcher.BeginInvoke(new Action(delegate()
                {
                    //                 if (Memory.Instance.CurrentProfile.CameraSlot == 0) { slot1.IsChecked = true; Button_Click_3(null, null); };
                    //                 if (Memory.Instance.CurrentProfile.CameraSlot == 1) { slot2.IsChecked = true; Button_Click(null, null); }
                    //                 if (Memory.Instance.CurrentProfile.CameraSlot == 2) { slot3.IsChecked = true; Button_Click_1(null, null); }
                 //   var temp = Memory.Instance.CurrentProfile.ImageCameraCollection.ToArray();
                    lock (Memory.Instance.CurrentProfile.locker)
                    {
                        var temp = Memory.Instance.CurrentProfile.ImageCameraCollection.ToArray();
                        var camera = Memory.Instance.CurrentProfile.CurrentImageCamera;
                        Memory.Instance.CurrentProfile.CurrentImageCamera = null;
                        Memory.Instance.CurrentProfile.ImageCameraCollection.Clear();
                        foreach (var item in temp) Memory.Instance.CurrentProfile.ImageCameraCollection.Add(item);
                        Memory.Instance.CurrentProfile.CurrentImageCamera = camera;

                    }
                }));
            Dispatcher.BeginInvoke(new Action(delegate()
            {
                DataContext = Memory.Instance.CurrentProfile;
                if (isReload)
                {
                    if (Memory.Instance.CurrentProfile.CameraSlot == 0) { slot1.IsChecked = true; Button_Click_3(null, null); };
                    if (Memory.Instance.CurrentProfile.CameraSlot == 1) { slot2.IsChecked = true; Button_Click(null, null); }
                    if (Memory.Instance.CurrentProfile.CameraSlot == 2) { slot3.IsChecked = true; Button_Click_1(null, null); }
                }
            }));
            Dispatcher.BeginInvoke(new Action(() => isStop = false));
       //     Dispatcher.BeginInvoke(new Action(() => DataContext = Memory.Instance.CurrentProfile));
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }
        private void ListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.CurrentProfile.IsAuto) return;
            ListBox list = sender as ListBox;
            if (list == null) return;
            Memory.Instance.CurrentProfile.CurrentImageCamera = list.SelectedItem as CameraInfo;
        }
        Grid oldParent;
        private void monitor_ChangeMode(object sender, EventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            panel.Visibility = Visibility.Hidden;
            fullScreen.Visibility = Visibility.Visible;
            oldParent = Memory.Instance.CurrentProfile.CurrentImageCamera.Image.Parent as Grid;
            oldParent.Children.RemoveAt(0);
            Grid grid = new Grid();
            grid.Children.Add(Memory.Instance.CurrentProfile.CurrentImageCamera.Image);
            CreateJoystick(grid);
            fullScreen.Child = grid;
        }

        private void SetJoystickOpacity(Joystick joystick)
        {
            Point point = Mouse.GetPosition(joystick);
            point.Offset(-90, -67.5);
            double distance = Math.Sqrt(point.X * point.X + point.Y * point.Y);
            if (distance < 110) joystick.Opacity = 0.8;
            else if (distance < 150) joystick.Opacity = -0.02 * distance + 3;
            else joystick.Opacity = 0;
        }

        private void CreateJoystick(Grid grid)
        {
            if (Memory.Instance.CurrentProfile.CurrentImageCamera.Has3D && Memory.Instance.CurrentProfile.CurrentImageCamera.IsPlayer == false)
            {
                var joystick = CreateJoystick();
                grid.Children.Add(joystick);
                grid.MouseMove += new MouseEventHandler(delegate(object sender1, MouseEventArgs e1)
                {
                    SetJoystickOpacity(joystick);
                });
                grid.MouseLeave += new MouseEventHandler(delegate(object sender1, MouseEventArgs e1)
                {
                    Point point = Mouse.GetPosition(grid);
                    if (point.X < 0 || point.Y < 0 || point.X > grid.ActualWidth || point.Y > grid.ActualHeight) joystick.Opacity = 0;
                });
            }
        }

        private Joystick CreateJoystick()
        {
            Joystick joystick = new Joystick();
            joystick.DataContext = Memory.Instance.CurrentProfile.CurrentImageCamera;
            joystick.Press += new PressEventHandler(joystick_Press);
            joystick.JoystickRemoteControl += new JoystickRemoteControlEventHandler(joystick_JoystickRemoteControl);
            joystick.Width = 180;
            joystick.Height = 135;
            joystick.Opacity = 0;
            joystick.Tag = 789;
            return joystick;
        }

        void joystick_JoystickRemoteControl(object sender, JoystickRemoteControlEventArgs args)
        {
            Joystick joy = sender as Joystick;
            if (joy == null) return;
            var kk = joy.Tag;
            if (args.State.HasFlag(JoystickButtonState.Down) || args.State.HasFlag(JoystickButtonState.Up) || args.State.HasFlag(JoystickButtonState.Left) || args.State.HasFlag(JoystickButtonState.Right) || args.State.HasFlag(JoystickButtonState.ZoomIn) || args.State.HasFlag(JoystickButtonState.ZoomOut))
            {
                this.Dispatcher.BeginInvoke(new Action(delegate()
                {
                    if (Memory.Instance.CurrentProfile == null) return;
                    lock (Memory.Instance.CurrentProfile.locker)
                    {
                        CameraInfo info = joy.DataContext as CameraInfo;
                        var imageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection.Where(x => x.Camera != null).FirstOrDefault(x => x.Camera.ID == args.ID);
                        if (info == null || info.Camera == null || info != imageCamera) return;
                        /* if (info == null || info.Camera == null || info != Memory.Instance.CurrentProfile.ImageCameraCollection[(int)args.ID]) return;*/
                        if (info.Has3D)
                        {
                            joy.Visibility = System.Windows.Visibility.Visible;
                            joy.BeginAnimation(Joystick.OpacityProperty, this.FindResource("joystickAnimation") as AnimationTimeline);
                        }
                    }
                }));
            }
        }

        void joystick_Press(object sender, PressEventArgs args)
        {
            if (Memory.Instance.CurrentProfile == null || Memory.Instance.CurrentProfile.CurrentImageCamera == null || Memory.Instance.CurrentProfile.CurrentImageCamera.Camera == null) return;
            CaptureIP.Joystick_Control(Memory.Instance.CurrentProfile.CurrentImageCamera.Camera.ID, args.State);
        }

        private void fullScreen_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (e.ClickCount == 2)
            {
                ((Grid)fullScreen.Child).Children.Clear();
                fullScreen.Child = null;
                fullScreen.Visibility = Visibility.Hidden;
                oldParent.Children.Insert(0, Memory.Instance.CurrentProfile.CurrentImageCamera.Image);
                panel.Visibility = Visibility.Visible;
            }
        }

        void Instance_Joystick(uint id, JoystickButtonState status)
        {
            this.Dispatcher.BeginInvoke(new Action(delegate()
            {
                if (Memory.Instance.CurrentProfile == null) return;
                if (fullScreen.IsVisible && fullScreen.Child as Grid != null && ((Grid)fullScreen.Child).Children.Count != 0 && ((Grid)fullScreen.Child).Children[0] as Image != Memory.Instance.CurrentProfile.CurrentImageCamera.Image) 
                {
                    UIElement element = ((Grid)fullScreen.Child).Children[0];
                    ((Grid)fullScreen.Child).Children.Clear();
                    fullScreen.Child = null;
                    oldParent.Children.Insert(0, element);
                    oldParent = Memory.Instance.CurrentProfile.CurrentImageCamera.Image.Parent as Grid;
                    oldParent.Children.RemoveAt(0);

                    Grid grid = new Grid();
                    grid.Children.Add(Memory.Instance.CurrentProfile.CurrentImageCamera.Image);
                    CreateJoystick(grid);
                    fullScreen.Child = grid;
                   
                }
            }));
        }

        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            monitor.ChangeView(MonitorView.Monitor4Plus);
            Memory.Instance.CurrentProfile.CameraSlot = 0;
        }

        private void Rec_Click(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.CurrentProfile.IsRec.HasValue && Memory.Instance.CurrentProfile.IsRec.Value) CaptureIP.WriteVideoStream(-1, PlayerState.STOP);
            else CaptureIP.WriteVideoStream(-1, PlayerState.WRITE);
        }

        private void AdminButtonClick(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            Button btn = sender as Button;
            if (btn == null) return;
            if (Memory.Instance.CurrentProfile.IsAdmin)
            {
                Memory.Instance.CurrentProfile.IsAdmin = false;
                return;
            }
            var win = new AdminWIndow() { DataContext = Memory.Instance.CurrentProfile, Owner = this, WindowStartupLocation = WindowStartupLocation.CenterOwner };
            win.SucceedPassword += new RoutedEventHandler(delegate(object sender1, RoutedEventArgs e1) { Memory.Instance.CurrentProfile.IsAdmin = !btn.IsPressed; });
            win.ShowDialog();
        }

        private void Joystick_Press(object sender, PressEventArgs args)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            CaptureIP.Joystick_Control(Memory.Instance.CurrentProfile.CurrentImageCamera.Camera.ID, args.State);
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            var setting = new Settings() { DataContext = Memory.Instance, Owner = this, WindowStartupLocation = WindowStartupLocation.CenterOwner };
            setting.ShowDialog();
            if (setting.IsChange)
            {
                Memory.Instance.CurrentProfile.IsAuto = true;
                var temp = Memory.Instance.CurrentProfile.CurrentImageCamera;
                Memory.Instance.CurrentProfile.CurrentImageCamera = null;
                Memory.Instance.CurrentProfile.CurrentImageCamera = temp;
                UpdateDataContex(true);
                Memory.Instance.CurrentProfile.IsAuto = false;
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            IsClosed = true;
            new Thread(KIll) { IsBackground = true }.Start();
            lock (locker) isStop = true;
            timer.Stop();
            if (Memory.Instance.CurrentProfile != null)
            {
                Config.Save(true);
                Memory.Instance.IsRelease = true;
                lock (Memory.Instance.CurrentProfile.locker)
                {
                    Memory.Instance.CurrentProfile.ImageCameraCollection.Clear();
                    Memory.Instance.CurrentProfile.CurrentImageCamera = null;
                }
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
            CaptureIP.Release_ALL();
        }

        private void KIll()
        {
            Thread.Sleep(5000);
            Process.GetCurrentProcess().Kill();
        }

        private void Button_Click_4(object sender, RoutedEventArgs e)
        {
            Writer.WriteLine("Button_Click_4");
            Memory.Instance.CurrentProfile.IsArchive = true;
            lock (Memory.Instance.CurrentProfile.locker)
            {
                Memory.Instance.CurrentProfile.ImageCameraCollectionBackup.Clear();
                Memory.Instance.CurrentProfile.ImageCameraCollectionBackup.AddRange(Memory.Instance.CurrentProfile.ImageCameraCollection);
                Memory.Instance.CurrentProfile.CurrentImageCameraBackup = Memory.Instance.CurrentProfile.CurrentImageCamera;
                Memory.Instance.CurrentProfile.ImageCameraCollection.Clear();
                Memory.Instance.CurrentProfile.CurrentImageCamera = null;
            }
            UpdateDataContex(true);

            Button_Click_5(null, null);
        }
        string folder = null;
        private void Button_Click_5(object sender, RoutedEventArgs e)
        {
            Writer.WriteLine("Button_Click_5");
            CaptureIP.SetPathsVideo(string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToSave) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToSave, Memory.Instance.CurrentProfile.PathToPlay);
            CaptureIP.SetPathsAudio(string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToSaveAudio) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToSaveAudio, Memory.Instance.CurrentProfile.PathToPlayAudio);
            foreach (var item in Memory.Instance.CurrentProfile.AudioDeviceCollection)
                CaptureIP.SetAudioDevice(item.Number, item.IsEnable.HasValue ? item.IsEnable.Value : false);
           // string folder;
            if (sender == null && e == null && ((App)App.Current).Folder != null && isLoading)
            {
                folder = ((App)App.Current).Folder;
            }
            else if (sender != null && e != null)
            {
                Browse win = new Browse() { Owner = this, WindowStartupLocation = WindowStartupLocation.CenterOwner };
                if (string.IsNullOrEmpty(Memory.Instance.CurrentProfile.PathToPlay) == false && System.IO.Directory.Exists(Memory.Instance.CurrentProfile.PathToPlay) == true)
                {
                    System.IO.DirectoryInfo dir = new System.IO.DirectoryInfo(Memory.Instance.CurrentProfile.PathToPlay);
                    win.dir.ItemsSource = dir.EnumerateDirectories().Select(x => x.Name);
                }
                win.ShowDialog();
                folder = win.dir.SelectedItem as string;
            }
            Writer.WriteLine("Button_Click_5 folder = {0}", folder);
            if (folder == null) return;
            Memory.Instance.CurrentProfile.ImageCameraCollection.Clear();
            monitor.slider.Visibility = System.Windows.Visibility.Collapsed;
            Title = string.Format("{0} ({1})", Local.Message.MainWindow_Message0, folder);

            int countFiles = CaptureIP.SetPathsVideo(Memory.Instance.CurrentProfile.PathToSave, string.Format("{0}\\{1}", string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToPlay) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToPlay, folder));

            CaptureIP.SetPathsAudio(string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToSaveAudio) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToSaveAudio, string.Format("{0}\\{1}", string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToPlayAudio) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToPlayAudio, folder));

            for (int i = 0; i < countFiles; i++) Memory.Instance.CurrentProfile.ImageCameraCollection.Add(new CameraInfo(false) { IsPlayer = true, PlayerIndex = i });
            if (Memory.Instance.CurrentProfile.ImageCameraCollection.Count != 0) Memory.Instance.CurrentProfile.CurrentImageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection[0];
            monitor.slider.Visibility = System.Windows.Visibility.Visible;
            UpdateDataContex(true);
            CaptureIP.WriteVideoStream(-1, PlayerState.PAUSE);
        }

        private void Button_Click_6(object sender, RoutedEventArgs e)
        {
            Memory.Instance.CurrentProfile.IsArchive = false;
            foreach (var item in Memory.Instance.CurrentProfile.AudioDeviceCollection)
                CaptureIP.SetAudioDevice(item.Number, item.IsEnable.HasValue ? item.IsEnable.Value : false);
            monitor.slider.Visibility = System.Windows.Visibility.Hidden;
            Memory.Instance.IsAudioDeviceEnable = true;
            lock (Memory.Instance.CurrentProfile.locker)
            {
                Memory.Instance.CurrentProfile.ImageCameraCollection.Clear();
                foreach (var item in Memory.Instance.CurrentProfile.ImageCameraCollectionBackup) Memory.Instance.CurrentProfile.ImageCameraCollection.Add(item);
                Memory.Instance.CurrentProfile.CurrentImageCamera = Memory.Instance.CurrentProfile.CurrentImageCameraBackup;
                Memory.Instance.CurrentProfile.ImageCameraCollectionBackup.Clear();
                Memory.Instance.CurrentProfile.CurrentImageCameraBackup = null;
            }
            UpdateDataContex(true);
            Title = Local.Message.MainWindow_Message0;
        }

        private void Grid_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            leftCell.Width = new GridLength(e.NewSize.Width * 0.3 + 158.8);
            rightCell.Width = new GridLength(e.NewSize.Width * 0.0548 + 64.96);
        }

        private void test_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (isLoading == false) UpdateDataContex(true);
        }

        private void CheckManual(object sender, RoutedEventArgs e)
        {
            monitor.slider.IsManual = true;
        }

        private void UncheckManual(object sender, RoutedEventArgs e)
        {
            monitor.slider.IsManual = false;
        }

        public bool IsClosed { get; private set; }
    }
}
