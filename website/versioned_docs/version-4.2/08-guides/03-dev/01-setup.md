---
slug: /dev/setup
---

# Setup

*![](https://user-images.githubusercontent.com/7110113/184558441-dc2783c1-0447-489d-9bde-b99d63b6d4b7.png)*


## Environment

The build environment is based on [Shaun Inman's docker image](https://github.com/shauninman/union-miyoomini-toolchain).

We recommend to use a Linux VM with Docker installed. For example you can use [VMware Workstation Player](https://www.vmware.com/fr/products/workstation-player.html) or [VirtualBox](https://www.virtualbox.org/wiki/Downloads).

You can find pre-installed Linux images on [linuxvmimages.com](https://www.linuxvmimages.com/)


## Building

Docker must be installed and running. 

The following command lines will install the required Docker image to get the preconfigured Toolchain for compilation (credits: [Shaun Inman](https://github.com/shauninman/union-miyoomini-toolchain)), compile all the sources and make a release.

Open a Terminal and type : 

`git clone https://github.com/OnionUI/Onion.git`

`cd Onion/`

`make git-submodules`

`make with-toolchain` or  
`make with-toolchain CMD=dev` (to enable debug logging )

Done!


## Toolchain

You can also use the command `make toolchain` to get access to the toolchain docker image, here to can run any commands, like `make dev`.
