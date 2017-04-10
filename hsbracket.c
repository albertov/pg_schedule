#include <HsFFI.h>
#include "Rts.h"
#include <stdio.h>

extern void __stginit_PGSchedule ( void );

static void pg_schedule_enter(void) __attribute__((constructor));
static void pg_schedule_enter(void)
{
  static char *argv[] = { "schedule.so", 0 }, **argv_ = argv;
  char **args  = argv;
  static int argc = 1;
  RtsConfig conf = defaultRtsConfig;
  conf.rts_opts_enabled = RtsOptsAll;
  hs_init_ghc(&argc, &args, conf);
  hs_add_root(__stginit_PGSchedule);
}

static void pg_schedule_exit(void) __attribute__((destructor));
static void pg_schedule_exit(void)
{
  hs_exit();
}
