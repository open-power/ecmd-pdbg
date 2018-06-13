/dts-v1/;

/ {
  include(common.m4)dnl
  include(p9n.dts.m4)dnl

  fsi0: kernelfsi@0 {
    #address-cells = <0x2>;
    #size-cells = <0x1>;
    compatible = "ibm,kernel-fsi";
    reg = <0x0 0x0 0x0>;
    index = <0x0>;

    CHIP(pib0, 0)

    hmfsi@10000 {
      compatible = "ibm,fsi-hmfsi";
      reg = <0x0 0x100000 0x8000>;
      port = <0x1>;
      index = <0x1>;

      CHIP(pib8, 8)
    };
  };
};

&pib0 {
  compatible = "ibm,fsi-pib", "ibm,power9-fsi-pib";
  chip-id = <0x0>;
  reg = <0x0 0x1000 0x7>;
  index = <0x0>;
};

&pib8 {
  compatible = "ibm,fsi-pib", "ibm,power9-fsi-pib";
  chip-id = <0x8>;
  reg = <0x0 0x1000 0x7>;
  index = <0x1>;
};
