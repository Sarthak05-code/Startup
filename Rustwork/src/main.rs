use std::{
    env,
    fs::File,
    io::{self, Write},
    process::{Command, Stdio},
};

fn main() {
    let mut history = Vec::new();

    loop {
        // Display prompt with current directory
        let cwd = env::current_dir()
            .map(|p| p.display().to_string())
            .unwrap_or_else(|_| "?".to_string());

        print!("{cwd} $ ");
        if let Err(e) = io::stdout().flush() {
            eprintln!("Failed to flush prompt: {e}");
            continue;
        }

        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(0) => {
                println!("\nGoodbye!");
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

        history.push(input_str.to_string());

        // Handle pipelines separated by '|'
        let pipe_commands: Vec<&str> = input_str.split('|').map(|s| s.trim()).collect();
        execute_pipeline(pipe_commands, &mut history);
    }
}

fn execute_pipeline(commands: Vec<&str>, history: &mut Vec<String>) {
    let num_commands = commands.len();
    let mut previous_stdout: Option<Stdio> = None;
    let mut children = Vec::new();

    for (i, cmd_str) in commands.iter().enumerate() {
        let is_first = i == 0;
        let is_last = i == num_commands - 1;

        // Expand environment variables (e.g., $HOME -> /home/user)
        let expanded = expand_env_vars(cmd_str);

        // Parse redirections (<, >)
        let (tokens, stdin_file, stdout_file) = match parse_redirections(&expanded) {
            Ok(res) => res,
            Err(e) => {
                eprintln!("Parse error: {e}");
                return;
            }
        };

        if tokens.is_empty() {
            continue;
        }

        // Handle built-in commands (cd, exit, history, clear, pwd)
        // Built-ins run directly in the shell process (unless piped)
        if num_commands == 1 && is_builtin(tokens[0]) {
            execute_builtin(&tokens, history);
            return;
        }

        // Configure Stdin
        let stdin_spec = if let Some(filename) = stdin_file {
            match File::open(filename) {
                Ok(file) => Stdio::from(file),
                Err(e) => {
                    eprintln!("Error opening {filename}: {e}");
                    return;
                }
            }
        } else if let Some(prev) = previous_stdout.take() {
            prev
        } else {
            Stdio::inherit()
        };

        // Configure Stdout
        let stdout_spec = if let Some(filename) = stdout_file {
            match File::create(filename) {
                Ok(file) => Stdio::from(file),
                Err(e) => {
                    eprintln!("Error creating {filename}: {e}");
                    return;
                }
            }
        } else if !is_last {
            Stdio::piped()
        } else {
            Stdio::inherit()
        };

        // Spawn child process
        match Command::new(tokens[0])
            .args(&tokens[1..])
            .stdin(stdin_spec)
            .stdout(stdout_spec)
            .spawn()
        {
            Ok(mut child) => {
                if !is_last {
                    // Capture child's stdout pipe for the next command in the pipeline
                    if let Some(pipe) = child.stdout.take() {
                        previous_stdout = Some(Stdio::from(pipe));
                    }
                }
                children.push(child);
            }
            Err(_) => {
                eprintln!("Command not found: {}", tokens[0]);
                return;
            }
        }
    }

    // Wait for all child processes in the pipeline to finish
    for mut child in children {
        let _ = child.wait();
    }
}

fn is_builtin(cmd: &str) -> bool {
    matches!(cmd, "cd" | "exit" | "history" | "pwd" | "clear" | "echo")
}

fn execute_builtin(tokens: &[&str], history: &[String]) {
    match tokens[0] {
        "exit" => {
            println!("You exited the shell");
            std::process::exit(0);
        }
        "pwd" => {
            if let Ok(path) = env::current_dir() {
                println!("{}", path.display());
            }
        }
        "cd" => {
            let target = if tokens.len() < 2 {
                env::var("HOME").ok()
            } else {
                Some(tokens[1].to_string())
            };

            if let Some(path) = target {
                if let Err(e) = env::set_current_dir(&path) {
                    eprintln!("cd: {path}: {e}");
                }
            } else {
                eprintln!("cd: HOME not set");
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
            print!("\x1B[2J\x1B[H");
            let _ = io::stdout().flush();
        }
        _ => {}
    }
}

/// Parses `<` and `>` tokens out of the command stream
fn parse_redirections<'a>(
    cmd_str: &'a str,
) -> Result<(Vec<&'a str>, Option<&'a str>, Option<&'a str>), String> {
    let mut tokens = Vec::new();
    let mut stdin_file = None;
    let mut stdout_file = None;

    let parts: Vec<&str> = cmd_str.split_whitespace().collect();
    let mut idx = 0;

    while idx < parts.len() {
        match parts[idx] {
            "<" => {
                idx += 1;
                if idx >= parts.len() {
                    return Err("Missing file for input redirection '<'".to_string());
                }
                stdin_file = Some(parts[idx]);
            }
            ">" => {
                idx += 1;
                if idx >= parts.len() {
                    return Err("Missing file for output redirection '>'".to_string());
                }
                stdout_file = Some(parts[idx]);
            }
            token => {
                tokens.push(token);
            }
        }
        idx += 1;
    }

    Ok((tokens, stdin_file, stdout_file))
}

/// Replaces `$VAR` with environment variable values
fn expand_env_vars(input: &str) -> String {
    let mut result = String::new();
    for token in input.split_whitespace() {
        if let Some(var_name) = token.strip_prefix('$') {
            if let Ok(val) = env::var(var_name) {
                result.push_str(&val);
            } else {
                result.push_str(token);
            }
        } else {
            result.push_str(token);
        }
        result.push(' ');
    }
    result.trim().to_string()
}
