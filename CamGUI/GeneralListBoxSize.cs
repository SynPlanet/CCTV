using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace Cam
{
    [ValueConversion(typeof(double), typeof(double))]
    class GeneralListBoxSize: IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            double k = 120d / 912d;
            double c = 130d - 1024d * k;
            return k * (double)value + c;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
