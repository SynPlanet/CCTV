using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Windows.Forms;

namespace Cam
{
    partial  class Memory
    {
        #region Tags
        const string ConfigurationTag = "Configuration";
        const string AdminTag = "Admin";
        const string PathToSaveTag = "PathToSaveVideo";
        const string PathToSaveAudioTag = "PathToSaveAudio";
        const string CameraArrayTag = "CameraArray";
        const string CameraTag = "Camera";
        const string ContentArrayTag = "ContentArray";
        const string ContentTag = "Content";
        const string RemoteControlTag = "RemoteControl";
        const string ProfilesTag = "Profiles";
        const string ProfileTag = "Profile";
        const string AudioEnableTag = "AudioEnable";
        const string ArchiveStartupTag = "ArchiveStartup";
        const string PathToPlayTag = "PathToPlayVideo";
        const string PathToPlayAudioTag = "PathToPlayAudio";
        const string SlotTag = "Slot";
        const string StartupWindowTag = "StartupWindow";
        const string TimeRecMinTag = "TimeRecMin";
        const string IsManualTag = "IsManual";
        const string AudioDeviceTag = "AudioDevice";
        const string DeviceTag = "Device";
        #endregion

        #region Attributes
        const string HeaderAttribute = "Header";
        const string TailAttribute = "Tail";
        const string NameAttribute = "Name";
        const string ValueAttribute = "Value";
        const string NumberAttribute = "Number";
        const string HasInfoAttribute = "HasInfo";
        const string Has360Attribute = "Has360";
        const string AddressAttribute = "Address";
        const string UserNameAttribute = "UserName";
        const string PasswordAttribute = "Password";
        const string IPAttribute = "IP";
        const string PortAttribute = "Port";
        const string CurrentProfileAttribute = "CurrentProfile";
        const string InitPTZ = "InitPTZ";
        #endregion

        public void Load(XmlTextReader xml)
        {
            xml.WhitespaceHandling = WhitespaceHandling.None;
            if (!ReadConfiguration(xml)) return;
            for (string tag = GetConfigurationTag(xml, 1); tag != null; tag = GetConfigurationTag(xml, 1))
            {
            start:
                if (tag == AdminTag) ReadAdmin(xml);
                else if (tag == ContentArrayTag)
                {
                    ReadContentArray(xml);
                    if (UpdateTag(xml, ref tag)) goto start;
                }
             /*   else if (tag == PathToSaveTag)
                {
                    ReadPathToSave(xml);
                    if (UpdateTag(xml, ref tag)) goto start;
                }*/
                else if (tag == ProfilesTag)
                {
                    ReadProfiles(xml);
                    if (UpdateTag(xml, ref tag)) goto start;
                }
                else if (tag == StartupWindowTag)
                {
                    ReadWindow(xml);
                }
                else if (tag == IsManualTag)
                {
                    ReadIsManual(xml);
                }
                else if (tag == InitPTZ)
                {
                    ReadInitPTZ(xml);
                }
                else if (tag == TimeRecMinTag)
                {
                    TimeRecMinRead(xml);
                }
               // else if (tag == RemoteControlTag) ReadRemoteControlTag(xml);
            }
         //   if (ImageCameraCollection.Count > 0) CurrentImageCamera = ImageCameraCollection[0];
        }

