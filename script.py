array = [0] * 100000

n = 16
registers = [[0]*64 for _ in range(n)] # n регистров по 64 бита в каждом

for i, register in enumerate(registers):
    try:
        ind = register.index(1)
        array[i*64 + ind] += 1
    except:
        print("zero register!")