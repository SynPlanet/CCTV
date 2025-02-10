using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows.Media;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Cam
{
    class CameraInfo : INotifyPropertyChanged
    {
        bool has3d = false;
        bool hasInfo = false;
        string content = string.Empty;
        string pathToSave = string.Empty;
        string name;
        bool isChecked;
        bool isRec;
        string address = string.Empty;
        string userName = string.Empty;
        string password = string.Empty;
        ICamera camera;
        public bool hasContent = false;
        int numberSlot = 0;
        JoystickButtonState joystickButtonState = JoystickButtonState.None;
        static Random rnd = new Random();
        int flagReconnect = 0;

        public CameraInfo()
        {
        }

        public CameraInfo(bool has3D)
        {
            Image = new Image() { StretchDirection = StretchDirection.Both, Stretch = Stretch.Uniform, Tag = rnd.Next() };
            Has3D = has3D;
            IsEmulate = true;
            Path = string.Empty;
        }

        public CameraInfo(string path, bool has3D, bool isEmulate)
        {
            Image = new Image() { StretchDirection = StretchDirection.Both, Stretch = Stretch.Uniform, Tag = rnd.Next() };
            Path = path;
            Has3D = has3D;
            IsEmulate = isEmulate;
        }

        public CameraInfo(string path, bool has3D) 
            : this(path, has3D, false) { }

        public string Path { get; private set; }

        public bool IsPlayer { get; set; }

        public int PlayerIndex { get; set; }

        public ICamera Camera
        {
            get { return camera; }
            set
            {
              /*  if (camera == null && value != null)
                {
                    has3d = CaptureIP.GetCamera_360(value.ID);
                    if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("Has3D"));
                }*/
                camera = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Camera"));
            }
        }

        
        public Image Image { get; private set; }

        public bool IsEmulate { get; private set; }

        public string Name
        {
            get { return name; }
            set
            {
                if (name == value) return;
                name = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Name"));
            }
        }

        public bool IsChecked
        {
            get { return isChecked; }
            set
            {
                if (isChecked == value) return;
                isChecked = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsChecked"));
            }
        }

        public bool HasInfo
        {
            get { return hasInfo; }
            set
            {
                if (hasInfo == value) return;
                hasInfo = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("HasInfo"));
            }
        }

        public JoystickButtonState JoystickButtonState
        {
            get { return joystickButtonState; }
            set
            {
                if (joystickButtonState == value) return;
                joystickButtonState = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("JoystickButtonState"));
            }
        }

        public bool IsRec
        {
            get { return isRec; }
            set
            {
                if (isRec != value)
                {
                    if (value) CaptureIP.WriteVideoStream(Camera.ID, PlayerState.WRITE);
                    else CaptureIP.WriteVideoStream(Camera.ID, PlayerState.STOP);
                }
            /*    isRec = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsRec"));
                Memory.Instance.UpdateRec();*/
            }
        }

        public void UpdateRecStatus(bool isRec)
        {
            if (this.isRec != isRec)
            {
                this.isRec = isRec;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsRec"));
                Memory.Instance.CurrentProfile.UpdateRec();
            }
        }

        public int NumberSlot 
        {
            get { return numberSlot; }
            set
            {
                if (numberSlot == value) return;
                numberSlot = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("NumberSlot"));
            }
        }

        public string Content
        {
            get { return content; }
            set
            {
                if (content == value) return;
                content = value;
                //Camera.Update(content);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Content"));
            }
        }


        public bool Has3D
        {
            get { return has3d; }
            set
            {
                if (has3d == value) return;
                has3d = value;
                if (Camera != null) CaptureIP.SetCamera_360(Camera.ID, value);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Has3D"));
            }
        }

        private void UpdateContent()
        {
            if (camera == null) return;
            hasContent = false;
            foreach (var item in Memory.Instance.ContentArray)
            {
                hasContent = Camera.Update(item.Item1, item.Item2);
                if (hasContent) break;
            }
            if (Memory.Instance.ContentArray.Count == 0)
            {
                hasContent = Camera.Update(string.Empty, string.Empty);
            }
            //временно убрал
            //has3d = CaptureIP.GetCamera_360(camera.ID);
            if (hasContent) CaptureIP.SetCamera_360(camera.ID, has3d); // временно вставил
            App.Current.Dispatcher.Invoke(new Action(delegate ()
            {
                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("Has3D"));
            }));
        }

        public void Reconnect()
        {
            if (System.Threading.Interlocked.CompareExchange(ref flagReconnect, 1, 0) == 1)
                return;
            try
            {
                if (hasContent)
                    return;
                MainWindow.Writer.WriteLine("{0} Start UpdateContent", camera.ID);
                    UpdateContent();
                MainWindow.Writer.WriteLine("{0} End UpdateContent", camera.ID);
            }
            finally
            {
                System.Threading.Interlocked.Exchange(ref flagReconnect, 0);
            }
        }

        public string Address
        {
            get { return address; }
            set
            {
                address = value;
               // UpdateContent();
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Address"));
            }
        }

        public string UserName
        {
            get { return userName; }
            set
            {
                if (userName == value) return;
                userName = value;
              //  UpdateContent();
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("UserName"));
            }
        }

        public string Password
        {
            get { return password; }
            set
            {
                if (password == value) return;
                password = value;
             //   UpdateContent();
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("Password"));
            }
        }

        public CameraInfo Clone()
        {
            var clone = new CameraInfo(Has3D);
            clone.Password = Password;
            clone.UserName = UserName;
            clone.Address = Address;
            clone.Content = Content;
            clone.NumberSlot = NumberSlot;
            clone.HasInfo = HasInfo;
            clone.IsChecked = IsChecked;
            clone.IsPlayer = IsPlayer;
            clone.PlayerIndex = PlayerIndex;
            clone.Name = Name;
            return clone;
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
