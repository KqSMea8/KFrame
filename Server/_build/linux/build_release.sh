mkdir -p /data
cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j 6
