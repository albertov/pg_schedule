#include "postgres.h"
#include "funcapi.h"
#include "fmgr.h"
#include "pgtime.h"
#include "utils/builtins.h"
#include "datatype/timestamp.h"
#include "utils/timestamp.h"
#include "utils/datetime.h"

#include "ccronexpr.h"
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

static int
_schedule_cmp(Datum a, Datum b)
{
  const char *sa = TextDatumGetCString(a);
  const char *sb = TextDatumGetCString(b);
  return strcmp(sa, sb);
}

static enum PGScheduleError pg_schedule_parse_opt(char *s, cron_expr *target)
{
  const char *error;
  cron_parse_expr(s, target, &error);
  if (error)
    return INVALID_SCHEDULE_FORMAT;
  return NO_ERROR;
}

static enum PGScheduleError pg_schedule_parse(char *s)
{
  cron_expr target;
  return pg_schedule_parse_opt(s, &target);
}

static enum PGScheduleError pg_schedule_contains(char *s, struct pg_tm *tm, int *contained)
{
  ereport(ERROR, (errcode(ERRCODE_ASSERT_FAILURE),        \
                    errmsg("invalid input syntax for type schedule")));
  return NO_RESULT;
}

static enum PGScheduleError pg_schedule_next(char *s, TimestampTz dt, TimestampTz *result)
{
  cron_expr target;
  time_t time;
  pg_schedule_parse_opt(s, &target);
  time = cron_next(&target, timestamptz_to_time_t(dt));
  if (time == -1)
    return NOT_IN_SCHEDULE;
  *result = time_t_to_timestamptz(time);
  return NO_ERROR;
}

static enum PGScheduleError pg_schedule_previous(char *s, TimestampTz dt, TimestampTz *result)
{
  cron_expr target;
  time_t time;
  pg_schedule_parse_opt(s, &target);
  time = cron_prev(&target, timestamptz_to_time_t(dt));
  if (time == -1)
    return NOT_IN_SCHEDULE;
  *result = time_t_to_timestamptz(time);
  return NO_ERROR;
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
  TimestampTz ts_in = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz ts_out;
  CHECK_STATUS (pg_schedule_next(s, ts_in, &ts_out));
  PG_RETURN_TIMESTAMPTZ(ts_out);
}

PG_FUNCTION_INFO_V1(schedule_previous);
Datum
schedule_previous(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz ts_in = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz ts_out;
  CHECK_STATUS (pg_schedule_previous(s, ts_in, &ts_out));
  PG_RETURN_TIMESTAMPTZ(ts_out);
}

PG_FUNCTION_INFO_V1(schedule_floor);
Datum
schedule_floor(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz ts_in = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz ts_out1;
  TimestampTz ts_out2;
  CHECK_STATUS (pg_schedule_previous(s, ts_in, &ts_out1));
  CHECK_STATUS (pg_schedule_next(s, ts_in, &ts_out2));
  if (ts_out2 == ts_in)
    PG_RETURN_TIMESTAMPTZ(ts_out2);
  else
    PG_RETURN_TIMESTAMPTZ(ts_out1);
}

PG_FUNCTION_INFO_V1(schedule_ceiling);
Datum
schedule_ceiling(PG_FUNCTION_ARGS)
{
  Datum arg = PG_GETARG_DATUM(0);
  char *s = TextDatumGetCString(arg);
  TimestampTz ts_in = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz ts_out1;
  TimestampTz ts_out2;
  CHECK_STATUS (pg_schedule_next(s, ts_in, &ts_out1));
  CHECK_STATUS (pg_schedule_previous(s, ts_in, &ts_out2));
  if (ts_out2 == ts_in)
    PG_RETURN_TIMESTAMPTZ(ts_out2);
  else
    PG_RETURN_TIMESTAMPTZ(ts_out1);
}

struct schedule_series_ctx {
  cron_expr target;
  TimestampTz ts;
  TimestampTz ts_to;
};

PG_FUNCTION_INFO_V1(schedule_series);
Datum
schedule_series(PG_FUNCTION_ARGS)
{
  FuncCallContext  *srf;
  struct schedule_series_ctx *ctx;
  TimestampTz ts;
  Datum result;

  if (SRF_IS_FIRSTCALL()) {
    Datum arg = PG_GETARG_DATUM(0);
    char *s = TextDatumGetCString(arg);
    srf = SRF_FIRSTCALL_INIT();
    srf->user_fctx = MemoryContextAlloc(srf->multi_call_memory_ctx, sizeof(struct schedule_series_ctx));
    ctx = (struct schedule_series_ctx *) srf->user_fctx;
    ctx->ts = PG_GETARG_TIMESTAMPTZ(1);
    ctx->ts_to = PG_GETARG_TIMESTAMPTZ(2);
    pg_schedule_parse_opt(s, &(ctx->target));
  }

  srf = SRF_PERCALL_SETUP();
  ctx = (struct schedule_series_ctx *) srf->user_fctx;
  ts = time_t_to_timestamptz(cron_next(&(ctx->target), timestamptz_to_time_t(ctx->ts)));
  if (ts > ctx->ts_to) {
    SRF_RETURN_DONE(srf);
  } else {
    result = TimestampTzGetDatum(ts);
    ctx->ts = ts;
    SRF_RETURN_NEXT(srf, result);
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
