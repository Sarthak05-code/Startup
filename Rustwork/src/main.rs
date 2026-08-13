use std::io::{self}; // Bring Write trait into scope for stdout().flush()
use std::thread;
use std::time::Duration;

mod terminal {
    use std::io::{self, Write};

    pub fn clear_terminal() {
        print!("\x1B[2J\x1B[1;1H");
        // Flush standard output so the ANSI escape code clears the screen immediately
        let _ = io::stdout().flush();
    }
}

fn main() {
    let hours = read_valid_number("Enter hours: ");
    let minutes = read_valid_number("Enter minutes: ");
    let seconds = read_valid_number("Enter seconds: ");

    // Normalize overflow
    let extra_minutes = seconds / 60;
    let s = seconds % 60;

    let total_minutes = minutes + extra_minutes;
    let extra_hours = total_minutes / 60;
    let m = total_minutes % 60;

    let h = hours + extra_hours;

    println!("\nStarting countdown for {:02}:{:02}:{:02}...", h, m, s);
    thread::sleep(Duration::from_secs(1));

    // Call timer with normalized time
    timer(h, m, s);
}

fn timer(mut hour: i32, mut minutes: i32, mut seconds: i32) {
    // Loop until all units reach zero
    while hour > 0 || minutes > 0 || seconds > 0 {
        terminal::clear_terminal();
        println!(
            "⏳ Time Remaining: {:02}:{:02}:{:02}",
            hour, minutes, seconds
        );

        // Wait 1 second before ticking down
        thread::sleep(Duration::from_secs(1));

        // Tick down logic
        if seconds > 0 {
            seconds -= 1;
        } else if minutes > 0 {
            seconds = 59;
            minutes -= 1;
        } else if hour > 0 {
            seconds = 59;
            minutes = 59;
            hour -= 1;
        }
    }

    // Final state using our clean terminal module
    terminal::clear_terminal();
    println!("🎉 00:00:00 - Time's up!");
}

fn read_valid_number(prompt: &str) -> i32 {
    loop {
        println!("{}", prompt);
        let mut input = String::new();

        match io::stdin().read_line(&mut input) {
            Ok(_) => match input.trim().parse::<i32>() {
                Ok(num) if num >= 0 => return num,
                Ok(_) => println!("⚠️  Please enter a non-negative number!\n"),
                Err(_) => println!(
                    "⚠️  '{}' is not a valid integer. Try again!\n",
                    input.trim()
                ),
            },
            Err(error) => {
                println!("⚠️  Failed to read line: {error}. Try again!\n");
            }
        }
    }
}
