# pg_schedule

Provides a cron-formatted 'schedule' type for postgresql

WARNING: All calculations are done internally in UTC so a schedule
like `0 0 * * *` will match every midnight on UTC which might not
be what you want, eg: you wont be able to (easily) match the same
time every day on localtime across DST changes.

However, this behaviour is usually what you want for sensor readings,
meteorological predictions, etc..

## Install

### Nixos

Clone this repo to /this/repo

Add to `/etc/configuration.nix`:

```nix
  services.postgresql = {
    enable = true;
    ... other config
    extraPlugins = [
      (import /this/repo/release.nix { postgresql = pkgs.postgresql95; }).pg_schedule
          
    ];
  };
```

### Ubuntu

TBD


## Functions

### schedule_contains

`schedule_contains(schedule, timestamptz) RETURNS bool`

Wether a timestamptz belongs to a schedule.

### schedule_next

`schedule_next(schedule, timestamptz) RETURNS timestamptz`

Returns the next strictly greater-than timestamp from a schedule.

### schedule_previous

`schedule_previous(schedule, timestamptz) RETURNS timestamptz`

Returns the next strictly lesser-than timestamp from a schedule.

### schedule_floor

`schedule_floor(schedule, timestamptz) RETURNS timestamptz`

Returns the next lesser-than-or-equal timestamp from a schedule.


### schedule_ceiling

`schedule_ceiling(schedule, timestamptz) RETURNS timestamptz`

Returns the next greater-than-or-equal timestamp from a schedule.

### schedule_series

`schedule_series(schedule, timestamptz, timestamptz) RETURNS SETOF timestamptz`


Generates the set of timestamps belonging to a schedule between two timestamps.

## Examples

### Runtime, horizon and times from the last 24 hours


```sql
SELECT                                                                                                                                                                                         
  run_time,
  horizon,
  run_time + ('1 hour'::interval * horizon) AS time
FROM 
  schedule_series('0 */6 * * *', (CURRENT_TIMESTAMP - '1 days'::interval), CURRENT_TIMESTAMP) run_time,
  generate_series(0, 6) horizon;
        run_time        | horizon |          time          
------------------------+---------+------------------------
 2017-04-09 12:00:00+00 |       0 | 2017-04-09 12:00:00+00
 2017-04-09 12:00:00+00 |       1 | 2017-04-09 13:00:00+00
 2017-04-09 12:00:00+00 |       2 | 2017-04-09 14:00:00+00
 2017-04-09 12:00:00+00 |       3 | 2017-04-09 15:00:00+00
 2017-04-09 12:00:00+00 |       4 | 2017-04-09 16:00:00+00
 2017-04-09 12:00:00+00 |       5 | 2017-04-09 17:00:00+00
 2017-04-09 12:00:00+00 |       6 | 2017-04-09 18:00:00+00
 2017-04-09 18:00:00+00 |       0 | 2017-04-09 18:00:00+00
 2017-04-09 18:00:00+00 |       1 | 2017-04-09 19:00:00+00
 2017-04-09 18:00:00+00 |       2 | 2017-04-09 20:00:00+00
 2017-04-09 18:00:00+00 |       3 | 2017-04-09 21:00:00+00
 2017-04-09 18:00:00+00 |       4 | 2017-04-09 22:00:00+00
 2017-04-09 18:00:00+00 |       5 | 2017-04-09 23:00:00+00
 2017-04-09 18:00:00+00 |       6 | 2017-04-10 00:00:00+00
 2017-04-10 00:00:00+00 |       0 | 2017-04-10 00:00:00+00
 2017-04-10 00:00:00+00 |       1 | 2017-04-10 01:00:00+00
 2017-04-10 00:00:00+00 |       2 | 2017-04-10 02:00:00+00
 2017-04-10 00:00:00+00 |       3 | 2017-04-10 03:00:00+00
 2017-04-10 00:00:00+00 |       4 | 2017-04-10 04:00:00+00
 2017-04-10 00:00:00+00 |       5 | 2017-04-10 05:00:00+00
 2017-04-10 00:00:00+00 |       6 | 2017-04-10 06:00:00+00
 2017-04-10 06:00:00+00 |       0 | 2017-04-10 06:00:00+00
 2017-04-10 06:00:00+00 |       1 | 2017-04-10 07:00:00+00
 2017-04-10 06:00:00+00 |       2 | 2017-04-10 08:00:00+00
 2017-04-10 06:00:00+00 |       3 | 2017-04-10 09:00:00+00
 2017-04-10 06:00:00+00 |       4 | 2017-04-10 10:00:00+00
 2017-04-10 06:00:00+00 |       5 | 2017-04-10 11:00:00+00
 2017-04-10 06:00:00+00 |       6 | 2017-04-10 12:00:00+00
 2017-04-10 12:00:00+00 |       0 | 2017-04-10 12:00:00+00
 2017-04-10 12:00:00+00 |       1 | 2017-04-10 13:00:00+00
 2017-04-10 12:00:00+00 |       2 | 2017-04-10 14:00:00+00
 2017-04-10 12:00:00+00 |       3 | 2017-04-10 15:00:00+00
 2017-04-10 12:00:00+00 |       4 | 2017-04-10 16:00:00+00
 2017-04-10 12:00:00+00 |       5 | 2017-04-10 17:00:00+00
 2017-04-10 12:00:00+00 |       6 | 2017-04-10 18:00:00+00
(35 rows)

Time: 0.373 ms
```

