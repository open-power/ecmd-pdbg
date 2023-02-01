# ecmd-pdbg
The code in this repo provides the glue code necessary for pdbg
to be used as an eCMD plugin

## building
Known good & working version of ecmd and pdbg are included as subrepos
and will build automatically as part of the ecmd-pdbg build

### Configuring ecmd-pdbg
In the ecmd-pdbg clone root dir run:
`$ ./config.py`
The initial config.py call will init and checkout the ecmd and pdbg
subrepos automatically (subsequent config.py calls will not do this)
config.py will then do the required config steps for the subrepos

### Building ecmd-pdbg
In the ecmd-pdbg clone root dir run:
`$ make`
This will build all three repos.

### Build just ecmd-pdbg
After doing the initial required build of ecmd and pdbg, you can build
just ecmd-pdbg with:
`$ make edbg-build`

### Configuring to build for openbmc
You can build ecmd-pdbg in a pre-established obmc build env with:
`$ ./config.py --bmc-build`
You can then build with a regular `make` call

### To Build using meson
Alternatively ecmd-pdbg can be built using `meson`.

Need `meson` and `ninja`. Alternatively, source an OpenBMC ARM/x86 SDK.
Before running the meson build the following submodule init and update 
commands should be executed

```
git submodule init
git submodule update
meson build && ninja -C build
```

#### Requirements on the native (build) machine
* Meson version should be >= 0.51
* Needs git command support
* Perl and Python is optional

#### Meson install procedure
Meson is available in the Python Package Index and can be installed with
`pip3 install meson` which requires root and will install it system-wide.


