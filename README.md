# pru-mcp3208-experiments

To build, download the PRU software support package, from
<https://git.ti.com/cgit/pru-software-support-package/pru-software-support-package/>
and extract to `/usr/share/ti/pru-software-support-package`. It can be 
extracted to another path as long as the `PRU_SSP` environment variable points
to it and is exported.

Aditionally, the PRU code generation tools must be downloaded from
<https://www.ti.com/tool/download/PRU-CGT-2-1/> and installed in
`/usr/share/ti/cgt-pru`. If it is installed in a different path, the
`PRU_CGT` environment variable must be exported and point to it.

For cross-compilation, export `CC` to the appropriate cross-compiler, like
`export CC=armv7a-hardfloat-linux-gnueabihf-gcc`.

To compile, simply run `make`. To load the firmware into the PRUs, run 
`make deploy` in the BeagleBone Black. It also configures the relevant pins.
The ARM Host program, `host_rpmsg_mcp3208` reads the data and prints the first
4 readings of each buffer into the STDOUT, together with the timestamp and
time difference between consecutive messages.

Pinout
------

* P9_28: Chip Select
* P9_29: SPI MISO
* P9_30: SPI MOSI
* P9_31: SPI Clock

