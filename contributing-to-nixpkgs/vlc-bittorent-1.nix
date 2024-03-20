{ pkgs ? import <nixpkgs> { } }:

pkgs.stdenv.mkDerivation {
  pname = "vlc-bittorrent";
  version = "2.15.0";

  src = fetchGit {
    url = "https://github.com/johang/vlc-bittorrent";
    rev = "6810d479e6c1f64046d3b30efe78774b49d1c95b";
  };
}
