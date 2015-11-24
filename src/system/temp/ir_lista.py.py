from math import *

def test_value(x):
    print(26.047*pow(x,-1.252));

def test_value_bin(x):
    if x < 18:
        return 0
    x_r = x/256*5;
    print("x= ", x, " y= ", 26.047*pow(x_r,-1.252))
    return 26.047*pow(x_r,-1.252)

def display(higher):
    res = []
    for x in range(higher):
        res.append(round(test_value_bin(x)))

    print(res)
    print(len(res))

display(120)
        
