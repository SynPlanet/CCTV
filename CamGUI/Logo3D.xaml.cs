using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Cam
{
    /// <summary>
    /// Interaction logic for Logo3D.xaml
    /// </summary>
    public partial class Logo3D : UserControl
    {
        bool isUp = true;
        public Logo3D()
        {
            InitializeComponent();
            CompositionTarget.Rendering += new EventHandler(CompositionTarget_Rendering);
            angle.Angle = -90;
        }

        void CompositionTarget_Rendering(object sender, EventArgs e)
        {

            if (angle.Angle > 20) isUp = false;
            if (angle.Angle < -90) isUp = true;
            if (isUp) angle.Angle += 1;
            else angle.Angle -= 1;
        }
    }
}
