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
using System.Windows.Media.Animation;

namespace Cam
{
    /// <summary>
    /// Interaction logic for Monitor10.xaml
    /// </summary>
    /// 
    internal partial class Monitor : UserControl
    {
        UniformGrid smallPanel = new UniformGrid();
        private int countBigMonitors = 4;
        private Point currentpoint = new Point();
        private MonitorView mode = MonitorView.None;
        private const int MAX_CAMERA_NUMBER = 128;
     
        public static readonly RoutedEvent ChangeModeEvent = EventManager.RegisterRoutedEvent("ChangeMode", RoutingStrategy.Direct, typeof(EventHandler), typeof(Monitor));

        public Monitor()
        {
            InitializeComponent();
            ChangeView(MonitorView.Monitor4);
        }

        public void ChangeView(MonitorView viewer)
        {
            mode = viewer;
            UpdateMonitors();
        }

        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            if (e.NewValue == null)
            {
                ClearPanel(bigPanel);
                ClearPanel(smallPanel);
                smallPanel = null;
            }
            else
            {
                FrameworkElement element = sender as FrameworkElement;
                if (element == null) return;
                Memory memory = Memory.Instance;
                if (mode == MonitorView.Monitor4)
                {
                    countBigMonitors = 4;
                    bigPanel.Columns = 2;
                    bigPanel.Rows = 2;
                }
                else if (mode == MonitorView.Monitor4Plus)
                {
                    countBigMonitors = 4;
                    bigPanel.Columns = 2;
                    bigPanel.Rows = 2;
                    if (memory.CurrentProfile.ImageCameraCollection.Count > 4)
                    {
                        countBigMonitors = 3;
                        smallPanel = new UniformGrid();
                        if (memory.CurrentProfile.ImageCameraCollection.Count < 6)
                        {
                            smallPanel.Columns = 1;
                            smallPanel.Rows = 2;
                        }
                        else if (memory.CurrentProfile.ImageCameraCollection.Count < 8)
                        {
                            smallPanel.Columns = 2;
                            smallPanel.Rows = 2;
                        }
                        else if (memory.CurrentProfile.ImageCameraCollection.Count < 12)
                        {
                          smallPanel.Columns = 3;
                          smallPanel.Rows = 2;
                        }
                        else
                        {
                            smallPanel.Columns = 3;
                            smallPanel.Rows = 3;
                        }
                    }
                }
                else if (mode == MonitorView.MonitorN)
                {
          bigPanel.Columns = 1;
          bigPanel.Rows = 1;

          while(memory.CurrentProfile.ImageCameraCollection.Count > (bigPanel.Columns * bigPanel.Rows))
          {
            if (bigPanel.Columns != bigPanel.Rows)
              bigPanel.Columns++;
            else
              bigPanel.Rows++;
          }
          countBigMonitors = MAX_CAMERA_NUMBER;
/*
          countBigMonitors = 36;
                    if (memory.CurrentProfile.ImageCameraCollection.Count < 2)
                    {
                        bigPanel.Columns = 1;
                        bigPanel.Rows = 1;
                    }
                    else if (memory.CurrentProfile.ImageCameraCollection.Count < 3)
                    {
                        bigPanel.Columns = 1;
                        bigPanel.Rows = 2;
                    }
                    else if (memory.CurrentProfile.ImageCameraCollection.Count < 5)
                    {
                        bigPanel.Columns = 2;
                        bigPanel.Rows = 2;
                    }
                    else if (memory.CurrentProfile.ImageCameraCollection.Count < 7)
                    {
                        bigPanel.Columns = 3;
                        bigPanel.Rows = 2;
                    }
                    else if (memory.CurrentProfile.ImageCameraCollection.Count < 10)
                    {
                      bigPanel.Columns = 3;
                      bigPanel.Rows = 3;
                    }
                    else if (memory.CurrentProfile.ImageCameraCollection.Count < 17)
                    {
                      bigPanel.Columns = 4;
                      bigPanel.Rows = 4;
                    }
                else if (memory.CurrentProfile.ImageCameraCollection.Count < 26)
                {
                  bigPanel.Columns = 5;
                  bigPanel.Rows = 5;
                }
                else if (memory.CurrentProfile.ImageCameraCollection.Count < 37)
                {
                  bigPanel.Columns = 6;
                  bigPanel.Rows = 6;
                }
                */
              }
              else
                return;

              for (int i = 0; i < countBigMonitors && i < memory.CurrentProfile.ImageCameraCollection.Count; i++)
              {
                bigPanel.Children.Add(InsertToGrid(memory.CurrentProfile.ImageCameraCollection[i], false));
              }

              if (smallPanel != null)
              {
                  bigPanel.Children.Add(smallPanel);
                  for (int i = countBigMonitors; i < 10 && i < memory.CurrentProfile.ImageCameraCollection.Count; i++)
                    smallPanel.Children.Add(InsertToGrid(memory.CurrentProfile.ImageCameraCollection[i], countBigMonitors == 9 ? false : true));
              }
            }
        }

