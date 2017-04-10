-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION schedule" to load this file. \quit

SET client_min_messages = warning;

CREATE TYPE schedule;

CREATE FUNCTION schedule_in(cstring) RETURNS schedule
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS 'MODULE_PATHNAME';

CREATE FUNCTION schedule_out(schedule) RETURNS cstring
    IMMUTABLE
    STRICT
    LANGUAGE C
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
    AS 'MODULE_PATHNAME';
