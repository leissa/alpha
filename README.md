# Hashing Modulo Alpha-Equivalence

C++ implementation of [Hashing Modulo Alpha-Equivalence](https://arxiv.org/abs/2105.02856).

## Build

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh), just do this:
```
git clone git@github.com:leissa/alpha.git
```
Otherwise, clone via HTTPS:
```
git clone https://github.com/leissa/alpha.git
```
Then, build with:
```
cd alpha
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.
