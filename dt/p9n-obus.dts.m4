define(`OBUS_BASE', `eval(0x9 + $1, 16)000000')dnl
define(`OBUS', `obus@OBUS_BASE($1) {
compatible = "ibm,power9-chiplet";
reg = <0x0 HEX(OBUS_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "obus";
index = <HEX(eval($2, 16))>;
}')dnl

OBUS(0, 0);
OBUS(3, 3);
