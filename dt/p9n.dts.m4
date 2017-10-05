define(`CHIP', `fsi@$1 {
		compatible = "ibm,fake-fsi";
		reg = <0x0 0x0 0x0>;

		index = <0x$1>;
		status = "hidden";

		pib@0 {
		      compatible = "ibm,fake-pib";
		      reg = <0x0 0x0 0x0>;
		      index = <0x$1>;

		      include(p9n-c.dts.m4)dnl
                      include(p9n-capp.dts.m4)dnl 
		      include(p9n-eq.dts.m4)dnl
		      include(p9n-ex.dts.m4)dnl
		      include(p9n-mca.dts.m4)dnl
		      include(p9n-mcs.dts.m4)dnl
		      include(p9n-mcbist.dts.m4)dnl
		      include(p9n-nv.dts.m4)dnl
		      include(p9n-obrick.dts.m4)dnl
		      include(p9n-obus.dts.m4)dnl
		      include(p9n-occ.dts.m4)dnl
		      include(p9n-pec.dts.m4)dnl
		      include(p9n-perv.dts.m4)dnl
		      include(p9n-phb.dts.m4)dnl
		      include(p9n-ppe.dts.m4)dnl
		      include(p9n-sbe.dts.m4)dnl
		      include(p9n-xbus.dts.m4)dnl

                      
		};
	};
        ')dnl
