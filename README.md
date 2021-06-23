# Test for libunwind and OpenSSL

Tries to use libunwind to unwind a stack that ends with OpenSSL's `bn_mul_mont` function. Causes a crash due to either some strange `RSP` usage or a bug in libunwind.

`gdb` is able to unwind the stack.

Download this repo and run

```
git clone https://github.com/tmhort/backtrace.git --recurse-submodules
cd backtrace
./run.sh
./test_bt
<crash>
gdb test_bt core
```
