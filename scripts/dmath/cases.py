"""The reviewed test design: one case group per public dmath function.

Hexadecimal inputs name exact binary64 values, not approximate decimal inputs.
Every boundary expansion retains its purpose in the emitted case name.
These groups were designed afresh; no legacy vectors or fingerprints are used.
"""

from dataclasses import dataclass
from oracle import bits, value, SIGN, INF


@dataclass(frozen=True)
class Case:
    name: str
    x: int
    y: int = 0


def word(x):
    return bits(float.fromhex(x)) if isinstance(x, str) else bits(float(x))


def c(name, x, y=0):
    return Case(name, word(x), word(y))


def edge(name, x, both_signs=False):
    u = word(x)
    assert 0 < u < INF
    rows = [Case(name + suffix, u + offset) for suffix, offset in
            [('_below', -1), ('_at', 0), ('_above', 1)]]
    if both_signs:
        rows += [Case('negative_' + r.name, r.x | SIGN) for r in rows]
    return rows


def special():
    return [
        Case('positive_zero', 0), Case('negative_zero', SIGN),
        Case('least_subnormal', 1), Case('negative_least_subnormal', SIGN | 1),
        Case('third_subnormal', 3), Case('negative_third_subnormal', SIGN | 3),
        Case('largest_subnormal', 0x000FFFFFFFFFFFFF),
        Case('negative_largest_subnormal', 0x800FFFFFFFFFFFFF),
        Case('least_normal', 0x0010000000000000),
        Case('negative_least_normal', 0x8010000000000000),
        Case('largest_finite', 0x7FEFFFFFFFFFFFFF),
        Case('negative_largest_finite', 0xFFEFFFFFFFFFFFFF),
        Case('positive_infinity', INF), Case('negative_infinity', INF | SIGN),
        Case('quiet_nan_payload', 0x7FF8ABCDEF135790),
        Case('negative_quiet_nan_payload', 0xFFF8ABCDEF135790),
        Case('signaling_nan_payload', 0x7FF0000000010248),
        Case('negative_signaling_nan_payload', 0xFFF0000000010248),
    ]


def cases_isfinite():
    return special() + [c('ordinary_positive', '0x1.abc123p+37'),
                        c('ordinary_negative', '-0x1.2468p-137')]


def cases_isinf():
    return special() + [Case('exponent_below_infinity', 0x7FE0000000000000),
                        Case('first_nan_encoding', INF + 1)]


def cases_isnan():
    return special() + [Case('all_fraction_bits_set', 0x7FFFFFFFFFFFFFFF),
                        Case('negative_all_bits_set', 0xFFFFFFFFFFFFFFFF)]


def cases_isnormal():
    return special() + edge('normal_transition', '0x1p-1022', True)


def cases_fabs():
    return special() + [c('negative_fraction', '-0x1.96p-4'),
                        c('negative_large_integral', '-0x1.23456789abcdep+89')]


def cases_copysign():
    rows = []
    for r in special() + [c('ordinary', '0x1.b38p+6')]:
        rows += [Case(r.name + '_positive_sign', r.x, word('0x1p-81')),
                 Case(r.name + '_negative_sign', r.x, word('-0x1p+81'))]
    rows += [Case('sign_from_negative_nan', word(7.625), 0xFFF0000000010248),
             Case('sign_from_negative_zero', word(7.625), SIGN),
             Case('sign_from_positive_nan', word(-7.625), 0x7FF8ABCDEF135790)]
    return rows


def ordering_cases():
    pairs = [(0, SIGN), (SIGN, 0), (SIGN, SIGN), (0, 0),
             (word(-7.625), word(3.1875)), (1, 3),
             (INF, word(11.125)), (INF | SIGN, word(-11.125))]
    rows = [Case('ordered_pair_' + str(i), a, b) for i, (a, b) in enumerate(pairs)]
    for nan in special()[-4:]:
        rows += [Case(nan.name + '_left', nan.x, word(13.25)),
                 Case(nan.name + '_right', word(13.25), nan.x),
                 Case(nan.name + '_both', nan.x, 0xFFF8ABCDEF135790)]
    return rows


