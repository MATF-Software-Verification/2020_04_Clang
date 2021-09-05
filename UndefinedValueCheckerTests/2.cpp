// RUN: %clang_analyze_cc1 -Wno-unused -analyzer-checker=alpha.unix.UndefinedValueChecker -verify %s
void test() {

	int arrInt[3] = {1,2,3};
	int tmp = arrInt[150] + 23;

}	
