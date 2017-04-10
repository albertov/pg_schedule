module PGSchedule () where

import Data.Time

import Foreign.C.Types
import Foreign.C.String
import Foreign.Ptr
import Foreign.Storable
import Foreign.Marshal.Alloc (free)
import Foreign.Marshal.Array (newArray)
import Data.ByteString.Unsafe (unsafePackCString)
import qualified Data.Text.Encoding as T
import Sigym4.Dimension.CronSchedule
import Sigym4.Dimension

#include "postgres.h"
#include "pgtime.h"
#include "schedule.h"

{#enum PGScheduleError {} deriving (Eq,Show) #}


newtype PGTime = PGTime UTCTime deriving Show

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

  poke p (PGTime (UTCTime day time)) = do
    let (TimeOfDay h m s) = timeToTimeOfDay time
    {#set pg_tm->tm_sec#} p (round s)
    {#set pg_tm->tm_min#} p (fromIntegral m)
    {#set pg_tm->tm_hour#} p (fromIntegral h)
    let (y,mth,d) = toGregorian day
    {#set pg_tm->tm_mday#} p (fromIntegral d)
    {#set pg_tm->tm_mon#} p (fromIntegral mth - 1)
    {#set pg_tm->tm_year#} p (fromIntegral y - 1900)
    {#set pg_tm->tm_wday#} p 0
    {#set pg_tm->tm_yday#} p 0
    {#set pg_tm->tm_isdst#} p 0
    {#set pg_tm->tm_gmtoff#} p 0
    {#set pg_tm->tm_zone#} p nullPtr --We won't be able to free anything we allocate here so we dont

returnStatus :: PGScheduleError -> IO CInt
returnStatus = return . fromIntegral . fromEnum

pg_schedule_parse :: CString -> IO CInt
pg_schedule_parse s = do
  t <- T.decodeUtf8 <$> unsafePackCString s
  returnStatus $ either (const INVALID_SCHEDULE_FORMAT) (const NO_ERROR) (mkCronSchedule t)
foreign export ccall pg_schedule_parse :: CString -> IO CInt

withSchedule :: CString -> (CronSchedule -> IO PGScheduleError) -> IO CInt
withSchedule s f = do
  t <- T.decodeUtf8 <$> unsafePackCString s
  case mkCronSchedule t of
    Right sched -> fromIntegral . fromEnum <$> f sched
    Left _ -> do
      -- Should not happen since type is 'schedule' so it has already been parsed
      -- properly by pg_schedule_parse
      returnStatus INVALID_SCHEDULE_FORMAT

pg_schedule_contains :: CString -> Ptr PGTime -> Ptr CInt -> IO CInt
pg_schedule_contains s dtPtr resPtr = withSchedule s $ \sched -> do
  PGTime dt <- peek dtPtr
  poke resPtr (if idelem sched dt then 1 else 0)
  pure NO_ERROR
foreign export ccall pg_schedule_contains :: CString -> Ptr PGTime -> Ptr CInt -> IO CInt


pg_schedule_next :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt
pg_schedule_next s dtPtr resPtr = withSchedule s $ \sched -> do
  PGTime dt <- peek dtPtr
  let result = idsucc sched =<< idfloor sched dt
  maybe (pure NO_RESULT) (\n -> poke resPtr (PGTime (unQ n)) >> pure NO_ERROR) result
foreign export ccall pg_schedule_next :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt

pg_schedule_previous :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt
pg_schedule_previous s dtPtr resPtr = withSchedule s $ \sched -> do
  PGTime dt <- peek dtPtr
  let result = idpred sched =<< idceiling sched dt
  maybe (pure NO_RESULT) (\n -> poke resPtr (PGTime (unQ n)) >> pure NO_ERROR) result
foreign export ccall pg_schedule_previous :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt

pg_schedule_floor :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt
pg_schedule_floor s dtPtr resPtr = withSchedule s $ \sched -> do
  PGTime dt <- peek dtPtr
  let result = idfloor sched dt
  maybe (pure NO_RESULT) (\n -> poke resPtr (PGTime (unQ n)) >> pure NO_ERROR) result
foreign export ccall pg_schedule_floor :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt

pg_schedule_ceiling :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt
pg_schedule_ceiling s dtPtr resPtr = withSchedule s $ \sched -> do
  PGTime dt <- peek dtPtr
  let result = idceiling sched dt
  maybe (pure NO_RESULT) (\n -> poke resPtr (PGTime (unQ n)) >> pure NO_ERROR) result
foreign export ccall pg_schedule_ceiling :: CString -> Ptr PGTime -> Ptr PGTime -> IO CInt

pg_schedule_series :: CString -> Ptr PGTime -> Ptr PGTime -> Ptr (Ptr PGTime) -> Ptr CInt -> IO CInt
pg_schedule_series s fromPtr toPtr resPtr countPtr = withSchedule s $ \sched -> do
  PGTime dtFrom <- peek fromPtr
  PGTime dtTo <- peek toPtr
  let series = map PGTime
             . takeWhile (<= dtTo)
             . map unQ
             $ idenumUp sched dtFrom
  poke resPtr =<< newArray series
  poke countPtr (fromIntegral (length series))
  return NO_ERROR
foreign export ccall pg_schedule_series :: CString -> Ptr PGTime -> Ptr PGTime -> Ptr (Ptr PGTime) -> Ptr CInt -> IO CInt

pg_schedule_free_series :: Ptr PGTime -> IO ()
pg_schedule_free_series = free
foreign export ccall pg_schedule_free_series :: Ptr PGTime -> IO ()
