define(`MCBIST_BASE', `eval(0x7000000 + $1 * 0x1000000, 16)')dnl
define(`MCBIST', `mcbist@MCBIST_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(MCBIST_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "mcbist";
index = <HEX(eval($2, 16))>;
}')dnl

MCBIST(0, 0);
MCBIST(1, 1);