def cases_fmin():
    return ordering_cases() + [c('equal_negative_numbers', -19.875, -19.875)]


def cases_fmax():
    return ordering_cases() + [c('equal_positive_numbers', 19.875, 19.875)]


def rounding_boundaries():
    rows = special()
    for label, x in [('unit', '0x1p+0'), ('half', '0x1p-1'),
                     ('word_split', '0x1p+20'), ('signed32', '0x1p+31'),
                     ('last_fractional_binade', '0x1p+51'),
                     ('integral_binade', '0x1p+52'), ('signed64', '0x1p+63')]:
        rows += edge(label, x, True)
    return rows


def cases_ceil():
    return rounding_boundaries() + [c('up_from_positive_fraction', 23.0625),
                                    c('up_from_negative_fraction', -23.0625)]


def cases_floor():
    return rounding_boundaries() + [c('down_from_positive_fraction', 23.9375),
                                    c('down_from_negative_fraction', -23.9375)]


def cases_trunc():
    return rounding_boundaries() + [c('drop_positive_fraction', 37.6875),
                                    c('drop_negative_fraction', -37.6875)]


def cases_modf():
    return rounding_boundaries() + [c('split_positive', 129.8125),
                                    c('split_negative', -129.8125),
                                    c('exact_negative_integer', -129)]


def cases_fmod():
    rows = [c('positive_remainder', 53.75, 7.5), c('negative_remainder', -53.75, 7.5),
            c('negative_divisor', 53.75, -7.5), c('both_negative', -53.75, -7.5),
            c('positive_exact_multiple', 52.5, 7.5),
            c('negative_exact_multiple', -52.5, 7.5),
            c('quotient_exceeds_double', '0x1.abcdefp+900', '0x1.7p-901'),
            c('subnormal_remainder', '0x1.0000000000001p-1022', '0x1p-1022'),
            Case('subnormal_division', 29, 7), Case('negative_subnormal_division', SIGN | 29, 7)]
    for r in special():
        rows += [Case(r.name + '_dividend', r.x, word(7.5)),
                 Case(r.name + '_divisor', word(-53.75), r.x)]
    return rows


def cases_sqrt():
    return (special() + [c('exact_square', 206.640625), c('irrational', 13.625),
                         c('wide_significand', '0x1.a5c739b18426fp+409')]
            + edge('square_neighbor', '0x1.9d48p+7')
            + edge('normal_subnormal_transition', '0x1p-1022'))


def cases_cbrt():
    rows = special() + [c('exact_positive_cube', 155.287109375),
                        c('exact_negative_cube', -155.287109375),
                        c('irrational_positive', 19.375), c('irrational_negative', -19.375)]
    for rem in range(3):
        rows += edge('exponent_class_' + str(rem), '0x1.73p+' + str(600 + rem), True)
    return rows


def cases_exp():
    rows = special() + [c('moderate_positive', 6.3125), c('moderate_negative', -6.3125),
                        c('large_finite_result', 708.375), c('subnormal_result', -738.625)]
    for name, x in [('tiny_shortcut', '0x1p-28'), ('first_reduction', '0x1.62e43p-2'),
                    ('second_reduction', '0x1.0a2b2p+0'),
                    ('overflow_boundary', '0x1.62e42fefa39efp+9')]:
        rows += edge(name, x, True)
    # The last nonzero result lies at this negative magnitude.
    rows += [Case('underflow_' + r.name, r.x | SIGN)
             for r in edge('threshold', '0x1.74910d52d3051p+9')]
    return rows


def cases_exp2():
    rows = special() + [c('fractional_positive', 12.3125), c('fractional_negative', -12.3125),
                        c('exact_normal_power', -847), c('subnormal_power', -1053)]
    for n in (9, 67, 143, 219):
        rows += edge('table_midpoint_' + str(n), value(word((n + 0.5) / 256)))
    rows += edge('overflow', '0x1p+10')
    rows += [Case('underflow_' + r.name, r.x | SIGN)
             for r in edge('tie', 1075)]
    return rows


