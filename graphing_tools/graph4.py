import os

#os.system("ls")
blocksizes = [16, 32, 64, 128]
L1_sizes = [1024,2*1024,4*1024,8*1024,16*1024, 32*1024]

s = ""
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val44.txt"
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val45.txt"
#os.system(test_s)

counter = 1
for size in L1_sizes:
    for block in blocksizes:
    
        #print(assoc, size)
        #s = "./sim 32 " + str(size) + " " + str(assoc) + " 0 0 0 0 ./traces/gcc_trace.txt > graph1/out_"+str(counter)+"_"+str(assoc)+"_" + str(size) + ".txt"
        s = "../sim "+str(block)+" " +str(size) +  " 4 0 0 0 0 ../traces/gcc_trace.txt > ../graph4/"+str(counter)+".txt"
        #print(s)
        os.system(s)
        counter+=1
        #break
        #os.system()