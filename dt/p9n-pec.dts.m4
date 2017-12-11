define(`PEC_BASE', `eval(0xD000000 + $1 * 0x1000000, 16)')dnl
define(`PEC', `pec@PEC_BASE($1) {
compatible = "ibm,power9-chiplet";
reg = <0x0 HEX(PEC_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "pec";
index = <HEX(eval($2, 16))>;
}')dnl

PEC(0, 0);
PEC(1, 1);
PEC(2, 2);
