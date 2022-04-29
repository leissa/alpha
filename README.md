# Small CMake-Based C++ "Hello World"

## Build

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh), just do this:
```
git clone git@github.com:leissa/cpp.git
```
Otherwise, clone via HTTPS:
```
git clone https://github.com/leissa/cpp.git
```
Then, build with:
```
cd cpp
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.
