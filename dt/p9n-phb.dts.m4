define(`PHB_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`PHB', `phb@PHB_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(PHB_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "phb";
index = <HEX(eval($2, 16))>;
}')dnl

PHB(0, 0);
PHB(1, 1);
PHB(2, 2);
PHB(3, 3);
PHB(4, 4);
PHB(5, 5);
