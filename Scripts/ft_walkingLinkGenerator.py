###################################################################################
#   Transit Walking (Access and Transfer) Link Generator
#   Created on May 24, 2012
#   By Alireza Khani (akhani@email.arizona.edu)
#   Reads ft_input_stops.dat, ft_input_zones.dat
#   Writes ft_input_accessLinks.dat, ft_input_transfers.dat
###################################################################################
import math
###################################################################################
stop = []
stopLat = []
stopLon = []
inFile = open("ft_input_stops.dat")
strIn = inFile.readline()
i=-1
while(1):
    strIn = inFile.readline()
    if(strIn) == "":
        break
    else:
        i = i + 1
        strSplt = strIn.split("\t")
        stop.append(strSplt[0])
        stopLat.append(float(strSplt[3]))
        stopLon.append(float(strSplt[4]))
        if i%1000==0:
            print i
inFile.close()
print i+1, "stops!"
###################################################################################
node = []
nodeLat = []
nodeLon = []
inFile = open("ft_input_zones.dat")
strIn = inFile.readline()
i=-1
while(1):
    strIn = inFile.readline()
    if(strIn) == "":
        break
    else:
        i = i + 1
        strSplt = strIn.split("\t")
        node.append(strSplt[0])
        nodeLat.append(float(strSplt[1]))
        nodeLon.append(float(strSplt[2]))
        if i%1000==0:
            print i
inFile.close()
print i+1, "nodes!"
################################################################################### 
outFile = open("ft_input_accessLinks.dat", "w")
outFile.write("TAZ\tstop\tdist\ttime\n")
degreesToRradians = math.pi/180.0
k=0
for i in range(len(node)):
    tmpLat1 = nodeLat[i] * degreesToRradians
    tmpLon1 = nodeLon[i] * degreesToRradians
    for j in range(len(stop)):
        tmpLat2 = stopLat[j] * degreesToRradians
        tmpLon2 = stopLon[j] * degreesToRradians
        tmpDist = (math.sin(math.pi/2.0 - tmpLat1) * math.sin(math.pi/2.0 - tmpLat2) * math.cos(tmpLon1 - tmpLon2) + math.cos(math.pi/2.0 - tmpLat1) * math.cos(math.pi/2.0 - tmpLat2))
        tmpDist = 3960 * math.acos(tmpDist)     #will crash with domain = 1.0
        tmpDist = max(tmpDist,0.001)
        if tmpDist <= 0.75:
            k = k + 1
            tmpNode = node[i]
            tmpStop = stop[j]
            tmpTime = tmpDist / 3.0 * 60
            strOut = str(tmpNode) + "\t" + str(tmpStop) + "\t" + str(round(tmpDist,3)) + "\t" + str(round(tmpTime,2)) + "\n"
            outFile.write(strOut)
            if k%10000==0:
                print i, j, k
outFile.close()
print k, "access links!"
################################################################################### 
outFile = open("ft_input_transfers.dat", "w")
outFile.write("fromStop\ttoStop\tdist\ttime\n")
degreesToRradians = math.pi/180.0
k=0
for i in range(len(stop)):
    tmpLat1 = stopLat[i] * degreesToRradians
    tmpLon1 = stopLon[i] * degreesToRradians
    for j in range(len(stop)):
        if i == j:
            continue
        tmpLat2 = stopLat[j] * degreesToRradians
        tmpLon2 = stopLon[j] * degreesToRradians
        tmpDist = (math.sin(math.pi/2.0 - tmpLat1) * math.sin(math.pi/2.0 - tmpLat2) * math.cos(tmpLon1 - tmpLon2) + math.cos(math.pi/2.0 - tmpLat1) * math.cos(math.pi/2.0 - tmpLat2))
        tmpDist = 3960 * math.acos(tmpDist)     #will crash with domain = 1.0
        tmpDist = max(tmpDist,0.001)
        if tmpDist <= 0.25:
            k = k + 1
            tmpStop1 = stop[i]
            tmpStop2 = stop[j]
            tmpTime = tmpDist / 3.0 * 60
            strOut = str(tmpStop1) + "\t" + str(tmpStop2) + "\t" + str(round(tmpDist,3)) + "\t" + str(round(tmpTime,2)) + "\n"
            outFile.write(strOut)
            if k%1000==0:
                print i, j, k
outFile.close()
print k, "transfers!"
print "Done!"







