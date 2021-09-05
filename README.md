# Undefined Value Checker 
Undefined Value Checker is a Clang Static Analyzer checker which finds undefined values of binary operands and specifically checks access to the array outside the boundaries if that array is an operand of binary operator

# Team members
Milos Mikovic 1050/2020

# Building environment
1. Build [LLVM](https://github.com/llvm/llvm-project) following the instructions from the Github page. (Over 8GiB of RAM highly recommended or increase swap memory). You can also use the following commands for building llvm:

``` cpp
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -DLLVM_ENABLE_PROJECTS=clang -G "Unix Makefiles" ../llvm
make (-j n (n - Specifies the number of jobs (commands) to run simultaneously.)) Example: make -j 8
```
3. Copy `UndefinedValueChecker.cpp` to the `llvm-project/clang/lib/StaticAnalyzer/Checkers` folder.
4. Edit the `CMakeLists.txt` file in the aforementioned `Checkers` folder so that `cmake` could recognize added `UndefinedValueChecker.cpp` file.
5. Add following block of code into `Checkers.td` file contained in `llvm-project/clang/include/clang/StaticAnalyzer/Checkers`:
``` cpp
let ParentPackage = UnixAlpha in {
    ...
    def UndefinedValueChecker : Checker<"UndefinedValueChecker">,  
                          HelpText<"Checker finds undefined values of binary operands and specifically checks access to the array outside the boundaries if that array is an operand of binary operator">,  
                          Documentation<NotDocumented>;  
    ...
} // end "alpha.unix" 
```
5. In the `build` directory, run `make clang`. This will only rebuild `Clang`.

# Usage
For applications written in C, run: ` clang --analyze -Xanalyzer -analyzer-checker=unix.alpha.UndefinedValueChecker example.c `

For applications written in C++, run: ` clang++ --analyze -Xanalyzer -analyzer-checker=unix.alpha.UndefinedValueChecker example.cpp `

Both `clang` and `clang++` can be found in `llvm-project/build/bin`.

# Testing

ShiftingChecker can be tested via `llvm-lit` and tests contained in `UndefinedValueCheckerTests` folder of this repository. Copy tests to `llvm-project/clang/test/Analysis` after rebuilding `clang`, run: ` llvm-project/build/bin/llvm-lit <Absolute path of a test in the Analysis folder> -a `
