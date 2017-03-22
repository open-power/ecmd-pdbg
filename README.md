# ecmd-pdbg
The code in this repo provides the glue code necessary for pdbg
to be used as an eCMD plugin

## building
Both eCMD and pdbg need to be built prior to building this repo.

### Building eCMD
Clone eCMD locally, then in the repo run:
`./config.py && make`

### Building pdbg
Clone pdbg locally, then in the repo run:
`./bootstrap.sh && CFLAGS="-fPIC" ./configure && make`

### Building ecmd-pdbg
Once the two builds above succeed, in this repo run:
`./config.py --ecmd-root /home/user/ecmd --pdbg-root /home/user/pdbg && make`
