const std = @import("std");

pub fn main(init : std.process.Init) !void {
    
        const gpa = init.gpa;
    

        const args = try init.minimal.args.toSlice(init.arena.allocator());

        std.log.info("Program has {d} args", .{args.len});
        for (args, 0..) |arg , i| {
            std.log.info(" args[{d}] = {s}  ", .{i , arg});
        }

        std.log.info("{d} env vars available", .{init.environ_map.count()});
        const greeting = try std.fmt.allocPrint(gpa, "Hello {s}!", .{"Zig 0.16.0"});

        defer gpa.free(greeting);
    
}