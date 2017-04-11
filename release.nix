{ pkgs ? let revision = "8f45ea61751509107a35dc00cad6721e56aa995a";
         in import (fetchTarball "https://github.com/NixOS/nixpkgs-channels/archive/${revision}.tar.gz") {}
, postgresql ? pkgs.postgresql
  # Wether to link haskell libraries statically to the extension.
  # Requires a fPIC version of GHC and all haskell libraries.
, linkStatically ? true
}:
let
  # We need a version of GHC built with -fPIC to be able to link
  # haskell libraries statically while being able to build a shared
  # library for postgres to load
  ghc-PIC =
    let
      config = builtins.toFile "build.mk" ''
        SRC_HC_OPTS   += -fPIC
        SRC_CC_OPTS   += -fPIC
        '';
    in
      pkgs.lib.overrideDerivation pkgs.haskell.compiler.ghc802 (o: {
        postPatch = "cp ${config} mk/build.mk";
      });
  hsLib = import "${pkgs.path}/pkgs/development/haskell-modules/lib.nix"
            { inherit pkgs; };
  haskellPackages = pkgs.haskell.packages.ghc802.override {
    ghc = if linkStatically then ghc-PIC else pkgs.haskell.compiler.ghc802;
    overrides = self: super: {
      mkDerivation = drv: super.mkDerivation ( drv // {
        configureFlags =
          let originalFlags = drv.configureFlags or [];
              fPICFlags = (if self.ghc.isGhcjs or false then [] else [
                "--ghc-option=-fPIC"
                "--ghc-option=-optc-fPIC"
                ]);
          in originalFlags ++ (if linkStatically then fPICFlags else []);
        });
      sigym4-dimension = self.callPackage 
        (pkgs.fetchFromGitHub {
            owner = "meteogrid";
            repo  = "sigym4-dimension";
            rev   = "2cf4450fb474ad1a15ec24b69b9465aab430b6e6";
            sha256 = "07sagp070bykcvha7wsscms856p6xna8dsrczspabh967qszp8vp";
          }) {};
      iso8601-duration = self.callPackage 
        (pkgs.fetchFromGitHub {
            owner = "meteogrid";
            repo  = "iso8601-duration";
            rev   = "12fd6bf23c08bc62044d0a0ac545392898f8eb8b";
            sha256 = "0bxs3b757nipz1z0h27sb3ghhfr4kz53hz4bsmvklv5ar5sw7wvp";
          }) {};
      cron = hsLib.dontCheck (self.callPackage 
        (pkgs.fetchFromGitHub {
            owner = "meteogrid";
            repo  = "cron";
            rev   = "d4947b19b8165bd4d4346ad5bf20c43536ab67ce";
            sha256 = "1hiv8rvba80nhz4rsiscwygk1imj37gbg920gag3g4iicaa77aa5";
          }) {});
    };
  };
in
{
  pg_schedule = pkgs.callPackage ./. {
    inherit haskellPackages postgresql linkStatically;
  };
}
