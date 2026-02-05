-- expect: 3628800
fac n = if n <= 1 then 1 else n * fac (n - 1)
main = fac 10
