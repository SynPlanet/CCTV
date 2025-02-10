using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Windows.Threading;
using System.Collections.Specialized;
using System.Collections.ObjectModel;
using System.Reflection;
using System.Diagnostics;

namespace Cam
{
    class Profile : INotifyPropertyChanged
    {
        private EmulationCameraManager cameraManager;
        private CameraInfo currentImageCamera = null;
        private string pathToSave = string.Empty;
        private string pathToPlay = string.Empty;
        private string pathToSaveAudio = string.Empty;
        private string pathToPlayAudio = string.Empty;
        private int port = 0;
        private int cameraSlot = 0;
        private bool isArchiveStartup = false;
        private bool audioEnable = false;
        private bool isFill = false;
        private bool isPlay = false;

        public object locker = new object();
        public bool IsCanSetCurrentCamera = true;

        public Profile(string name, bool canUpdate)
        {
            this.Name = name;
            ImageCameraCollection = new ObservableCollection<CameraInfo>();
            AudioDeviceCollection = new ObservableCollection<AudioDevice>();
            cameraManager = new EmulationCameraManager();
            ImageCameraCollectionBackup = new List<CameraInfo>();
            if (canUpdate) ImageCameraCollection.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(ImageCameraCollection_CollectionChanged);
            if (canUpdate) AudioDeviceCollection.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(AudioDeviceCollection_CollectionChanged);
        }

        public Profile(string name, bool canUpdate, Profile parent)
        {
            this.Name = name;
            ImageCameraCollection = new ObservableCollection<CameraInfo>(parent.ImageCameraCollection);
            AudioDeviceCollection = new ObservableCollection<AudioDevice>(parent.AudioDeviceCollection);
            cameraManager = parent.cameraManager;
            if (canUpdate) ImageCameraCollection.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(ImageCameraCollection_CollectionChanged);
            if (canUpdate) AudioDeviceCollection.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(AudioDeviceCollection_CollectionChanged);
            PathToSave = parent.PathToSave;
            PathToPlay = parent.PathToPlay;
            PathToSaveAudio = parent.PathToSaveAudio;
            PathToPlayAudio = parent.PathToPlayAudio;
            RemoteControlPort = parent.RemoteControlPort;
            CurrentImageCamera = parent.CurrentImageCamera;
            CameraSlot = parent.CameraSlot;
            IsArchiveStartup = parent.IsArchiveStartup;
            AudioEnable = parent.AudioEnable;
        }

        public void Fill(Profile profile)
        {
            if (profile == null) return;
            isFill = true;
            lock (locker)
            {
                ImageCameraCollection.Clear();
                AudioDeviceCollection.Clear();
                foreach (var item in profile.AudioDeviceCollection) AudioDeviceCollection.Add(item.Clone());
                foreach (var item in profile.ImageCameraCollection) ImageCameraCollection.Add(item.Clone());
                if (ImageCameraCollection.Count == 0) CurrentImageCamera = null;
                else CurrentImageCamera = ImageCameraCollection[0];
            }
           // cameraManager = profile.cameraManager;
            PathToSave = profile.PathToSave;
            PathToPlay = profile.PathToPlay;
            PathToSaveAudio = profile.PathToSaveAudio;
            PathToPlayAudio = profile.PathToPlayAudio;
            RemoteControlPort = profile.RemoteControlPort;
            CameraSlot = profile.CameraSlot;
            IsArchiveStartup = profile.IsArchiveStartup;
            AudioEnable = profile.AudioEnable;
            isFill = false;
        }

        public void FillAudioDevice(Profile profile)
        {
            if (profile == null) return;
            isFill = true;
            lock (locker)
            {
                AudioDeviceCollection.Clear();
                foreach (var item in profile.AudioDeviceCollection) AudioDeviceCollection.Add(item.Clone());
            }
            isFill = false;
        }

        public string Name { get; private set; }

