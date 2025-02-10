using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace Cam
{
    class AudioDevice : INotifyPropertyChanged
    {
        private bool? isEnable;

        public AudioDevice(string name, int number)
        {
            Name = name;
            Number = number;
            isEnable = false;
        }

        public int Number { get; private set; }

        public String Name { get; private set; }

        public bool? IsEnable
        {
            get { return isEnable; }
            set
            {
                isEnable = value;

                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs("IsEnable"));
            }
        }

        public AudioDevice Clone()
        {
            var clone = new AudioDevice(Name, Number);
            clone.IsEnable = IsEnable;
            return clone;
        }

        public event PropertyChangedEventHandler PropertyChanged;
    }
}