def cases_exp10():
    return (special() + [c('exact_positive_power', 17), c('negative_power', -17),
                         c('fractional_positive', 4.1875), c('fractional_negative', -4.1875),
                         c('subnormal_result', -319.75)]
            + edge('overflow', '0x1.34413509f79ffp+8'))


def log_boundaries():
    return (special() + edge('near_one', 1) + edge('normalize', '0x1.6a09ep+0')
            + [c('less_than_one', 0.171875), c('greater_than_one', 23.5625)])


def cases_log():
    return log_boundaries() + [c('wide_exponent', '0x1.a9bcdef012345p+723')]


def cases_log2():
    return log_boundaries() + [c('exact_binary_power', '0x1p-817'),
                               c('exact_subnormal_power', '0x1p-1053')]


def cases_log10():
    return log_boundaries() + [c('decimal_power', 10000000),
                               c('small_decimal', 0.00000003125)]


def cases_log_base():
    rows = [c('base_above_one', 19.375, 3.25), c('base_below_one', 19.375, 0.3125),
            c('argument_below_one', 0.3125, 3.25), c('equal_base_and_argument', 3.25, 3.25),
            c('zero_result_sign', 1, 0.3125), c('base_one_positive', 19.375, 1),
            c('base_one_negative', 0.3125, 1), c('both_one', 1, 1),
            c('near_one_pair', '0x1.0000000000003p+0', '0x1.0000000000007p+0')]
    for r in special():
        rows += [Case(r.name + '_argument', r.x, word(3.25)),
                 Case(r.name + '_base', word(19.375), r.x)]
    rows += [Case('both_infinite', INF, INF), Case('both_zero', 0, 0)]
    return rows


def cases_pow():
    rows = [c('positive_fractional', 3.3125, 2.1875),
            c('negative_odd_integer', -3.3125, 7), c('negative_even_integer', -3.3125, 6),
            c('negative_reciprocal', -3.3125, -7), c('negative_fractional_domain', -3.3125, 2.1875),
            c('square_shortcut', '0x1.abcdef1234567p+5', 2),
            c('cube_shortcut', '0x1.abcdef1234567p+5', 3),
            c('fourth_power_shortcut', '0x1.abcdef1234567p+5', 4),
            c('sqrt_shortcut', 23.5625, 0.5), c('reciprocal_shortcut', 23.5625, -1),
            c('near_one_large_positive', '0x1.0000000000003p+0', '0x1p+49'),
            c('near_one_large_negative', '0x1.ffffffffffffbp-1', '-0x1p+49'),
            c('finite_overflow', 7.625, 4096), c('finite_underflow', 7.625, -4096),
            c('negative_odd_overflow', -7.625, 4097), c('negative_odd_underflow', -7.625, -4097),
            c('largest_odd_exponent', -1, '0x1.fffffffffffffp+52'),
            c('first_even_only_binade', -1, '0x1p+53')]
    for r in special():
        rows += [Case(r.name + '_odd_power', r.x, word(7)),
                 Case(r.name + '_negative_odd_power', r.x, word(-7)),
                 Case(r.name + '_zero_power', r.x, 0),
                 Case(r.name + '_exponent', word(3.3125), r.x)]
    rows += [Case('one_to_nan', word(1), 0xFFF0000000010248),
             Case('negative_one_to_infinity', word(-1), INF),
             Case('negative_zero_fractional_pole', SIGN, word(-0.25))]
    return rows


def angle_cases():
    rows = special() + [c('small_positive_angle', 0.34375), c('small_negative_angle', -0.34375),
                        c('moderate_positive_angle', 43.8125), c('moderate_negative_angle', -43.8125),
                        c('large_reduction', '0x1.a5c739b18426fp+41'),
                        c('very_large_reduction', '0x1.73b4a82cf19dep+811')]
    rows += edge('quarter_turn_kernel', '0x1.921fb54442d18p-1', True)
    rows += edge('medium_reduction_cutoff', '0x1.921fbp+20', True)
    return rows


