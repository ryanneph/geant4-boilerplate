import numpy as np
import h5py

class Volume:
    """simple class for passing arbitrary floating precision intensity volumes to dosecalc instead of structured dicom files"""
    def __init__(self, arr, start=(0,0,0), voxelsize=(0.1, 0.1, 0.1)):
        if (arr.ndim != 3):
            raise ValueError("array must be 3D with ZYX data ordering")
        self.data = arr
        self.start = start
        self.voxelsize = voxelsize

    def __str__(self):
        return  'Volume:\n' +\
                '  start:      ({:0.2f}, {:0.2f}, {:0.2f}) [cm]\n'.format(*self.start) +\
                '  voxelsizet: ({:0.2f}, {:0.2f}, {:0.2f}) [cm]\n'.format(*self.voxelsize) +\
                '  sizet:      ({:0.2f}, {:0.2f}, {:0.2f})\n'.format(*self.data.shape[::-1]) +\
                '  data ({:0.2f}, {:0.2f}, {:0.2f}):\n{!s}'.format(*self.data.shape, self.data)

    @classmethod
    def CenterAt(cls, arr, center, voxelsize=(0.1, 0.1, 0.1)):
        """Place array with center as specified. arr is expected as 3D array in ZYX order"""
        # determine start coords
        size = arr.shape[::-1]
        start = np.array(center) - np.multiply(size, voxelsize)/2. + 0.5*voxelsize
        return cls(arr, start, voxelsize)

    def generate(self, fname):
        """Write H5 file to disk"""
        with h5py.File(fname, 'w') as h5fd:
            # filetype version
            ftgroup = h5fd.require_group("filetype")
            ftgroup.attrs.create("ftmagic", 0x2A, dtype='uint8')
            ftgroup.attrs.create("ftversionmajor", 1, dtype='uint8')
            ftgroup.attrs.create("ftversionminor", 0, dtype='uint8')

            ds = h5fd.create_dataset("data", data=self.data, dtype='float32')
            ds.attrs.create("dicom_start_cm", self.start, dtype='float32')
            ds.attrs.create("voxel_size_cm", self.voxelsize, dtype='float32')


if __name__ == '__main__':
    vol = Volume.CenterAt(np.zeros((128, 90, 90)), (0, 0, 12.8), (0.2, 0.2, 0.2))
    print(vol)
    #  vol.generate('test_vol.h5')
