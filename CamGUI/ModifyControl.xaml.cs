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
using System.Windows.Controls.Primitives;

namespace Cam
{
    /// <summary>
    /// Interaction logic for ModifyControl.xaml
    /// </summary>
    public partial class ModifyControl : UserControl
    {
        
        public ModifyControl()
        {
            InitializeComponent();
        }

        private void Rec_Click(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.CurrentProfile.CurrentImageCamera != null)
                Memory.Instance.CurrentProfile.CurrentImageCamera.IsRec = !Memory.Instance.CurrentProfile.CurrentImageCamera.IsRec;
        }

        private void ToggleButton_Checked(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            ToggleButton btn = sender as ToggleButton;
            if (btn == null && Memory.Instance.CurrentProfile.CurrentImageCamera != null) return;
            Memory.Instance.CurrentProfile.CurrentImageCamera.Has3D = btn.IsChecked.Value;
            if (Memory.Instance.CurrentProfile.IsAuto) return;
            Memory.Instance.CurrentProfile.IsAuto = true;
            FrameworkElement win = GetWindow(this);
            win.DataContext = null;
            win.DataContext = Memory.Instance.CurrentProfile;
            Memory.Instance.CurrentProfile.IsAuto = false;
        }

        private FrameworkElement GetWindow(FrameworkElement element)
        {
            if (element.Parent == null) return element;
            return GetWindow(element.Parent as FrameworkElement);
        }

        private void ToggleButton_Checked_1(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            ToggleButton btn = sender as ToggleButton;
            if (btn == null && Memory.Instance.CurrentProfile.CurrentImageCamera != null) return;
            Memory.Instance.CurrentProfile.CurrentImageCamera.HasInfo = btn.IsChecked.Value;
        }
    }
}