        private void ReadIsManual(XmlTextReader xml)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try { IsManual = bool.Parse(xml.Value); }
                catch { }
            }
        }

        private void ReadInitPTZ(XmlTextReader xml)
        {

            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try { InitPTZFlag = int.Parse(xml.Value); }
                catch { }
            }
        }

        private void TimeRecMinRead(XmlTextReader xml)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try { TimeMinRec = int.Parse(xml.Value); }
                catch { }
            }
        }

        private void ReadProfiles(XmlTextReader xml)
        {
            string currentProfile = xml.GetAttributeExt(CurrentProfileAttribute);
            for (string tag = GetConfigurationTag(xml, 2); tag != null; tag = GetConfigurationTag(xml, 2))
            {
                if (tag == ProfileTag)
                {
                    ReadProfile(xml);
                }
            }
            CurrentProfileName = currentProfile;
        }

        private void ReadProfile(XmlTextReader xml)
        {
            string name = xml.GetAttribute(NameAttribute);
            if (name == string.Empty) return;
            Profile profile = new Profile(name, false);
            for (string tag = GetConfigurationTag(xml, 3); tag != null; tag = GetConfigurationTag(xml, 3))
            {
            start:
                if (tag == PathToSaveTag)
                {
                    ReadPathToSave(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == PathToPlayTag)
                {
                    ReadPathToPlay(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == ArchiveStartupTag)
                {
                    ReadArchiveStartup(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                if (tag == PathToSaveAudioTag)
                {
                    ReadPathToSaveAudio(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == PathToPlayAudioTag)
                {
                    ReadPathToPlayAudio(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == AudioEnableTag)
                {
                    ReadAudioStartup(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == SlotTag)
                {
                    ReadSlot(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == RemoteControlTag)
                {
                    ReadRemoteControlTag(xml, profile);
                }
                else if (tag == CameraArrayTag)
                {
                    ReadCameraArray(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
                else if (tag == AudioDeviceTag)
                {
                    ReadAudioDevice(xml, profile);
                    if (UpdateTag(xml, ref tag, 3)) goto start;
                }
            }
            ProfileArray.Add(profile);
            Profile tmp = new Profile(name, true);
            tmp.Fill(profile);
            ProfileCollection.Add(tmp);
        }

        private bool UpdateTag(XmlTextReader xml, ref string tag, int depth)
        {
            if (xml.IsEmptyElement) return false;
            if (xml.Depth == depth && xml.NodeType == XmlNodeType.Element)
            {
                tag = xml.Name;
                return true;
            }
            return false;
        }

        private bool UpdateTag(XmlTextReader xml, ref string tag)
        {
            if (xml.IsEmptyElement) return false;
            if (xml.Depth == 1 && xml.NodeType == XmlNodeType.Element)
            {
                tag = xml.Name;
                return true;
            }
            return false;
        }

        private void ReadRemoteControlTag(XmlTextReader xml, Profile profile)
        {
            try
            {
                profile.RemoteControlPort = int.Parse(xml.GetAttributeExt(PortAttribute));
            }
            catch 
            {
                
            }
        }

        private void ReadAudioDevice(XmlTextReader xml, Profile profile)
        {
            for (string tag = GetConfigurationTag(xml, 4); tag != null; tag = GetConfigurationTag(xml, 4))
            {
                if (tag == DeviceTag)
                {
                    try
                    {
                        string name = xml.GetAttributeExt(NameAttribute);
                        bool value;
                        int number;
                        if (string.IsNullOrEmpty(name) || string.IsNullOrWhiteSpace(name)) throw new Exception(Local.Message.Memory_XML_Message0);
                        if (profile.AudioDeviceCollection.FirstOrDefault(x => x.Name == name) != null) throw new Exception(Local.Message.Memory_XML_Message1);
                        if (bool.TryParse(xml.GetAttributeExt(ValueAttribute), out value) == false) throw new Exception(Local.Message.Memory_XML_Message2);
                        if (int.TryParse(xml.GetAttributeExt(NumberAttribute), out number) == false) throw new Exception(Local.Message.Memory_XML_Message3);
                        if (profile.AudioDeviceCollection.FirstOrDefault(x => x.Number == number) != null) throw new Exception(Local.Message.Memory_XML_Message4);
                        AudioDevice device = new AudioDevice(name, number);
                        device.IsEnable = value;
                        profile.AudioDeviceCollection.Add(device);
                    }
                    catch (Exception ex) { MessageBox.Show(ex.Message, "", MessageBoxButtons.OK, MessageBoxIcon.Error); App.Current.Shutdown(); }
                }
            }
        }

        private void ReadCameraArray(XmlTextReader xml, Profile profile)
        {
            for (string tag = GetConfigurationTag(xml, 4); tag != null; tag = GetConfigurationTag(xml, 4))
            {
                if (tag == CameraTag)
                {
                    CameraInfo camera = new CameraInfo(bool.Parse(xml.GetAttributeExt(Has360Attribute)));
                    camera.HasInfo = bool.Parse(xml.GetAttributeExt(HasInfoAttribute));
                    camera.Name = xml.GetAttributeExt(NameAttribute);
                    camera.UserName = xml.GetAttributeExt(UserNameAttribute);
                    camera.Password = xml.GetAttributeExt(PasswordAttribute);
                    camera.Address = xml.GetAttributeExt(AddressAttribute);
                    profile.ImageCameraCollection.Add(camera);
                }
            }
        }

        private void ReadPathToSave(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
                profile.PathToSave = xml.Value;
           
        }

        private void ReadPathToPlay(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
                profile.PathToPlay = xml.Value;

        }

        private void ReadPathToSaveAudio(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
                profile.PathToSaveAudio = xml.Value;

        }

        private void ReadPathToPlayAudio(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
                profile.PathToPlayAudio = xml.Value;

        }

        private void ReadWindow(XmlTextReader xml)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try { RunToWindow = int.Parse(xml.Value); }
                catch { }
            }

        }

        private void ReadArchiveStartup(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try
                {
                    profile.IsArchiveStartup = bool.Parse(xml.Value);
                }
                catch { }
            }

        }

        private void ReadAudioStartup(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try
                {
                    profile.AudioEnable = bool.Parse(xml.Value);
                }
                catch { }
            }

        }

        private void ReadSlot(XmlTextReader xml, Profile profile)
        {
            if (xml.Read() && xml.NodeType == XmlNodeType.Text)
            {
                try
                {
                    profile.CameraSlot = int.Parse(xml.Value);
                    if (profile.CameraSlot < 0 || profile.CameraSlot > 2) profile.CameraSlot = 0;
                }
                catch { profile.CameraSlot = 0; }
            }

        }


        private void ReadContentArray(XmlTextReader xml)
        {
            for (string tag = GetConfigurationTag(xml, 2); tag != null; tag = GetConfigurationTag(xml, 2))
            {
                if (tag == ContentTag) ContentArray.Add(new Tuple<string, string>(xml.GetAttributeExt(HeaderAttribute), xml.GetAttributeExt(TailAttribute)));
            }
        }

        private void ReadAdmin(XmlTextReader xml)
        {
            Password = xml.GetAttributeExt(PasswordAttribute);
        }
        
        private bool ReadConfiguration(XmlTextReader xml)
        {
            xml.Read();
            if (xml.Depth == 0 && xml.NodeType == XmlNodeType.Element && xml.Name == ConfigurationTag) return true;
            else return false;
        }

        private string GetConfigurationTag(XmlTextReader xml, int depth)
        {
            string value = null;
            while (xml.Read() && xml.Depth >= depth)
                if (xml.Depth == depth && xml.NodeType == XmlNodeType.Element) return xml.Name;
            return value;
        }

        public void Save(XmlTextWriter xml)
        {
            xml.WriteStartElement(ConfigurationTag);
            AdminSave(xml);
            InitPTZSave(xml);
            IsManualSave(xml);
            ContentSave(xml);
            WindowSave(xml);
            TimeMinRecSave(xml);
            ProfilesSave(xml);
            xml.WriteEndElement();
            xml.Flush();
        }

        private void IsManualSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(IsManualTag);
            xml.WriteString(IsManual.ToString());
            xml.WriteEndElement();
        }

        private void InitPTZSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(InitPTZ);
            xml.WriteString(InitPTZFlag.ToString());
            xml.WriteEndElement();
        }

        private void ProfilesSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(ProfilesTag);
            if (CurrentProfile == null) xml.WriteAttributeString(CurrentProfileAttribute, string.Empty);
            else xml.WriteAttributeString(CurrentProfileAttribute, CurrentProfile.Name);
            foreach (var profile in ProfileArray) ProfileSave(xml, profile);
            xml.WriteEndElement();
        }

        private void ProfileSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(ProfileTag);
            xml.WriteAttributeString(NameAttribute, profile.Name);
            PathSave(xml, profile);
            PathToPlay(xml, profile);
            PathSaveAudio(xml, profile);
            PathToPlayAudio(xml, profile);
            AudioStartupSave(xml, profile);
            ArchiveStartupSave(xml, profile);
            RemoteControlSave(xml, profile);
            CameraSave(xml, profile);
            AudioDeviceSave(xml, profile);
            SlotSave(xml, profile);
            xml.WriteEndElement();
        }

        private void AudioDeviceSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(AudioDeviceTag);
            foreach (var item in profile.AudioDeviceCollection)
            {
                xml.WriteStartElement(DeviceTag);
                xml.WriteAttributeString(NumberAttribute, item.Number.ToString());
                xml.WriteAttributeString(NameAttribute, item.Name);
                xml.WriteAttributeString(ValueAttribute, item.IsEnable.Value.ToString());
                xml.WriteEndElement();
            }
            xml.WriteEndElement();
        }

        private void RemoteControlSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(RemoteControlTag);
            xml.WriteAttributeString(PortAttribute, profile.RemoteControlPort.ToString());
            xml.WriteEndElement();
        }

        private void CameraSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(CameraArrayTag);
            foreach (var item in profile.ImageCameraCollection)
            {
                xml.WriteStartElement(CameraTag);
                xml.WriteAttributeString(NameAttribute, item.Name);
                xml.WriteAttributeString(AddressAttribute, item.Address);
                xml.WriteAttributeString(UserNameAttribute, item.UserName);
                xml.WriteAttributeString(PasswordAttribute, item.Password);
                xml.WriteAttributeString(HasInfoAttribute, item.HasInfo.ToString());
                xml.WriteAttributeString(Has360Attribute, item.Has3D.ToString());
                xml.WriteEndElement();
            }
            xml.WriteEndElement();
        }

        private void ContentSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(ContentArrayTag);
            foreach (var item in ContentArray)
            {
                xml.WriteStartElement(ContentTag);
                xml.WriteAttributeString(HeaderAttribute, item.Item1);
                xml.WriteAttributeString(TailAttribute, item.Item2);
                xml.WriteEndElement();
            }
            if (ContentArray.Count == 0)
            {
                xml.WriteStartElement(ContentTag);
                xml.WriteAttributeString(HeaderAttribute, "");
                xml.WriteAttributeString(TailAttribute, "");
                xml.WriteEndElement();
            }
            xml.WriteEndElement();
        }

        private void PathSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(PathToSaveTag);
            xml.WriteString(profile.PathToSave);
            xml.WriteEndElement();
        }

        private void PathToPlay(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(PathToPlayTag);
            xml.WriteString(profile.PathToPlay);
            xml.WriteEndElement();
        }

        private void PathSaveAudio(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(PathToSaveAudioTag);
            xml.WriteString(profile.PathToSaveAudio);
            xml.WriteEndElement();
        }

        private void PathToPlayAudio(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(PathToPlayAudioTag);
            xml.WriteString(profile.PathToPlayAudio);
            xml.WriteEndElement();
        }

        private void WindowSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(StartupWindowTag);
            xml.WriteString(RunToWindow.ToString());
            xml.WriteEndElement();
        }

        private void TimeMinRecSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(TimeRecMinTag);
            xml.WriteString(TimeMinRec.ToString());
            xml.WriteEndElement();
        }

        private void ArchiveStartupSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(ArchiveStartupTag);
            xml.WriteString(profile.IsArchiveStartup.ToString());
            xml.WriteEndElement();
        }

        private void AudioStartupSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(AudioEnableTag);
            xml.WriteString(profile.AudioEnable.ToString());
            xml.WriteEndElement();
        }

        private void SlotSave(XmlTextWriter xml, Profile profile)
        {
            xml.WriteStartElement(SlotTag);
            xml.WriteString(profile.CameraSlot.ToString());
            xml.WriteEndElement();
        }

        private void AdminSave(XmlTextWriter xml)
        {
            xml.WriteStartElement(AdminTag);
            xml.WriteAttributeString(PasswordAttribute, Password);
            xml.WriteEndElement();
        }
    }
}
