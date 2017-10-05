define(`CAPP_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`CAPP', `capp@CAPP_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(CAPP_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "capp";
index = <HEX(eval($2, 16))>;
}')dnl

CAPP(0, 0);
CAPP(1, 1);
