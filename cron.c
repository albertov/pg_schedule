#include "cron.h"

#if !defined(LINT) && !defined(lint)
char *copyright[] = {
    "@(#) Copyright 1988,1989,1990,1993,1994 by Paul Vixie",
    "@(#) All rights reserved"};
#endif

char *MonthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    NULL};

char *DowNames[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
    NULL};

char *ecodes[] = {
    "no error",
    "bad minute",
    "bad hour",
    "bad day-of-month",
    "bad month",
    "bad day-of-week",
    "bad command",
    "bad time specifier",
    "bad username",
    "command too long",
    NULL};