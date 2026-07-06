const std = @import("std");

pub fn main() !void {
    var result : usize = 1;
    const number : usize = 4;
    for(0..number + 1) |i| { // since i dont know how to do <= , ill use the + 1 tactic
        result *= i;
    }
    std.debug.print("{}\n", .{result});
    
}