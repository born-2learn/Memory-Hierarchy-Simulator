import os

#os.system("ls")
sizes = []
assocs = [1,2,4,8,32]
for i in range(10, 14):
    sizes.append(2**i)

s = ""
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val44.txt"
#test_s = "./sim 16 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val45.txt"
#os.system(test_s)

counter = 1
for assoc in assocs:
    for size in sizes:
        #print(assoc, size)
        #s = "./sim 32 " + str(size) + " " + str(assoc) + " 0 0 0 0 ./traces/gcc_trace.txt > graph1/out_"+str(counter)+"_"+str(assoc)+"_" + str(size) + ".txt"
        s = "../sim 32 " + str(size) + " " + str(assoc) + " 16384 8 0 0 ../traces/gcc_trace.txt > ../graph2/"+str(counter)+".txt"
        print(s)
        #os.system(s)
        counter+=1
        #break
        #os.system()