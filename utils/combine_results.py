######################################################################
# combine_results.py
#
# Created:      06 June 2017 - Ryan Neph
# Modified:     22 May 2018 - Ryan Neph
# Description:  Monte Carlo (PBM 207) - Spring 2017 - Final Project
#               Data Combination
#
# Dependencies: Numpy
# Example usage:   'python combine_results.py'
######################################################################

import sys
import os
import math
import struct
import numpy as np

# datasize
nx = ny = 90
nz = 125
N = nx*ny*nz

# combination params
start = 1
end = 4
results_root = sys.argv[1]
storeto = os.path.join(results_root, 'combined')
os.makedirs(storeto, exist_ok=True)
combine_bin_fnames = ['dose3d', 'photonFluence']

def load_bin(path):
    bytesper = 8  # stored in double format
    nbytes = N*bytesper
    with open(path, 'rb') as f:
        contents = f.read()
        if len(contents) != nbytes:
            raise Exception('Expected {:d} bytes but unpacked {:d} bytes from file'.format(nbytes, len(contents)))
        dose = struct.unpack('{:d}d'.format(N), contents)
    return np.array(dose).reshape((nz, ny, nx))

def save_bin(path, arr):
    bytesper = 8  # stored in double format
    nbytes = N*bytesper
    with open(path, 'wb') as f:
        barr = arr.tobytes()
        f.write(barr)


results_dname_list = [os.path.join(results_root, 'full_run_run_{:d}'.format(x)) for x in range(start,end+1)]
comb_arrays = {k: np.zeros((nz, ny, nx)) for k in combine_bin_fnames}
for dname in results_dname_list:
    for i, p in enumerate(combine_bin_fnames):
        fullpath = os.path.join(dname, p)
        if not os.path.isfile(fullpath):
            raise FileNotFoundError('"dose-bin" #{:d} ("{!s}") must be a valid path to a binary dose file'.format(i, fullpath))
        print('adding vals from {!s} to {!s}'.format(fullpath, p))
        arr = load_bin(fullpath)
        # add to persistent array
        comb_arrays[p] += arr

for p, arr in comb_arrays.items():
    fullpath = os.path.join(storeto, p)
    save_bin(fullpath, arr)

