# Contributing to Nixpkgs

I have been a big fan of [Nix](https://nixos.org) for a few years now, however
I never ended up learning it properly. It has been (still is) something that I
will repeatedly attempt to use for a job I know it is made for, but end up not
succeeding because the lack of understanding. Of course there have been
[minor](https://github.com/kintrix007/play-next/blob/master/default.nix)
[successes](https://github.com/kintrix007/minesweeper/blob/master/default.nix)
along the way when I managed to make it cooperate, but often times I just did
not know how to describe what I wanted in terms of what Nix makes available to
me.

## What exactly is Nix?

Nix is a package manager. And a programming/markup language. And there is an
entire OS (a Linux distro) built around it. But [you can use it on any Linux or
Mac machine](https://nixos.org/download/#download-nix).

Nixpkgs, the software repository the Nix package manager uses, is probably the
biggest *binary* repository that exists with over 80,000 packages. Yes, I
specifically said *binary* so that the AUR does not qualify. However, the
humongous amount of packages pre-built is not the reason I love Nix. Althogh,
it is very nice.

It is a package manager which claims to have solved [dependency
hell](https://en.wikipedia.org/wiki/Dependency_hell). If two programs need
different versions of the same library, it will simply install both versions,
and just make it work. The "and make just it work" part is a lot more
complicated than what I am making it sound like, but generally it just works.

## Prerequisites

I will not go in-depth about the syntax of the Nix language. Although it is not
*difficult* to read, pre se, it does require a basic understand of common
functional programming concepts and does use unusual syntax in places.

Basic understanding of the Nix language is recommended.

## Building a package with Nix

Oh, did I mention that Nix is also a build tool? Well, it is. Let's do a deep
dive. Here is an example of using Nix to build [GNU
Hello](https://www.gnu.org/software/hello/):

[`hello.nix`](./hello.nix)

```nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.stdenv.mkDerivation rec {
  pname = "hello";
  version = "2.12.1";

  src = pkgs.fetchurl {
    url = "https://ftp.gnu.org/gnu/hello/hello-${version}.tar.gz";
    sha256 = "086vqwk2wl8zfs47sq2xpjc9k066ilmb8z6dn0q6ymwjzlm196cd";
  };
}
```

It specifies to use the Nixpkgs version that is used on the current system,
then it creates a "derivation". A derivation is a *thing* that describes how to
build a package, effectively.

But all that is specified is the package name (`pname`), the version
(`version`) and the source (`src`). How does it know how to actually, y'know,
build it?

The answer is generic builders. Nixpkgs has a bunch of generic builders for
common build tools. GNU Hello is built with `autotools`, so it just
automatically figures out how to build a package with `autotools`. This leads
to very elegant package descriptions, since the steps for building for common
build tools do not need to be specified.

Otherwise, if Nixpkgs does not already have a generic builder for the build
tool a project uses, the build instructions can be specified as Bash snippets,
or a single bash script file.

Enough talk, let's *actually* build it.

```sh
$ nix-build hello.nix -o hello
this derivation will be built:
  /nix/store/bfw39x992w52r4sqml563zkzwn4aczcp-hello-2.12.1.drv
building '/nix/store/bfw39x992w52r4sqml563zkzwn4aczcp-hello-2.12.1.drv'...
Running phase: unpackPhase
unpacking source archive /nix/store/pa10z4ngm0g83kx9mssrqzz30s84vq7k-hello-2.12.1.tar.gz
source root is hello-2.12.1
setting SOURCE_DATE_EPOCH to timestamp 1653865426 of file hello-2.12.1/ChangeLog
Running phase: patchPhase
Running phase: updateAutotoolsGnuConfigScriptsPhase
Updating Autotools / GNU config script to a newer upstream version: ./build-aux/config.sub
Updating Autotools / GNU config script to a newer upstream version: ./build-aux/config.guess
Running phase: configurePhase
[...] # Omitted the rest of the 1353 lines of build output
/nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1
```

There are some interesting things here, other then the fact that it is a *lot*
of lines. First, that this all happens under `/nix/store`. This is how Nix
makes sure the builds stay isolated, so it can be as reproducible as possible.

The last line (which is the only one actually printed to `stdout`, the rest
were on `stderr`) shows us where the binary has been built to. It is not in the
current directory, but it gets symlinked there.

```sh
$ ls -F
hello@  hello.nix
$ file hello
hello: symbolic link to /nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1
$ file /nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1
/nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1: directory
```

Oh, alright, so that is not an executable, but a directory. Let's look at its
contents.

```sh
$ ls hello
bin  share
$ ls hello/bin
hello
```

Aha! That is where it is. Let's see if it works

```sh
$ ./hello/bin/hello 
Hello, world!
```

Seems to work just fine. It also has a `share/` directory since the packages
are all self-contained under their store path. Instead of adding it under
`/usr/local/share`, it all keeps it under `/nix/store/<hash>-package/share`.
Upon install this path gets added to `XDG_DATA_DIRS` so that packages work as
intended. Similarly, the `bin/` directory gets added to `PATH` to be able to
run the binaries like any other. Pretty clever.

We can delete it with the help of `nix-store --delete`:

```sh
$ realpath hello
/nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1
$ rm hello
$ nix-store --delete /nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1
finding garbage collector roots...
removing stale link from '/nix/var/nix/gcroots/auto/q1zlrwh5qg28kcfi517np5m0kmrz2wmm' to '/home/kin/blog/contributing-to-nixpkgs/hello'
deleting '/nix/store/sc0rz1y1qy03xn40wg3n2i80hbv1r4s7-hello-2.12.1'
deleting unused links...
note: currently hard linking saves -0.00 MiB
1 store paths deleted, 0.19 MiB freed
```

Unfortunately I do not know of a simpler way of deleting the build output of a
single package. Alternatively, `rm hello && nix-collec-garbage` would have also
worked, but it would removed all other packages <sub>that are not GC roots
themselves or dependencies of GC roots</sub>.

## Building the VLC BitTorrent plugin

Enough exploring, let's package `vlc-bittorent` with Nix. Oddly enough, it is
not part of Nixpkgs, even though it can be found under the same name in the
Debian/Ubuntu and the Fedora repos. So let's add it:

```nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.stdenv.mkDerivation {
  pname = "vlc-bittorrent";
  version = "2.15.0";

  src = fetchGit {
    url = "https://github.com/johang/vlc-bittorrent";
    rev = "6810d479e6c1f64046d3b30efe78774b49d1c95b";
  };
}
```

Let's try building it too:

```sh
$ nix-build vlc-bittorent.nix 
this derivation will be built:
  /nix/store/0117p9yhqs5qcnhlkhxz15rf3ka1yx9y-vlc-bittorrent-2.15.0.drv
building '/nix/store/0117p9yhqs5qcnhlkhxz15rf3ka1yx9y-vlc-bittorrent-2.15.0.drv'...
Running phase: unpackPhase
unpacking source archive /nix/store/c9w5y8cl1pw4szy5s4i0k8pxxpw34fc1-source
source root is source
Running phase: patchPhase
Running phase: updateAutotoolsGnuConfigScriptsPhase
Running phase: configurePhase
no configure script, doing nothing
Running phase: buildPhase
no Makefile or custom buildPhase, doing nothing
Running phase: installPhase
no Makefile or custom installPhase, doing nothing
Running phase: fixupPhase
error: builder for '/nix/store/0117p9yhqs5qcnhlkhxz15rf3ka1yx9y-vlc-bittorrent-2.15.0.drv' failed to produce output path for output 'out' at '/nix/store/0117p9yhqs5qcnhlkhxz15rf3ka1yx9y-vlc-bittorrent-2.15.0.drv.chroot/nix/store/qhzg57m6l2zymyf85ysx6w91sk23xz4c-vlc-bittorrent-2.15.0'
```

Alright, that is not gonna work, I'll actually need to specify a bit more to
Nix. For this package it clearly could not just figure out how to build it.

The error message explains what happened and with a bit of understanding about
how Nix builds packages we can decipher what went wrong. Nothing actually got
written to the build directory (`out`). In other words, it did not build
anything. Very sensible that that results in an error.

It seems that here we will actually need to specify the build steps. Shouldn't
be too bad. First, let's check
[upstream](https://github.com/johang/vlc-bittorrent?tab=readme-ov-file#building-from-git-on-a-recent-debianubuntu)
for the "official" build steps:

```sh
sudo apt-get install autoconf autoconf-archive automake libtool make \
  libvlc-dev libvlccore-dev libtorrent-rasterbar-dev g++
git clone https://github.com/johang/vlc-bittorrent.git vlc-bittorrent
cd vlc-bittorrent
autoreconf -i
./configure --prefix=/tmp/vlc
make
make install
```

Let's copy over the actual build and install commands. We already have cloning
the repo figured out declaratively with `fetchGit`.

```nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.stdenv.mkDerivation {
  pname = "vlc-bittorrent";
  version = "2.15.0";

  src = fetchGit {
    url = "https://github.com/johang/vlc-bittorrent";
    rev = "6810d479e6c1f64046d3b30efe78774b49d1c95b";
    ref = "master";
  };

  buildPhase = ''
    autoreconf -i
    ./configure --prefix=$out
    make
  '';

  installPhase = ''
    make install
  '';
}
```

We separate the commands into the build and install phases. People might want
to only build a package and not yet install it, for example. Nix supports this.

We also make a small change, we change `./configure --prefix=/tmp/vlc` to
`./configure --prefix=$out`. This is because we want the build to happen in its
own nix store path, so we specify it. `out` is a shell variable the Nix
automatically assigns to the store path it builds the package under. Something
like `out=/nix/store/k4rxnw719z628brqinrhlxmfibw1cz2q-vlc-bittorrent-2.15.0`.

```sh
$ nix-build vlc-bittorent.nix 
this derivation will be built:
  /nix/store/1i6fj0dmp4fapnigxbkfkws4pdk286nn-vlc-bittorrent-2.15.0.drv
building '/nix/store/1i6fj0dmp4fapnigxbkfkws4pdk286nn-vlc-bittorrent-2.15.0.drv'...
Running phase: unpackPhase
unpacking source archive /nix/store/c9w5y8cl1pw4szy5s4i0k8pxxpw34fc1-source
source root is source
Running phase: patchPhase
Running phase: updateAutotoolsGnuConfigScriptsPhase
Running phase: configurePhase
no configure script, doing nothing
Running phase: buildPhase
/nix/store/10i1kjjq5szjn1gp6418x8bc1hswqc90-stdenv-linux/setup: line 1553: autoreconf: command not found
error: builder for '/nix/store/1i6fj0dmp4fapnigxbkfkws4pdk286nn-vlc-bittorrent-2.15.0.drv' failed with exit code 127;
       last 9 log lines:
       > Running phase: unpackPhase
       > unpacking source archive /nix/store/c9w5y8cl1pw4szy5s4i0k8pxxpw34fc1-source
       > source root is source
       > Running phase: patchPhase
       > Running phase: updateAutotoolsGnuConfigScriptsPhase
       > Running phase: configurePhase
       > no configure script, doing nothing
       > Running phase: buildPhase
       > /nix/store/10i1kjjq5szjn1gp6418x8bc1hswqc90-stdenv-linux/setup: line 1553: autoreconf: command not found
       For full logs, run 'nix log /nix/store/1i6fj0dmp4fapnigxbkfkws4pdk286nn-vlc-bittorrent-2.15.0.drv'.
```

Looks better. Now it actually tries to build it, but does not have the
`autoreconf` tool on the PATH. We need to *actually* declare the dependencies.
This will involve a bit of hand-translating package names from Debian to Nix,
but it is not too bad.

```txt
autoconf                   ==>  autoconf
autoconf-archive           ==>  autoconf-archive
automake                   ==>  automake
g++                        ==>  gcc
libtool                    ==>  libtool
libtorrent-rasterbar-dev   ==>  libtorrent-rasterbar
libvlc-dev libvlccore-dev  ==>  libvlc
make                       ==>  gnumake
```

After listing these packages as build dependencies I have the following:

```nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.stdenv.mkDerivation {
  pname = "vlc-bittorrent";
  version = "2.15.0";

  nativeBuildInputs = with pkgs; [
    autoconf
    autoconf-archive
    automake
    gcc
    libtool
    gnumake
  ];

  buildInputs = with pkgs; [
    libtorrent-rasterbar
    libvlc
  ];

  src = fetchGit {
    url = "https://github.com/johang/vlc-bittorrent";
    rev = "6810d479e6c1f64046d3b30efe78774b49d1c95b";
    ref = "master";
  };

  buildPhase = ''
    autoreconf -i
    ./configure --prefix=$out
    make
  '';

  installPhase = ''
    make install
  '';
}
```

We separate the dependencies needed for building the package
(`nativeBuildInputs`) from the ones that are also needed to run it
(`buildInputs`). I just used my intuition about which one should go where. The
packages listed as `buildInputs` are present **both** at build time, and they
also get installed as dependecies of the package. When someone actually decides
to install it.

Technically, we could list ass dependencies in `buildInputs` and it would be
fine, but it makes sense to separate them. You would not want users to have to
install `automake` just to be able to use a torrent plugin.

*Note:* Technically, Nix does some pruning of runtime dependencies, so they
would get treated like `nativeBuildInputs` either way, but that system is not
perfect.

Okay, let's build it, hopefully that is enough.

```sh
$ nix-build vlc-bittorent.nix 
this derivation will be built:
  /nix/store/ggyycrj3dl6kimwq4bw2y4q2n7xy0myf-vlc-bittorrent-2.15.0.drv
building '/nix/store/ggyycrj3dl6kimwq4bw2y4q2n7xy0myf-vlc-bittorrent-2.15.0.drv'...
Running phase: unpackPhase
unpacking source archive /nix/store/c9w5y8cl1pw4szy5s4i0k8pxxpw34fc1-source
source root is source
Running phase: patchPhase
Running phase: updateAutotoolsGnuConfigScriptsPhase
Running phase: configurePhase
no configure script, doing nothing
Running phase: buildPhase
aclocal: warning: couldn't open directory 'm4': No such file or directory
libtoolize: putting auxiliary files in AC_CONFIG_AUX_DIR, 'build-aux'.
libtoolize: copying file 'build-aux/ltmain.sh'
libtoolize: putting macros in AC_CONFIG_MACRO_DIRS, 'm4'.
libtoolize: copying file 'm4/libtool.m4'
libtoolize: copying file 'm4/ltoptions.m4'
libtoolize: copying file 'm4/ltsugar.m4'
libtoolize: copying file 'm4/ltversion.m4'
libtoolize: copying file 'm4/lt~obsolete.m4'
libtoolize: Consider adding '-I m4' to ACLOCAL_AMFLAGS in Makefile.am.
configure.ac:32: installing 'build-aux/compile'
configure.ac:32: installing 'build-aux/config.guess'
configure.ac:32: installing 'build-aux/config.sub'
configure.ac:14: installing 'build-aux/install-sh'
configure.ac:14: installing 'build-aux/missing'
configure.ac:7: installing 'build-aux/tap-driver.sh'
src/Makefile.am: installing 'build-aux/depcomp'
checking whether the C++ compiler works... yes
checking for C++ compiler default output file name... a.out
checking for suffix of executables... 
checking whether we are cross compiling... no
checking for suffix of object files... o
checking whether the compiler supports GNU C++... yes
checking whether g++ accepts -g... yes
checking for g++ option to enable C++11 features... none needed
checking for gawk... gawk
checking for a BSD-compatible install... /nix/store/rk067yylvhyb7a360n8k1ps4lb4xsbl3-coreutils-9.3/bin/install -c
checking whether build environment is sane... yes
checking for a race-free mkdir -p... /nix/store/rk067yylvhyb7a360n8k1ps4lb4xsbl3-coreutils-9.3/bin/mkdir -p
checking whether make sets $(MAKE)... yes
checking whether make supports the include directive... yes (GNU style)
checking whether make supports nested variables... yes
checking dependency style of g++... gcc3
./configure: line 4320: syntax error near unexpected token `VLC_PLUGIN,'
./configure: line 4320: `PKG_CHECK_MODULES(VLC_PLUGIN, vlc-plugin >= 3.0.0)'
error: builder for '/nix/store/ggyycrj3dl6kimwq4bw2y4q2n7xy0myf-vlc-bittorrent-2.15.0.drv' failed with exit code 2;
       last 10 log lines:
       > checking for gawk... gawk
       > checking for a BSD-compatible install... /nix/store/rk067yylvhyb7a360n8k1ps4lb4xsbl3-coreutils-9.3/bin/install -c
       > checking whether build environment is sane... yes
       > checking for a race-free mkdir -p... /nix/store/rk067yylvhyb7a360n8k1ps4lb4xsbl3-coreutils-9.3/bin/mkdir -p
       > checking whether make sets $(MAKE)... yes
       > checking whether make supports the include directive... yes (GNU style)
       > checking whether make supports nested variables... yes
       > checking dependency style of g++... gcc3
       > ./configure: line 4320: syntax error near unexpected token `VLC_PLUGIN,'
       > ./configure: line 4320: `PKG_CHECK_MODULES(VLC_PLUGIN, vlc-plugin >= 3.0.0)'
       For full logs, run 'nix log /nix/store/ggyycrj3dl6kimwq4bw2y4q2n7xy0myf-vlc-bittorrent-2.15.0.drv'.
```

Syntax error near unexpected token? What? And in `./configure` of all places?
Not a very helpful error. The full logs are identical to what has already been
printed, to `nix log` is not helpful here.

TODO: finish

Additionally, `boost`, `pkg-config` and `openssl` are also dependencies of it.
