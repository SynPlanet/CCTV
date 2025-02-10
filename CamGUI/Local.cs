using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;
using System.Reflection;

namespace Cam
{
    class Local
    {
        private static readonly Local instance = new Local();

        public static Local Message { get { return instance; } }

        private Local()
        {
            Initialize(Properties.Resources.DefaultLocal);
            Initialize(Load(string.Format(@"{0}.txt", Thread.CurrentThread.CurrentCulture.Name)));
        }

        private string Load(string file)
        {
            try
            {
                using (StreamReader reader = new System.IO.StreamReader(file, Encoding.UTF8)) return reader.ReadToEnd();
            }
            catch { return string.Empty; }
        }

        private void Initialize(string resourceMessages)
        {
            var properties = this.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic).Where(x => x.PropertyType == typeof(string) && x.CanRead && x.CanWrite).ToArray();
            var messages = GetMessages(resourceMessages);
            foreach (var item in messages.Join(properties, x => x.Key, y => y.Name, (x, y) => new { Property = y, Message = x.Value }))
            {
                var parts = item.Property.Name.Split(new string[] { "_P" }, StringSplitOptions.RemoveEmptyEntries);
                int value;
                if (int.TryParse(parts[parts.Length - 1], out value))
                {
                    try { string.Format(item.Message, new object[value]); }
                    catch { continue; }
                }
                item.Property.SetValue(this, item.Message, null);
            }
        }

        private Dictionary<string, string> GetMessages(string resourceMessages)
        {
            Dictionary<string, string> messages = new Dictionary<string, string>();
            foreach (var item in resourceMessages.Split(new string[] { Environment.NewLine }, StringSplitOptions.RemoveEmptyEntries).Select(x => x.Trim()).Where(x => x.Length != 0))
            {
                int spaceIndex = item.IndexOf(' ');
                int tabIndex = item.IndexOf('\t');
                if (spaceIndex == -1 && tabIndex == -1) continue;
                if (spaceIndex == -1) spaceIndex = int.MaxValue;
                if (tabIndex == -1) tabIndex = int.MaxValue;
                int index;
                if (spaceIndex < tabIndex) index = spaceIndex;
                else index = tabIndex;
                try { messages.Add(item.Substring(0, index), item.Substring(index).TrimStart()); }
                catch (ArgumentException) { }
            }
            return messages;
        }

        public string MainWindow_Message0 { get; private set; }
        public string MainWindow_Message1 { get; private set; }
        public string MainWindow_Message2 { get; private set; }
        public string MainWindow_Message3 { get; private set; }
        public string MainWindow_Message4 { get; private set; }
        public string MainWindow_Message5 { get; private set; }
        public string MainWindow_Message6 { get; private set; }
        public string MainWindow_Message7 { get; private set; }
        public string MainWindow_Message8 { get; private set; }
        public string MainWindow_Message9 { get; private set; }
        public string MainWindow_Message10 { get; private set; }
        public string MainWindow_Message11 { get; private set; }
        public string MainWindow_Message12 { get; private set; }
        public string MainWindow_Message13 { get; private set; }
        public string MainWindow_Message14 { get; private set; }

        public string Monitor_Message0 { get; private set; }
        public string Monitor_Message1 { get; private set; }
        public string Monitor_Message2 { get; private set; }

        public string PanelInformation_Message0 { get; private set; }
        public string PanelInformation_Message1 { get; private set; }
        public string PanelInformation_Message2 { get; private set; }
        public string PanelInformation_Message3 { get; private set; }
        public string PanelInformation_Message4 { get; private set; }
        public string PanelInformation_Message5 { get; private set; }

        public string EmulationCameraManager_Message0 { get; private set; }
        public string EmulationCameraManager_Message1 { get; private set; }

        public string ParametersControl_Message0 { get; private set; }
        public string ParametersControl_Message1 { get; private set; }
        public string ParametersControl_Message2 { get; private set; }
        public string ParametersControl_Message3 { get; private set; }
        public string ParametersControl_Message4 { get; private set; }

        public string ModifyControl_Message0 { get; private set; }
        public string ModifyControl_Message1 { get; private set; }
        public string ModifyControl_Message2 { get; private set; }

        public string AdminWIndow_Message0 { get; private set; }
        public string AdminWIndow_Message1 { get; private set; }
        public string AdminWIndow_Message2 { get; private set; }
        public string AdminWIndow_Message3 { get; private set; }
        public string AdminWIndow_Message4 { get; private set; }

        public string Settings_Message0 { get; private set; }
        public string Settings_Message1 { get; private set; }
        public string Settings_Message2 { get; private set; }
        public string Settings_Message3 { get; private set; }
        public string Settings_Message4 { get; private set; }
        public string Settings_Message5 { get; private set; }
        public string Settings_Message6 { get; private set; }
        public string Settings_Message7 { get; private set; }
        public string Settings_Message8 { get; private set; }
        public string Settings_Message9 { get; private set; }
        public string Settings_Message10 { get; private set; }
        public string Settings_Message11 { get; private set; }

        public string Audio_Message0 { get; private set; }

        public string Memory_XML_Message0 { get; private set; }
        public string Memory_XML_Message1 { get; private set; }
        public string Memory_XML_Message2 { get; private set; }
        public string Memory_XML_Message3 { get; private set; }
        public string Memory_XML_Message4 { get; private set; }
    }
}
