
data MyType = EmptyV | TwoV Int Int

a :: [(Int,Bool)]
a = [(1, True), (2, False)]

b :: MyType
b = TwoV 3 4

c :: MyType
c = EmptyV

f :: Int -> MyType
f x = if x > 3 then TwoV x 3 else EmptyV
