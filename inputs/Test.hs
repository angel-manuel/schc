module Main (
    facr,
    fac,
    main
) where

-- module Main where

{- Block comment -}
-- Line comment

-- main :: IO ()
-- main = putStrLn (show (fac 6))

facr :: Int -> Int -> Int
facr start end = if start >= end - 1 then start else (facr start h) * (facr h end)
    where
        h = start + div (end - start) 2

fac :: Int -> Int
fac n = facr 1 (n + 1)
    where {
lelelelelele = 42 ; lolol = 1337
}

easy :: Int
easy = let
    a = 4
    b = 6
  in
    let { c = 8;
d = 10 } in
    a + b

main :: IO ()
main = do
    line <- getLine
    let n = (read line) :: Int
        res = fac n
    putStrLn (show res)
    putStrLn "END"