        private void ClearPanel(Panel panel)
        {
            if (panel == null) return;
            Grid grid;
            for (int i = 0; i < panel.Children.Count; i++)
            {
                if (panel.Children[i] is Grid == false) continue;
                grid = (Grid)panel.Children[i];
                for (int k = 0; k < grid.Children.Count; k++)
                {
                    if (grid.Children[k] is Border)
                    {
                        Grid grid1 = (Grid)(((Border)((Border)grid.Children[k]).Child).Child);
                        while (grid1.Children.Count != 0) grid1.Children.RemoveAt(0);
                        ((Border)((Border)grid.Children[k]).Child).Child = null;
                        ((Border)grid.Children[k]).Child = null;
                    }
                }
                grid.Children.Clear();
            }
            panel.Children.Clear();
        }

        private Grid InsertToGrid(CameraInfo info, bool isSmall)
        {
            Grid grid = new Grid();
            grid.Children.Add(CreateDisplay(info.Image));
            grid.DataContext = info;
            grid.Children.Add(CreateLogo3D(isSmall));
            if (!isSmall&&info.Has3D)
            {
                Joystick joystick = CreateJoystick();
                grid.Children.Add(joystick);
                grid.MouseMove += new MouseEventHandler(delegate(object sender, MouseEventArgs e)
                    {
                        SetJoystickOpacity(joystick);
                    });
                grid.MouseLeave += new MouseEventHandler(delegate(object sender, MouseEventArgs e) {
                    Point point = Mouse.GetPosition(grid);
                    if (point.X < 0 || point.Y < 0 || point.X > grid.ActualWidth || point.Y > grid.ActualHeight) joystick.Opacity = 0;
                });
            }
            return grid;
        }

        private void SetJoystickOpacity(Joystick joystick)
        {
            Point point = Mouse.GetPosition(joystick);
            point.Offset(-90, -67.5);
            double distance = Math.Sqrt(point.X * point.X + point.Y * point.Y);
            if (distance < 110) joystick.Opacity = 0.8;
            else if (distance < 150) joystick.Opacity = -0.02 * distance  + 3;
            else joystick.Opacity = 0;
        }

