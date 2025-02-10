using System.Collections.Generic;

namespace Cam.Text3D
{
    internal class CircularList<T> : List<T>
    {
        // Accessor won't raise exception if index is out of range!
        public new T this[int i]
        {
            get
            {
                return base[Normalize(i)];
            }
            set
            {
                base[Normalize(i)] = value;
            }
        }
        int Normalize(int i)
        {
            if (Count == 0)
                return i;

            while (i < 0)
                i += Count;

            while (i >= Count)
                i -= Count;

            return i;
        }
    }
}
