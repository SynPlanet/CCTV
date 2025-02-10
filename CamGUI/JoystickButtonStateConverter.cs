using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace Cam
{
    [ValueConversion(typeof(JoystickButtonState), typeof(bool))]
    class JoystickButtonStateConverter: IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (Memory.Instance.CurrentProfile == null) return JoystickButtonState.None;
            JoystickButtonState state = (JoystickButtonState)value;
            if (parameter as string == "All") return state != JoystickButtonState.None && Memory.Instance.HasJoystick;
            return state.HasFlag(GetJoystickButtonState(parameter));
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        private JoystickButtonState GetJoystickButtonState(object obj)
        {
            string str = obj as string;
            if (str == null) return JoystickButtonState.None;
            switch (str)
            {
                case "Up":
                    return JoystickButtonState.Up;
                case "Down":
                    return JoystickButtonState.Down;
                case "Left":
                    return JoystickButtonState.Left;
                case "Right":
                    return JoystickButtonState.Right;
                case "ZoomIn":
                    return JoystickButtonState.ZoomIn;
                case "ZoomOut":
                    return JoystickButtonState.ZoomOut;
                default:
                    return JoystickButtonState.None;
            }
        }
    }
}
