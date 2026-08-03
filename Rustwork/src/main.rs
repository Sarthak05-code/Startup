#![allow(warnings)]

pub unsafe fn unsafe_swap<T>(x: *mut T, y: *mut T) {
    let mut temp = std::mem::MaybeUninit::<T>::uninit();

    std::ptr::copy_nonoverlapping(x, temp.as_mut_ptr(), 1);

    std::ptr::copy_nonoverlapping(y, x, 1);

    std::ptr::copy_nonoverlapping(temp.as_ptr(), y, 1);
}

fn main() {
    let mut a: String = String::from("Hello");
    let mut b: String = String::from("World");

    println!("Before swap a = {a} b = {b}");

    let ptr_a: *mut String = &mut a;
    let ptr_b: *mut String = &mut b;

    unsafe {
        unsafe_swap(ptr_a, ptr_b);
    }

    println!("After swap : {a} {b}");
}
