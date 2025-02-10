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
    /// Interaction logic for AdminWIndow.xaml
    /// </summary>
    internal partial class AdminWIndow : Window
    {
        public static readonly RoutedEvent SucceedPasswordEvent = EventManager.RegisterRoutedEvent("SucceedPassword", RoutingStrategy.Direct, typeof(RoutedEventArgs), typeof(AdminWIndow));

        public AdminWIndow()
        {
            InitializeComponent();
        }

        public event RoutedEventHandler SucceedPassword
        {
            add
            {
                this.AddHandler(SucceedPasswordEvent, value);
            }
            remove
            {
                this.RemoveHandler(SucceedPasswordEvent, value);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (password.Password == Memory.Instance.Password) RaiseEvent(new RoutedEventArgs(SucceedPasswordEvent, this));
            else MessageBox.Show(this, Local.Message.AdminWIndow_Message3, Local.Message.AdminWIndow_Message4, MessageBoxButton.OK, MessageBoxImage.Error, MessageBoxResult.OK, MessageBoxOptions.None);
            Button_Click_1(sender, e);

        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            password.Focus();
        }
    }
}
