# 2020_04_Clang
# Izrada novih semantičkih provera u okviru Clang alata

## Clanovi tima
Milos Mikovic 1050/2020

# Building environment
1. Build [LLVM](https://github.com/llvm/llvm-project) following the instructions from the Github page. (Over 8GiB of RAM highly recommended or increase swap memory).
2. Copy `ExampleChecker.cpp` to the `llvm-project/clang/lib/StaticAnalyzer/Checkers` folder.
3. Edit the `CMakeLists.txt` file in the aforementioned `Checkers` folder so that `cmake` could recognize added `ExampleChecker.cpp` file.
4. Add following block of code into `Checkers.td` file contained in `llvm-project/clang/include/clang/StaticAnalyzer/Checkers`:
``` cpp
let ParentPackage = UnixAlpha in {
    ...
    def ShiftingChecker : Checker<"ExampleChecker">,  
                          HelpText<"Checker finds undefined values of binary operands and specifically checks access to the array outside the boundaries if that array is an operand of binary operator">,  
                          Documentation<NotDocumented>;  
    ...
} // end "alpha.unix" 
```
5. In the `build` directory, run `make clang`. This will only rebuild `Clang`.

# Usage
For applications written in C, run: ` clang --analyze -Xanalyzer -analyzer-checker=unix.alpha example.c `

For applications written in C++, run: ` clang++ --analyze -Xanalyzer -analyzer-checker=unix.alpha example.cpp `

Both `clang` and `clang++` can be found in `llvm-project/build/bin`.

# Testing

ShiftingChecker can be tested via `llvm-lit` and tests contained in `ExampleCheckerTest` folder of this repository. Copy tests to `llvm-project/clang/test/Analysis` after rebuilding `clang`, run: ` llvm-project/build/bin/llvm-lit <Absolute path of a test in the Analysis folder> -a `
