use std::io;

fn fibonacci(n: u32, memo: &mut Vec<u64>) -> u64 {
    if n == 0 {
        return 0;
    }
    if n <= 2 {
        return 1;
    }
    // Check if already computed
    if memo[n as usize] != 0 {
        return memo[n as usize];
    }
    let result = fibonacci(n - 1, memo) + fibonacci(n - 2, memo);
    memo[n as usize] = result;
    result
}

fn main() {
    println!("Enter a number: ");
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();

    match input.trim().parse::<u32>() {
        Ok(n) => {
            // Pre-allocate memo with zeros; index 0 unused for simplicity
            let mut memo = vec![0u64; (n + 1) as usize];
            
            print!("Fibonacci sequence: ");
            for i in 0..=n {
                let result = fibonacci(i, &mut memo);
                print!("{result} ");
            }
            println!();
        }
        Err(_) => println!("Please enter a valid non-negative number."),
    }
}