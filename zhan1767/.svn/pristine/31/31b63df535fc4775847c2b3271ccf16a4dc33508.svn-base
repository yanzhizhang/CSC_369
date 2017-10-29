instruction_counter = 0
data_counter = 0
instruction_dic = {}
data_dic = {}

with open("tr-heaploop.ref") as f:
    content = f.readlines()
content = [x.strip() for x in content]

for i in content:
    if i[-1] == "I":
        instruction_counter += 1
        if (instruction_dic.has_key((i[0:5]))):
            instruction_dic[i[0:5]] += 1
        else:
            instruction_dic[i[0:5]] = 1

    if i[-1] == "S":
        data_counter += 1
        if (data_dic.has_key((i[0:8]))):
            data_dic[i[0:8]] += 1
        else:
            data_dic[i[0:8]] = 1

    if i[-1] == "L":
        data_counter += 1
        if (data_dic.has_key((i[0:5]))):
            data_dic[i[0:5]] += 1
        else:
            data_dic[i[0:5]] = 1

    if i[-1] == "M":
        data_counter += 1
        if (data_dic.has_key((i[0:6]))):
            data_dic[i[0:6]] += 1
        else:
            data_dic[i[0:6]] = 1

with open("tr-matmul.ref") as f:
    content = f.readlines()
content = [x.strip() for x in content]

for i in content:
    if i[-1] == "I":
        instruction_counter += 1
        if (instruction_dic.has_key((i[0:5]))):
            instruction_dic[i[0:5]] += 1
        else:
            instruction_dic[i[0:5]] = 1

    if i[-1] == "S":
        data_counter += 1
        if (data_dic.has_key((i[0:8]))):
            data_dic[i[0:8]] += 1
        else:
            data_dic[i[0:8]] = 1

    if i[-1] == "L":
        data_counter += 1
        if (data_dic.has_key((i[0:5]))):
            data_dic[i[0:5]] += 1
        else:
            data_dic[i[0:5]] = 1

    if i[-1] == "M":
        data_counter += 1
        if (data_dic.has_key((i[0:6]))):
            data_dic[i[0:6]] += 1
        else:
            data_dic[i[0:6]] = 1

instruction_list = instruction_dic.items()
data_list = data_dic.items()



readme = open("pages.txt", "w")
readme.write("Instructions counts: %d\n" %instruction_counter)
readme.write("Data counts: %d\n\n\n" %data_counter)

readme.write("Instructions\n")
for i in instruction_list:
    readme.write("%s,%d\n" % (i[0],i[1]))

readme.write("\nData\n")
for i in data_list:
    readme.write("%s,%d\n" % (i[0],i[1]))
readme.close()