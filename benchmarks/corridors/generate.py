#!/usr/bin/env python3
"""Recreate the deterministic two-room corridor benchmark fixtures."""

from collections import deque
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUTPUT = HERE / "generated-corridor-benchmark"
OUTPUT.mkdir(exist_ok=True)
ROOM_WIDTH = 5
HEIGHT = 7


def create_case(name, length, corridor_width, one_way=False):
    width = 2 * ROOM_WIDTH + length
    middle = HEIGHT // 2
    rows = []
    for y in range(HEIGHT):
        opening = middle - (corridor_width - 1) // 2 <= y < middle - (corridor_width - 1) // 2 + corridor_width
        rows.append('.' * ROOM_WIDTH + ('.' if opening else 'T') * length + '.' * ROOM_WIDTH)

    map_name = name + '.map'
    (OUTPUT / map_name).write_text(
        f'type octile\nheight {HEIGHT}\nwidth {width}\nmap\n' + '\n'.join(rows) + '\n'
    )

    # Closest to the entrance first. Alternating sides keeps every even prefix balanced.
    slots = [(x, y) for x in (4, 3, 2, 1) for y in (3, 2, 4, 1, 5)]
    starts = []
    
    for i in range(8):
        left = one_way or i % 2 == 0
        x, y = slots[i if one_way else i // 2]
        starts.append((x if left else width - 1 - x, y))

    goals = [(width - 1 - x, y) for x, y in starts]

    def distance(start, goal):
        pending = deque([(start, 0)])
        visited = {start}

        while pending:
            (x, y), d = pending.popleft()
            if (x, y) == goal:
                return d
            for nx, ny in ((x, y - 1), (x + 1, y), (x, y + 1), (x - 1, y)):
                if 0 <= nx < width and 0 <= ny < HEIGHT and rows[ny][nx] == '.' and (nx, ny) not in visited:
                    visited.add((nx, ny))
                    pending.append(((nx, ny), d + 1))
        raise ValueError('Unreachable goal')

    assert len(set(starts)) == len(set(goals)) == 8
    lines = ['version 1']
    for (sx, sy), (gx, gy) in zip(starts, goals):
        lines.append(f'0\t{map_name}\t{width}\t{HEIGHT}\t{sx}\t{sy}\t{gx}\t{gy}\t{distance((sx, sy), (gx, gy))}')
    (OUTPUT / (name + '.scen')).write_text('\n'.join(lines) + '\n')


cases = [
    ('short-one-way', 4, 1, True),
    ('short-opposing', 4, 1, False),
    ('medium-opposing', 8, 1, False),
    ('long-opposing', 16, 1, False),
    ('medium-wide', 8, 2, False),
    ('long-wide', 16, 2, False),
    ('length-20-opposing', 20, 1, False),
    ('length-25-opposing', 25, 1, False),
    ('length-30-opposing', 30, 1, False),
    ('length-20-wide', 20, 2, False),
    ('length-25-wide', 25, 2, False),
    ('length-30-wide', 30, 2, False),
]
for case in cases:
    create_case(*case)

(OUTPUT / 'inputs.txt').write_text(''.join(
    f'benchmarks/corridors/generated-corridor-benchmark/{name}.map benchmarks/corridors/generated-corridor-benchmark/{name}.scen\n'
    for name, *_ in cases
))
