using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Cam
{
    abstract class Log
    {
        private StreamWriter stream = null;

        protected Log(StreamWriter stream)
        {
            this.stream = stream;
            if (this.stream != null) this.stream.AutoFlush = true;
        }

        public void WriteLine(string format, params object[] args)
        {
            if (stream == null)return;
            lock (stream) stream.WriteLine(format, args);
            
        }

        public void WriteLine()
        {
            if (stream == null) return;
            lock (stream) stream.WriteLine();
        }
    }
}
