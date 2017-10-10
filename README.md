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
subrepos automatically  
Subsequent config.py calls will not do this

### Building ecmd-pdbg
In the ecmd-pdbg clone root dir run:  
`$ make`  
This will call configure on both the ecmd and pdbg subrepos, then build
all three repos.

### Build without calling configure
To avoid having the default make rule always calling the subrepo configures,
run:  
`$ make build`

### Build just ecmd-pdbg
After doing the initial required build of ecmd and pdbg, you can build
just ecmd-pdbg with:  
`$ make edbg-build`

### Configuring to build for openbmc
You can build ecmd-pdbg in a pre-established obmc build env with:  
`$ ./config --target armv5e --ld "${CXX}"`  
You can then build with a regular `make` call
