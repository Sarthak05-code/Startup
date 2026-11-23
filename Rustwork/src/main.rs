use std::fs::File;
use std::io::{self, Read};

const BYTES_PER_LINE: usize = 16;

fn main() {
    // 1. Ask for the filename
    let mut filename = String::new();

    print!("Enter filename: ");

    io::Write::flush(&mut io::stdout()).unwrap();

    io::stdin()
        .read_line(&mut filename)
        .expect("Failed to read filename");

    let filename = filename.trim();

    // 2. Open the file
    let mut file = File::open(filename).expect("Could not open file");

    // 3. Read the file into a byte vector
    let mut buffer = Vec::new();

    file.read_to_end(&mut buffer).expect("Could not read file");

    // 4. Display the hex dump
    println!("\nOffset       Hexadecimal                                      ASCII");
    println!("--------------------------------------------------------------------------");

    for (line_number, chunk) in buffer.chunks(BYTES_PER_LINE).enumerate() {
        // Calculate the offset
        let offset = line_number * BYTES_PER_LINE;

        print!("{:08X}   ", offset);

        // Display hexadecimal bytes
        for byte in chunk {
            print!("{:02X} ", byte);
        }

        // Fill empty spaces for incomplete rows
        for _ in chunk.len()..BYTES_PER_LINE {
            print!("   ");
        }

        print!("  ");

        // Display ASCII characters
        for byte in chunk {
            if byte.is_ascii_graphic() || *byte == b' ' {
                print!("{}", *byte as char);
            } else {
                print!(".");
            }
        }

        println!();
    }
}
