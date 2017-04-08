#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(schedule_world);
PG_FUNCTION_INFO_V1(schedule_text_arg);
PG_FUNCTION_INFO_V1(schedule_ereport);

Datum
schedule_world(PG_FUNCTION_ARGS)
{
	PG_RETURN_TEXT_P(cstring_to_text("Schedule, World!"));
}

Datum
schedule_text_arg(PG_FUNCTION_ARGS)
{
	text *schedule		= cstring_to_text("Schedule, ");
	int32 schedule_sz	= VARSIZE(schedule) - VARHDRSZ;

	text *name		= PG_GETARG_TEXT_P(0);
	int32 name_sz	= VARSIZE(name) - VARHDRSZ;

	text *tail		= cstring_to_text("!");
	int32 tail_sz	= VARSIZE(tail) - VARHDRSZ;

	int32 out_sz	= schedule_sz + name_sz + tail_sz + VARHDRSZ;
	text *out		= (text *) palloc(out_sz);

	SET_VARSIZE(out, out_sz);

	memcpy(VARDATA(out), VARDATA(schedule), schedule_sz);
	memcpy(VARDATA(out) + schedule_sz, VARDATA(name), name_sz);
	memcpy(VARDATA(out) + schedule_sz + name_sz, VARDATA(tail), tail_sz);

	PG_RETURN_TEXT_P(out);
}

Datum
schedule_ereport(PG_FUNCTION_ARGS)
{
	ereport(ERROR,
			(errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
			errmsg("null value not allowed")));

	PG_RETURN_VOID();
}
