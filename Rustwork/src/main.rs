use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::process;

#[derive(Default)]
struct Stats {
    letters: usize,
    uppercase: usize,
    lowercase: usize,
    vowels: usize,
    consonants: usize,
    digits: usize,
    whitespace: usize,
    special: usize,
    lines: usize,
    total: usize,
}

fn is_vowel(c: char) -> bool {
    matches!(c.to_ascii_lowercase(), 'a' | 'e' | 'i' | 'o' | 'u')
}

// Scan one line and update the running totals.
// freq[0] = 'a', freq[1] = 'b', ... freq[25] = 'z' (case-insensitive).
fn analyze_line(line: &str, s: &mut Stats, freq: &mut [usize; 26]) {
    for c in line.chars() {
        s.total += 1;

        if c.is_ascii_alphabetic() {
            s.letters += 1;

            if c.is_ascii_uppercase() {
                s.uppercase += 1;
            } else {
                s.lowercase += 1;
            }

            if is_vowel(c) {
                s.vowels += 1;
            } else {
                s.consonants += 1;
            }

            let index = (c.to_ascii_lowercase() as u8 - b'a') as usize;
            freq[index] += 1;
        } else if c.is_ascii_digit() {
            s.digits += 1;
        } else if c.is_whitespace() {
            s.whitespace += 1;
        } else {
            s.special += 1;
        }
    }
}

fn print_frequency(freq: &[usize; 26]) {
    // Keep only letters that appear, then sort by count (high to low)
    let mut pairs: Vec<(char, usize)> = freq
        .iter()
        .enumerate()
        .filter(|(_, &count)| count > 0)
        .map(|(i, &count)| ((b'a' + i as u8) as char, count))
        .collect();

    pairs.sort_by(|a, b| b.1.cmp(&a.1).then(a.0.cmp(&b.0)));

    println!();
    println!("Letter frequency (case-insensitive)");
    println!("{:-<30}", "");

    let max = pairs.first().map(|p| p.1).unwrap_or(1);

    for (letter, count) in pairs {
        // Scale the bar so the most common letter is 20 chars wide
        let bar_len = (count * 20 / max).max(1);
        println!("{}  {:>5}  {}", letter, count, "#".repeat(bar_len));
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        eprintln!("Usage: {} <file>", args[0]);
        process::exit(1);
    }

    let path = &args[1];

    let file = match File::open(path) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Could not open '{}': {}", path, e);
            process::exit(1);
        }
    };

    // BufReader reads the file in chunks, so we never hold the whole file in memory
    let mut reader = BufReader::new(file);
    let mut line = String::new();
    let mut stats = Stats::default();
    let mut freq = [0usize; 26];

    loop {
        line.clear();

        match reader.read_line(&mut line) {
            Ok(0) => break, // 0 bytes read means end of file
            Ok(_) => {
                stats.lines += 1;
                analyze_line(&line, &mut stats, &mut freq);
            }
            Err(e) => {
                eprintln!("Error reading '{}': {} (is it a text file?)", path, e);
                process::exit(1);
            }
        }
    }

    println!("File: {}", path);
    println!("{:-<30}", "");
    println!("{:<16}{:>8}", "Total chars", stats.total);
    println!("{:<16}{:>8}", "Lines", stats.lines);
    println!("{:<16}{:>8}", "Letters", stats.letters);
    println!("{:<16}{:>8}", "  Uppercase", stats.uppercase);
    println!("{:<16}{:>8}", "  Lowercase", stats.lowercase);
    println!("{:<16}{:>8}", "  Vowels", stats.vowels);
    println!("{:<16}{:>8}", "  Consonants", stats.consonants);
    println!("{:<16}{:>8}", "Digits", stats.digits);
    println!("{:<16}{:>8}", "Whitespace", stats.whitespace);
    println!("{:<16}{:>8}", "Special chars", stats.special);

    print_frequency(&freq);
}
