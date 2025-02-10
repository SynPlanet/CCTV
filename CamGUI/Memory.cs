using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Windows.Controls;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows.Threading;
using System.Runtime.InteropServices;

namespace Cam
{
    partial class Memory : INotifyPropertyChanged
    {
        private readonly static Memory instance = new Memory();
        public static Memory Instance { get { return instance; } }

        private Profile currentProfile = null;

        private GCHandle updatePlayerStatusHandle;
        private GCHandle joy_IsConnectHandle;
        private GCHandle joy_ControlHandle;

        public bool isAdmin = false;
        public bool? isArchive = null;
        private bool isAudioDeviceEnable = false;

        private Memory()
        {
            IsAudioDeviceEnable = true;
            ContentArray = new List<Tuple<string, string>>();
            Password = string.Empty;
            ProfileCollection = new ObservableCollection<Profile>();
            ProfileArray = new List<Profile>();

            PlayerStatus method = new PlayerStatus(UpdatePlayerStatus);
            updatePlayerStatusHandle = GCHandle.Alloc(method);
            CaptureIP.CallBackFunc_PlayerState(method);

            Joy_IsConnect method1 = new Joy_IsConnect(Joy_IsConnect);
            joy_IsConnectHandle = GCHandle.Alloc(method1);

            Joy_Control method2 = new Joy_Control(Joy_Control);
            joy_ControlHandle = GCHandle.Alloc(method2);

            CaptureIP.CallBackFunc_JoyControl(method1, method2);

        }

        void UpdatePlayerStatus(uint id, PlayerState status)
        {
            if (status == PlayerState.EXIT)
            {
                App.Current.Dispatcher.BeginInvoke(new Action(delegate() { App.Current.Shutdown(); }));
                return;
            }
            if (status == PlayerState.PLAY || status == PlayerState.WRITE) IsAudioDeviceEnable = false;
            else IsAudioDeviceEnable = true;
          
            if (currentProfile != null) currentProfile.UpdatePlayerStatus(id, status);
        }

        void Joy_IsConnect(bool hasJoy)
        {
            HasJoystick = hasJoy;
        }

        void Joy_Control(uint id, JoystickButtonState status)
        {
            if (currentProfile != null) currentProfile.Joy_Control(id, status);
        }


        public ObservableCollection<Profile> ProfileCollection { get; private set; }

        public List<Profile> ProfileArray { get; set; }

        public void Commit()
        {
            var profile = ProfileArray.Find(x => x.Name == currentProfile.Name);
            if (profile == null) return;
            profile.Fill(currentProfile);
        }

        public Profile CurrentProfile
        {
            get { return currentProfile; }
            set
            {
                if (value == null) return;
                if (currentProfile != value)
                {
                    IsAudioDeviceEnable = true;
                    if (currentProfile != null)
                    {
                        currentProfile.IsCanSetCurrentCamera = false;
                        currentProfile.Fill(ProfileArray.Find(x => x.Name == currentProfile.Name));
                        currentProfile.IsCanSetCurrentCamera = true;
                    }
                    currentProfile = value;
                    if (currentProfile.AudioEnable)
                    {
                        if (AudioWindow == null) AudioWindow = new Audio();
                        AudioWindow.DataContext = Memory.instance;
                        AudioWindow.Show();
                        Dispatcher.CurrentDispatcher.BeginInvoke(new Action(delegate() {
                            App.Current.MainWindow.Topmost = true;
                            Dispatcher.CurrentDispatcher.BeginInvoke(new Action(delegate()
                            {
                                App.Current.MainWindow.Topmost = false;

                            }));

                        }));
                    }
                    else
                    {
                        var temp = AudioWindow;
                        AudioWindow = null;
                        if (temp != null) temp.Close();
                    }
                    currentProfile.UpdateCurrentImageCamera();
                    CaptureIP.SetPathsVideo(string.IsNullOrWhiteSpace(value.PathToSave) ? Environment.CurrentDirectory : value.PathToSave, value.PathToPlay);
                    CaptureIP.SetPathsAudio(string.IsNullOrWhiteSpace(value.PathToSaveAudio) ? Environment.CurrentDirectory : value.PathToSaveAudio, value.PathToPlayAudio);
                    foreach (var item in value.AudioDeviceCollection)
                        CaptureIP.SetAudioDevice(item.Number, item.IsEnable.HasValue ? item.IsEnable.Value : false);
                }
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("CurrentProfile"));
            }
        }

        public string NewProfile
        {
            get 
            {
                if (currentProfile == null) return string.Empty;
                return currentProfile.Name;
            }
            set
            {
                if (string.IsNullOrEmpty(value)) return;
                var profileTmp = ProfileCollection.FirstOrDefault(x => x.Name == value);
                if (profileTmp != null) return;
                Profile profile = new Profile(value, true, currentProfile);
                ProfileCollection.Add(profile);
                ProfileArray.Add(new Profile(value, false));
                CurrentProfile = profile;

            }
        }

        public bool IsAudioDeviceEnable
        {
            get { return isAudioDeviceEnable; }
            set
            {
                isAudioDeviceEnable = value;
                if (PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs("IsAudioDeviceEnable"));
            }
        }

        public string CurrentProfileName { get; set; }

        public bool HasJoystick { get; private set; }

        public bool IsRelease { get; set; }

        public int RunToWindow { get; private set; }

        public int TimeMinRec { get; private set; }

        public string Password { get; private set; }

        public int InitPTZFlag { get; private set; }

        public bool IsManual { get; private set; }

        public Audio AudioWindow { get; set; }

        public List<Tuple<string, string>> ContentArray { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
