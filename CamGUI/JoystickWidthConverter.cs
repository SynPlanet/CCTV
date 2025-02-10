using System;
using System.Windows.Data;

namespace Cam
{
    [ValueConversion(typeof(double), typeof(double))]
    class JoystickWidthConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            double k = 130d / 912d;
            double c = 120d - 1024d * k;
            k = k * (double)value + c;
            if (k > 180) return 180;
            return k;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
