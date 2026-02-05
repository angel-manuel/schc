-- expect: 720
fac n = if n <= 1 then 1 else n * fac (n - 1)
main = putStrLn (show (fac 6))
