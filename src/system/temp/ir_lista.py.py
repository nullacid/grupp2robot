from math import *

def test_value_bin(x):
	#old function 26.047*pow(x_r,-1.252)
    if x < 18:
        return 0
    x_r = x/256*5;
    print("x= ", x, " y= ", 27.166*pow(x_r,-1.225))
    return 27.166*pow(x_r,-1.225)

def display(higher):
    res = []
    for x in range(higher):
        res.append(round(test_value_bin(x)))

    print(res)
    print(len(res))

display(200)
        
