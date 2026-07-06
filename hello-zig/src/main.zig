const std = @import("std");


pub fn main() !void {

    const year = 2002;

    if((year % 400 == 0) or (year % 100 != 0 and year % 4 == 0)) {
        std.debug.print("The year is a leap year", .{});
    } else {
        std.debug.print("The year ain't a leap year", .{});
    }
        
    
    
}