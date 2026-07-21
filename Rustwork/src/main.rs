use std::io;

fn even_or_odd(n: i32) -> &'static str {
    if n % 2 == 0 {
        "Even"
    } else {
        "Odd"
    }
}

fn main() {
    let array_values: Vec<i32> = vec![1, 2, 3, 4, 5];
    println!("Enter an index (0-4): ");

    let mut value = String::new();
    io::stdin()
        .read_line(&mut value)
        .expect("Please enter a valid input");

    let index: usize = value.trim().parse().expect("Error, enter a valid number");

    match array_values.get(index) {
        Some(val) => println!("Value at index {index}: {val} is {}", even_or_odd(*val)),
        None => println!("Index {index} is out of bounds"),
    }
}