# Test for libunwind and OpenSSL

Tries to use libunwind to unwind a stack that ends with OpenSSL's `bn_mul_mont` function. Causes a crash due to either some strange `RSP` usage or a bug in libunwind.

`gdb` is able to unwind the stack.

Download this repo and run

```
git clone https://github.com/tmhort/backtrace.git
cd backtrace
git submodule update --init
make
./test_bt
<crash>
gdb test_bt core
```

Alternatively, the `run.sh` script can be used to build everything.

Do the following to increase debug from libunwind:
```
export UNW_DEBUG_LEVEL=15
```

The libunwind submodule is a fork of https://github.com/libunwind/libunwind (as of Jun 23, 2021).
It includes a small patch to output via `Debug()` the values that pop()/push()/pick() return/use
when processing CFI CFA expressions.

To run within `gdb`, issue the following commands before `run`:
```
set env UNW_DEBUG_LEVEL=15
handle SIG35 nostop noprint noignore
```

The signal handler is necessary because the test program uses a timeer to periodically invoke
`unw_backtrace()`. The program will eventually invoke this during OpenSSL's `bn_mul_mont()`
which will cause a segmentation fault.

It appears that libunwind is unable to correctly calculate the values (registers) in a CFI CFA
expression. In this case, the expression is:
```
.cfi_cfa_expression %rsp+8,%r9,8,mul,plus,deref,+8
```
(See https://github.com/openssl/openssl/blob/OpenSSL_1_1_1-stable/crypto/bn/asm/x86_64-mont.pl#L172)

The value of `%rsp` appears correct, but the value of `%r9` is incorrect (way too big), and the
`mult` operation blows this value up, such that the eventual `deref` causes a segmentation fault.
