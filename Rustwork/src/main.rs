use std::io;

// could consider using memoization ??
fn return_recursion(x: usize, memo: &mut Vec<Option<i32>>) -> i32 {
    if let Some(value) = memo[x] {
        return value;
    }

    if x == 0 {
        memo[0] = Some(0);
        return 0;
    }

    if x == 1 {
        memo[1] = Some(1);
        return 1;
    }
    let result = return_recursion(x - 1, memo) + return_recursion(x - 2, memo);
    result
}

fn main() {
    let test_array = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    println!("Current array: {:?}", test_array);
    println!("1. Reverse");
    println!("2. Normal");
    println!("3. Squared");
    println!("4. Fibonacci");

    let mut order = String::new();
    io::stdin()
        .read_line(&mut order)
        .expect("Failed to read input");

    let choice: i32 = match order.trim().parse() {
        Ok(num) => num,
        Err(_) => {
            println!("Please enter a valid number.");
            return;
        }
    };

    match choice {
        1 => {
            for value in test_array.iter().rev() {
                print!("{}\t", value);
            }
        }
        2 => {
            for value in test_array.iter() {
                print!("{}\t", value);
            }
        }
        3 => {
            for value in test_array.iter().map(|x| x * x) {
                print!("{}\t", value)
            }
        }
        4 => {
            let mut memo = vec![None; test_array.len()];
            for value in 0..test_array.len() {
                print!("{}\t", return_recursion(value, &mut memo));
            }
        }

        _ => println!("Invalid choice!"),
    }
}
