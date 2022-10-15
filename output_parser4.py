import os

for i in range(1,25):
    with open(os.getcwd()+'/graph4/'+str(i)+".txt", 'r') as fp:
        lines = fp.readlines()
        for l in lines:
            #print(l)
            if "L1 miss rate" in l:
                #print(fp)
                print(float(l[18:25]))
                #pass