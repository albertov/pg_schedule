GHC = ghc
#LIBS=-lHSrts_thr-ghc8.0.2 -lCffi_thr -lrt
CFLAGS = $(shell pg_config --cflags)
CFLAGS += -I $(shell pg_config --includedir-server)
GHCFLAGS  = $(CFLAGS) -Wall -O2 -dynamic

all: schedule.so

schedule.so: schedule.c

%.so:
	$(CC) -fPIC -shared $(CFLAGS)  $^ -o $@ $(LIBS)
	#$(GHC) --make -shared $(GHCFLAGS) $^ -o $@ $(LIBS)


%_stub.h: %.hs
	$(GHC) -c $(GHCFLAGS) -O0 $< -o /dev/null

clean:
	rm -rf *.o *.dyn_o *.hi *.dyn_hi *.so *_o_split *_stub.h .cabal-sandbox cabal.sandbox.config
