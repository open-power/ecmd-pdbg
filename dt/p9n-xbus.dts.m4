define(`XBUS_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`XBUS', `xbus@XBUS_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(XBUS_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "xbus";
index = <HEX(eval($2, 16))>;
}')dnl

XBUS(1, 1);
XBUS(2, 2);
