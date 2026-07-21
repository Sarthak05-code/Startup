fn main() {
    // _ ensure the compiler won't ask about the used array
    let _demo_array = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    let array_first_name = [
        "Sarthak",
        "Sagar",
        "Sangam",
        "Shavya",
        "Shuvam",
        "Shreeya",
        "Sneha",
        "Sampradha",
        "Supnil",
        "Sudil",
    ];

    let array_last_name = [
        "Thapa",
        "Baskoti",
        "Manandhar",
        "Dangol",
        "Shah",
        "Shrestha",
        "Sigdel",
        "Bajracharya",
        "Basnet",
        "Maharjan",
    ];
    // seems confusing, but ill learn this slowly.
    for (first, last) in array_first_name
        .iter()
        .flat_map(|f| array_last_name.iter().map(move |l| (f, l)))
    {
        println!("{} {}", first, last);
    }
}
