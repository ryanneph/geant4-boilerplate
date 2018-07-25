import sys
import os
import struct
import numpy as np

from materials import *

OUTPUT_DIR = './output'

material_map = [
    water,
    icru_lung,
    titanium,
    icrp_adipose_tissue,
    muscle,
    bone,
    air,
]

def generate(arr, voxelsize, iso, phantom_name='slab'):
    """Args:
        slab_defs ([(int, material), ...]: define thickness and material of each slice in slab phantom
    """
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # isocenter defined at first face along Z-axis
    size = arr.shape[::-1]
    center = list(iso)
    center[2] += (voxelsize[2]*size[2]/2.)
    with open(os.path.join(OUTPUT_DIR, "mcgeo_{!s}.txt").format(phantom_name), 'w') as fd:
        fd.write("{:d} {:d} {:d}\n".format(*size))
        fd.write("{:f} {:f} {:f}\n".format(*voxelsize))
        fd.write("{:f} {:f} {:f}\n".format(*center))

        # ZYX ORDERING
        for matid in np.nditer(arr):
            mat = material_map[matid]
            fd.write("{:f} {:d} {:d} {:f}\n".format(*mat))


if __name__ == "__main__":
    # properties defined as (X,Y,Z)
    size = (90, 90, 128)
    voxelsize = (2, 2, 2) # mm
    iso = [0, 0, 0]
    # data ordered as [Z,Y,X]
    arr = np.zeros(size[::-1], dtype=np.int8)
    generate(arr, voxelsize, iso, 'array')
