using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media.Imaging;
using System.Windows.Controls;
using System.Windows.Media;
using System.ComponentModel;

namespace Cam
{
    interface ICamera : INotifyPropertyChanged
    {
        void Update();
        void UpdateImage();
        Image PictureControl { get; }
        bool Update(string header, string tile);

        int ID { get; }
        int Width { get; }
        int Height { get; }
        int Frequency { get; }
        string TextInfo { get; }
        CameraInfo CameraInfo { get; }
    }
}
