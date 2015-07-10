# Samuel Havron
# https://www.github.com/samuelhavron
# MIT License

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

for i in xrange(0, int(data_points)):
    xr = random.uniform(0, 1000)
    yr = random.uniform(0, 1000)
    print>>fx, xr
    print>>fy, yr
    print>>fxy, "%f %f" % (xr, yr)

fx.close()
fy.close()
fxy.close()
