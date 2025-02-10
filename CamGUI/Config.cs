using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Security.Cryptography;
using System.Security;
using System.Xml;
using System.IO;

namespace Cam
{
    static class Config
    {
        readonly static string fileName = "Data.xml";

        public static void Initialize() { }

        public static void Load()
        {
            ReadXML(Memory.Instance.Load);
        }

        public static void Save(bool isProfile)
        {
            if (!Memory.Instance.IsRelease)
            {
                if (isProfile == false) Memory.Instance.Commit();
                WriteXML(Memory.Instance.Save);
            }
        }

        private static void ReadXML(Action<XmlTextReader> readerMethod)
        {
            try
            {
                using (FileStream stream = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read))
                using (XmlTextReader xml = new XmlTextReader(stream))
                    if (readerMethod != null) readerMethod(xml);
            }
            catch (Exception ex) { Console.Write(ex); }
        }

        private static void WriteXML(Action<XmlTextWriter> writerMethod)
        {
            try
            {
                using (MemoryStream memory = new MemoryStream())
                using (XmlTextWriter xml = new XmlTextWriter(memory, System.Text.Encoding.UTF8) { Formatting = Formatting.Indented })
                {
                    writerMethod(xml);
                    memory.Position = 0;
                    byte[] buffer = new byte[memory.Length];
                    memory.Read(buffer, 0, buffer.Length);
                    using (FileStream stream = new FileStream(fileName, FileMode.Create, FileAccess.Write)) stream.Write(buffer, 0, buffer.Length);
                }
            }
            catch { }
        }

    }
}
