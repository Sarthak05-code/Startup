lists = []


def main():
    n = int(input("Enter no of elements: "))
    for i in range(n):
        take = int(input(f"Enter No. {i+1}: "))
        lists.append(take)

    print(f"The unsorted List: {lists}")
    for i in range(len(lists)):
        for j in range(i + 1 , len(lists)):
            if lists[i] > lists[j]:
                temp = lists[i]
                lists[i] = lists[j]
                lists[j] = temp
    print(f"The sorted lists = {lists}")

   


main()