### Best choices for a (run_time, horizon) that covers current time


```sql
SELECT
  run_time,
  horizon,
  run_time + ('1 hour'::interval * horizon) AS time
FROM 
  schedule_series('0 */6 * * *', (CURRENT_TIMESTAMP - '3 days'::interval), CURRENT_TIMESTAMP) run_time,
  generate_series(0, 36) horizon
WHERE run_time + ('1 hour'::interval * horizon) = schedule_floor('0 * * * *', CURRENT_TIMESTAMP)
ORDER BY horizon ASC;
        run_time        | horizon |          time          
------------------------+---------+------------------------
 2017-04-10 12:00:00+00 |       2 | 2017-04-10 14:00:00+00
 2017-04-10 06:00:00+00 |       8 | 2017-04-10 14:00:00+00
 2017-04-10 00:00:00+00 |      14 | 2017-04-10 14:00:00+00
 2017-04-09 18:00:00+00 |      20 | 2017-04-10 14:00:00+00
 2017-04-09 12:00:00+00 |      26 | 2017-04-10 14:00:00+00
 2017-04-09 06:00:00+00 |      32 | 2017-04-10 14:00:00+00
(6 rows)

Time: 7.524 ms
```

The above query can be slow since the query planner apparently
can't figure out `schedule_floor(current_timestamp)` is constant
for the duration of the transaction and re-computes it for every row. 
It can be optimized as:

```sql

WITH closest_time AS (SELECT schedule_floor('0 * * * *', current_timestamp))
SELECT
  run_time,
  horizon,
  run_time + ('1 hour'::interval * horizon) AS time
FROM 
  schedule_series('0 */6 * * *', (CURRENT_TIMESTAMP - '3 days'::interval), CURRENT_TIMESTAMP) run_time,
  generate_series(0, 36) horizon
WHERE run_time + ('1 hour'::interval * horizon) = (SELECT * FROM closest_time)
ORDER BY horizon ASC;
        run_time        | horizon |          time          
------------------------+---------+------------------------
 2017-04-10 12:00:00+00 |       2 | 2017-04-10 14:00:00+00
 2017-04-10 06:00:00+00 |       8 | 2017-04-10 14:00:00+00
 2017-04-10 00:00:00+00 |      14 | 2017-04-10 14:00:00+00
 2017-04-09 18:00:00+00 |      20 | 2017-04-10 14:00:00+00
 2017-04-09 12:00:00+00 |      26 | 2017-04-10 14:00:00+00
 2017-04-09 06:00:00+00 |      32 | 2017-04-10 14:00:00+00
(6 rows)

Time: 0.597 ms
```
