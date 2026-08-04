use std::fs::File;
use std::io::Write;

fn main() {
    let mut file = File::create("hello.txt").unwrap();

    file.write_all(b"Hello Rust!")
        .unwrap();

    println!("Written successfully.");
}