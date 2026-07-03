const std = @import("std");

const Operation = enum {
    add,
    product,
};

fn calculator(comptime op: Operation, a: i32, b: i32) i32 {
    return switch (op) {
        .add => a + b,
        .product => a * b,
    };
}

fn divide(a: i32, b: i32) !i32 {
    if (b == 0) {
        return error.DivideByZero;
    }
    return @divTrunc(a, b);
}

fn calculate() !void {
    const result = try divide(10, 0);
    std.debug.print("{}\n", .{result});
}

pub fn main() !void {
    std.debug.print("{}\n", .{calculator(.add, 4, 5)});
    std.debug.print("{}\n", .{calculator(.product, 4, 45)});

    try calculate();
}
