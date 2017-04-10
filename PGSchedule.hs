module PGSchedule () where

import PGTime

import Foreign.C.Types
import Foreign.C.String
import Foreign.Ptr
import Foreign.Storable (peek)
import Data.ByteString.Unsafe (unsafePackCString)
import qualified Data.Text.Encoding as T
import System.IO
import Sigym4.Dimension.CronSchedule
import Sigym4.Dimension

pg_schedule_parse :: CString -> IO CInt
pg_schedule_parse s = do
  t <- T.decodeUtf8 <$> unsafePackCString s
  pure (either (const 0) (const 1) (mkCronSchedule t))

foreign export ccall pg_schedule_parse :: CString -> IO CInt

pg_schedule_contains :: CString -> Ptr PGTime -> IO CInt
pg_schedule_contains s dtPtr = do
  t <- T.decodeUtf8 <$> unsafePackCString s
  case (mkCronSchedule t) of
    Right sched -> do
      PGTime dt <- peek dtPtr
      pure (if idelem sched dt then 1 else 0)
    Left e -> do
      -- Should not happen
      hPrint stderr ("pg_schedule_contains_timestamp: could not parse schedule: " ++ show e)
      pure 0

foreign export ccall pg_schedule_contains :: CString -> Ptr PGTime -> IO CInt
