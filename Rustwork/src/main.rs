use std::hash::Hasher;

struct Fnv1a(u64);

impl Default for Fnv1a {
    fn default() -> Self {
        Fnv1a(0xcbf29ce484222325)
    }
}

impl Hasher for Fnv1a {
    fn finish(&self) -> u64 {
        self.0
    }
    fn write(&mut self, bytes: &[u8]) {
        for &b in bytes {
            self.0 ^= b as u64;
            self.0 = self.0.wrapping_mul(0x100000001b3);
        }
    }
}

fn hash_password(password: &str) -> u64 {
    let mut h = Fnv1a::default();
    h.write(password.as_bytes());
    h.finish()
}

struct Login {
    username: String,
    password_hash: u64,
    is_login: bool,
}

impl Login {
    fn new(username: &str, password: &str) -> Self {
        Login {
            username: username.to_string(),
            password_hash: hash_password(password),
            is_login: false,
        }
    }

    fn login(&mut self, attempt: &str) {
        self.is_login = hash_password(attempt) == self.password_hash;
        if self.is_login {
            println!("Login successful.");
        } else {
            println!("Wrong password.");
        }
    }

    fn show_data(&self) {
        if !self.is_login {
            println!("You aren't allowed to show data. Login to try again.");
            return;
        }
        println!("Hello, {}", self.username);
    }
}

fn main() {
    let mut user = Login::new("Sarthak", "123456");
    user.show_data(); // blocked
    user.login("wrong"); // fails
    user.login("123456"); // succeeds
    user.show_data(); // allowed
}
