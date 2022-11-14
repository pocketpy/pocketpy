for i in range(10):
    for j in range(10):
        goto .test 
        print(2)
    label .test
    print(i)

# 15, 23
# 5, 28