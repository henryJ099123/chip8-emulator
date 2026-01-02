# Test Information

1. `test_arithmetic_8XYZ`: tests instructions `8XY0...8XYE`.
Expect 9 check marks. If there is an X, the instruction that failed counts
downward (i.e. in column major order).
2. For instructions that read keys from registers, it only reads the last
nibble, as written in [this resource](). This may break with some programs
that expect the `F` key, for example, `EX9E`, if `VX` stores `1F` instead of `0F`.
3. 

