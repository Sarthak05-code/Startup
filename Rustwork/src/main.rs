

fn _trapezoid(x : &Vec<i32> , y : &Vec<i32> , n : i32 ) -> f32 {

    let h = ();
    1.0
}



fn main() {
    let a : f64 = 1.0;
    let b : f64 = 4.0;
    let x = [12,11,23,17, 21];
    let _y = [10 ,31,43,19, 22];

    let n = x.len();

    let result = simpson_one_three(a , b , n.try_into().unwrap());
    println!("{} is the value" , result);
    
}

fn simpson_one_three(a: f64, b: f64 , n : i32) -> f64 {
    let  h  : f64  = (b - a) / n as f64;

    h 
}