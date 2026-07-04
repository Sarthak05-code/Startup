const std = @import("std");

fn checker(x: i32) []const u8 {
    return switch (x) {
        0 => "Zero",
        else => if (@mod(x, 2) == 0)
            "Even"
        else
            "Odd",
    };
}

fn fibonacci(x: i32) i32 {
    if (x <= 1) // 0 and 1
        return x;
    return fibonacci(x - 2) + fibonacci(x - 1);
}

pub fn main() !void {
    const array: [10]u8 = .{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    for (array) |arr| {
        std.debug.print("{} => {s} and Fibonacci-Sequence => {}\n", .{ arr, checker(arr), fibonacci(arr) });
    }
}
