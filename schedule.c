#include "postgres.h"
#include "fmgr.h"
#include "pgtime.h"
#include "utils/builtins.h"
#include "datatype/timestamp.h"
#include "utils/timestamp.h"
#include "PGSchedule_stub.h"

PG_MODULE_MAGIC;

typedef struct varlena scheduletype;


#define DatumGetScheduleP(X)      ((scheduletype *) PG_DETOAST_DATUM(X))
#define DatumGetSchedulePP(X)     ((scheduletype *) PG_DETOAST_DATUM_PACKED(X))
#define SchedulePGetDatum(X)      PointerGetDatum(X)

#define PG_GETARG_SCHEDULE_P(n)   DatumGetScheduleP(PG_GETARG_DATUM(n))
#define PG_GETARG_SCHEDULE_PP(n)  DatumGetSchedulePP(PG_GETARG_DATUM(n))
#define PG_RETURN_SCHEDULE_P(x)   PG_RETURN_POINTER(x)

static void
parse_schedule(const char *s)
{
  if (!pg_schedule_parse((HsPtr*)s)) {
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                    errmsg("invalid input syntax for type schedule"))); 
  }
}


PG_FUNCTION_INFO_V1(schedule_in);
Datum
schedule_in(PG_FUNCTION_ARGS)
{
  char *s = PG_GETARG_CSTRING(0);
  scheduletype *vardata;

  parse_schedule(s);
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

  PG_RETURN_BOOL(pg_schedule_contains(s, tm)==1? TRUE : FALSE);
}
