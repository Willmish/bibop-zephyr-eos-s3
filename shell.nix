# shell.nix
# We assume that Zephyr is cloned somewhere on the system already
let
  pkgs = import <nixpkgs> {};
in
with pkgs;
mkShell {
  buildInputs = [
    gdb
    picocom
	git
	cmake
	ninja
	gperf
	ccache
	dfu-util
    dtc
	wget
    python3Full
	python310Packages.pyelftools
	#python3-pip
	#python3-setuptools
	#python3-tk
	#python3-wheel
	xz
	file
	gnumake
	gcc
    gccMultiStdenv
	#gcc-multilib
    #"g++-multilib
    SDL2
	#libsdl2-dev
	#libmagic1
  ];
  shellHook = ''
     setxkbmap -v workman_pl_de && xset r 66
  '';
}
