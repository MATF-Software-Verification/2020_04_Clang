// RUN: %clang_analyze_cc1 -Wno-unused -analyzer-checker=alpha.unix.UndefinedValueChecker -verify %s
void test() {

    int x;
    int *ptr = &x;
    int tmp = *ptr + 23;
    
}	
