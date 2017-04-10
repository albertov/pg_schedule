-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION schedule" to load this file. \quit

SET client_min_messages = warning;

CREATE TYPE schedule;

CREATE FUNCTION schedule_in(cstring) RETURNS schedule
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_out(schedule) RETURNS cstring
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE TYPE schedule (
    INTERNALLENGTH = -1,
    INPUT = schedule_in,
    OUTPUT = schedule_out
);


CREATE CAST (schedule AS text) WITH INOUT AS ASSIGNMENT;
CREATE CAST (text AS schedule) WITH INOUT AS ASSIGNMENT;

CREATE FUNCTION schedule_contains(schedule, timestamptz) RETURNS bool
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_next(schedule, timestamptz) RETURNS timestamptz
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_previous(schedule, timestamptz) RETURNS timestamptz
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_floor(schedule, timestamptz) RETURNS timestamptz
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_ceiling(schedule, timestamptz) RETURNS timestamptz
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_series(schedule, timestamptz, timestamptz) RETURNS SETOF timestamptz
    IMMUTABLE
    STRICT
    LANGUAGE C
    COST 100
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_lt(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_le(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_eq(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_ne(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_ge(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_gt(schedule, schedule) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_cmp(schedule, schedule) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE OPERATOR < (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = >,
    NEGATOR = >=,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = schedule_lt
);

CREATE OPERATOR <= (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = >=,
    NEGATOR = >,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = schedule_le
);

CREATE OPERATOR = (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = =,
    NEGATOR = <>,
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES,
    MERGES,
    PROCEDURE = schedule_eq
);

CREATE OPERATOR <> (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = <>,
    NEGATOR = =,
    RESTRICT = neqsel,
    JOIN = neqjoinsel,
    PROCEDURE = schedule_ne
);

CREATE OPERATOR >= (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = <=,
    NEGATOR = <,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = schedule_ge
);

CREATE OPERATOR > (
    LEFTARG = schedule,
    RIGHTARG = schedule,
    COMMUTATOR = <,
    NEGATOR = <=,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = schedule_gt
);

CREATE OPERATOR CLASS schedule_ops
    DEFAULT FOR TYPE schedule USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       schedule_cmp(schedule, schedule);
