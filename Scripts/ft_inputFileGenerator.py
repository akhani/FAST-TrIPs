###################################################################################
#   Transit Input File Generator
#   Created on September 9, 2013
#   By Alireza Khani (akhani@email.arizona.edu)
#   Reads GTFS Files
#   Writes ft_input_stops.dat, ft_input_trips.dat, ft_input_routes.dat, ft_input_stopTimes.dat, ft_input_shapes.dat
###################################################################################
import datetime
import sys
###################################################################################

calendar = []
trips = []
routes = []
stopTimes = []
stops = []
shapes = []

year=int(raw_input("Please enter year (yyyy): "))
month=int(raw_input("Please enter month (mm): "))
day=int(raw_input("Please enter day (dd): "))
date = year*10000 + month*100 + day
dayOfWeek = datetime.date(year, month, day).isoweekday()
###############################################################################

inFile = open("calendar.txt","r")
for tmpIn in inFile:
    calendar = calendar + [tmpIn[:-1].split(",")]
service = [c[0] for c in calendar if (c[dayOfWeek]=='1' and int(c[8])<=date and int(c[9])>=date)]
inFile.close()

inFile = open("trips.txt","r")
for tmpIn in inFile:
    if tmpIn[:-1].split(",")[1] in service:
        trips = trips + [tmpIn[:-1].split(",")]
        #print [tmpIn[:-1].split(",")]
routeIds = set([t[0] for t in trips])
shapeIds = set([t[7] for t in trips])
inFile.close()

inFile = open("routes.txt","r")
for tmpIn in inFile:
    if tmpIn[:-1].split(",")[0] in routeIds:    #[t[0] for t in trips]:
        routes = routes + [tmpIn[:-1].split(",")]
inFile.close()

inFile = open("stop_times.txt","r")
tmpTrip = ''; exclude = 0; i=0
for tmpIn in inFile:
    i=i+1
    #if i%100000==0: print i
    tmpList = tmpIn[:-1].split(",")
    if tmpList[0]==tmpTrip and exclude==1:
        continue
    if tmpList[0]==tmpTrip and exclude==0:
        stopTimes = stopTimes + [tmpList]
        continue
    if tmpList[0] in [t[2] for t in trips]:
        stopTimes = stopTimes + [tmpList]
        tmpTrip=tmpList[0]
        exclude=0
    else:
        tmpTrip=tmpList[0]
        exclude=1
inFile.close()
stopIds = set([st[3] for st in stopTimes])

inFile = open("stops.txt","r")
for tmpIn in inFile:
    if tmpIn[:-1].split(",")[0] in stopIds:     #[st[3] for st in stopTimes]:
        stops = stops + [tmpIn[:-1].split(",")]
inFile.close()

inFile = open("shapes.txt","r")
tmpShape = ''; exclude = 0; i=0
for tmpIn in inFile:
    i=i+1
    if i%100000==0: print i
    tmpList = tmpIn[:-1].split(",")
    if tmpList[0]==tmpShape and exclude==1:
        continue
    if tmpList[0]==tmpShape and exclude==0:
        shapes = shapes + [tmpList]
        continue
    if tmpList[0] in shapeIds:  #[t[7] for t in trips]:
        shapes = shapes + [tmpList]
        tmpShape=tmpList[0]
        exclude=0
    else:
        tmpShape=tmpList[0]
        exclude=1
inFile.close()

print "%i trips, %i routes, %i stop times, %i stops, %i shapes!" %(len(trips), len(routes), len(stopTimes), len(stops), len(shapes))
#print len(service), len(trips), len(routes), len(stopTimes), len(stops), len(shapeIds), len(shapes)
################################################################################
FT=raw_input("Do you want to prepare input files for FAST-TrIPs? (y/n): ")
while FT not in ["y","n"]:
    FT=raw_input("Do you want to prepare input files for FAST-TrIPs? (y/n): ")

if FT=="n":
    sys.exit()

outFile = open("ft_input_stops.dat","w")
outFile.write("stopId\tstopName\tstopDesciption\tLatitude\tLongitude\tcapacity\n")
for s in stops:
    outFile.write(str(s[0]+'\t'+s[2]+'\t'+s[3]+'\t'+s[4]+'\t'+s[5]+'\t'+'100'+'\n'))
outFile.close()

outFile = open("ft_input_routes.dat","w")
outFile.write("routeId\trouteShortName\trouteLongName\trouteType\n")
for r in routes:
    outFile.write(str(r[0]+'\t'+r[2]+'\t'+r[3]+'\t'+r[5]+'\n'))
outFile.close()

outFile = open("ft_input_trips.dat","w")
outFile.write("tripId\trouteId\ttype\tstartTime\tcapacity\tshapeId\tdirectionId\n")
for t in trips:
    _type = [r[5] for r in routes if r[0]==t[0]][0]
    startTime = min([st[2] for st in stopTimes if st[0]==t[2]])
    startTime = startTime.split(":")[0]+startTime.split(":")[1]+startTime.split(":")[2]
    capacity = 60
    outFile.write(str(t[2]+'\t'+t[0]+'\t'+_type+'\t'+startTime+'\t'+str(capacity)+'\t'+t[7]+'\t'+t[5]+'\n'))
outFile.close()

outFile = open("ft_input_stopTimes.dat","w")
outFile.write("tripId\tarrivalTime\tdepartureTime\tstopId\tsequence\n")
for st in stopTimes:
    outFile.write(str(st[0]+'\t'+st[1].split(":")[0]+st[1].split(":")[1]+st[1].split(":")[2]+'\t'+st[2].split(":")[0]+st[2].split(":")[1]+st[2].split(":")[2]+'\t'+st[3]+'\t'+st[4]+'\n'))
outFile.close()

outFile = open("ft_input_shapes.dat","w")
outFile.write("shapeId\tlatitude\tlongitude\tsequence\tdistTraveled\n")
for sh in shapes:
    outFile.write(str(sh[0]+'\t'+sh[1]+'\t'+sh[2]+'\t'+sh[3]+'\t'+sh[4]+'\n'))
outFile.close()
print "Done!"

