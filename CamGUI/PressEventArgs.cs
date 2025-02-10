using System.Windows;

namespace Cam
{
    class PressEventArgs : RoutedEventArgs
    {
        public PressEventArgs(RoutedEvent routedEvent, object source, JoystickButtonState state)
            : base(routedEvent, source)
        {
            State = state;
        }

        public JoystickButtonState State { get; private set; }
    }
}
