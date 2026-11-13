"""Conservative lexical partitions of direct, read-only INTEGER aliases.

Only classify a variable when every source occurrence is its declaration or a
literal comparison/truth test/constant mask. Any remaining use (helper argument,
write, indexing, arithmetic, variable bound) leaves the domain unknown. These
are C++ source partitions, never original-code proof or dynamic coverage.
"""
from __future__ import annotations

import re

INT_MIN, INT_MAX = -(1 << 31), (1 << 31)-1
LITERAL = r'(?:-?0[xX][0-9a-fA-F]+|-?\d+)'


def clean(text):
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group().count('\n'), text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def literal(text):
    return int(text, 16 if 'x' in text.lower() else 10)


def compare(value, op, bound):
    return {'==': value == bound, '!=': value != bound, '<': value < bound,
            '<=': value <= bound, '>': value > bound, '>=': value >= bound}[op]


def fingerprint(value, spec):
    if spec['kind'] == 'automated_cpp_comparison_partition':
        return tuple(compare(value, op, bound) for op, bound in spec['comparisons'])
    if spec['kind'] == 'automated_cpp_mask_partition':
        return value & spec['mask']
    raise ValueError(spec['kind'])


def classify(text, runtime_index):
    code = clean(text)
    param = r'params\s*\[\s*' + str(runtime_index) + r'\s*\]'
    cast = r'(?:int\s*\(\s*' + param + r'\s*\)|\(\s*int\s*\)\s*' + param + r')'
    declarations = list(re.finditer(r'\b(?:const\s+)?int\s+(\w+)\s*=\s*' + cast + r'\s*;', code))
    if not declarations:
        return None
    # Different aliases and helper/global name collisions need real dataflow.
    aliases = {m[1] for m in declarations}
    if len(aliases) != 1:
        return None
    # A second numeric/direct use of params[index] must not be hidden by a
    # conveniently named selector alias.
    for mention in re.finditer(param, code):
        if not any(d.start() <= mention.start() and mention.end() <= d.end() for d in declarations):
            return None
    name = aliases.pop()
    word = r'\b' + re.escape(name) + r'\b'
    covered = []
    for match in declarations:
        token = re.search(word, match.group())
        covered.append((match.start()+token.start(), match.start()+token.end()))

    comparisons, masks, evidence = set(), set(), []

    def accept(match, token_pattern=word):
        token = re.search(token_pattern, match.group())
        span = (match.start()+token.start(), match.start()+token.end())
        covered.append(span)
        evidence.append(dict(line=code.count('\n', 0, match.start())+1, expression=match.group()))

    def starts_operand(match):
        prefix = code[:match.start()].rstrip()
        return not prefix or prefix[-1] in '(=?:,;!&|'

    end_operand = r'\b(?=\s*(?:[)?:;,}&|]|$))'
    for match in re.finditer(word + r'\s*(==|!=|<=|>=|<|>)\s*(' + LITERAL + r')' + end_operand, code):
        if not starts_operand(match):
            continue
        comparisons.add((match[1], literal(match[2])))
        accept(match)
    reverse_op = {'==': '==', '!=': '!=', '<': '>', '<=': '>=', '>': '<', '>=': '<='}
    for match in re.finditer(r'(?<![\w.])(' + LITERAL + r')\s*(==|!=|<=|>=|<|>)\s*' + word, code):
        suffix = code[match.end():].lstrip()
        if not starts_operand(match) or suffix and suffix[0] not in ')?:;,}&|':
            continue
        comparisons.add((reverse_op[match[2]], literal(match[1])))
        accept(match)
    for match in re.finditer(r'\b(?:if|while)\s*\(\s*(!?)\s*' + word + r'\s*\)', code):
        comparisons.add(('==' if match[1] else '!=', 0))
        accept(match)
    for match in re.finditer(word + r'\s*\?', code):
        if not starts_operand(match):
            continue
        comparisons.add(('!=', 0))
        accept(match)
    for match in re.finditer(word + r'\s*&\s*(' + LITERAL + r')' + end_operand, code):
        if not starts_operand(match):
            continue
        masks.add(literal(match[1]))
        accept(match)

    all_uses = [match.span() for match in re.finditer(word, code)]
    if any(span not in covered for span in all_uses):
        return None
    if not comparisons and not masks or comparisons and masks:
        return None
    common = dict(alias=name, source_expressions=evidence,
                  domain_complete_for_this_cpp_selector=True,
                  original_and_native_domain_qualification='not_established_by_this_audit',
                  method='All lexical alias occurrences must be declarations or recognized read-only literal tests.')
    if masks:
        if any(mask < 0 or mask > 255 for mask in masks):
            return None
        union = 0
        for mask in masks:
            union |= mask
        values = [value for value in range(union+1) if value & ~union == 0]
        return dict(common, kind='automated_cpp_mask_partition', mask=union,
                    masks=sorted(masks), representatives=values,
                    equivalence=f'Equal (int32_value & {union}) makes all direct literal mask uses equal.')
    if any(bound < INT_MIN or bound > INT_MAX for _, bound in comparisons):
        return None
    comparisons = sorted(comparisons)
    candidates = {INT_MIN, INT_MAX, 0}
    for _, bound in comparisons:
        candidates.update(v for v in (bound-1, bound, bound+1) if INT_MIN <= v <= INT_MAX)
    # Prefer small representatives; retain all distinct predicate signatures.
    reps, signatures = [], set()
    for value in sorted(candidates, key=lambda v: (abs(v), v)):
        signature = tuple(compare(value, op, bound) for op, bound in comparisons)
        if signature not in signatures:
            reps.append(value)
            signatures.add(signature)
    return dict(common, kind='automated_cpp_comparison_partition', comparisons=comparisons,
                representatives=sorted(reps),
                equivalence='Equal ordered predicate signatures for every direct literal comparison/truth test.')
