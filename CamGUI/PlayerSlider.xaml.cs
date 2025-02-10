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
using System.Runtime.InteropServices;

namespace Cam
{
    /// <summary>
    /// Interaction logic for PlayerSlider.xaml
    /// </summary>
    public partial class PlayerSlider : UserControl
    {
        GetTimeMovie method;
        GCHandle methodHandle;
        static Image playImg = new Image() { Source = new BitmapImage(new Uri("pack://application:,,,/Play.png")) };
        static Image stopImg = new Image() { Source = new BitmapImage(new Uri("pack://application:,,,/Stop.png")) };
        private int currentTime = 0;
        private int maximum = 0;

        public PlayerSlider()
        {
            if (Memory.Instance.CurrentProfile != null)
                Memory.Instance.CurrentProfile.IsPlay = false;
            InitializeComponent();
            maximum = 360;
            btn.Content = stopImg;
            method = new GetTimeMovie(Timer);
            methodHandle = GCHandle.Alloc(method);
            slider.Width = 0;
            IsManual = Memory.Instance.IsManual;
          
            sliderGrid.SizeChanged += new SizeChangedEventHandler(sliderGrid_SizeChanged);
        }

        void sliderGrid_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            UpdatePositionSlider();
        }


        private void Grid_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (btn.Visibility == Visibility.Visible)
            {
                Point point = e.GetPosition(sender as Grid);
                double size = sliderGrid.ActualWidth - sliderGrid.Height;
                double offset = sliderGrid.Height / 2;
                double cursorPosition = point.X;
                if (cursorPosition < offset) cursorPosition = offset;
                else if (cursorPosition > size + offset) cursorPosition = size + offset;
                slider.Width = cursorPosition - offset;
                int t = (int)(slider.Width * Maximum / size);
                MainWindow.Writer.WriteLine("size = {0} offset =  {1} cursorPosition = {2} Maximum = {3} slider.Width = {4} t =  {5}", size, offset, cursorPosition, Maximum, slider.Width, t);
                UpdateClock(t);
                CaptureIP.SetTimePlay(t);
            }
        }

        public int Maximum { get { return maximum; } }

        private void btn_Click(object sender, RoutedEventArgs e)
        {
            if (btn.Visibility == Visibility)
            {
                if (Memory.Instance.CurrentProfile != null)
                    Memory.Instance.CurrentProfile.IsPlay = !Memory.Instance.CurrentProfile.IsPlay;
                UpdatePlayButton();
            }
        }

        private void UserControl_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (((MainWindow)App.Current.MainWindow).IsClosed) return;
            if (Visibility != Visibility.Visible)
            {
                CaptureIP.CallBackFunc_GetTimePlayer(null);
            }
            else
            {
                CaptureIP.CallBackFunc_GetTimePlayer(method);
               // IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(int)));
                try
                {
                    //if (CaptureIP.GetTotalTimePlay(ptr) != 0)
                  //  {
                  //      Maximum = Marshal.ReadInt32(ptr);
                  //      if (Maximum <= 0) Visibility = Visibility.Collapsed;
                 //       else
                  //      {
                            slider.Width = 0;
                            UpdatePositionSlider();
                  //      }
                  //  }
                  //  else Visibility = Visibility.Collapsed;
                }
                finally
                {
                   // if (ptr != IntPtr.Zero) Marshal.FreeHGlobal(ptr);
                }
            }
        }

        private void Timer(uint time, uint totalTime, PlayerState state)
        {
            System.Threading.Interlocked.Exchange(ref currentTime, (int)time);
            System.Threading.Interlocked.Exchange(ref maximum, (int)totalTime);
            Dispatcher.BeginInvoke(new Action(delegate()
            {
                if (Memory.Instance.CurrentProfile != null)
                {
                    if (state == PlayerState.PLAY || state == PlayerState.WRITE) Memory.Instance.CurrentProfile.IsPlay = true;
                    else Memory.Instance.CurrentProfile.IsPlay = false;
                    btn.Content = Memory.Instance.CurrentProfile.IsPlay ? stopImg : playImg;
                    UpdatePositionSlider();
                    if (state == PlayerState.PLAY || state == PlayerState.WRITE) Memory.Instance.IsAudioDeviceEnable = false;
                    else Memory.Instance.IsAudioDeviceEnable = true;
                }
            }));
           
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            CaptureIP.CallBackFunc_GetTimePlayer(null);
            methodHandle.Free();
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
            Dispatcher.BeginInvoke(new Action(() => UpdatePositionSlider()));
        }

        private void UpdatePositionSlider()
        {
            double size = sliderGrid.ActualWidth - sliderGrid.Height;
            if (Maximum < 1) maximum = 1;
            if (size < 0) size = 0;
            double width = currentTime * size / Maximum;
            if (width < 0 || double.IsNaN(width)) width = 0;
            MainWindow.Writer.WriteLine("UpdatePositionSlider=> ActualWidth = {0} Height = {1} Maximum = {2} size = {3} currentTime = {4} width = {5}",
                sliderGrid.ActualWidth, sliderGrid.Height, Maximum, size, currentTime, width);
            slider.Width = width;
            UpdateClock(currentTime);
        }

        private void UpdateClock(int ms)
        {
            TimeSpan time = TimeSpan.FromMilliseconds(ms);
            if (((int)time.TotalHours) > 0) currentTimeText.Text = string.Format("{0}:{1}:{2}", (int)time.TotalHours, time.Minutes.ToString().PadLeft(2, '0'), time.Seconds.ToString().PadLeft(2, '0'));
            else currentTimeText.Text = string.Format("{0}:{1}", time.Minutes.ToString().PadLeft(2, '0'), time.Seconds.ToString().PadLeft(2, '0'));
        }

        private void UpdatePlayButton()
        {
            if (btn.Visibility == Visibility.Visible)
            {
                if (Memory.Instance.CurrentProfile != null)
                {
                    btn.Content = Memory.Instance.CurrentProfile.IsPlay ? stopImg : playImg;
                    CaptureIP.WriteVideoStream(-1, Memory.Instance.CurrentProfile.IsPlay ? PlayerState.PLAY : PlayerState.PAUSE);
                }
            }
        }

        public bool IsManual
        {
            set
            {

                slider.Width = 0; // Для испарвления глюка WPF. Если у нас ширина равна максимальной возможной, то размер контейнера не изменяется при исменнении Visibility!!!
                if (value) btn.Visibility = Visibility.Visible;
                else btn.Visibility = Visibility.Collapsed;

                CaptureIP.SetPlayerManualControl(value);
            }
        }
    }
}
