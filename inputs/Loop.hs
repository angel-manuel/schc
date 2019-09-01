module Main where

a :: Int
a = b

b :: Int
b = a

main :: IO ()
main = print a
