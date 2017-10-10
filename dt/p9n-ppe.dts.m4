define(`PPE_BASE', `eval(0x20000000 + $1 * 0x1000000, 16)')dnl
define(`PPE', `ppe@PPE_BASE($1) {
compatible = "ibm,none";
reg = <0x0 HEX(PPE_BASE($1)) 0xfffff>;
ecmd,chip-unit-type = "ppe";
index = <HEX(eval($2, 16))>;
}')dnl

PPE(0, 0);
PPE(10, 10);
PPE(11, 11);
PPE(12, 12);
PPE(13, 13);
PPE(20, 20);
PPE(21, 21);
PPE(22, 22);
PPE(23, 23);
PPE(24, 24);
PPE(25, 25);
PPE(30, 30);
PPE(31, 31);
PPE(32, 32);
PPE(33, 33);
PPE(34, 34);
PPE(35, 35);
PPE(40, 40);
PPE(41, 41);
PPE(42, 42);
PPE(50, 50);
PPE(51, 51);
PPE(52, 52);
