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
using System.Windows.Shapes;
using System.IO.IsolatedStorage;
using System.IO;
using System.Xml.Serialization;
using System.Windows.Controls.Primitives;

namespace Cam
{
    /// <summary>
    /// Interaction logic for Audio.xaml
    /// </summary>
    public partial class Audio : Window
    {
        WindowParam info;

        public Audio()
        {
            InitializeComponent();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (Memory.Instance.AudioWindow != null)
            {
                e.Cancel = true;
                this.WindowState = System.Windows.WindowState.Minimized;
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            IsolatedStorageFile machineStorage = null;
            IsolatedStorageFileStream stream = null;
            WindowParam info;
            try
            {
                machineStorage = IsolatedStorageFile.GetMachineStoreForAssembly();
                stream = new IsolatedStorageFileStream("AudioWindow.set", FileMode.Open, machineStorage);
                XmlSerializer xml = new XmlSerializer(typeof(WindowParam));
                info = (WindowParam)xml.Deserialize(stream);
                Left = info.Left;
                Top = info.Top;
                Width = info.Width;
                Height = info.Height;
                WindowState = info.WindowState;
            }
            catch
            {
                info = new WindowParam();
                info.Top = Top;
                info.Left = Left;
                info.Width = Width;
                info.Height = Height;
                info.WindowState = WindowState;
            }
            finally 
            {
                if (stream != null) stream.Close();
                if (machineStorage != null) machineStorage.Close();
            }
            this.info = info;
        }

        private void Window_LocationChanged(object sender, EventArgs e)
        {
            if (info == null || Left < -30000 || Top < -30000) return;
            info.Left = Left;
            info.Top = Top;
            UpdateWindowsState();
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (info == null) return;
            info.Width = Width;
            info.Height = Height;
            UpdateWindowsState();
        }

        private void Window_StateChanged(object sender, EventArgs e)
        {
            if (info == null) return;
            info.WindowState = WindowState;
            UpdateWindowsState();
        }

        private void UpdateWindowsState()
        {
            if (info == null || App.Current.MainWindow == null) return;
            IsolatedStorageFile machineStorage = null;
            IsolatedStorageFileStream stream = null;
            try
            {
                machineStorage = IsolatedStorageFile.GetMachineStoreForAssembly();
                stream = new IsolatedStorageFileStream("AudioWindow.set", FileMode.Create, machineStorage);
                XmlSerializer xml = new XmlSerializer(typeof(WindowParam));
                xml.Serialize(stream, info);
            }
            catch { }
            finally
            {
                if (stream != null) stream.Close();
                if (machineStorage != null) machineStorage.Close();
            }
        }

        private void ToggleButton_Checked(object sender, RoutedEventArgs e)
        {
            ToggleButton btn = sender as ToggleButton;
            if (btn == null) return;
            AudioDevice device = btn.DataContext as AudioDevice;
            if (device == null) return;
            CaptureIP.SetAudioDevice(device.Number, device.IsEnable.HasValue ? device.IsEnable.Value : false);
        }
    }
}
