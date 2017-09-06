
{- Block comment -}
-- Line comment

facr start end = if 1 then 666 else 777 {- if start >= end - 1 then start else (facr start h) * (facr h end)
    where
        h = start + div (end - start) 2

-- fac :: Int -> Int
fac n = facr 1 (n + 1)

-- main :: IO ()
main = do
    line <- getLine
    let n = (read line) :: Int
    let res = fac n
    putStrLn (show res) -}
