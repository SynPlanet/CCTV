using System.Windows;

namespace Cam.Text3D
{
    internal abstract class DeepTextBase : Text3DBase
    {
        // Depth dependency property and property.
        public static readonly DependencyProperty DepthProperty =
            DependencyProperty.Register("Depth",
                typeof(double),
                typeof(DeepTextBase),
                new PropertyMetadata(1.0, PropertyChanged));

        public double Depth
        {
            set { SetValue(DepthProperty, value); }
            get { return (double)GetValue(DepthProperty); }
        }
    }
}
