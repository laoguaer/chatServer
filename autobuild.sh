set -x
mkdir build
rm -rf `pwd`/build/*
cd build
cmake ..
make
cd ..