        private TextBlock CreateInfo()
        {
            TextBlock info = new TextBlock();
            info.Background = Brushes.Black;
            info.Foreground = this.FindResource("WhiteTextBrush") as SolidColorBrush;
            info.VerticalAlignment = VerticalAlignment.Bottom;
            info.HorizontalAlignment = HorizontalAlignment.Left;
            info.Margin = new Thickness(10, 0, 10, 0);
            info.SetBinding(TextBlock.TextProperty, new Binding("Camera.TextInfo"));
            DataTrigger trigger = new DataTrigger() { Binding = new Binding("HasInfo"), Value = false };
            trigger.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Hidden));
            Style style = new Style();
            style.Triggers.Add(trigger);
            info.Style = style;
            return info;
        }

        private TextBlock CreateInfoSlot()
        {
            TextBlock info = new TextBlock();
            info.FontSize = 30;
          //  info.Background = Brushes.Black;
            info.Foreground = this.FindResource("WhiteTextBrush") as SolidColorBrush;
            info.VerticalAlignment = VerticalAlignment.Top;
            info.HorizontalAlignment = HorizontalAlignment.Left;
            info.Margin = new Thickness(10, 0, 10, 0);
            info.SetBinding(TextBlock.TextProperty, new Binding("NumberSlot"));
          /*  DataTrigger trigger = new DataTrigger() { Binding = new Binding("HasInfo"), Value = false };
            trigger.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Hidden));
            Style style = new Style();
            style.Triggers.Add(trigger);
            info.Style = style;*/
            return info;
        }

        private Joystick CreateJoystick()
        {
            Joystick joystick = new Joystick();
            joystick.PreviewMouseDown += joystick_MouseDown;
            joystick.PreviewMouseMove += new MouseEventHandler(joystick_MouseMove);
            joystick.PreviewDragEnter += new DragEventHandler(CheckJoystickDrop);
            joystick.PreviewDragOver += new DragEventHandler(CheckJoystickDrop);
            joystick.PreviewDrop += new DragEventHandler(joystick_PreviewDrop);
            joystick.MouseDoubleClick += new MouseButtonEventHandler(joystick_PreviewMouseDoubleClick);
            joystick.Press += new PressEventHandler(joystick_Press);
            joystick.JoystickRemoteControl += new JoystickRemoteControlEventHandler(joystick_JoystickRemoteControl);
            joystick.AllowDrop = true;
            Style style = new Style(typeof(UserControl));
            style.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Hidden));
            DataTrigger triger = new DataTrigger() { Binding = new Binding("Has3D"), Value = true };
            triger.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Visible));
            style.Triggers.Add(triger);
          
            joystick.Style = style;
            joystick.Width = 180;
            joystick.Height = 135;
            joystick.Opacity = 0;
            return joystick;
        }

        void joystick_JoystickRemoteControl(object sender, JoystickRemoteControlEventArgs args)
        {
            Joystick joy = sender as Joystick;
            if (joy == null) return;
            if (args.State.HasFlag(JoystickButtonState.Down) || args.State.HasFlag(JoystickButtonState.Up) || args.State.HasFlag(JoystickButtonState.Left) || args.State.HasFlag(JoystickButtonState.Right) || args.State.HasFlag(JoystickButtonState.ZoomIn) || args.State.HasFlag(JoystickButtonState.ZoomOut))
            {
                this.Dispatcher.BeginInvoke(new Action(delegate()
                {
                    if (Memory.Instance.CurrentProfile == null) return;
                    lock (Memory.Instance.CurrentProfile.locker)
                    {
                        CameraInfo info = joy.DataContext as CameraInfo;
                        var imageCamera = Memory.Instance.CurrentProfile.ImageCameraCollection.Where(x => x.Camera != null).FirstOrDefault(x => x.Camera.ID == args.ID);
                        if (info == null || info.Camera == null || info != imageCamera) return;
                       /* if (info == null || info.Camera == null || info != Memory.Instance.CurrentProfile.ImageCameraCollection[(int)args.ID]) return;*/
                        if (info.Has3D)
                        {
                            joy.Visibility = System.Windows.Visibility.Visible;
                            joy.BeginAnimation(Joystick.OpacityProperty, this.FindResource("joystickAnimation") as AnimationTimeline);
                        }
                    }
                }));
            }
        }

        void joystick_Press(object sender, PressEventArgs args)
        {
            FrameworkElement element = sender as FrameworkElement;
            if (element == null) return;
            CameraInfo info = element.DataContext as CameraInfo;
            if (info == null || info.Camera == null) return;
            CaptureIP.Joystick_Control(info.Camera.ID, args.State);
        }

        void joystick_PreviewMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                RaiseEvent(new RoutedEventArgs(ChangeModeEvent, this));
            }
        }

        private Logo3D CreateLogo3D(bool isSmall)
        {
            Logo3D logo = new Logo3D();
            logo.HorizontalAlignment = HorizontalAlignment.Right;
            logo.VerticalAlignment = VerticalAlignment.Bottom;
            logo.Height = 50;
            logo.Width = 80;
            logo.Opacity = 0.9;
            Style style = new Style(typeof(UserControl));
            style.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Hidden));
            MultiDataTrigger triger = new MultiDataTrigger();
            triger.Conditions.Add(new Condition() { Binding = new Binding("Has3D"), Value = true });
            triger.Conditions.Add(new Condition() { Binding = new Binding("HasInfo"), Value = true });
            triger.Setters.Add(new Setter(UserControl.VisibilityProperty, Visibility.Visible));
            style.Triggers.Add(triger);
            logo.Style = style;
            if (isSmall)
            {
                logo.RenderTransformOrigin = new Point(1, 1);
                logo.RenderTransform = new ScaleTransform(0.5, 0.5);
            }
            return logo;
        }

        private Border CreateDisplay(Image img)
        {
            Grid grid = new Grid();
            grid.Children.Add(img);
            grid.Children.Add(CreateInfo());
            grid.Children.Add(CreateInfoSlot());
            Border border = new Border() { Child = grid, AllowDrop = true, Style = this.FindResource("MonitorImageBorderStyle") as Style };
            border.PreviewDrop += new DragEventHandler(border_PreviewDrop);
            border.PreviewMouseMove += new MouseEventHandler(border_MouseMove);
            border.MouseDown += new MouseButtonEventHandler(border_MouseDown);
            border.PreviewDragEnter += new DragEventHandler(CheckDrop);
            border.PreviewDragOver += new DragEventHandler(CheckDrop);
            border.MouseLeftButtonDown += new MouseButtonEventHandler(border_MouseLeftButtonDown);
            border = new Border() { Child = border, Style = this.FindResource("MonitorBorderStyle") as Style };
            return border;
        }

        void border_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ClickCount == 2)
            {
                RaiseEvent(new RoutedEventArgs(ChangeModeEvent, this));
            }
        }

        private void border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            try
            {
                Memory.Instance.CurrentProfile.IsAuto = true;
                Border border = sender as Border;
                if (border == null) return;
                Grid grid = border.Child as Grid;
                if (grid == null || grid.Children.Count == 0) return;
                Image image = grid.Children[0] as Image;
                if (image == null) return;
                border = border.Parent as Border;
                if (border == null) return;
                foreach (var item in Memory.Instance.CurrentProfile.ImageCameraCollection)
                    if (item.Image == image) { Memory.Instance.CurrentProfile.CurrentImageCamera = item; break; }
            }
            finally { Memory.Instance.CurrentProfile.IsAuto = false; }
        }
        
        private void joystick_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Joystick joystick = sender as Joystick;
            if (joystick == null) return;
            Grid grid = joystick.Parent as Grid;
            if (grid == null) return;
            foreach (FrameworkElement child in grid.Children)
            {
                if (child is Border)
                {
                    border_MouseDown(((Border)child).Child, e);
                }
            }
        }

        private void CheckJoystickDrop(object sender, DragEventArgs e)
        {
            Joystick joystick = sender as Joystick;
            if (joystick == null) return;
            Grid grid = joystick.Parent as Grid;
            if (grid == null) return;
            foreach (FrameworkElement child in grid.Children)
            {
                if (child is Border)
                {
                    CheckDrop(child, e);
                    break;
                }
            }
        }

        private void CheckDrop(object sender, DragEventArgs e)
        {
            bool isCorrect = false;
            Border borderSrc = sender as Border;
            if (borderSrc != null)
            {
                borderSrc = borderSrc.Parent as Border;
                if (borderSrc != null)
                {
                    if (e.Effects == DragDropEffects.Move)
                    {
                        Border borderDist = e.Data.GetData(typeof(Border)) as Border;
                        if (borderDist != null && ((CameraInfo)(borderSrc.DataContext)).Image != null && ((CameraInfo)(borderDist.DataContext)).Image != null && borderDist != borderSrc && (bigPanel.Children.Contains((Panel)borderDist.Parent) || smallPanel.Children.Contains((Panel)borderDist.Parent)))
                            isCorrect = true;
                    }
                }
            }
            if (!isCorrect) e.Effects = DragDropEffects.None;
            e.Handled = true;
        }

        private void border_MouseMove(object sender, MouseEventArgs e)
        {
            if (currentpoint == e.GetPosition(this)) return;
            else currentpoint = e.GetPosition(this);
            Border border = sender as Border;
            if (border == null) return;
            if (e.LeftButton == MouseButtonState.Pressed)
                if (border.Parent != null) DragDrop.DoDragDrop(border, border.Parent, DragDropEffects.Move);
        }

        private void joystick_MouseMove(object sender, MouseEventArgs e)
        {
            Joystick joystick = sender as Joystick;
            if (joystick == null) return;
            Grid grid = joystick.Parent as Grid;
            if (grid == null) return;
            foreach (FrameworkElement item in grid.Children)
            {
                if (item is Border)
                {
                    border_MouseMove(((Border)item).Child, e);
                    break;
                }
            }
        }

        void joystick_PreviewDrop(object sender, DragEventArgs e)
        {
            Joystick joystick = sender as Joystick;
            if (joystick == null) return;
            Grid grid = joystick.Parent as Grid;
            if (grid == null) return;
            foreach (FrameworkElement child in grid.Children)
            {
                if (child is Border)
                {
                    border_PreviewDrop(child, e);
                    break;
                }
            }
        }

        private void border_PreviewDrop(object sender, DragEventArgs e)
        {
            if (Memory.Instance.CurrentProfile == null) return;
            Border borderDist = (Border)sender;
            Border borderSrc = (Border)e.Data.GetData(typeof(Border));
            Memory memory = Memory.Instance;
            if (memory == null) return;

            CameraInfo first, second;
            FindDropElemens(out first, out second, memory.CurrentProfile.ImageCameraCollection.First(x => x.Image == ((Grid)((Border)borderSrc.Child).Child).Children[0]), memory.CurrentProfile.ImageCameraCollection.First(x => x.Image == ((Grid)borderDist.Child).Children[0]));
            if (first == null || second == null || first == second) return;
            int firstIndex = memory.CurrentProfile.ImageCameraCollection.IndexOf(first);
            int secondIndex = memory.CurrentProfile.ImageCameraCollection.IndexOf(second);
            memory.CurrentProfile.ImageCameraCollection.Move(secondIndex, firstIndex);
            memory.CurrentProfile.ImageCameraCollection.Move(firstIndex + 1, secondIndex);
            UpdateMonitors();
          
        }

        private void FindDropElemens(out CameraInfo first, out CameraInfo second, CameraInfo src, CameraInfo dst)
        {
            first = second = null;
            if (Memory.Instance.CurrentProfile == null) return;
            foreach (var item in Memory.Instance.CurrentProfile.ImageCameraCollection)
            {
                if (item == src || item == dst)
                {
                    if (first == null) first = item;
                    else if (second == null)
                    {
                        second = item;
                        break;
                    }
                }
            }
        }

        private void UpdateMonitors()
        {
            UserControl_DataContextChanged(this, new DependencyPropertyChangedEventArgs(UserControl.DataContextProperty, null, null));
            UserControl_DataContextChanged(this, new DependencyPropertyChangedEventArgs(UserControl.DataContextProperty, null, new object()));
        }

        public event EventHandler ChangeMode
        {
            add
            {
                this.AddHandler(ChangeModeEvent, value);
            }
            remove
            {
                this.RemoveHandler(ChangeModeEvent, value);
            }
        }

       
        private void Grid_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            currentpoint = e.GetPosition(this);
        }

       

        private void TabItem_MouseDown(object sender, MouseButtonEventArgs e)
        {

        }
    }
}
