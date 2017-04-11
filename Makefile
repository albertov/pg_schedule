GHC = ghc
C2HS = c2hs
ifeq ($(LINK_STATICALLY),YES)
  LIBS=-lHSrts_thr -lCffi
  GHCFLAGS = -fPIC -optc-fPIC
else
  LIBS=-lHSrts_thr-ghc$(shell ghc --numeric-version)
  GHCFLAGS=-dynamic
endif

PG_INCLUDE = $(shell pg_config --includedir-server)
CFLAGS += -I$(PG_INCLUDE)
GHCFLAGS += $(CFLAGS) -Wall -O2

all: schedule.so

schedule.so: schedule.c hsbracket.c PGSchedule.hs
PGSchedule_stub.h: PGSchedule.hs
schedule.c: PGSchedule_stub.h

%.hs: %.chs
	$(C2HS) -C "-I$(PG_INCLUDE)"  $^
%.so:
	$(GHC) --make -shared $(GHCFLAGS) $^ -o $@ $(LIBS) -fforce-recomp

%_stub.h:
	$(GHC) --make -c $(CFLAGS) -O0 $<

clean:
	rm -rf *.o *.dyn_o *.hi *.dyn_hi *.so *_o_split *_stub.h .cabal-sandbox cabal.sandbox.config PGSchedule.hs
