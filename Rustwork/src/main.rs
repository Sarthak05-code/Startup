use std::{
    io::{self, Write},
    process,
};

struct Task {
    id: usize,
    description: String,
    completed: bool,
}

struct TaskManger {
    tasks: Vec<Task>,
}

impl TaskManger {
    fn new() -> Self {
        Self { tasks: Vec::new() }
    }

    fn add_tasks(&mut self, description: String) {
        let id = self.tasks.len() + 1;
        let task = Task {
            id,
            description,
            completed: false,
        };

        self.tasks.push(task);

        println!("Task added sucessfully.");
    }

    fn show_tasks(&self) {
        if self.tasks.is_empty() {
            println!("No task available");
            return;
        }

        for task in &self.tasks {
            let status = if task.completed { 'x' } else { ' ' };
            println!("{} [{}] {}", task.id, status, task.description);
        }
    }

    fn show_not_completed(&self) {
        let mut found = false;

        for task in &self.tasks {
            if !task.completed {
                println!("{}. [] {}", task.id, task.description);
                found = true;
            }
        }
        if !found {
            println!("No Incomplete Tasks Found.");
        }
    }

    fn show_completed(&self) {
        let mut found = false;

        for task in &self.tasks {
            if task.completed {
                println!("{}. [x] {}", task.id, task.description);
                found = true;
            }
        }
        if !found {
            println!("No complete task found");
        }
    }

    fn complete_task(&mut self, id: usize) {
        for task in &mut self.tasks {
            if task.id == id {
                task.completed = true;
                println!("Task {} completed.", id);
                return;
            }
        }
        print!("No task found. ");
    }

    fn delete_task(&mut self, id: usize) {
        let index = self.tasks.iter().position(|task| task.id == id);
        match index {
            Some(index) => {
                self.tasks.remove(index);
                for (i, task) in self.tasks.iter_mut().enumerate() {
                    task.id = i + 1;
                }
                println!("Task {} deleted ", id);
            }
            None => {
                println!("Task id not found.")
            }
        }
    }
}

fn processs_command(input: &str, manager: &mut TaskManger) {
    if input == "exit" {
        println!("Exiting program");
        process::exit(0);
    }

    if let Some(description) = input.strip_prefix("task add ") {
        manager.add_tasks(description.to_string());
        return;
    }

    // tokenize here;
    let parts: Vec<&str> = input.split_whitespace().collect();

    // Command match;
    match parts.as_slice() {
        ["task", "show"] => {
            manager.show_tasks();
        }

        ["task", "show", "--nc"] => {
            manager.show_not_completed();
        }

        ["task", "show", "--c"] => {
            manager.show_completed();
        }

        ["task", "done", id] => match id.parse::<usize>() {
            Ok(id) => {
                manager.complete_task(id);
            }
            Err(_) => {
                println!("Invalid Task Id");
            }
        },

        ["task", "del", id] => match id.parse::<usize>() {
            Ok(id) => {
                manager.delete_task(id);
            }
            Err(_) => {
                println!("No id found to delete");
            }
        },
        _ => {
            println!("Invalid Command")
        }
    }
}

fn main() {
    let mut manager = TaskManger::new();
    println!("Task Manger. ");
    println!("Type exit to quit. ");
    println!();

    loop {
        print!("> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();

        io::stdin().read_line(&mut input).unwrap();

        let input = input.trim();

        if input.is_empty() {
            continue;
        }

        processs_command(input, &mut manager);
    }
}
