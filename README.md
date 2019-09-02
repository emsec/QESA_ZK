# QESA_ZK
Code for our CCS paper "Efficient zero-knowledge arguments in the discrete log setting, revisited"

[eprint version](https://eprint.iacr.org/2019/944)

Please direct your questions regarding the underlying math towards M. Klooß (michael.klooss@kit.edu).
Questions regarding the code should be send to M. Hoffmann (max.hoffmann@rub.de).

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

## Understanding The C++ Code
In order to familiarize yourself with the code and get started, take a look at the files in `src/test/`.
Every proof has its own test file.

## Arithmetic Circuits to Quadratic Equations
In `AC_to_QuadEq` a collection of Python3 scripts can be found that transforms an arithmetic circuit into quadratic equation matrices.
We support outputs of pinocchio and jsnark.
Simply run `python3 converter.py <circuit_file>` and the scripts will generate three output files:
* `<circuit_file>_out` an optimized circuit with quadratic gates
* `<circuit_file>_mat` the resulting quadratic circuit in matrix form
* `<circuit_file>_wit` a randomized witness that satisfies the circuit, i.e., random input values ranging from 0 to 255, first input value is 1, and the corresponding wire values of the circuit.


You can use `python3 generate_witness.py <circuit_file> <input_values>` to generate valid wire assignments for a specific set of input values (separated via space on the command line).
The first input value has to be 1.
