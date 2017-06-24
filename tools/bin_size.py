#!/usr/bin/python
import subprocess
import sys
from collections import defaultdict

print(sys.argv[1])
output = subprocess.check_output(['nm', '-S', '--size-sort', '--demangle', '--print-file-name', sys.argv[1]])

lines = output.splitlines()
print(lines[0])

summary = defaultdict(lambda: 0)
max_val = 0
max_line = ''
for x in lines:
    space1 = x.find(' ')+1
    space2 = x[space1:].find(' ')+1
    space3 = x[space1+space2:].find(' ')
    size_s = x[space1:space1+space2]
    size = int(size_s, 16)
    section_s = x[space1+space2:space1+space2+space3]
    section_s = section_s.strip().upper()
    if section_s != 'T':
        continue
    if size > max_val:
        max_val = size
        max_line = x
    try:
        archive, obj, addr = x[:space1].split(':')
    except ValueError:
        obj, addr = x[:space1].split(':')
    #print(obj, size)
    summary[obj] += size

sorter = []
sum = 0
for k,v in summary.iteritems():
    sorter.append((k ,v))
    sum += v

sorter.sort(key=lambda k: k[1])
for s in sorter:
    print('total', s)
print('max_line', max_line)
print('sum', sum)
