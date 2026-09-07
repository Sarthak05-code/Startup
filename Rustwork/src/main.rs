use std::io;

fn main() {
    println!("Enter a word. ");
    let mut char1 = String::new();
    io::stdin().read_line(&mut char1).unwrap();
    let char1 = char1.trim();
    println!("Enter a second word. ");
    let mut char2 = String::new();
    io::stdin().read_line(&mut char2).unwrap();
    let char2 = char2.trim();

    let mut byte1: Vec<char> = char1.chars().collect();
    let mut byte2: Vec<char> = char2.chars().collect();

    byte1.sort();
    byte2.sort();

    if byte1 == byte2 {
        println!("The {char1} and {char2} are anagram. ");
    } else {
        println!("The {char1} and {char2} aren't anagram");
    }
}
