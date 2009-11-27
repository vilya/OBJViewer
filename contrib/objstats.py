#!/usr/bin/env python
import sys


def main():
  low = [10000.0] * 3
  high = [-10000.0] * 3

  with open(sys.argv[1]) as f:
    for v in (map(float, line[2:].split()) for line in f if line.startswith('v ')):
      low = [min(*c) for c in zip(low, v)]
      high = [max(*c) for c in zip(high, v)]

  for result in zip( ('x', 'y', 'z'), low, high ):
    print "%s = %f - %f" % tuple(result)


if __name__ == '__main__':
  main()

