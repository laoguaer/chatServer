set -x
mkdir build
rm -rf `pwd`/build/*
cd build
cmake ..
make -j 4
cd ..
