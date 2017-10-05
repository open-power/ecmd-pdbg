define(`EX_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`EX', `ex@EX_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(EX_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "ex";
index = <HEX(eval($2, 16))>;
}')dnl

EX(0, 0);
EX(1, 1);
EX(2, 2);
EX(3, 3);
EX(4, 4);
EX(5, 5);
EX(6, 6);
EX(7, 7);
EX(8, 8);
EX(9, 9);
EX(10, 10);
EX(11, 11);
