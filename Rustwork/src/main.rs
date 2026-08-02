use std::{
    io::{self, Write},
    process,
};

fn main() {
    loop {
        print!("myshell> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        let input = input.trim();

        // split the value; Eg : git status -> ["git" , "status"]
        let tokens: Vec<&str> = input.split_whitespace().collect();

        if tokens.is_empty() {
            continue;
        }

        // only take the first word;
        match tokens[0] {
            "exit" => {
                println!("You exited the shell");
                process::exit(0);
            }
            "pwd" => {
                if tokens.len() > 1 {
                    println!("pwd : too many argument.")
                } else {
                    let current_dir = std::env::current_dir().unwrap();
                    println!("{} ", current_dir.display());
                }
            }

            "echo" => {
                println!("{}", tokens[1..].join(" "));
            }

            "clear" => {
                println!("Doesn't work yet.")
            }
            _ => {}
        }
    }
}
