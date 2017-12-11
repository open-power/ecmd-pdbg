define(`NV_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`NV', `nv@NV_BASE($1) {
compatible = "ibm,power9-chiplet";
reg = <0x0 HEX(NV_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "nv";
index = <HEX(eval($2, 16))>;
}')dnl

NV(0, 0);
NV(1, 1);
NV(2, 2);
NV(3, 3);
NV(4, 4);
NV(5, 5);
