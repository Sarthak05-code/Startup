numbers = []


def main():

    inputs = int(input("Enter the number of data you wanna enter: "))
    for i in range(inputs):
        num = int(input(f"Enter {i+1} number: "))
        numbers.append(num)

    target = int(input("Enter the number u wanna target: "))
    for i in range(len(numbers)):
        for j in range(i + 1 , len(numbers) - 1):
            if int(numbers[j]) + int(numbers[j+1]) == int(target):
                print(f"Pair found: {numbers[j]} and {numbers[j+1]} = {target}")
                return
    print("No pair found.")


   


if __name__ == "__main__":
    main()
