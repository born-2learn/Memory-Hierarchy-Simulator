import os

#os.system("ls")
sizes = []
assocs = [1,2,4,8,32]
for i in range(10, 21):
    sizes.append(2**i)

s = ""
test_s = "./sim 32 1024 1 0 0 0 0 ./traces/gcc_trace.txt > runs/val45.txt"
os.system(test_s)
for assoc in assocs:
    for size in sizes:
        #print(assoc, size)
        #s = "./sim 32 " + str(size) + " " + str(assoc) + " 0 0 0 0 ./traces/gcc_trace.txt > ./graph1/out_" + str(size) + "_" + str(assoc) + ".txt"
        #os.system(s)
        break
        #os.system()