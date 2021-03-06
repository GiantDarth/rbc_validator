#!/usr/bin/env bash

set -x

./rbc_validator --mode=ecc -rv -m2
./rbc_validator --mode=ecc -bv -m2

# Compressed Form
[[ $(./rbc_validator --mode=ecc -v -m2 \
    000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f \
    02d253229db92099808ace5668aa3c1b182a3857ebacbee5e67eeb0f5e422a1ce9) == \
  "100102030405060718090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f" ]]

# Uncompressed Form
[[ $(./rbc_validator --mode=ecc -v -m2 \
    000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e10 \
    04d253229db92099808ace5668aa3c1b182a3857ebacbee5e67eeb0f5e422a1ce90f1f52b9ba6ea8242d469c6208f8ba304056181a85406542bf3a89b5badb1cee) == \
  "100102030405060718090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f" ]]

# Hybrid Form
[[ $(./rbc_validator --mode=ecc -v -m2 \
    000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f \
    06d253229db92099808ace5668aa3c1b182a3857ebacbee5e67eeb0f5e422a1ce90f1f52b9ba6ea8242d469c6208f8ba304056181a85406542bf3a89b5badb1cee) == \
  "100102030405060718090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f" ]]

[[ $(./rbc_validator --mode=ecc -rvca -m2 -t1 |& grep searched | cut -d' ' -f4) == 32897 ]]
[[ $(./rbc_validator --mode=ecc -rvcaf -m2 -t1 |& grep searched | cut -d' ' -f4) == 32640 ]]

[[ $(./rbc_validator --mode=ecc -rvca -m2 |& grep searched | cut -d' ' -f4) == 32897 ]]
[[ $(./rbc_validator --mode=ecc -rvcaf -m2 |& grep searched | cut -d' ' -f4) == 32640 ]]
