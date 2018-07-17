import sys
sys.path.append('/home/ryan/projects/BiCEP-Dosecalc/extern/python') # volume.py, fmaps.py
import os
import struct
import numpy as np

from volume import Volume
from fmaps import Fmaps, Beam
from materials import *

OUTPUT_DIR = './output'

def generate_slab_phantom(size, voxelsize, iso, slab_defs, phantom_name='slab'):
    """Args:
        slab_defs ([(int, material), ...]: define thickness and material of each slice in slab phantom
    """
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    dens = np.empty(np.product(size))

    # write Monte Carlo geo.txt file
    center = list(iso)
    center[2] += (voxelsize[2]*size[2]/2.)
    with open(os.path.join(OUTPUT_DIR, "mcgeo_{!s}.txt").format(phantom_name), 'w') as fd:
        fd.write("{:d} {:d} {:d}\n".format(*size))
        fd.write("{:f} {:f} {:f}\n".format(*voxelsize))
        fd.write("{:f} {:f} {:f}\n".format(*center))

        # ZYX ORDERING
        idx = 0
        for layer in slab_defs:
            for ii in range(np.prod([layer[0], *size[:2]])):
                dens[idx] = layer[1][0]
                fd.write("{:f} {:d} {:d} {:f}\n".format(*layer[1]))
                idx += 1

    # write dosecalc ready phantom file and fmaps file
    #  vol = Volume.CenterAt(dens.astype('f').reshape(size[::-1]), np.divide(center, 10), np.divide(voxelsize, 10))
    dens = dens.reshape(size[::-1])
    dens = np.concatenate([np.zeros((4, size[1], size[0])), dens], axis=0)
    vol = Volume.CenterAt(np.ascontiguousarray(np.transpose(dens.astype('f'), (1,0,2))), (0, voxelsize[1]*(dens.shape[1]-4)/20, 0), np.divide(voxelsize, 10))
    print(vol.data.shape)
    vol.generate(os.path.join(OUTPUT_DIR, 'phantom_{!s}.h5'.format(phantom_name)))
    fmaps = Fmaps()
    fmap_wts = np.ones((10,10))
    #  fmaps.addBeam(Beam(fmap_wts, gantry_deg=-90, couch_deg=90, coll_deg=0, iso=np.divide(iso, 10), sad=100, beamlet_size=(0.5, 0.5)))
    fmaps.addBeam(Beam(fmap_wts, gantry_deg=0, couch_deg=0, coll_deg=0, iso=np.divide(iso, 10), sad=100, beamlet_size=(0.5,0.5)))
    fmaps.generate(os.path.join(OUTPUT_DIR, 'fmaps_{!s}.h5'.format(phantom_name)))


def dens2HU(v):
    """convert density in g/cm3 to Hounsfield Units (HU) using simple analytical model"""
    return v*1024.0 - 1024.0

def water_phantom():
    size = (90, 90, 125)
    voxelsize = (2, 2, 2) # mm
    iso = [0, 0, 0]

    slab_defs = [(size[2], water)]
    generate_slab_phantom(size, voxelsize, iso, slab_defs, 'water')

def slab_phantom():
    size = (90, 90, 128)
    voxelsize = (2, 2, 2) # mm
    iso = [0, 0, 0]

    unit_pix = 8
    slab_defs = [
        (1*unit_pix, icrp_adipose_tissue),
        (1*unit_pix, muscle),
        (1*unit_pix, bone),
        (1*unit_pix, muscle),
        (6*unit_pix, air),
        (1*unit_pix, muscle),
        (1*unit_pix, bone),
        (1*unit_pix, icrp_adipose_tissue),
        (1*unit_pix, bone),
        (1*unit_pix, muscle),
        (1*unit_pix, icrp_adipose_tissue),
    ]
    generate_slab_phantom(size, voxelsize, iso, slab_defs, 'slab')


if __name__ == "__main__":
    water_phantom()
    slab_phantom()
