# Secret-Shared Shuffle

This is an unofficial implememtation of [Secret-Shared Shuffle](https://eprint.iacr.org/2019/1340.pdf) for learning.

## Usage

```bash
mkdir -p out/build/linux

cmake -S . -B out/build/linux  \
  -DSUDO_FETCH=OFF \
  -DFETCH_AUTO=ON \
  -DSSS_NO_SYSTEM_PATH=true \
  -DPARALLEL_FETCH=4 \
  -DCMAKE_BUILD_TYPE=Release \
  -DSSS_ENABLE_ASAN=ON \
  -DCOPROTO_ENABLE_BOOST=ON \
  -DUSE_LIBOTE=ON
  
cmake --build out/build/linux   --parallel 4
```

You can use `-DUSE_LIBOTE=ON` or `-DUSE_EMP=ON` to switch the MPC backend between [libOTe](https://github.com/osu-crypto/libOTe) and [emp-toolkit](https://github.com/emp-toolkit).

`-DCOPROTO_ENABLE_BOOST` is important because the project use `asioConnect` for communication.