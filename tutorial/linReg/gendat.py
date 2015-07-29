# Samuel Havron
# https://www.github.com/samuelhavron
# MIT License

from sys import argv
import random

try:
    script, filenameXY, data_points = argv
except ValueError:
    print "Usage: python %s <xycatfile> <number_of_points>" % argv[0]
    quit()

print "Generating %d random data points in %s" % (
    int(data_points), filenameXY)

fxy = open(filenameXY, 'w')
fxy.truncate()

for i in xrange(0, int(data_points)):
    xr = random.uniform(0, 100)
    yr = random.uniform(0, 100)
    print>>fxy, "%f %f" % (xr, yr)

fxy.close()
