#[allow(dead_code)]
enum Base {
    Binary,
    Deciaml,
    Octal,
    Hexadecimal,
}
impl Base {
    fn radix(self) -> u32 {
        match self {
            Base::Binary => 2,
            Base::Deciaml => 10,
            Base::Octal => 8,
            Base::Hexadecimal => 16,
        }
    }
}

fn to_base(number: usize, base: Base, buffer: &mut [u8]) -> &str {
    let radix = base.radix() as usize;
    const DIGITS: &[u8] = b"0123456789abcdef";
    if number == 0 {
        buffer[0] = b'0';
        return std::str::from_utf8(&buffer[..1]).unwrap();
    }
    let mut temp = number;
    let mut index = 0;

    while temp > 0 {
        buffer[index] = DIGITS[temp % radix];
        temp /= radix;
        index += 1;
    }

    buffer[..index].reverse();
    std::str::from_utf8(&buffer[..index]).unwrap()
}

fn main() {
    let mut buffer = [0u8; 64];
    let binary = to_base(123, Base::Binary, &mut buffer);
    println!("Binary = {binary}");

    let mut buffer2 = [0u8; 64];
    let hex = to_base(2553, Base::Hexadecimal, &mut buffer2);
    println!("The hex value is {hex}");
}
