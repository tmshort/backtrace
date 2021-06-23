
#include <iostream>
#include <chrono>
#include <assert.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <libunwind.h>
#include <atomic>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string.h>
#include <cstdint>

#define BN_ULONG        unsigned long 

#define SIGHEARTBEAT (SIGRTMIN + 1)
#define gettid() (long int)syscall(__NR_gettid)

extern "C"
{
int bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp, const BN_ULONG *np, const BN_ULONG *n0, int num);
}

int f3(uint8_t n1, uint16_t n2, uint64_t n3, uint8_t n4, uint32_t n5, uint8_t n6, uint32_t n7, uint16_t n8, uint64_t n9)
{
  volatile uint64_t ctr = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + rand();

  unsigned long r = 0;
  unsigned long *a, *b, *orig_a, *orig_b;
  //unsigned long a[4] = {18446744073709551612UL,};
  //unsigned long b[4] = {1,};
  unsigned long nd  = 18446744073709551615UL;
  unsigned long n0[2] = {0,1};
  int num = 4;

  orig_a = a = (unsigned long *)malloc(32 * sizeof(*a));
  orig_b = b = (unsigned long *)malloc(32 * sizeof(*b));
  memset(a, 0, 32 * sizeof(*a));
  memset(b, 0, 32 * sizeof(*b));
  a = &a[28];
  b = &b[28];
  a[0] = 18446744073709551612UL;
  b[0] = 1;
  bn_mul_mont(&r, a, b, &nd, n0, num);

  free(orig_a); free(orig_b);
  return ctr + nd;
}

int f2(int num, int limit, uint8_t n1, uint16_t n2, uint64_t n3, uint8_t n4, uint32_t n5, uint8_t n6, uint32_t n7, uint16_t n8, uint64_t n9)
{
  volatile uint8_t i1 = n1; i1++;
  volatile uint16_t i2 = n2; i2++;
  volatile uint64_t i3 = n3; i3++;
  volatile uint8_t i4 = n4; i4++;
  volatile uint32_t i5 = n5; i5++;
  volatile uint8_t i6 = n6; i6++;
  volatile uint32_t i7 = n7; i7++;
  volatile uint16_t i8 = n8; i8++;
  volatile uint64_t i9 = n9; i9++;
  int res = 0;

  if (num < limit)
  {
    res += f2(num + 1, limit, i1, i2, i3, i4, i5, i6, i7, i8, i9);
  }
  else
  {
    static int ctr = 0;
    ctr += res;
    res = f3(i1, i2, i3, i4, i5, i6, i7, i8, i9);
  }

  return res;
}

int f1(int n)
{
  int r = rand();
  return f2(0, rand() % 10, r + 1, r + 2, r + 3, r+4, r+5, r+6, r+7, r+8, r+9);
}

bool active = false;
std::atomic<bool> capture_st(false);

void* frames[64];
int recorded = 0;

void monitor(int /*sig*/, siginfo_t* /*info*/, void* /* ctxt */)
{
  static __thread bool tls_callingBacktrace = false;
  if (tls_callingBacktrace) return;
  if (capture_st)
  {
    tls_callingBacktrace = true;
    recorded = unw_backtrace(frames, 64);
    tls_callingBacktrace = false;
  }
}

int main()
{
    srand (time(NULL));

    // register the signal handler for stack recording
    struct sigaction act = {};
    act.sa_sigaction = monitor;
    act.sa_flags = SA_SIGINFO|SA_RESTART;
    ::sigfillset(&act.sa_mask);
    assert(::sigaction(SIGHEARTBEAT, &act, NULL) >= 0);

    // initialize timer
    timer_t timer;
    ::sigevent_t event = {};
    event.sigev_notify = SIGEV_THREAD_ID;
    event.sigev_signo = SIGHEARTBEAT;
    event.sigev_value.sival_ptr = NULL;
    event._sigev_un._tid = gettid();
    assert(::timer_create(CLOCK_MONOTONIC, &event, &timer) == 0);

    // start timer
    int64_t const pulse = 10000000;     // nsecs - heartbeat interrupt interval
    static struct itimerspec interval = { {0, pulse}, {0, pulse} };
    assert(::timer_settime(timer, 0, &interval, NULL) == 0);


    auto start = std::chrono::system_clock::now();
    uint64_t numOps = 100000000000UL;

    static int ctr = 0;
    for (uint64_t i = 0; i < numOps; i++)
    {
      capture_st = true;
      f1(ctr++);
      capture_st = false;

      if (recorded)
      {
        std::cout << "recorded stack: ";
        for (int i = 0; i < recorded; i++)
        {
          std::cout << ".";
          //std::cout << frames[i] << " ";
        }
        std::cout << std::endl;
      }

      recorded = 0;
    }

    auto end = std::chrono::system_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "avgTime=" << totalTime/numOps << std::endl;

    return 0;
}
