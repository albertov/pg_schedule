#include "postgres.h"
#include "funcapi.h"
#include "fmgr.h"
#include "pgtime.h"
#include "utils/builtins.h"
#include "datatype/timestamp.h"
#include "utils/timestamp.h"
#include "utils/datetime.h"

#include "cron.h"
#include "schedule.h"

PG_MODULE_MAGIC;

typedef struct varlena scheduletype;


#define DatumGetScheduleP(X)      ((scheduletype *) PG_DETOAST_DATUM(X))
#define DatumGetSchedulePP(X)     ((scheduletype *) PG_DETOAST_DATUM_PACKED(X))
#define SchedulePGetDatum(X)      PointerGetDatum(X)

#define PG_GETARG_SCHEDULE_P(n)   DatumGetScheduleP(PG_GETARG_DATUM(n))
#define PG_GETARG_SCHEDULE_PP(n)  DatumGetSchedulePP(PG_GETARG_DATUM(n))
#define PG_RETURN_SCHEDULE_P(x)   PG_RETURN_POINTER(x)
#define CHECK_STATUS(x)           switch(x) {                            \
  case NO_ERROR:                                                         \
    break;                                                               \
  case INVALID_SCHEDULE_FORMAT:                                          \
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),        \
                    errmsg("invalid input syntax for type schedule")));  \
  case NOT_IN_SCHEDULE:                                                  \
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),          \
                      errmsg("parameter does not belong to schedule"))); \
  case NO_RESULT:                                                        \
      PG_RETURN_NULL();                                                  \
  }


static enum PGScheduleError pg_schedule_parse(char *s)
{
  if (parse_cron_entry(s))
    return NO_ERROR;
  return INVALID_SCHEDULE_FORMAT;
}


static enum PGScheduleError pg_schedule_contains(char *s, struct pg_tm *tm, int *contained)
{
  return NO_RESULT;
}


static enum PGScheduleError pg_schedule_next(char *s, struct pg_tm *tm, struct pg_tm *result)
{
  return NO_RESULT;
}


static enum PGScheduleError pg_schedule_previous(char *s, struct pg_tm *tm, struct pg_tm *result)
{
  return NO_RESULT;
}


static enum PGScheduleError pg_schedule_floor(char *s, struct pg_tm *tm, struct pg_tm *result)
{
  return NO_RESULT;
}


static enum PGScheduleError pg_schedule_ceiling(char *s, struct pg_tm *tm, struct pg_tm *result)
{
  return NO_RESULT;
}


static enum PGScheduleError pg_schedule_series(char *s, struct pg_tm *tmFrom, struct pg_tm *tmTo, void *user_fctx, uint64 *max_calls)
{
  return NO_RESULT;
}


static void pg_schedule_free_series(void *user_fctx) {

}


static TimestampTz to_timestamptz(struct pg_tm tm)
{
  tm.tm_mon += 1;
  tm.tm_year += 1900;
  Timestamp result;
  if (tm2timestamp(&tm, 0, NULL, &result) != 0) {
      ereport(ERROR, (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                      errmsg("timestamp is out of range"))); 
  }
  return (TimestampTz) result;
}


static int
_schedule_cmp(Datum a, Datum b)
{
  const char *sa = TextDatumGetCString(a);
  const char *sb = TextDatumGetCString(b);
  return strcmp(sa, sb);
}



PG_FUNCTION_INFO_V1(schedule_in);
Datum
schedule_in(PG_FUNCTION_ARGS)
{
  char *s = PG_GETARG_CSTRING(0);
  scheduletype *vardata;

  CHECK_STATUS (pg_schedule_parse(s))
  vardata = (scheduletype *) cstring_to_text(s);
  PG_RETURN_SCHEDULE_P(vardata);
}

PG_FUNCTION_INFO_V1(schedule_out);
Datum
schedule_out(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);

  PG_RETURN_CSTRING(TextDatumGetCString(arg));
}

PG_FUNCTION_INFO_V1(schedule_contains);
Datum
schedule_contains(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz dt = PG_GETARG_TIMESTAMPTZ(1);
  pg_time_t pgt = timestamptz_to_time_t(dt);
  struct pg_tm *tm = pg_gmtime(&pgt);
  int contained;
  CHECK_STATUS (pg_schedule_contains(s, tm, &contained));
  PG_RETURN_BOOL(contained==1? true : false);
}

