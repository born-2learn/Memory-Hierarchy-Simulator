import os

#os.system("ls")
L2_sizes = [16*1024, 32*1024, 64*1024]
L1_sizes = [1024, 2048, 4096, 8192]


s = ""
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val44.txt"
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val45.txt"
#os.system(test_s)

counter = 1

for L2_size in L2_sizes:
    for L1_size in L1_sizes:
        #print(assoc, size)
        #s = "./sim 32 " + str(size) + " " + str(assoc) + " 0 0 0 0 ./traces/gcc_trace.txt > graph1/out_"+str(counter)+"_"+str(assoc)+"_" + str(size) + ".txt"
        s = "../sim 32 "+str(L1_size) + " 4 "+str(L2_size)+ " 8 0 0 ../traces/gcc_trace.txt > ../graph5/"+str(counter)+".txt"
        #print(s)
        os.system(s)
        counter+=1
        #break
        #os.system()