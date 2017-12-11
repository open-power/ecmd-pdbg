define(`OBRICK_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`OBRICK', `obrick@OBRICK_BASE($1) {
compatible = "ibm,power9-chiplet";
reg = <0x0 HEX(OBRICK_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "obrick";
index = <HEX(eval($2, 16))>;
}')dnl

OBRICK(0, 0);
OBRICK(1, 1);
OBRICK(2, 2);
OBRICK(9, 9);
OBRICK(10, 10);
OBRICK(11, 11);
