"""Library for creating custom fluence map (fmaps) files used by dosecalc

This format replaces the simpler test formatted "beamlist.txt" file that specifies beam angles, isocenters, and SADs.
This format instead defines each beam with its angles, isocenter, SAD, fluence map size, beamlet size, and fluence intensities.
"""

import math
import numpy as np
import h5py


class Beam:
    """Simple class for representing a single beam instance with an attached fluence intensity map"""
    def __init__(self, wts, gantry=0, couch=0, coll=0, iso=(0,0,0), sad=100, beamlet_size=(0.5, 0.5), gantry_deg=None, couch_deg=None, coll_deg=None):
        self.wts = wts
        self.gantry = gantry if (gantry_deg is None) else (gantry_deg*math.pi/180.)
        self.couch = couch if (couch_deg is None) else (couch_deg*math.pi/180.)
        self.coll = coll if (coll_deg is None) else (coll_deg*math.pi/180.)
        self.sad = sad
        self.iso = iso
        self.beamlet_size = beamlet_size

    def __str__(self):
        return  'Beam:\n' +\
                '  gantry: {:0.2f} [rad]\n'.format(self.gantry) +\
                '  couch:  {:0.2f} [rad]\n'.format(self.couch) +\
                '  coll:   {:0.2f} [rad]\n'.format(self.coll) +\
                '  src:    ({:0.2f}, {:0.2f}, {:0.2f}) [cm]\n'.format(*self.src) +\
                '  iso:    ({:0.2f}, {:0.2f}, {:0.2f}) [cm]\n'.format(*self.iso) +\
                '  dir:    ({:0.2f}, {:0.2f}, {:0.2f})\n'.format(*self.direction) +\
                '  sad:    {:0.2f} [cm]\n'.format(self.sad) +\
                '  beamlet_size: {:0.2f}x{:0.2f} [cm]\n'.format(*self.beamlet_size) +\
                '  fmap ({:d}x{:d}):\n{!s}'.format(*self.wts.shape, self.wts)

    @property
    def src(self):
        return np.array(self.iso) + self.rotate_bev2gcs(np.array([0, -self.sad, 0]), self.gantry, self.couch, self.coll)

    @property
    def direction(self):
        return self.rotate_bev2gcs((0,1,0), self.gantry, self.couch, self.coll)

    @staticmethod
    def rotate_gcs2bev(vec, gantry, couch, coll):
        s = math.sin(-couch)
        c = math.cos(-couch)
        rotation_axis = (s, 0., c)                                             # couch rotation
        tmp = Beam.RotateAroundAxisAtOriginRHS(vec, rotation_axis, -gantry)    # gantry rotation
        return Beam.RotateAroundAxisAtOriginRHS(tmp, (0., 1., 0.), couch+coll) # coll rotation + correction

    @staticmethod
    def rotate_bev2gcs(vec, gantry, couch, coll):
        # invert what was done in forward rotation
        s = math.sin(-couch)
        c = math.cos(-couch)
        tmp = Beam.RotateAroundAxisAtOriginRHS(vec, (0., 1., 0.), -(couch+coll)) # coll rotation + correction
        rotation_axis = (s, 0., c);                                              # couch rotation
        return Beam.RotateAroundAxisAtOriginRHS(tmp, rotation_axis, gantry)      # gantry rotation

    @staticmethod
    def RotateAroundAxisAtOriginRHS(p, r, t):
        # ASSUMES r IS NORMALIZED ALREADY and center is (0, 0, 0)
        # p - vector to rotate
        # r - rotation axis
        # t - rotation angle [rads]
        s = math.sin(t)
        c = math.cos(t)
        return ((-r[0]*(-r[0]*p[0] - r[1]*p[1] - r[2]*p[2]))*(1-c) + p[0]*c + (-r[2]*p[1] + r[1]*p[2])*s,
                (-r[1]*(-r[0]*p[0] - r[1]*p[1] - r[2]*p[2]))*(1-c) + p[1]*c + (+r[2]*p[0] - r[0]*p[2])*s,
                (-r[2]*(-r[0]*p[0] - r[1]*p[1] - r[2]*p[2]))*(1-c) + p[2]*c + (-r[1]*p[0] + r[0]*p[1])*s )

class Fmaps:
    """Container class for 'Beam' objects, that implements the file writing functions"""
    def __init__(self):
        self.beams = []

    def __str__(self):
        return '\n\n'.join([str(x) for x in self.beams])

    def addBeam(self, beam):
        self.beams.append(beam)

    def generate(self, fname):
        with h5py.File(fname, 'w') as h5fd:
            # filetype version
            ftgroup = h5fd.require_group("filetype")
            ftgroup.attrs.create("ftmagic", 0x2C, dtype='uint8')
            ftgroup.attrs.create("ftversionmajor", 1, dtype='uint8')
            ftgroup.attrs.create("ftversionminor", 1, dtype='uint8')

            beamgroup = h5fd.require_group("beams").require_group("metadata")
            for ii, beam in enumerate(self.beams):
                bg = beamgroup.require_group('beam_{:05d}'.format(ii))
                bg.attrs.create('beam_uid', ii, dtype='uint16')
                try: N_beamlets = np.count_nonzero(beam.wts)
                except: N_beamlets = 0
                bg.attrs.create('N_beamlets', N_beamlets, dtype='uint16')
                bg.attrs.create('isocenter_type', np.string_("man"))
                specs_type = np.array([(
                    ii,
                    beam.gantry,
                    beam.couch,
                    beam.coll,
                    beam.src,
                    beam.direction,
                    beam.iso,
                    beam.wts.shape,
                    beam.beamlet_size,
                )], dtype=[
                    ('uid', 'uint16'),
                    ('gantry_rot_rad', 'float32'),
                    ('couch_rot_rad', 'float32'),
                    ('coll_rot_rad', 'float32'),
                    ('src_coords_cm', ('float32', (3,))),
                    ('direction', ('float32', (3,))),
                    ('iso_coords_cm', ('float32', (3,))),
                    ('fmap_dims', ('uint32', (2,))),
                    ('beamlet_size_cm', ('float32', (2,))),
                ])
                bg.attrs.create('beam_specs', specs_type)
                bg.attrs.create('fmap_weights', beam.wts, dtype='float32')


if __name__ == '__main__':
    fmaps = Fmaps()
    fmaps.addBeam(Beam(np.zeros((40,40)), gantry_deg=0, couch_deg=0, coll_deg=0, iso=(0, 0, 0), sad=100))
    fmaps.addBeam(Beam(np.zeros((40,40)), gantry_deg=20,couch_deg=0, coll_deg=0, iso=(0, 0, 0), sad=100))
    fmaps.addBeam(Beam(np.zeros((40,40)), gantry_deg=0, couch_deg=20,coll_deg=0, iso=(0, 0, 0), sad=100))
    print(fmaps)
    #  fmaps.generate('test_fmaps.h5')

