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
    /// Interaction logic for Joystick.xaml
    /// </summary>
    
    delegate void PressEventHandler(object sender, PressEventArgs args);
    delegate void JoystickRemoteControlEventHandler(object sender, JoystickRemoteControlEventArgs args);

    internal partial class Joystick : UserControl
    {
        public static readonly RoutedEvent PressEvent = EventManager.RegisterRoutedEvent("Press", RoutingStrategy.Direct, typeof(PressEventHandler), typeof(Joystick));
        public static readonly RoutedEvent JoystickRemoteControlEvent = EventManager.RegisterRoutedEvent("JoystickRemoteControl", RoutingStrategy.Direct, typeof(JoystickRemoteControlEventHandler), typeof(Joystick));

        public Joystick()
        {
            InitializeComponent();
            Mouse.AddMouseUpHandler(Application.Current.MainWindow, RepeatButton_MouseUpt);
        }

         private void RepeatButton_MouseUpt(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.None;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void MoveUp(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.Up));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.Up;
        }

        private void MoveDown(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.Down));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.Down;
        }

        private void DownLeft(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.Left));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.Left;
        }

        private void DownRight(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.Right));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.Right;
        }

        private void IncreaseScale(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.ZoomIn));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.ZoomIn;
        }

        private void DecreaseScale(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (Memory.Instance.HasJoystick) return;
            RaiseEvent(new PressEventArgs(PressEvent, this, JoystickButtonState.ZoomOut));
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            camera.JoystickButtonState = JoystickButtonState.ZoomOut;
        }

        public event PressEventHandler Press
        {
            add
            {
                this.AddHandler(PressEvent, value);
            }
            remove
            {
                this.RemoveHandler(PressEvent, value);
            }
        }

        public event JoystickRemoteControlEventHandler JoystickRemoteControl
        {
            add
            {
                this.AddHandler(JoystickRemoteControlEvent, value);
            }
            remove
            {
                this.RemoveHandler(JoystickRemoteControlEvent, value);
            }
        }

        private void LockMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
        }

        private void RepeatButton_MouseUp(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.Up)) camera.JoystickButtonState ^= JoystickButtonState.Up;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void RepeatButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.Down)) camera.JoystickButtonState ^= JoystickButtonState.Down;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void RepeatButton_MouseLeft(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.Left)) camera.JoystickButtonState ^= JoystickButtonState.Left;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void RepeatButton_MouseRight(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.Right)) camera.JoystickButtonState ^= JoystickButtonState.Right;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void RepeatButton_MouseZoomIn(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.ZoomIn)) camera.JoystickButtonState ^= JoystickButtonState.ZoomIn;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void RepeatButton_MouseZoomOut(object sender, MouseButtonEventArgs e)
        {
            CameraInfo camera = DataContext as CameraInfo;
            if (camera == null) return;
            if (camera.JoystickButtonState.HasFlag(JoystickButtonState.ZoomOut)) camera.JoystickButtonState ^= JoystickButtonState.ZoomOut;
            RaiseEvent(new PressEventArgs(PressEvent, this, camera.JoystickButtonState));
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            Memory.Instance.CurrentProfile.Joystick += new Joy_Control(Instance_Joystick);
        }

        void Instance_Joystick(uint id, JoystickButtonState status)
        {
            Dispatcher.Invoke(new Action(() => RaiseEvent(new JoystickRemoteControlEventArgs(JoystickRemoteControlEvent, this, id, status))));
            //RaiseEvent(new JoystickRemoteControlEventArgs(JoystickRemoteControlEvent, this, id, status));
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            Memory.Instance.CurrentProfile.Joystick -= new Joy_Control(Instance_Joystick);
        }
    }
}
