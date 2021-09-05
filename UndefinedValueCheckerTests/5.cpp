// RUN: %clang_analyze_cc1 -Wno-unused -analyzer-checker=alpha.unix.UndefinedValueChecker -verify %s
void test() {

	int unDefInt;
    int tmp = 23 + unDefInt;
    
}	
