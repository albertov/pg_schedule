module PGTime where

import Data.Time
import Data.Time.Calendar.OrdinalDate
import Data.Time.Calendar.WeekDate

import Foreign.Storable
import Foreign.Ptr (nullPtr)


#include "postgres.h"
#include "pgtime.h"

newtype PGTime = PGTime { unPGTime :: UTCTime }
  deriving Show

{#pointer * pg_tm as PGTime nocode#}

instance Storable PGTime where
  sizeOf _ = {#sizeof pg_tm #}
  alignment _ = {#alignof pg_tm #}
  peek = fmap PGTime . peekUTCTime
    where
      peekUTCTime p =
        UTCTime <$> peekDay p <*> (timeOfDayToTime <$> peekTimeOfDay p)
      peekDay p = do
        y <- (+1900) . fromIntegral <$> {#get pg_tm->tm_year#} p
        m <- (+1) . fromIntegral <$> {#get pg_tm->tm_mon#} p
        d <- fromIntegral <$> {#get pg_tm->tm_mday#} p
        return (fromGregorian y m d)
      peekTimeOfDay p = do
        h <- fromIntegral <$> {#get pg_tm->tm_hour#} p
        m <- fromIntegral <$> {#get pg_tm->tm_min#} p
        s <- fromIntegral <$> {#get pg_tm->tm_sec#} p
        return (TimeOfDay h m s)
        {-
      peekZonedTime p =
        ZonedTime <$> (LocalTime <$> peekDay p <*> peekTimeOfDay p)
                  <*> peekTimeZone p
      peekTimeZone p = do
        mins <- fromIntegral <$> {#get pg_tm->tm_gmtoff#} p
        isDst <- (\v -> if v==1 then True else False) <$> {#get pg_tm->tm_isdst#} p
        return (TimeZone mins isDst "")
        -}
{-
         struct pg_tm
   26 {
   27     int         tm_sec;
   28     int         tm_min;
   29     int         tm_hour;
   30     int         tm_mday;
   31     int         tm_mon;         /* origin 1, not 0! */
   32     int         tm_year;        /* relative to 1900 */
   33     int         tm_wday;
   34     int         tm_yday;
   35     int         tm_isdst;
   36     long int    tm_gmtoff;
   37     const char *tm_zone;
   38 };
-}
  poke p (PGTime (UTCTime day time)) = do
    let (TimeOfDay h m s) = timeToTimeOfDay time
    {#set pg_tm->tm_sec#} p (round s)
    {#set pg_tm->tm_min#} p (fromIntegral m)
    {#set pg_tm->tm_hour#} p (fromIntegral h)
    let (y,mth,d) = toGregorian day
    {#set pg_tm->tm_mday#} p (fromIntegral d)
    {#set pg_tm->tm_mon#} p (fromIntegral mth - 1)
    {#set pg_tm->tm_year#} p (fromIntegral y - 1900)
    let (_,_,wday) = toWeekDate day
    {#set pg_tm->tm_wday#} p (fromIntegral wday)
    {#set pg_tm->tm_yday#} p (fromIntegral (snd (toOrdinalDate day)))
    {#set pg_tm->tm_isdst#} p 0
    {#set pg_tm->tm_gmtoff#} p 0
    {#set pg_tm->tm_zone#} p nullPtr --We won't be able to free anything we allocate here so we dont
