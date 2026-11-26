use std::collections::HashMap;

fn bait_logic(enemy_list: &HashMap<&str, i32>) {
    let mut bait_hp = i32::MAX;
    let mut bait_name = "";

    for (name, hp) in enemy_list.iter() {
        if *hp < bait_hp {
            bait_hp = *hp;
            bait_name = name;
        }
    }

    println!("Bait: {bait_name}, HP: {bait_hp}");
}

fn main() {
    let mut enemies_and_hp: HashMap<&str, i32> = HashMap::new();

    enemies_and_hp.insert("Small Boss", 12_000);
    enemies_and_hp.insert("Medium Boss", 25_000);
    enemies_and_hp.insert("Large Boss", 30_000);

    bait_logic(&enemies_and_hp);

    enemies_and_hp.remove(&"Small Boss");
    bait_logic(&enemies_and_hp);
}
