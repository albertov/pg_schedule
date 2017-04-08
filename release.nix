{ pkgs ? import <nixpkgs> {}
, postgresql ? pkgs.postgresql
}:
let
  haskellPackages = (import ../sigym4 {}).haskellPackages;
in
{
  pg_schedule = pkgs.callPackage ./. { inherit haskellPackages postgresql; };
}
