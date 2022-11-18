import os

'''
for dirname,_, files in os.walk(os.getcwd()+'/graph1'):
    for f in files:
        print(f.rstrip(".txt"))
    for ff in sorted(files, key=int):
        #print(ff)
    for f in files:
        #print(f)
        with open(os.getcwd()+'/graph1/'+f, 'r') as fp:
            lines = fp.readlines()
            for l in lines:
                #print(l)
                if "L1 miss rate" in l:
                    #print(float(l[18:25]))
                    pass
'''
for i in range(1,56):
    with open(os.getcwd()+'/graph1/'+str(i)+".txt", 'r') as fp:
        lines = fp.readlines()
        for l in lines:
            #print(l)
            if "L1 miss rate" in l:
                #print(fp)
                print(float(l[18:25]))
                #pass
