define(`C_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`C', `c@C_BASE($1) {
#address-cells = <0x1>;
#size-cells = <0x0>;
compatible = "ibm,power9-core";
reg = <0x0 HEX(C_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "c";
index = <HEX(eval($2, 16))>;

THREAD(0);
THREAD(1);
THREAD(2);
THREAD(3);
}')dnl
define(`THREAD_BASE', `eval($1, 16)')dnl
define(`THREAD',`thread@THREAD_BASE($1) {
compatible = "ibm,power9-thread";
reg = <0x0>;
tid = <HEX(eval($1, 16))>;
index = <HEX(eval($1, 16))>;
}')dnl

#address-cells = <0x2>;
#size-cells = <0x1>;

adu@90000 {
	  compatible = "ibm,power9-adu";
	  reg = <0x0 0x90000 0x5>;
};

C(0, 0);
C(1, 1);
C(2, 2);
C(3, 3);
C(4, 4);
C(5, 5);
C(6, 6);
C(7, 7);
C(8, 8);
C(9, 9);
C(10, 10);
C(11, 11);
C(12, 12);
C(13, 13);
C(14, 14);
C(15, 15);
C(16, 16);
C(17, 17);
C(18, 18);
C(19, 19);
C(20, 20);
C(21, 21);
C(22, 22);
C(23, 23);
