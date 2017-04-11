{ stdenv
, postgresql
, haskellPackages
, linkStatically ? false
}:

let
  ghc = haskellPackages.ghcWithPackages (p: with p; [ sigym4-dimension c2hs ]);
in
stdenv.mkDerivation rec {
  version = "1.0";
  shortname = "pg_schedule";
  name = "${shortname}-${version}";

  src = stdenv.lib.cleanSource ./.;

  buildInputs = [ postgresql ghc ];

  buildPhase = ''
    make LINK_STATICALLY="${if linkStatically then "YES" else "NO"}"
  '';

  installPhase = ''
    mkdir -p $out/bin   # for buildEnv to setup proper symlinks
    strip --strip-all schedule.so
    install -D schedule.so -t $out/lib/
    install -D ./{schedule--1.0.sql,schedule.control} -t $out/share/extension
  '';

  meta = {
    description = "Cron schedule type for postgresql";
    homepage = https://github.com/albertov/pg_schedule;
    license = stdenv.lib.licenses.mit;
  };
}
