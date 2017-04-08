-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION schedule" to load this file. \quit

CREATE FUNCTION schedule_world()
    RETURNS text
    AS 'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION schedule_text_arg(text)
    RETURNS text
    AS 'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION schedule_ereport()
    RETURNS void
    AS 'MODULE_PATHNAME'
    LANGUAGE C;
