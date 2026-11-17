use std::io::{self, Write};

fn main() {
    println!("Enter ip-address : ");
    io::stdout().flush().unwrap();
    let mut ip_address = String::new();
    io::stdin()
        .read_line(&mut ip_address)
        .expect("Failed to read user input");

    let ip_address = ip_address.trim();
    println!("Your ip address is : {ip_address}");

    for octals in ip_address.split(".") {
        let n: u8 = octals.parse().expect("Failed to parse");
        println!("{n:>3} -> {:08b}", n);
    }
}
