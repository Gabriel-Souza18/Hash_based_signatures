
def a():
    arq = open("Grande.txt", "w")
    for i in range(0,1000000):
        arq.write("a")
        
    arq.write("-cdc76e5c 9914fb92 81a1c7e2 84d73e67 f1809a48 a497200e 046d39cc c7112cd0\n")
    arq.close()
    
def b():
    arq = open("Grande.txt", "w")
    for i in range(0,16777216):
        arq.write("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno")
        
    arq.write("-50e72a0e 26442fe2 552dc393 8ac58658 228c0cbf b1d2ca87 2ae43526 6fcd055e\n")
    arq.close()

    
a()
    