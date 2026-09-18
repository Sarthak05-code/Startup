struct Login {
    username: String,
}

trait Data {
    fn show_data(&self);
}

impl Data for Login {
    fn show_data(&self) {
        println!("Hello, {}", self.username);
    }
}

fn main() {
    let login = Login {
        username: String::from("Sarthak"),
    };
    login.show_data();
}
