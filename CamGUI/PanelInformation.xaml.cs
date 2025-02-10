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

namespace Cam
{
    /// <summary>
    /// Interaction logic for PanelInformation.xaml
    /// </summary>
    public partial class PanelInformation : UserControl
    {
        bool textChangeIsManual = false;

        public PanelInformation()
        {
            InitializeComponent();
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (textChangeIsManual)
            {
                textChangeIsManual = false;
                if (Memory.Instance.IsRelease) return;
                btnCancel.Visibility = btnSave.Visibility = Memory.Instance.CurrentProfile.CurrentImageCamera.Name == name.Text && Memory.Instance.CurrentProfile.CurrentImageCamera.Address == address.Text
                     && Memory.Instance.CurrentProfile.CurrentImageCamera.UserName == userName.Text && Memory.Instance.CurrentProfile.CurrentImageCamera.Password == password.Password ? Visibility.Hidden : Visibility.Visible;
            }
        }

        private void TextBox_PreviewKeyDown(object sender, KeyEventArgs e) 
        {
            textChangeIsManual = true;
            if (e.Key == Key.Enter) btnSave_Click(btnSave, null);
            if (e.Key == Key.Escape) btnCancel_Click(btnSave, null);
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            name.Text = Memory.Instance.CurrentProfile.CurrentImageCamera.Name;
            address.Text = Memory.Instance.CurrentProfile.CurrentImageCamera.Address;
            userName.Text = Memory.Instance.CurrentProfile.CurrentImageCamera.UserName;
            password.Password = Memory.Instance.CurrentProfile.CurrentImageCamera.Password;
            btnCancel.Visibility = btnSave.Visibility = Visibility.Hidden;
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            Memory.Instance.CurrentProfile.CurrentImageCamera.Name = name.Text;
            Memory.Instance.CurrentProfile.CurrentImageCamera.Address = address.Text;
            Memory.Instance.CurrentProfile.CurrentImageCamera.UserName = userName.Text;
            Memory.Instance.CurrentProfile.CurrentImageCamera.Password = password.Password;
            btnCancel.Visibility = btnSave.Visibility = Visibility.Hidden;
        }

        private void password_PasswordChanged(object sender, RoutedEventArgs e)
        {
            TextBox_TextChanged(sender, null);
        }
    }
}