        void AudioDeviceCollection_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add)
                foreach (AudioDevice item in e.NewItems)
                {
                    item.PropertyChanged += new PropertyChangedEventHandler(AudioDeviceItem_PropertyChanged);
                }
            else return;
        }

        void AudioDeviceItem_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "IsEnable")
            {
                var profile = Memory.Instance.ProfileArray.Find(x => x.Name == Name);
                if (profile == null) return;
                profile.FillAudioDevice(this);
            }
        }

        void ImageCameraCollection_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            try
            {
                if (e.Action == NotifyCollectionChangedAction.Add)
                    foreach (CameraInfo item in e.NewItems)
                    {
                        cameraManager.AddCamera(item);
                    }
                else if (e.Action == NotifyCollectionChangedAction.Reset) cameraManager.Clear();
                else if (e.Action == NotifyCollectionChangedAction.Remove) cameraManager.Remove(((CameraInfo)e.OldItems[0]).Camera);

                else return;
            }
            finally
            {
                lock (locker)
                {
                    for (int i = 0; i < ImageCameraCollection.Count; i++)
                    {
                        if (ImageCameraCollection[i] != null)
                            ImageCameraCollection[i].NumberSlot = i + 1;
                    }
                }
            }
        }

        public void UpdateFrameCameraCollection()
        {
            cameraManager.UpdateBuffer();
            cameraManager.Update();
        }

        public void UpdatePlayerStatus(uint id, PlayerState status)
        {
            CameraInfo camera = ImageCameraCollection.FirstOrDefault(x => x.Camera != null && x.Camera.ID == id);
            if (camera == null) return;
            camera.UpdateRecStatus(status == PlayerState.WRITE);
        }

        public void Joy_Control(uint id, JoystickButtonState status)
        {
            if (!Memory.Instance.HasJoystick) return;
            Dispatcher.CurrentDispatcher.Invoke(new Action(delegate()
            {
                lock (locker)
                {
                    var imageCamera = ImageCameraCollection.Where(x => x.Camera != null).FirstOrDefault(x => x.Camera.ID == id);
                    if (imageCamera == null) return;
                    imageCamera.JoystickButtonState = status;
                    CurrentImageCamera = imageCamera;
                }
            //    if (ImageCameraCollection.Count <= id || ImageCameraCollection[(int)id] == null || ImageCameraCollection[(int)id].Camera == null) return;
            //    ImageCameraCollection[(int)id].JoystickButtonState = status;
            //    CurrentImageCamera = ImageCameraCollection[(int)id];
                if (Joystick != null) Joystick(id, status);
            }));
        }

        public event Joy_Control Joystick;

        public bool IsAuto { get; set; }

        public bool IsAdmin
        {
            get { return Memory.Instance.isAdmin; }
            set
            {
                Memory.Instance.isAdmin = value;
                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("IsAdmin"));
            }
        }

        public bool IsArchive
        {
            get { return Memory.Instance.isArchive.HasValue ? Memory.Instance.isArchive.Value : false; }
            set
            {
                if (Memory.Instance.isArchive != value)
                {
                    Memory.Instance.isArchive = value;
                    CaptureIP.SetTypeInterface(value ? PlayerState.PLAY : PlayerState.WRITE);
                    if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("IsArchive"));
                }
            }
        }

        public bool IsArchiveStartup
        {
            get { return isArchiveStartup; }
            set
            {
                isArchiveStartup = value;
                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("IsArchiveStartup"));
            }
        }

        public bool AudioEnable
        {
            get { return audioEnable; }
            set
            {
                audioEnable = value;
                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("AudioEnable"));

                if (!isFill && this == Memory.Instance.CurrentProfile) 
                {
                    if (audioEnable)
                    {
                        Memory.Instance.AudioWindow = new Audio() { DataContext = Memory.Instance };
                        Memory.Instance.AudioWindow.Show();
                    }
                    else
                    {
                        var temp = Memory.Instance.AudioWindow;
                        Memory.Instance.AudioWindow = null;
                        if (temp != null) temp.Close();
                    }
                }
            }
        }

        public List<CameraInfo> ImageCameraCollectionBackup { get; private set; }

        public CameraInfo CurrentImageCameraBackup { get; set; }

        public ObservableCollection<CameraInfo> ImageCameraCollection { get; private set; }

        public ObservableCollection<AudioDevice> AudioDeviceCollection { get; private set; }

        public void UpdateCurrentImageCamera()
        {
            var temp = currentImageCamera;
            currentImageCamera = null;
            CurrentImageCamera = temp;
        }

        public CameraInfo CurrentImageCamera
        {
            get { return currentImageCamera; }
            set
            {
                if (currentImageCamera == value) return;
                currentImageCamera = value;
                if (IsCanSetCurrentCamera)
                {
                    Dispatcher.CurrentDispatcher.Invoke(new Action(delegate()
                    {
                        for (int i = 0; i < ImageCameraCollection.Count; i++)
                        {
                            ImageCameraCollection[i].IsChecked = ImageCameraCollection[i] == currentImageCamera;
                            if (ImageCameraCollection[i].IsChecked && ImageCameraCollection[i].Camera != null) CaptureIP.SetCurrentCamera(ImageCameraCollection[i].Camera.ID);
                        }

                    }));
                }

                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("CurrentImageCamera"));
            }
        }

        public string PathToSave
        {
            get
            {
                return pathToSave;
            }
            set
            {
                if (pathToSave == value) return;
                pathToSave = value;
                CaptureIP.SetPathsVideo(string.IsNullOrWhiteSpace(pathToSave) ? Environment.CurrentDirectory : pathToSave, PathToPlay);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("PathToSave"));
            }
        }

        public string PathToPlay
        {
            get
            {
                return pathToPlay;
            }
            set
            {
                if (pathToPlay == value) return;
                pathToPlay = value;
              //  CaptureIP.SetPathsAudio(string.IsNullOrWhiteSpace(PathToSaveAudio) ? Environment.CurrentDirectory : PathToSave, PathToPlayAudio);
              //  CaptureIP.SetPathRec(pathToSave);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("PathToPlay"));
            }
        }

        public string PathToSaveAudio
        {
            get
            {
                return pathToSaveAudio;
            }
            set
            {
                if (pathToSaveAudio == value) return;
                pathToSaveAudio = value;
              //  CaptureIP.SetPathRec(string.IsNullOrWhiteSpace(pathToSaveAudio) ? Environment.CurrentDirectory : pathToSaveAudio);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("PathToSaveAudio"));
            }
        }

        public string PathToPlayAudio
        {
            get
            {
                return pathToPlayAudio;
            }
            set
            {
                if (pathToPlayAudio == value) return;
                pathToPlayAudio = value;
                //  CaptureIP.SetPathRec(pathToSave);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("PathToPlayAudio"));
            }
        }

        public int RemoteControlPort
        {
            get
            {
                return port;
            }
            set
            {
                if (value < 0 || value > 65535) throw new ArgumentOutOfRangeException();
                if (port == value) return;
                port = value;
                CaptureIP.SetPort_VideoControl(port);
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("RemoteControlPort"));
            }
        }

        public bool? IsRec
        {
            get
            {
                if (ImageCameraCollection.Count == 0) return false;
                int count = ImageCameraCollection.Count(x => x.IsRec == true);
                if (count == ImageCameraCollection.Count) return true;
                else if (count == 0) return false;
                else return null;
            }
            set
            {
                if (value == null) return;
                foreach (var item in ImageCameraCollection) item.IsRec = value.Value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsRec"));
            }
        }

        public int CameraSlot
        {
            get { return cameraSlot; }
            set
            {
                cameraSlot = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("CameraSlot"));
            }
        }

        public bool IsPlay
        {
            get { return isPlay; }
            set
            {
                isPlay = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsPlay"));
            }
        }


        public void UpdateRec()
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs("IsRec"));
        }

        public event PropertyChangedEventHandler PropertyChanged;
        public override string ToString()
        {
            return Name;
        }
    }
}
