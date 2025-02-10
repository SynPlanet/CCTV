using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace Cam
{
    static class Extension
    {
        public static string GetAttributeExt(this XmlTextReader xml, string name)
        {
            string value = xml.GetAttribute(name);
            return value == null ? string.Empty : value;
        }
    }
}
