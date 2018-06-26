/dts-v1/;

/ {
  include(common.m4)dnl
  include(p9n.dts.m4)dnl

  CHIP(pib0, 0)
  CHIP(pib8, 8)
};

&pib0 {
  compatible = "ibm,host-pib";
  chip-id = <0x0>;
  index = <0x0>;
};

&pib8 {
  compatible = "ibm,host-pib";
  chip-id = <0x8>;
  index = <0x1>;
};
