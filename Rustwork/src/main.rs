use std::{fs::OpenOptions, io::Read};

fn main() -> std::io::Result<()> {
    let mut sql = OpenOptions::new().read(true).open("sql.txt")?;

    let mut text = String::new();
    sql.read_to_string(&mut text)?;
    println!("{}", text);
    Ok(())
}
