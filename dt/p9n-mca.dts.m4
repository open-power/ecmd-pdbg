define(`MCA_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`MCA', `mca@MCA_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(MCA_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "mca";
index = <HEX(eval($2, 16))>;
}')dnl

MCA(0, 0);
MCA(1, 1);
MCA(2, 2);
MCA(3, 3);
MCA(4, 4);
MCA(5, 5);
MCA(6, 6);
MCA(7, 7);
