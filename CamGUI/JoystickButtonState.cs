using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Cam
{
    [Flags]
    enum JoystickButtonState
    {
        None = 0,
        Up = 1 << 0,
        Down = 1 << 1,
        Left = 1 << 2,
        Right = 1 << 3,
        ZoomIn = 1 << 4,
        ZoomOut = 1 << 5
    }
}