PG_FUNCTION_INFO_V1(schedule_next);
Datum
schedule_next(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz dt = PG_GETARG_TIMESTAMPTZ(1);
  pg_time_t pgt = timestamptz_to_time_t(dt);
  struct pg_tm *tm = pg_gmtime(&pgt);
  struct pg_tm result;
  CHECK_STATUS (pg_schedule_next(s, tm, &result));
  PG_RETURN_TIMESTAMPTZ(to_timestamptz(result));
}

PG_FUNCTION_INFO_V1(schedule_previous);
Datum
schedule_previous(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz dt = PG_GETARG_TIMESTAMPTZ(1);
  pg_time_t pgt = timestamptz_to_time_t(dt);
  struct pg_tm *tm = pg_gmtime(&pgt);
  struct pg_tm result;
  CHECK_STATUS (pg_schedule_previous(s, tm, &result));
  PG_RETURN_TIMESTAMPTZ(to_timestamptz(result));
}

PG_FUNCTION_INFO_V1(schedule_floor);
Datum
schedule_floor(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz dt = PG_GETARG_TIMESTAMPTZ(1);
  pg_time_t pgt = timestamptz_to_time_t(dt);
  struct pg_tm *tm = pg_gmtime(&pgt);
  struct pg_tm result;
  CHECK_STATUS (pg_schedule_floor(s, tm, &result));
  PG_RETURN_TIMESTAMPTZ(to_timestamptz(result));
}

PG_FUNCTION_INFO_V1(schedule_ceiling);
Datum
schedule_ceiling(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz dt = PG_GETARG_TIMESTAMPTZ(1);
  pg_time_t pgt = timestamptz_to_time_t(dt);
  struct pg_tm *tm = pg_gmtime(&pgt);
  struct pg_tm result;
  CHECK_STATUS (pg_schedule_ceiling(s, tm, &result));
  PG_RETURN_TIMESTAMPTZ(to_timestamptz(result));
}

PG_FUNCTION_INFO_V1(schedule_series);
Datum
schedule_series(PG_FUNCTION_ARGS)
{
  FuncCallContext  *srf;

  if (SRF_IS_FIRSTCALL()) {
    Datum arg = PG_GETARG_DATUM(0);
    char *s = TextDatumGetCString(arg);

    pg_time_t pgtFrom = timestamptz_to_time_t(PG_GETARG_TIMESTAMPTZ(1));
    struct pg_tm tmFrom = *pg_gmtime(&pgtFrom);

    pg_time_t pgtTo = timestamptz_to_time_t(PG_GETARG_TIMESTAMPTZ(2));
    struct pg_tm tmTo = *pg_gmtime(&pgtTo);

    srf = SRF_FIRSTCALL_INIT();

    srf->user_fctx = MemoryContextAlloc(srf->multi_call_memory_ctx, sizeof(struct pg_tm*));
    CHECK_STATUS (pg_schedule_series(s, &tmFrom, &tmTo, &srf->user_fctx, &srf->max_calls));
  }

  srf = SRF_PERCALL_SETUP();

  if (srf->call_cntr < srf->max_calls) {
    struct pg_tm *times = (struct pg_tm *) srf->user_fctx;
    Datum result = TimestampTzGetDatum(to_timestamptz(times[srf->call_cntr]));
    SRF_RETURN_NEXT(srf, result);
  } else {
    pg_schedule_free_series(srf->user_fctx);
    SRF_RETURN_DONE(srf);
  }
}

PG_FUNCTION_INFO_V1(schedule_lt);
Datum
schedule_lt(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) < 0);
}

PG_FUNCTION_INFO_V1(schedule_le);
Datum
schedule_le(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) <= 0);
}

PG_FUNCTION_INFO_V1(schedule_eq);
Datum
schedule_eq(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) == 0);
}

PG_FUNCTION_INFO_V1(schedule_ne);
Datum
schedule_ne(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) != 0);
}

PG_FUNCTION_INFO_V1(schedule_ge);
Datum
schedule_ge(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) >= 0);
}

PG_FUNCTION_INFO_V1(schedule_gt);
Datum
schedule_gt(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_BOOL(_schedule_cmp(arg1, arg2) > 0);
}

PG_FUNCTION_INFO_V1(schedule_cmp);
Datum
schedule_cmp(PG_FUNCTION_ARGS)
{
        Datum arg1 = PG_GETARG_DATUM(0);
        Datum arg2 = PG_GETARG_DATUM(1);

        PG_RETURN_INT32(_schedule_cmp(arg1, arg2));
}
