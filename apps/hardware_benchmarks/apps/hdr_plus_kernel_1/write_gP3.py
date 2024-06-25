

ymax = 32
xmax = 32
nmax = 3

totalCount = (ymax * xmax * nmax) - 1
outfile = open("gp3.txt",'w')

outfile.write("gP3_LUT(x, y, n) = ")
# outfile.write("select(x == 0, y == 0, n == 0, u16(0)),")
# outfile.write("\n")

for y in range(ymax):
    for x in range(xmax):
        for n in range(nmax):
            if(not(x == xmax-1 and y == ymax-1 and n == nmax-1)):
                outfile.write(f"select(x == {x} && y == {y} && n == {n}, u16({x}),")

                if (y == ymax-1 and x == xmax-1 and n == nmax-2):
                    outfile.write(" u16(1)")
                else:
                    outfile.write("\n")


for i in range(totalCount):
    outfile.write(")")
outfile.write(";")

outfile.close()