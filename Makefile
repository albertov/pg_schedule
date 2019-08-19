MODULE_big = schedule
OBJS = schedule.o \
	$(WIN32RES)

EXTENSION = schedule
DATA = schedule--1.0.sql
PGFILEDESC = "pg_schedule - Provides a cron-formatted 'schedule' type"

HEADERS = schedule.h

REGRESS = schedule

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)