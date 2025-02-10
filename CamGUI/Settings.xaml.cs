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

namespace Cam
{
    /// <summary>
    /// Interaction logic for Settings.xaml
    /// </summary>
    public partial class Settings : Window
    {
        public Settings()
        {
            InitializeComponent();
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            Config.Save(false);
            CaptureIP.SetPathsAudio(string.IsNullOrWhiteSpace(Memory.Instance.CurrentProfile.PathToSaveAudio) ? Environment.CurrentDirectory : Memory.Instance.CurrentProfile.PathToSaveAudio, Memory.Instance.CurrentProfile.PathToPlayAudio);
            Close();
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            
        }

        private void TextBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape) btnCancel_Click(btnSave, null);
        }

        private void TextBoxPort_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            IsChange = true;
            int port = 0;
            try
            {
                port = int.Parse(this.portToSave.Text);
                if (port < 0 || port > 65535) portToSave.Text = Memory.Instance.CurrentProfile.RemoteControlPort.ToString();
                Memory.Instance.CurrentProfile.RemoteControlPort = int.Parse(this.portToSave.Text); 
            }
            catch { portToSave.Text = Memory.Instance.CurrentProfile.RemoteControlPort.ToString(); }
        }

        private void ComboBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            FrameworkElement element = sender as FrameworkElement;
            if (element == null) return;
            if (e.Key == Key.Delete)
            {
                if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
                {
                    RemoveProfile(element);
                    
                }
            }
            else if (e.Key == Key.Enter)
            {
                IsChange = true;
                Dispatcher.BeginInvoke(new Action(delegate() { element.Focus(); }));
            }
        }

        private void RemoveProfile(FrameworkElement element)
        {
            if (Memory.Instance.ProfileCollection.Remove(Memory.Instance.CurrentProfile))
            {
                IsChange = true;
                Memory.Instance.ProfileArray.RemoveAll(x => x.Name == Memory.Instance.CurrentProfile.Name);
                if (Memory.Instance.ProfileCollection.Count != 0) Memory.Instance.CurrentProfile = Memory.Instance.ProfileCollection[0];
                else Memory.Instance.CurrentProfile = new Profile(string.Empty, true);
                Dispatcher.BeginInvoke(new Action(delegate() { element.Focus(); }));
            }
        }
        private void CheckBox_Checked(object sender, RoutedEventArgs e)
        {
            IsChange = true;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            RemoveProfile(test);
        }

        public bool IsChange { get; private set; }

        private void pathToSave_TextChanged(object sender, TextChangedEventArgs e)
        {
            IsChange = true;
        }

        private void pathToPlay_TextChanged(object sender, TextChangedEventArgs e)
        {
            IsChange = true;
        }

        private void test_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            IsChange = true;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            IsChange = false;
        }
    }
}
