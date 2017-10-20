define(`EQ_BASE', `eval(0x10000000 + $1 * 0x1000000, 16)')dnl
define(`EQ', `eq@EQ_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(EQ_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "eq";
index = <HEX(eval($2, 16))>;
}')dnl

EQ(0, 0);
EQ(1, 1);
EQ(2, 2);
EQ(3, 3);
EQ(4, 4);
EQ(5, 5);
