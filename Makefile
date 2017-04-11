GHC = ghc
C2HS = c2hs
LIBS=-lHSrts -lCffi -lrt

PG_INCLUDE = $(shell pg_config --includedir-server)
CFLAGS += -I$(PG_INCLUDE)
GHCFLAGS  = $(CFLAGS)  -Wall -O2 -fPIC -optc-fPIC

all: schedule.so

schedule.so: schedule.c hsbracket.c PGSchedule.hs
PGSchedule_stub.h: PGSchedule.hs
schedule.c: PGSchedule_stub.h

%.hs: %.chs
	$(C2HS) -C "-I$(PG_INCLUDE)"  $^
%.so:
	$(GHC) --make -shared $(GHCFLAGS) $^ -o $@ $(LIBS) -fforce-recomp

%_stub.h:
	$(GHC) --make -c $(GHCFLAGS) -O0 $<

clean:
	rm -rf *.o *.dyn_o *.hi *.dyn_hi *.so *_o_split *_stub.h .cabal-sandbox cabal.sandbox.config PGSchedule.hs
