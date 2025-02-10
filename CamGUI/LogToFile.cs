using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Cam
{
    class LogToFile : Log
    {
        public LogToFile(string fileName)
            : base(new StreamWriter(new FileStream(fileName, FileMode.Create, FileAccess.Write, FileShare.Read)))
        {
        }
            
    }
}
