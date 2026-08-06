use std::{collections::HashMap, io};

fn main() {
    println!("Enter a word: ");
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();
    let mut freq = HashMap::new();
    for ch in input.trim().chars() {
        *freq.entry(ch).or_insert(0) += 1;
    }

    let mut repeated = 0;

    for (ch, count) in &freq {
        if *count > 1 {
            println!("{} appears {} times", ch, count);
            repeated += 1;
        }
    }
    println!("Repeated character {repeated}\n");
}