def cases_sin():
    return angle_cases() + edge('tiny_cutoff', '0x1p-26', True)


def cases_cos():
    return angle_cases() + edge('tiny_cutoff', '0x1.6a09ep-27', True)


def cases_tan():
    return (angle_cases() + edge('reciprocal_pole', '0x1.2d97c7f3321d2p+2', True)
            + edge('kernel_transform', '0x1.59428p-1', True))


def cases_sincos():
    return angle_cases() + [c('second_quadrant', 2.3125), c('third_quadrant', 4.1875),
                            c('fourth_quadrant', 5.9375)]


def cases_asin():
    return (special() + edge('domain_endpoint', 1, True) + edge('half_interval', 0.5, True)
            + edge('near_endpoint_formula', '0x1.f3333p-1', True)
            + [c('interior_positive', 0.71875), c('interior_negative', -0.71875)])


def cases_acos():
    return (special() + edge('domain_endpoint', 1, True) + edge('half_interval', 0.5, True)
            + edge('tiny_cutoff', '0x1p-57', True)
            + [c('interior_positive', 0.28125), c('interior_negative', -0.28125)])


def cases_atan():
    rows = special() + [c('ordinary_positive', 9.3125), c('ordinary_negative', -9.3125)]
    for label, x in [('tiny_cutoff', '0x1p-27'), ('first_interval', 0.4375),
                     ('second_interval', 0.6875), ('third_interval', 1.1875),
                     ('reciprocal_interval', 2.4375), ('asymptote', '0x1p+66')]:
        rows += edge(label, x, True)
    return rows


def cases_atan2():
    rows = []
    for i, (y, x) in enumerate([(5.8125, 0.21875), (5.8125, -0.21875),
                                (-5.8125, 0.21875), (-5.8125, -0.21875)]):
        rows.append(c('quadrant_' + str(i + 1), y, x))
    for y in [0, SIGN, INF, INF | SIGN]:
        for x in [0, SIGN, INF, INF | SIGN]:
            rows.append(Case('axes_%016x_%016x' % (y, x), y, x))
    for r in special():
        rows += [Case(r.name + '_ordinate', r.x, word(-5.8125)),
                 Case(r.name + '_abscissa', word(-5.8125), r.x)]
    rows += [c('tiny_ratio', '0x1p-1000', '0x1p+900'),
             c('huge_ratio', '0x1p+900', '0x1p-1000'),
             c('tiny_ratio_negative_x', '0x1p-1000', '-0x1p+900'),
             c('unit_abscissa_shortcut', 1.8125, 1)]
    return rows


# Declaration order is the category order in the test reports.
GROUPS = {
    'classification': ['isfinite', 'isinf', 'isnan', 'isnormal'],
    'sign_and_order': ['fabs', 'copysign', 'fmin', 'fmax'],
    'rounding_and_remainder': ['ceil', 'floor', 'trunc', 'modf', 'fmod'],
    'roots': ['sqrt', 'cbrt'],
    'exponentials': ['exp', 'exp2', 'exp10', 'pow'],
    'logarithms': ['log', 'log2', 'log10', 'log_base'],
    'trigonometry': ['sin', 'cos', 'tan', 'sincos'],
    'inverse_trigonometry': ['asin', 'acos', 'atan', 'atan2'],
}

# Tolerances apply only to the independent accuracy oracle, never to the frozen
# implementation results. They are case-suite budgets, not universal bounds.
ULPS = {name: 0 for name in sum(list(GROUPS.values())[:4], [])}
ULPS.update(cbrt=1, exp=1, exp2=1, exp10=2, pow=2, log=1, log2=1, log10=1,
            log_base=3, sin=1, cos=1, tan=1, sincos=1, asin=1, acos=1, atan=1, atan2=2)
