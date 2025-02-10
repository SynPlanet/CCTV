using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;

namespace Cam
{
    class JoystickRemoteControlEventArgs : PressEventArgs
    {
        public JoystickRemoteControlEventArgs(RoutedEvent routedEvent, object source, uint id, JoystickButtonState state)
            : base(routedEvent, source, state)
        {
            ID = id;
        }

        public uint ID { get; private set; }

    }
}
