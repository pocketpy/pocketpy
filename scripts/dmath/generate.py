"""Generate/check per-function cases from the independent oracle.

Run: python scripts/dmath/generate.py [--check]
Existing frozen output bits are preserved only if the case name and inputs
match. New cases are emitted as PENDING and fail normal test runs until their
results have been reviewed on independent compiler builds. This script never
calls the implementation or silently blesses changed results.
"""

import argparse
from decimal import localcontext, InvalidOperation, DivisionByZero, Overflow
from pathlib import Path
import re
import cases
from oracle import reference, value, INF, MASK, SIGN

ROOT = Path(__file__).resolve().parents[2]
DEST = ROOT / 'tests/dmath/cases'


def result_text(u):
    magnitude = u & MASK
    if magnitude > INF:
        return 'nan'
    if magnitude == INF:
        return '-inf' if u & SIGN else '+inf'
    return '%016x' % u


def old_results(path):
    rows, sweep = {}, 'PENDING'
    if path.exists():
        for line in path.read_text().splitlines():
            if line.startswith('# sweep '):
                sweep = line.split()[2]
            elif line and not line.startswith('#'):
                fields = line.split()
                rows[tuple(fields[:3])] = [result_text(int(x, 16)) if len(x) == 16 else x
                                           for x in fields[3:5]]
    return rows, sweep


def render(name, rows, path):
    existing, sweep = old_results(path)
    text = ['# dmath_' + name,
            '# Independent oracle: Decimal at 430 and 570 digits / exact Fraction arithmetic.',
            '# Frozen bits and sweep require review; generation never recalibrates them.',
            '# Nonfinite outputs: nan checks classification; +/-inf checks classification and sign.',
            '# sweep ' + sweep,
            '# case  input0  input1  frozen0  frozen1  reference0  reference1  max_ulp']
    for row in rows:
        refs = []
        for precision in (430, 570):
            with localcontext() as context:
                context.prec = precision
                for trap in (InvalidOperation, DivisionByZero, Overflow):
                    context.traps[trap] = False
                refs.append(reference(name, row.x, row.y))
        if refs[0] != refs[1]:
            raise RuntimeError('reference did not converge: ' + name + '/' + row.name)
        outputs = refs[0] + (0,) * (2 - len(refs[0]))
        key = (row.name, '%016x' % row.x, '%016x' % row.y)
        frozen = existing.get(key, ['PENDING', 'PENDING'])
        # Human-readable inputs stay next to their exact binary representations.
        comment = ' # x=' + value(row.x).hex() + ' y=' + value(row.y).hex()
        fields = [*key, *frozen, *[result_text(r) for r in outputs], str(cases.ULPS[name])]
        text.append(' '.join(fields) + comment)
    return '\n'.join(text) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    exported = set(re.findall(r'^(?:double|int|void) dmath_(\w+)\(',
                             (ROOT / 'include/pocketpy/common/dmath.h').read_text(), re.M))
    designed = {n for names in cases.GROUPS.values() for n in names}
    if exported != designed:
        raise RuntimeError('missing/extra function groups: ' + repr(exported ^ designed))
    registered = set(re.findall(r'\{\s*"[a-z_]+",\s*"(\w+)",\s*T_',
                                (ROOT / 'src2/test_dmath.c').read_text()))
    if registered != designed:
        raise RuntimeError('C runner groups differ: ' + repr(registered ^ designed))
    DEST.mkdir(parents=True, exist_ok=True)
    total = 0
    for group, names in cases.GROUPS.items():
        for name in names:
            rows = getattr(cases, 'cases_' + name)()
            if len({row.name for row in rows}) != len(rows):
                raise RuntimeError('duplicate case name in ' + name)
            path = DEST / (name + '.txt')
            content = render(name, rows, path)
            if args.check:
                if not path.exists() or path.read_text() != content:
                    raise RuntimeError(str(path) + ' is out of date')
            else:
                path.write_text(content, encoding='ascii', newline='\n')
            total += len(rows)
            print(group + '/' + name + ': ' + str(len(rows)) + ' cases', flush=True)
    print(str(len(designed)) + ' functions, ' + str(total) + ' named cases')


if __name__ == '__main__':
    main()
