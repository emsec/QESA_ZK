# QESA_ZK
Code for our CCS paper "Efficient zero-knowledge arguments in the discrete log setting, revisited"

[eprint version](https://eprint.iacr.org/2019/944)

## Build Instructions
The C++ code uses the [RELIC toolkit](https://github.com/relic-toolkit/relic).
Clone the repository and run
```
mkdir build
cd build

cmake ../ -DWITH="BC;DV;BN;MD;FP;EP" \
    -DTESTS=0 -DBENCH=0 -DCHECK=off -DCOLOR=off -DDOCUM=off \
    -DFP_PRIME=255 -DFP_PMERS=on -DRAND=UDEV -DARITH=GMP \
    -DCOMP="-O3 -funroll-loops -fomit-frame-pointer -fPIC" -DWSIZE=64 -DSTLIB=on -DSHLIB=off $1

make -j
```

Then copy the `librelic_s.a` to `proof_system/lib/` and simply run `make` in the `proof_system` directory.

## Citation

If you use this code in an academic context, please cite our paper using the reference below:
\[to be done\]

## Notes
The code in this repository will be cleaned up over time.

Likewise this readme will be improved.
