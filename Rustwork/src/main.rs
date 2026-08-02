use std::{
    env,
    io::{self, Write},
    process::{self, Command},
};

fn main() {
    let mut history = Vec::new();

    loop {
        print!("myshell> ");
        if let Err(e) = io::stdout().flush() {
            eprintln!("Failed to flush stdout: {e}");
            continue;
        }

        let mut input = String::new();

        // Handle EOF (e.g., Ctrl+D) gracefully instead of unwrap panic / infinite loop
        match io::stdin().read_line(&mut input) {
            Ok(0) => {
                println!("\nExiting shell...");
                break;
            }
            Ok(_) => {}
            Err(e) => {
                eprintln!("Error reading input: {e}");
                continue;
            }
        }

        let input_str = input.trim();
        if input_str.is_empty() {
            continue;
        }

        // Store command string in history
        history.push(input_str.to_string());

        let tokens: Vec<&str> = input_str.split_whitespace().collect();

        match tokens[0] {
            "exit" => {
                println!("You exited the shell");
                process::exit(0);
            }

            "pwd" => {
                if tokens.len() > 1 {
                    println!("pwd: too many arguments");
                } else {
                    match env::current_dir() {
                        Ok(current_dir) => println!("{}", current_dir.display()),
                        Err(e) => eprintln!("pwd error: {e}"),
                    }
                }
            }

            "cd" => {
                if tokens.len() < 2 {
                    // Default to HOME directory if no path is given
                    if let Ok(home) = env::var("HOME") {
                        if let Err(e) = env::set_current_dir(home) {
                            eprintln!("cd: {e}");
                        }
                    } else {
                        println!("cd: missing argument");
                    }
                } else {
                    if let Err(e) = env::set_current_dir(tokens[1]) {
                        eprintln!("cd: {e}");
                    }
                }
            }

            "history" => {
                for (i, cmd) in history.iter().enumerate() {
                    println!("{:3}  {}", i + 1, cmd);
                }
            }

            "echo" => {
                println!("{}", tokens[1..].join(" "));
            }

            "clear" => {
                // ANSI escape sequence to clear terminal screen and reset cursor position
                print!("\x1B[2J\x1B[H");
                let _ = io::stdout().flush();
            }

            _ => {
                // Execute external commands
                match Command::new(tokens[0]).args(&tokens[1..]).spawn() {
                    Ok(mut child) => {
                        if let Err(e) = child.wait() {
                            eprintln!("Error waiting for process: {e}");
                        }
                    }
                    Err(_) => {
                        println!("Command not found: {}", tokens[0]);
                    }
                }
            }
        }
    }
}
