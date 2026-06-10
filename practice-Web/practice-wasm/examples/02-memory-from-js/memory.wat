(module
  (memory (export "memory") 1)

  (func $sum_i32 (param $offset i32) (param $count i32) (result i32)
    (local $index i32)
    (local $sum i32)

    loop $loop
      local.get $index
      local.get $count
      i32.lt_s
      if
        local.get $sum
        local.get $offset
        local.get $index
        i32.const 4
        i32.mul
        i32.add
        i32.load
        i32.add
        local.set $sum

        local.get $index
        i32.const 1
        i32.add
        local.set $index

        br $loop
      end
    end

    local.get $sum)

  (export "sum_i32" (func $sum_i32)))

