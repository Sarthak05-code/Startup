struct Login {
    username: String,
    password: i32,
    is_login: bool,
}

trait Data {
    fn show_data(&self);
    fn show_password(&self);
    fn check_login(&self) -> bool;
}

impl Data for Login {
    fn show_data(&self) {
        println!("Hello, {}", self.username);
    }

    fn show_password(&self) {
        println!("Your password is : {}", self.password);
    }

    fn check_login(&self) -> bool {
        self.is_login
    }
}

fn main() {
    let login = Login {
        username: String::from("Sarthak"),
        password: 123456,
        is_login: true,
    };
    login.show_data();
    login.show_password();
    login.check_login();
}
