import subprocess
import sys
import time

cmd = [
    "qemu-system-arm",
    "-M",
    "raspi2b",
    "-kernel",
    "build/raspi2/armv7-a/debug/kernel.elf",
    "-serial",
    "stdio",
    "-drive",
    "if=sd,id=sd0,format=raw,file=image/disk.img",
    "-smp",
    "4",
    "-nographic",
]

p = subprocess.Popen(
    cmd,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    bufsize=0,
)
out = b""
sent_ps = False
start = time.time()
deadline = start + 45
while time.time() < deadline:
    chunk = p.stdout.read(1)
    if not chunk:
        if p.poll() is not None:
            break
        continue
    out += chunk
    sys.stdout.write(chunk.decode("utf-8", errors="ignore"))
    sys.stdout.flush()
    text = out.decode("utf-8", errors="ignore")
    if (not sent_ps) and ("yiyiya$" in text or "Welcome" in text):
        time.sleep(2)
        p.stdin.write(b"ps\n")
        p.stdin.flush()
        sent_ps = True
    if sent_ps and b"faults ticks" in out:
        time.sleep(0.5)
        break
    if sent_ps and (time.time() - start > 20):
        break

p.kill()
print("\n--- captured ps section ---")
for line in text.splitlines():
    if line.strip() and (
        line.startswith("id ")
        or line[0:1].isdigit()
        or "monitor" in line
        or "ap " in line
    ):
        print(line)
