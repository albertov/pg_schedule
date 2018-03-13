#include <HsFFI.h>
#include "Rts.h"
#include <stdio.h>

static void pg_schedule_enter(void) __attribute__((constructor));
static void pg_schedule_enter(void)
{
  static char *argv[] = { "schedule.so", 0 }, **argv_ = argv;
  static int argc = 1;
  RtsConfig conf = defaultRtsConfig;
  conf.rts_opts_enabled = RtsOptsAll;
  hs_init_ghc(&argc, &argv_, conf);
}

static void pg_schedule_exit(void) __attribute__((destructor));
static void pg_schedule_exit(void)
{
  hs_exit();
}
