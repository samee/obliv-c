# Samuel Havron
# https://www.github.com/samuelhavron
# BSD License

from sys import argv
import random

try:
    script, filenameX, filenameY, filenameXY, data_points = argv
except ValueError:
    print "Usage: python %s <xfile> <yfile> <xycatfile> <number_of_points>" % argv[0]
    quit()

print "Generating %d random data points in %s, %s, and %s" % (
    int(data_points), filenameX, filenameY, filenameXY)

fx = open(filenameX, 'w')
fy = open(filenameY, 'w')
fxy = open(filenameXY, 'w')
fx.truncate()
fy.truncate()
fxy.truncate()

xr = 0.0
for i in xrange(0, int(data_points)):
    xr += round(random.uniform(0, 1000), 4)
    yr = xr + round(random.uniform(0, 5), 2) # create artificial correlation
    if (xr >= 32000 or yr >= 32000):
        xr -= round(random.uniform(1000, 10000), 4)
        yr -= round(random.uniform(1000, 10000), 4)
    print>>fx, xr
    print>>fy, yr
    print>>fxy, "%f %f" % (xr, yr)

fx.close()
fy.close()
fxy.close()