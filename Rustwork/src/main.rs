use std::collections::HashMap;

struct User {
    id: u32,
    username: String,
    total_score: f64,
}

impl User {
    fn new(id: u32, username: impl Into<String>, total_score: f64) -> Self {
        Self {
            id,
            username: username.into(),
            total_score,
        }
    }
}

fn insert_user(storage: &mut HashMap<u32, User>, user: User) -> Result<(), String> {
    if storage.contains_key(&user.id) {
        return Err(format!("User with ID : {} already exists.", user.id));
    }
    storage.insert(user.id, user);
    Ok(())
}

fn get_user(storage: &HashMap<u32, User>, id: u32) -> Option<&User> {
    storage.get(&id)
}

fn print_all_users(storage: &HashMap<u32, User>) {
    for (id, user) in storage.iter() {
        println!(
            "Key : {} || ID: {} || Name : {} || TotalMarks : {:.2}",
            id, user.id, user.username, user.total_score
        );
    }
}

fn main() -> Result<(), String> {
    let mut user_storage: HashMap<u32, User> = HashMap::new();

    insert_user(&mut user_storage, User::new(101, "Sarthak", 101.10))?;
    insert_user(&mut user_storage, User::new(102, "Ram", 88.50))?;
    insert_user(&mut user_storage, User::new(103, "Sita", 92.25))?;

    print_all_users(&user_storage);

    match get_user(&user_storage, 101) {
        Some(user) => {
            println!(
                "Id : {} || Name : {} || TotalMarks : {:.2} \n",
                user.id, user.username, user.total_score
            )
        }
        None => println!("No user Found.\n"),
    }

    Ok(())
}
