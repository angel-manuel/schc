
{- Block comment -}
-- Line comment

fac :: Int -> Int
fac n = facr 1 (n + 1)
    where
        facr start end = if start >= end - 1 then start else (facr start h) * (facr h end)
            where
                h = start + div (end - start) 2

main :: IO ()
main = do
    line <- getLine
    let n = (read line) :: Int
    let res = fac n
    putStrLn (show res)
