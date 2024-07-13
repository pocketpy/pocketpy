# dict delete test
data = []
j = 6
for i in range(65535):
    j = ((j*5+1) % 65535)
    data.append(str(j))
