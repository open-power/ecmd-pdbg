define(`MCS_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`MCS', `mcs@MCS_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(MCS_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "mcs";
index = <HEX(eval($2, 16))>;
}')dnl

MCS(0, 0);
MCS(1, 1);
MCS(2, 2);
MCS(3, 3);
