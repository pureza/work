import Control.Monad
import Control.Concurrent
import Control.Concurrent.STM
import Array
import System.Random

--
-- Counter data type
--

data Counter = Counter Int

makeCounter :: Counter
makeCounter = Counter 0

counterValue :: Counter -> Int
counterValue (Counter value) = value

counterInc :: Counter -> Counter
counterInc (Counter value) = (Counter (value + 1))

--
-- Buffer data type
--
data Buffer = Buffer Int Int [Int]

makeBuffer :: Int -> Buffer
makeBuffer size = Buffer 0 0 [1 .. size]

get :: Buffer -> (Int, Buffer)
get (Buffer start end contents) = let position = start `mod` length(contents)
                                  in (contents!!position, Buffer (start + 1) end contents)

put :: Buffer -> Int -> Buffer
put (Buffer start end contents) value = let position = end `mod` length(contents)
                                        in Buffer start (end + 1) ((take position contents) ++ [value] ++ (drop (position + 1) contents))

len :: Buffer -> Int
len (Buffer start end contents) = end - start

capacity :: Buffer -> Int
capacity (Buffer start end contents) = length contents

bufferPrint :: Buffer -> IO()
bufferPrint (Buffer start end contents) = do { print(show start ++ " " ++ show end ++ " " ++ show contents) }

forever :: IO () -> IO ()
forever action = do 
  action
  forever action

randomDelay :: IO()
randomDelay = do
  waitTime <- getStdRandom (randomR (1, 1000000))
  threadDelay waitTime

producer :: TVar Buffer -> TVar Counter -> Int -> IO ()
producer txBuffer txCounter threadId = do { 
                                 ; value <- atomically (do { 
                                                           ; counter <- (readTVar txCounter)
                                                           ; buffer <- (readTVar txBuffer)
                                                           ; check((len buffer) < (capacity buffer))
                                                           ; newCounter <- return (counterInc (counterInc counter))
                                                           ; newBuffer <- return (put buffer (counterValue newCounter))
                                                           ; writeTVar txBuffer newBuffer
                                                           ; writeTVar txCounter newCounter
						           ; return (counterValue newCounter) })
                                 ; putStrLn $ "Put: " ++ show threadId ++ " " ++ show value
                                 ; randomDelay
				 }

consumer :: TVar Buffer -> Int -> IO ()
consumer txBuffer threadId = do {
                     ; value <- atomically (do {
                                               ; buffer <- (readTVar txBuffer)
                                               ; check((len buffer) > 0)
                                               ; (value, newBuffer) <- return (get buffer)
                                               ; writeTVar txBuffer newBuffer
                                               ; return value })
                     ; putStrLn $ "Get: " ++ show threadId ++ " " ++ show value
                     ; randomDelay
                     }


main = do { 
	    buffer <- atomically (newTVar (makeBuffer 10))
	    ; counter <- atomically (newTVar makeCounter)
	    ; sequence_ [forkIO (forever (producer buffer counter n)) | n <- [1..10]]
            ; sequence_ [forkIO (forever (consumer buffer n)) | n <- [1..10]]
	    ; threadDelay 100000000
	  }


