/dts-v1/;

/ {
  include(common.m4)dnl
  include(p9n.dts.m4)dnl

  CHIP(pib0, 0)
};

&pib0 {
  compatible = "ibm,fake-pib";
  reg = <0x0 0x0 0x0>;
  index = <0x0>;
};
