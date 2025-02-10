using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Controls;
using System.Windows;

namespace Cam
{
    [ValueConversion(typeof(TabItem), typeof(Thickness))]
    class TabItemBorderConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            TabItem tab = value as TabItem;
            if (tab == null) return null;
            TabControl control = tab.Parent as TabControl;
            if (control == null) return null;
            int index = control.Items.IndexOf(tab);
            if (index < 0) return null;
            if (index == 0 && control.Items.Count == 1) return new Thickness(0);
            if (index + 1 != control.Items.Count) return new Thickness(0, 0, 2, 0);
            if (index + 1 == control.Items.Count) return new Thickness(0);
            return new Thickness(0);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
